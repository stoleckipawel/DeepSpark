---
title: "Frame Graph — MVP to Real Engines"
date: 2026-02-09
draft: true
description: "Article plan preview — Frame Graph implementation & usage, from MVP to production engines."
tags: ["rendering", "frame-graph", "gpu", "architecture"]
categories: ["analysis"]
series: ["Rendering Architecture"]
---

<!-- This is a preview-only file generated from ARTICLE_PLAN.md. Not the final article. -->

# Part I — Theory

*What a render graph is, what problems it solves, and why every major engine uses one.*

---

## Why You Want One

1. **Composability.** Add a pass, remove a pass, reorder passes. Nothing breaks. The graph recompiles.
2. **Memory.** 30–50% VRAM saved. The compiler alias-packs transient resources. No human does this reliably at 20+ passes.
3. **Barriers.** Automatic. Correct. You never write one again.
4. **Inspectability.** The frame is data. Export it, diff it, visualize it.

Frostbite introduced the frame graph at GDC 2017. UE5 ships it as **RDG**. Unity has its own in SRP. Every major renderer now uses one — here's why, and how to build your own.

| What you'll get | |
|---|---|
| **Build** | A working frame graph in C++, from blank file to aliasing + barriers |
| **Map** | Every piece to UE5's RDG — read the source with confidence |
| **Compare** | Where Frostbite's design diverges, and where it's still ahead |

If you've watched VRAM spike from non-overlapping textures or chased a black screen after reordering a pass — this is for you.

---

## The Problem

**Month 1 — 3 passes, everything's fine.**
Depth prepass → GBuffer → lighting. Two barriers, hand-placed. Two textures, both allocated at init. Code is clean, readable, correct.

> At this scale, manual management works. You know every resource by name.

**Month 6 — 12 passes, cracks appear.**
Same renderer, now with SSAO, SSR, bloom, TAA, shadow cascades. Three things going wrong in the same codebase:

- Someone adds SSAO but doesn't realize the GBuffer pass needs an updated barrier now — nothing in the code makes that dependency visible. Visual artifacts on fresh build.
- VRAM is now 380 MB. Someone notices the SSAO transient texture and the bloom transient texture never overlap — but aliasing them would mean auditing both passes and every future pass that might touch them. No one does it.
- Two branches touch the render loop the same week. Git merges them cleanly — but the shadow pass ends up *after* lighting. The code compiles, nothing tests for render order, and subtly wrong lighting ships unnoticed.

> No single change broke it. The *accumulation* broke it.

**Month 18 — 25 passes, nobody touches it.**
The renderer works, but:

- VRAM is 900 MB. Profiling shows 400 MB is aliasable — but the lifetime analysis would take a week and break the next time anyone adds a pass.
- There are 47 barrier calls. Three are redundant, two are missing, one is in the wrong queue. Nobody knows which.
- Adding a new pass takes 2 days — 30 minutes to write the shader, the rest to figure out where to slot it and what barriers it needs.

> The renderer isn't wrong. It's *fragile*. Every change is a risk.

```
  Month 1              Month 6              Month 18
  ───────              ───────              ────────
  3 passes             12 passes            25 passes
  2 barriers           18 barriers          47 barriers
  ~40 MB VRAM          380 MB VRAM          900 MB VRAM
  0 aliasable          ~80 MB aliasable     400 MB aliasable
  ✓ manageable         ⚠ fragile            ✗ untouchable

  Complexity:
  ██                   ██████████           █████████████████████████
  ↑ linear growth      ↑ super-linear       ↑ every change is risk
    in passes            in mental load        nobody volunteers
```

The pattern is always the same: manual resource management works at small scale and fails at compound scale. Not because engineers are sloppy — because *no human tracks 25 lifetimes and 47 transitions in their head every sprint*. You need a system that sees the whole frame at once.

---

## The Core Idea

A render graph is like organizing pierogi night:

- Each pass is a step — make the dough, prepare the filling, fold, boil, fry.
- Each resource is a bowl or tool with a lifecycle — "big mixing bowl: needed for dough and filling, free after folding."
- The graph compiler is whoever reads all the steps first, figures out which bowls to reuse, and picks an order where nothing waits.

More precisely:

- **Directed Acyclic Graph (DAG)** of render passes.
- **Edges** are resource dependencies (read/write).
- **Frame** = declare passes → compile → execute.

```
  ┌───────┐   GBuffer    ┌──────┐   SSAO tex   ┌─────────┐
  │Depth  │─────────▶│GBuf  │──────────▶│Lighting│
  │Prepass│            │Pass  │            │         │
  └───────┘            └───┬──┘            └────┬────┘
       │                │                     │
       │ depth tex       │ GBuffer              │ HDR
       │                ┌─┴────┐              ┌───┴─────┐
       └─────────────▶│ SSAO │────────────▶│ Tonemap │─▶ Present
                       └──────┘              └─────────┘

  Nodes = passes     Edges = resource dependencies
  Arrow direction = data flow (write → read)
```

Resources in the graph come in two kinds:

|  | **Transient** | **Imported** |
|--|--------------|-------------|
| **Lifetime** | Single frame | Across frames |
| **Declared as** | Description (size, format) | Existing GPU handle |
| **GPU memory** | Allocated at compile time — **virtual** until then | Already allocated externally |
| **Aliasable** | Yes — non-overlapping lifetimes share physical memory | No — lifetime extends beyond the frame |
| **Examples** | GBuffer MRTs, SSAO scratch, bloom scratch | Backbuffer, TAA history, shadow atlas, blue noise LUT |

This split is what makes aliasing possible. Transient resources are just descriptions until the compiler maps them to real memory — so two that never overlap can land on the same allocation. Imported resources are already owned by something else; the graph tracks their barriers but leaves their memory alone.

```
┌─────────┐    ┌───────────────────┐    ┌─────────┐
│ DECLARE │ →  │     COMPILE       │ →  │ EXECUTE │
│ passes  │    │ • schedule order  │    │ record  │
│ & deps  │    │ • compute aliases │    │ cmds    │
└─────────┘    │ • insert barriers │    └─────────┘
               └───────────────────┘
```

The compile step does three things:

```
  ┌──────────────────────────────────────────────────────────────┐
  │                       COMPILE                               │
  │                                                              │
  │  ┌─────────┐      ┌──────────┐      ┌─────────────┐         │
  │  │SCHEDULE │ ───▶ │ ALLOCATE │ ───▶ │ SYNCHRONIZE │         │
  │  └────┬────┘      └────┬─────┘      └──────┬──────┘         │
  │       │                │                    │                │
  │  topo-sort DAG    map virtual          insert barriers      │
  │  (Kahn's alg)     resources to         between passes       │
  │  detect cycles     physical memory      that hand off        │
  │  → pass order      alias overlaps       resources            │
  │                    → memory map         → barrier list       │
  └──────────────────────────────────────────────────────────────┘
                    All three: O(V + E)
```

