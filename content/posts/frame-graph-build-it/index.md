---
title: "Frame Graph — Build It"
date: 2026-02-10
lastmod: 2026-02-23
draft: false
authors: ["Pawel Stolecki"]
description: "Three iterations from blank file to working frame graph with automatic barriers and memory aliasing."
tags: ["rendering", "frame-graph", "gpu", "architecture", "cpp"]
categories: ["analysis"]
summary: "Three C++ iterations — v1 scaffold, v2 dependencies and barriers, v3 lifetime analysis and memory aliasing — building a complete frame graph from scratch."
showTableOfContents: false
keywords: ["frame graph C++", "render graph implementation", "topological sort", "Kahn algorithm", "barrier insertion", "memory aliasing", "resource lifetime", "GPU programming"]
---

{{< article-nav >}}

<div class="ds-series-nav">
📖 <strong>Part II of IV.</strong>&ensp; <a href="../frame-graph-theory/">Theory</a> → <em>Build It</em> → <a href="../frame-graph-advanced/">Beyond MVP</a> → <a href="../frame-graph-production/">Production Engines</a>
</div>

*Part I laid out the theory — declare, compile, execute. Now we turn that blueprint into code. Three iterations, each one building on the last: v1 lays the scaffold, v2 adds dependency-driven execution order (topological sort, pass culling, and automatic barriers), and v3 introduces lifetime analysis so non-overlapping resources can share the same heap. Time to get our hands dirty.*

<!-- MVP progression — animated power-up timeline -->
<div style="margin:1.6em 0 1.2em;position:relative;padding-left:3em;">
  <div style="margin-left:-3em;margin-bottom:1.4em;font-size:1.15em;font-weight:900;text-align:center;letter-spacing:.03em;">🧬 MVP Progression</div>
  <!-- vertical connector -->
  <div style="position:absolute;left:1.15em;top:3.2em;bottom:.8em;width:3px;background:linear-gradient(to bottom, var(--ds-info), var(--ds-code), var(--ds-success));border-radius:2px;opacity:.45;"></div>

  <!-- ── v1 ── -->
  <a href="#-v1--the-scaffold" style="text-decoration:none;color:inherit;display:block;position:relative;margin-bottom:1.6em;cursor:pointer;" onmouseover="this.querySelector('.mvp-card').style.transform='translateX(4px)';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-info-rgb),.5)'" onmouseout="this.querySelector('.mvp-card').style.transform='';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-info-rgb),.2)'">
    <div class="ds-dot ds-dot--info" style="left:-3em;top:.3em;">v1</div>
    <div class="mvp-card ds-mvp-card ds-mvp-card--info">
      <div class="ds-mvp-card__title ds-mvp-card__title--info">
        <span>The Scaffold</span>
        <span class="ds-badge ds-badge--info ds-badge--loc">~90 LOC</span>
      </div>
      <div class="ds-mvp-card__desc">Pass declaration, virtual resources, linear execution.</div>
      <div class="ds-mvp-card__tags">
        <span class="ds-badge ds-badge--info">🔓 AddPass</span>
        <span class="ds-badge ds-badge--info">🔓 CreateResource</span>
        <span class="ds-badge ds-badge--info">🔓 ImportResource</span>
        <span class="ds-badge ds-badge--info">🔓 Execute()</span>
      </div>
      <div class="ds-power-bar"><div class="ds-power-bar__fill ds-power-bar__fill--info"></div></div>
      <div class="ds-power-bar__labels"><span>DECLARE</span><span>COMPILE</span><span>EXECUTE</span></div>
    </div>
  </a>

  <!-- ── v2 ── -->
  <a href="#-mvp-v2--dependencies--barriers" style="text-decoration:none;color:inherit;display:block;position:relative;margin-bottom:1.6em;cursor:pointer;" onmouseover="this.querySelector('.mvp-card').style.transform='translateX(4px)';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-code-rgb),.5)'" onmouseout="this.querySelector('.mvp-card').style.transform='';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-code-rgb),.2)'">
    <div class="ds-dot ds-dot--code" style="left:-3em;top:.3em;">v2</div>
    <div class="mvp-card ds-mvp-card ds-mvp-card--code">
      <div class="ds-mvp-card__title ds-mvp-card__title--code">
        <span>Dependencies & Barriers</span>
        <span class="ds-badge ds-badge--code ds-badge--loc">~260 LOC</span>
      </div>
      <div class="ds-mvp-card__desc">Resource versioning → edges → topo-sort → dead-pass culling → automatic barrier insertion.</div>
      <div class="ds-mvp-card__tags">
        <span class="ds-badge ds-badge--code">🔓 read / write</span>
        <span class="ds-badge ds-badge--code">🔓 topo-sort</span>
        <span class="ds-badge ds-badge--code">🔓 pass culling</span>
        <span class="ds-badge ds-badge--code">🔓 auto barriers</span>
      </div>
      <div class="ds-power-bar"><div class="ds-power-bar__fill ds-power-bar__fill--v2"></div></div>
      <div class="ds-power-bar__labels"><span>DECLARE</span><span>COMPILE</span><span>EXECUTE</span></div>
    </div>
  </a>

  <!-- ── v3 ── -->
  <a href="#-mvp-v3--lifetimes--aliasing" style="text-decoration:none;color:inherit;display:block;position:relative;cursor:pointer;" onmouseover="this.querySelector('.mvp-card').style.transform='translateX(4px)';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-success-rgb),.6)'" onmouseout="this.querySelector('.mvp-card').style.transform='';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-success-rgb),.3)'">
    <div class="ds-dot ds-dot--success" style="left:-3em;top:.3em;">v3</div>
    <div class="mvp-card ds-mvp-card ds-mvp-card--success">
      <div class="ds-mvp-card__title ds-mvp-card__title--success">
        <span>Lifetimes & Aliasing</span>
        <span class="ds-badge ds-badge--success ds-badge--loc">~500 LOC</span>
        <span class="ds-badge ds-badge--solid-success" style="font-size:.62em;font-weight:800;">★ FULL MVP</span>
      </div>
      <div class="ds-mvp-card__desc">Compile precomputes everything — sort, cull, lifetimes, aliasing, barriers — into a <code>CompiledPlan</code>.</div>
      <div class="ds-mvp-card__tags">
        <span class="ds-badge ds-badge--success">🔓 lifetime scan</span>
        <span class="ds-badge ds-badge--success">🔓 memory aliasing</span>
        <span class="ds-badge ds-badge--success">⚡ VRAM aliasing</span>
      </div>
      <div class="ds-power-bar"><div class="ds-power-bar__fill ds-power-bar__fill--full"></div></div>
      <div class="ds-power-bar__labels"><span>DECLARE</span><span>COMPILE</span><span>EXECUTE</span></div>
    </div>
  </a>
</div>

---

## 🏗 Architecture & API Decisions

We start from the API you *want* to write, then build toward it — starting with bare scaffolding and ending with automatic barriers and memory aliasing.

<!-- UML class diagram — API overview -->
<div style="display:flex;justify-content:center;gap:1.8em;margin:0 0 .5em;font-size:.78em;font-weight:600;opacity:.85;">
  <span style="display:inline-flex;align-items:center;gap:.35em;"><span style="display:inline-block;width:10px;height:10px;border-radius:3px;background:transparent;border:2.5px solid #d97706;"></span> v1 — Scaffold</span>
  <span style="display:inline-flex;align-items:center;gap:.35em;"><span style="display:inline-block;width:10px;height:10px;border-radius:3px;background:transparent;border:2.5px solid #6366f1;"></span> v2 — Dependencies</span>
  <span style="display:inline-flex;align-items:center;gap:.35em;"><span style="display:inline-block;width:10px;height:10px;border-radius:3px;background:transparent;border:2.5px solid #16a34a;"></span> v3 — Aliasing</span>
</div>
{{< mermaid >}}
classDiagram
direction TB

