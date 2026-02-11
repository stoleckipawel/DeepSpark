---
title: "Frame Graph — Build It"
date: 2026-02-10
draft: true
description: "Three iterations from blank file to working frame graph with automatic barriers and memory aliasing."
tags: ["rendering", "frame-graph", "gpu", "architecture", "cpp"]
categories: ["analysis"]
series: ["Rendering Architecture"]
showTableOfContents: false
---

{{< article-nav >}}

*Three iterations from blank file to working frame graph with automatic barriers and memory aliasing. Each version builds on the last — by the end you'll have something you can drop into a real renderer.*

---

## API Design

We start from the API you *want* to write — a minimal `FrameGraph` that declares a depth prepass, GBuffer pass, and lighting pass in ~20 lines of C++.

### Design principles

<div style="margin:1em 0 1.4em;display:grid;grid-template-columns:repeat(3,1fr);gap:.8em;">
  <div style="padding:1em;border-radius:10px;border-top:3px solid #3b82f6;background:rgba(59,130,246,.04);">
    <div style="font-weight:800;font-size:.92em;margin-bottom:.4em;color:#3b82f6;">λ² &ensp;Two lambdas</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      <strong>Setup</strong> — runs at declaration. Declares reads &amp; writes. No GPU work.<br>
      <strong>Execute</strong> — runs later. Records GPU commands into a fully resolved environment.
    </div>
  </div>
  <div style="padding:1em;border-radius:10px;border-top:3px solid #8b5cf6;background:rgba(139,92,246,.04);">
    <div style="font-weight:800;font-size:.92em;margin-bottom:.4em;color:#8b5cf6;">📐 &ensp;Virtual resources</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      Requested by description (<code>{1920, 1080, RGBA8}</code>), not GPU handle. Virtual until the compiler maps them to memory.
    </div>
  </div>
  <div style="padding:1em;border-radius:10px;border-top:3px solid #22c55e;background:rgba(34,197,94,.04);">
    <div style="font-weight:800;font-size:.92em;margin-bottom:.4em;color:#22c55e;">♻️ &ensp;Owned lifetimes</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      The graph owns every transient resource from first use to last. You never call create or destroy.
    </div>
  </div>
</div>

These three ideas produce a natural pipeline — declare your intent, let the compiler optimize, then execute:

<!-- Timeline: declaration → compile → execution -->
<div class="diagram-phases">
  <div class="dph-col" style="border-color:#3b82f6">
    <div class="dph-title" style="color:#3b82f6">Declaration time</div>
    <div class="dph-body">
      <code>addPass(setup, execute)</code><br>
      ├ setup lambda runs<br>
      &nbsp;&nbsp;• declare reads<br>
      &nbsp;&nbsp;• declare writes<br>
      &nbsp;&nbsp;• request resources<br>
      └ <strong>no GPU work here</strong>
    </div>
  </div>
  <div class="dph-col" style="border-color:#8b5cf6">
    <div class="dph-title" style="color:#8b5cf6">Graph Compiler</div>
    <div class="dph-body">
      • sort<br>
      • alias<br>
      • barrier
    </div>
  </div>
  <div class="dph-col" style="border-color:#22c55e">
    <div class="dph-title" style="color:#22c55e">Execution time</div>
    <div class="dph-body">
      execute lambda runs here<br>
      → record draw/dispatch<br>
      &nbsp;&nbsp;actual GPU cmds<br><br>
      <em>barriers already<br>inserted by compiler</em>
    </div>
  </div>
</div>

### UE5 mapping

If you've worked with UE5's RDG, our API maps directly:

<div style="margin:.8em 0;font-size:.9em;line-height:1.8;font-family:ui-monospace,monospace;">
  <span style="opacity:.5;">ours</span> <code>addPass(setup, execute)</code> &ensp;→&ensp; <span style="opacity:.5;">UE5</span> <code>FRDGBuilder::AddPass</code><br>
  <span style="opacity:.5;">ours</span> <code>ResourceHandle</code> &ensp;→&ensp; <span style="opacity:.5;">UE5</span> <code>FRDGTextureRef</code> / <code>FRDGBufferRef</code><br>
  <span style="opacity:.5;">ours</span> <code>setup lambda</code> &ensp;→&ensp; <span style="opacity:.5;">UE5</span> <code>BEGIN_SHADER_PARAMETER_STRUCT</code><br>
  <span style="opacity:.5;">ours</span> <code>execute lambda</code> &ensp;→&ensp; <span style="opacity:.5;">UE5</span> <code>execute lambda</code> <span style="font-family:inherit;opacity:.5;">(same concept)</span>
</div>

<div style="margin:1em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid #f59e0b;background:rgba(245,158,11,.05);font-size:.9em;line-height:1.6;">
⚠️ <strong>UE5's macro tradeoff:</strong> The <code>BEGIN_SHADER_PARAMETER_STRUCT</code> approach is opaque, hard to debug, and impossible to compose dynamically. Our explicit two-lambda API is simpler and more flexible — UE5 traded that flexibility for compile-time validation and reflection.
</div>

