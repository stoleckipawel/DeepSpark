---
title: "Frame Graph — Production Engines"
date: 2026-02-11
draft: false
description: "How UE5 and Frostbite implement frame graphs at scale — plus the upgrade roadmap from MVP to production."
tags: ["rendering", "frame-graph", "gpu", "architecture", "ue5"]
categories: ["analysis"]
series: ["Rendering Architecture"]
showTableOfContents: false
---

{{< article-nav >}}

<div style="margin:0 0 1.5em;padding:.7em 1em;border-radius:8px;background:rgba(99,102,241,.04);border:1px solid rgba(99,102,241,.12);font-size:.88em;line-height:1.6;opacity:.85;">
📖 <strong>Part III of III.</strong>&ensp; <a href="/posts/frame-graph-theory/">Theory</a> → <a href="/posts/frame-graph-build-it/">Build It</a> → <em>Production Engines</em>
</div>

<div style="border-left:4px solid #6366f1;background:linear-gradient(135deg,rgba(99,102,241,.06),transparent);border-radius:0 10px 10px 0;padding:1em 1.3em;margin:1em 0;font-size:.95em;font-style:italic;line-height:1.55">
How UE5 and Frostbite implement the same ideas at scale — what they added, what they compromised, and where they still differ.
</div>

[Part II](/posts/frame-graph-build-it/) left us with a working frame graph — automatic barriers, pass culling, and memory aliasing in ~300 lines of C++. That's a solid MVP, but production engines face problems we didn't: parallel command recording, subpass merging, async compute scheduling, and managing thousands of passes across legacy codebases. This article examines how UE5 and Frostbite solved those problems, then maps out the path from MVP to production.

---

## ① Declare — Pass & Resource Registration

Every engine starts the same way: passes declare what they read and write, resources are requested by description, and the graph accumulates edges. The differences are in *how* that declaration happens.

### UE5 RDG

Each `AddPass` takes a parameter struct + execute lambda. The struct *is* the setup phase — macros generate metadata, RDG extracts dependency edges:

<div class="diagram-macro">
  <div class="dm-code">
    <span style="color:#8b5cf6">BEGIN_SHADER_PARAMETER_STRUCT(...)</span><br>
    &nbsp;&nbsp;SHADER_PARAMETER_RDG_TEXTURE(Input)<br>
    &nbsp;&nbsp;RENDER_TARGET_BINDING_SLOT(Output)<br>
    <span style="color:#8b5cf6">END_SHADER_PARAMETER_STRUCT()</span>
  </div>
  <div class="dm-arrow">→</div>
  <div class="dm-result">
    <span style="color:#22c55e">read edge</span> ←<br>
    <span style="color:#ef4444">write edge</span> ← &nbsp;→ DAG
  </div>
</div>
<div style="font-size:.78em;opacity:.6;margin-top:-.3em">Macro generates metadata → RDG extracts dependency edges. No separate setup lambda needed.</div>

**Pass flags** control queue and behavior — `ERDGPassFlags::Raster`, `::Compute`, `::AsyncCompute`, `::NeverCull`, `::Copy`. **Resources** are either transient (`CreateTexture` — graph-owned, eligible for aliasing) or imported (`RegisterExternalTexture` — externally owned, barriers tracked but no aliasing).

