#include "frame_graph_v3.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <numeric>
#include <queue>
#include <unordered_set>

// == FrameGraph implementation =================================

ResourceHandle FrameGraph::CreateResource(const ResourceDesc& desc) {
    entries.push_back({ desc, {{}}, ResourceState::Undefined, false });
    return { static_cast<ResourceIndex>(entries.size() - 1) };
}

ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc,
                                          ResourceState initialState) {
    entries.push_back({ desc, {{}}, initialState, true });
    return { static_cast<ResourceIndex>(entries.size() - 1) };
}

void FrameGraph::Read(PassIndex passIdx, ResourceHandle h) {
    auto& ver = entries[h.index].versions.back();
    if (ver.HasWriter()) {
        passes[passIdx].dependsOn.push_back(ver.writerPass);
    }
    ver.readerPasses.push_back(passIdx);
    passes[passIdx].reads.push_back(h);
}

void FrameGraph::Write(PassIndex passIdx, ResourceHandle h) {
    entries[h.index].versions.push_back({});
    entries[h.index].versions.back().writerPass = passIdx;
    passes[passIdx].writes.push_back(h);
}

void FrameGraph::ReadWrite(PassIndex passIdx, ResourceHandle h) {
    auto& ver = entries[h.index].versions.back();
    if (ver.HasWriter()) {
        passes[passIdx].dependsOn.push_back(ver.writerPass);
    }
    entries[h.index].versions.push_back({});
    entries[h.index].versions.back().writerPass = passIdx;
    passes[passIdx].reads.push_back(h);
    passes[passIdx].writes.push_back(h);
    passes[passIdx].readWrites.push_back(h);
}

// == v3: compile -- builds the execution plan + allocates memory ==

FrameGraph::CompiledPlan FrameGraph::Compile() {
    printf("\n[1] Building dependency edges...\n");
    BuildEdges();
    printf("[2] Topological sort...\n");
    auto sorted   = TopoSort();
    printf("[3] Culling dead passes...\n");
    Cull(sorted);
    printf("[4] Scanning resource lifetimes...\n");
    auto lifetimes = ScanLifetimes(sorted);   // NEW v3
    printf("[5] Aliasing resources (greedy free-list)...\n");
    auto mapping   = AliasResources(lifetimes); // NEW v3
    printf("[6] Computing barriers...\n");
    auto barriers  = ComputeBarriers(sorted, mapping);   // NEW v3

    // The compiled plan is fully determined — execution order, memory
    // mapping, and every barrier transition.  Execute is pure playback.
    return { std::move(sorted), std::move(mapping), std::move(barriers) };
}

// == v3: execute -- runs the compiled plan =====================

void FrameGraph::EmitBarriers(const std::vector<Barrier>& barriers) {
    for (auto& b : barriers) {
        if (b.isAliasing) {
            printf("    aliasing barrier: physical block shared by resource[%u] -> resource[%u]\n",
                   b.aliasBefore, b.resourceIndex);
        } else {
            printf("    barrier: resource[%u] %s -> %s\n",
                   b.resourceIndex,
                   StateName(b.oldState),
                   StateName(b.newState));
        }
        // e.g. vkCmdPipelineBarrier / ID3D12GraphicsCommandList::ResourceBarrier
    }
}

void FrameGraph::Execute(const CompiledPlan& plan) {
    printf("[7] Executing (replaying precomputed barriers):\n");
    for (PassIndex orderIdx = 0; orderIdx < plan.sorted.size(); orderIdx++) {
        PassIndex passIdx = plan.sorted[orderIdx];
        if (!passes[passIdx].alive) {
            printf("  -- skip: %s (CULLED)\n", passes[passIdx].name.c_str());
            continue;
        }
        EmitBarriers(plan.barriers[orderIdx]);
        passes[passIdx].Execute(/* &cmdList */);
    }
    passes.clear();
    entries.clear();
}

// convenience: compile + execute in one call
void FrameGraph::Execute() { Execute(Compile()); }

// == Build dependency edges ====================================

void FrameGraph::BuildEdges() {
    for (PassIndex i = 0; i < passes.size(); i++) {
        std::unordered_set<PassIndex> seen;
        for (PassIndex dep : passes[i].dependsOn) {
            if (seen.insert(dep).second) {
                passes[dep].successors.push_back(i);
                passes[i].inDegree++;
            }
        }
    }
}

