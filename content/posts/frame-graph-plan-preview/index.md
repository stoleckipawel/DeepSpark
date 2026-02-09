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

## Why You Want One

1. **Composability.** Add a pass, remove a pass, reorder passes. Nothing breaks. The graph recompiles.
2. **Memory.** 30–50% VRAM saved. The compiler alias-packs transient resources. No human does this reliably at 20+ passes.
3. **Barriers.** Automatic. Correct. You never write one again.
4. **Inspectability.** The frame is data. Export it, diff it, visualize it.

Every major engine shipped since 2017 uses some form of this. Frostbite introduced the concept at GDC 2017; UE5 ships it today as the **Rendering Dependency Graph (RDG)**. Unity and Godot have their own variants. This article explains how it works, gives you one to build yourself, and then maps every piece to UE5's RDG so you can read its source with confidence.

We'll go from a blank file to a working frame graph in C++, then walk through how UE5's RDG implements the same ideas at production scale — with notes on where Frostbite's original design diverges.

If you've ever watched VRAM spike because two passes that never overlap both hold full-res textures — or chased a black screen after reordering a render pass — this is for you.

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

**Schedule** — sort passes so every dependency is satisfied before execution. This is a topological sort on the DAG (Kahn's algorithm). If a cycle exists, the compiler catches it here.

**Allocate** — assign GPU memory to virtual resources. Two transient resources with non-overlapping lifetimes share the same physical block. Sort by first use, scan a short free-list, done.

**Synchronize** — insert barriers between passes that hand off resources. The compiler knows exactly who wrote what and who reads it next, so it places the minimal set — no conservative over-sync.

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
| Debug inspection | grep + printf | Graph export (DOT, visual tools) |

<!-- TODO: SVG diagrams — "Without graph" (tangled barrier spaghetti) vs "With graph" (clean DAG) -->

This isn't theoretical. Frostbite reported 50% VRAM reduction from aliasing at GDC 2017. UE5's RDG ships the same optimization today — every `FRDGTexture` marked as transient goes through the same aliasing pipeline we're about to build. The MVP we build next will give you automatic lifetimes and aliasing by Section 8, plus automatic barriers by Section 7. After that, we map everything to UE5's RDG.

---

## API Design

We start from the API you *want* to write — a minimal `FrameGraph` setup that declares a depth prepass, GBuffer pass, and lighting pass in ~20 lines of C++.

Key design choices visible in the API:

- Passes are lambdas — but *two* lambdas, and the split matters:
  - **Setup** runs at declaration time (CPU-side, building the graph). This is where you declare "I read texture A, I write texture B." No GPU work happens here — you're describing intent.
  - **Execute** runs later, during the execution phase, after the graph has been compiled. This is where you record actual GPU commands (`draw`, `dispatch`). By this point, the graph has already resolved all barriers and memory.
- Resources are requested by description (`{1920, 1080, RGBA8}`), not by GPU handle. They're virtual until the compiler maps them to physical memory.
- The graph owns transient resource lifetimes — the user never calls create/destroy. Imported resources (TAA history, backbuffer) are passed in from outside.

If you've seen UE5 code, this should look familiar. Our `addPass(setup, execute)` maps directly to `FRDGBuilder::AddPass`. Our `ResourceHandle` is their `FRDGTextureRef`. The two-lambda split is the same — UE5 just wraps it in a macro (`BEGIN_SHADER_PARAMETER_STRUCT`) to auto-generate the setup declarations.

The macro approach has a cost: it's opaque, hard to debug, and impossible to compose dynamically. If you want to declare resources conditionally at runtime, you fight the macro system. Our explicit two-lambda API is simpler and more flexible — UE5 traded that flexibility for compile-time validation and reflection.

This is the API we're building toward. The next three sections construct the internals, version by version.

<!-- TODO: ~20-line C++ usage snippet here -->

---

## MVP v1 — Declare & Execute

**Data structures:**
- `RenderPass` — a name, a setup function, and an execute function. (UE5: `FRDGPass`)
- `ResourceDesc` — a description (width, height, format, flags). This is the *virtual* description — no GPU handle yet. (UE5: `FRDGTextureDesc`)
- `ResourceHandle` — a lightweight index into the graph's resource array. (UE5: `FRDGTextureRef` / `FRDGBufferRef`)
- `FrameGraph` — owns flat arrays of passes and resources. Flat arrays are intentional: everything is frame-scoped, so you can use a linear allocator and free it all at frame end — zero fragmentation. (UE5: `FRDGBuilder` — the central object that owns the graph)

**Flow:** Declare passes in order → execute in order. No dependency tracking yet. Resources are created eagerly.

<!-- TODO: Full buildable C++ — structs, addPass(), execute(). ~60–80 lines. -->

**What it proves:** The lambda-based pass declaration pattern works. You can already compose passes without manual barrier calls (even though barriers are no-ops here).

**What it lacks:** This version executes passes in declaration order and creates every resource upfront. It's correct but wasteful. Version 2 adds the graph.

---

## MVP v2 — Dependencies & Barriers

**Resource versioning:** A resource can be written by pass A, read by pass B, then written *again* by pass C. To keep edges correct, each write creates a new **version** of the resource. Pass B's read depends on version 1 (A's write), not version 2 (C's write). Without versioning, the dependency graph would be ambiguous — this is the "rename on write" pattern.

**Resource tracking:** Each resource version tracks who wrote it and who reads it. On write, create a new version and record the pass. On read, record the pass and add a dependency edge from the writer. In practice, most resources have 1 writer and 1–3 readers.

**Dependency graph:** Stored as an adjacency list — for each pass, a list of passes that must come after it. For 25 passes you'll typically have 30–50 edges.

**Topological sort:** Kahn's algorithm (BFS-based). Maintain an in-degree count per pass and a queue of passes with zero dependencies. Pop from queue, decrement neighbors' in-degree, push newly-ready passes. Runs in O(V + E). If the output has fewer passes than input, a cycle exists — report an error. Kahn's is preferred over DFS-based topo-sort because cycle detection falls out naturally.

**Pass culling:** Walk backwards from the final output (present/backbuffer). Mark every reachable pass. Any unmarked pass is dead — remove it and release its resource declarations. This is ~10 lines but immediately useful: disable SSAO by not reading its output, and the pass (and all its resources) vanishes automatically. Complexity: O(V + E).

**Barrier insertion:** Walk the sorted order. For each pass, check each resource it uses against a lookup table tracking the resource's current state (last pipeline stage, access flags, image layout). If the usage changed, emit a barrier. A barrier at the GPU level means three things:
1. **Pipeline stall** — wait for the previous stage to finish writing.
2. **Cache flush/invalidate** — ensure writes are visible to the next reader.
3. **Layout transition** — e.g., from `COLOR_ATTACHMENT_OPTIMAL` (render target) to `SHADER_READ_ONLY_OPTIMAL` (sampled texture). In Vulkan this is a `VkImageMemoryBarrier2` with `srcStageMask` / `dstStageMask`; in D3D12 it's a `D3D12_RESOURCE_BARRIER` with `StateBefore` / `StateAfter`.

The graph can reason about the optimal `srcStageMask` and `dstStageMask` because it knows exactly which pass wrote and which pass reads — no conservative over-synchronization needed.

<!-- TODO: Show the diff from v1 — resource versioning, topo-sort, cull, barrier insertion. -->

**What it proves:** Automatic barriers from declared dependencies. Pass reordering is safe. Dead passes are culled. Three of the four intro promises delivered.

UE5 does exactly this. When you call `FRDGBuilder::AddPass` with `ERDGPassFlags::Raster` or `ERDGPassFlags::Compute`, RDG builds the same dependency graph from your declared reads/writes, topologically sorts it, culls dead passes, and inserts barriers — all before recording a single GPU command.

One caveat: UE5's migration to RDG is *incomplete*. Large parts of the renderer still use legacy immediate-mode `FRHICommandList` calls outside the graph. These "untracked" resources bypass RDG's barrier and aliasing systems entirely. The result: you get the graph's benefits only for passes that have been ported. Legacy passes still need manual barriers at the boundary where RDG-managed and unmanaged resources meet. This is the cost of retrofitting a graph onto a 25-year-old codebase — and a good argument for designing with a graph from the start.

**What it lacks:** Resources still live for the entire frame. Version 3 adds lifetime analysis and memory aliasing.

---

## MVP v3 — Lifetimes & Aliasing

**First/last use:** Walk the sorted pass list. For each transient resource, record `firstUsePass` and `lastUsePass`. Imported resources are excluded — their lifetimes extend beyond the frame.

**Reference counting:** Increment refcount at first use, decrement at last use. When refcount hits zero, that resource's physical memory is eligible for reuse by a later resource.

**Aliasing algorithm:** Sort transient resources by first-use pass, then scan a free-list for a compatible physical allocation. If one fits, reuse it. If not, allocate fresh.

The free-list is just a small array of available memory blocks, sorted by size. Lookup is linear, but with fewer than 10 entries that's trivially fast. The overall algorithm is a **greedy interval-coloring**: given resource lifetime intervals, assign physical memory slots such that overlapping intervals never share a slot. The greedy approach (sorted by start time) is optimal for interval graphs — a well-known result from combinatorics. A resource is "compatible" if:
1. Same memory type (e.g., device-local for textures, host-visible for readback).
2. Size ≥ what the new resource needs.
3. Non-overlapping lifetime (guaranteed by the graph — the previous user's `lastUsePass` < this resource's `firstUsePass`).

This is essentially a greedy interval-scheduling algorithm on resource lifetimes — the same approach Frostbite described at GDC 2017.

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

<!-- TODO: Diff from v2 — lifetime tracking, free-list allocator. ~30–40 new lines. -->

**What it proves:** The full value prop — automatic memory aliasing *and* automatic barriers. The MVP is now feature-equivalent to Frostbite's 2017 GDC demo (minus async compute).

In UE5, this is handled by the transient resource allocator. Any `FRDGTexture` created through `FRDGBuilder::CreateTexture` (as opposed to `RegisterExternalTexture`) is transient and eligible for aliasing. The RDG compiler runs the same lifetime analysis and free-list scan we just built.

A limitation worth noting: UE5 only aliases *transient* resources. Imported resources — even when their lifetimes are fully known within the frame — are never aliased. Frostbite's original design was more aggressive here, aliasing across a broader set of resources by tracking GPU-timeline lifetimes rather than just graph-declared lifetimes. If your renderer has large imported resources with predictable per-frame usage patterns, UE5's approach leaves VRAM on the table.

---

## A Real Frame

**Deferred Pipeline**

Depth prepass → GBuffer → SSAO → Lighting → Tonemap → Present

Two kinds of resources in play:
- **Transient:** GBuffer MRTs, SSAO scratch, HDR lighting buffer, bloom scratch — created and destroyed within this frame. Aliased by the graph.
- **Imported:** Backbuffer (acquired from swapchain, presented at end), TAA history (read from last frame, written this frame for next frame), shadow atlas (persistent, updated incrementally). The graph tracks their barriers but doesn't own their memory.

<!-- TODO: Full addPass chain with resource declarations + compiled execution order + auto-inserted barriers + memory aliasing in action -->

**Forward Pipeline**

Depth prepass → Forward shading (with MSAA) → Resolve → Post-process → Present

Fewer passes, different resource shapes, but same API. Fewer aliasing opportunities (fewer transient resources), but barriers are still fully automatic.

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

## Production Engines

### UE5's Rendering Dependency Graph (RDG)

UE5's RDG is the frame graph you're most likely to work with. It was retrofitted onto a 25-year-old renderer, so every design choice reflects a tension: do this properly *and* don't break the 10,000 existing draw calls.

**The builder pattern.** Everything goes through `FRDGBuilder`. You call `AddPass` to declare passes, `CreateTexture` / `CreateBuffer` to declare transient resources, and `RegisterExternalTexture` to import persistent ones. The builder accumulates the full DAG, then compiles and executes it in one shot at the end of the frame. This is the same declare → compile → execute flow from our MVP.

**Pass declaration.** Each `AddPass` call takes a parameter struct (declared with `BEGIN_SHADER_PARAMETER_STRUCT`) and an execute lambda. The parameter struct *is* the setup phase — it declares every resource the pass reads or writes through typed fields (`SHADER_PARAMETER_RDG_TEXTURE`, `RENDER_TARGET_BINDING_SLOT`, etc.). RDG reads these declarations to build dependency edges. No separate setup lambda needed — the struct metadata replaces it.

**Pass flags.** Every pass is tagged:
- `ERDGPassFlags::Raster` — graphics queue, expects render targets
- `ERDGPassFlags::Compute` — graphics queue, compute dispatch
- `ERDGPassFlags::AsyncCompute` — runs on the async compute queue
- `ERDGPassFlags::NeverCull` — exempt from dead-pass culling (useful for debug/readback passes)

**Resource types.** RDG tracks two resource types with identical dependency/barrier machinery:
- `FRDGTexture` / `FRDGTextureRef` — textures (render targets, SRVs, UAVs)
- `FRDGBuffer` / `FRDGBufferRef` — structured buffers, vertex/index buffers, indirect argument buffers

Both go through the same aliasing and barrier system. Buffer aliasing uses size and alignment rather than format matching.

**Transient allocator.** Resources created via `CreateTexture` are transient — RDG owns their lifetime and aliases them using the same interval-based approach we built in MVP v3. The allocator pools physical memory across frames so allocation costs amortize to near-zero after the first few frames.

**Barrier strategy.** RDG batches barriers by default — it collects all transitions needed before a pass and issues them together. It also supports split barriers (begin transition early, end transition late) to give the GPU more overlap room. This is closer to Frostbite's model than to simpler single-point barriers.

**Pass culling.** If a pass's outputs are never read by a subsequent pass, RDG marks it dead and skips it entirely. This is reference-count based — decrement when a resource's reader count hits zero, propagate backward. Same idea as our MVP, but integrated with the transient allocator so dead passes' memory is never allocated at all.

**Parallel command recording.** Because RDG knows the full DAG, it can identify passes with no dependencies between them and record their GPU commands on separate threads via parallel `FRHICommandList` instances. This is free CPU parallelism that falls out of having the dependency graph — and it's one of the main performance motivations for RDG in a scene with 50+ passes.

**Hybrid rebuild.** RDG uses a hybrid strategy — it caches the compiled graph and only rebuilds when the pass structure changes. For frames where the same passes run with the same resource shapes, the compile step is essentially free. When passes do change (e.g., debug visualization toggled), it falls back to a full recompile, which is still microseconds.

**Debugging.** `RDG Insights` in the Unreal editor visualizes the full pass graph, resource lifetimes, and barrier placement. You can also dump the graph to a log for offline analysis. This is the "inspectability" promise from the intro — the frame is data.

**What RDG gets wrong (or leaves on the table):**

- **Incomplete migration.** Significant parts of UE5's renderer still bypass RDG entirely, using legacy immediate-mode `FRHICommandList` calls. At the boundaries between RDG-managed and unmanaged code, you're back to manual barriers. This undermines the "you never write a barrier again" promise — it's only true for the passes that have been ported.
- **Macro-heavy API.** `BEGIN_SHADER_PARAMETER_STRUCT` provides compile-time validation and reflection, but it's opaque, hard to step through in a debugger, and resists dynamic composition. Declaring resources conditionally at runtime means fighting the macro system or falling back to manual parameter passing.
- **Transient-only aliasing.** RDG only aliases resources it creates. Imported resources with known per-frame lifetimes (e.g., a shadow atlas that's written early and read late) could theoretically share memory with transient resources during their idle window — but RDG doesn't attempt this. Frostbite's design was more aggressive here.
- **No automatic subpass merging.** On tile-based mobile GPUs, merging adjacent passes into Vulkan subpasses is critical for performance. RDG delegates this to the RHI layer rather than handling it in the graph compiler, which means the graph doesn't have enough information to make optimal merging decisions.
- **Async compute is opt-in, not automatic.** You manually tag passes with `ERDGPassFlags::AsyncCompute`. The compiler doesn't analyze the DAG to *discover* which passes could overlap — it trusts your annotation. If you forget the flag, or tag the wrong pass, you get suboptimal scheduling or sync bugs.

### Where Frostbite started

Frostbite's frame graph (O'Donnell & Barczak, GDC 2017: *"FrameGraph: Extensible Rendering Architecture in Frostbite"*) is where the modern render graph concept originates. Key ideas from that talk that shaped every later implementation:

- **Transient resource system** — the first production engine to alias GPU memory based on per-frame lifetime analysis. They reported 50% VRAM reduction on Battlefield 1.
- **Split barriers** — Frostbite was the first to split `begin` and `end` barrier placement to maximize GPU overlap. UE5 adopted the same idea.
- **Graph export** — internal DOT-format export for debugging. Every engine since has built something equivalent.
- **Dynamic rebuild** — Frostbite rebuilds the graph every frame (fully dynamic), unlike UE5's hybrid caching. The Frostbite team argued the compile cost is so low that caching adds complexity for negligible gain.

The Frostbite approach is more aggressive than UE5's in several areas: fully dynamic rebuild (no caching complexity), more granular split barriers, aliasing beyond just transient resources, and subpass merging at the graph level. It's also less concerned with backward compatibility — Frostbite controls the full engine, while UE5 has to support third-party game code and a partially-migrated legacy renderer.

### Other implementations

**Unity (SRP Render Graph)** — shipped as part of the Scriptable Render Pipeline. Handles pass culling and transient resource aliasing in URP/HDRP backends. Async compute support varies by platform. Designed for portability across mobile and desktop, so it avoids some of the more aggressive GPU-specific optimizations.

**Godot** — newest and simplest. The `RenderingDevice` abstraction includes basic render graph concepts, but with less automatic aliasing and no split barriers. Reflects a smaller team prioritizing clarity over advanced features. A good reference if you want to read a minimal, readable implementation.

### Comparison

| Feature | UE5 RDG | Frostbite | Unity SRP | Godot |
|---------|---------|-----------|-----------|-------|
| Rebuild strategy | hybrid (cached) | dynamic | dynamic | dynamic |
| Pass culling | ✓ auto | ✓ refcount | ✓ auto | partial |
| Memory aliasing | ✓ transient | ✓ full | ✓ transient | limited |
| Async compute | ✓ flag-based | ✓ | varies | basic |
| Split barriers | ✓ | ✓ | ✗ | ✗ |
| Parallel recording | ✓ | ✓ | limited | ✗ |
| Buffer tracking | ✓ | ✓ | ✓ | basic |
| Debug tools | RDG Insights | DOT export | Frame Debugger | — |

---

## Upgrade Roadmap

You've built the MVP. Here's what to add, in what order, and why.

### 1. Memory aliasing
**Priority: HIGH · Difficulty: Medium**

Biggest bang-for-buck. Reduces VRAM usage 30–50% for transient resources. The core idea is **interval-graph coloring** — assign physical memory to virtual resources such that no two overlapping lifetimes share an allocation. A greedy first-fit approach (sorted by start time) is provably optimal for interval graphs. For more aggressive packing, look at **linear-scan register allocation** from compiler literature — the same problem in a different domain. UE5's transient allocator does exactly this. Add immediately after the MVP works.

### 2. Pass merging / subpass folding
**Priority: HIGH on mobile · Difficulty: Medium**

Critical for tile-based GPUs (Mali, Adreno, Apple). Merge compatible passes into Vulkan subpasses or Metal render pass load/store actions. The algorithm identifies adjacent passes with compatible render targets and shared framebuffer dimensions, then contracts them into a single node — a **graph contraction** problem. A union-find structure tracks merged groups efficiently. UE5 doesn't do this automatically in RDG — subpass merging is handled at a lower level in the RHI — but Frostbite's original design included it. Add if targeting mobile or console (Switch).

### 3. Async compute
**Priority: MEDIUM · Difficulty: High**

Requires multi-queue infrastructure (compute queue + graphics queue). The graph compiler must find independent subgraphs that can execute concurrently — passes with **no path between them** in the DAG. This is a reachability problem: compute which passes can reach which, then any two unreachable passes can overlap. For small graphs (< 30 nodes), a bitset per node makes reachability queries near-instant. Fence placement is needed wherever dependency edges cross queue boundaries. In UE5, you opt in per pass with `ERDGPassFlags::AsyncCompute`; the RDG compiler handles fence insertion and cross-queue synchronization. Add after you have GPU-bound workloads that can overlap (e.g., SSAO while shadow maps render).

### 4. Split barriers
**Priority: LOW · Difficulty: High**

Place "begin" barrier as early as possible (right after the source pass finishes), "end" barrier as late as possible (right before the destination pass starts) → GPU has more room to overlap work between them. Finding the optimal placement is a range query on the sorted pass list. Both Frostbite and UE5 support split barriers. Diminishing returns unless you're already saturating the pipeline. Add last, and only if profiling shows barrier stalls.

### Priority matrix

| Feature | VRAM Savings | GPU Time Savings | Impl Effort | Add When |
|---------|-------------|-----------------|-------------|----------|
| Memory aliasing | ★★★ | ★ | ★★ | First |
| Pass merging | ★ | ★★★ (mobile) | ★★ | If mobile |
| Async compute | — | ★★ | ★★★ | If GPU-bound |
| Split barriers | — | ★ | ★★★ | Last |

---

## Closing

A render graph is not always the right answer. If your project has a fixed pipeline with 3–4 passes that will never change, the overhead of a graph compiler is wasted complexity. But the moment your renderer needs to *grow* — new passes, new platforms, new debug tools — the graph pays for itself in the first week.

If you've made it this far, you now understand every major piece of UE5's RDG: the builder pattern, the two-phase pass declaration, transient resource aliasing, automatic barriers, pass culling, async compute flags, and the hybrid rebuild strategy. You can open `RenderGraphBuilder.h` and read it, not reverse-engineer it.

The point isn't that every project needs a render graph. The point is that if you understand how they work, you'll make a better decision about whether *yours* does.

<!-- TODO: Full source: [github.com/username/frame-graph-mvp](link) -->