class FrameGraph{
  +ResourceHandle CreateResource(desc)
  +ResourceHandle ImportResource(desc, state)
  +AddPass(name, setup, execute)
  +Read(passIdx, handle)
  +Write(passIdx, handle)
  +ReadWrite(passIdx, handle)
  +CompiledPlan Compile()
  +Execute(plan)
  -BuildEdges()
  -PassIndex[] TopoSort()
  -Cull(sortedPasses)
  -Lifetime[] ScanLifetimes(sortedPasses)
  -BlockIndex[] AliasResources(lifetimes)
  -ResourceState StateForUsage(passIdx, handle, isWrite)
  -Barrier[][] ComputeBarriers(sortedPasses, mapping)
  -EmitBarriers(barriers)
}
class ResourceHandle{
  +ResourceIndex index
  +bool IsValid()
}
class ResourceDesc{
  +uint32 width
  +uint32 height
  +Format format
}
class Format{
  RGBA8
  RGBA16F
  R8
  D32F
}
class RenderPass{
  +string name
  +function Setup
  +function Execute
  +ResourceHandle[] reads
  +ResourceHandle[] writes
  +ResourceHandle[] readWrites
  +PassIndex[] dependsOn
  +PassIndex[] successors
  +uint32 inDegree
  +bool alive
}
class ResourceEntry{
  +ResourceDesc desc
  +ResourceVersion[] versions
  +ResourceState currentState
  +bool imported
}
class ResourceVersion{
  +PassIndex writerPass
  +PassIndex[] readerPasses
}
class ResourceState{
  Undefined
  ColorAttachment
  DepthAttachment
  ShaderRead
  UnorderedAccess
  Present
}
class CompiledPlan{
  +PassIndex[] sorted
  +BlockIndex[] mapping
  +Barrier[][] barriers
}
class Barrier{
  +ResourceIndex resourceIndex
  +ResourceState oldState
  +ResourceState newState
  +bool isAliasing
  +ResourceIndex aliasBefore
}
class Lifetime{
  +PassIndex firstUse
  +PassIndex lastUse
  +bool isTransient
}
class PhysicalBlock{
  +uint32 sizeBytes
  +PassIndex availAfter
}
FrameGraph *-- RenderPass : owns passes
FrameGraph *-- ResourceEntry : owns resources
FrameGraph ..> CompiledPlan : produces
FrameGraph ..> ResourceHandle : returns
FrameGraph ..> Lifetime : computes per resource
FrameGraph ..> PhysicalBlock : allocates from free-list
RenderPass --> ResourceHandle : reads/writes
ResourceEntry *-- ResourceDesc : describes
ResourceEntry *-- ResourceVersion : tracks per version
ResourceEntry --> ResourceState : current state
ResourceDesc --> Format : pixel format
CompiledPlan *-- Barrier : pre-pass transitions
Barrier --> ResourceState : old/new state

style ResourceHandle stroke:#d97706,stroke-width:2.5px
style ResourceDesc stroke:#d97706,stroke-width:2.5px
style Format stroke:#d97706,stroke-width:2.5px
style RenderPass stroke:#d97706,stroke-width:2.5px
style ResourceEntry stroke:#6366f1,stroke-width:2.5px
style ResourceVersion stroke:#6366f1,stroke-width:2.5px
style ResourceState stroke:#6366f1,stroke-width:2.5px
style Barrier stroke:#6366f1,stroke-width:2.5px
style CompiledPlan stroke:#6366f1,stroke-width:2.5px
style Lifetime stroke:#16a34a,stroke-width:2.5px
style PhysicalBlock stroke:#16a34a,stroke-width:2.5px
{{< /mermaid >}}

### 🔀 Design choices

The three-phase model from [Part I](../frame-graph-theory/) forces eight API decisions. Every choice is driven by the same question: *what does the graph compiler need, and what's the cheapest way to give it?*

<div style="margin:1.2em 0;font-size:.88em;">
<table style="width:100%;border-collapse:collapse;line-height:1.5;">
<thead>
<tr style="border-bottom:2px solid rgba(var(--ds-indigo-rgb),.15);text-align:left;">
  <th style="padding:.5em .6em;width:2.5em;">#</th>
  <th style="padding:.5em .6em;">Question</th>
  <th style="padding:.5em .6em;">Our pick</th>
  <th style="padding:.5em .6em;">Why</th>
  <th style="padding:.5em .6em;opacity:.6;">Alternative</th>
</tr>
</thead>
<tbody>
<tr><td colspan="5" style="padding:.6em .6em .3em;font-weight:800;font-size:.85em;letter-spacing:.04em;color:var(--ds-info-light);border-bottom:1px solid rgba(var(--ds-info-rgb),.12);">DECLARE — how passes and resources enter the graph</td></tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);">
  <td style="padding:.5em .6em;font-weight:700;">①</td>
  <td style="padding:.5em .6em;">How does setup talk to execute?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Lambda captures</strong></td>
  <td style="padding:.5em .6em;opacity:.8;">Zero boilerplate — handles live in scope, both lambdas capture them directly.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Type-erased pass data — <code>AddPass&lt;PassData&gt;(setup, exec)</code>. Decouples setup/execute across TUs.</td>
</tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);background:rgba(var(--ds-indigo-rgb),.02);">
  <td style="padding:.5em .6em;font-weight:700;">②</td>
  <td style="padding:.5em .6em;">Where do DAG edges come from?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Explicit <code>fg.Read/Write(pass, h)</code></strong></td>
  <td style="padding:.5em .6em;opacity:.8;">Every edge is an explicit call — easy to grep and debug.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Scoped builder — <code>builder.Read/Write(h)</code> auto-binds to the current pass. Prevents mis-wiring at scale.</td>
</tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);">
  <td style="padding:.5em .6em;font-weight:700;">③</td>
  <td style="padding:.5em .6em;">What is a resource handle?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Plain <code>uint32_t</code> index</strong></td>
  <td style="padding:.5em .6em;opacity:.8;">One integer, trivially copyable — no templates, no overhead.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Typed wrappers — <code>FRDGTextureRef</code> / <code>FRDGBufferRef</code>. Compile-time safety for 700+ passes (UE5).</td>
</tr>
<tr><td colspan="5" style="padding:.6em .6em .3em;font-weight:800;font-size:.85em;letter-spacing:.04em;color:var(--ds-code-light);border-bottom:1px solid rgba(var(--ds-code-rgb),.12);">COMPILE — what the graph analyser decides</td></tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);">
  <td style="padding:.5em .6em;font-weight:700;">④</td>
  <td style="padding:.5em .6em;">Is compile explicit?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Yes — <code>Compile()→Execute(plan)</code></strong></td>
  <td style="padding:.5em .6em;opacity:.8;">Returned plan struct lets you log, validate, and visualise the DAG — invaluable while learning.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Implicit — <code>Execute()</code> compiles internally. Simpler call site, less ceremony.</td>
</tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);background:rgba(var(--ds-indigo-rgb),.02);">
  <td style="padding:.5em .6em;font-weight:700;">⑤</td>
  <td style="padding:.5em .6em;">How does culling find the root?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Last sorted pass</strong></td>
  <td style="padding:.5em .6em;opacity:.8;">Zero config — Present is naturally last in topo order. Breaks with multiple output roots; add a <code>NeverCull</code> flag when you need them.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Write-to-imported heuristic + <code>NeverCull</code> flags. Supports multiple output roots.</td>
</tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);">
  <td style="padding:.5em .6em;font-weight:700;">⑥</td>
  <td style="padding:.5em .6em;">Queue model?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Single graphics queue</strong></td>
  <td style="padding:.5em .6em;opacity:.8;">Keeps barrier logic to plain resource state transitions — no cross-queue barriers. Multi-queue is a compiler feature layered on top; clean upgrade path.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Multi-queue + async compute. 10–30% GPU uplift but needs fences & cross-queue barriers. <a href="../frame-graph-advanced/" style="opacity:.7;">Part III</a>.</td>
</tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);background:rgba(var(--ds-indigo-rgb),.02);">
  <td style="padding:.5em .6em;font-weight:700;">⑦</td>
  <td style="padding:.5em .6em;">Rebuild frequency?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Full rebuild every frame</strong></td>
  <td style="padding:.5em .6em;opacity:.8;">You need a significantly more complex frame before this becomes visibly heavy — for an MVP, full rebuild is fine.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Cached topology — re-compile only on structural change. Near-zero steady-state cost but complex invalidation logic.</td>
</tr>
<tr><td colspan="5" style="padding:.6em .6em .3em;font-weight:800;font-size:.85em;letter-spacing:.04em;color:var(--ds-success);border-bottom:1px solid rgba(var(--ds-success-rgb),.12);">EXECUTE — how the compiled plan becomes GPU commands</td></tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);">
  <td style="padding:.5em .6em;font-weight:700;">⑧</td>
  <td style="padding:.5em .6em;">Recording strategy?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Single command list</strong></td>
  <td style="padding:.5em .6em;opacity:.8;">Sequential walk — trivial to implement and debug. CPU cost is noise at ~25 passes. Swap to parallel deferred command lists when pass count exceeds ~60.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Parallel command lists — one per pass group, recorded across threads. Scales to 100+ passes (UE5).</td>
</tr>
</tbody>
</table>
</div>



### 🚀 The Target API

With those choices made, here's where we're headed — the final API in under 30 lines:

{{< include-code file="api_demo.cpp" lang="cpp" open="true" >}}

### 🧱 v1 — The Scaffold

<div class="ds-callout ds-callout--info">
🎯 <strong>Goal:</strong> Declare passes and virtual resources, execute in registration order — the skeleton that v2 and v3 build on.
</div>

Three types are all we need to start: a `ResourceDesc` (width, height, format — no GPU handle yet), a `ResourceHandle` that's just an index, and a `RenderPass` with setup + execute lambdas. The `FrameGraph` class owns arrays of both and runs passes in declaration order. No dependency tracking, no barriers — just the foundation that v2 and v3 build on.