// == Kahn's topological sort -- O(V + E) ========================

std::vector<PassIndex> FrameGraph::TopoSort() {
    std::queue<PassIndex> q;
    std::vector<uint32_t> inDeg(passes.size());
    for (PassIndex i = 0; i < passes.size(); i++) {
        inDeg[i] = passes[i].inDegree;
        if (inDeg[i] == 0) q.push(i);
    }
    std::vector<PassIndex> order;
    while (!q.empty()) {
        PassIndex cur = q.front(); q.pop();
        order.push_back(cur);
        for (PassIndex succ : passes[cur].successors) {
            if (--inDeg[succ] == 0)
                q.push(succ);
        }
    }
    assert(order.size() == passes.size() && "Cycle detected!");
    printf("  Topological order: ");
    for (PassIndex i = 0; i < order.size(); i++) {
        printf("%s%s", passes[order[i]].name.c_str(),
               i + 1 < order.size() ? " -> " : "\n");
    }
    return order;
}

// == Cull dead passes ==========================================

void FrameGraph::Cull(const std::vector<PassIndex>& sorted) {
    if (sorted.empty()) return;
    passes[sorted.back()].alive = true;
    for (int i = static_cast<int>(sorted.size()) - 1; i >= 0; i--) {
        if (!passes[sorted[i]].alive) continue;
        for (PassIndex dep : passes[sorted[i]].dependsOn)
            passes[dep].alive = true;
    }
    printf("  Culling result:   ");
    for (PassIndex i = 0; i < passes.size(); i++) {
        printf("%s=%s%s", passes[i].name.c_str(),
               passes[i].alive ? "ALIVE" : "DEAD",
               i + 1 < passes.size() ? ", " : "\n");
    }
}

// == State inference (NEW v3 — shared by ComputeBarriers) =====

ResourceState FrameGraph::StateForUsage(PassIndex passIdx, ResourceHandle h, bool isWrite) const {
    // If the caller registered this handle via ReadWrite(), it's a UAV.
    for (auto& rw : passes[passIdx].readWrites)
        if (rw.index == h.index) return ResourceState::UnorderedAccess;
    if (isWrite)
        return (entries[h.index].desc.format == Format::D32F)
            ? ResourceState::DepthAttachment : ResourceState::ColorAttachment;
    return ResourceState::ShaderRead;
}

// == Compute barriers (NEW v3 — runs during Compile) ==========

std::vector<std::vector<Barrier>> FrameGraph::ComputeBarriers(
        const std::vector<PassIndex>& sorted,
        const std::vector<BlockIndex>& mapping) {
    std::vector<std::vector<Barrier>> result(sorted.size());

    // blockOwner[block] = which virtual resource currently occupies it.
    std::vector<ResourceIndex> blockOwner;
    {
        BlockIndex maxBlock = 0;
        for (auto m : mapping) if (m != UINT32_MAX) maxBlock = std::max(maxBlock, m + 1);
        blockOwner.assign(maxBlock, UINT32_MAX);
    }

    for (PassIndex orderIdx = 0; orderIdx < sorted.size(); orderIdx++) {
        PassIndex passIdx = sorted[orderIdx];
        if (!passes[passIdx].alive) continue;

        // --- Phase 1: aliasing barriers (block changes occupant) ---
        auto emitAliasingIfNeeded = [&](ResourceHandle h) {
            BlockIndex block = mapping[h.index];
            if (block == UINT32_MAX) return;
            if (blockOwner[block] != UINT32_MAX && blockOwner[block] != h.index) {
                result[orderIdx].push_back(
                    { h.index, ResourceState::Undefined, ResourceState::Undefined,
                      true, blockOwner[block] });
            }
            blockOwner[block] = h.index;
        };
        for (auto& h : passes[passIdx].reads)  emitAliasingIfNeeded(h);
        for (auto& h : passes[passIdx].writes) emitAliasingIfNeeded(h);

        // --- Phase 2: state-transition barriers ---
        auto recordTransition = [&](ResourceHandle h, bool isWrite) {
            ResourceState needed = StateForUsage(passIdx, h, isWrite);
            if (entries[h.index].currentState != needed) {
                result[orderIdx].push_back(
                    { h.index, entries[h.index].currentState, needed });
                entries[h.index].currentState = needed;
            }
        };
        for (auto& h : passes[passIdx].reads)  recordTransition(h, false);
        for (auto& h : passes[passIdx].writes) recordTransition(h, true);
    }

    uint32_t total = 0;
    for (auto& v : result) total += static_cast<uint32_t>(v.size());
    printf("  Barriers computed: %u transition(s) across %u passes\n",
           total, static_cast<uint32_t>(sorted.size()));
    return result;
}

