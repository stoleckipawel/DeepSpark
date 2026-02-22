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

<div style="margin:0 0 1.5em;padding:.7em 1em;border-radius:8px;background:rgba(var(--ds-indigo-rgb),.04);border:1px solid rgba(var(--ds-indigo-rgb),.12);font-size:.88em;line-height:1.6;opacity:.85;">
📖 <strong>Part II of IV.</strong>&ensp; <a href="../frame-graph-theory/">Theory</a> → <em>Build It</em> → <a href="../frame-graph-advanced/">Beyond MVP</a> → <a href="../frame-graph-production/">Production Engines</a>
</div>

*Part I laid out the theory — declare, compile, execute. Now we turn that blueprint into code. Three iterations, each one building on the last: v1 lays the scaffold, v2 adds dependency-driven execution order (topological sort, pass culling, and automatic barriers), and v3 introduces lifetime analysis so non-overlapping resources can share the same heap. Time to get our hands dirty.*

<!-- MVP progression — animated power-up timeline -->
<style>
@keyframes mvp-glow { 0%,100%{box-shadow:0 0 8px rgba(var(--ds-success-rgb),.25),0 0 0 3px rgba(var(--ds-success-rgb),.15);} 50%{box-shadow:0 0 20px rgba(var(--ds-success-rgb),.45),0 0 0 5px rgba(var(--ds-success-rgb),.2);} }
@keyframes mvp-bar-shine { 0%{background-position:200% 0;} 100%{background-position:-200% 0;} }
</style>
<div style="margin:1.6em 0 1.2em;position:relative;padding-left:3em;">
  <div style="margin-left:-3em;margin-bottom:1.4em;font-size:1.15em;font-weight:900;text-align:center;letter-spacing:.03em;">🧬 MVP Progression</div>
  <!-- vertical connector -->
  <div style="position:absolute;left:1.15em;top:3.2em;bottom:.8em;width:3px;background:linear-gradient(to bottom, var(--ds-info), var(--ds-code), var(--ds-success));border-radius:2px;opacity:.45;"></div>

  <!-- ── v1 ── -->
  <a href="#-v1--the-scaffold" style="text-decoration:none;color:inherit;display:block;position:relative;margin-bottom:1.6em;cursor:pointer;" onmouseover="this.querySelector('.mvp-card').style.transform='translateX(4px)';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-info-rgb),.5)'" onmouseout="this.querySelector('.mvp-card').style.transform='';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-info-rgb),.2)'">
    <div style="position:absolute;left:-3em;top:.3em;width:2.3em;height:2.3em;border-radius:50%;background:var(--ds-info);display:flex;align-items:center;justify-content:center;font-weight:900;font-size:.72em;color:#fff;box-shadow:0 0 0 3px rgba(var(--ds-info-rgb),.15);z-index:1;">v1</div>
    <div class="mvp-card" style="padding:.8em 1em;border-radius:10px;border:1.5px solid rgba(var(--ds-info-rgb),.2);background:linear-gradient(135deg,rgba(var(--ds-info-rgb),.07) 0%,transparent 60%);transition:all .2s ease;">
      <div style="display:flex;align-items:baseline;gap:.5em;margin-bottom:.3em;">
        <span style="font-weight:900;font-size:1em;color:var(--ds-info);">The Scaffold</span>
        <span style="font-size:.65em;font-weight:700;padding:.15em .5em;border-radius:9px;background:rgba(var(--ds-info-rgb),.12);color:var(--ds-info);white-space:nowrap;">~90 LOC</span>
      </div>
      <div style="font-size:.84em;line-height:1.5;opacity:.85;margin-bottom:.5em;">Pass declaration, virtual resources, linear execution.</div>
      <!-- unlocks -->
      <div style="display:flex;flex-wrap:wrap;gap:.35em;margin-bottom:.6em;">
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-info-rgb),.1);color:var(--ds-info);font-weight:700;">🔓 AddPass</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-info-rgb),.1);color:var(--ds-info);font-weight:700;">🔓 CreateResource</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-info-rgb),.1);color:var(--ds-info);font-weight:700;">🔓 ImportResource</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-info-rgb),.1);color:var(--ds-info);font-weight:700;">🔓 Execute()</span>
      </div>
      <!-- power bar -->
      <div style="height:8px;border-radius:4px;background:rgba(127,127,127,.08);overflow:hidden;">
        <div style="width:20%;height:100%;border-radius:4px;background:var(--ds-info);"></div>
      </div>
      <div style="display:flex;justify-content:space-between;margin-top:.25em;font-size:.6em;opacity:.4;font-weight:600;"><span>DECLARE</span><span>COMPILE</span><span>EXECUTE</span></div>
    </div>
  </a>

  <!-- ── v2 ── -->
  <a href="#-mvp-v2--dependencies--barriers" style="text-decoration:none;color:inherit;display:block;position:relative;margin-bottom:1.6em;cursor:pointer;" onmouseover="this.querySelector('.mvp-card').style.transform='translateX(4px)';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-code-rgb),.5)'" onmouseout="this.querySelector('.mvp-card').style.transform='';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-code-rgb),.2)'">
    <div style="position:absolute;left:-3em;top:.3em;width:2.3em;height:2.3em;border-radius:50%;background:var(--ds-code);display:flex;align-items:center;justify-content:center;font-weight:900;font-size:.72em;color:#fff;box-shadow:0 0 0 3px rgba(var(--ds-code-rgb),.18);z-index:1;">v2</div>
    <div class="mvp-card" style="padding:.8em 1em;border-radius:10px;border:1.5px solid rgba(var(--ds-code-rgb),.2);background:linear-gradient(135deg,rgba(var(--ds-code-rgb),.07) 0%,transparent 60%);transition:all .2s ease;">
      <div style="display:flex;align-items:baseline;gap:.5em;margin-bottom:.3em;">
        <span style="font-weight:900;font-size:1em;color:var(--ds-code);">Dependencies & Barriers</span>
        <span style="font-size:.65em;font-weight:700;padding:.15em .5em;border-radius:9px;background:rgba(var(--ds-code-rgb),.12);color:var(--ds-code);white-space:nowrap;">~260 LOC</span>
      </div>
      <div style="font-size:.84em;line-height:1.5;opacity:.85;margin-bottom:.5em;">Resource versioning → edges → topo-sort → dead-pass culling → automatic barrier insertion.</div>
      <!-- unlocks -->
      <div style="display:flex;flex-wrap:wrap;gap:.35em;margin-bottom:.6em;">
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-code-rgb),.1);color:var(--ds-code);font-weight:700;">🔓 read / write</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-code-rgb),.1);color:var(--ds-code);font-weight:700;">🔓 topo-sort</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-code-rgb),.1);color:var(--ds-code);font-weight:700;">🔓 pass culling</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-code-rgb),.1);color:var(--ds-code);font-weight:700;">🔓 auto barriers</span>
      </div>
      <!-- power bar -->
      <div style="height:8px;border-radius:4px;background:rgba(127,127,127,.08);overflow:hidden;">
        <div style="width:65%;height:100%;border-radius:4px;background:linear-gradient(90deg,var(--ds-info),var(--ds-code));"></div>
      </div>
      <div style="display:flex;justify-content:space-between;margin-top:.25em;font-size:.6em;opacity:.4;font-weight:600;"><span>DECLARE</span><span>COMPILE</span><span>EXECUTE</span></div>
    </div>
  </a>

  <!-- ── v3 ── -->
  <a href="#-mvp-v3--lifetimes--aliasing" style="text-decoration:none;color:inherit;display:block;position:relative;cursor:pointer;" onmouseover="this.querySelector('.mvp-card').style.transform='translateX(4px)';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-success-rgb),.6)'" onmouseout="this.querySelector('.mvp-card').style.transform='';this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-success-rgb),.3)'">
    <div style="position:absolute;left:-3em;top:.3em;width:2.3em;height:2.3em;border-radius:50%;background:var(--ds-success);display:flex;align-items:center;justify-content:center;font-weight:900;font-size:.72em;color:#fff;animation:mvp-glow 2.5s ease-in-out infinite;z-index:1;">v3</div>
    <div class="mvp-card" style="padding:.8em 1em;border-radius:10px;border:2px solid rgba(var(--ds-success-rgb),.3);background:linear-gradient(135deg,rgba(var(--ds-success-rgb),.09) 0%,rgba(var(--ds-success-rgb),.02) 40%,transparent 70%);transition:all .2s ease;box-shadow:0 2px 16px rgba(var(--ds-success-rgb),.08);">
      <div style="display:flex;align-items:baseline;gap:.5em;margin-bottom:.3em;">
        <span style="font-weight:900;font-size:1.05em;color:var(--ds-success);">Lifetimes & Aliasing</span>
        <span style="font-size:.65em;font-weight:700;padding:.15em .5em;border-radius:9px;background:rgba(var(--ds-success-rgb),.12);color:var(--ds-success);white-space:nowrap;">~400 LOC</span>
        <span style="font-size:.62em;font-weight:800;padding:.15em .5em;border-radius:9px;background:var(--ds-success);color:#fff;white-space:nowrap;">★ FULL MVP</span>
      </div>
      <div style="font-size:.84em;line-height:1.5;opacity:.85;margin-bottom:.5em;">Compile precomputes everything — sort, cull, lifetimes, aliasing, barriers — into a <code>CompiledPlan</code>.</div>
      <!-- unlocks -->
      <div style="display:flex;flex-wrap:wrap;gap:.35em;margin-bottom:.6em;">
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-success-rgb),.1);color:var(--ds-success);font-weight:700;">🔓 lifetime scan</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-success-rgb),.1);color:var(--ds-success);font-weight:700;">🔓 memory aliasing</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-success-rgb),.12);color:var(--ds-success);font-weight:700;">⚡ VRAM aliasing</span>
      </div>
      <!-- power bar — full, with animated shine -->
      <div style="height:8px;border-radius:4px;background:rgba(127,127,127,.08);overflow:hidden;">
        <div style="width:100%;height:100%;border-radius:4px;background:linear-gradient(90deg,var(--ds-info),var(--ds-code),var(--ds-success));background-size:200% 100%;animation:mvp-bar-shine 3s linear infinite;"></div>
      </div>
      <div style="display:flex;justify-content:space-between;margin-top:.25em;font-size:.6em;opacity:.4;font-weight:600;"><span>DECLARE</span><span>COMPILE</span><span>EXECUTE</span></div>
    </div>
  </a>
