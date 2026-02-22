#include "frame_graph_v3.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <numeric>
#include <queue>
#include <unordered_set>

// ── FrameGraph implementation ─────────────────────────────────

ResourceHandle FrameGraph::createResource(const ResourceDesc& desc) {
    entries_.push_back({ desc, {{}}, ResourceState::Undefined });
    return { static_cast<uint32_t>(entries_.size() - 1) };
}

ResourceHandle FrameGraph::importResource(const ResourceDesc& desc,
                                          ResourceState initialState) {
    entries_.push_back({ desc, {{}}, initialState, true });
    return { static_cast<uint32_t>(entries_.size() - 1) };
}

void FrameGraph::read(uint32_t passIdx, ResourceHandle h) {
    auto& ver = entries_[h.index].versions.back();
    if (ver.writerPass != UINT32_MAX) {
        passes_[passIdx].dependsOn.push_back(ver.writerPass);
    }
    ver.readerPasses.push_back(passIdx);
    passes_[passIdx].reads.push_back(h);
}

void FrameGraph::write(uint32_t passIdx, ResourceHandle h) {
    entries_[h.index].versions.push_back({});
    entries_[h.index].versions.back().writerPass = passIdx;
    passes_[passIdx].writes.push_back(h);
}

// ── v3: compile — builds the execution plan + allocates memory ──

FrameGraph::CompiledPlan FrameGraph::compile() {
    printf("\n[1] Building dependency edges...\n");
    buildEdges();
    printf("[2] Topological sort...\n");
    auto sorted   = topoSort();
    printf("[3] Culling dead passes...\n");
    cull(sorted);
    printf("[4] Scanning resource lifetimes...\n");
    auto lifetimes = scanLifetimes(sorted);   // NEW v3
    printf("[5] Aliasing resources (greedy free-list)...\n");
    auto mapping   = aliasResources(lifetimes); // NEW v3

    // Physical bindings are now decided — execute can't change them.
    // This makes the compiled plan cacheable and thread-safe.
    return { std::move(sorted), std::move(mapping) };
}

// ── v3: execute — runs the compiled plan ─────────────────────

void FrameGraph::execute(const CompiledPlan& plan) {
    printf("[6] Executing (with automatic barriers):\n");
    for (uint32_t idx : plan.sorted) {
        if (!passes_[idx].alive) {
            printf("  -- skip: %s (CULLED)\n", passes_[idx].name.c_str());
            continue;
        }
        insertBarriers(idx);
        passes_[idx].execute(/* &cmdList */);
    }
    passes_.clear();
    entries_.clear();
}

// convenience: compile + execute in one call
void FrameGraph::execute() { execute(compile()); }

// ── Build dependency edges ────────────────────────────────────

void FrameGraph::buildEdges() {
    for (uint32_t i = 0; i < passes_.size(); i++) {
        std::unordered_set<uint32_t> seen;
        for (uint32_t dep : passes_[i].dependsOn) {
            if (seen.insert(dep).second) {
                passes_[dep].successors.push_back(i);
                passes_[i].inDegree++;
            }
        }
    }
}

// ── Kahn's topological sort — O(V + E) ────────────────────────

std::vector<uint32_t> FrameGraph::topoSort() {
    std::queue<uint32_t> q;
    std::vector<uint32_t> inDeg(passes_.size());
    for (uint32_t i = 0; i < passes_.size(); i++) {
        inDeg[i] = passes_[i].inDegree;
        if (inDeg[i] == 0) q.push(i);
    }
    std::vector<uint32_t> order;
    while (!q.empty()) {
        uint32_t cur = q.front(); q.pop();
        order.push_back(cur);
        for (uint32_t succ : passes_[cur].successors) {
            if (--inDeg[succ] == 0)
                q.push(succ);
        }
    }
    assert(order.size() == passes_.size() && "Cycle detected!");
    printf("  Topological order: ");
    for (uint32_t i = 0; i < order.size(); i++) {
        printf("%s%s", passes_[order[i]].name.c_str(),
               i + 1 < order.size() ? " -> " : "\n");
    }
    return order;
}

// ── Cull dead passes ──────────────────────────────────────────

void FrameGraph::cull(const std::vector<uint32_t>& sorted) {
    if (sorted.empty()) return;
    passes_[sorted.back()].alive = true;
    for (int i = static_cast<int>(sorted.size()) - 1; i >= 0; i--) {
        if (!passes_[sorted[i]].alive) continue;
        for (uint32_t dep : passes_[sorted[i]].dependsOn)
            passes_[dep].alive = true;
    }
    printf("  Culling result:   ");
    for (uint32_t i = 0; i < passes_.size(); i++) {
        printf("%s=%s%s", passes_[i].name.c_str(),
               passes_[i].alive ? "ALIVE" : "DEAD",
               i + 1 < passes_.size() ? ", " : "\n");
    }
}

// ── Insert barriers ───────────────────────────────────────────