<div style="display:flex;gap:1em;flex-wrap:wrap;margin:1em 0">
  <div style="flex:1;min-width:260px;border:1px solid rgba(59,130,246,.25);border-radius:10px;overflow:hidden">
    <div style="background:linear-gradient(135deg,rgba(59,130,246,.12),rgba(59,130,246,.05));padding:.6em 1em;font-weight:700;font-size:.9em;color:#3b82f6;border-bottom:1px solid rgba(59,130,246,.15)">Pass Flags</div>
    <div style="padding:.6em 1em;font-size:.85em;line-height:1.8">
      <code>ERDGPassFlags::Raster</code> — Graphics queue, render targets<br>
      <code>ERDGPassFlags::Compute</code> — Graphics queue, compute dispatch<br>
      <code>ERDGPassFlags::AsyncCompute</code> — Async compute queue<br>
      <code>ERDGPassFlags::NeverCull</code> — Exempt from dead-pass culling<br>
      <code>ERDGPassFlags::Copy</code> — Copy queue operations<br>
      <code>ERDGPassFlags::SkipRenderPass</code> — Raster pass that manages its own render pass
    </div>
  </div>
  <div style="flex:1;min-width:260px;border:1px solid rgba(139,92,246,.25);border-radius:10px;overflow:hidden">
    <div style="background:linear-gradient(135deg,rgba(139,92,246,.12),rgba(139,92,246,.05));padding:.6em 1em;font-weight:700;font-size:.9em;color:#8b5cf6;border-bottom:1px solid rgba(139,92,246,.15)">Resource Types</div>
    <div style="padding:.6em 1em;font-size:.85em;line-height:1.8">
      <code>FRDGTexture</code> / <code>FRDGTextureRef</code> — Render targets, SRVs, UAVs<br>
      <code>FRDGBuffer</code> / <code>FRDGBufferRef</code> — Structured, vertex/index, indirect args<br>
      <code>FRDGUniformBuffer</code> — Uniform/constant buffer references<br>
      Created via <code>CreateTexture()</code> (transient) or <code>RegisterExternalTexture()</code> (imported)
    </div>
  </div>
</div>

### Frostbite

Frostbite's GDC 2017 talk described a similar lambda-based declaration — setup lambda declares reads/writes, execute lambda records GPU commands. The exact current implementation isn't public.

### What's different from our MVP

<div class="diagram-ftable">
<table>
  <tr><th>Declaration aspect</th><th>Our MVP</th><th>Production engines</th></tr>
  <tr><td><strong>Edge declaration</strong></td><td>Explicit <code>read()</code> / <code>write()</code> calls in setup lambda</td><td>UE5: macro-generated metadata. Frostbite: lambda-based, similar to MVP.</td></tr>
  <tr><td><strong>Resource creation</strong></td><td>All transient, created by description</td><td>Transient + imported distinction. Imported resources track barriers but aren't aliased in UE5.</td></tr>
  <tr><td><strong>Queue assignment</strong></td><td>Single queue</td><td>Per-pass flags: graphics, compute, async compute, copy</td></tr>
  <tr><td><strong>Rebuild</strong></td><td>Full rebuild every frame</td><td>UE5: hybrid (cached topology, invalidated on change). Others: dynamic rebuild.</td></tr>
</table>
</div>

---

## ② Compile — The Graph Compiler at Scale

This is where production engines diverge most from our MVP. The compile phase runs entirely on the CPU, between declaration and execution. Our MVP does five things here: topo-sort, cull, scan lifetimes, alias, and compute barriers. Production engines do the same five — plus pass merging, async compute scheduling, split barrier placement, and barrier batching.

<div class="diagram-phases">
  <div class="dph-col" style="border-color:#8b5cf6;flex:1;">
    <div class="dph-title" style="color:#8b5cf6">MVP compile</div>
    <div class="dph-body" style="font-size:.84em;">
      ├ topo-sort<br>
      ├ cull dead passes<br>
      ├ scan lifetimes<br>
      ├ alias memory<br>
      └ compute barriers
    </div>
  </div>
  <div style="display:flex;align-items:center;font-size:1.4em;color:#8b5cf6;font-weight:700">→</div>
  <div class="dph-col" style="border-color:#6366f1;flex:1.4;">
    <div class="dph-title" style="color:#6366f1">Production compile</div>
    <div class="dph-body" style="font-size:.84em;">
      ├ topo-sort<br>
      ├ cull dead passes<br>
      ├ scan lifetimes<br>
      ├ alias memory <span style="opacity:.5">+ cross-frame pooling</span><br>
      ├ <strong>merge passes</strong> (subpass optimization)<br>
      ├ <strong>schedule async compute</strong><br>
      ├ compute barriers <span style="opacity:.5">+ split begin/end</span><br>
      └ <strong>batch barriers</strong>
    </div>
  </div>
</div>

Every step below is a compile-time operation — no GPU work, no command recording. The compiler sees the full DAG and makes optimal decisions the pass author never has to think about.

### Pass culling

Same algorithm as our MVP — backward reachability from the output — but at larger scale. UE5 uses refcount-based culling and skips allocation entirely for culled passes (saves transient allocator work). Culled passes never execute, never allocate resources, never emit barriers — they vanish as if they were never declared.

