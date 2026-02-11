---
title: "Frame Graph — Build It"
date: 2026-02-10
draft: false
description: "Three iterations from blank file to working frame graph with automatic barriers and memory aliasing."
tags: ["rendering", "frame-graph", "gpu", "architecture", "cpp"]
categories: ["analysis"]
series: ["Rendering Architecture"]
showTableOfContents: false
---

{{< article-nav >}}

<div style="margin:0 0 1.5em;padding:.7em 1em;border-radius:8px;background:rgba(99,102,241,.04);border:1px solid rgba(99,102,241,.12);font-size:.88em;line-height:1.6;opacity:.85;">
📖 <strong>Part II of III.</strong>&ensp; <a href="/posts/frame-graph-theory/">Theory</a> → <em>Build It</em> → <a href="/posts/frame-graph-production/">Production Engines</a>
</div>

*Three iterations from blank file to working frame graph with automatic barriers and memory aliasing. Each version builds on the last — by the end you'll have something you can drop into a real renderer.*

[Part I](/posts/frame-graph-theory/) covered what a frame graph is — the three-phase lifecycle (declare → compile → execute), the DAG, and why every major engine uses one. Now we implement it. Three C++ iterations, each adding a layer: v1 is scaffolding, v2 adds the dependency graph with automatic barriers and pass culling, v3 adds lifetime analysis and memory aliasing.

---