void FrameGraph::insertBarriers(uint32_t passIdx) {
    auto stateForUsage = [](bool isWrite, Format fmt) {
        if (isWrite)
            return (fmt == Format::D32F) ? ResourceState::DepthAttachment
                                         : ResourceState::ColorAttachment;
        return ResourceState::ShaderRead;
    };

    for (auto& h : passes_[passIdx].reads) {
        ResourceState needed = ResourceState::ShaderRead;
        if (entries_[h.index].currentState != needed) {
            printf("    barrier: resource[%u] %s -> %s\n",
                   h.index,
                   stateName(entries_[h.index].currentState),
                   stateName(needed));
            entries_[h.index].currentState = needed;
        }
    }
    for (auto& h : passes_[passIdx].writes) {
        ResourceState needed = stateForUsage(true, entries_[h.index].desc.format);
        if (entries_[h.index].currentState != needed) {
            printf("    barrier: resource[%u] %s -> %s\n",
                   h.index,
                   stateName(entries_[h.index].currentState),
                   stateName(needed));
            entries_[h.index].currentState = needed;
        }
    }
}

// ── Scan lifetimes (NEW v3) ───────────────────────────────────

std::vector<Lifetime> FrameGraph::scanLifetimes(const std::vector<uint32_t>& sorted) {
    std::vector<Lifetime> life(entries_.size());

    // Imported resources are not transient — skip them during aliasing.
    for (uint32_t i = 0; i < entries_.size(); i++) {
        if (entries_[i].imported) life[i].isTransient = false;
    }

    for (uint32_t order = 0; order < sorted.size(); order++) {
        uint32_t passIdx = sorted[order];
        if (!passes_[passIdx].alive) continue;

        for (auto& h : passes_[passIdx].reads) {
            life[h.index].firstUse = std::min(life[h.index].firstUse, order);
            life[h.index].lastUse  = std::max(life[h.index].lastUse,  order);
        }
        for (auto& h : passes_[passIdx].writes) {
            life[h.index].firstUse = std::min(life[h.index].firstUse, order);
            life[h.index].lastUse  = std::max(life[h.index].lastUse,  order);
        }
    }
    printf("  Lifetimes (in sorted pass order):\n");
    for (uint32_t i = 0; i < life.size(); i++) {
        if (life[i].firstUse == UINT32_MAX) {
            printf("    resource[%u] unused (dead)\n", i);
        } else {
            printf("    resource[%u] alive [pass %u .. pass %u]\n",
                   i, life[i].firstUse, life[i].lastUse);
        }
    }
    return life;
}

// ── Greedy free-list aliasing (NEW v3) ────────────────────────

std::vector<uint32_t> FrameGraph::aliasResources(const std::vector<Lifetime>& lifetimes) {
    std::vector<PhysicalBlock> freeList;
    std::vector<uint32_t> mapping(entries_.size(), UINT32_MAX);
    uint32_t totalWithout = 0;

    std::vector<uint32_t> indices(entries_.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](uint32_t a, uint32_t b) {
        return lifetimes[a].firstUse < lifetimes[b].firstUse;
    });

    printf("  Aliasing:\n");
    for (uint32_t resIdx : indices) {
        if (!lifetimes[resIdx].isTransient) continue;
        if (lifetimes[resIdx].firstUse == UINT32_MAX) continue;

        uint32_t needed = entries_[resIdx].desc.width
                        * entries_[resIdx].desc.height
                        * bytesPerPixel(entries_[resIdx].desc.format);
        totalWithout += needed;
        bool reused = false;

        for (uint32_t b = 0; b < freeList.size(); b++) {
            if (freeList[b].availAfter < lifetimes[resIdx].firstUse
                && freeList[b].sizeBytes >= needed) {
                mapping[resIdx] = b;
                freeList[b].availAfter = lifetimes[resIdx].lastUse;
                reused = true;
                printf("    resource[%u] -> reuse physical block %u  "
                       "(%.1f MB, lifetime [%u..%u])\n",
                       resIdx, b, needed / (1024.0f * 1024.0f),
                       lifetimes[resIdx].firstUse,
                       lifetimes[resIdx].lastUse);
                break;
            }
        }

        if (!reused) {
            mapping[resIdx] = static_cast<uint32_t>(freeList.size());
            printf("    resource[%u] -> NEW physical block %u   "
                   "(%.1f MB, lifetime [%u..%u])\n",
                   resIdx, static_cast<uint32_t>(freeList.size()),
                   needed / (1024.0f * 1024.0f),
                   lifetimes[resIdx].firstUse,
                   lifetimes[resIdx].lastUse);
            freeList.push_back({ needed, entries_[resIdx].desc.format,
                                 lifetimes[resIdx].lastUse });
        }
    }

    uint32_t totalWith = 0;
    for (auto& blk : freeList) totalWith += blk.sizeBytes;
    printf("  Memory: %u physical blocks for %u virtual resources\n",
           static_cast<uint32_t>(freeList.size()),
           static_cast<uint32_t>(entries_.size()));
    printf("  Without aliasing: %.1f MB\n",
           totalWithout / (1024.0f * 1024.0f));
    printf("  With aliasing:    %.1f MB (saved %.1f MB, %.0f%%)\n",
           totalWith / (1024.0f * 1024.0f),
           (totalWithout - totalWith) / (1024.0f * 1024.0f),
           totalWithout > 0 ? 100.0f * (totalWithout - totalWith) / totalWithout : 0.0f);

    return mapping;
}