</div>

---

## 🏗 Architecture & API Decisions

We start from the API you *want* to write, then build toward it — starting with bare scaffolding and ending with automatic barriers and memory aliasing.

<!-- UML class diagram — API overview -->
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
  -uint32[] AliasResources(lifetimes)
  -Barrier[] ComputeBarriers(sortedPasses)
  -EmitBarriers(barriers)
}
class RenderPass{
  +string name
  +function Setup
  +function Execute
  +ResourceHandle[] reads
  +ResourceHandle[] writes
  +ResourceHandle[] readWrites ← UAV
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
class ResourceHandle{
  +ResourceIndex index
  +bool IsValid()
}
class ResourceDesc{
  +uint32 width
  +uint32 height
  +Format format
}
class ResourceVersion{
  +PassIndex writerPass
  +PassIndex[] readerPasses
}
class CompiledPlan{
  +PassIndex[] sortedPassOrder
  +uint32[] virtualToPhysicalBlock
  +Barrier[] barriersPerPass
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
  +PassIndex freeAfterPass
}
class Format{
  RGBA8
  RGBA16F
  R8
  D32F
}
class ResourceState{
  Undefined
  ColorAttachment
  DepthAttachment
  ShaderRead
  UnorderedAccess
  Present
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
<tr><td colspan="5" style="padding:.6em .6em .3em;font-weight:800;font-size:.85em;letter-spacing:.04em;color:var(--ds-info);border-bottom:1px solid rgba(var(--ds-info-rgb),.12);">DECLARE — how passes and resources enter the graph</td></tr>
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
<tr><td colspan="5" style="padding:.6em .6em .3em;font-weight:800;font-size:.85em;letter-spacing:.04em;color:var(--ds-code);border-bottom:1px solid rgba(var(--ds-code-rgb),.12);">COMPILE — what the graph analyser decides</td></tr>
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

<div style="margin:1em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid var(--ds-info);background:rgba(var(--ds-info-rgb),.04);font-size:.92em;line-height:1.6;">
🎯 <strong>Goal:</strong> Declare passes and virtual resources, execute in registration order — the skeleton that v2 and v3 build on.
</div>

Three types are all we need to start: a `ResourceDesc` (width, height, format — no GPU handle yet), a `ResourceHandle` that's just an index, and a `RenderPass` with setup + execute lambdas. The `FrameGraph` class owns arrays of both and runs passes in declaration order. No dependency tracking, no barriers — just the foundation that v2 and v3 build on.

{{< code-diff title="v1 — Resource types (frame_graph_v1.h)" >}}
@@ frame_graph_v1.h — Format, ResourceDesc, ResourceHandle @@
+enum class Format { RGBA8, RGBA16F, R8, D32F };
+
+struct ResourceDesc {
+    uint32_t width  = 0;
+    uint32_t height = 0;
+    Format   format = Format::RGBA8;
+};
+
+using ResourceIndex = uint32_t;  // readable alias for resource array indices
+
+// Lightweight handle — index into FrameGraph's resource array, no GPU memory involved.
+struct ResourceHandle {
+    ResourceIndex index = UINT32_MAX;
+    bool IsValid() const { return index != UINT32_MAX; }
+};
{{< /code-diff >}}

A pass is two lambdas — setup (runs now, wires the DAG) and execute (stored for later, records GPU commands). v1 doesn't use setup yet, but the slot is there for v2:

{{< code-diff title="v1 — RenderPass + FrameGraph class (frame_graph_v1.h)" >}}
@@ frame_graph_v1.h — RenderPass struct @@
+// A render pass: Setup wires the DAG (declares reads/writes), Execute records GPU commands.
+struct RenderPass {
+    std::string                        name;
+    std::function<void()>              Setup;    // build the DAG (v1: unused)
+    std::function<void(/*cmd list*/)>  Execute;  // record GPU commands
+};

@@ frame_graph_v1.h — FrameGraph class @@
+class FrameGraph {
+public:
+    ResourceHandle CreateResource(const ResourceDesc& desc);  // transient — graph owns lifetime
+    ResourceHandle ImportResource(const ResourceDesc& desc);  // external — e.g. swapchain backbuffer
+
+    // AddPass: runs setup immediately (wires DAG), stores execute for later.
+    template <typename SetupFn, typename ExecFn>
+    void AddPass(const std::string& name, SetupFn&& setup, ExecFn&& exec) {
+        passes.push_back({ name, std::forward<SetupFn>(setup),
+                                  std::forward<ExecFn>(exec) });
+        passes.back().Setup();  // run setup immediately — DAG is built here
+    }
+
+    void Execute();  // v1: just run in declaration order
+
+private:
+    std::vector<RenderPass>    passes;
+    std::vector<ResourceDesc>  resources;
+};
{{< /code-diff >}}

`Execute()` is the simplest possible loop — walk passes in order, call each callback, clear everything for the next frame:

{{< code-diff title="v1 — Implementation (frame_graph_v1.cpp)" >}}
@@ frame_graph_v1.cpp — CreateResource / ImportResource @@
+// No GPU memory is allocated yet — that happens at execute time.
+ResourceHandle FrameGraph::CreateResource(const ResourceDesc& desc) {
+    resources.push_back(desc);
+    return { static_cast<ResourceIndex>(resources.size() - 1) };
+}
+
+ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc) {
+    resources.push_back(desc);  // v1: same as create (no aliasing yet)
+    return { static_cast<ResourceIndex>(resources.size() - 1) };
+}

@@ frame_graph_v1.cpp — Execute() @@
+// v1 execute: just run passes in the order they were declared.
+void FrameGraph::Execute() {
+    printf("\n[1] Executing (declaration order -- no compile step):\n");
+    for (auto& pass : passes) {
+        printf("  >> exec: %s\n", pass.name.c_str());
+        pass.Execute(/* &cmdList */);
+    }
+    passes.clear();     // reset for next frame
+    resources.clear();
+}
{{< /code-diff >}}

Full source and runnable example:

{{< include-code file="frame_graph_v1.h" lang="cpp" compact="true" >}}
{{< include-code file="frame_graph_v1.cpp" lang="cpp" compact="true" >}}
{{< include-code file="example_v1.cpp" lang="cpp" compile="true" deps="frame_graph_v1.h,frame_graph_v1.cpp" compact="true" >}}

Compiles and runs — the execute lambdas are stubs, but the scaffolding is real. Every piece we add in v2 and v3 goes into this same `FrameGraph` class.

<div style="display:grid;grid-template-columns:1fr 1fr;gap:.8em;margin:1em 0;">
  <div style="padding:.7em 1em;border-radius:8px;border-left:4px solid var(--ds-success);background:rgba(var(--ds-success-rgb),.05);font-size:.9em;line-height:1.5;">
    <strong style="color:var(--ds-success);">✓ What it proves</strong><br>
    The lambda-based pass declaration pattern works. You can already compose passes without manual barrier calls (even though barriers are no-ops here).
  </div>
  <div style="padding:.7em 1em;border-radius:8px;border-left:4px solid var(--ds-danger);background:rgba(var(--ds-danger-rgb),.05);font-size:.9em;line-height:1.5;">
    <strong style="color:var(--ds-danger);">✗ What it lacks</strong><br>
    Executes passes in declaration order, creates every resource upfront. Correct but wasteful. Version 2 adds the graph.
  </div>
</div>

---

## 🔗 MVP v2 — Dependencies & Barriers

<div style="margin:1em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid var(--ds-info);background:rgba(var(--ds-info-rgb),.04);font-size:.92em;line-height:1.6;">
🎯 <strong>Goal:</strong> Automatic pass ordering, dead-pass culling, and barrier insertion — the graph now drives the GPU instead of you.
</div>

Four steps in strict order — each one's output is the next one's input:

<div style="margin:.8em 0 1.2em;display:grid;grid-template-columns:1fr auto 1fr auto 1fr auto 1fr;gap:0;align-items:stretch;border-radius:10px;overflow:hidden;border:1.5px solid rgba(var(--ds-indigo-rgb),.2);">
  <a href="#v2-versioning" style="padding:.7em .5em .5em;background:rgba(var(--ds-info-rgb),.05);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(var(--ds-info-rgb),.12)'" onmouseout="this.style.background='rgba(var(--ds-info-rgb),.05)'">
    <div style="font-size:.65em;font-weight:800;opacity:.45;margin-bottom:.15em;">STEP 1</div>
    <div style="font-size:1.2em;margin-bottom:.15em;">🔀</div>
    <div style="font-weight:800;font-size:.85em;color:var(--ds-info);">Versioning</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">reads/writes produce edges</div>
  </a>
  <div style="display:flex;align-items:center;font-size:1.1em;opacity:.35;padding:0 .1em;">→</div>
  <a href="#v2-toposort" style="padding:.7em .5em .5em;background:rgba(var(--ds-code-rgb),.05);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(var(--ds-code-rgb),.12)'" onmouseout="this.style.background='rgba(var(--ds-code-rgb),.05)'">
    <div style="font-size:.65em;font-weight:800;opacity:.45;margin-bottom:.15em;">STEP 2</div>
    <div style="font-size:1.2em;margin-bottom:.15em;">📦</div>
    <div style="font-weight:800;font-size:.85em;color:var(--ds-code);">Topo Sort</div>
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

Here's what changes from v1. The `ResourceDesc` array becomes `ResourceEntry` — each entry carries a version list. `RenderPass` gains dependency tracking fields. And two new methods, `Read()` and `Write()`, wire everything together:

{{< code-diff title="v1 → v2 — Resource versioning & dependency tracking" >}}
@@ frame_graph_v2.h — PassIndex alias, ResourceVersion, ResourceEntry @@
+using PassIndex = uint32_t;  // readable alias for pass array indices
+
+struct ResourceVersion {
+    PassIndex writerPass = UINT32_MAX;   // Each Read() links to the current version's writer → automatic dependency edge.
+    std::vector<PassIndex> readerPasses; // Each Write() to a resource creates a new version.
+    bool HasWriter() const { return writerPass != UINT32_MAX; }
+};
+
+// Replaces raw ResourceDesc — now tracks version history per resource.
+struct ResourceEntry {
+    ResourceDesc desc;
+    std::vector<ResourceVersion> versions;  // version 0, 1, 2...
+    bool imported = false;   // imported = externally owned (e.g. swapchain)
+};

@@ frame_graph_v2.h — RenderPass gets reads/writes/dependsOn @@
 struct RenderPass {
     std::string name;
     std::function<void()>             Setup;
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
-    std::vector<ResourceDesc>  resources;
+    std::vector<ResourceEntry> entries;  // now with versioning

@@ frame_graph_v2.cpp — CreateResource / ImportResource now use ResourceEntry @@
 ResourceHandle FrameGraph::CreateResource(const ResourceDesc& desc) {
-    resources.push_back(desc);
-    return { static_cast<ResourceIndex>(resources.size() - 1) };
+    entries.push_back({ desc, {{}} });
+    return { static_cast<ResourceIndex>(entries.size() - 1) };
 }

 ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc) {
-    resources.push_back(desc);
-    return { static_cast<ResourceIndex>(resources.size() - 1) };
+    entries.push_back({ desc, {{}}, /*imported=*/true });
+    return { static_cast<ResourceIndex>(entries.size() - 1) };
 }

@@ frame_graph_v2.cpp — Read() and Write() build dependency edges @@
+// Read: look up who last wrote this resource → add a dependency edge from that writer to this pass.
+void FrameGraph::Read(PassIndex passIdx, ResourceHandle h) {
+    auto& ver = entries[h.index].versions.back();   // current version
+    if (ver.HasWriter()) {
+        passes[passIdx].dependsOn.push_back(ver.writerPass);  // RAW edge
+    }
+    ver.readerPasses.push_back(passIdx);  // track who reads this version
+    passes[passIdx].reads.push_back(h);   // record for barrier insertion
+}
+
+// Write: bump the resource's version — previous readers/writers stay on older versions.
+void FrameGraph::Write(PassIndex passIdx, ResourceHandle h) {
+    entries[h.index].versions.push_back({});              // bump version
+    entries[h.index].versions.back().writerPass = passIdx; // this pass owns the new version
+    passes[passIdx].writes.push_back(h);                   // record for barrier insertion
+}
+
+// ReadWrite (UAV): read current version + bump it — the caller explicitly declares unordered access.
+void FrameGraph::ReadWrite(PassIndex passIdx, ResourceHandle h) {
+    auto& ver = entries[h.index].versions.back();
+    if (ver.HasWriter()) {
+        passes[passIdx].dependsOn.push_back(ver.writerPass);  // depend on previous writer
+    }
+    entries[h.index].versions.push_back({});              // bump version (it's a write)
+    entries[h.index].versions.back().writerPass = passIdx;
+    passes[passIdx].reads.push_back(h);      // appears in both lists (for barriers + lifetimes)
+    passes[passIdx].writes.push_back(h);
+    passes[passIdx].readWrites.push_back(h); // marks this handle as UAV for StateForUsage
+}
{{< /code-diff >}}

Every `Write()` pushes a new version. Every `Read()` finds the current version's writer and records a `dependsOn` edge. Those edges feed the next three steps.

---

<span id="v2-toposort"></span>

### 📊 Topological sort (Kahn's algorithm)

With edges in place, we need an execution order that respects every dependency. Kahn’s algorithm ([theory refresher](/posts/frame-graph-theory/#sorting-and-culling)) gives us one in O(V+E). `BuildEdges()` deduplicates the raw `dependsOn` entries and builds the adjacency list; `TopoSort()` does the zero-in-degree queue drain:

{{< code-diff title="v2 — Edge building + Kahn's topological sort" >}}
@@ frame_graph_v2.h — RenderPass gets successors + inDegree (for Kahn's) @@
 struct RenderPass {
     ...
+    std::vector<PassIndex> successors;     // passes that depend on this one
+    uint32_t inDegree = 0;                 // incoming edge count (Kahn's)
 };

@@ frame_graph_v2.cpp — BuildEdges() @@
+// Deduplicate raw dependsOn edges and build forward adjacency list (successors) for Kahn's algorithm.
+void FrameGraph::BuildEdges() {
+    for (PassIndex i = 0; i < passes.size(); i++) {
+        std::unordered_set<PassIndex> seen;
+        for (PassIndex dep : passes[i].dependsOn) {
+            if (seen.insert(dep).second) {       // first time seeing this edge?
+                passes[dep].successors.push_back(i);  // forward link: dep → i
+                passes[i].inDegree++;                  // i has one more incoming edge
+            }
+        }
+    }
+}

@@ frame_graph_v2.cpp — TopoSort() @@
+// Kahn's algorithm: dequeue zero-in-degree passes → valid execution order respecting all dependencies.
+std::vector<PassIndex> FrameGraph::TopoSort() {
+    std::queue<PassIndex> q;
+    std::vector<uint32_t> inDeg(passes.size());
+    for (PassIndex i = 0; i < passes.size(); i++) {
+        inDeg[i] = passes[i].inDegree;
+        if (inDeg[i] == 0) q.push(i);  // no dependencies → ready immediately
+    }
+    std::vector<PassIndex> order;
+    while (!q.empty()) {
+        PassIndex cur = q.front(); q.pop();
+        order.push_back(cur);
+        for (PassIndex succ : passes[cur].successors) {
+            if (--inDeg[succ] == 0)     // all of succ's dependencies done?
+                q.push(succ);           // succ is now ready
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

{{< code-diff title="v2 — Pass culling" >}}
@@ frame_graph_v2.h — RenderPass gets alive flag @@
 struct RenderPass {
     ...
+    bool alive = false;                    // survives the cull?
 };

@@ frame_graph_v2.cpp — Cull() @@
+// Dead-code elimination: walk backward from the final output pass, marking dependencies alive.
+void FrameGraph::Cull(const std::vector<PassIndex>& sorted) {
+    if (sorted.empty()) return;
+    passes[sorted.back()].alive = true;   // last pass = the final output (e.g. Present)
+    for (int i = static_cast<int>(sorted.size()) - 1; i >= 0; i--) {
+        if (!passes[sorted[i]].alive) continue;         // skip dead passes
+        for (PassIndex dep : passes[sorted[i]].dependsOn)
+            passes[dep].alive = true;   // my dependency is needed → keep it alive
+    }
+}
{{< /code-diff >}}

---

<span id="v2-barriers"></span>

### 🚧 Barrier insertion

GPUs need explicit state transitions between resource usages — color attachment → shader read, undefined → depth, etc. The graph already knows every resource's read/write history ([theory refresher](/posts/frame-graph-theory/#barriers)), so the compiler can figure out every transition *before* execution starts.

The idea: walk the sorted pass list, compare each resource's tracked state to what the pass needs, and record a barrier when they differ. In the final architecture (v3), this happens entirely during `Compile()` — every transition is precomputed and stored in `CompiledPlan`, and `Execute()` just replays them. But v2 doesn't have a compile/execute split yet, so `EmitBarriers()` runs inline during execution — compute the transition and submit it in one step. It's the simplest correct approach, and v3 will separate the two cleanly.

{{< code-diff title="v2 — Barrier insertion + Execute() rewrite" >}}
@@ frame_graph_v2.h — ResourceState enum @@
+enum class ResourceState { Undefined, ColorAttachment, DepthAttachment,
+                           ShaderRead, UnorderedAccess, Present };

@@ frame_graph_v2.h — ResourceEntry gets currentState @@
 struct ResourceEntry {
     ...
+    ResourceState currentState = ResourceState::Undefined;
 };

@@ frame_graph_v2.h — ImportResource() accepts initial state @@
-    ResourceHandle ImportResource(const ResourceDesc& desc);
+    ResourceHandle ImportResource(const ResourceDesc& desc,
+                                  ResourceState initialState = ResourceState::Undefined);

@@ frame_graph_v2.cpp — CreateResource / ImportResource updated for ResourceState @@
 ResourceHandle FrameGraph::CreateResource(const ResourceDesc& desc) {
-    entries.push_back({ desc, {{}} });
+    entries.push_back({ desc, {{}}, ResourceState::Undefined, false });
     return { static_cast<ResourceIndex>(entries.size() - 1) };
 }

-ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc) {
-    entries.push_back({ desc, {{}}, /*imported=*/true });
+ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc,
+                                          ResourceState initialState) {
+    entries.push_back({ desc, {{}}, initialState, true });
     return { static_cast<ResourceIndex>(entries.size() - 1) };
 }

@@ frame_graph_v2.cpp — EmitBarriers() @@
+// Compare tracked state to what this pass needs, emit a transition if they differ. v2 computes+submits inline; v3 splits into ComputeBarriers() + EmitBarriers().
+void FrameGraph::EmitBarriers(PassIndex passIdx) {
+    auto IsUAV = [&](ResourceHandle h) {
+        for (auto& rw : passes[passIdx].readWrites)
+            if (rw.index == h.index) return true;
+        return false;
+    };
+    auto StateForUsage = [&](ResourceHandle h, bool isWrite) {
+        if (IsUAV(h)) return ResourceState::UnorderedAccess;
+        if (isWrite)
+            return (entries[h.index].desc.format == Format::D32F)
+                ? ResourceState::DepthAttachment : ResourceState::ColorAttachment;
+        return ResourceState::ShaderRead;
+    };
+    for (auto& h : passes[passIdx].reads) {
+        ResourceState needed = StateForUsage(h, false);
+        if (entries[h.index].currentState != needed) {
+            entries[h.index].currentState = needed;
+        }
+    }
+    for (auto& h : passes[passIdx].writes) {
+        ResourceState needed = StateForUsage(h, true);
+        if (entries[h.index].currentState != needed) {
+            entries[h.index].currentState = needed;
+        }
+    }
+}

@@ frame_graph_v2.cpp — Execute() @@
+// Full v2 pipeline: BuildEdges → TopoSort → Cull → emit barriers + execute each surviving pass.
+void FrameGraph::Execute() {
+    BuildEdges();
+    auto sorted = TopoSort();
+    Cull(sorted);
+    for (PassIndex idx : sorted) {
+        if (!passes[idx].alive) continue;  // culled — skip
+        EmitBarriers(idx);                 // transition resources to correct state
+        passes[idx].Execute(/* &cmdList */);
+    }
+    passes.clear();     // reset for next frame
+    entries.clear();
+}
{{< /code-diff >}}

All four pieces — versioning, sorting, culling, barriers — compose into that `Execute()` body. Each step feeds the next: versioning creates edges, edges feed the sort, the sort enables culling, and the surviving sorted passes get automatic barriers.

---

### 🧩 Full v2 source

<div style="margin:.6em 0;font-size:.84em;opacity:.65;line-height:1.5;">
ℹ The full source files below include <code>printf</code> diagnostics (topo-sort order, culling results, barrier transitions) that are omitted from the diffs above to keep the focus on structure. These diagnostics are invaluable for debugging — read through them in the source.
</div>

{{< include-code file="frame_graph_v2.h" lang="cpp" compact="true" >}}
{{< include-code file="frame_graph_v2.cpp" lang="cpp" compact="true" >}}
{{< include-code file="example_v2.cpp" lang="cpp" compile="true" deps="frame_graph_v2.h,frame_graph_v2.cpp" compact="true" >}}

That's three of the four intro promises delivered — automatic ordering, barrier insertion, and dead-pass culling. The only piece missing: resources still live for the entire frame. Version 3 fixes that with lifetime analysis, memory aliasing, and the proper compile/execute barrier separation.

UE5's RDG follows the same pattern. When you call `FRDGBuilder::AddPass`, RDG builds the dependency graph from your declared reads/writes, topologically sorts it, culls dead passes, computes barriers, and stores them in the compiled plan — all before recording a single GPU command.

---

## 💾 MVP v3 — Lifetimes & Aliasing

<div style="margin:1em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid var(--ds-info);background:rgba(var(--ds-info-rgb),.04);font-size:.92em;line-height:1.6;">
🎯 <strong>Goal:</strong> Non-overlapping transient resources share physical memory — automatic VRAM aliasing with savings that depend on pass topology and resolution (Frostbite reported ~50% on BF1's deferred pipeline).
</div>

V2 gives us ordering, culling, and barriers — but every transient resource still gets its own VRAM for the entire frame. Resources whose lifetimes don’t overlap can share the same physical memory ([theory refresher](/posts/frame-graph-theory/#allocation-and-aliasing)). Time to implement that.

Two new structs — a `Lifetime` per resource, a `PhysicalBlock` per heap slot, and a `Barrier` to store precomputed transitions. The lifetime scan walks the sorted pass list, recording each transient resource's `firstUse` / `lastUse` indices:

{{< code-diff title="v2 → v3 — Lifetime structs, barrier struct & scan" >}}
@@ frame_graph_v3.h — PhysicalBlock, Lifetime, Barrier structs @@
+// A physical memory slot — multiple virtual resources can reuse it if their lifetimes don't overlap.
+struct PhysicalBlock {
+    uint32_t sizeBytes  = 0;        // block size (aligned)
+    PassIndex availAfter = 0;       // free after this sorted pass
+};
+
+// Per-resource lifetime in sorted-pass indices — drives aliasing decisions.
+struct Lifetime {
+    PassIndex firstUse = UINT32_MAX; // first sorted pass that touches this resource
+    PassIndex lastUse  = 0;          // last sorted pass that touches this resource
+    bool      isTransient = true;    // false for imported resources (externally owned)
+};
+
+// Precomputed barrier — stored during Compile(), replayed during Execute().
+struct Barrier {
+    ResourceIndex resourceIndex;    // which resource to transition
+    ResourceState oldState;         // state before this pass
+    ResourceState newState;         // state this pass requires
+    bool          isAliasing   = false;     // true = aliasing barrier (block changes occupant)
+    ResourceIndex aliasBefore  = UINT32_MAX; // resource being evicted (only when isAliasing)
+};

@@ frame_graph_v3.h — Allocation helpers @@
+// Minimum placement alignment for aliased heap resources (real APIs enforce similar, e.g. 64 KB).
+static constexpr uint32_t kPlacementAlignment = 65536;  // 64 KB
+
+inline uint32_t AlignUp(uint32_t value, uint32_t alignment) {
+    return (value + alignment - 1) & ~(alignment - 1);
+}
+
+inline uint32_t BytesPerPixel(Format fmt) {
+    switch (fmt) {
+        case Format::R8:      return 1;
+        case Format::RGBA8:   return 4;
+        case Format::D32F:    return 4;
+        case Format::RGBA16F: return 8;
+        default:              return 4;
+    }
+}
+
+// Aligned allocation size — real drivers add row padding/tiling; we approximate with a round-up.
+inline uint32_t AllocSize(const ResourceDesc& desc) {
+    uint32_t raw = desc.width * desc.height * BytesPerPixel(desc.format);
+    return AlignUp(raw, kPlacementAlignment);
+}

@@ frame_graph_v3.cpp — ScanLifetimes() @@
+// Record each resource's first/last use in sorted order — non-overlapping intervals can share memory.
+std::vector<Lifetime> FrameGraph::ScanLifetimes(const std::vector<PassIndex>& sorted) {
+    std::vector<Lifetime> life(entries.size());
+
+    // Imported resources (e.g. swapchain) are externally owned — exclude from aliasing.
+    for (ResourceIndex i = 0; i < entries.size(); i++) {
+        if (entries[i].imported) life[i].isTransient = false;
+    }
+
+    // Update first/last use for every resource each surviving pass touches.
+    for (PassIndex order = 0; order < sorted.size(); order++) {
+        PassIndex passIdx = sorted[order];
+        if (!passes[passIdx].alive) continue;
+
+        for (auto& h : passes[passIdx].reads) {
+            life[h.index].firstUse = std::min(life[h.index].firstUse, order);
+            life[h.index].lastUse  = std::max(life[h.index].lastUse,  order);
+        }
+        for (auto& h : passes[passIdx].writes) {
+            life[h.index].firstUse = std::min(life[h.index].firstUse, order);
+            life[h.index].lastUse  = std::max(life[h.index].lastUse,  order);
+        }
+    }
+    return life;
+}
{{< /code-diff >}}

This requires **placed resources** at the API level — GPU memory allocated from a heap, with resources bound to offsets within it. In D3D12, that means `ID3D12Heap` + `CreatePlacedResource`. In Vulkan, `VkDeviceMemory` + `vkBindImageMemory` at different offsets. Without placed resources (i.e., `CreateCommittedResource` or Vulkan dedicated allocations), each resource gets its own memory and aliasing is impossible — which is why the graph's allocator works with heaps.

The second half of the algorithm — the greedy free-list allocator. Sort resources by `firstUse`, then try to fit each one into an existing block whose previous user has finished:

{{< code-diff title="v3 — Greedy free-list aliasing + Compile() integration" >}}
@@ frame_graph_v3.h — CompiledPlan + Compile() / Execute(plan) @@
+    struct CompiledPlan {
+        std::vector<PassIndex> sorted;
+        std::vector<uint32_t> mapping;                  // mapping[virtualIdx] → physicalBlock
+        std::vector<std::vector<Barrier>> barriers;     // barriers[orderIdx] → pre-pass transitions
+    };
+
+    CompiledPlan Compile();
+    void Execute(const CompiledPlan& plan);
     void Execute();  // now: convenience wrapper — compile + execute in one call

@@ frame_graph_v3.h — new private methods @@
+    std::vector<Lifetime> ScanLifetimes(const std::vector<PassIndex>& sorted);
+    std::vector<uint32_t> AliasResources(const std::vector<Lifetime>& lifetimes);
+    std::vector<std::vector<Barrier>> ComputeBarriers(const std::vector<PassIndex>& sorted,
+                                                       const std::vector<uint32_t>& mapping);

@@ frame_graph_v3.cpp — AliasResources() @@
+// Greedy first-fit: sort by firstUse, reuse any free block that fits, else allocate a new one.
+std::vector<uint32_t> FrameGraph::AliasResources(const std::vector<Lifetime>& lifetimes) {
+    std::vector<PhysicalBlock> freeList;
+    std::vector<uint32_t> mapping(entries.size(), UINT32_MAX);
+
+    // Process resources in the order they're first used.
+    std::vector<ResourceIndex> indices(entries.size());
+    std::iota(indices.begin(), indices.end(), 0);
+    std::sort(indices.begin(), indices.end(), [&](ResourceIndex a, ResourceIndex b) {
+        return lifetimes[a].firstUse < lifetimes[b].firstUse;
+    });
+
+    for (ResourceIndex resIdx : indices) {
+        if (!lifetimes[resIdx].isTransient) continue;      // skip imported resources
+        if (lifetimes[resIdx].firstUse == UINT32_MAX) continue;  // never used
+
+        uint32_t needed = AllocSize(entries[resIdx].desc);
+        bool reused = false;
+
+        // Scan existing blocks — can we reuse one that's now free?
+        for (uint32_t b = 0; b < freeList.size(); b++) {
+            if (freeList[b].availAfter < lifetimes[resIdx].firstUse  // block is free
+                && freeList[b].sizeBytes >= needed) {                 // and large enough
+                mapping[resIdx] = b;         // reuse this block
+                freeList[b].availAfter = lifetimes[resIdx].lastUse;  // extend occupancy
+                reused = true;
+                break;
+            }
+        }
+
+        if (!reused) {  // no fit found → allocate a new physical block
+            mapping[resIdx] = static_cast<uint32_t>(freeList.size());
+            freeList.push_back({ needed, lifetimes[resIdx].lastUse });
+        }
+    }
+    return mapping;
+}

@@ frame_graph_v3.cpp — Compile() @@
+// Full compile pipeline — all analysis runs here before any GPU commands. Returns a self-contained plan.
+FrameGraph::CompiledPlan FrameGraph::Compile() {
+    BuildEdges();                                // 1. deduplicate dependency edges
+    auto sorted   = TopoSort();                  // 2. valid execution order
+    Cull(sorted);                                // 3. remove unreachable passes
+    auto lifetimes = ScanLifetimes(sorted);      // 4. when is each resource alive?
+    auto mapping   = AliasResources(lifetimes);  // 5. share memory where lifetimes don't overlap
+    auto barriers  = ComputeBarriers(sorted, mapping); // 6. precompute every state + aliasing transition
+    return { std::move(sorted), std::move(mapping), std::move(barriers) };
+}

@@ frame_graph_v3.cpp — ComputeBarriers() @@
+// Pure analysis — track each resource's state, record a Barrier when a pass needs a different one.
+// Also detects aliasing barriers: when a physical block's occupant changes.
+std::vector<std::vector<Barrier>> FrameGraph::ComputeBarriers(
+        const std::vector<PassIndex>& sorted,
+        const std::vector<uint32_t>& mapping) {
+    std::vector<std::vector<Barrier>> result(sorted.size());
+    // Track which virtual resource currently occupies each physical block.
+    std::vector<ResourceIndex> blockOwner(/* numBlocks */, UINT32_MAX);
+    for (PassIndex orderIdx = 0; orderIdx < sorted.size(); orderIdx++) {
+        PassIndex passIdx = sorted[orderIdx];
+        if (!passes[passIdx].alive) continue;
+        // Aliasing: if a resource's physical block has a different occupant, emit aliasing barrier.
+        auto emitAliasingIfNeeded = [&](ResourceHandle h) {
+            uint32_t block = mapping[h.index];
+            if (block == UINT32_MAX) return;                              // imported or unmapped
+            if (blockOwner[block] != UINT32_MAX && blockOwner[block] != h.index) {
+                Barrier ab{};
+                ab.resourceIndex = h.index;       // the new occupant
+                ab.isAliasing    = true;
+                ab.aliasBefore   = blockOwner[block]; // the old occupant
+                result[orderIdx].push_back(ab);   // emit before state transitions
+            }
+            blockOwner[block] = h.index;          // update block ownership
+        };
+        for (auto& h : passes[passIdx].reads)  emitAliasingIfNeeded(h);
+        for (auto& h : passes[passIdx].writes) emitAliasingIfNeeded(h);
+        // Same UAV-aware heuristic as v2's EmitBarriers — checks readWrites list.
+        auto IsUAV = [&](ResourceHandle h) {
+            for (auto& rw : passes[passIdx].readWrites)
+                if (rw.index == h.index) return true;
+            return false;
+        };
+        auto StateForUsage = [&](ResourceHandle h, bool isWrite) {
+            if (IsUAV(h)) return ResourceState::UnorderedAccess;
+            if (isWrite)
+                return (entries[h.index].desc.format == Format::D32F)
+                    ? ResourceState::DepthAttachment : ResourceState::ColorAttachment;
+            return ResourceState::ShaderRead;
+        };
+        for (auto& h : passes[passIdx].reads) {
+            ResourceState needed = StateForUsage(h, false);
+            if (entries[h.index].currentState != needed) {
+                result[orderIdx].push_back(                  // record transition
+                    { h.index, entries[h.index].currentState, needed });
+                entries[h.index].currentState = needed;       // update tracked state
+            }
+        }
+        for (auto& h : passes[passIdx].writes) {
+            ResourceState needed = StateForUsage(h, true);
+            if (entries[h.index].currentState != needed) {
+                result[orderIdx].push_back(                  // record transition
+                    { h.index, entries[h.index].currentState, needed });
+                entries[h.index].currentState = needed;       // update tracked state
+            }
+        }
+    }
+    return result;
+}

@@ frame_graph_v3.cpp — EmitBarriers() @@
+void FrameGraph::EmitBarriers(const std::vector<Barrier>& barriers) {
+    for (auto& b : barriers) {
+        if (b.isAliasing) {
+            // D3D12: D3D12_RESOURCE_BARRIER_TYPE_ALIASING / Vulkan: memory barrier on the shared heap region.
+        } else {
+            // D3D12: ResourceBarrier(StateBefore→StateAfter) / Vulkan: vkCmdPipelineBarrier(oldLayout→newLayout).
+        }
+    }
+}

@@ frame_graph_v3.cpp — Execute() @@
+// v3 Execute: pure playback — emit precomputed barriers and call execute lambdas, no analysis.
-void FrameGraph::Execute() {
-    BuildEdges();
-    auto sorted = TopoSort();
-    Cull(sorted);
-    for (PassIndex idx : sorted) {
-        if (!passes[idx].alive) continue;
-        EmitBarriers(idx);
-        passes[idx].Execute(/* &cmdList */);
-    }
-    passes.clear();
-    entries.clear();
-}
+void FrameGraph::Execute(const CompiledPlan& plan) {
+    for (PassIndex orderIdx = 0; orderIdx < plan.sorted.size(); orderIdx++) {
+        PassIndex passIdx = plan.sorted[orderIdx];
+        if (!passes[passIdx].alive) continue;
+        EmitBarriers(plan.barriers[orderIdx]);  // replay precomputed transitions
+        passes[passIdx].Execute(/* &cmdList */); // record GPU commands
+    }
+    passes.clear();     // reset for next frame
+    entries.clear();
+}
+
+// Convenience: compile + execute in one call.
+void FrameGraph::Execute() { Execute(Compile()); }
{{< /code-diff >}}

~100 new lines on top of v2. **The key architectural change**: v3 separates barrier computation from submission. `ComputeBarriers()` walks the sorted passes during compile, detects every state transition, and stores them in the `CompiledPlan`. Execute is now a pure playback loop — it submits precomputed barriers and calls execute lambdas. No state tracking, no decisions.

Aliasing runs once per frame in O(R log R + R·B) — sort by first-use, then for each resource scan the free list for a fit. B is the number of physical blocks (bounded by R), so worst-case is O(R²) — but in practice B stays small (~3–5 blocks for ~15 resources), making the scan effectively linear. Sub-microsecond for 15 transient resources.

`ComputeBarriers()` now also detects aliasing transitions: it tracks which virtual resource occupies each physical block, and when a block's occupant changes, it emits an aliasing barrier before the state-transition barriers. D3D12 calls this `D3D12_RESOURCE_BARRIER_TYPE_ALIASING`; Vulkan uses a memory barrier on the shared heap region. Without it, caches may serve stale data from the previous occupant.

<div style="margin:1.2em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid var(--ds-info);background:rgba(var(--ds-info-rgb),.04);font-size:.88em;line-height:1.6;">
📝 <strong>Alignment and real GPU sizing.</strong>&ensp;
Our <code>AllocSize()</code> rounds up to a 64 KB placement alignment — the same constraint real GPUs enforce when placing resources into shared heaps. This matters because without alignment, two resources that appear to fit in the same block could overlap at the hardware level. The raw <code>BytesPerPixel()</code> calculation is still a simplification though: production allocators query the driver for actual sizes (which include row padding, tiling overhead, and per-resource alignment). The aliasing algorithm itself is unchanged — you just swap the size input.
</div>

That's the full value prop — automatic memory aliasing, precomputed barriers, and the clean compile/execute separation, all from a single `FrameGraph` class. UE5's transient resource allocator does the same thing: any `FRDGTexture` created through `FRDGBuilder::CreateTexture` (vs `RegisterExternalTexture`) is transient and eligible for aliasing, using the same lifetime analysis and free-list scan we just built.

---

### 🧩 Full v3 source

{{< include-code file="frame_graph_v3.h" lang="cpp" compact="true" >}}
{{< include-code file="frame_graph_v3.cpp" lang="cpp" compact="true" >}}
{{< include-code file="example_v3.cpp" lang="cpp" compile="true" deps="frame_graph_v3.h,frame_graph_v3.cpp" compact="true" >}}

---

### ✅ What the MVP delivers

The finished `FrameGraph` class. Here's what it does every frame, broken down by phase — the same declare → compile → execute lifecycle from [Part I](/posts/frame-graph-theory/):

<div style="margin:1.2em 0;display:grid;grid-template-columns:repeat(3,1fr);gap:.8em;">
  <div style="padding:.8em 1em;border-radius:10px;border-top:3px solid var(--ds-info);background:rgba(var(--ds-info-rgb),.04);">
    <div style="font-weight:800;font-size:.88em;margin-bottom:.5em;color:var(--ds-info);">① Declare</div>
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
    <div style="font-weight:800;font-size:.88em;margin-bottom:.5em;color:var(--ds-code);">② Compile</div>
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

That's the full MVP — a single `FrameGraph` class that handles dependency-driven ordering, culling, aliasing, and precomputed barriers. Compile analyzes and decides; execute submits and runs. Every concept from [Part I](/posts/frame-graph-theory/) now exists as running code.

### 🔮 What's next

The MVP handles one queue, one barrier type, and one allocation strategy. Production engines go further: **async compute** overlaps GPU work across queues and **split barriers** let the driver pipeline state transitions instead of stalling. [Part III — Beyond MVP](../frame-graph-advanced/) breaks down each of these upgrades and shows where they plug into the architecture we just built.

---

<div style="margin:2em 0 0;padding:1em 1.2em;border-radius:10px;border:1px solid rgba(var(--ds-indigo-rgb),.2);background:rgba(var(--ds-indigo-rgb),.03);display:flex;justify-content:space-between;align-items:center;">
  <a href="../frame-graph-theory/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    ← Previous: Part I — Theory
  </a>
  <a href="../frame-graph-advanced/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    Next: Part III — Beyond MVP →
  </a>
</div>