### Putting it together

Here's how the final API reads — three passes, ~20 lines:

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

## MVP v1 — Declare & Execute

**Data structures:**

<div class="diagram-struct">
  <div class="dst-title">FrameGraph <span class="dst-comment">(UE5: FRDGBuilder)</span></div>
  <div class="dst-section">
    <strong>passes[]</strong> → <strong>RenderPass</strong> <span class="dst-comment">(UE5: FRDGPass)</span><br>
    &nbsp;&nbsp;• name<br>
    &nbsp;&nbsp;• setup() <span class="dst-comment">← build the DAG</span><br>
    &nbsp;&nbsp;• execute() <span class="dst-comment">← record GPU cmds</span>
  </div>
  <div class="dst-section">
    <strong>resources[]</strong> → <strong>ResourceDesc</strong> <span class="dst-comment">(UE5: FRDGTextureDesc)</span><br>
    &nbsp;&nbsp;• width, height, format<br>
    &nbsp;&nbsp;• virtual — no GPU handle yet
  </div>
  <div class="dst-section">
    <strong>ResourceHandle</strong> = index into resources[]<br>
    <span class="dst-comment">(UE5: FRDGTextureRef / FRDGBufferRef)</span>
  </div>
  <div class="dst-section">
    <span class="dst-comment">← linear allocator: all frame-scoped, free at frame end</span>
  </div>
</div>

**Flow:** Declare passes in order → execute in order. No dependency tracking yet. Resources are created eagerly.

**Buildable C++ — the full MVP v1** ([frame_graph_v1.h](frame_graph_v1.h)):

{{< include-code file="frame_graph_v1.h" lang="cpp" >}}

**Usage** — three passes wired together ([example_v1.cpp](example_v1.cpp)):

{{< include-code file="example_v1.cpp" lang="cpp" compile="true" deps="frame_graph_v1.h" >}}

This compiles and runs. It doesn't *do* anything on the GPU yet — the execute lambdas are stubs — but the scaffolding is real. Every piece we add in v2 and v3 goes into this same `FrameGraph` class.

<div style="display:grid;grid-template-columns:1fr 1fr;gap:.8em;margin:1em 0;">
  <div style="padding:.7em 1em;border-radius:8px;border-left:4px solid #22c55e;background:rgba(34,197,94,.05);font-size:.9em;line-height:1.5;">
    <strong style="color:#22c55e;">✓ What it proves</strong><br>
    The lambda-based pass declaration pattern works. You can already compose passes without manual barrier calls (even though barriers are no-ops here).
  </div>
  <div style="padding:.7em 1em;border-radius:8px;border-left:4px solid #ef4444;background:rgba(239,68,68,.05);font-size:.9em;line-height:1.5;">
    <strong style="color:#ef4444;">✗ What it lacks</strong><br>
    Executes passes in declaration order, creates every resource upfront. Correct but wasteful. Version 2 adds the graph.
  </div>
</div>

---

## MVP v2 — Dependencies & Barriers

<div style="margin:1em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid #3b82f6;background:rgba(59,130,246,.04);font-size:.92em;line-height:1.6;">
🎯 <strong>Goal:</strong> Automatic pass ordering, dead-pass culling, and barrier insertion — the core value of a render graph.
</div>

