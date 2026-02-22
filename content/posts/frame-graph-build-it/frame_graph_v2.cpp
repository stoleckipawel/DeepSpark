#include "frame_graph_v2.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
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

void FrameGraph::Execute() {
    printf("\n[1] Building dependency edges...\n");
    BuildEdges();
    printf("[2] Topological sort...\n");
    auto sorted = TopoSort();
    printf("[3] Culling dead passes...\n");
    Cull(sorted);

    printf("[4] Executing (with automatic barriers):\n");
    for (PassIndex idx : sorted) {
        if (!passes[idx].alive) {
            printf("  -- skip: %s (CULLED)\n", passes[idx].name.c_str());
            continue;
        }
        EmitBarriers(idx);
        passes[idx].Execute(/* &cmdList */);
    }
    passes.clear();
    entries.clear();
}

// == Build dependency edges ====================================

void FrameGraph::BuildEdges() {
    for (PassIndex i = 0; i < passes.size(); i++) {
        // Deduplicate dependency edges and build successor list.
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
        // Walk the adjacency list -- O(E) total across all nodes.
        for (PassIndex succ : passes[cur].successors) {
            if (--inDeg[succ] == 0)
                q.push(succ);
        }
    }
    assert(order.size() == passes.size() && "Cycle detected!");
    printf("  Topological order: ");
    for (uint32_t i = 0; i < order.size(); i++) {
        printf("%s%s", passes[order[i]].name.c_str(),
               i + 1 < order.size() ? " -> " : "\n");
    }
    return order;
}

// == Cull dead passes (backward walk from output) ==============

void FrameGraph::Cull(const std::vector<PassIndex>& sorted) {
    // Mark the last pass (present) as alive, then walk backward.
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

// == Emit barriers where resource state changes ===============

void FrameGraph::EmitBarriers(PassIndex passIdx) {
    auto IsUAV = [&](ResourceHandle h) {
        for (auto& rw : passes[passIdx].readWrites)
            if (rw.index == h.index) return true;
        return false;
    };
    auto StateForUsage = [&](ResourceHandle h, bool isWrite) {
        if (IsUAV(h)) return ResourceState::UnorderedAccess;
        if (isWrite)
            return (entries[h.index].desc.format == Format::D32F)
                ? ResourceState::DepthAttachment : ResourceState::ColorAttachment;
        return ResourceState::ShaderRead;
    };

    for (auto& h : passes[passIdx].reads) {
        ResourceState needed = StateForUsage(h, false);
        if (entries[h.index].currentState != needed) {
            printf("    barrier: resource[%u] %s -> %s\n",
                   h.index,
                   StateName(entries[h.index].currentState),
                   StateName(needed));
            entries[h.index].currentState = needed;
        }
    }
    for (auto& h : passes[passIdx].writes) {
        ResourceState needed = StateForUsage(h, true);
        if (entries[h.index].currentState != needed) {
            printf("    barrier: resource[%u] %s -> %s\n",
                   h.index,
                   StateName(entries[h.index].currentState),
                   StateName(needed));
            entries[h.index].currentState = needed;
        }
    }
}