### Memory aliasing

Both engines use the same core algorithm from [Part II](/posts/frame-graph-build-it/) — lifetime scanning + free-list allocation. The production refinements:

<div class="diagram-ftable">
<table>
  <tr><th>Refinement</th><th>UE5 RDG</th><th>Frostbite (GDC talk)</th></tr>
  <tr><td><strong>Placed resources</strong></td><td><code>FRDGTransientResourceAllocator</code> binds into <code>ID3D12Heap</code> offsets</td><td>Heap sub-allocation</td></tr>
  <tr><td><strong>Size bucketing</strong></td><td>Power-of-two in transient allocator</td><td>Custom bin sizes</td></tr>
  <tr><td><strong>Cross-frame pooling</strong></td><td>Persistent pool, peak-N-frames sizing</td><td>Pooling described in talk</td></tr>
  <tr><td><strong>Imported aliasing</strong></td><td><span style="color:#ef4444">✗</span> transient only</td><td>Described as supported</td></tr>
</table>
</div>

Our MVP allocates fresh each frame. Production engines **pool across frames** — once a heap is allocated, it persists and gets reused. UE5's `FRDGTransientResourceAllocator` tracks peak usage over several frames and only grows the pool when needed. This amortizes allocation cost to near zero in steady state.

### Pass merging

Pass merging is a compile-time optimization: the compiler identifies adjacent passes that share render targets and fuses them into a single render pass. On consoles with fixed-function hardware and on PC with D3D12 Render Pass Tier 2, this lets the GPU keep data on-chip between fused subpasses, avoiding expensive DRAM round-trips.

How each engine handles it:

- **UE5 RDG** delegates to the RHI layer. The graph compiler doesn't merge passes itself — pass authors never see subpasses, and the graph has no subpass concept.
- **Frostbite's** GDC talk described automatic merging in the graph compiler as a first-class feature.

### Async compute scheduling

Async compute lets the GPU overlap independent work on separate hardware queues — compute shaders running alongside rasterization. The compiler must identify which passes can safely run async, insert cross-queue fences, and manage resource ownership transfers.

| Engine | Approach | Discovery |
|--------|----------|-----------|
| **UE5** | Opt-in via `ERDGPassFlags::AsyncCompute` per pass | Manual — compiler trusts the flag, handles fence insertion + cross-queue sync |
| **Frostbite** | Described as automatic in GDC talk | Reachability analysis in the compiler |

**Hardware reality:** NVIDIA uses separate async engines. AMD exposes more independent CUs. Some GPUs just time-slice — always profile to confirm real overlap. Vulkan requires explicit queue family ownership transfer; D3D12 uses `ID3D12Fence`. Both are expensive — only worth it if overlap wins exceed transfer cost.

### Barrier batching & split barriers

Our MVP inserts one barrier at a time. Production engines batch multiple transitions into a single API call and split barriers across pass gaps for better GPU pipelining.

UE5 batches transitions via `FRDGBarrierBatchBegin`/`FRDGBarrierBatchEnd` — multiple resource transitions coalesced into one API call. Split barriers place the "begin" transition as early as possible and the "end" just before the resource is needed, giving the GPU time to pipeline the transition.

Diminishing returns on desktop — modern drivers hide barrier latency internally. Biggest wins on expensive layout transitions (depth → shader-read) and console GPUs with more exposed pipeline control. Add last, and only if profiling shows barrier stalls.

### Compile comparison