{{< code-diff title="v1 — Resource types (frame_graph_v1.h)" collapsible="true" >}}
@@ frame_graph_v1.h — Format, ResourceDesc, ResourceHandle @@
+enum class Format
+{
+    RGBA8,
+    RGBA16F,
+    R8,
+    D32F
+};
+
+struct ResourceDesc
+{
+    uint32_t width = 0;
+    uint32_t height = 0;
+    Format format = Format::RGBA8;
+};
+
+using ResourceIndex = uint32_t;  // readable alias for resource array indices
+
+// Lightweight handle — index into FrameGraph's resource array, no GPU memory involved.
+struct ResourceHandle
+{
+    ResourceIndex index = UINT32_MAX;
+    bool IsValid() const { return index != UINT32_MAX; }
+};
{{< /code-diff >}}

A pass is two lambdas — setup (runs now, wires the DAG) and execute (stored for later, records GPU commands). v1 doesn't use setup yet, but the slot is there for v2:

{{< code-diff title="v1 — RenderPass struct (frame_graph_v1.h)" collapsible="true" >}}
@@ frame_graph_v1.h — RenderPass struct @@
+// A render pass: Setup wires the DAG (declares reads/writes), Execute records GPU commands.
+struct RenderPass
+{
+    std::string name;
+    std::function<void()> Setup;                // build the DAG (v1: unused)
+    std::function<void(/*cmd list*/)> Execute;  // record GPU commands
+};
{{< /code-diff >}}

The `FrameGraph` class owns two arrays — passes and resources. `AddPass` runs setup immediately — the DAG is built during declaration, not lazily. `Execute()` in v1 just walks passes in order:

{{< code-diff title="v1 — FrameGraph class (frame_graph_v1.h)" collapsible="true" >}}
@@ frame_graph_v1.h — FrameGraph class @@
+class FrameGraph
+{
+  public:
+    ResourceHandle CreateResource(const ResourceDesc& desc);  // transient — graph owns lifetime
+    ResourceHandle ImportResource(const ResourceDesc& desc);  // external — e.g. swapchain backbuffer
+
+    // AddPass: runs setup immediately (wires DAG), stores execute for later.
+    template <typename SetupFn, typename ExecFn>
+    void AddPass(const std::string& name, SetupFn&& setup, ExecFn&& exec)
+    {
+        passes.push_back({name, std::forward<SetupFn>(setup), std::forward<ExecFn>(exec)});
+        passes.back().Setup();  // run setup immediately — DAG is built here
+    }
+
+    void Execute();  // v1: just run in declaration order
+
+  private:
+    std::vector<RenderPass> passes;
+    std::vector<ResourceDesc> resources;
+};
{{< /code-diff >}}

`CreateResource` and `ImportResource` both push a descriptor into the resources array and return a handle. No GPU memory yet — that happens at execute time. In v1 they're identical; v2 will differentiate imported resources:

{{< code-diff title="v1 — CreateResource / ImportResource" collapsible="true" >}}
@@ frame_graph_v1.cpp — CreateResource / ImportResource @@
+// No GPU memory is allocated yet — that happens at execute time.
+ResourceHandle FrameGraph::CreateResource(const ResourceDesc& desc)
+{
+    resources.push_back(desc);
+    return {static_cast<ResourceIndex>(resources.size() - 1)};
+}
+
+ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc)
+{
+    resources.push_back(desc);  // v1: same as create (no aliasing yet)
+    return {static_cast<ResourceIndex>(resources.size() - 1)};
+}
{{< /code-diff >}}

`Execute()` is the simplest possible loop: walk passes in declaration order, call each callback, clear everything for the next frame. No compile step, no reordering — just playback:

{{< code-diff title="v1 — Execute()" collapsible="true" >}}
@@ frame_graph_v1.cpp — Execute() @@
+// v1 execute: just run passes in the order they were declared.
+void FrameGraph::Execute()
+{
+    printf("\n[1] Executing (declaration order -- no compile step):\n");
+    for (auto& pass : passes)
+    {
+        printf("  >> exec: %s\n", pass.name.c_str());
+        pass.Execute(/* &cmdList */);
+    }
+    passes.clear();  // reset for next frame
+    resources.clear();
+}
{{< /code-diff >}}

Full source and runnable example:

{{< include-code file="frame_graph_v1.h" lang="cpp" compact="true" >}}
{{< include-code file="frame_graph_v1.cpp" lang="cpp" compact="true" >}}
{{< include-code file="example_v1.cpp" lang="cpp" compile="true" deps="frame_graph_v1.h,frame_graph_v1.cpp" compact="true" >}}

Compiles and runs — the execute lambdas are stubs, but the scaffolding is real. Every piece we add in v2 and v3 goes into this same `FrameGraph` class.

<div class="ds-grid-2col" style="gap:.8em;margin:1em 0;">
  <div class="ds-callout ds-callout--success" style="font-size:.9em;line-height:1.5;">
    <strong style="color:var(--ds-success);">✓ What it proves</strong><br>
    The lambda-based pass declaration pattern works. You can already compose passes without manual barrier calls (even though barriers are no-ops here).
  </div>
  <div class="ds-callout ds-callout--danger" style="font-size:.9em;line-height:1.5;">
    <strong style="color:var(--ds-danger);">✗ What it lacks</strong><br>
    Executes passes in declaration order, creates every resource upfront. Correct but wasteful. Version 2 adds the graph.
  </div>
</div>

---

## 🔗 MVP v2 — Dependencies & Barriers

<div class="ds-callout ds-callout--info">
🎯 <strong>Goal:</strong> Automatic pass ordering, dead-pass culling, and barrier insertion — the graph now drives the GPU instead of you.
</div>

Four steps in strict order — each one's output is the next one's input:

<div style="margin:.8em 0 1.2em;display:grid;grid-template-columns:1fr auto 1fr auto 1fr auto 1fr;gap:0;align-items:stretch;border-radius:10px;overflow:hidden;border:1.5px solid rgba(var(--ds-indigo-rgb),.2);">
  <a href="#v2-versioning" style="padding:.7em .5em .5em;background:rgba(var(--ds-info-rgb),.05);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(var(--ds-info-rgb),.12)'" onmouseout="this.style.background='rgba(var(--ds-info-rgb),.05)'">
    <div style="font-size:.65em;font-weight:800;opacity:.45;margin-bottom:.15em;">STEP 1</div>
    <div style="font-size:1.2em;margin-bottom:.15em;">🔀</div>
    <div style="font-weight:800;font-size:.85em;color:var(--ds-info-light);">Versioning</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">reads/writes produce edges</div>
  </a>
  <div style="display:flex;align-items:center;font-size:1.1em;opacity:.35;padding:0 .1em;">→</div>
  <a href="#v2-toposort" style="padding:.7em .5em .5em;background:rgba(var(--ds-code-rgb),.05);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(var(--ds-code-rgb),.12)'" onmouseout="this.style.background='rgba(var(--ds-code-rgb),.05)'">
    <div style="font-size:.65em;font-weight:800;opacity:.45;margin-bottom:.15em;">STEP 2</div>
    <div style="font-size:1.2em;margin-bottom:.15em;">📦</div>
    <div style="font-weight:800;font-size:.85em;color:var(--ds-code-light);">Topo Sort</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">edges become execution order</div>
  </a>
  <div style="display:flex;align-items:center;font-size:1.1em;opacity:.35;padding:0 .1em;">→</div>
  <a href="#v2-culling" style="padding:.7em .5em .5em;background:rgba(var(--ds-warn-rgb),.05);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(var(--ds-warn-rgb),.12)'" onmouseout="this.style.background='rgba(var(--ds-warn-rgb),.05)'">
    <div style="font-size:.65em;font-weight:800;opacity:.45;margin-bottom:.15em;">STEP 3</div>
    <div style="font-size:1.2em;margin-bottom:.15em;">✂</div>
    <div style="font-weight:800;font-size:.85em;color:var(--ds-warn);">Pass Culling</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">walk backward, kill dead passes</div>
  </a>
  <div style="display:flex;align-items:center;font-size:1.1em;opacity:.35;padding:0 .1em;">→</div>
  <a href="#v2-barriers" style="padding:.7em .5em .5em;background:rgba(var(--ds-danger-rgb),.05);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(var(--ds-danger-rgb),.12)'" onmouseout="this.style.background='rgba(var(--ds-danger-rgb),.05)'">
    <div style="font-size:.65em;font-weight:800;opacity:.45;margin-bottom:.15em;">STEP 4</div>
    <div style="font-size:1.2em;margin-bottom:.15em;">🚧</div>
    <div style="font-weight:800;font-size:.85em;color:var(--ds-danger);">Barriers</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">emit GPU state transitions</div>
  </a>
</div>

<span id="v2-versioning"></span>

### 🔀 Resource versioning — the data structure