## 🏗️ API Design

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
    <div class="dph-title" style="color:#3b82f6">① Declaration <span style="font-weight:400;font-size:.75em;opacity:.7;">CPU</span></div>
    <div class="dph-body">
      <code>addPass(setup, execute)</code><br>
      ├ setup lambda runs<br>
      &nbsp;&nbsp;• declare reads / writes<br>
      &nbsp;&nbsp;• request resources<br>
      └ <strong>no GPU work, no allocation</strong>
      <div style="margin-top:.6em;padding:.35em .6em;border-radius:5px;background:rgba(59,130,246,.08);font-size:.82em;line-height:1.4;border:1px solid rgba(59,130,246,.12);">
        Resources are <strong>virtual</strong> — just a description + handle index. Zero bytes allocated.
      </div>
    </div>
  </div>
  <div class="dph-col" style="border-color:#8b5cf6">
    <div class="dph-title" style="color:#8b5cf6">② Compile <span style="font-weight:400;font-size:.75em;opacity:.7;">CPU</span></div>
    <div class="dph-body">
      ├ <strong>sort</strong> — topo order (Kahn's)<br>
      ├ <strong>cull</strong> — remove dead passes<br>
      ├ <strong>alias</strong> — map virtual → physical<br>
      └ <strong>barrier</strong> — emit transitions
      <div style="margin-top:.6em;padding:.35em .6em;border-radius:5px;background:rgba(139,92,246,.08);font-size:.82em;line-height:1.4;border:1px solid rgba(139,92,246,.12);">
        Aliasing <strong>planned</strong> here — non-overlapping lifetimes will share the same heap.
      </div>
    </div>
  </div>
  <div class="dph-col" style="border-color:#22c55e">
    <div class="dph-title" style="color:#22c55e">③ Execute <span style="font-weight:400;font-size:.75em;opacity:.7;">GPU</span></div>
    <div class="dph-body">
      for each pass in sorted order:<br>
      ├ insert pre-computed barriers<br>
      └ call execute lambda<br>
      &nbsp;&nbsp;→ draw / dispatch / copy
      <div style="margin-top:.6em;padding:.35em .6em;border-radius:5px;background:rgba(34,197,94,.08);font-size:.82em;line-height:1.4;border:1px solid rgba(34,197,94,.12);">
        Lambdas see a <strong>fully resolved</strong> environment — memory bound, barriers placed, resources ready.
      </div>
    </div>
  </div>
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

## 🧱 MVP v1 — Declare & Execute

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

{{< include-code file="frame_graph_v1.h" lang="cpp" compact="true" >}}
{{< include-code file="example_v1.cpp" lang="cpp" compile="true" deps="frame_graph_v1.h" compact="true" >}}

Compiles and runs — the execute lambdas are stubs, but the scaffolding is real. Every piece we add in v2 and v3 goes into this same `FrameGraph` class.

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

## 🔗 MVP v2 — Dependencies & Barriers

<div style="margin:1em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid #3b82f6;background:rgba(59,130,246,.04);font-size:.92em;line-height:1.6;">
🎯 <strong>Goal:</strong> Automatic pass ordering, dead-pass culling, and barrier insertion — the core value of a render graph.
</div>

Four pieces, each feeding the next:

<div style="margin:.8em 0 1.2em;display:grid;grid-template-columns:repeat(4,1fr);gap:0;border-radius:10px;overflow:hidden;border:1.5px solid rgba(99,102,241,.2);">
  <a href="#v2-versioning" style="padding:.7em .6em .5em;background:rgba(59,130,246,.05);border-right:1px solid rgba(99,102,241,.12);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(59,130,246,.12)'" onmouseout="this.style.background='rgba(59,130,246,.05)'">
    <div style="font-size:1.2em;margin-bottom:.15em;">🔀</div>
    <div style="font-weight:800;font-size:.85em;color:#3b82f6;">Versioning</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">reads/writes → edges</div>
  </a>
  <a href="#v2-toposort" style="padding:.7em .6em .5em;background:rgba(139,92,246,.05);border-right:1px solid rgba(99,102,241,.12);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(139,92,246,.12)'" onmouseout="this.style.background='rgba(139,92,246,.05)'">
    <div style="font-size:1.2em;margin-bottom:.15em;">📦</div>
    <div style="font-weight:800;font-size:.85em;color:#8b5cf6;">Topo Sort</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">edges → execution order</div>
  </a>
  <a href="#v2-culling" style="padding:.7em .6em .5em;background:rgba(245,158,11,.05);border-right:1px solid rgba(99,102,241,.12);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(245,158,11,.12)'" onmouseout="this.style.background='rgba(245,158,11,.05)'">
    <div style="font-size:1.2em;margin-bottom:.15em;">✂️</div>
    <div style="font-weight:800;font-size:.85em;color:#f59e0b;">Pass Culling</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">kill unreachable passes</div>
  </a>
  <a href="#v2-barriers" style="padding:.7em .6em .5em;background:rgba(239,68,68,.05);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(239,68,68,.12)'" onmouseout="this.style.background='rgba(239,68,68,.05)'">
    <div style="font-size:1.2em;margin-bottom:.15em;">🚧</div>
    <div style="font-weight:800;font-size:.85em;color:#ef4444;">Barriers</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">emit GPU transitions</div>
  </a>
</div>

<span id="v2-versioning"></span>

### 🔀 Resource versioning & the dependency graph

Multiple passes can read the same resource without conflict — but when a pass *writes* to it, every later reader needs to know which write they depend on. The solution: each write bumps the resource's **version number**. Readers attach to the version that existed when they were declared, so dependency edges stay precise even when the same resource is written multiple times per frame.

<div style="margin:1.2em 0;font-size:.85em;">
  <div style="border-radius:10px;overflow:hidden;border:1.5px solid rgba(99,102,241,.15);">
    <div style="padding:.5em .8em;background:rgba(99,102,241,.06);border-bottom:1px solid rgba(99,102,241,.1);font-weight:700;font-size:.9em;text-align:center;">Pixel History — HDR target through the frame</div>
    <div style="display:grid;grid-template-columns:auto auto 1fr;gap:0;">
      <div style="padding:.45em .6em;background:rgba(59,130,246,.06);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.08);font-weight:700;text-align:center;color:#3b82f6;font-size:.82em;">v1</div>
      <div style="padding:.45em .6em;background:rgba(59,130,246,.12);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.08);font-weight:700;text-align:center;color:#3b82f6;font-size:.75em;">WRITE</div>
      <div style="padding:.45em .8em;border-bottom:1px solid rgba(99,102,241,.08);font-size:.86em;">
        <span style="font-weight:700;">Lighting</span> — renders lit color into HDR target
      </div>
      <div style="padding:.35em .6em;background:rgba(59,130,246,.03);border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.7em;opacity:.4;text-align:center;">v1</div>
      <div style="padding:.35em .6em;background:rgba(139,92,246,.08);border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-weight:600;text-align:center;color:#8b5cf6;font-size:.75em;">read</div>
      <div style="padding:.35em .8em;border-bottom:1px solid rgba(99,102,241,.06);font-size:.84em;opacity:.85;">
        <span style="font-weight:600;">Bloom</span> — samples bright pixels <span style="opacity:.4;font-size:.88em;">(still v1)</span>
      </div>
      <div style="padding:.35em .6em;background:rgba(59,130,246,.03);border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.7em;opacity:.4;text-align:center;">v1</div>
      <div style="padding:.35em .6em;background:rgba(139,92,246,.08);border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-weight:600;text-align:center;color:#8b5cf6;font-size:.75em;">read</div>
      <div style="padding:.35em .8em;border-bottom:1px solid rgba(99,102,241,.06);font-size:.84em;opacity:.85;">
        <span style="font-weight:600;">Reflections</span> — samples for SSR <span style="opacity:.4;font-size:.88em;">(still v1)</span>
      </div>
      <div style="padding:.35em .6em;background:rgba(59,130,246,.03);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.08);font-size:.7em;opacity:.4;text-align:center;">v1</div>
      <div style="padding:.35em .6em;background:rgba(139,92,246,.08);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.08);font-weight:600;text-align:center;color:#8b5cf6;font-size:.75em;">read</div>
      <div style="padding:.35em .8em;border-bottom:1px solid rgba(99,102,241,.08);font-size:.84em;opacity:.85;">
        <span style="font-weight:600;">Fog</span> — reads scene color for aerial blending <span style="opacity:.4;font-size:.88em;">(still v1)</span>
      </div>
      <div style="padding:.45em .6em;background:rgba(34,197,94,.06);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.08);font-weight:700;text-align:center;color:#22c55e;font-size:.82em;">v2</div>
      <div style="padding:.45em .6em;background:rgba(34,197,94,.12);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.08);font-weight:700;text-align:center;color:#22c55e;font-size:.75em;">WRITE</div>
      <div style="padding:.45em .8em;border-bottom:1px solid rgba(99,102,241,.08);font-size:.86em;">
        <span style="font-weight:700;">Composite</span> — overwrites with final blended result <span style="opacity:.4;font-size:.88em;">(bumps to v2)</span>
      </div>
      <div style="padding:.35em .6em;background:rgba(34,197,94,.03);border-right:1px solid rgba(99,102,241,.08);font-size:.7em;opacity:.4;text-align:center;">v2</div>
      <div style="padding:.35em .6em;background:rgba(139,92,246,.08);border-right:1px solid rgba(99,102,241,.08);font-weight:600;text-align:center;color:#8b5cf6;font-size:.75em;">read</div>
      <div style="padding:.35em .8em;font-size:.84em;opacity:.85;">
        <span style="font-weight:600;">Tonemap</span> — maps HDR → SDR for display <span style="opacity:.4;font-size:.88em;">(reads v2, not v1)</span>
      </div>
    </div>
  </div>
  <div style="margin-top:.4em;font-size:.82em;opacity:.6;">Reads never bump the version — three passes read v1 without conflict. Only a write creates v2. Tonemap depends on Composite (v2 writer), with <strong>no edge</strong> to Lighting or any v1 reader.</div>
</div>

---

<span id="v2-toposort"></span>

### 📊 Topological sort (Kahn's algorithm)

Count incoming edges per pass. Any pass with zero incoming edges has all dependencies satisfied — emit it, decrement its neighbors' counts, repeat until the queue is empty. If the output is shorter than the pass count, the graph has a cycle.

{{< interactive-toposort >}}

---

<span id="v2-culling"></span>

### ✂️ Pass culling

<div style="display:grid;grid-template-columns:auto 1fr;gap:.6em .9em;align-items:start;margin:.8em 0 1.2em;padding:.8em 1em;border-radius:10px;background:linear-gradient(135deg,rgba(245,158,11,.06),transparent);border:1px solid rgba(245,158,11,.18);font-size:.9em;line-height:1.6;">
  <span style="font-size:1.3em;line-height:1;">🔙</span>
  <span><strong>Algorithm:</strong> Walk backwards from the final output (present / backbuffer). Mark every reachable pass as <em>alive</em>.</span>
  <span style="font-size:1.3em;line-height:1;">💀</span>
  <span><strong>Result:</strong> Any unmarked pass is dead — removed along with all its resource declarations. No <code>#ifdef</code>, no flag.</span>
  <span style="font-size:1.3em;line-height:1;">⏱️</span>
  <span><strong>Cost:</strong> O(V + E) — one linear walk over the graph.</span>
</div>

Toggle edges in the DAG to see it live — disconnect a pass and the compiler removes it along with its resources. No `#ifdef`, no feature flag — just a missing edge.

{{< interactive-dag >}}

---

<span id="v2-barriers"></span>

### 🚧 Barrier insertion

A GPU resource can't be a render target and a shader input at the same time — the hardware needs to flush caches, change memory layout, and switch access modes between those uses. That transition is a **barrier**.

The graph already knows the sorted pass order and what each pass reads or writes. So for every resource handoff — GBuffer goes from "being written by pass A" to "being read by pass B" — it inserts the correct barrier automatically. Here's every type of barrier a real frame needs:

<div style="overflow-x:auto;margin:1em 0;">
<table style="width:100%;border-collapse:collapse;border-radius:10px;overflow:hidden;font-size:.88em;">
  <thead>
    <tr style="background:linear-gradient(135deg,rgba(99,102,241,.1),rgba(59,130,246,.08));">
      <th style="padding:.6em .8em;text-align:left;border-bottom:2px solid rgba(99,102,241,.2);width:28%;">Transition</th>
      <th style="padding:.6em .8em;text-align:left;border-bottom:2px solid rgba(99,102,241,.2);width:30%;">Example</th>
      <th style="padding:.6em .8em;text-align:left;border-bottom:2px solid rgba(99,102,241,.2);">API</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="padding:.5em .8em;font-weight:600;">Render Target → Shader Read</td>
      <td style="padding:.5em .8em;font-size:.9em;opacity:.8;">GBuffer → Lighting samples it</td>
      <td style="padding:.5em .8em;font-size:.82em;font-family:ui-monospace,monospace;line-height:1.6;"><span style="color:#ef4444;">VK</span> COLOR_ATTACHMENT → FRAGMENT_SHADER<br><span style="color:#3b82f6;">DX</span> RENDER_TARGET → PIXEL_SHADER_RESOURCE</td>
    </tr>
    <tr style="background:rgba(127,127,127,.03);">
      <td style="padding:.5em .8em;font-weight:600;">Depth Write → Depth Read</td>
      <td style="padding:.5em .8em;font-size:.9em;opacity:.8;">Shadows → Lighting reads as texture</td>
      <td style="padding:.5em .8em;font-size:.82em;font-family:ui-monospace,monospace;line-height:1.6;"><span style="color:#ef4444;">VK</span> LATE_FRAGMENT_TESTS → FRAGMENT_SHADER<br><span style="color:#3b82f6;">DX</span> DEPTH_WRITE → PIXEL_SHADER_RESOURCE</td>
    </tr>
    <tr>
      <td style="padding:.5em .8em;font-weight:600;">UAV Write → UAV Read</td>
      <td style="padding:.5em .8em;font-size:.9em;opacity:.8;">Bloom mip N → mip N+1</td>
      <td style="padding:.5em .8em;font-size:.82em;font-family:ui-monospace,monospace;line-height:1.6;"><span style="color:#ef4444;">VK</span> COMPUTE_SHADER (W) → COMPUTE_SHADER (R)<br><span style="color:#3b82f6;">DX</span> UAV barrier (flush caches)</td>
    </tr>
    <tr style="background:rgba(127,127,127,.03);">
      <td style="padding:.5em .8em;font-weight:600;">Shader Read → Render Target</td>
      <td style="padding:.5em .8em;font-size:.9em;opacity:.8;">Lighting read HDR → Tonemap writes</td>
      <td style="padding:.5em .8em;font-size:.82em;font-family:ui-monospace,monospace;line-height:1.6;"><span style="color:#ef4444;">VK</span> FRAGMENT_SHADER → COLOR_ATTACHMENT<br><span style="color:#3b82f6;">DX</span> PIXEL_SHADER_RESOURCE → RENDER_TARGET</td>
    </tr>
    <tr>
      <td style="padding:.5em .8em;font-weight:600;">Render Target → Present</td>
      <td style="padding:.5em .8em;font-size:.9em;opacity:.8;">Final composite → swapchain</td>
      <td style="padding:.5em .8em;font-size:.82em;font-family:ui-monospace,monospace;line-height:1.6;"><span style="color:#ef4444;">VK</span> COLOR_ATTACHMENT → BOTTOM_OF_PIPE<br><span style="color:#3b82f6;">DX</span> RENDER_TARGET → PRESENT</td>
    </tr>
    <tr style="background:rgba(127,127,127,.03);">
      <td style="padding:.5em .8em;font-weight:600;">Aliasing Barrier</td>
      <td style="padding:.5em .8em;font-size:.9em;opacity:.8;">GBuffer dies → HDR reuses memory</td>
      <td style="padding:.5em .8em;font-size:.82em;font-family:ui-monospace,monospace;line-height:1.6;"><span style="color:#3b82f6;">DX</span> RESOURCE_BARRIER_TYPE_ALIASING<br><span style="color:#ef4444;">VK</span> layout UNDEFINED (discard)</td>
    </tr>
  </tbody>
</table>
</div>

{{< interactive-barriers >}}

<div style="margin:1em 0;padding:.8em 1em;border-radius:8px;border-left:3px solid rgba(239,68,68,.5);background:rgba(239,68,68,.04);font-size:.9em;line-height:1.6;">
A real frame needs <strong>dozens of these</strong>. Miss one → rendering corruption or a GPU crash. Add an unnecessary one → the GPU stalls waiting for nothing. Managing this by hand is tedious and error-prone — the graph sees every read/write edge and emits the exact set automatically.
</div>

---

### Putting it together — v1 → v2 diff

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

Full updated source:

{{< include-code file="frame_graph_v2.h" lang="cpp" compact="true" >}}
{{< include-code file="example_v2.cpp" lang="cpp" compile="true" deps="frame_graph_v2.h" compact="true" >}}

That's three of the four intro promises delivered — automatic ordering, barrier insertion, and dead-pass culling. The only piece missing: resources still live for the entire frame. Version 3 fixes that with lifetime analysis and memory aliasing.

UE5's RDG does the same thing. When you call `FRDGBuilder::AddPass`, RDG builds the dependency graph from your declared reads/writes, topologically sorts it, culls dead passes, and inserts barriers — all before recording a single GPU command. The migration is incomplete, though — large parts of UE5's renderer still use legacy `FRHICommandList` calls outside the graph, requiring manual barriers at the RDG boundary. More on that in [Part III](/posts/frame-graph-production/).

---

## 💾 MVP v3 — Lifetimes & Aliasing

V2 gives us ordering, culling, and barriers — but every transient resource lives for the entire frame. A 1080p deferred pipeline allocates ~52 MB of transient textures that are each used for only 2–3 passes. If their lifetimes don't overlap, they can share physical memory. That's aliasing, and it typically saves 30–50% VRAM.

The algorithm has three steps. First, **scan lifetimes**: walk the sorted pass list and record each transient resource's `firstUsePass` and `lastUsePass` (imported resources are excluded — they're externally owned). Second, **track refcounts**: increment at first use, decrement at last use; when a resource's refcount hits zero, its physical memory becomes available. Third, **free-list scan**: sort resources by first-use, then greedily try to fit each one into an existing physical block that's compatible (same memory type, large enough, available after the previous user finished). Fit → reuse. No fit → allocate a new block. This is greedy interval-coloring.