**Resource versioning:** A resource can be written by pass A, read by pass B, then written *again* by pass C. To keep edges correct, each write creates a new **version** of the resource. Pass B's read depends on version 1 (A's write), not version 2 (C's write). Without versioning, the dependency graph would be ambiguous — this is the "rename on write" pattern.

<div class="diagram-version">
  <div class="dv-row">
    <span class="dv-pass">Pass A</span>
    <span class="dv-action">writes</span>
    <span class="dv-res">GBuffer v1</span>
    <span class="dv-edge">──→</span>
    <span class="dv-pass" style="background:#8b5cf6">Pass B</span>
    <span class="dv-action">reads</span>
    <span class="dv-res">GBuffer v1</span>
  </div>
  <div class="dv-row" style="margin-top:.3em">
    <span class="dv-pass">Pass C</span>
    <span class="dv-action">writes</span>
    <span class="dv-res">GBuffer v2</span>
    <span class="dv-edge">──→</span>
    <span class="dv-pass" style="background:#8b5cf6">Pass D</span>
    <span class="dv-action">reads</span>
    <span class="dv-res">GBuffer v2</span>
  </div>
  <div style="font-size:.78em;opacity:.6;margin-top:.5em;line-height:1.6">
    B depends on A (v1), D depends on C (v2).<br>
    B does <strong>NOT</strong> depend on C — versioning keeps them separate.
  </div>
</div>

**Resource tracking:** Each resource version tracks who wrote it and who reads it. On write, create a new version and record the pass. On read, record the pass and add a dependency edge from the writer. In practice, most resources have 1 writer and 1–3 readers.

**Dependency graph:** Stored as an adjacency list — for each pass, a list of passes that must come after it. For 25 passes you'll typically have 30–50 edges.

**Topological sort (Kahn's algorithm):**

The algorithm counts incoming edges (in-degree) for every pass. Passes with zero in-degree have no unsatisfied dependencies — they're ready to run. Step through the interactive demo below to see how the queue drains, in-degrees decrement, and a valid execution order emerges:

{{< interactive-toposort >}}

Runs in O(V + E). Kahn's is preferred over DFS-based topo-sort because cycle detection falls out naturally — if the sorted output is shorter than the pass count, a cycle exists.

**Pass culling:** Walk backwards from the final output (present/backbuffer). Mark every reachable pass. Any unmarked pass is dead — remove it and release its resource declarations. This is ~10 lines but immediately useful: disable SSAO by not reading its output, and the pass (and all its resources) vanishes automatically. Complexity: O(V + E). Try disabling edges in the interactive graph below and watch passes get culled in real time:

{{< interactive-dag >}}

**Barrier insertion:** Walk the sorted order. For each pass, check each resource against a state table tracking its current pipeline stage, access flags, and image layout. If usage changed, emit a barrier.

Here's what that looks like in practice — every one of these is a barrier your graph inserts automatically, but you'd have to place by hand without one:

<div class="barrier-zoo-grid">
  <div class="bz-header">Barrier zoo — the transitions a real frame actually needs</div>
  <div class="bz-cards">
    <div class="bz-card">
      <div class="bz-card-head"><span class="bz-num">1</span> Render Target → Shader Read</div>
      <div class="bz-desc">GBuffer writes albedo → Lighting samples it</div>
      <div class="bz-tag bz-common">most common</div>
      <div class="bz-api"><span class="bz-vk">VK</span> COLOR_ATTACHMENT_OUTPUT → FRAGMENT_SHADER</div>
      <div class="bz-api"><span class="bz-dx">DX</span> RENDER_TARGET → PIXEL_SHADER_RESOURCE</div>
    </div>
    <div class="bz-card">
      <div class="bz-card-head"><span class="bz-num">2</span> Depth Write → Depth Read</div>
      <div class="bz-desc">Shadow pass writes depth → Lighting reads as texture</div>
      <div class="bz-tag bz-shadow">shadow sampling</div>
      <div class="bz-api"><span class="bz-vk">VK</span> LATE_FRAGMENT_TESTS → FRAGMENT_SHADER</div>
      <div class="bz-api"><span class="bz-dx">DX</span> DEPTH_WRITE → PIXEL_SHADER_RESOURCE</div>
    </div>
    <div class="bz-card">
      <div class="bz-card-head"><span class="bz-num">3</span> UAV Write → UAV Read</div>
      <div class="bz-desc">Bloom downsample mip N → reads it for mip N+1</div>
      <div class="bz-tag bz-compute">compute ping-pong</div>
      <div class="bz-api"><span class="bz-vk">VK</span> COMPUTE_SHADER (W) → COMPUTE_SHADER (R)</div>
      <div class="bz-api"><span class="bz-dx">DX</span> UAV barrier (flush compute caches)</div>
    </div>
    <div class="bz-card">
      <div class="bz-card-head"><span class="bz-num">4</span> Shader Read → Render Target</div>
      <div class="bz-desc">Lighting sampled HDR → Tonemap writes to it</div>
      <div class="bz-tag bz-reuse">resource reuse</div>
      <div class="bz-api"><span class="bz-vk">VK</span> FRAGMENT_SHADER → COLOR_ATTACHMENT_OUTPUT</div>
      <div class="bz-api"><span class="bz-dx">DX</span> PIXEL_SHADER_RESOURCE → RENDER_TARGET</div>
    </div>
    <div class="bz-card">
      <div class="bz-card-head"><span class="bz-num">5</span> Render Target → Present</div>
      <div class="bz-desc">Final composite → swapchain present</div>
      <div class="bz-tag bz-every">every frame</div>
      <div class="bz-api"><span class="bz-vk">VK</span> COLOR_ATTACHMENT_OUTPUT → BOTTOM_OF_PIPE</div>
      <div class="bz-api"><span class="bz-dx">DX</span> RENDER_TARGET → PRESENT</div>
    </div>
    <div class="bz-card">
      <div class="bz-card-head"><span class="bz-num">6</span> Aliasing Barrier</div>
      <div class="bz-desc">GBuffer dies → HDR reuses same physical memory</div>
      <div class="bz-tag bz-alias">memory aliasing</div>
      <div class="bz-api"><span class="bz-dx">DX</span> RESOURCE_BARRIER_TYPE_ALIASING</div>
      <div class="bz-api"><span class="bz-vk">VK</span> image layout UNDEFINED (discard)</div>
    </div>
  </div>
</div>

{{< interactive-barriers >}}

A 25-pass frame needs 30–50 of these. Miss one: corruption or device lost. Add a redundant one: GPU stall for nothing. Get the source/dest stages wrong: validation errors or subtle frame-order bugs that only show on a different vendor's driver. This is exactly why you don't want to place them by hand — the graph sees every read/write edge and emits the *exact* set.

**Diff from v1 — what changes in the code:**

We need four new pieces: (1) resource versioning with read/write tracking, (2) adjacency list for the DAG, (3) topological sort, (4) pass culling, and (5) barrier insertion. Additions marked with `// NEW v2` in the source:

{{< code-diff title="v1 → v2 — Key structural changes" >}}
@@ RenderPass struct @@
 struct RenderPass {
     std::string name;
     std::function<void()>             setup;
     std::function<void(/*cmd list*/)> execute;
+    std::vector<ResourceHandle> reads;     // NEW v2
+    std::vector<ResourceHandle> writes;    // NEW v2
+    std::vector<uint32_t> dependsOn;       // NEW v2
+    std::vector<uint32_t> successors;      // NEW v2
+    uint32_t inDegree = 0;                 // NEW v2
+    bool     alive    = false;             // NEW v2
 };

@@ FrameGraph class — new methods @@
+    void read(uint32_t passIdx, ResourceHandle h);  // link to resource version
+    void write(uint32_t passIdx, ResourceHandle h);  // create new version

@@ FrameGraph::execute() @@
-    // v1: just run every pass in declaration order.
-    for (auto& pass : passes_)
-        pass.execute();
+    // v2: build edges, topo-sort, cull, then run in sorted order.
+    buildEdges();
+    auto sorted = topoSort();   // Kahn's algorithm — O(V+E)
+    cull(sorted);               // backward walk from output
+    for (uint32_t idx : sorted) {
+        if (!passes_[idx].alive) continue;  // skip dead
+        insertBarriers(idx);                // auto barriers
+        passes_[idx].execute();
+    }

@@ New internal data @@
-    std::vector<ResourceDesc>  resources_;
+    std::vector<ResourceEntry> entries_;  // now with versioning
{{< /code-diff >}}

Full updated source ([frame_graph_v2.h](frame_graph_v2.h)):

{{< include-code file="frame_graph_v2.h" lang="cpp" >}}

**Usage — same passes, now with declared dependencies** ([example_v2.cpp](example_v2.cpp)):

{{< include-code file="example_v2.cpp" lang="cpp" compile="true" deps="frame_graph_v2.h" >}}

Remove GBuffer's read of `depth` and the depth prepass gets culled automatically — no `#ifdef`, no flag. That's the graph working for you.

<div style="margin:1.5em 0 .8em;padding:.8em 1.2em;border-radius:10px;background:linear-gradient(135deg,rgba(139,92,246,.08),rgba(59,130,246,.08));border:2px solid rgba(139,92,246,.2);position:relative;">
<div style="font-size:1.1em;font-weight:800;margin-bottom:.3em;">🎯 See culling in action</div>
<div style="font-size:.92em;line-height:1.6;color:var(--color-neutral-700,#374151);">Compare these two variants — identical pipeline, but in variant B, Lighting doesn't read SSAO's output. The graph automatically <strong>culls the dead pass</strong> and releases its resources:</div>
</div>

{{< compile-compare fileA="example_v2_ssao_alive.cpp" fileB="example_v2_ssao_dead.cpp" labelA="SSAO Connected (alive)" labelB="SSAO Disconnected (culled)" deps="frame_graph_v2.h" >}}

<div style="display:grid;grid-template-columns:1fr 1fr;gap:.8em;margin:1em 0;">
  <div style="padding:.7em 1em;border-radius:8px;border-left:4px solid #22c55e;background:rgba(34,197,94,.05);font-size:.9em;line-height:1.5;">
    <strong style="color:#22c55e;">✓ What it proves</strong><br>
    Automatic barriers from declared dependencies. Pass reordering is safe. Dead passes are culled. Three of the four intro promises delivered.
  </div>
  <div style="padding:.7em 1em;border-radius:8px;border-left:4px solid #ef4444;background:rgba(239,68,68,.05);font-size:.9em;line-height:1.5;">
    <strong style="color:#ef4444;">✗ What it lacks</strong><br>
    Resources still live for the entire frame. Version 3 adds lifetime analysis and memory aliasing.
  </div>
</div>

<div style="margin:1.2em 0;padding:.8em 1em;border-radius:8px;background:rgba(139,92,246,.05);border:1px solid rgba(139,92,246,.15);font-size:.9em;line-height:1.6;">
<strong style="color:#8b5cf6;">🎮 UE5 does exactly this.</strong> When you call <code>FRDGBuilder::AddPass</code> with <code>ERDGPassFlags::Raster</code> or <code>ERDGPassFlags::Compute</code>, RDG builds the same dependency graph from your declared reads/writes, topologically sorts it, culls dead passes, and inserts barriers — all before recording a single GPU command.
</div>

<div style="margin:1em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid #f59e0b;background:rgba(245,158,11,.05);font-size:.9em;line-height:1.6;">
⚠️ <strong>Caveat:</strong> UE5's migration to RDG is <em>incomplete</em>. Large parts of the renderer still use legacy immediate-mode <code>FRHICommandList</code> calls outside the graph. These "untracked" resources bypass RDG's barrier and aliasing systems entirely — you get the graph's benefits only for passes that have been ported. Legacy passes still need manual barriers at the RDG boundary. This is the cost of retrofitting a graph onto a 25-year-old codebase.
</div>

---

## MVP v3 — Lifetimes & Aliasing

<div style="margin:1em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid #22c55e;background:rgba(34,197,94,.04);font-size:.92em;line-height:1.6;">
🎯 <strong>Goal:</strong> The final piece — 30–50% VRAM savings by reusing physical memory across transient resources whose lifetimes don't overlap.
</div>

<div style="display:grid;grid-template-columns:1fr 1fr 1fr;gap:.7em;margin:1.2em 0;">
  <div style="border-radius:8px;border:1px solid var(--color-neutral-300,#d4d4d4);padding:.7em .8em;font-size:.88em;line-height:1.6;">
    <div style="font-weight:700;color:#3b82f6;margin-bottom:.3em;">1. First/last use</div>
    Walk sorted passes. For each transient resource, record <code>firstUsePass</code> and <code>lastUsePass</code>. Imported resources excluded.
  </div>
  <div style="border-radius:8px;border:1px solid var(--color-neutral-300,#d4d4d4);padding:.7em .8em;font-size:.88em;line-height:1.6;">
    <div style="font-weight:700;color:#8b5cf6;margin-bottom:.3em;">2. Refcount</div>
    Increment at first use, decrement at last use. When refcount hits zero, physical memory is eligible for reuse.
  </div>
  <div style="border-radius:8px;border:1px solid var(--color-neutral-300,#d4d4d4);padding:.7em .8em;font-size:.88em;line-height:1.6;">
    <div style="font-weight:700;color:#22c55e;margin-bottom:.3em;">3. Free-list scan</div>
    Sort by first-use, scan for compatible physical allocation. Fit → reuse. No fit → allocate. <strong>Greedy interval-coloring.</strong>
  </div>
</div>

<div class="diagram-compat">
  <div class="dc-box">
    <strong>Candidate block</strong><br>
    <span class="dc-check">✓?</span> device-local<br>
    <span class="dc-check">✓?</span> 16 MB<br>
    <span class="dc-check">✓?</span> avail after P4
  </div>
  <div class="dc-box">
    <strong>New resource</strong><br>
    same memory type?<br>
    8 MB needed<br>
    first use at P5
  </div>
  <div class="dc-arrow">→</div>
  <div class="dc-result">all three ✓ → reuse!</div>
</div>

This is the same approach Frostbite described at GDC 2017.

**How aliasing works at the API level:** Two virtual resources sharing physical memory requires **placed resources** — GPU memory allocated from a heap, with resources bound to offsets within it.

<div style="display:grid;grid-template-columns:1fr 1fr;gap:.8em;margin:1em 0;">
  <div style="border-radius:8px;border:1.5px solid rgba(59,130,246,.3);padding:.7em .9em;font-size:.88em;line-height:1.6;">
    <div style="font-weight:700;color:#3b82f6;margin-bottom:.3em;">D3D12</div>
    Create <code>ID3D12Heap</code> (e.g., 64 MB device-local). Call <code>CreatePlacedResource</code> to bind each virtual resource at an offset. Two resources with non-overlapping lifetimes share the heap.
  </div>
  <div style="border-radius:8px;border:1.5px solid rgba(139,92,246,.3);padding:.7em .9em;font-size:.88em;line-height:1.6;">
    <div style="font-weight:700;color:#8b5cf6;margin-bottom:.3em;">Vulkan</div>
    Allocate <code>VkDeviceMemory</code>. Create <code>VkImage</code>/<code>VkBuffer</code> normally, then call <code>vkBindImageMemory</code> pointing into the same memory block at different offsets.
  </div>
</div>

<div style="margin:.8em 0;padding:.5em .8em;border-radius:6px;background:rgba(245,158,11,.05);border:1px solid rgba(245,158,11,.15);font-size:.88em;">
⚙️ Without placed resources, each <code>CreateCommittedResource</code> (D3D12) or dedicated allocation (Vulkan) gets its own memory — aliasing is impossible. This is why the graph's allocator works with heaps.
</div>

**Worked example** (1080p deferred pipeline):

| Virtual Resource | Format | Size | Lifetime (passes) |
|-----------------|--------|------|--------------------|
| GBuffer Albedo | RGBA8 | 8 MB | 2–4 |
| GBuffer Normals | RGB10A2 | 8 MB | 2–4 |
| SSAO Scratch | R8 | 2 MB | 3–4 |
| SSAO Result | R8 | 2 MB | 4–5 |
| HDR Lighting | RGBA16F | 16 MB | 5–6 |
| Bloom Scratch | RGBA16F | 16 MB | 6–7 |

Without aliasing: 52 MB. With aliasing: GBuffer Albedo and HDR Lighting share one 16 MB block (lifetimes don't overlap). GBuffer Normals and Bloom Scratch share another. SSAO Scratch and SSAO Result share a third.

<div style="margin:.8em 0;padding:.6em 1em;border-radius:8px;background:linear-gradient(135deg,rgba(34,197,94,.06),rgba(59,130,246,.06));border:1px solid rgba(34,197,94,.2);font-size:.92em;font-weight:600;text-align:center;">
💾 Physical memory: 36 MB — <span style="color:#22c55e;">31% saved</span>. In complex pipelines: <span style="color:#22c55e;">40–50% savings</span>.
</div>

{{< interactive-aliasing >}}

**Diff from v2 — lifetime tracking and free-list allocator:**

Two additions to the `FrameGraph` class: (1) a lifetime scan that records each transient resource's first and last use in the sorted pass order, and (2) a greedy free-list allocator that reuses physical blocks when lifetimes don't overlap.

{{< code-diff title="v2 → v3 — Key additions for lifetime analysis & aliasing" >}}
@@ New structs @@
+struct PhysicalBlock {              // physical memory slot
+    uint32_t sizeBytes  = 0;
+    Format   format     = Format::RGBA8;
+    uint32_t availAfter = 0;        // free after this pass
+};
+
+struct Lifetime {                   // per-resource timing
+    uint32_t firstUse = UINT32_MAX;
+    uint32_t lastUse  = 0;
+    bool     isTransient = true;
+};

@@ FrameGraph::execute() @@
     auto sorted = topoSort();
     cull(sorted);
+    auto lifetimes = scanLifetimes(sorted);     // NEW v3
+    auto mapping   = aliasResources(lifetimes); // NEW v3
     // ... existing barrier + execute loop ...

@@ scanLifetimes() — walk sorted passes, record first/last use @@
+    for (uint32_t order = 0; order < sorted.size(); order++) {
+        for (auto& h : passes_[sorted[order]].reads) {
+            life[h.index].firstUse = min(life[h.index].firstUse, order);
+            life[h.index].lastUse  = max(life[h.index].lastUse,  order);
+        }
+        // ... same for writes ...
+    }

@@ aliasResources() — greedy free-list scan @@
+    // sort resources by firstUse, then scan free list:
+    for (uint32_t resIdx : indices) {
+        for (uint32_t b = 0; b < freeList.size(); b++) {
+            if (freeList[b].availAfter < firstUse && sizeOK) {
+                mapping[resIdx] = b;  // reuse!
+                break;
+            }
+        }
+        if (!reused) freeList.push_back(newBlock); // allocate
+    }
{{< /code-diff >}}

The complete v3 file integrates all v2 code plus lifetime analysis and aliasing into a single compilable header ([frame_graph_v3.h](frame_graph_v3.h)):

{{< include-code file="frame_graph_v3.h" lang="cpp" >}}

**Usage** — lifetime analysis and aliasing in action ([example_v3.cpp](example_v3.cpp)):

{{< include-code file="example_v3.cpp" lang="cpp" compile="true" deps="frame_graph_v3.h" >}}

That's ~70 new lines on top of v2. The aliasing runs once per frame in O(R log R) — sort the resources, then a linear scan of the free list. For 15 transient resources this is sub-microsecond.

<div style="display:flex;gap:1em;flex-wrap:wrap;margin:1.2em 0">
  <div style="flex:1;min-width:240px;background:linear-gradient(135deg,rgba(34,197,94,.08),rgba(34,197,94,.03));border:1px solid rgba(34,197,94,.25);border-radius:10px;padding:1em 1.2em">
    <div style="font-weight:700;color:#22c55e;margin-bottom:.4em">✓ What v3 proves</div>
    <div style="font-size:.92em;line-height:1.5">The full value prop — automatic memory aliasing <em>and</em> automatic barriers. Feature-equivalent to Frostbite's 2017 GDC demo (minus async compute).</div>
  </div>
  <div style="flex:1;min-width:240px;background:linear-gradient(135deg,rgba(239,68,68,.08),rgba(239,68,68,.03));border:1px solid rgba(239,68,68,.25);border-radius:10px;padding:1em 1.2em">
    <div style="font-weight:700;color:#ef4444;margin-bottom:.4em">✗ Still missing</div>
    <div style="font-size:.92em;line-height:1.5">Async compute, split barriers, pass merging, parallel recording. These are production features — covered in <a href="/posts/frame-graph-production/">Part III</a>.</div>
  </div>
</div>

<div style="border-left:4px solid #8b5cf6;background:linear-gradient(135deg,rgba(139,92,246,.06),transparent);border-radius:0 10px 10px 0;padding:1em 1.2em;margin:1em 0">
  <div style="font-weight:700;color:#8b5cf6;font-size:.9em;margin-bottom:.3em">🔮 UE5 equivalent</div>
  <div style="font-size:.9em;line-height:1.55">This is handled by the <strong>transient resource allocator</strong>. Any <code>FRDGTexture</code> created through <code>FRDGBuilder::CreateTexture</code> (vs <code>RegisterExternalTexture</code>) is transient and eligible for aliasing. The RDG compiler runs the same lifetime analysis and free-list scan we just built.</div>
</div>

<div style="border-left:4px solid #f59e0b;background:linear-gradient(135deg,rgba(245,158,11,.06),transparent);border-radius:0 10px 10px 0;padding:1em 1.2em;margin:1em 0">
  <div style="font-weight:700;color:#f59e0b;font-size:.9em;margin-bottom:.3em">⚠ Limitation</div>
  <div style="font-size:.9em;line-height:1.55">UE5 only aliases <em>transient</em> resources. Imported resources — even with fully known lifetimes — are never aliased. Frostbite was more aggressive, aliasing across a broader set by tracking GPU-timeline lifetimes. If your renderer has large imported resources with predictable per-frame usage, UE5's approach leaves VRAM on the table.</div>
</div>

---

## A Real Frame

**Deferred Pipeline**

Depth prepass → GBuffer → SSAO → Lighting → Tonemap → Present

<div class="diagram-flow" style="justify-content:center;flex-wrap:wrap">
  <div class="df-step df-primary">Depth<span class="df-sub">depth (T)</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-primary">GBuf<span class="df-sub">albedo (T) · norm (T)</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-primary">SSAO<span class="df-sub">scratch (T) · result (T)</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-primary">Lighting<span class="df-sub">HDR (T)</span></div>
  <div class="df-arrow"></div>
  <div class="df-step">Tonemap</div>
  <div class="df-arrow"></div>
  <div class="df-step df-success">Present<span class="df-sub">backbuffer (imported)</span></div>
</div>
<div style="text-align:center;font-size:.75em;opacity:.5;margin-top:-.3em">(T) = transient — aliased by graph &nbsp;&nbsp;&nbsp; (imported) = owned externally</div>

<div style="display:flex;gap:1em;flex-wrap:wrap;margin:1em 0">
  <div style="flex:1;min-width:220px;border:1px solid rgba(59,130,246,.3);border-radius:10px;padding:1em 1.2em;background:linear-gradient(135deg,rgba(59,130,246,.06),transparent)">
    <div style="font-weight:700;color:#3b82f6;margin-bottom:.4em">⚡ Transient</div>
    <div style="font-size:.9em;line-height:1.55">GBuffer MRTs, SSAO scratch, HDR lighting buffer, bloom scratch — created and destroyed within this frame. <strong>Aliased by the graph.</strong></div>
  </div>
  <div style="flex:1;min-width:220px;border:1px solid rgba(139,92,246,.3);border-radius:10px;padding:1em 1.2em;background:linear-gradient(135deg,rgba(139,92,246,.06),transparent)">
    <div style="font-weight:700;color:#8b5cf6;margin-bottom:.4em">📌 Imported</div>
    <div style="font-size:.9em;line-height:1.55">Backbuffer (swapchain), TAA history (read last frame → written this frame), shadow atlas (persistent, updated incrementally). <strong>Graph tracks barriers but doesn't own their memory.</strong></div>
  </div>
</div>

**Forward Pipeline**

<div class="diagram-flow" style="justify-content:center;flex-wrap:wrap">
  <div class="df-step df-primary">Depth<span class="df-sub">depth (T)</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-primary">Forward + MSAA<span class="df-sub">color MSAA (T)</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-primary">Resolve<span class="df-sub">color (T)</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-primary">PostProc<span class="df-sub">HDR (T)</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-success">Present<span class="df-sub">backbuffer (imported)</span></div>
</div>
<div style="text-align:center;font-size:.75em;opacity:.5;margin-top:-.3em">Fewer passes, fewer transient resources → less aliasing opportunity. Same API, same automatic barriers.</div>

**Side-by-side**

<div style="overflow-x:auto;margin:1em 0">
<table style="width:100%;border-collapse:collapse;border-radius:10px;overflow:hidden;font-size:.92em">
  <thead>
    <tr style="background:linear-gradient(135deg,rgba(59,130,246,.12),rgba(139,92,246,.1))">
      <th style="padding:.7em 1em;text-align:left;border-bottom:2px solid rgba(59,130,246,.2)">Aspect</th>
      <th style="padding:.7em 1em;text-align:center;border-bottom:2px solid rgba(59,130,246,.2);color:#3b82f6">Deferred</th>
      <th style="padding:.7em 1em;text-align:center;border-bottom:2px solid rgba(139,92,246,.2);color:#8b5cf6">Forward</th>
    </tr>
  </thead>
  <tbody>
    <tr><td style="padding:.5em 1em">Passes</td><td style="padding:.5em 1em;text-align:center">6</td><td style="padding:.5em 1em;text-align:center">5</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.5em 1em">Peak VRAM (no aliasing)</td><td style="padding:.5em 1em;text-align:center">X MB</td><td style="padding:.5em 1em;text-align:center">Y MB</td></tr>
    <tr><td style="padding:.5em 1em">Peak VRAM (with aliasing)</td><td style="padding:.5em 1em;text-align:center">0.6X MB</td><td style="padding:.5em 1em;text-align:center">0.75Y MB</td></tr>
    <tr style="background:linear-gradient(90deg,rgba(34,197,94,.08),rgba(34,197,94,.04))"><td style="padding:.5em 1em;font-weight:700">VRAM saved by aliasing</td><td style="padding:.5em 1em;text-align:center;font-weight:700;color:#22c55e;font-size:1.1em">40%</td><td style="padding:.5em 1em;text-align:center;font-weight:700;color:#22c55e;font-size:1.1em">25%</td></tr>
    <tr><td style="padding:.5em 1em">Barriers auto-inserted</td><td style="padding:.5em 1em;text-align:center">8</td><td style="padding:.5em 1em;text-align:center">5</td></tr>
  </tbody>
</table>
</div>

**What about CPU cost?** Every phase is linear-time:

<div style="overflow-x:auto;margin:1em 0">
<table style="width:100%;border-collapse:collapse;font-size:.9em">
  <thead>
    <tr>
      <th style="padding:.6em 1em;text-align:left;border-bottom:2px solid rgba(34,197,94,.3);color:#22c55e">Phase</th>
      <th style="padding:.6em 1em;text-align:center;border-bottom:2px solid rgba(34,197,94,.3)">Complexity</th>
      <th style="padding:.6em 1em;text-align:left;border-bottom:2px solid rgba(34,197,94,.3)">Notes</th>
    </tr>
  </thead>
  <tbody>
    <tr><td style="padding:.45em 1em;font-weight:600">Topological sort</td><td style="padding:.45em 1em;text-align:center;font-family:ui-monospace,monospace;color:#22c55e">O(V + E)</td><td style="padding:.45em 1em;font-size:.9em;opacity:.8">Kahn's algorithm — passes + edges</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.45em 1em;font-weight:600">Pass culling</td><td style="padding:.45em 1em;text-align:center;font-family:ui-monospace,monospace;color:#22c55e">O(V + E)</td><td style="padding:.45em 1em;font-size:.9em;opacity:.8">Backward reachability from output</td></tr>
    <tr><td style="padding:.45em 1em;font-weight:600">Lifetime scan</td><td style="padding:.45em 1em;text-align:center;font-family:ui-monospace,monospace;color:#22c55e">O(V)</td><td style="padding:.45em 1em;font-size:.9em;opacity:.8">Single pass over sorted list</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.45em 1em;font-weight:600">Aliasing</td><td style="padding:.45em 1em;text-align:center;font-family:ui-monospace,monospace;color:#22c55e">O(R log R)</td><td style="padding:.45em 1em;font-size:.9em;opacity:.8">Sort by first-use, then O(R) free-list scan</td></tr>
    <tr><td style="padding:.45em 1em;font-weight:600">Barrier insertion</td><td style="padding:.45em 1em;text-align:center;font-family:ui-monospace,monospace;color:#22c55e">O(V)</td><td style="padding:.45em 1em;font-size:.9em;opacity:.8">Linear scan with state lookup</td></tr>
  </tbody>
</table>
</div>

<div style="font-size:.88em;line-height:1.5;opacity:.75;margin:-.3em 0 1em 0">Where V = passes (~25), E = dependency edges (~50), R = transient resources (~15). Everything is linear or near-linear. All data fits in L1 cache — the entire compile is well under 0.1 ms.</div>

<div style="border-left:4px solid #6366f1;background:linear-gradient(135deg,rgba(99,102,241,.08),transparent);border-radius:0 10px 10px 0;padding:1.1em 1.3em;margin:1.5em 0;font-size:1.05em;font-style:italic;line-height:1.5">
The graph doesn't care about your rendering <em>strategy</em>. It cares about your <em>dependencies</em>. That's the whole point.
</div>

---

<div style="margin:2em 0 0;padding:1em 1.2em;border-radius:10px;border:1px solid rgba(99,102,241,.2);background:rgba(99,102,241,.03);display:flex;justify-content:space-between;align-items:center;">
  <a href="/posts/frame-graph-theory/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    ← Previous: Part I — Theory
  </a>
  <a href="/posts/frame-graph-production/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    Next: Part III — Production Engines →
  </a>
</div>