Every write bumps a version number; every read attaches to the current version. That’s enough to produce precise dependency edges ([theory refresher](/posts/frame-graph-theory/#how-edges-form--resource-versioning)).

The key data structure: each resource entry tracks its **current version** (incremented on write) and a **writer pass index** per version. When a pass calls `Read(h)`, the graph looks up the current version's writer and adds a dependency edge from that writer to the reading pass.

Here's what changes from v1. The `ResourceDesc` array becomes `ResourceEntry` — each entry carries a version list and an `imported` flag. `ResourceVersion` tracks which pass wrote each version and which passes read it — this is the data Read/Write use to build edges:

{{< code-diff title="v1 → v2 — ResourceVersion & ResourceEntry" collapsible="true" >}}
@@ frame_graph_v2.h — PassIndex alias, ResourceVersion, ResourceEntry @@
+using PassIndex = uint32_t;  // readable alias for pass array indices
+
+struct ResourceVersion
+{
+    PassIndex writerPass = UINT32_MAX;    // Each Read() links to the current version's writer → automatic dependency edge.
+    std::vector<PassIndex> readerPasses;  // Each Write() to a resource creates a new version.
+    bool HasWriter() const { return writerPass != UINT32_MAX; }
+};
+
+// Replaces raw ResourceDesc — now tracks version history per resource.
+struct ResourceEntry
+{
+    ResourceDesc desc;
+    std::vector<ResourceVersion> versions;  // version 0, 1, 2...
+    bool imported = false;  // imported = externally owned (e.g. swapchain)
+};
{{< /code-diff >}}

`RenderPass` gains `reads`, `writes`, `readWrites` (UAV), and `dependsOn` vectors. The FrameGraph class adds three new methods — `Read()`, `Write()`, `ReadWrite()` — and the internal storage switches from a flat `ResourceDesc` array to `ResourceEntry`:

{{< code-diff title="v1 → v2 — RenderPass & FrameGraph API changes" collapsible="true" >}}
@@ frame_graph_v2.h — RenderPass gets reads/writes/dependsOn @@
 struct RenderPass
 {
     std::string name;
     std::function<void()> Setup;
     std::function<void(/*cmd list*/)> Execute;
+    std::vector<ResourceHandle> reads;
+    std::vector<ResourceHandle> writes;
+    std::vector<ResourceHandle> readWrites;  // UAV (explicit)
+    std::vector<PassIndex> dependsOn;
 };

@@ frame_graph_v2.h — FrameGraph adds Read(), Write(), ReadWrite() @@
+    void Read(PassIndex passIdx, ResourceHandle h);
+    void Write(PassIndex passIdx, ResourceHandle h);
+    void ReadWrite(PassIndex passIdx, ResourceHandle h);  // UAV access

@@ frame_graph_v2.h — ResourceDesc[] becomes ResourceEntry[] @@
-    std::vector<ResourceDesc> resources;
+    std::vector<ResourceEntry> entries;  // now with versioning
{{< /code-diff >}}

`CreateResource` / `ImportResource` now build `ResourceEntry` objects (with an empty initial version). The real work is in the three access methods:

{{< code-diff title="v1 → v2 — CreateResource / ImportResource updated" collapsible="true" >}}
@@ frame_graph_v2.cpp — CreateResource / ImportResource now use ResourceEntry @@
 ResourceHandle FrameGraph::CreateResource(const ResourceDesc& desc)
 {
-    resources.push_back(desc);
-    return {static_cast<ResourceIndex>(resources.size() - 1)};
+    entries.push_back({desc, {{}}});
+    return {static_cast<ResourceIndex>(entries.size() - 1)};
 }

 ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc)
 {
-    resources.push_back(desc);
-    return {static_cast<ResourceIndex>(resources.size() - 1)};
+    entries.push_back({desc, {{}}, /*imported=*/true});
+    return {static_cast<ResourceIndex>(entries.size() - 1)};
 }
{{< /code-diff >}}

`Read()` looks up the current version's writer and adds a RAW (read-after-write) dependency edge — "I need the result of whoever last wrote this." It also registers itself as a reader so that a future `Write()` knows who to protect:

{{< code-diff title="v1 → v2 — Read()" collapsible="true" >}}
@@ frame_graph_v2.cpp — Read() @@
+// Read: look up who last wrote this resource → add a dependency edge from that writer to this pass.
+void FrameGraph::Read(PassIndex passIdx, ResourceHandle h)
+{
+    auto& ver = entries[h.index].versions.back();  // current version
+    if (ver.HasWriter())
+    {
+        passes[passIdx].dependsOn.push_back(ver.writerPass);  // RAW edge
+    }
+    ver.readerPasses.push_back(passIdx);  // track who reads this version
+    passes[passIdx].reads.push_back(h);   // record for barrier insertion
+}
{{< /code-diff >}}

`Write()` does the opposite: it adds WAR (write-after-read) edges from every reader of the current version — ensuring they all finish before the overwrite — then bumps the version so future reads see the new data:

{{< code-diff title="v1 → v2 — Write()" collapsible="true" >}}
@@ frame_graph_v2.cpp — Write() @@
+// Write: add WAR edges from current-version readers, then bump the version.
+void FrameGraph::Write(PassIndex passIdx, ResourceHandle h)
+{
+    auto& ver = entries[h.index].versions.back();  // current version (pre-bump)
+    for (PassIndex reader : ver.readerPasses)
+        passes[passIdx].dependsOn.push_back(reader);  // WAR edge: reader must finish first
+    entries[h.index].versions.push_back({});           // bump version
+    entries[h.index].versions.back().writerPass = passIdx;  // this pass owns the new version
+    passes[passIdx].writes.push_back(h);  // record for barrier insertion
+}
{{< /code-diff >}}

`ReadWrite()` (UAV) combines both patterns: RAW edge from the previous writer, WAR edges from current readers, version bump, and it pushes the handle into all three lists (reads, writes, readWrites) so the barrier system can identify it as an unordered-access resource:

{{< code-diff title="v1 → v2 — ReadWrite() (UAV)" collapsible="true" >}}
@@ frame_graph_v2.cpp — ReadWrite() (UAV) @@
+// ReadWrite (UAV): depend on previous writer + WAR edges from readers, then bump version.
+void FrameGraph::ReadWrite(PassIndex passIdx, ResourceHandle h)
+{
+    auto& ver = entries[h.index].versions.back();
+    if (ver.HasWriter())
+    {
+        passes[passIdx].dependsOn.push_back(ver.writerPass);  // RAW edge
+    }
+    for (PassIndex reader : ver.readerPasses)
+        passes[passIdx].dependsOn.push_back(reader);  // WAR edge
+    entries[h.index].versions.push_back({});           // bump version (it's a write)
+    entries[h.index].versions.back().writerPass = passIdx;
+    passes[passIdx].reads.push_back(h);       // appears in both lists (for barriers + lifetimes)
+    passes[passIdx].writes.push_back(h);
+    passes[passIdx].readWrites.push_back(h);  // marks this handle as UAV for StateForUsage
+}
{{< /code-diff >}}

Every `Write()` adds WAR edges from every reader of the current version (so they finish before the overwrite), then bumps the version. Every `Read()` finds the current version's writer and records a RAW edge. Together they capture both read-after-write and write-after-read hazards — those edges feed the next three steps.

---

<span id="v2-toposort"></span>

### 📊 Topological sort (Kahn's algorithm)

With edges in place, we need an execution order that respects every dependency. Kahn’s algorithm ([theory refresher](/posts/frame-graph-theory/#sorting-and-culling)) gives us one in O(V+E). `BuildEdges()` deduplicates the raw `dependsOn` entries and builds the adjacency list; `TopoSort()` does the zero-in-degree queue drain:

{{< code-diff title="v2 — Edge deduplication (BuildEdges)" collapsible="true" >}}
@@ frame_graph_v2.h — RenderPass gets successors + inDegree (for Kahn's) @@
 struct RenderPass
 {
     ...
+    std::vector<PassIndex> successors;  // passes that depend on this one
+    uint32_t inDegree = 0;              // incoming edge count (Kahn's)
 };

@@ frame_graph_v2.cpp — BuildEdges() @@
+// Deduplicate raw dependsOn edges and build forward adjacency list (successors) for Kahn's algorithm.
+void FrameGraph::BuildEdges()
+{
+    for (PassIndex i = 0; i < passes.size(); i++)
+    {
+        std::unordered_set<PassIndex> seen;
+        for (PassIndex dep : passes[i].dependsOn)
+        {
+            if (seen.insert(dep).second)  // first time seeing this edge?
+            {
+                passes[dep].successors.push_back(i);  // forward link: dep → i
+                passes[i].inDegree++;                  // i has one more incoming edge
+            }
+        }
+    }
+}
{{< /code-diff >}}

With the adjacency list built, `TopoSort()` implements Kahn's zero-in-degree queue drain — any pass whose dependencies are all satisfied gets dequeued next:

{{< code-diff title="v2 — Kahn's topological sort" collapsible="true" >}}
@@ frame_graph_v2.cpp — TopoSort() @@
+// Kahn's algorithm: dequeue zero-in-degree passes → valid execution order respecting all dependencies.
+std::vector<PassIndex> FrameGraph::TopoSort()
+{
+    std::queue<PassIndex> q;
+    std::vector<uint32_t> inDeg(passes.size());
+    for (PassIndex i = 0; i < passes.size(); i++)
+    {
+        inDeg[i] = passes[i].inDegree;
+        if (inDeg[i] == 0)
+            q.push(i);  // no dependencies → ready immediately
+    }
+    std::vector<PassIndex> order;
+    while (!q.empty())
+    {
+        PassIndex cur = q.front();
+        q.pop();
+        order.push_back(cur);
+        for (PassIndex succ : passes[cur].successors)
+        {
+            if (--inDeg[succ] == 0)  // all of succ's dependencies done?
+                q.push(succ);        // succ is now ready
+        }
+    }
+    // If we didn't visit every pass, the graph has a cycle — invalid.
+    assert(order.size() == passes.size() && "Cycle detected!");
+    return order;
+}
{{< /code-diff >}}

---

<span id="v2-culling"></span>

### ✂ Pass culling

A sorted graph still runs passes nobody reads from. Culling is dead-code elimination for GPU work ([theory refresher](/posts/frame-graph-theory/#sorting-and-culling)) — a single backward walk marks the final pass alive, then propagates through `dependsOn` edges:

{{< code-diff title="v2 — Pass culling" collapsible="true" >}}
@@ frame_graph_v2.h — RenderPass gets alive flag @@
 struct RenderPass
 {
     ...
+    bool alive = false;  // survives the cull?
 };

@@ frame_graph_v2.cpp — Cull() @@
+// Dead-code elimination: walk backward from the final output pass, marking dependencies alive.
+void FrameGraph::Cull(const std::vector<PassIndex>& sorted)
+{
+    if (sorted.empty())
+        return;
+    passes[sorted.back()].alive = true;  // last pass = the final output (e.g. Present)
+    for (int i = static_cast<int>(sorted.size()) - 1; i >= 0; i--)
+    {
+        if (!passes[sorted[i]].alive)
+            continue;  // skip dead passes
+        for (PassIndex dep : passes[sorted[i]].dependsOn)
+            passes[dep].alive = true;  // my dependency is needed → keep it alive
+    }
+}
{{< /code-diff >}}

---

<span id="v2-barriers"></span>

### 🚧 Barrier insertion

GPUs need explicit state transitions between resource usages — color attachment → shader read, undefined → depth, etc. The graph already knows every resource's read/write history ([theory refresher](/posts/frame-graph-theory/#barriers)), so the compiler can figure out every transition *before* execution starts.

The idea: walk the sorted pass list, compare each resource's tracked state to what the pass needs, and record a barrier when they differ. This is where we introduce the **compile / execute split** — `Compile()` precomputes every transition into a `CompiledPlan`, and `Execute()` replays them. No state tracking at execution time, no decisions — just playback.

First, the new types. `ResourceState` is an enum with six values (Undefined, ColorAttachment, DepthAttachment, ShaderRead, UnorderedAccess, Present). `Barrier` pairs a resource index with old/new state. `ResourceEntry` gains a `currentState` field, and `ImportResource` takes an initial state so the swapchain can start as `Present`:

{{< code-diff title="v2 — ResourceState, Barrier & ResourceEntry changes" collapsible="true" >}}
@@ frame_graph_v2.h — ResourceState enum + Barrier struct @@
+enum class ResourceState
+{
+    Undefined,
+    ColorAttachment,
+    DepthAttachment,
+    ShaderRead,
+    UnorderedAccess,
+    Present
+};
+
+// A single resource-state transition.
+struct Barrier
+{
+    ResourceIndex resourceIndex;  // which resource to transition
+    ResourceState oldState;       // state before this pass
+    ResourceState newState;       // state this pass requires
+};

@@ frame_graph_v2.h — ResourceEntry gets currentState @@
 struct ResourceEntry
 {
     ...
+    ResourceState currentState = ResourceState::Undefined;
 };

@@ frame_graph_v2.h — ImportResource() accepts initial state @@
-    ResourceHandle ImportResource(const ResourceDesc& desc);
+    ResourceHandle ImportResource(
+        const ResourceDesc& desc,
+        ResourceState initialState = ResourceState::Undefined);
{{< /code-diff >}}

With those types in place, we introduce the **compile / execute split**. `CompiledPlan` holds the topological order and a 2D barrier array (`barriers[orderIdx]` = transitions before that pass). `Compile()` returns a plan; `Execute()` replays it. `CreateResource` / `ImportResource` gain a fourth field for initial state:

{{< code-diff title="v2 — CompiledPlan, Compile/Execute split & updated constructors" collapsible="true" >}}
@@ frame_graph_v2.h — CompiledPlan + Compile/Execute split @@
+    struct CompiledPlan
+    {
+        std::vector<PassIndex> sorted;               // topological execution order
+        std::vector<std::vector<Barrier>> barriers;  // barriers[orderIdx] → pre-pass transitions
+    };
+
+    CompiledPlan Compile();
+    void Execute(const CompiledPlan& plan);
     void Execute();  // convenience: compile + execute in one call

@@ frame_graph_v2.cpp — CreateResource / ImportResource updated for ResourceState @@
 ResourceHandle FrameGraph::CreateResource(const ResourceDesc& desc)
 {
-    entries.push_back({desc, {{}}});
+    entries.push_back({desc, {{}}, ResourceState::Undefined, false});
     return {static_cast<ResourceIndex>(entries.size() - 1)};
 }

-ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc)
+ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc, ResourceState initialState)
 {
-    entries.push_back({desc, {{}}, /*imported=*/true});
+    entries.push_back({desc, {{}}, initialState, true});
     return {static_cast<ResourceIndex>(entries.size() - 1)};
 }
{{< /code-diff >}}

With the type system in place, `ComputeBarriers()` walks the sorted pass list. For each surviving pass it first infers the required state for every resource the pass touches. `IsUAV` checks the readWrites list; `StateForUsage` maps usage to one of the six states (`ShaderRead` for reads, `ColorAttachment` or `DepthAttachment` for writes based on format, `UnorderedAccess` for UAVs):

{{< code-diff title="v2 — ComputeBarriers() — state inference helpers" collapsible="true" >}}
@@ frame_graph_v2.cpp — ComputeBarriers() state inference @@
+// Walk sorted passes, compare tracked state to each resource's needed state, record transitions.
+std::vector<std::vector<Barrier>> FrameGraph::ComputeBarriers(const std::vector<PassIndex>& sorted)
+{
+    std::vector<std::vector<Barrier>> result(sorted.size());
+    for (PassIndex orderIdx = 0; orderIdx < sorted.size(); orderIdx++)
+    {
+        PassIndex passIdx = sorted[orderIdx];
+        if (!passes[passIdx].alive)
+            continue;
+
+        auto IsUAV = [&](ResourceHandle h)
+        {
+            for (auto& rw : passes[passIdx].readWrites)
+                if (rw.index == h.index)
+                    return true;
+            return false;
+        };
+        auto StateForUsage = [&](ResourceHandle h, bool isWrite)
+        {
+            if (IsUAV(h))
+                return ResourceState::UnorderedAccess;
+            if (isWrite)
+                return (entries[h.index].desc.format == Format::D32F) ? ResourceState::DepthAttachment
+                                                                     : ResourceState::ColorAttachment;
+            return ResourceState::ShaderRead;
+        };
{{< /code-diff >}}

With the required state known, `recordTransition` compares it to the resource's tracked `currentState`. When they differ it records a `Barrier` (resource index + old/new state) and updates the tracker. Two loops fire it — once for reads, once for writes — producing a 2D vector where `barriers[orderIdx]` holds every transition that must fire before that pass runs:

{{< code-diff title="v2 — ComputeBarriers() — record transitions" collapsible="true" >}}
@@ frame_graph_v2.cpp — ComputeBarriers() transition recording @@
+        auto recordTransition = [&](ResourceHandle h, bool isWrite)
+        {
+            ResourceState needed = StateForUsage(h, isWrite);
+            if (entries[h.index].currentState != needed)
+            {
+                result[orderIdx].push_back({h.index, entries[h.index].currentState, needed});
+                entries[h.index].currentState = needed;
+            }
+        };
+        for (auto& h : passes[passIdx].reads)
+            recordTransition(h, false);
+        for (auto& h : passes[passIdx].writes)
+            recordTransition(h, true);
+    }
+    return result;
+}
{{< /code-diff >}}

`EmitBarriers()` replays those transitions on the GPU — in production this maps to `vkCmdPipelineBarrier` (Vulkan) or `ID3D12GraphicsCommandList::ResourceBarrier` (D3D12). For our MVP it's a one-liner stub:

{{< code-diff title="v2 — EmitBarriers()" collapsible="true" >}}
@@ frame_graph_v2.cpp — EmitBarriers() (replay) @@
+// Replay precomputed transitions — in production this calls the GPU API.
+void FrameGraph::EmitBarriers(const std::vector<Barrier>& barriers)
+{
+    for (auto& b : barriers)
+    {
+        // vkCmdPipelineBarrier / ResourceBarrier
+    }
+}
{{< /code-diff >}}

`Compile()` chains all four stages — edges, sort, cull, barriers — into a `CompiledPlan`. `Execute()` walks the plan in order: emit precomputed barriers, call the pass's execute lambda, repeat. A convenience overload does both in one call:

{{< code-diff title="v2 — Compile() & Execute()" collapsible="true" >}}
@@ frame_graph_v2.cpp — Compile() + Execute() @@
+// Full compile pipeline: sort → cull → precompute barriers. Returns a self-contained plan.
+FrameGraph::CompiledPlan FrameGraph::Compile()
+{
+    BuildEdges();
+    auto sorted = TopoSort();
+    Cull(sorted);
+    auto barriers = ComputeBarriers(sorted);
+    return {std::move(sorted), std::move(barriers)};
+}
+
+// Pure playback — emit precomputed barriers, call execute lambdas. No analysis.
+void FrameGraph::Execute(const CompiledPlan& plan)
+{
+    for (PassIndex orderIdx = 0; orderIdx < plan.sorted.size(); orderIdx++)
+    {
+        PassIndex passIdx = plan.sorted[orderIdx];
+        if (!passes[passIdx].alive)
+            continue;
+        EmitBarriers(plan.barriers[orderIdx]);
+        passes[passIdx].Execute(/* &cmdList */);
+    }
+    passes.clear();
+    entries.clear();
+}
+
+void FrameGraph::Execute()
+{
+    Execute(Compile());
+}
{{< /code-diff >}}

All four pieces — versioning, sorting, culling, barriers — compose into `Compile()`. Each step feeds the next: versioning creates edges, edges feed the sort, the sort enables culling, and the surviving sorted passes get precomputed barriers. `Execute()` is pure playback.

---

### 🧩 Full v2 source

<div style="margin:.6em 0;font-size:.84em;opacity:.65;line-height:1.5;">
ℹ The full source files below include <code>printf</code> diagnostics (topo-sort order, culling results, barrier transitions) that are omitted from the diffs above to keep the focus on structure. These diagnostics are invaluable for debugging — read through them in the source.
</div>

{{< include-code file="frame_graph_v2.h" lang="cpp" compact="true" >}}
{{< include-code file="frame_graph_v2.cpp" lang="cpp" compact="true" >}}
{{< include-code file="example_v2.cpp" lang="cpp" compile="true" deps="frame_graph_v2.h,frame_graph_v2.cpp" compact="true" >}}

That's three of the four intro promises delivered — automatic ordering, barrier insertion, and dead-pass culling — plus a clean compile/execute split that v3 will extend. The only piece missing: resources still live for the entire frame. Version 3 fixes that with lifetime analysis and memory aliasing.

UE5's RDG follows the same pattern. When you call `FRDGBuilder::AddPass`, RDG builds the dependency graph from your declared reads/writes, topologically sorts it, culls dead passes, computes barriers, and stores them in the compiled plan — all before recording a single GPU command.

---

## 💾 MVP v3 — Lifetimes & Aliasing

<div class="ds-callout ds-callout--info">
🎯 <strong>Goal:</strong> Non-overlapping transient resources share physical memory — automatic VRAM aliasing with savings that depend on pass topology and resolution (Frostbite reported ~50% on BF1's deferred pipeline).
</div>

V2 gives us ordering, culling, and barriers — but every transient resource still owns its own VRAM for the entire frame, even when it's only alive for two passes out of twelve. A 1080p G-Buffer alone eats ~32 MB; the depth target another ~8 MB — and both are dead after lighting. Meanwhile the post-process chain needs similarly-sized targets that could reuse that exact same memory, because the lifetimes never overlap ([theory refresher](/posts/frame-graph-theory/#allocation-and-aliasing)). That's what v3 automates.

The implementation adds two data structures — `Lifetime` (first/last sorted-pass index per resource) and `PhysicalBlock` (a reusable heap slot) — and extends `Barrier` with aliasing context. First, the lifetime scan. It walks the sorted pass list and records when each transient resource is first touched and last touched:

{{< code-diff title="v3 — New data structures: lifetime tracking & aliasing barriers" collapsible="true" >}}
@@ frame_graph_v3.h — PhysicalBlock, Lifetime structs @@
+// A physical memory slot — multiple virtual resources can reuse it if their lifetimes don't overlap.
+struct PhysicalBlock
+{
+    uint32_t sizeBytes = 0;        // block size (aligned)
+    PassIndex availAfter = 0;      // free after this sorted pass
+};
+
+// Per-resource lifetime in sorted-pass indices — drives aliasing decisions.
+struct Lifetime
+{
+    PassIndex firstUse = UINT32_MAX;  // first sorted pass that touches this resource
+    PassIndex lastUse = 0;            // last sorted pass that touches this resource
+    bool isTransient = true;          // false for imported resources (externally owned)
+};
+
@@ frame_graph_v3.h — Barrier extended with aliasing fields @@
 // Base Barrier already defined in v2 — v3 adds aliasing context.
 struct Barrier
 {
     ResourceIndex resourceIndex;
     ResourceState oldState;
     ResourceState newState;
+    bool isAliasing = false;                 // aliasing barrier (block changes occupant)
+    ResourceIndex aliasBefore = UINT32_MAX;  // resource being evicted
 };
{{< /code-diff >}}

To alias at the heap level, we need to know sizes. `AllocSize()` computes an aligned allocation size per resource — `width × height × BytesPerPixel`, rounded up to 64 KB (the same placement alignment real GPUs enforce). The diff adds `AllocSize()`, `AlignUp()`, and `BytesPerPixel()`:

{{< code-diff title="v3 — Allocation helpers (AllocSize, alignment)" collapsible="true" >}}
@@ frame_graph_v3.h — Allocation helpers @@
+// Minimum placement alignment for aliased heap resources (real APIs enforce similar, e.g. 64 KB).
+static constexpr uint32_t kPlacementAlignment = 65536;  // 64 KB
+
+inline uint32_t AlignUp(uint32_t value, uint32_t alignment)
+{
+    return (value + alignment - 1) & ~(alignment - 1);
+}
+
+inline uint32_t BytesPerPixel(Format fmt)
+{
+    switch (fmt)
+    {
+        case Format::R8:
+            return 1;
+        case Format::RGBA8:
+            return 4;
+        case Format::D32F:
+            return 4;
+        case Format::RGBA16F:
+            return 8;
+        default:
+            return 4;
+    }
+}
+
+// Aligned allocation size — real drivers add row padding/tiling; we approximate with a round-up.
+inline uint32_t AllocSize(const ResourceDesc& desc)
+{
+    uint32_t raw = desc.width * desc.height * BytesPerPixel(desc.format);
+    return AlignUp(raw, kPlacementAlignment);
+}
{{< /code-diff >}}

`ScanLifetimes()` walks the sorted pass list and records each transient resource's first and last use (as sorted-pass indices). Imported resources are excluded. The resulting intervals drive the aliasing allocator — non-overlapping intervals can share one physical block:

{{< code-diff title="v3 — ScanLifetimes()" collapsible="true" >}}
@@ frame_graph_v3.cpp — ScanLifetimes() @@
+// Record each resource's first/last use in sorted order — non-overlapping intervals can share memory.
+std::vector<Lifetime> FrameGraph::ScanLifetimes(const std::vector<PassIndex>& sorted)
+{
+    std::vector<Lifetime> life(entries.size());
+
+    // Imported resources (e.g. swapchain) are externally owned — exclude from aliasing.
+    for (ResourceIndex i = 0; i < entries.size(); i++)
+    {
+        if (entries[i].imported)
+            life[i].isTransient = false;
+    }
+
+    // Update first/last use for every resource each surviving pass touches.
+    for (PassIndex order = 0; order < sorted.size(); order++)
+    {
+        PassIndex passIdx = sorted[order];
+        if (!passes[passIdx].alive)
+            continue;
+
+        for (auto& h : passes[passIdx].reads)
+        {
+            life[h.index].firstUse = std::min(life[h.index].firstUse, order);
+            life[h.index].lastUse  = std::max(life[h.index].lastUse,  order);
+        }
+        for (auto& h : passes[passIdx].writes)
+        {
+            life[h.index].firstUse = std::min(life[h.index].firstUse, order);
+            life[h.index].lastUse  = std::max(life[h.index].lastUse,  order);
+        }
+    }
+    return life;
+}
{{< /code-diff >}}

This requires **placed resources** at the API level — GPU memory allocated from a heap, with resources bound to offsets within it. In D3D12, that means `ID3D12Heap` + `CreatePlacedResource`. In Vulkan, `VkDeviceMemory` + `vkBindImageMemory` at different offsets. Without placed resources (i.e. `CreateCommittedResource` or Vulkan dedicated allocations), each resource gets its own memory and aliasing is impossible — which is exactly why the graph's allocator needs to work with heaps.

With lifetimes in hand, the greedy free-list allocator is straightforward. Sort resources by `firstUse`, walk them in order, and for each one either reuse an existing physical block whose previous occupant has finished — or allocate a new one. `CompiledPlan` gains a `mapping` vector (virtual resource → physical block), and `ComputeBarriers()` gains a Phase 1 that emits aliasing barriers whenever a block changes occupant:

{{< code-diff title="v3 — Header changes (BlockIndex, CompiledPlan, new methods)" collapsible="true" >}}
@@ frame_graph_v3.h — BlockIndex typedef @@
+using BlockIndex = uint32_t;   // index into the physical-block free list

@@ frame_graph_v3.h — CompiledPlan extended with mapping @@
 struct CompiledPlan
 {
     std::vector<PassIndex> sorted;
+    std::vector<BlockIndex> mapping;               // mapping[ResourceIndex] → physical block
     std::vector<std::vector<Barrier>> barriers;
 };

@@ frame_graph_v3.h — new private methods @@
+    std::vector<Lifetime>   ScanLifetimes(const std::vector<PassIndex>& sorted);
+    std::vector<BlockIndex> AliasResources(const std::vector<Lifetime>& lifetimes);
+    ResourceState StateForUsage(PassIndex passIdx, ResourceHandle h, bool isWrite) const;
-    std::vector<std::vector<Barrier>> ComputeBarriers(const std::vector<PassIndex>& sorted);
+    std::vector<std::vector<Barrier>> ComputeBarriers(const std::vector<PassIndex>& sorted,
+                                                       const std::vector<BlockIndex>& mapping);
{{< /code-diff >}}

The greedy free-list allocator implements the aliasing strategy. First it builds a `firstUse`-sorted index so resources are processed in the order they appear in the frame:

{{< code-diff title="v3 — AliasResources() setup & sorting" collapsible="true" >}}
@@ frame_graph_v3.cpp — AliasResources() setup @@
+// Greedy first-fit: sort by firstUse, reuse any free block that fits, else allocate a new one.
+std::vector<BlockIndex> FrameGraph::AliasResources(const std::vector<Lifetime>& lifetimes)
+{
+    std::vector<PhysicalBlock> freeList;
+    std::vector<BlockIndex> mapping(entries.size(), UINT32_MAX);
+
+    // Process resources in the order they're first used.
+    std::vector<ResourceIndex> indices(entries.size());
+    std::iota(indices.begin(), indices.end(), 0);
+    std::sort(
+        indices.begin(),
+        indices.end(),
+        [&](ResourceIndex a, ResourceIndex b)
+        {
+            return lifetimes[a].firstUse < lifetimes[b].firstUse;
+        });
{{< /code-diff >}}

For each transient resource, scan existing physical blocks for one that is both free (previous occupant's `lastUse` < this resource's `firstUse`) and large enough. If found, reuse it; otherwise allocate a new block. The result is a `mapping` vector — `mapping[ResourceIndex]` → physical block index:

{{< code-diff title="v3 — AliasResources() allocation loop" collapsible="true" >}}
@@ frame_graph_v3.cpp — AliasResources() allocation loop @@
+    for (ResourceIndex resIdx : indices)
+    {
+        if (!lifetimes[resIdx].isTransient)
+            continue;      // skip imported resources
+        if (lifetimes[resIdx].firstUse == UINT32_MAX)
+            continue;  // never used
+
+        uint32_t needed = AllocSize(entries[resIdx].desc);
+        bool reused = false;
+
+        // Scan existing blocks — can we reuse one that's now free?
+        for (BlockIndex b = 0; b < freeList.size(); b++)
+        {
+            if (freeList[b].availAfter < lifetimes[resIdx].firstUse
+                && freeList[b].sizeBytes >= needed)
+            {
+                mapping[resIdx] = b;         // reuse this block
+                freeList[b].availAfter = lifetimes[resIdx].lastUse;  // extend occupancy
+                reused = true;
+                break;
+            }
+        }
+
+        if (!reused)
+        {  // no fit found → allocate a new physical block
+            mapping[resIdx] = static_cast<BlockIndex>(freeList.size());
+            freeList.push_back({needed, lifetimes[resIdx].lastUse});
+        }
+    }
+    return mapping;
+}
{{< /code-diff >}}

`Compile()` chains all stages: build edges → topo-sort → cull → scan lifetimes → alias → compute barriers. The diff shows the updated `Compile()` body and the new `StateForUsage()` method (extracted from v2's inline lambda so both phases can reuse it). `StateForUsage` maps a resource handle + read/write flag to a `ResourceState` — UAV handles get `UnorderedAccess`, depth-format writes get `DepthAttachment`, other writes get `ColorAttachment`, reads get `ShaderRead`:

{{< code-diff title="v3 — Updated Compile() & state inference" collapsible="true" >}}
@@ frame_graph_v3.cpp — Compile() extended with lifetime analysis + aliasing @@
 FrameGraph::CompiledPlan FrameGraph::Compile()
 {
     BuildEdges();
     auto sorted   = TopoSort();
     Cull(sorted);
+    auto lifetimes = ScanLifetimes(sorted);      // when is each resource alive?
+    auto mapping   = AliasResources(lifetimes);  // share memory where lifetimes don't overlap
-    auto barriers  = ComputeBarriers(sorted);
+    auto barriers  = ComputeBarriers(sorted, mapping); // extended: also emits aliasing transitions
-    return { std::move(sorted), std::move(barriers) };
+    return { std::move(sorted), std::move(mapping), std::move(barriers) };
 }

@@ frame_graph_v3.cpp — StateForUsage() extracted as class method @@
+// Infer the ResourceState a pass needs for a given resource handle.
+ResourceState FrameGraph::StateForUsage(PassIndex passIdx, ResourceHandle h, bool isWrite) const
+{
+    for (auto& rw : passes[passIdx].readWrites)
+        if (rw.index == h.index)
+            return ResourceState::UnorderedAccess;
+    if (isWrite)
+        return (entries[h.index].desc.format == Format::D32F)
+            ? ResourceState::DepthAttachment : ResourceState::ColorAttachment;
+    return ResourceState::ShaderRead;
+}
{{< /code-diff >}}

`ComputeBarriers()` now runs two phases per pass. The function signature gains a `mapping` parameter (from `AliasResources`), and a `blockOwner` vector tracks which virtual resource currently occupies each physical block. First, the setup and handle deduplication (UAVs appear in both reads and writes — we collect unique handles once):

{{< code-diff title="v3 — ComputeBarriers() setup & handle dedup" collapsible="true" >}}
@@ frame_graph_v3.cpp — ComputeBarriers() rewritten with aliasing @@
+// v3 ComputeBarriers: two phases per pass instead of v2's one.
+//   Phase 1 — aliasing:  did this physical block change occupant? If so, emit an aliasing barrier.
+//   Phase 2 — state:     did this resource's state change? If so, emit a state-transition barrier.

 std::vector<std::vector<Barrier>> FrameGraph::ComputeBarriers(
         const std::vector<PassIndex>& sorted,
         const std::vector<BlockIndex>& mapping)
 {
     std::vector<std::vector<Barrier>> result(sorted.size());

+    // blockOwner[block] = which virtual resource currently occupies it.
+    std::vector<ResourceIndex> blockOwner;
+    {
+        BlockIndex maxBlock = 0;
+        for (auto m : mapping)
+            if (m != UINT32_MAX)
+                maxBlock = std::max(maxBlock, m + 1);
+        blockOwner.assign(maxBlock, UINT32_MAX);
+    }

     for (PassIndex orderIdx = 0; orderIdx < sorted.size(); orderIdx++)
     {
         PassIndex passIdx = sorted[orderIdx];
         if (!passes[passIdx].alive)
             continue;

+        // ── Collect unique handles (ReadWrite puts h in both reads & writes) ──
+        std::vector<std::pair<ResourceHandle, bool>> unique;  // {handle, isWrite}
+        std::unordered_set<ResourceIndex> seen;
+        for (auto& h : passes[passIdx].reads)
+            if (seen.insert(h.index).second)
+                unique.push_back({h, false});
+        for (auto& h : passes[passIdx].writes)
+        {
+            if (seen.insert(h.index).second)
+            {
+                unique.push_back({h, true});
+            }
+            else
+            {
+                // already in reads — upgrade to write (UAV)
+                for (auto& [uh, w] : unique)
+                    if (uh.index == h.index)
+                    {
+                        w = true;
+                        break;
+                    }
+            }
+        }
{{< /code-diff >}}

**Phase 1 (aliasing):** for each unique handle, check if its physical block was previously occupied by a different resource — if so, emit an aliasing barrier so the GPU flushes caches. **Phase 2 (state transitions):** same as v2 — compare tracked state to needed state, emit a transition when they differ:

{{< code-diff title="v3 — ComputeBarriers() Phase 1 & 2" collapsible="true" >}}
@@ frame_graph_v3.cpp — Phase 1 (aliasing) + Phase 2 (state transitions) @@
+        // ── Phase 1: aliasing barriers ──────────────────────────
+        for (auto& [h, _] : unique)
+        {
+            BlockIndex block = mapping[h.index];
+            if (block == UINT32_MAX)
+                continue;  // imported — no aliasing
+            if (blockOwner[block] != UINT32_MAX && blockOwner[block] != h.index)
+            {
+                result[orderIdx].push_back(
+                    {h.index, ResourceState::Undefined, ResourceState::Undefined, true, blockOwner[block]});
+            }
+            blockOwner[block] = h.index;  // update current occupant
+        }
+
+        // ── Phase 2: state-transition barriers (same as v2) ────
+        for (auto& [h, isWrite] : unique)
+        {
+            ResourceState needed = StateForUsage(passIdx, h, isWrite);
+            if (entries[h.index].currentState != needed)
+            {
+                result[orderIdx].push_back(
+                    {h.index, entries[h.index].currentState, needed});
+                entries[h.index].currentState = needed;
+            }
+        }
     }
     return result;
 }
{{< /code-diff >}}

`EmitBarriers()` grows a branch to dispatch the two barrier types — aliasing transitions go to the heap-level API, state transitions go to the per-resource API:

{{< code-diff title="v3 — EmitBarriers() with aliasing dispatch" collapsible="true" >}}
@@ frame_graph_v3.cpp — EmitBarriers() extended with aliasing handling @@
 void FrameGraph::EmitBarriers(const std::vector<Barrier>& barriers)
 {
     for (auto& b : barriers)
     {
-        // (v2: only state transitions)
+        if (b.isAliasing)
+        {
+            // D3D12: D3D12_RESOURCE_BARRIER_TYPE_ALIASING / Vulkan: memory barrier on the shared heap region.
+        }
+        else
+        {
             // D3D12: ResourceBarrier(StateBefore→StateAfter) / Vulkan: vkCmdPipelineBarrier(oldLayout→newLayout).
+        }
     }
 }
{{< /code-diff >}}

`Execute()` stays unchanged — still pure playback of the compiled plan. All the new logic lives in `Compile()`, which now runs five stages instead of three: build edges → topo-sort → cull → **scan lifetimes → alias → compute barriers**. The last three stages add ~170 lines on top of v2.

The aliasing barriers matter for correctness, not just bookkeeping. When a physical block changes occupant, the GPU's L2 cache may still hold stale data from the previous resource. D3D12 exposes this as `D3D12_RESOURCE_BARRIER_TYPE_ALIASING`; Vulkan uses a `VkMemoryBarrier` on the heap region covering both the outgoing and incoming resource. Omitting these leads to corruption that's timing-dependent and GPU-vendor-specific — exactly the kind of bug that only shows up in the field.

<div class="ds-callout ds-callout--info" style="font-size:.88em;">
📝 <strong>Alignment and real GPU sizing.</strong>&ensp;
Our <code>AllocSize()</code> rounds up to a 64 KB placement alignment — the same constraint real GPUs enforce when placing resources into shared heaps. Without alignment, two resources that appear to fit in the same block would overlap at the hardware level. The raw <code>BytesPerPixel()</code> calculation is still a simplification: production allocators query the driver for actual row padding, tiling overhead, and per-format alignment. The aliasing algorithm itself is unchanged — you just swap the size input.
</div>

That wraps v3. Starting from v2's compile/execute split, we added lifetime analysis, a greedy free-list allocator, and aliasing-aware barrier insertion — the same architecture Frostbite described at GDC 2017, and the same approach UE5 uses today for every transient `FRDGTexture` created through `FRDGBuilder::CreateTexture`. The graph now owns the full lifecycle: declare virtual resources → analyze dependencies → pack physical memory → precompute every barrier → execute as pure playback.

---

### 🧩 Full v3 source

{{< include-code file="frame_graph_v3.h" lang="cpp" compact="true" >}}
{{< include-code file="frame_graph_v3.cpp" lang="cpp" compact="true" >}}
{{< include-code file="example_v3.cpp" lang="cpp" compile="true" deps="frame_graph_v3.h,frame_graph_v3.cpp" compact="true" >}}

---

### ✅ What the MVP delivers

Here's the full per-frame lifecycle — the same three-phase architecture from [Part I](/posts/frame-graph-theory/), now backed by real code:

<div style="margin:1.2em 0;display:grid;grid-template-columns:repeat(3,1fr);gap:.8em;">
  <div style="padding:.8em 1em;border-radius:10px;border-top:3px solid var(--ds-info);background:rgba(var(--ds-info-rgb),.04);">
    <div style="font-weight:800;font-size:.88em;margin-bottom:.5em;color:var(--ds-info-light);">① Declare</div>
    <div style="font-size:.84em;line-height:1.6;opacity:.85;">
      Each <code>AddPass</code> runs its setup lambda:<br>
      • declare reads &amp; writes<br>
      • request virtual resources<br>
      • version tracking builds edges
    </div>
    <div style="margin-top:.5em;padding:.3em .5em;border-radius:5px;background:rgba(var(--ds-info-rgb),.08);font-size:.76em;line-height:1.4;border:1px solid rgba(var(--ds-info-rgb),.12);">
      <strong>Zero GPU work.</strong> Resources are descriptions — no memory allocated yet.
    </div>
  </div>
  <div style="padding:.8em 1em;border-radius:10px;border-top:3px solid var(--ds-code);background:rgba(var(--ds-code-rgb),.04);">
    <div style="font-weight:800;font-size:.88em;margin-bottom:.5em;color:var(--ds-code-light);">② Compile</div>
    <div style="font-size:.84em;line-height:1.6;opacity:.85;">
      All automatic, all linear-time:<br>
      • <strong>sort</strong> — topo order (Kahn's)<br>
      • <strong>cull</strong> — kill dead passes<br>
      • <strong>scan lifetimes</strong> — first/last use<br>
      • <strong>alias</strong> — free-list reuse<br>
      • <strong>compute barriers</strong> — detect state transitions
    </div>
    <div style="margin-top:.5em;padding:.3em .5em;border-radius:5px;background:rgba(var(--ds-code-rgb),.08);font-size:.76em;line-height:1.4;border:1px solid rgba(var(--ds-code-rgb),.12);">
      Produces a <code>CompiledPlan</code> — execution order, memory mapping, <em>and</em> every barrier. No GPU work yet.
    </div>
  </div>
  <div style="padding:.8em 1em;border-radius:10px;border-top:3px solid var(--ds-success);background:rgba(var(--ds-success-rgb),.04);">
    <div style="font-weight:800;font-size:.88em;margin-bottom:.5em;color:var(--ds-success);">③ Execute</div>
    <div style="font-size:.84em;line-height:1.6;opacity:.85;">
      Pure playback — no analysis:<br>
      • <strong>submit precomputed barriers</strong><br>
      • call execute lambda<br>
      • resources already aliased &amp; bound
    </div>
    <div style="margin-top:.5em;padding:.3em .5em;border-radius:5px;background:rgba(var(--ds-success-rgb),.08);font-size:.76em;line-height:1.4;border:1px solid rgba(var(--ds-success-rgb),.12);">
      <strong>No decisions.</strong> Compile analyzed + decided. Execute just submits.
    </div>
  </div>
</div>

~350 lines of C++17. No external dependencies, no allocator library, no render backend — just the algorithmic core that production engines build on. Every concept from [Part I](/posts/frame-graph-theory/) — virtual resources, dependency edges, topological ordering, dead-pass culling, barrier inference, lifetime analysis, and VRAM aliasing — now exists as running, compilable code.

### 🔮 What's next

The MVP runs everything on a single queue, issues barriers as immediate full-pipeline stalls, and uses the simplest possible allocation strategy. Production engines push past all three constraints:

- **Async compute** — the graph already encodes which passes are independent. Mapping those onto a second hardware queue lets the GPU overlap compute work (SSAO, light culling, particle simulation) with graphics work on the same frame, recovering otherwise-idle ALU cycles.
- **Split barriers** — instead of stalling the full pipeline at the point of use, split barriers separate the "start transitioning" signal from the "must be done" signal. The driver gets a window to schedule the transition in parallel with unrelated work — often eliminating the stall entirely.

Both features plug directly into the `CompiledPlan` architecture: async compute adds a queue assignment per pass, and split barriers replace single `Barrier` entries with begin/end pairs. The graph's structure doesn't change — only the execution strategy does.

[Part III — Beyond MVP](../frame-graph-advanced/) walks through both upgrades with code diffs against the v3 base we just built.

---

<div class="ds-article-footer">
  <a href="../frame-graph-theory/">
    ← Previous: Part I — Theory
  </a>
  <a href="../frame-graph-advanced/">
    Next: Part III — Beyond MVP →
  </a>
</div>