// == Scan lifetimes (NEW v3) ===================================

std::vector<Lifetime> FrameGraph::ScanLifetimes(const std::vector<PassIndex>& sorted) {
    std::vector<Lifetime> life(entries.size());

    // Imported resources are not transient -- skip them during aliasing.
    for (ResourceIndex i = 0; i < entries.size(); i++) {
        if (entries[i].imported) life[i].isTransient = false;
    }

    for (PassIndex order = 0; order < sorted.size(); order++) {
        PassIndex passIdx = sorted[order];
        if (!passes[passIdx].alive) continue;

        for (auto& h : passes[passIdx].reads) {
            life[h.index].firstUse = std::min(life[h.index].firstUse, order);
            life[h.index].lastUse  = std::max(life[h.index].lastUse,  order);
        }
        for (auto& h : passes[passIdx].writes) {
            life[h.index].firstUse = std::min(life[h.index].firstUse, order);
            life[h.index].lastUse  = std::max(life[h.index].lastUse,  order);
        }
    }
    printf("  Lifetimes (in sorted pass order):\n");
    for (ResourceIndex i = 0; i < life.size(); i++) {
        if (life[i].firstUse == UINT32_MAX) {
            printf("    resource[%u] unused (dead)\n", i);
        } else {
            printf("    resource[%u] alive [pass %u .. pass %u]\n",
                   i, life[i].firstUse, life[i].lastUse);
        }
    }
    return life;
}

// == Greedy free-list aliasing (NEW v3) ========================

std::vector<BlockIndex> FrameGraph::AliasResources(const std::vector<Lifetime>& lifetimes) {
    std::vector<PhysicalBlock> freeList;
    std::vector<BlockIndex> mapping(entries.size(), UINT32_MAX);
    uint32_t totalWithout = 0;

    std::vector<ResourceIndex> indices(entries.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](ResourceIndex a, ResourceIndex b) {
        return lifetimes[a].firstUse < lifetimes[b].firstUse;
    });

    printf("  Aliasing:\n");
    for (ResourceIndex resIdx : indices) {
        if (!lifetimes[resIdx].isTransient) continue;
        if (lifetimes[resIdx].firstUse == UINT32_MAX) continue;

        uint32_t needed = AllocSize(entries[resIdx].desc);
        totalWithout += needed;
        bool reused = false;

        for (BlockIndex b = 0; b < freeList.size(); b++) {
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
            mapping[resIdx] = static_cast<BlockIndex>(freeList.size());
            printf("    resource[%u] -> NEW physical block %u   "
                   "(%.1f MB, lifetime [%u..%u])\n",
                   resIdx, static_cast<BlockIndex>(freeList.size()),
                   needed / (1024.0f * 1024.0f),
                   lifetimes[resIdx].firstUse,
                   lifetimes[resIdx].lastUse);
            freeList.push_back({ needed, lifetimes[resIdx].lastUse });
        }
    }

    uint32_t totalWith = 0;
    for (auto& blk : freeList) totalWith += blk.sizeBytes;
    printf("  Memory: %u physical blocks for %u virtual resources\n",
           static_cast<uint32_t>(freeList.size()),
           static_cast<uint32_t>(entries.size()));
    printf("  Without aliasing: %.1f MB\n",
           totalWithout / (1024.0f * 1024.0f));
    printf("  With aliasing:    %.1f MB (saved %.1f MB, %.0f%%)\n",
           totalWith / (1024.0f * 1024.0f),
           (totalWithout - totalWith) / (1024.0f * 1024.0f),
           totalWithout > 0 ? 100.0f * (totalWithout - totalWith) / totalWithout : 0.0f);

    return mapping;
}