- **Schedule** — topological sort (Kahn's algorithm). If a cycle exists, the compiler catches it here.
- **Allocate** — two transient resources with non-overlapping lifetimes share the same physical block.
- **Synchronize** — the compiler knows who wrote what and who reads it next — minimal barriers, no over-sync.

All three are linear-time. For a typical 25-pass frame the entire compile takes microseconds. Exact cost breakdown in [A Real Frame](#a-real-frame).

> The renderer doesn't *run* passes — it *submits a plan*. The graph compiler sees every resource lifetime in the frame at once, so it can pack transient resources into the minimum memory footprint, place every barrier automatically, and cull passes whose outputs nobody reads. This is the inversion of control that makes everything else possible.

**How often does it rebuild?** Three strategies, each a valid tradeoff:

| Strategy | How it works | CPU cost | Flexibility |
|----------|-------------|----------|-------------|
| **Dynamic** | Rebuild the graph every frame | Microseconds | Full — passes appear and disappear freely |
| **Hybrid** | Cache the compiled result, invalidate when passes change | Near-zero on cache hit | Full, with small bookkeeping overhead |
| **Static** | Compile once at init, replay every frame | Zero | None — fixed pipeline only |

Most engines use **dynamic** (Frostbite) or **hybrid** (UE5). The compile is so cheap that caching buys little — but some engines do it anyway to skip redundant barrier recalculation. A fully static graph only makes sense if your pass structure truly never changes, which is rare in practice.

---

## The Payoff

| Concern | Without Graph | With Graph |
|---------|--------------|------------|
| **Memory aliasing** | Opt-in, fragile, rarely done | Automatic — compiler sees all lifetimes. **30–50% VRAM saved.** |
| **Transient resource lifetime** | Manual create/destroy, leaked or over-retained | Scoped to first..last use. Zero waste. |
| Barrier placement | Manual, per-pass | Automatic from declared read/write |
| Pass reordering | Breaks silently | Safe — compiler respects dependencies |
| Pass culling | Manual ifdef / flag checks | Automatic — unused outputs = dead pass |
| Async compute | Manual queue sync | Compiler schedules across queues |


<!-- TODO: SVG diagrams — "Without graph" (tangled barrier spaghetti) vs "With graph" (clean DAG) -->

This isn't theoretical. Frostbite reported 50% VRAM reduction from aliasing at GDC 2017. UE5's RDG ships the same optimization today — every `FRDGTexture` marked as transient goes through the same aliasing pipeline we're about to build. The MVP we build next will give you automatic lifetimes and aliasing by Section 8, plus automatic barriers by Section 7. After that, we map everything to UE5's RDG.

---

## API Design

We start from the API you *want* to write — a minimal `FrameGraph` setup that declares a depth prepass, GBuffer pass, and lighting pass in ~20 lines of C++.

Key design choices visible in the API:

- Passes are lambdas — but *two* lambdas, and the split matters:
  - **Setup** runs at declaration time (CPU-side, building the graph). Declares "I read texture A, I write texture B." No GPU work.
  - **Execute** runs later, during the execution phase. Records actual GPU commands. By this point, barriers and memory are resolved.
- Resources are requested by description (`{1920, 1080, RGBA8}`), not by GPU handle. Virtual until the compiler maps them.
- The graph owns transient lifetimes — the user never calls create/destroy.

```
  Declaration time                Compile              Execution time
  ────────────────                ───────              ──────────────

  addPass(setup, execute)                              execute lambda
        │                                               runs here
        ├── setup lambda runs               ┌────────▶ record draw/dispatch
        │   • declare reads       graph       │          actual GPU cmds
        │   • declare writes      compiler    │
        │   • request resources  ────────┤
        │                        • sort       │
        └── no GPU work here     • alias      └── barriers already
                                 • barrier       inserted by compiler
```

If you've seen UE5 code, this should look familiar:

| Our API | UE5 Equivalent |
|---------|----------------|
| `addPass(setup, execute)` | `FRDGBuilder::AddPass` |
| `ResourceHandle` | `FRDGTextureRef` / `FRDGBufferRef` |
| setup lambda | `BEGIN_SHADER_PARAMETER_STRUCT` macro |
| execute lambda | Execute lambda (same concept) |

The macro approach has a cost: it's opaque, hard to debug, and impossible to compose dynamically. If you want to declare resources conditionally at runtime, you fight the macro system. Our explicit two-lambda API is simpler and more flexible — UE5 traded that flexibility for compile-time validation and reflection.

This is the API we're building toward. The next three sections construct the internals, version by version. Here's a preview of how the final API reads:

```cpp
FrameGraph fg;
auto depth = fg.createResource({1920, 1080, Format::D32F});
auto gbufA = fg.createResource({1920, 1080, Format::RGBA8});
auto gbufN = fg.createResource({1920, 1080, Format::RGBA8});
auto hdr   = fg.createResource({1920, 1080, Format::RGBA16F});

fg.addPass("DepthPrepass",
    [&]() { fg.write(0, depth); },
    [&](/*cmd*/) { /* draw scene depth-only */ });

fg.addPass("GBuffer",
    [&]() { fg.read(1, depth); fg.write(1, gbufA); fg.write(1, gbufN); },
    [&](/*cmd*/) { /* draw scene to GBuffer MRTs */ });

fg.addPass("Lighting",
    [&]() { fg.read(2, gbufA); fg.read(2, gbufN); fg.write(2, hdr); },
    [&](/*cmd*/) { /* fullscreen lighting pass */ });

fg.execute();  // → topo-sort, cull, alias, barrier, run
```

Three passes, declared as lambdas. The graph handles the rest — ordering, barriers, memory. We build this step by step below.

---

# Part II — Build It

*Three iterations from blank file to working frame graph with automatic barriers and memory aliasing. Each version builds on the last — by the end you'll have something you can drop into a real renderer.*

---

## MVP v1 — Declare & Execute

**Data structures:**

```
  ┌────────────────────────────────────────────────────────┐
  │  FrameGraph  (UE5: FRDGBuilder)                           │
  │                                                          │
  │  passes[]  ───▶ ┌────────────────────────────────┐   │
  │                 │ RenderPass (UE5: FRDGPass)    │   │
  │                 │  • name                        │   │
  │                 │  • setup()    ← build the DAG   │   │
  │                 │  • execute()  ← record GPU cmds │   │
  │                 └────────────────────────────────┘   │
  │                                                          │
  │  resources[] ─▶ ┌────────────────────────────────┐   │
  │                 │ ResourceDesc (UE5: FRDGTextureDesc)  │   │
  │                 │  • width, height, format       │   │
  │                 │  • virtual — no GPU handle yet │   │
  │                 └────────────────────────────────┘   │
  │                                                          │
  │  ResourceHandle  = index into resources[]                 │
  │                    (UE5: FRDGTextureRef / FRDGBufferRef)  │
  │                                                          │
  │  ← linear allocator: all frame-scoped, free at frame end  │
  └────────────────────────────────────────────────────────┘
```

**Flow:** Declare passes in order → execute in order. No dependency tracking yet. Resources are created eagerly.

**Buildable C++ — the full MVP v1:**

```cpp
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// ── Resource description (virtual until compile) ──────────────
enum class Format { RGBA8, RGBA16F, R8, D32F };

struct ResourceDesc {
    uint32_t width  = 0;
    uint32_t height = 0;
    Format   format = Format::RGBA8;
};

// Handle = typed index into the graph's resource array.
// No GPU memory behind it yet — just a number.
struct ResourceHandle {
    uint32_t index = UINT32_MAX;
    bool isValid() const { return index != UINT32_MAX; }
};

// ── Render pass ───────────────────────────────────────────────
struct RenderPass {
    std::string                        name;
    std::function<void()>              setup;    // build the DAG (v1: unused)
    std::function<void(/*cmd list*/)>  execute;  // record GPU commands
};

// ── Frame graph ───────────────────────────────────────────────
class FrameGraph {
public:
    // Create a virtual resource — returns a handle, not GPU memory.
    ResourceHandle createResource(const ResourceDesc& desc) {
        resources_.push_back(desc);
        return { static_cast<uint32_t>(resources_.size() - 1) };
    }

    // Register a pass. Setup runs now; execute is stored for later.
    template <typename SetupFn, typename ExecFn>
    void addPass(const std::string& name, SetupFn&& setup, ExecFn&& exec) {
        passes_.push_back({ name, std::forward<SetupFn>(setup),
                                   std::forward<ExecFn>(exec) });
        passes_.back().setup();  // run setup immediately
    }

    // Compile + execute. v1 is trivial — just run in declaration order.
    void execute() {
        // In v1 we skip compile entirely — no sorting, no culling,
        // no barriers. Just run every pass in the order it was added.
        for (auto& pass : passes_) {
            // Here you'd bind resources, begin render pass, etc.
            pass.execute(/* &cmdList */);
        }

        // Frame over — clear everything for next frame.
        passes_.clear();
        resources_.clear();
    }

private:
    std::vector<RenderPass>    passes_;
    std::vector<ResourceDesc>  resources_;
};
```

**Usage** — three passes wired together in ~20 lines:

```cpp
FrameGraph fg;

auto depth  = fg.createResource({1920, 1080, Format::D32F});
auto gbufA  = fg.createResource({1920, 1080, Format::RGBA8});
auto gbufN  = fg.createResource({1920, 1080, Format::RGBA8});
auto hdr    = fg.createResource({1920, 1080, Format::RGBA16F});

fg.addPass("DepthPrepass",
    [&]()          { /* setup: declare writes depth */ },
    [&](/*cmd*/)   { /* execute: draw scene depth-only */ });

fg.addPass("GBuffer",
    [&]()          { /* setup: reads depth, writes gbufA, gbufN */ },
    [&](/*cmd*/)   { /* execute: draw scene to GBuffer MRTs */ });

fg.addPass("Lighting",
    [&]()          { /* setup: reads gbufA, gbufN, writes hdr */ },
    [&](/*cmd*/)   { /* execute: fullscreen lighting pass */ });

fg.execute();  // runs all three in order — done
```

This compiles and runs. It doesn't *do* anything on the GPU yet — the execute lambdas are stubs — but the scaffolding is real. Every piece we add in v2 and v3 goes into this same `FrameGraph` class.

**What it proves:** The lambda-based pass declaration pattern works. You can already compose passes without manual barrier calls (even though barriers are no-ops here).

**What it lacks:** This version executes passes in declaration order and creates every resource upfront. It's correct but wasteful. Version 2 adds the graph.

---

## MVP v2 — Dependencies & Barriers

**Resource versioning:** A resource can be written by pass A, read by pass B, then written *again* by pass C. To keep edges correct, each write creates a new **version** of the resource. Pass B's read depends on version 1 (A's write), not version 2 (C's write). Without versioning, the dependency graph would be ambiguous — this is the "rename on write" pattern.

```
  Pass A          Pass B          Pass C
  writes          reads           writes
  GBuffer v1      GBuffer v1      GBuffer v2
     │               ▲               │
     └───────────────┘               │
                                    │
                    Pass D reads ───┘
                    GBuffer v2

  B depends on A (v1), D depends on C (v2)
  B does NOT depend on C — versioning keeps them separate
```

**Resource tracking:** Each resource version tracks who wrote it and who reads it. On write, create a new version and record the pass. On read, record the pass and add a dependency edge from the writer. In practice, most resources have 1 writer and 1–3 readers.

**Dependency graph:** Stored as an adjacency list — for each pass, a list of passes that must come after it. For 25 passes you'll typically have 30–50 edges.

**Topological sort (Kahn's algorithm):**

```
  Step 1: count in-degrees         Step 2: BFS from zero-degree nodes

  Depth[0]  GBuf[1]  SSAO[1]      Queue: [Depth]
    │        ▲  │       ▲            pop Depth → output, decrement GBuf
    └────────┘  └───────┘          Queue: [GBuf]
                                     pop GBuf → output, decrement SSAO
  Lighting[2]  Tonemap[1]           Queue: [SSAO]
     ▲  ▲        ▲                   pop SSAO → output, decrement Lighting
     │  │        │                  Queue: [Lighting]
     │  └────────┘                  ...

  Output: Depth → GBuf → SSAO → Lighting → Tonemap
  If output.size < input.size → cycle detected!
```

Runs in O(V + E). Kahn's is preferred over DFS-based topo-sort because cycle detection falls out naturally.

**Pass culling:** Walk backwards from the final output (present/backbuffer). Mark every reachable pass. Any unmarked pass is dead — remove it and release its resource declarations. This is ~10 lines but immediately useful: disable SSAO by not reading its output, and the pass (and all its resources) vanishes automatically. Complexity: O(V + E).

```
  Before culling:                    After culling:

  ┌───────┐   ┌──────┐   ┌─────────┐   ┌───────┐   ┌─────────┐
  │ Depth │─▶│ GBuf │─▶│Lighting│   │ Depth │─▶│Lighting│
  └───────┘   └──┬───┘   └─────────┘   └───────┘   └─────────┘
              │
         ┌────┴──┐                    SSAO removed —
         │ SSAO  │  ← output unread     its resources freed
         └───────┘
```

**Barrier insertion:** Walk the sorted order. For each pass, check each resource against a state table tracking its current pipeline stage, access flags, and image layout. If usage changed, emit a barrier.

Here's what that looks like in practice — every one of these is a barrier your graph inserts automatically, but you'd have to place by hand without one:

```
  ┌────────────────────────────────────────────────────────────────────┐
  │  Barrier zoo — the transitions a real frame actually needs        │
  ├────────────────────────────────────────────────────────────────────┤
  │                                                                    │
  │  1. Render target → Shader read (most common)                     │
  │     GBuffer pass writes albedo → Lighting pass samples it          │
  │     Vulkan:  COLOR_ATTACHMENT_OUTPUT → FRAGMENT_SHADER             │
  │     D3D12:   RENDER_TARGET → PIXEL_SHADER_RESOURCE                │
  │                                                                    │
  │  2. Depth write → Depth read (shadow sampling)                    │
  │     Shadow pass writes depth → Lighting reads as texture           │
  │     Vulkan:  LATE_FRAGMENT_TESTS → FRAGMENT_SHADER                │
  │     D3D12:   DEPTH_WRITE → PIXEL_SHADER_RESOURCE                  │
  │     Layout:  DEPTH_ATTACHMENT → SHADER_READ_ONLY                  │
  │                                                                    │
  │  3. Compute UAV write → Compute UAV read (ping-pong)              │
  │     Bloom downsample writes mip N → reads mip N to write mip N+1  │
  │     Vulkan:  COMPUTE_SHADER (WRITE) → COMPUTE_SHADER (READ)       │
  │     D3D12:   UAV barrier (same resource, same state — still       │
  │              needed to flush compute caches!)                      │
  │                                                                    │
  │  4. Shader read → Render target (reuse after sampling)            │
  │     Lighting sampled HDR buffer → Tonemap now writes to it         │
  │     Vulkan:  FRAGMENT_SHADER → COLOR_ATTACHMENT_OUTPUT             │
  │     Layout:  SHADER_READ_ONLY → COLOR_ATTACHMENT_OPTIMAL          │
  │                                                                    │
  │  5. Render target → Present (every frame, easy to forget)         │
  │     Final composite → swapchain present                            │
  │     Vulkan:  COLOR_ATTACHMENT_OUTPUT → BOTTOM_OF_PIPE             │
  │     D3D12:   RENDER_TARGET → PRESENT                              │
  │                                                                    │
  │  6. Aliasing barrier (two resources, same memory)                 │
  │     GBuffer dies → HDR lighting reuses same physical block         │
  │     D3D12:   explicit D3D12_RESOURCE_BARRIER_TYPE_ALIASING        │
  │     Vulkan:  handled via image layout UNDEFINED (discard)          │
  │                                                                    │
  └────────────────────────────────────────────────────────────────────┘
```

A 25-pass frame needs 30–50 of these. Miss one: corruption or device lost. Add a redundant one: GPU stall for nothing. Get the source/dest stages wrong: validation errors or subtle frame-order bugs that only show on a different vendor's driver. This is exactly why you don't want to place them by hand — the graph sees every read/write edge and emits the *exact* set.

**Diff from v1 — what changes in the code:**

We need four new pieces: (1) resource versioning with read/write tracking, (2) adjacency list for the DAG, (3) topological sort, (4) pass culling, and (5) barrier insertion. Here's the updated `FrameGraph` class — additions marked with `// NEW v2`:

```cpp
#include <algorithm>
#include <cassert>
#include <queue>
#include <unordered_set>

// ── Resource state tracking (NEW v2) ──────────────────────────
enum class ResourceState { Undefined, ColorAttachment, DepthAttachment,
                           ShaderRead, Present };

struct ResourceVersion {                 // NEW v2
    uint32_t writerPass = UINT32_MAX;    // which pass wrote this version
    std::vector<uint32_t> readerPasses;  // which passes read it
};

// Extend ResourceDesc with tracking:
struct ResourceEntry {
    ResourceDesc desc;
    std::vector<ResourceVersion> versions;  // version 0, 1, 2...
    ResourceState currentState = ResourceState::Undefined;
};

// ── Updated render pass ───────────────────────────────────────
struct RenderPass {
    std::string name;
    std::function<void()>             setup;
    std::function<void(/*cmd list*/)> execute;

    std::vector<ResourceHandle> reads;    // NEW v2
    std::vector<ResourceHandle> writes;   // NEW v2
    std::vector<uint32_t> dependsOn;      // NEW v2 — adjacency list
    uint32_t inDegree = 0;                // NEW v2 — for Kahn's
    bool     alive    = false;            // NEW v2 — for culling
};

// ── Updated FrameGraph ────────────────────────────────────────
class FrameGraph {
public:
    ResourceHandle createResource(const ResourceDesc& desc) {
        entries_.push_back({ desc, {{}}, ResourceState::Undefined });
        return { static_cast<uint32_t>(entries_.size() - 1) };
    }

    // Declare a read — links this pass to the resource's current version.
    void read(uint32_t passIdx, ResourceHandle h) {    // NEW v2
        auto& ver = entries_[h.index].versions.back();
        if (ver.writerPass != UINT32_MAX) {
            passes_[passIdx].dependsOn.push_back(ver.writerPass);
            passes_[ver.writerPass].inDegree; // edge tracked
        }
        ver.readerPasses.push_back(passIdx);
        passes_[passIdx].reads.push_back(h);
    }

    // Declare a write — creates a new version of the resource.
    void write(uint32_t passIdx, ResourceHandle h) {   // NEW v2
        entries_[h.index].versions.push_back({});
        entries_[h.index].versions.back().writerPass = passIdx;
        passes_[passIdx].writes.push_back(h);
    }

    template <typename SetupFn, typename ExecFn>
    void addPass(const std::string& name, SetupFn&& setup, ExecFn&& exec) {
        uint32_t idx = static_cast<uint32_t>(passes_.size());
        passes_.push_back({ name, std::forward<SetupFn>(setup),
                                   std::forward<ExecFn>(exec) });
        currentPass_ = idx;   // NEW v2 — so setup can call read()/write()
        passes_.back().setup();
    }

    void execute() {
        buildEdges();        // NEW v2
        auto sorted = topoSort();        // NEW v2
        cull(sorted);        // NEW v2

        for (uint32_t idx : sorted) {
            if (!passes_[idx].alive) continue;  // skip culled
            insertBarriers(idx);                 // NEW v2
            passes_[idx].execute(/* &cmdList */);
        }
        passes_.clear();
        entries_.clear();
    }

private:
    uint32_t currentPass_ = 0;
    std::vector<RenderPass>    passes_;
    std::vector<ResourceEntry> entries_;

    // ── Build dependency edges ────────────────────────────────
    void buildEdges() {                              // NEW v2
        for (auto& pass : passes_) {
            // deduplicate and count in-degrees
            std::unordered_set<uint32_t> seen;
            for (uint32_t dep : pass.dependsOn) {
                if (seen.insert(dep).second)
                    passes_[dep].inDegree++;
            }
        }
    }

    // ── Kahn's topological sort ───────────────────────────────
    std::vector<uint32_t> topoSort() {               // NEW v2
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
            // visit all passes that depend on 'cur'
            for (uint32_t i = 0; i < passes_.size(); i++) {
                for (uint32_t dep : passes_[i].dependsOn) {
                    if (dep == cur && --inDeg[i] == 0)
                        q.push(i);
                }
            }
        }
        assert(order.size() == passes_.size() && "Cycle detected!");
        return order;
    }

    // ── Cull dead passes (backward walk from output) ──────────
    void cull(const std::vector<uint32_t>& sorted) { // NEW v2
        // Mark the last pass (present) as alive, then walk backward.
        if (sorted.empty()) return;
        passes_[sorted.back()].alive = true;
        for (int i = static_cast<int>(sorted.size()) - 1; i >= 0; i--) {
            if (!passes_[sorted[i]].alive) continue;
            for (uint32_t dep : passes_[sorted[i]].dependsOn)
                passes_[dep].alive = true;
        }
    }

    // ── Insert barriers where resource state changes ──────────
    void insertBarriers(uint32_t passIdx) {          // NEW v2
        auto stateForUsage = [](bool isWrite, Format fmt) {
            if (isWrite)
                return (fmt == Format::D32F) ? ResourceState::DepthAttachment
                                             : ResourceState::ColorAttachment;
            return ResourceState::ShaderRead;
        };

        for (auto& h : passes_[passIdx].reads) {
            ResourceState needed = ResourceState::ShaderRead;
            if (entries_[h.index].currentState != needed) {
                // → emit barrier (old state → ShaderRead)
                entries_[h.index].currentState = needed;
            }
        }
        for (auto& h : passes_[passIdx].writes) {
            ResourceState needed = stateForUsage(true, entries_[h.index].desc.format);
            if (entries_[h.index].currentState != needed) {
                // → emit barrier (old state → write state)
                entries_[h.index].currentState = needed;
            }
        }
    }
};
```

**Usage — same passes, now with declared dependencies:**

```cpp
FrameGraph fg;
auto depth = fg.createResource({1920, 1080, Format::D32F});
auto gbufA = fg.createResource({1920, 1080, Format::RGBA8});
auto hdr   = fg.createResource({1920, 1080, Format::RGBA16F});

fg.addPass("DepthPrepass",
    [&]() { fg.write(0, depth); },
    [&](/*cmd*/) { /* draw depth */ });

fg.addPass("GBuffer",
    [&]() { fg.read(1, depth); fg.write(1, gbufA); },
    [&](/*cmd*/) { /* draw GBuffer */ });

fg.addPass("Lighting",
    [&]() { fg.read(2, gbufA); fg.write(2, hdr); },
    [&](/*cmd*/) { /* fullscreen lighting */ });

fg.execute();  // → sorts, culls dead passes, inserts barriers, runs
```

Remove GBuffer's read of `depth` and the depth prepass gets culled automatically — no `#ifdef`, no flag. That's the graph working for you.

**What it proves:** Automatic barriers from declared dependencies. Pass reordering is safe. Dead passes are culled. Three of the four intro promises delivered.

UE5 does exactly this. When you call `FRDGBuilder::AddPass` with `ERDGPassFlags::Raster` or `ERDGPassFlags::Compute`, RDG builds the same dependency graph from your declared reads/writes, topologically sorts it, culls dead passes, and inserts barriers — all before recording a single GPU command.

One caveat: UE5's migration to RDG is *incomplete*. Large parts of the renderer still use legacy immediate-mode `FRHICommandList` calls outside the graph. These "untracked" resources bypass RDG's barrier and aliasing systems entirely. The result: you get the graph's benefits only for passes that have been ported. Legacy passes still need manual barriers at the boundary where RDG-managed and unmanaged resources meet. This is the cost of retrofitting a graph onto a 25-year-old codebase — and a good argument for designing with a graph from the start.

**What it lacks:** Resources still live for the entire frame. Version 3 adds lifetime analysis and memory aliasing.

---

## MVP v3 — Lifetimes & Aliasing

**First/last use:** Walk the sorted pass list. For each transient resource, record `firstUsePass` and `lastUsePass`. Imported resources are excluded — their lifetimes extend beyond the frame.

**Reference counting:** Increment refcount at first use, decrement at last use. When refcount hits zero, that resource's physical memory is eligible for reuse by a later resource.

**Aliasing algorithm:** Sort transient resources by first-use pass, then scan a free-list for a compatible physical allocation. If one fits, reuse it. If not, allocate fresh.

The free-list is a small array of available memory blocks. The overall algorithm is a **greedy interval-coloring** — assign physical memory slots such that overlapping intervals never share a slot.

```
  Compatibility check for reusing a free-list block:

  Candidate block          New resource
  ┌────────────────┐       ┌────────────────┐
  │ device-local     │  ✓?  │ device-local     │  same memory type?
  │ 16 MB            │  ✓?  │ 8 MB needed      │  block big enough?
  │ avail after P4   │  ✓?  │ first use at P5  │  lifetimes don't overlap?
  └────────────────┘       └────────────────┘
       all three ✓ → reuse!       any ✗ → allocate new block
```

This is the same approach Frostbite described at GDC 2017.

```
  Pass:  1       2       3       4       5       6       7
         │       │       │       │       │       │       │
  GBuf Albedo    ███████████████████████  8 MB  ┐
  GBuf Normals   ███████████████████████  8 MB  │
  SSAO Scratch           ████████████  2 MB  │ Virtual
  SSAO Result                    ███████  2 MB  │
  HDR Lighting                           ███████ 16 MB  │
  Bloom Scratch                                  ███ 16 MB ┘
         │       │       │       │       │       │       │
                          52 MB total

  Physical mapping (after aliasing):
  ┌─────────────────────────────────────────┐
  │ Block A (16 MB): GBuf Albedo → HDR Lighting  │
  │ Block B (16 MB): GBuf Normals → Bloom Scratch │
  │ Block C  (2 MB): SSAO Scratch → SSAO Result   │
  └─────────────────────────────────────────┘
                          36 MB physical — 31% saved
```

**Worked example** (1080p deferred pipeline):

| Virtual Resource | Format | Size | Lifetime (passes) |
|-----------------|--------|------|--------------------|
| GBuffer Albedo | RGBA8 | 8 MB | 2–4 |
| GBuffer Normals | RGB10A2 | 8 MB | 2–4 |
| SSAO Scratch | R8 | 2 MB | 3–4 |
| SSAO Result | R8 | 2 MB | 4–5 |
| HDR Lighting | RGBA16F | 16 MB | 5–6 |
| Bloom Scratch | RGBA16F | 16 MB | 6–7 |

Without aliasing: 52 MB. With aliasing: GBuffer Albedo and HDR Lighting share one 16 MB block (lifetimes don't overlap). GBuffer Normals and Bloom Scratch share another. SSAO Scratch and SSAO Result share a third. **Physical memory: 36 MB — 31% saved.** In more complex pipelines with more transient resources, savings reach 40–50%.

**Diff from v2 — lifetime tracking and free-list allocator:**

Two additions to the `FrameGraph` class: (1) a lifetime scan that records each transient resource's first and last use in the sorted pass order, and (2) a greedy free-list allocator that reuses physical blocks when lifetimes don't overlap.

```cpp
// ── Physical memory block ─────────────────────────────────────
struct PhysicalBlock {
    uint32_t sizeBytes   = 0;
    Format   format      = Format::RGBA8;
    uint32_t availAfter  = 0;  // pass index after which this block is free
};

// ── Lifetime info per resource ────────────────────────────────
struct Lifetime {
    uint32_t firstUse = UINT32_MAX;   // earliest pass that touches it
    uint32_t lastUse  = 0;            // latest pass that touches it
    bool     isTransient = true;      // imported resources excluded
};

// Add these to FrameGraph, called between topoSort() and execute():

// ── Step 1: Scan lifetimes ────────────────────────────────────
std::vector<Lifetime> scanLifetimes(const std::vector<uint32_t>& sorted) {
    std::vector<Lifetime> life(entries_.size());

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
    return life;
}

// ── Step 2: Greedy free-list aliasing ─────────────────────────
// Returns a mapping: virtual resource index → physical block index.
std::vector<uint32_t> aliasResources(const std::vector<Lifetime>& lifetimes) {
    std::vector<PhysicalBlock> freeList;
    std::vector<uint32_t> mapping(entries_.size(), UINT32_MAX);

    // Sort resources by first-use for greedy scan.
    std::vector<uint32_t> indices(entries_.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](uint32_t a, uint32_t b) {
        return lifetimes[a].firstUse < lifetimes[b].firstUse;
    });

    for (uint32_t resIdx : indices) {
        if (!lifetimes[resIdx].isTransient) continue;
        if (lifetimes[resIdx].firstUse == UINT32_MAX) continue; // unused

        uint32_t needed = entries_[resIdx].desc.width
                        * entries_[resIdx].desc.height * 4; // simplified
        bool reused = false;

        // Try to find a compatible free block.
        for (uint32_t b = 0; b < freeList.size(); b++) {
            if (freeList[b].availAfter < lifetimes[resIdx].firstUse
                && freeList[b].sizeBytes >= needed) {
                // Reuse this block!
                mapping[resIdx] = b;
                freeList[b].availAfter = lifetimes[resIdx].lastUse;
                reused = true;
                break;
            }
        }

        if (!reused) {
            // Allocate a new physical block.
            mapping[resIdx] = static_cast<uint32_t>(freeList.size());
            freeList.push_back({ needed, entries_[resIdx].desc.format,
                                 lifetimes[resIdx].lastUse });
        }
    }
    return mapping;  // use this to bind physical memory before execute
}
```

**Updated `execute()` — now with lifetime analysis:**

```cpp
void execute() {
    buildEdges();
    auto sorted   = topoSort();
    cull(sorted);
    auto lifetimes = scanLifetimes(sorted);   // NEW v3
    auto mapping   = aliasResources(lifetimes); // NEW v3

    // At this point, 'mapping' tells you which physical block
    // each virtual resource should use. Bind them here.

    for (uint32_t idx : sorted) {
        if (!passes_[idx].alive) continue;
        insertBarriers(idx);
        passes_[idx].execute(/* &cmdList */);
    }
    passes_.clear();
    entries_.clear();
}
```

That's ~70 new lines on top of v2. The aliasing runs once per frame in O(R log R) — sort the resources, then a linear scan of the free list. For 15 transient resources this is sub-microsecond.

**What it proves:** The full value prop — automatic memory aliasing *and* automatic barriers. The MVP is now feature-equivalent to Frostbite's 2017 GDC demo (minus async compute).

In UE5, this is handled by the transient resource allocator. Any `FRDGTexture` created through `FRDGBuilder::CreateTexture` (as opposed to `RegisterExternalTexture`) is transient and eligible for aliasing. The RDG compiler runs the same lifetime analysis and free-list scan we just built.

A limitation worth noting: UE5 only aliases *transient* resources. Imported resources — even when their lifetimes are fully known within the frame — are never aliased. Frostbite's original design was more aggressive here, aliasing across a broader set of resources by tracking GPU-timeline lifetimes rather than just graph-declared lifetimes. If your renderer has large imported resources with predictable per-frame usage patterns, UE5's approach leaves VRAM on the table.

---

## A Real Frame

**Deferred Pipeline**

Depth prepass → GBuffer → SSAO → Lighting → Tonemap → Present

```
  ┌───────┐    ┌──────┐    ┌──────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐
  │ Depth │───▶│ GBuf │───▶│ SSAO │───▶│Lighting│───▶│ Tonemap │───▶│ Present │
  └───────┘    └──────┘    └──────┘    └─────────┘    └─────────┘    └─────────┘
       │         │ │        │ │           │              │
       │         │ │        │ │           │              │
    depth     GBuf  GBuf   SSAO  SSAO      HDR          backbuffer
    (T)      albedo norm  scratch result  lighting       (imported)
             (T)    (T)    (T)    (T)      (T)

  (T) = transient — aliased by graph     (imported) = owned externally
```

Two kinds of resources in play:
- **Transient:** GBuffer MRTs, SSAO scratch, HDR lighting buffer, bloom scratch — created and destroyed within this frame. Aliased by the graph.
- **Imported:** Backbuffer (acquired from swapchain, presented at end), TAA history (read from last frame, written this frame for next frame), shadow atlas (persistent, updated incrementally). The graph tracks their barriers but doesn't own their memory.

<!-- TODO: Full addPass chain with resource declarations + compiled execution order + auto-inserted barriers + memory aliasing in action -->

**Forward Pipeline**

```
  ┌───────┐    ┌─────────┐    ┌─────────┐    ┌──────────┐    ┌─────────┐
  │ Depth │───▶│ Forward │───▶│ Resolve │───▶│PostProc │───▶│ Present │
  └───────┘    │ + MSAA  │    └─────────┘    └──────────┘    └─────────┘
               └─────────┘
    depth(T)    color MSAA(T)  color(T)       HDR(T)          backbuffer
                                                              (imported)

  Fewer passes, fewer transient resources → less aliasing opportunity
  Same API, same automatic barriers
```

**Side-by-side**

| Aspect | Deferred | Forward |
|--------|----------|---------|
| Passes | 6 | 5 |
| **Peak VRAM (no aliasing)** | X MB | Y MB |
| **Peak VRAM (with aliasing)** | 0.6X MB | 0.75Y MB |
| **VRAM saved by aliasing** | **40%** | **25%** |
| Barriers auto-inserted | 8 | 5 |

**What about CPU cost?** Every phase is linear-time:

| Phase | Complexity | Notes |
|-------|-----------|-------|
| Topological sort | O(V + E) | Kahn's algorithm — passes + edges |
| Pass culling | O(V + E) | Backward reachability from output |
| Lifetime scan | O(V) | Single pass over sorted list |
| Aliasing | O(R log R) | Sort by first-use, then O(R) free-list scan |
| Barrier insertion | O(V) | Linear scan with state lookup |

Where V = passes (~25), E = dependency edges (~50), R = transient resources (~15). Everything is linear or near-linear in the graph size. All data fits in L1 cache, so the constant factors are tiny — the entire compile is well under 0.1 ms even on a cold rebuild.

> The graph doesn't care about your rendering *strategy*. It cares about your *dependencies*. That's the whole point.

---

# Part III — Production Engines

*How UE5, Frostbite, and Unity implement the same ideas at scale — what they added, what they compromised, and where they still differ.*

---

## Production Engines

### UE5's Rendering Dependency Graph (RDG)

UE5's RDG is the frame graph you're most likely to work with. It was retrofitted onto a 25-year-old renderer, so every design choice reflects a tension: do this properly *and* don't break the 10,000 existing draw calls.

```
  UE5 RDG pipeline (same declare → compile → execute):

  Game thread                                            Render thread
  ──────────────────────────────                         ──────────────
  FRDGBuilder::AddPass(...)  ─┐
  FRDGBuilder::AddPass(...)   │  accumulates              FRDGBuilder
  FRDGBuilder::AddPass(...)   ├─ full DAG ──────────────▶ ::Execute()
  CreateTexture(...)          │  (passes, resources,       ├─ compile
  RegisterExternalTexture(...)┘   edges)                   ├─ allocate
                                                           ├─ barriers
                                                           └─ record cmds
```

**Pass declaration.** Each `AddPass` takes a parameter struct + execute lambda. The struct *is* the setup phase:

```
  BEGIN_SHADER_PARAMETER_STRUCT(...)          ┌──────────────┐
    SHADER_PARAMETER_RDG_TEXTURE(Input)  ──▶ │ read edge    │
    RENDER_TARGET_BINDING_SLOT(Output)   ──▶ │ write edge   │──▶ DAG
  END_SHADER_PARAMETER_STRUCT()              └──────────────┘

  Macro generates metadata → RDG extracts dependency edges
  No separate setup lambda needed
```

**Pass flags & resource types:**

| Pass flags | Meaning |
|-----------|---------|
| `ERDGPassFlags::Raster` | Graphics queue, expects render targets |
| `ERDGPassFlags::Compute` | Graphics queue, compute dispatch |
| `ERDGPassFlags::AsyncCompute` | Async compute queue |
| `ERDGPassFlags::NeverCull` | Exempt from dead-pass culling |

| Resource type | Covers |
|--------------|--------|
| `FRDGTexture` / `FRDGTextureRef` | Render targets, SRVs, UAVs |
| `FRDGBuffer` / `FRDGBufferRef` | Structured, vertex/index, indirect args |

Both go through the same aliasing and barrier system.

**Key systems — how they map to our MVP:**

```
  ┌─────────────────────────────────────────────────────────┐
  │ Feature            │ Our MVP         │ UE5 RDG          │
  ├────────────────────┼─────────────────┼──────────────────┤
  │ Transient alloc    │ free-list scan  │ pooled allocator  │
  │                    │ per frame       │ amortized across  │
  │                    │                 │ frames            │
  ├────────────────────┼─────────────────┼──────────────────┤
  │ Barriers           │ one-at-a-time   │ batched + split   │
  │                    │                 │ begin/end          │
  ├────────────────────┼─────────────────┼──────────────────┤
  │ Pass culling       │ backward walk   │ refcount-based    │
  │                    │ from output     │ + skip allocation │
  ├────────────────────┼─────────────────┼──────────────────┤
  │ Cmd recording      │ single thread   │ parallel FRHICmdList│
  ├────────────────────┼─────────────────┼──────────────────┤
  │ Rebuild            │ dynamic         │ hybrid (cached)   │
  └────────────────────┴─────────────────┴──────────────────┘
```

**Debugging.** `RDG Insights` in the Unreal editor visualizes the full pass graph, resource lifetimes, and barrier placement. The frame is data — export it, diff it, analyze offline.

**What RDG gets wrong (or leaves on the table):**

```
  ┌──────────────────────────────────────────────────────┐
  │                   RDG Limitations                     │
  ├──────────────────────────────────────────────────────┤
  │                                                      │
  │  ✗  Incomplete migration                             │
  │     Legacy FRHICommandList ←→ RDG boundary           │
  │     = manual barriers at the seam                    │
  │                                                      │
  │  ✗  Macro-heavy API                                  │
  │     BEGIN_SHADER_PARAMETER_STRUCT → opaque, no       │
  │     debugger stepping, fights dynamic composition    │
  │                                                      │
  │  ✗  Transient-only aliasing                          │
  │     Imported resources never aliased, even when      │
  │     lifetime is fully known within the frame         │
  │                                                      │
  │  ✗  No automatic subpass merging                     │
  │     Delegated to RHI — graph can't optimize tiles    │
  │                                                      │
  │  ✗  Async compute is opt-in                          │
  │     Manual ERDGPassFlags::AsyncCompute tagging       │
  │     Compiler trusts, doesn't discover                │
  │                                                      │
  └──────────────────────────────────────────────────────┘
```

### Where Frostbite started

Frostbite's frame graph (O'Donnell & Barczak, GDC 2017: *"FrameGraph: Extensible Rendering Architecture in Frostbite"*) is where the modern render graph concept originates.

```
  Frostbite innovations that shaped every later engine:

  ╔═══════════════════════╦═══════════════════════════════════╗
  ║ Transient resources   ║ First production aliasing.        ║
  ║                       ║ 50% VRAM saved on Battlefield 1.  ║
  ╠═══════════════════════╬═══════════════════════════════════╣
  ║ Split barriers        ║ begin/end placement for GPU       ║
  ║                       ║ overlap. UE5 adopted this.        ║
  ╠═══════════════════════╬═══════════════════════════════════╣
  ║ Graph export          ║ DOT-format debug. Every engine    ║
  ║                       ║ since has built equivalent.       ║
  ╠═══════════════════════╬═══════════════════════════════════╣
  ║ Dynamic rebuild       ║ Full rebuild every frame.         ║
  ║                       ║ "Compile cost so low, caching     ║
  ║                       ║  adds complexity for nothing."    ║
  ╚═══════════════════════╩═══════════════════════════════════╝
```

**Frostbite vs UE5 — design spectrum:**

```
  More aggressive                              More conservative
  ◄──────────────────────────────────────────────────────────────►

  Frostbite                                    UE5 RDG
  ┌──────────────────────┐                    ┌──────────────────┐
  │ ✓ fully dynamic      │                    │ ✓ hybrid/cached  │
  │ ✓ alias everything   │                    │ ✓ transient only │
  │ ✓ subpass merging    │                    │ ✗ RHI-delegated  │
  │ ✓ auto async        │                    │ ✗ opt-in async   │
  │ ✗ no legacy support │                    │ ✓ legacy compat  │
  │ ✗ closed engine     │                    │ ✓ 3P game code   │
  └──────────────────────┘                    └──────────────────┘

  Frostbite controls the full engine.
  UE5 must support 25 years of existing code.
```

### Other implementations

**Unity (SRP Render Graph)** — shipped as part of the Scriptable Render Pipeline. Handles pass culling and transient resource aliasing in URP/HDRP backends. Async compute support varies by platform. Designed for portability across mobile and desktop, so it avoids some of the more aggressive GPU-specific optimizations.

### Comparison

| Feature | UE5 RDG | Frostbite | Unity SRP |
|---------|---------|-----------|----------|
| Rebuild strategy | hybrid (cached) | dynamic | dynamic |
| Pass culling | ✓ auto | ✓ refcount | ✓ auto |
| Memory aliasing | ✓ transient | ✓ full | ✓ transient |
| Async compute | ✓ flag-based | ✓ | varies |
| Split barriers | ✓ | ✓ | ✗ |
| Parallel recording | ✓ | ✓ | limited |
| Buffer tracking | ✓ | ✓ | ✓ |

---

## Upgrade Roadmap

You've built the MVP. Here's what to add, in what order, and why.

### 1. Memory aliasing
**Priority: HIGH · Difficulty: Medium**

Biggest bang-for-buck. Reduces VRAM usage 30–50% for transient resources. The core idea is **interval-graph coloring** — assign physical memory to virtual resources such that no two overlapping lifetimes share an allocation.

**How the algorithm works:**

After topological sort gives you a linear pass order, walk the pass list and record each transient resource's **first use** (birth) and **last use** (death). This gives you a set of intervals — one per resource. Now you need to pack those intervals into the fewest physical allocations, where no two intervals sharing an allocation overlap.

```
  Step-by-step free-list walkthrough:

  Sorted resources:     GBuf (16MB, pass 2-4)  SSAO (2MB, pass 3-5)  HDR (16MB, pass 5-7)

  Process GBuf:         free list: [empty]
                        → no match → allocate Block A (16MB)
                        free list: [A: 16MB, avail after pass 4]

  Process SSAO:         free list: [A: 16MB, avail after pass 4]
                        → A available after 4, but SSAO starts at 3 → overlap!
                        → no match → allocate Block B (2MB)
                        free list: [A: avail after 4] [B: avail after 5]

  Process HDR:          free list: [A: avail after 4] [B: avail after 5]
                        → A: 16MB, avail after 4, HDR starts at 5 → ✓ fits!
                        → reuse Block A
                        free list: [A: avail after 7] [B: avail after 5]

  Result: 3 virtual resources → 2 physical blocks (34MB → 18MB)
```

The greedy first-fit approach is provably optimal for interval graphs. For more aggressive packing, see **linear-scan register allocation** from compiler literature — same problem, different domain. Round up to power-of-two buckets to reduce fragmentation (UE5 does this).

**What to watch out for:**

```
  ⚠ Aliasing pitfalls:

  ┌─────────────────┐     ┌──────────────────────────────────────┐
  │ Format compat   │ ──▶ │ depth/stencil metadata may conflict  │
  │                 │     │ with color targets on same VkMemory  │
  │                 │     │ → skip aliasing for depth formats    │
  ├─────────────────┤     ├──────────────────────────────────────┤
  │ Initialization  │ ──▶ │ reused memory = garbage contents     │
  │                 │     │ → first use MUST be a full clear     │
  │                 │     │   or fullscreen write                │
  ├─────────────────┤     ├──────────────────────────────────────┤
  │ Imported res    │ ──▶ │ survive across frames — never alias  │
  │                 │     │ only transient resources qualify     │
  └─────────────────┘     └──────────────────────────────────────┘
```

UE5's transient allocator does exactly this. Add immediately after the MVP works.

### 2. Pass merging / subpass folding
**Priority: HIGH on mobile · Difficulty: Medium**

Critical for tile-based GPUs (Mali, Adreno, Apple). Merge compatible passes into Vulkan subpasses or Metal render pass load/store actions.

```
  Without merging (tile-based GPU):        With merging:

  Pass A (GBuffer)                         Pass A+B (merged subpass)
    ├─ render to tile                        ├─ render A to tile
    ├─ flush tile → main memory  ✗ slow      ├─ B reads tile directly
    └─ done                                  │   (subpass input — free!)
                                             └─ flush once → main memory
  Pass B (Lighting)
    ├─ load from main memory     ✗ slow
    ├─ render to tile                      Saves: 1 flush + 1 load
    ├─ flush tile → main memory            per merged pair
    └─ done                                = massive bandwidth savings
                                           on mobile
```

**How the algorithm works:**

The algorithm walks the sorted pass list and identifies **merge candidates** — adjacent passes that pass a checklist:

```
  Can Pass A and Pass B merge?

  Same RT dimensions?  ─── no ──▶ SKIP (tile sizes differ)
        │ yes
        ▼
  Compatible attachments? ── no ──▶ SKIP (B samples A at arbitrary UVs)
        │ yes (B reads A's output
        │      at current pixel only)
        ▼
  No external deps?  ──── no ──▶ SKIP (buffer dep leaves render pass)
        │ yes
        ▼
  ┌─────────────┐
  │ MERGE A + B │  → union-find groups them
  │ → 1 subpass │  → one VkRenderPass with N subpasses
  └─────────────┘
```

**Translating to API calls:**

| API | Merged group becomes | Intermediate attachments |
|-----|---------------------|------------------------|
| **Vulkan** | Single `VkRenderPass` + N `VkSubpassDescription` | Subpass inputs (tile-local read) |
| **Metal** | One `MTLRenderPassDescriptor`, `storeAction = .dontCare` for intermediates | `loadAction = .load` |
| **D3D12** | `BeginRenderPass`/`EndRenderPass` (Tier 1/2) | No direct subpass — similar via render pass tiers |

**What to watch out for:**

- **Depth-stencil reuse** — be careful when merging passes that both write depth. Only one depth attachment per subpass group.
- **Order matters** — the union-find should only merge passes that are *already adjacent* in the topological order. Merging non-adjacent passes would reorder execution.
- **Profiling** — on desktop GPUs (NVIDIA, AMD), subpass merging has minimal impact because they don't use tile-based rendering. Don't add this complexity unless you ship on mobile or Switch.

UE5 doesn't do this automatically in RDG — subpass merging is handled at a lower level in the RHI — but Frostbite's original design included it. Add if targeting mobile or console (Switch).

### 3. Async compute
**Priority: MEDIUM · Difficulty: High**

Requires multi-queue infrastructure (compute queue + graphics queue). The graph compiler must find independent subgraphs that can execute concurrently — passes with **no path between them** in the DAG.

```
  Graphics queue:  ┌───────┐  ┌─────────┐  ┌─────────┐
                   │ Depth │  │ Shadows │  │Lighting│
                   └───────┘  └─────────┘  └────┬────┘
                                               │
                                          fence │ sync
                                               │
  Compute queue:        ┌──────┐              │
                        │ SSAO │ ───────────┘
                        └──────┘
                        ▲
                  no path between
                 Shadows and SSAO → can overlap
```

**How the reachability analysis works:**

```
  Reachability bitset example (5 passes):

  DAG:  Depth → GBuf → Lighting → Tonemap
                  ↘ SSAO ↗

  Pass         Bitset (who can I reach?)
  ──────────   ──────────────────────────
  Depth        [0, 1, 1, 1, 1]  → reaches everything
  GBuf         [0, 0, 1, 1, 1]  → reaches SSAO, Light, Tone
  SSAO         [0, 0, 0, 1, 1]  → reaches Light, Tone
  Lighting     [0, 0, 0, 0, 1]  → reaches Tonemap
  Tonemap      [0, 0, 0, 0, 0]  → leaf

  Can Shadows overlap SSAO?
  reachable[Shadows].test(SSAO) = false
  reachable[SSAO].test(Shadows) = false
  → independent! → can run on different queues
```

**Steps:**

1. **Build reachability** — walk in reverse topological order. Each pass's bitset = union of successors' bitsets + successors themselves.
2. **Query independence** — two passes overlap iff neither can reach the other. One AND + one compare per query.
3. **Partition** — greedily assign compute-eligible passes to the compute queue whenever they're independent from the current graphics tail.

**Fence placement:**

Wherever a dependency edge crosses queue boundaries, you need a GPU fence (semaphore signal + wait). Walk the DAG edges: if source pass is on queue A and dest pass is on queue B, insert a fence. Minimize fence count by transitively reducing: if pass C already waits on a fence that pass B signaled, and pass B is after pass A on the same queue, pass C doesn't need a separate fence from pass A.

```
  Without transitive reduction:    With transitive reduction:
  
  Graphics: [A] ──fence──→ [C]    Graphics: [A] ──────────→ [C]
              │                                              ▲
              └──fence──→ [D]                                │
                                   Compute:  [B] ──fence──┘
  Compute:  [B] ──fence──→ [C]
              │                    B's fence covers both C and D
              └──fence──→ [D]     (D is after C on graphics queue)
```

**What to watch out for:**

```
  Should this pass go async?

  Is it compute-only?  ──── no ──▶ can't (needs rasterization)
        │ yes
        ▼
  Duration > 0.5ms?   ──── no ──▶ don't bother
        │ yes                      (fence overhead ≈ 5-15µs
        ▼                           eats the savings)
  Independent from
  graphics tail?       ──── no ──▶ can't (DAG dependency)
        │ yes
        ▼
  ┌──────────────────┐
  │ ASYNC COMPUTE ✓  │  Good candidates: SSAO, volumetrics,
  └──────────────────┘  particle sim, light clustering
```

| Concern | Detail |
|---------|--------|
| **Queue ownership** | Vulkan: explicit `srcQueueFamilyIndex`/`dstQueueFamilyIndex` transfer. D3D12: `ID3D12Fence`. Both expensive — only if overlap wins exceed transfer cost. |
| **HW contention** | NVIDIA: separate async engines. AMD: more independent CUs. Some GPUs just time-slice — profile to confirm real overlap. |

In UE5, you opt in per pass with `ERDGPassFlags::AsyncCompute`; the RDG compiler handles fence insertion and cross-queue synchronization. Add after you have GPU-bound workloads that can genuinely overlap (e.g., SSAO while shadow maps render).

### 4. Split barriers
**Priority: LOW · Difficulty: High**

Place "begin" barrier as early as possible (right after the source pass finishes), "end" barrier as late as possible (right before the destination pass starts) → GPU has more room to overlap work between them.

```
  Regular barrier:        Split barrier:

  Pass A                  Pass A
    │                       │
    │                       ├─ begin barrier (flush)
    │                       │
    █ barrier (stall)       │  Pass B runs here —
    │                       │  GPU overlaps work
    │                       │
  Pass B                    ├─ end barrier (invalidate)
                            │
                          Pass C

  Less stall = more overlap = faster frame
```

**How the placement algorithm works:**

For each resource transition (resource R transitions from state S1 in pass A to state S2 in pass C):

1. **Begin barrier placement** — find the earliest point after pass A where R is no longer read or written. This is pass A's position + 1 in the sorted list (i.e., immediately after A finishes). Insert a "begin" that flushes caches for S1.
2. **End barrier placement** — find the latest point before pass C where R is still not yet needed. This is pass C's position − 1 (i.e., immediately before C starts). Insert an "end" that invalidates caches for S2.
3. **The gap between begin and end** is where the GPU can freely schedule other work without stalling on this transition.

**Translating to API calls:**

| | Begin (after source pass) | End (before dest pass) |
|---|---|---|
| **Vulkan** | `vkCmdSetEvent2` (flush src stages) | `vkCmdWaitEvents2` (invalidate dst stages) |
| **D3D12** | `BARRIER_FLAG_BEGIN_ONLY` | `BARRIER_FLAG_END_ONLY` |

```
  Concrete example — GBuffer → Lighting transition:

  Pass 3: GBuffer write       ←── begin barrier here
  Pass 4: SSAO (unrelated)         (flush COLOR_ATTACHMENT caches)
  Pass 5: Bloom (unrelated)        ↕ GPU freely executes 4 & 5
  Pass 6: Lighting read        ←── end barrier here
                                   (invalidate SHADER_READ caches)

  Gap of 2 passes = 2 passes of free overlap
```

**What to watch out for:**

```
  When to split vs. regular barrier:

  Gap size     Action         Why
  ─────────    ──────────     ──────────────────────────────
  0 passes     regular        begin/end adjacent → no benefit
  1 pass       maybe          marginal overlap
  2+ passes    split          measurable GPU overlap
  cross-queue  fence instead  can't split across queues
```

- **Driver overhead** — each `VkEvent` costs driver tracking. Only split when the gap spans 2+ passes.
- **Validation** — Vulkan validation layers flag bad event sequencing. Test with validation early.
- **Diminishing returns** — modern desktop drivers hide barrier latency internally. Biggest wins on: mobile GPUs, heavy pass gaps, expensive layout changes (depth → shader-read).
- **Async interaction** — if begin/end cross queue boundaries, use a fence instead. Handle before the split barrier pass.

Both Frostbite and UE5 support split barriers. Diminishing returns unless you're already saturating the pipeline. Add last, and only if profiling shows barrier stalls.

---

## Closing

A render graph is not always the right answer. If your project has a fixed pipeline with 3–4 passes that will never change, the overhead of a graph compiler is wasted complexity. But the moment your renderer needs to *grow* — new passes, new platforms, new debug tools — the graph pays for itself in the first week.

If you've made it this far, you now understand every major piece of UE5's RDG: the builder pattern, the two-phase pass declaration, transient resource aliasing, automatic barriers, pass culling, async compute flags, and the hybrid rebuild strategy. You can open `RenderGraphBuilder.h` and read it, not reverse-engineer it.

The point isn't that every project needs a render graph. The point is that if you understand how they work, you'll make a better decision about whether *yours* does.

<!-- TODO: Full source: [github.com/username/frame-graph-mvp](link) -->

---

## Resources

Further reading, ordered from "start here" to deep dives.

```
  Start here                Go deeper               Go deepest
  ──────────────            ──────────────           ──────────────
  Wijiler video (15 min) →  Frostbite GDC talk   →  UE5 RDG source
  Loggini overview       →  themaister blog      →  Vulkan sync blog
  GPUOpen render graphs  →  D3D12 barriers doc   →  AMD RPS SDK
```

**Quick visual intro (start here)** — **[Rendergraphs & High Level Rendering in Modern Graphics APIs — Wijiler (YouTube)](https://www.youtube.com/watch?v=FBYg64QKjFo)**
~15-minute video covering what render graphs are and how they fit into modern graphics APIs. Best starting point if you prefer video over text.

**Render graphs overview** — **[Render Graphs — GPUOpen](https://gpuopen.com/learn/render-graphs/)**
AMD's overview of render graph concepts and their RPS SDK. Covers declare/compile/execute, barriers, aliasing with D3D12 and Vulkan backends.

**The original talk that started it all** — **[FrameGraph: Extensible Rendering Architecture in Frostbite (GDC 2017)](https://www.gdcvault.com/play/1024612/FrameGraph-Extensible-Rendering-Architecture-in)**
Yuriy O'Donnell's GDC 2017 presentation — where the modern frame graph concept was introduced. If you read one thing, make it this.

**Render Graphs with D3D12 examples** — **[Render Graphs — Riccardo Loggini](https://logins.github.io/graphics/2021/05/31/RenderGraphs.html)**
Practical walkthrough with D3D12 placed resources. Covers setup/compile/execute phases with concrete code and transient memory aliasing.

**Render graphs and Vulkan — a deep dive** — **[themaister](https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/)**
Complete Vulkan render graph implementation in Granite. Covers subpass merging, barrier placement with VkEvent, async compute, and render target aliasing.

**UE5 Render Dependency Graph — official docs** — **[Render Dependency Graph in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine/)**
Epic's official RDG documentation. Covers `FRDGBuilder`, pass declaration, transient allocation, async compute, and RDG Insights debugging tools.

**Vulkan synchronization explained** — **[Understanding Vulkan Synchronization — Khronos Blog](https://www.khronos.org/blog/understanding-vulkan-synchronization)**
Khronos Group's guide to Vulkan sync primitives: pipeline barriers, events, semaphores, fences, and timeline semaphores.

**D3D12 resource barriers reference** — **[Using Resource Barriers — Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12)**
Microsoft's reference on D3D12 transition, aliasing, UAV, and split barriers. The exact API calls a D3D12 frame graph backend needs to emit.

**AMD Render Pipeline Shaders SDK (open source)** — **[RenderPipelineShaders — GitHub](https://github.com/GPUOpen-LibrariesAndSDKs/RenderPipelineShaders)**
AMD's open-source render graph framework (MIT). Automatic barriers, transient aliasing, RPSL language extension for HLSL. D3D12 + Vulkan.

