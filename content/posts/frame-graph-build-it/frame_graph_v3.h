#pragma once
// Frame Graph MVP v3 -- Lifetimes & Aliasing
// Adds: lifetime analysis, greedy free-list memory aliasing.
// Builds on v2 (dependencies, topo-sort, culling, barriers).
//
// Compile: clang++ -std=c++17 -o example_v3 example_v3.cpp
//     or: g++ -std=c++17 -o example_v3 example_v3.cpp

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
using BlockIndex    = uint32_t;   // index into the physical-block free list

struct ResourceHandle {
    ResourceIndex index = UINT32_MAX;
    bool IsValid() const { return index != UINT32_MAX; }
};

// == Resource state tracking ===================================
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

// Precomputed barrier — stored during Compile(), replayed during Execute().
struct Barrier {
    ResourceIndex resourceIndex;
    ResourceState oldState;
    ResourceState newState;
    bool          isAliasing   = false;     // aliasing barrier (block changes occupant)
    ResourceIndex aliasBefore  = UINT32_MAX; // resource being evicted
};

struct ResourceVersion {
    PassIndex writerPass = UINT32_MAX;
    std::vector<PassIndex> readerPasses;
    bool HasWriter() const { return writerPass != UINT32_MAX; }
};

struct ResourceEntry {
    ResourceDesc desc;
    std::vector<ResourceVersion> versions;
    ResourceState currentState = ResourceState::Undefined;
    bool imported = false;   // imported resources are not owned by the graph
};

// == Physical memory block ======================================
struct PhysicalBlock {
    uint32_t sizeBytes   = 0;
    PassIndex availAfter = 0;  // sorted pass after which this block is free
};

// == Allocation helpers ========================================

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

// == Lifetime info per resource =================================
struct Lifetime {
    PassIndex firstUse = UINT32_MAX;
    PassIndex lastUse  = 0;
    bool      isTransient = true;
};

// == Render pass ===============================================
struct RenderPass {
    std::string name;
    std::function<void()>             Setup;
    std::function<void(/*cmd list*/)> Execute;

    std::vector<ResourceHandle> reads;
    std::vector<ResourceHandle> writes;
    std::vector<ResourceHandle> readWrites;  // UAV (explicit)
    std::vector<PassIndex> dependsOn;
    std::vector<PassIndex> successors;
    uint32_t inDegree = 0;
    bool     alive    = false;
};

// == Frame graph (v3: full MVP) ================================
class FrameGraph {
public:
    ResourceHandle CreateResource(const ResourceDesc& desc);
    ResourceHandle ImportResource(const ResourceDesc& desc,
                                  ResourceState initialState = ResourceState::Undefined);

    void Read(PassIndex passIdx, ResourceHandle h);
    void Write(PassIndex passIdx, ResourceHandle h);
    void ReadWrite(PassIndex passIdx, ResourceHandle h);  // UAV access

    template <typename SetupFn, typename ExecFn>
    void AddPass(const std::string& name, SetupFn&& setup, ExecFn&& exec) {
        passes.push_back({ name, std::forward<SetupFn>(setup),
                                   std::forward<ExecFn>(exec) });
        passes.back().Setup();
    }

    // == compile — builds the execution plan + allocates memory ==
    struct CompiledPlan {
        std::vector<PassIndex> sorted;
        std::vector<BlockIndex> mapping;                 // mapping[ResourceIndex] → physical block
        std::vector<std::vector<Barrier>> barriers;
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
    std::vector<Lifetime> ScanLifetimes(const std::vector<PassIndex>& sorted);
    std::vector<BlockIndex> AliasResources(const std::vector<Lifetime>& lifetimes);
    ResourceState StateForUsage(PassIndex passIdx, ResourceHandle h, bool isWrite) const;
    std::vector<std::vector<Barrier>> ComputeBarriers(const std::vector<PassIndex>& sorted,
                                                       const std::vector<BlockIndex>& mapping);
    void EmitBarriers(const std::vector<Barrier>& barriers);
};