<div style="overflow-x:auto;margin:1em 0">
<table style="width:100%;border-collapse:collapse;border-radius:10px;overflow:hidden;font-size:.9em">
  <thead>
    <tr style="background:linear-gradient(135deg,rgba(99,102,241,.1),rgba(59,130,246,.08))">
      <th style="padding:.7em 1em;text-align:left;border-bottom:2px solid rgba(99,102,241,.2)">Compile feature</th>
      <th style="padding:.7em 1em;text-align:center;border-bottom:2px solid rgba(59,130,246,.2);color:#3b82f6">UE5 RDG</th>
      <th style="padding:.7em 1em;text-align:center;border-bottom:2px solid rgba(34,197,94,.2);color:#22c55e">Frostbite (GDC)</th>
    </tr>
  </thead>
  <tbody>
    <tr><td style="padding:.5em 1em;font-weight:600">Pass culling</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> refcount-based</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> refcount</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.5em 1em;font-weight:600">Memory aliasing</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> transient only</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> described as full</td></tr>
    <tr><td style="padding:.5em 1em;font-weight:600">Pass merging</td><td style="padding:.5em 1em;text-align:center">RHI layer</td><td style="padding:.5em 1em;text-align:center">graph compiler</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.5em 1em;font-weight:600">Async compute</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> opt-in flag</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> automatic</td></tr>
    <tr><td style="padding:.5em 1em;font-weight:600">Split barriers</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> batched</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.5em 1em;font-weight:600">Barrier batching</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td></tr>
    <tr><td style="padding:.5em 1em;font-weight:600">Rebuild strategy</td><td style="padding:.5em 1em;text-align:center">hybrid (cached)</td><td style="padding:.5em 1em;text-align:center">dynamic</td></tr>
  </tbody>
</table>
</div>

---

## ③ Execute — Recording & Submission

After the compiler finishes, every decision has been made — pass order, memory layout, barrier placement. The execute phase just walks the plan and records GPU commands. Here's where production engines scale beyond our MVP.

### Parallel command recording

Our MVP records on a single thread. Production engines split the sorted pass list into groups and record each group on a separate thread using secondary command buffers (Vulkan) or command lists (D3D12), then merge at submit.

UE5 creates parallel `FRHICommandList` instances — one per pass group — and joins them before queue submission. This is where the bulk of CPU frame time goes in a graph-based renderer, so parallelizing it matters.

### The legacy boundary (UE5)

The biggest practical challenge with RDG isn't the graph itself — it's the seam between RDG-managed passes and legacy `FRHICommandList` code. At this boundary:

- Barriers must be inserted manually (RDG can't see what the legacy code does)
- Resources must be "extracted" from RDG via `ConvertToExternalTexture()` before legacy code can use them
- Re-importing back into RDG requires `RegisterExternalTexture()` with correct state tracking

This boundary is shrinking every release as Epic migrates more passes to RDG, but in practice you'll still hit it when integrating third-party plugins or older rendering features.

### Debug & visualization

<div style="display:flex;align-items:flex-start;gap:.8em;border:1px solid rgba(34,197,94,.2);border-radius:10px;padding:1em 1.2em;margin:1em 0;background:linear-gradient(135deg,rgba(34,197,94,.05),transparent)">
  <span style="font-size:1.4em;line-height:1">🔍</span>
  <div style="font-size:.9em;line-height:1.55"><strong>RDG Insights.</strong> Enable via the Unreal editor to visualize the full pass graph, resource lifetimes, and barrier placement. Use <code>r.RDG.Debug</code> CVars for validation: <code>r.RDG.Debug.FlushGPU</code> serializes execution for debugging, <code>r.RDG.Debug.ExtendResourceLifetimes</code> disables aliasing to isolate corruption bugs. The frame is data — export it, diff it, analyze offline.</div>
</div>

### Navigating the UE5 RDG source

<div class="diagram-steps">
  <div class="ds-step">
    <div class="ds-num">1</div>
    <div><code>RenderGraphBuilder.h</code> — <code>FRDGBuilder</code> is the graph object. <code>AddPass()</code>, <code>CreateTexture()</code>, <code>Execute()</code> are all here. Start reading here.</div>
  </div>
  <div class="ds-step">
    <div class="ds-num">2</div>
    <div><code>RenderGraphPass.h</code> — <code>FRDGPass</code> stores the parameter struct, execute lambda, and pass flags. The macro-generated metadata lives on the parameter struct.</div>
  </div>
  <div class="ds-step">
    <div class="ds-num">3</div>
    <div><code>RenderGraphResources.h</code> — <code>FRDGTexture</code>, <code>FRDGBuffer</code>, and their SRV/UAV views. Tracks current state for barrier emission. Check <code>FRDGResource::GetRHI()</code> to see when virtual becomes physical.</div>
  </div>
  <div class="ds-step">
    <div class="ds-num">4</div>
    <div><code>RenderGraphPrivate.h</code> — The compile phase: topological sort, pass culling, barrier batching, async compute fence insertion. The core algorithms live here.</div>
  </div>
</div>

### UE5 RDG — known limitations

<div class="diagram-limits">
  <div class="dl-title">RDG Limitations</div>
  <div class="dl-item"><span class="dl-x">✗</span> <strong>Incomplete migration</strong> — Legacy FRHICommandList ←→ RDG boundary = manual barriers at the seam</div>
  <div class="dl-item"><span class="dl-x">✗</span> <strong>Macro-heavy API</strong> — BEGIN_SHADER_PARAMETER_STRUCT → opaque, no debugger stepping, fights dynamic composition</div>
  <div class="dl-item"><span class="dl-x">✗</span> <strong>Transient-only aliasing</strong> — Imported resources never aliased, even when lifetime is fully known within the frame</div>
  <div class="dl-item"><span class="dl-x">✗</span> <strong>No automatic subpass merging</strong> — Delegated to RHI — graph can't optimize render pass structure directly</div>
  <div class="dl-item"><span class="dl-x">✗</span> <strong>Async compute is opt-in</strong> — Manual ERDGPassFlags::AsyncCompute tagging. Compiler trusts, doesn't discover.</div>
</div>

---

## 🏁 Closing

A render graph is not always the right answer. If your project has a fixed pipeline with 3–4 passes that will never change, the overhead of a graph compiler is wasted complexity. But the moment your renderer needs to *grow* — new passes, new platforms, new debug tools — the graph pays for itself in the first week.

Across these three articles, we covered the full arc: [Part I](/posts/frame-graph-theory/) laid out all the theory — the declare/compile/execute lifecycle, pass merging, async compute, and split barriers. [Part II](/posts/frame-graph-build-it/) turned the core into working C++ — automatic barriers, pass culling, and memory aliasing. And this article mapped those ideas onto what ships in UE5 and Frostbite, showing how production engines implement the same concepts at scale.

You can now open `RenderGraphBuilder.h` in UE5 and *read* it, not reverse-engineer it. You know what `FRDGBuilder::AddPass` builds, how the transient allocator aliases memory, why `ERDGPassFlags::AsyncCompute` exists, and where the RDG boundary with legacy code still leaks.

The point isn't that every project needs a render graph. The point is that if you understand how they work, you'll make a better decision about whether *yours* does.

---

## 📚 Resources

- **[Rendergraphs & High Level Rendering — Wijiler (YouTube)](https://www.youtube.com/watch?v=FBYg64QKjFo)** — 15-minute visual intro to render graphs and modern graphics APIs.
- **[Render Graphs — GPUOpen](https://gpuopen.com/learn/render-graphs/)** — AMD's overview covering declare/compile/execute, barriers, and aliasing.
- **[FrameGraph: Extensible Rendering Architecture in Frostbite (GDC 2017)](https://www.gdcvault.com/play/1024612/FrameGraph-Extensible-Rendering-Architecture-in)** — The original talk that introduced the modern frame graph concept.
- **[Render Graphs — Riccardo Loggini](https://logins.github.io/graphics/2021/05/31/RenderGraphs.html)** — Practical walkthrough with D3D12 placed resources and transient aliasing.
- **[Render graphs and Vulkan — themaister](https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/)** — Full Vulkan implementation covering subpass merging, barriers, and async compute.
- **[Render Dependency Graph — Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine/)** — Epic's official RDG documentation.
- **[Understanding Vulkan Synchronization — Khronos Blog](https://www.khronos.org/blog/understanding-vulkan-synchronization)** — Pipeline barriers, events, semaphores, fences, and timeline semaphores.
- **[Using Resource Barriers — Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12)** — D3D12 transition, aliasing, UAV, and split barriers reference.
- **[RenderPipelineShaders — GitHub (AMD)](https://github.com/GPUOpen-LibrariesAndSDKs/RenderPipelineShaders)** — Open-source render graph framework with automatic barriers and transient aliasing.

---

<div style="margin:2em 0 0;padding:1em 1.2em;border-radius:10px;border:1px solid rgba(99,102,241,.2);background:rgba(99,102,241,.03);display:flex;justify-content:flex-start;">
  <a href="/posts/frame-graph-build-it/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    ← Previous: Part II — Build It
  </a>
</div>