Without aliasing, every transient resource is a **committed allocation** — its own chunk of VRAM from creation to end of frame, even if it's only used for 2–3 passes. Here's what that looks like for six transient resources at 1080p:

<div style="margin:1.2em 0;font-size:.85em;">
  <div style="border-radius:10px;overflow:hidden;border:1.5px solid rgba(239,68,68,.15);">
    <div style="padding:.5em .8em;background:rgba(239,68,68,.06);border-bottom:1px solid rgba(239,68,68,.1);font-weight:700;font-size:.9em;text-align:center;">❌ No aliasing — every resource owns its memory for the full frame</div>
    <div style="display:grid;grid-template-columns:140px repeat(7,1fr);gap:0;">
      <div style="padding:.35em .6em;background:rgba(127,127,127,.04);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.08);font-size:.75em;font-weight:600;opacity:.5;"></div>
      <div style="padding:.35em .3em;background:rgba(127,127,127,.04);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.72em;font-weight:600;opacity:.4;">P1</div>
      <div style="padding:.35em .3em;background:rgba(127,127,127,.04);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.72em;font-weight:600;opacity:.4;">P2</div>
      <div style="padding:.35em .3em;background:rgba(127,127,127,.04);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.72em;font-weight:600;opacity:.4;">P3</div>
      <div style="padding:.35em .3em;background:rgba(127,127,127,.04);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.72em;font-weight:600;opacity:.4;">P4</div>
      <div style="padding:.35em .3em;background:rgba(127,127,127,.04);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.72em;font-weight:600;opacity:.4;">P5</div>
      <div style="padding:.35em .3em;background:rgba(127,127,127,.04);border-bottom:1px solid rgba(99,102,241,.08);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.72em;font-weight:600;opacity:.4;">P6</div>
      <div style="padding:.35em .3em;background:rgba(127,127,127,.04);border-bottom:1px solid rgba(99,102,241,.08);text-align:center;font-size:.72em;font-weight:600;opacity:.4;">P7</div>
      <div style="padding:.3em .6em;border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.78em;font-weight:600;">GBuffer Albedo<div style="font-size:.8em;opacity:.4;">8 MB</div></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(59,130,246,.15);border-top:3px solid #3b82f6;border-bottom:3px solid #3b82f6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(59,130,246,.15);border-top:3px solid #3b82f6;border-bottom:3px solid #3b82f6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(59,130,246,.15);border-top:3px solid #3b82f6;border-bottom:3px solid #3b82f6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);background:rgba(239,68,68,.06);"></div>
      <div style="padding:.3em .6em;border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.78em;font-weight:600;">GBuffer Normals<div style="font-size:.8em;opacity:.4;">8 MB</div></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(139,92,246,.15);border-top:3px solid #8b5cf6;border-bottom:3px solid #8b5cf6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(139,92,246,.15);border-top:3px solid #8b5cf6;border-bottom:3px solid #8b5cf6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(139,92,246,.15);border-top:3px solid #8b5cf6;border-bottom:3px solid #8b5cf6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);background:rgba(239,68,68,.06);"></div>
      <div style="padding:.3em .6em;border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.78em;font-weight:600;">SSAO Scratch<div style="font-size:.8em;opacity:.4;">2 MB</div></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(34,197,94,.15);border-top:3px solid #22c55e;border-bottom:3px solid #22c55e;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(34,197,94,.15);border-top:3px solid #22c55e;border-bottom:3px solid #22c55e;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);background:rgba(239,68,68,.06);"></div>
      <div style="padding:.3em .6em;border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.78em;font-weight:600;">SSAO Result<div style="font-size:.8em;opacity:.4;">2 MB</div></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(245,158,11,.15);border-top:3px solid #f59e0b;border-bottom:3px solid #f59e0b;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(245,158,11,.15);border-top:3px solid #f59e0b;border-bottom:3px solid #f59e0b;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);background:rgba(239,68,68,.06);"></div>
      <div style="padding:.3em .6em;border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.78em;font-weight:600;">HDR Lighting<div style="font-size:.8em;opacity:.4;">16 MB</div></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(236,72,153,.15);border-top:3px solid #ec4899;border-bottom:3px solid #ec4899;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(236,72,153,.15);border-top:3px solid #ec4899;border-bottom:3px solid #ec4899;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);background:rgba(239,68,68,.06);"></div>
      <div style="padding:.3em .6em;border-right:1px solid rgba(99,102,241,.08);font-size:.78em;font-weight:600;">Bloom Scratch<div style="font-size:.8em;opacity:.4;">16 MB</div></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);background:rgba(239,68,68,.06);"></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);background:rgba(14,165,233,.15);border-top:3px solid #0ea5e9;border-bottom:3px solid #0ea5e9;"></div>
      <div style="background:rgba(14,165,233,.15);border-top:3px solid #0ea5e9;border-bottom:3px solid #0ea5e9;"></div>
    </div>
  </div>
  <div style="margin-top:.4em;font-size:.82em;opacity:.6;">
    <span style="color:#ef4444;">Red cells</span> = memory allocated but unused — wasted VRAM. Each resource holds its full allocation across the entire frame even though it's only active for 2–3 passes. Total: <strong style="color:#ef4444;">52 MB</strong> committed.
  </div>
