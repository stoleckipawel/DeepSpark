#include "frame_graph_v2.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <queue>
#include <unordered_set>

// ── FrameGraph implementation ─────────────────────────────────

ResourceHandle FrameGraph::createResource(const ResourceDesc& desc) {
    entries_.push_back({ desc, {{}}, ResourceState::Undefined, false });
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

void FrameGraph::execute() {
    printf("\n[1] Building dependency edges...\n");
    buildEdges();
    printf("[2] Topological sort...\n");
    auto sorted = topoSort();
    printf("[3] Culling dead passes...\n");
    cull(sorted);

    printf("[4] Executing (with automatic barriers):\n");
    for (uint32_t idx : sorted) {
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

// ── Build dependency edges ────────────────────────────────────

void FrameGraph::buildEdges() {
    for (uint32_t i = 0; i < passes_.size(); i++) {
        // Deduplicate dependency edges and build successor list.
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
        // Walk the adjacency list — O(E) total across all nodes.
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

// ── Cull dead passes (backward walk from output) ──────────────

void FrameGraph::cull(const std::vector<uint32_t>& sorted) {
    // Mark the last pass (present) as alive, then walk backward.
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

// ── Insert barriers where resource state changes ──────────────

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
