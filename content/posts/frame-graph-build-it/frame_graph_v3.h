#pragma once
// Frame Graph MVP v3 -- Lifetimes & Aliasing
// Adds: lifetime analysis, greedy free-list memory aliasing.
// Builds on v2 (dependencies, topo-sort, culling, barriers).
//
// Compile: g++ -std=c++17 -o example_v3 example_v3.cpp frame_graph_v3.cpp

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

struct ResourceHandle {
    uint32_t index = UINT32_MAX;
    bool IsValid() const { return index != UINT32_MAX; }
};

// == Resource state tracking ===================================
enum class ResourceState { Undefined, ColorAttachment, DepthAttachment,
                           ShaderRead, Present };

inline const char* StateName(ResourceState s) {
    switch (s) {
        case ResourceState::Undefined:       return "Undefined";
        case ResourceState::ColorAttachment: return "ColorAttachment";
        case ResourceState::DepthAttachment: return "DepthAttachment";
        case ResourceState::ShaderRead:      return "ShaderRead";
        case ResourceState::Present:         return "Present";
        default:                             return "?";
    }
}

// == Precomputed barrier (NEW v3) ==============================
struct Barrier {
    uint32_t      resourceIndex;
    ResourceState oldState;
    ResourceState newState;
};

struct ResourceVersion {
    uint32_t writerPass = UINT32_MAX;
    std::vector<uint32_t> readerPasses;
    bool HasWriter() const { return writerPass != UINT32_MAX; }
};

struct ResourceEntry {
    ResourceDesc desc;
    std::vector<ResourceVersion> versions;
    ResourceState currentState = ResourceState::Undefined;
    bool imported = false;   // imported resources are not owned by the graph
};

// == Physical memory block (NEW v3) ============================
struct PhysicalBlock {
    uint32_t sizeBytes   = 0;
    uint32_t availAfter  = 0;  // pass index after which this block is free
};

// == Allocation helpers (NEW v3) ================================

// Minimum placement alignment for aliased heap resources.
// Real APIs enforce similar constraints (e.g. 64 KB on most GPUs).
static constexpr uint32_t kPlacementAlignment = 65536;  // 64 KB

inline uint32_t AlignUp(uint32_t value, uint32_t alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

inline uint32_t BytesPerPixel(Format fmt) {
    switch (fmt) {
        case Format::R8:      return 1;
        case Format::RGBA8:   return 4;
        case Format::D32F:    return 4;
        case Format::RGBA16F: return 8;
        default:              return 4;
    }
}

// Compute aligned allocation size for a resource.
// Real drivers add row padding and tiling overhead; we approximate
// with a simple alignment round-up to demonstrate the principle.
inline uint32_t AllocSize(const ResourceDesc& desc) {
    uint32_t raw = desc.width * desc.height * BytesPerPixel(desc.format);
    return AlignUp(raw, kPlacementAlignment);
}

// == Lifetime info per resource (NEW v3) =======================
struct Lifetime {
    uint32_t firstUse = UINT32_MAX;
    uint32_t lastUse  = 0;
    bool     isTransient = true;
};

// == Render pass ===============================================
struct RenderPass {
    std::string name;
    std::function<void()>             Setup;
    std::function<void(/*cmd list*/)> Execute;

    std::vector<ResourceHandle> reads;
    std::vector<ResourceHandle> writes;
    std::vector<uint32_t> dependsOn;
    std::vector<uint32_t> successors;
    uint32_t inDegree = 0;
    bool     alive    = false;
};

// == Frame graph (v3: full MVP) ================================
class FrameGraph {
public:
    ResourceHandle CreateResource(const ResourceDesc& desc);
    ResourceHandle ImportResource(const ResourceDesc& desc,
                                  ResourceState initialState = ResourceState::Undefined);

    void Read(uint32_t passIdx, ResourceHandle h);
    void Write(uint32_t passIdx, ResourceHandle h);

    template <typename SetupFn, typename ExecFn>
    void AddPass(const std::string& name, SetupFn&& setup, ExecFn&& exec) {
        passes.push_back({ name, std::forward<SetupFn>(setup),
                                   std::forward<ExecFn>(exec) });
        passes.back().Setup();
    }

    // == v3: compile -- builds the execution plan + allocates memory ==
    struct CompiledPlan {
        std::vector<uint32_t> sorted;
        std::vector<uint32_t> mapping;                  // mapping[virtualIdx] → physicalBlock
        std::vector<std::vector<Barrier>> barriers;     // barriers[orderIdx] → pre-pass transitions
    };

    CompiledPlan Compile();

    // == v3: execute -- runs the compiled plan =================
    void Execute(const CompiledPlan& plan);

    // convenience: compile + execute in one call
    void Execute();

private:
    std::vector<RenderPass>    passes;
    std::vector<ResourceEntry> entries;

    void BuildEdges();
    std::vector<uint32_t> TopoSort();
    void Cull(const std::vector<uint32_t>& sorted);
    std::vector<Lifetime> ScanLifetimes(const std::vector<uint32_t>& sorted);  // NEW v3
    std::vector<uint32_t> AliasResources(const std::vector<Lifetime>& lifetimes); // NEW v3
    std::vector<std::vector<Barrier>> ComputeBarriers(const std::vector<uint32_t>& sorted); // NEW v3
};