</div>

Most of that memory sits idle. The colored bars show when each resource is actually used — everything else is waste. The graph knows every lifetime, so it can do better. Resources whose lifetimes don't overlap can share the same physical memory:

<div style="margin:1.2em 0;font-size:.85em;">
  <div style="border-radius:10px;overflow:hidden;border:1.5px solid rgba(99,102,241,.15);">
    <div style="display:grid;grid-template-columns:140px repeat(7,1fr);gap:0;">
      <div style="padding:.4em .6em;background:rgba(99,102,241,.06);border-bottom:1px solid rgba(99,102,241,.1);border-right:1px solid rgba(99,102,241,.08);font-weight:700;font-size:.82em;">Resource</div>
      <div style="padding:.4em .3em;background:rgba(99,102,241,.06);border-bottom:1px solid rgba(99,102,241,.1);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.75em;font-weight:600;opacity:.5;">P1</div>
      <div style="padding:.4em .3em;background:rgba(99,102,241,.06);border-bottom:1px solid rgba(99,102,241,.1);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.75em;font-weight:600;opacity:.5;">P2</div>
      <div style="padding:.4em .3em;background:rgba(99,102,241,.06);border-bottom:1px solid rgba(99,102,241,.1);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.75em;font-weight:600;opacity:.5;">P3</div>
      <div style="padding:.4em .3em;background:rgba(99,102,241,.06);border-bottom:1px solid rgba(99,102,241,.1);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.75em;font-weight:600;opacity:.5;">P4</div>
      <div style="padding:.4em .3em;background:rgba(99,102,241,.06);border-bottom:1px solid rgba(99,102,241,.1);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.75em;font-weight:600;opacity:.5;">P5</div>
      <div style="padding:.4em .3em;background:rgba(99,102,241,.06);border-bottom:1px solid rgba(99,102,241,.1);border-right:1px solid rgba(99,102,241,.05);text-align:center;font-size:.75em;font-weight:600;opacity:.5;">P6</div>
      <div style="padding:.4em .3em;background:rgba(99,102,241,.06);border-bottom:1px solid rgba(99,102,241,.1);text-align:center;font-size:.75em;font-weight:600;opacity:.5;">P7</div>
      <div style="padding:.35em .6em;border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.82em;font-weight:600;">GBuffer Albedo<div style="font-size:.8em;opacity:.4;">8 MB</div></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(59,130,246,.2);border-top:3px solid #3b82f6;border-bottom:3px solid #3b82f6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(59,130,246,.2);border-top:3px solid #3b82f6;border-bottom:3px solid #3b82f6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(59,130,246,.2);border-top:3px solid #3b82f6;border-bottom:3px solid #3b82f6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);"></div>
      <div style="padding:.35em .6em;border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.82em;font-weight:600;">HDR Lighting<div style="font-size:.8em;opacity:.4;">16 MB → <span style="color:#3b82f6;">slot A</span></div></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(59,130,246,.2);border-top:3px solid #3b82f6;border-bottom:3px solid #3b82f6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(59,130,246,.2);border-top:3px solid #3b82f6;border-bottom:3px solid #3b82f6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);"></div>
      <div style="padding:.35em .6em;border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.82em;font-weight:600;">GBuffer Normals<div style="font-size:.8em;opacity:.4;">8 MB</div></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(139,92,246,.2);border-top:3px solid #8b5cf6;border-bottom:3px solid #8b5cf6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(139,92,246,.2);border-top:3px solid #8b5cf6;border-bottom:3px solid #8b5cf6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(139,92,246,.2);border-top:3px solid #8b5cf6;border-bottom:3px solid #8b5cf6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);"></div>
      <div style="padding:.35em .6em;border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.82em;font-weight:600;">Bloom Scratch<div style="font-size:.8em;opacity:.4;">16 MB → <span style="color:#8b5cf6;">slot B</span></div></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(139,92,246,.2);border-top:3px solid #8b5cf6;border-bottom:3px solid #8b5cf6;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);background:rgba(139,92,246,.2);border-top:3px solid #8b5cf6;border-bottom:3px solid #8b5cf6;"></div>
      <div style="padding:.35em .6em;border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.08);font-size:.82em;font-weight:600;">SSAO Scratch<div style="font-size:.8em;opacity:.4;">2 MB</div></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(34,197,94,.2);border-top:3px solid #22c55e;border-bottom:3px solid #22c55e;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);background:rgba(34,197,94,.2);border-top:3px solid #22c55e;border-bottom:3px solid #22c55e;"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-bottom:1px solid rgba(99,102,241,.06);"></div>
      <div style="padding:.35em .6em;border-right:1px solid rgba(99,102,241,.08);font-size:.82em;font-weight:600;">SSAO Result<div style="font-size:.8em;opacity:.4;">2 MB → <span style="color:#22c55e;">slot C</span></div></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);"></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);background:rgba(34,197,94,.2);border-top:3px solid #22c55e;border-bottom:3px solid #22c55e;"></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);background:rgba(34,197,94,.2);border-top:3px solid #22c55e;border-bottom:3px solid #22c55e;"></div>
      <div style="border-right:1px solid rgba(99,102,241,.05);"></div>
      <div></div>
    </div>
  </div>
  <div style="margin-top:.5em;font-size:.82em;opacity:.6;">Same color = same physical memory. GBuffer Albedo dies at P4, HDR Lighting starts at P5 → both fit in <span style="color:#3b82f6;font-weight:600;">slot A</span>. Three physical blocks serve six virtual resources.</div>
