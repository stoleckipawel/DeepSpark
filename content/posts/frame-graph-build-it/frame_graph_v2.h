#pragma once
// Frame Graph MVP v2 -- Dependencies & Barriers
// Adds: resource versioning, DAG with adjacency list, Kahn's topo-sort,
//       pass culling, and automatic barrier insertion.
//
// Compile: clang++ -std=c++17 -o example_v2 example_v2.cpp
//     or: g++ -std=c++17 -o example_v2 example_v2.cpp

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// == Resource description (virtual until compile) ==============
enum class Format { RGBA8, RGBA16F, R8, D32F };

struct ResourceDesc {
    uint32_t width  = 0;
    uint32_t height = 0;
    Format   format = Format::RGBA8;
};

using ResourceIndex = uint32_t;
using PassIndex     = uint32_t;

struct ResourceHandle {
    ResourceIndex index = UINT32_MAX;
    bool IsValid() const { return index != UINT32_MAX; }
};

// == Resource state tracking ====================================
enum class ResourceState { Undefined, ColorAttachment, DepthAttachment,
                           ShaderRead, UnorderedAccess, Present };

inline const char* StateName(ResourceState s) {
    switch (s) {
        case ResourceState::Undefined:       return "Undefined";
        case ResourceState::ColorAttachment: return "ColorAttachment";
        case ResourceState::DepthAttachment: return "DepthAttachment";
        case ResourceState::ShaderRead:      return "ShaderRead";
        case ResourceState::UnorderedAccess: return "UnorderedAccess";
        case ResourceState::Present:         return "Present";
        default:                             return "?";
    }
}

struct ResourceVersion {
    PassIndex writerPass = UINT32_MAX;   // which pass wrote this version
    std::vector<PassIndex> readerPasses; // which passes read it
    bool HasWriter() const { return writerPass != UINT32_MAX; }
};

// A single resource-state transition.
struct Barrier {
    ResourceIndex resourceIndex;
    ResourceState oldState;
    ResourceState newState;
};

// Extend ResourceDesc with tracking:
struct ResourceEntry {
    ResourceDesc desc;
    std::vector<ResourceVersion> versions;  // version 0, 1, 2...
    ResourceState currentState = ResourceState::Undefined;
    bool imported = false;   // imported resources are not owned by the graph
};

// == Updated render pass =======================================
struct RenderPass {
    std::string name;
    std::function<void()>             Setup;
    std::function<void(/*cmd list*/)> Execute;

    std::vector<ResourceHandle> reads;
    std::vector<ResourceHandle> writes;
    std::vector<ResourceHandle> readWrites; // UAV (explicit)
    std::vector<PassIndex> dependsOn;     // passes this pass depends on
    std::vector<PassIndex> successors;    // passes that depend on this pass
    uint32_t inDegree = 0;                // for Kahn's
    bool     alive    = false;            // for culling
};

// == Updated FrameGraph ========================================
class FrameGraph {
public:
    ResourceHandle CreateResource(const ResourceDesc& desc);

    // Import an external resource (e.g. swapchain backbuffer).
    // The graph tracks barriers but does not own or alias its memory.
    ResourceHandle ImportResource(const ResourceDesc& desc,
                                  ResourceState initialState = ResourceState::Undefined);

    // Declare a read -- links this pass to the resource's current version.
    void Read(PassIndex passIdx, ResourceHandle h);

    // Declare a write -- creates a new version of the resource.
    void Write(PassIndex passIdx, ResourceHandle h);

    // Declare a UAV access -- read + write on the same version.
    void ReadWrite(PassIndex passIdx, ResourceHandle h);

    template <typename SetupFn, typename ExecFn>
    void AddPass(const std::string& name, SetupFn&& setup, ExecFn&& exec) {
        passes.push_back({ name, std::forward<SetupFn>(setup),
                                   std::forward<ExecFn>(exec) });
        passes.back().Setup();
    }

    // == v2: compile — precompute sort, cull, barriers =========
    struct CompiledPlan {
        std::vector<PassIndex> sorted;                    // topological execution order
        std::vector<std::vector<Barrier>> barriers;       // barriers[orderIdx] → pre-pass transitions
    };

    CompiledPlan Compile();
    void Execute(const CompiledPlan& plan);

    // convenience: compile + execute in one call
    void Execute();

private:
    std::vector<RenderPass>    passes;
    std::vector<ResourceEntry> entries;

    void BuildEdges();
    std::vector<PassIndex> TopoSort();
    void Cull(const std::vector<PassIndex>& sorted);
    std::vector<std::vector<Barrier>> ComputeBarriers(const std::vector<PassIndex>& sorted);
    void EmitBarriers(const std::vector<Barrier>& barriers);
};