</div>

<div style="display:flex;align-items:center;gap:1em;margin:1em 0;padding:.6em 1em;border-radius:8px;background:linear-gradient(90deg,rgba(239,68,68,.06),rgba(34,197,94,.06));">
  <div style="text-align:center;line-height:1.3;">
    <div style="font-size:.75em;opacity:.6;text-transform:uppercase;letter-spacing:.05em;">Without aliasing</div>
    <div style="font-size:1.4em;font-weight:800;color:#ef4444;">52 MB</div>
  </div>
  <div style="font-size:1.5em;opacity:.3;">→</div>
  <div style="text-align:center;line-height:1.3;">
    <div style="font-size:.75em;opacity:.6;text-transform:uppercase;letter-spacing:.05em;">With aliasing</div>
    <div style="font-size:1.4em;font-weight:800;color:#22c55e;">36 MB</div>
  </div>
  <div style="margin-left:auto;font-size:.85em;line-height:1.4;opacity:.8;">
    3 physical blocks shared across 6 virtual resources.<br>
    <strong style="color:#22c55e;">31% saved</strong> — in complex pipelines: 40–50%.
  </div>
</div>

This requires **placed resources** at the API level — GPU memory allocated from a heap, with resources bound to offsets within it. In D3D12, that means `ID3D12Heap` + `CreatePlacedResource`. In Vulkan, `VkDeviceMemory` + `vkBindImageMemory` at different offsets. Without placed resources (i.e., `CreateCommittedResource` or Vulkan dedicated allocations), each resource gets its own memory and aliasing is impossible — which is why the graph's allocator works with heaps.

Drag the interactive timeline below to see how resources share physical blocks as their lifetimes end:

{{< interactive-aliasing >}}

---

### Putting it together — v2 → v3 diff

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

Complete v3 source — all v2 code plus lifetime analysis and aliasing:

{{< include-code file="frame_graph_v3.h" lang="cpp" compact="true" >}}
{{< include-code file="example_v3.cpp" lang="cpp" compile="true" deps="frame_graph_v3.h" compact="true" >}}

~70 new lines on top of v2. Aliasing runs once per frame in O(R log R) — sort, then linear scan of the free list. Sub-microsecond for 15 transient resources.

That's the full value prop — automatic memory aliasing *and* automatic barriers from a single `FrameGraph` class. UE5's transient resource allocator does the same thing: any `FRDGTexture` created through `FRDGBuilder::CreateTexture` (vs `RegisterExternalTexture`) is transient and eligible for aliasing, using the same lifetime analysis and free-list scan we just built.

---

### What the MVP delivers

Three iterations produced a single `FrameGraph` class. Here's what it does every frame, broken down by phase — the same declare → compile → execute lifecycle from [Part I](/posts/frame-graph-theory/):

<div style="margin:1.2em 0;display:grid;grid-template-columns:repeat(3,1fr);gap:.8em;">
  <div style="padding:.8em 1em;border-radius:10px;border-top:3px solid #3b82f6;background:rgba(59,130,246,.04);">
    <div style="font-weight:800;font-size:.88em;margin-bottom:.5em;color:#3b82f6;">① Declare</div>
    <div style="font-size:.84em;line-height:1.6;opacity:.85;">
      Each <code>addPass</code> runs its setup lambda:<br>
      • declare reads &amp; writes<br>
      • request virtual resources<br>
      • version tracking builds edges
    </div>
    <div style="margin-top:.5em;padding:.3em .5em;border-radius:5px;background:rgba(59,130,246,.08);font-size:.76em;line-height:1.4;border:1px solid rgba(59,130,246,.12);">
      <strong>Zero GPU work.</strong> Resources are descriptions — no memory allocated yet.
    </div>
  </div>
  <div style="padding:.8em 1em;border-radius:10px;border-top:3px solid #8b5cf6;background:rgba(139,92,246,.04);">
    <div style="font-weight:800;font-size:.88em;margin-bottom:.5em;color:#8b5cf6;">② Compile</div>
    <div style="font-size:.84em;line-height:1.6;opacity:.85;">
      All automatic, all linear-time:<br>
      • <strong>sort</strong> — topo order (Kahn's)<br>
      • <strong>cull</strong> — kill dead passes<br>
      • <strong>scan lifetimes</strong> — first/last use<br>
      • <strong>alias</strong> — free-list reuse<br>
      • <strong>compute barriers</strong>
    </div>
    <div style="margin-top:.5em;padding:.3em .5em;border-radius:5px;background:rgba(139,92,246,.08);font-size:.76em;line-height:1.4;border:1px solid rgba(139,92,246,.12);">
      Everything linear or near-linear — all data fits in L1 cache.
    </div>
  </div>
  <div style="padding:.8em 1em;border-radius:10px;border-top:3px solid #22c55e;background:rgba(34,197,94,.04);">
    <div style="font-weight:800;font-size:.88em;margin-bottom:.5em;color:#22c55e;">③ Execute</div>
    <div style="font-size:.84em;line-height:1.6;opacity:.85;">
      Walk sorted, living passes:<br>
      • insert pre-computed barriers<br>
      • call execute lambda<br>
      • resources already aliased &amp; bound
    </div>
    <div style="margin-top:.5em;padding:.3em .5em;border-radius:5px;background:rgba(34,197,94,.08);font-size:.76em;line-height:1.4;border:1px solid rgba(34,197,94,.12);">
      <strong>Lambdas see a fully resolved environment.</strong> No manual barriers, no manual memory.
    </div>
  </div>
</div>

**Compile cost by step:**

<div style="overflow-x:auto;margin:.6em 0 1em">
<table style="width:100%;border-collapse:collapse;font-size:.88em">
  <thead>
    <tr>
      <th style="padding:.5em .8em;text-align:left;border-bottom:2px solid rgba(139,92,246,.3);color:#8b5cf6;width:30%">Compile step</th>
      <th style="padding:.5em .8em;text-align:center;border-bottom:2px solid rgba(139,92,246,.3);width:18%">Complexity</th>
      <th style="padding:.5em .8em;text-align:left;border-bottom:2px solid rgba(139,92,246,.3)">Algorithm</th>
    </tr>
  </thead>
  <tbody>
    <tr><td style="padding:.4em .8em;font-weight:600;">Topological sort</td><td style="padding:.4em .8em;text-align:center;font-family:ui-monospace,monospace;color:#8b5cf6">O(V + E)</td><td style="padding:.4em .8em;font-size:.9em;opacity:.8">Kahn's — passes + edges</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.4em .8em;font-weight:600;">Pass culling</td><td style="padding:.4em .8em;text-align:center;font-family:ui-monospace,monospace;color:#8b5cf6">O(V + E)</td><td style="padding:.4em .8em;font-size:.9em;opacity:.8">Backward reachability from output</td></tr>
    <tr><td style="padding:.4em .8em;font-weight:600;">Lifetime scan</td><td style="padding:.4em .8em;text-align:center;font-family:ui-monospace,monospace;color:#8b5cf6">O(V)</td><td style="padding:.4em .8em;font-size:.9em;opacity:.8">Single pass over sorted list</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.4em .8em;font-weight:600;">Aliasing</td><td style="padding:.4em .8em;text-align:center;font-family:ui-monospace,monospace;color:#8b5cf6">O(R log R)</td><td style="padding:.4em .8em;font-size:.9em;opacity:.8">Sort by first-use, greedy free-list scan</td></tr>
    <tr><td style="padding:.4em .8em;font-weight:600;">Barrier computation</td><td style="padding:.4em .8em;text-align:center;font-family:ui-monospace,monospace;color:#8b5cf6">O(V)</td><td style="padding:.4em .8em;font-size:.9em;opacity:.8">Linear scan with state lookup</td></tr>
  </tbody>
</table>
</div>
<div style="font-size:.84em;line-height:1.5;opacity:.7;margin:-.3em 0 1em 0">V = passes (~25), E = dependency edges (~50), R = transient resources (~15). Everything linear or near-linear.</div>

The graph doesn't care about your rendering *strategy*. It cares about your *dependencies*. Deferred or forward, the same `FrameGraph` class handles both — different topology, same automatic barriers and aliasing. That's the whole point.

---

<div style="margin:2em 0 0;padding:1em 1.2em;border-radius:10px;border:1px solid rgba(99,102,241,.2);background:rgba(99,102,241,.03);display:flex;justify-content:space-between;align-items:center;">
  <a href="/posts/frame-graph-theory/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    ← Previous: Part I — Theory
  </a>
  <a href="/posts/frame-graph-production/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    Next: Part III — Production Engines →
  </a>
</div>
