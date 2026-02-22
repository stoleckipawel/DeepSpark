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

*Part I laid out the theory — declare, compile, execute. Now we turn that blueprint into code. Three iterations, each one unlocking a capability that wasn't there before: first a topological sort gives us dependency-driven execution order, then the compiler injects barriers automatically, and finally memory aliasing lets resources share the same heap. Each version builds on the last — time to get our hands dirty.*

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
        <span style="font-size:.65em;font-weight:700;padding:.15em .5em;border-radius:9px;background:rgba(var(--ds-info-rgb),.12);color:var(--ds-info);white-space:nowrap;">~120 LOC</span>
      </div>
      <div style="font-size:.84em;line-height:1.5;opacity:.85;margin-bottom:.5em;">Pass declaration, virtual resources, execute in order. The skeleton everything else plugs into.</div>
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
        <span style="font-size:.65em;font-weight:700;padding:.15em .5em;border-radius:9px;background:rgba(var(--ds-code-rgb),.12);color:var(--ds-code);white-space:nowrap;">~300 LOC</span>
      </div>
      <div style="font-size:.84em;line-height:1.5;opacity:.85;margin-bottom:.5em;">Resource versioning → edges → topo-sort → dead-pass culling → automatic barrier insertion. <strong>Everything the GPU needs, derived from the DAG you already built.</strong></div>
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
        <span style="font-size:.62em;font-weight:800;padding:.15em .5em;border-radius:9px;background:var(--ds-success);color:#fff;white-space:nowrap;">★ PRODUCTION-READY</span>
      </div>
      <div style="font-size:.84em;line-height:1.5;opacity:.85;margin-bottom:.5em;">Lifetime scan + greedy free-list allocator. Non-overlapping resources share physical memory — <strong>~50% VRAM saved</strong>.</div>
      <!-- unlocks -->
      <div style="display:flex;flex-wrap:wrap;gap:.35em;margin-bottom:.6em;">
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-success-rgb),.1);color:var(--ds-success);font-weight:700;">🔓 lifetime scan</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-success-rgb),.1);color:var(--ds-success);font-weight:700;">🔓 memory aliasing</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-success-rgb),.1);color:var(--ds-success);font-weight:700;">🔓 heap compaction</span>
        <span style="font-size:.68em;padding:.15em .55em;border-radius:9px;background:rgba(var(--ds-success-rgb),.12);color:var(--ds-success);font-weight:700;">⚡ 50% VRAM savings</span>
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
    direction LR

    class FrameGraph {
        -vector passes
        -vector entries
        +CreateResource(desc) ResourceHandle
        +ImportResource(desc, state) ResourceHandle
        +AddPass(name, setup, execute) void
        +Read(passIdx, handle) void
        +Write(passIdx, handle) void
        +Compile() CompiledPlan
        +Execute(plan) void
    }

    class RenderPass {
        +string name
        +function setup
        +function execute
        +vector reads
        +vector writes
        +vector dependsOn
        +bool alive
    }

    class ResourceHandle {
        +uint32_t index
        +IsValid() bool
    }

    class ResourceDesc {
        +uint32_t width
        +uint32_t height
        +Format format
    }

    class ResourceEntry {
        +ResourceDesc desc
        +vector versions
        +ResourceState currentState
        +bool imported
    }

    class ResourceVersion {
        +uint32_t writerPass
        +vector readerPasses
    }

    class Lifetime {
        +uint32_t firstUse
        +uint32_t lastUse
        +bool isTransient
    }

    class PhysicalBlock {
        +uint32_t sizeBytes
        +uint32_t availAfter
    }

    class Format {
        RGBA8
        RGBA16F
        R8
        D32F
    }
    note for Format "enum"

    class ResourceState {
        Undefined
        ColorAttachment
        DepthAttachment
        ShaderRead
        Present
    }
    note for ResourceState "enum"

    FrameGraph *-- RenderPass : owns
    FrameGraph *-- ResourceEntry : owns
    ResourceEntry *-- ResourceDesc
    ResourceEntry *-- ResourceVersion
    RenderPass --> ResourceHandle : references
    FrameGraph ..> ResourceHandle : creates
    FrameGraph ..> Lifetime : computes
    FrameGraph ..> PhysicalBlock : allocates
    ResourceEntry --> ResourceState
    ResourceDesc --> Format
{{< /mermaid >}}

### 🔀 Design choices

The three-phase model from [Part I](../frame-graph-theory/) forces nine API decisions. Every choice is driven by the same question: *what does the graph compiler need, and what's the cheapest way to give it?*

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
  <td style="padding:.5em .6em;opacity:.8;">Zero boilerplate — handles live in scope, both lambdas capture them directly. Won't scale past one TU per pass; migrate to typed pass data when that matters.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Type-erased pass data — <code>AddPass&lt;PassData&gt;(setup, exec)</code>. Decouples setup/execute across TUs.</td>
</tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);background:rgba(var(--ds-indigo-rgb),.02);">
  <td style="padding:.5em .6em;font-weight:700;">②</td>
  <td style="padding:.5em .6em;">Where do DAG edges come from?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Explicit <code>fg.Read/Write(pass, h)</code></strong></td>
  <td style="padding:.5em .6em;opacity:.8;">Every edge is an explicit call — easy to grep and debug. Scales fine; a scoped builder is syntactic sugar, not a structural change.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Scoped builder — <code>builder.Read/Write(h)</code> auto-binds to the current pass. Prevents mis-wiring at scale.</td>
</tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);">
  <td style="padding:.5em .6em;font-weight:700;">③</td>
  <td style="padding:.5em .6em;">What is a resource handle?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Plain <code>uint32_t</code> index</strong></td>
  <td style="padding:.5em .6em;opacity:.8;">One integer, trivially copyable — no templates, no overhead. A <code>using</code> alias away from typed wrappers when pass count grows.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Typed wrappers — <code>FRDGTextureRef</code> / <code>FRDGBufferRef</code>. Compile-time safety for 700+ passes (UE5).</td>
</tr>
<tr><td colspan="5" style="padding:.6em .6em .3em;font-weight:800;font-size:.85em;letter-spacing:.04em;color:var(--ds-code);border-bottom:1px solid rgba(var(--ds-code-rgb),.12);">COMPILE — what the graph analyser decides</td></tr>
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);">
  <td style="padding:.5em .6em;font-weight:700;">④</td>
  <td style="padding:.5em .6em;">Is compile explicit?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Yes — <code>Compile()→Execute(plan)</code></strong></td>
  <td style="padding:.5em .6em;opacity:.8;">Returned plan struct lets you log, validate, and visualise the DAG — invaluable while learning. Production-ready as-is.</td>
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
  <td style="padding:.5em .6em;opacity:.8;">Under 100 µs at ~25 passes — free perf budget to just rebuild. Adapts to res changes & toggles with zero invalidation logic. Cache later if profiling says so.</td>
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
<tr style="border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);background:rgba(var(--ds-indigo-rgb),.02);">
  <td style="padding:.5em .6em;font-weight:700;">⑨</td>
  <td style="padding:.5em .6em;">How does a pass get the actual GPU resource from a handle?</td>
  <td style="padding:.5em .6em;white-space:nowrap;"><strong>Context lookup</strong></td>
  <td style="padding:.5em .6em;opacity:.8;"><code>ctx.GetTexture(handle)</code> — each pass asks for what it needs at runtime. One array lookup per resource, trivially cheap. The callback stays self-contained with no setup from the executor.</td>
  <td style="padding:.5em .6em;opacity:.55;font-size:.92em;">Bindless indices — handles map directly to descriptor-heap slots. The callback passes an integer to the shader (<code>ResourceDescriptorHeap[idx]</code>) with no CPU-side lookup, but requires a bindless-capable API.</td>
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
@@ New types — resource description + handle @@
+enum class Format { RGBA8, RGBA16F, R8, D32F };
+
+struct ResourceDesc {
+    uint32_t width  = 0;
+    uint32_t height = 0;
+    Format   format = Format::RGBA8;
+};
+
+struct ResourceHandle {
+    uint32_t index = UINT32_MAX;
+    bool IsValid() const { return index != UINT32_MAX; }
+};
{{< /code-diff >}}

A pass is two lambdas — setup (runs now, wires the DAG) and execute (stored for later, records GPU commands). v1 doesn't use setup yet, but the slot is there for v2:

{{< code-diff title="v1 — RenderPass + FrameGraph class (frame_graph_v1.h)" >}}
@@ RenderPass @@
+struct RenderPass {
+    std::string                        name;
+    std::function<void()>              Setup;    // build the DAG (v1: unused)
+    std::function<void(/*cmd list*/)>  Execute;  // record GPU commands
+};

@@ FrameGraph — owns passes + resources @@
+class FrameGraph {
+public:
+    ResourceHandle CreateResource(const ResourceDesc& desc);
+    ResourceHandle ImportResource(const ResourceDesc& desc);
+
+    template <typename SetupFn, typename ExecFn>
+    void AddPass(const std::string& name, SetupFn&& setup, ExecFn&& exec) {
+        passes.push_back({ name, std::forward<SetupFn>(setup),
+                                  std::forward<ExecFn>(exec) });
+        passes.back().Setup();  // run setup immediately
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
@@ CreateResource / ImportResource @@
+ResourceHandle FrameGraph::CreateResource(const ResourceDesc& desc) {
+    resources.push_back(desc);
+    return { static_cast<uint32_t>(resources.size() - 1) };
+}
+
+ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc) {
+    resources.push_back(desc);  // v1: same as create (no aliasing yet)
+    return { static_cast<uint32_t>(resources.size() - 1) };
+}

@@ execute — declaration order, no compile step @@
+void FrameGraph::Execute() {
+    printf("\n[1] Executing (declaration order -- no compile step):\n");
+    for (auto& pass : passes) {
+        printf("  >> exec: %s\n", pass.name.c_str());
+        pass.Execute(/* &cmdList */);
+    }
+    passes.clear();
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
@@ New type — version tracking (.h) @@
+struct ResourceVersion {                 // NEW v2
+    uint32_t writerPass = UINT32_MAX;    // which pass wrote this version
+    std::vector<uint32_t> readerPasses;  // which passes read it
+    bool HasWriter() const { return writerPass != UINT32_MAX; }
+};
+
+struct ResourceEntry {
+    ResourceDesc desc;
+    std::vector<ResourceVersion> versions;  // version 0, 1, 2...
+    bool imported = false;   // imported resources: barriers tracked, not aliased
+};

@@ RenderPass — dependency edges (.h) @@
 struct RenderPass {
     std::string name;
     std::function<void()>             Setup;
     std::function<void(/*cmd list*/)> Execute;
+    std::vector<ResourceHandle> reads;     // NEW v2
+    std::vector<ResourceHandle> writes;    // NEW v2
+    std::vector<uint32_t> dependsOn;       // NEW v2
 };

@@ FrameGraph — new declarations (.h) @@
+    void Read(uint32_t passIdx, ResourceHandle h);    // NEW v2
+    void Write(uint32_t passIdx, ResourceHandle h);   // NEW v2

@@ Storage (.h) @@
-    std::vector<ResourceDesc>  resources;
+    std::vector<ResourceEntry> entries;  // now with versioning

@@ CreateResource / ImportResource — use ResourceEntry (.cpp) @@
 ResourceHandle FrameGraph::CreateResource(const ResourceDesc& desc) {
-    resources.push_back(desc);
-    return { static_cast<uint32_t>(resources.size() - 1) };
+    entries.push_back({ desc, {{}} });
+    return { static_cast<uint32_t>(entries.size() - 1) };
 }

 ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc) {
-    resources.push_back(desc);
-    return { static_cast<uint32_t>(resources.size() - 1) };
+    entries.push_back({ desc, {{}}, /*imported=*/true });
+    return { static_cast<uint32_t>(entries.size() - 1) };
 }

@@ Read / Write (.cpp) @@
+void FrameGraph::Read(uint32_t passIdx, ResourceHandle h) {
+    auto& ver = entries[h.index].versions.back();
+    if (ver.HasWriter()) {
+        passes[passIdx].dependsOn.push_back(ver.writerPass);
+    }
+    ver.readerPasses.push_back(passIdx);
+    passes[passIdx].reads.push_back(h);
+}
+
+void FrameGraph::Write(uint32_t passIdx, ResourceHandle h) {
+    entries[h.index].versions.push_back({});
+    entries[h.index].versions.back().writerPass = passIdx;
+    passes[passIdx].writes.push_back(h);
+}
{{< /code-diff >}}

Every `Write()` pushes a new version. Every `Read()` finds the current version's writer and records a `dependsOn` edge. Those edges feed the next three steps.

---

<span id="v2-toposort"></span>

### 📊 Topological sort (Kahn's algorithm)

With edges in place, we need an execution order that respects every dependency. Kahn’s algorithm ([theory refresher](/posts/frame-graph-theory/#sorting-and-culling)) gives us one in O(V+E). `BuildEdges()` deduplicates the raw `dependsOn` entries and builds the adjacency list; `TopoSort()` does the zero-in-degree queue drain:

{{< code-diff title="v2 — Edge building + Kahn's topological sort" >}}
@@ RenderPass — new fields for the sort (.h) @@
 struct RenderPass {
     ...
+    std::vector<uint32_t> successors;      // passes that depend on this one
+    uint32_t inDegree = 0;                 // incoming edge count (Kahn's)
 };

@@ BuildEdges() — deduplicate and build adjacency list (.cpp) @@
+void FrameGraph::BuildEdges() {
+    for (uint32_t i = 0; i < passes.size(); i++) {
+        std::unordered_set<uint32_t> seen;
+        for (uint32_t dep : passes[i].dependsOn) {
+            if (seen.insert(dep).second) {
+                passes[dep].successors.push_back(i);
+                passes[i].inDegree++;
+            }
+        }
+    }
+}

@@ TopoSort() — Kahn's algorithm, O(V + E) (.cpp) @@
+std::vector<uint32_t> FrameGraph::TopoSort() {
+    std::queue<uint32_t> q;
+    std::vector<uint32_t> inDeg(passes.size());
+    for (uint32_t i = 0; i < passes.size(); i++) {
+        inDeg[i] = passes[i].inDegree;
+        if (inDeg[i] == 0) q.push(i);
+    }
+    std::vector<uint32_t> order;
+    while (!q.empty()) {
+        uint32_t cur = q.front(); q.pop();
+        order.push_back(cur);
+        for (uint32_t succ : passes[cur].successors) {
+            if (--inDeg[succ] == 0)
+                q.push(succ);
+        }
+    }
+    assert(order.size() == passes.size() && "Cycle detected!");
+    return order;
+}
{{< /code-diff >}}

---

<span id="v2-culling"></span>

### ✂ Pass culling

A sorted graph still runs passes nobody reads from. Culling is dead-code elimination for GPU work ([theory refresher](/posts/frame-graph-theory/#sorting-and-culling)) — a single backward walk marks the final pass alive, then propagates through `dependsOn` edges:

{{< code-diff title="v2 — Pass culling" >}}
@@ RenderPass — new field for culling (.h) @@
 struct RenderPass {
     ...
+    bool alive = false;                    // survives the cull?
 };

@@ Cull() — backward reachability from output (.cpp) @@
+void FrameGraph::Cull(const std::vector<uint32_t>& sorted) {
+    if (sorted.empty()) return;
+    passes[sorted.back()].alive = true;   // last pass = output
+    for (int i = static_cast<int>(sorted.size()) - 1; i >= 0; i--) {
+        if (!passes[sorted[i]].alive) continue;
+        for (uint32_t dep : passes[sorted[i]].dependsOn)
+            passes[dep].alive = true;
+    }
+}
{{< /code-diff >}}

---

<span id="v2-barriers"></span>

### 🚧 Barrier insertion

The GPU needs explicit state transitions between usages — color attachment, shader read, depth, etc. Because the graph already knows every resource’s read/write history ([theory refresher](/posts/frame-graph-theory/#barriers)), the compiler can emit them automatically. Walk each pass’s resources, compare tracked state to what the pass needs, and insert a barrier when they differ:

{{< code-diff title="v2 — Barrier insertion + Execute() rewrite" >}}
@@ New type — resource state tracking (.h) @@
+enum class ResourceState { Undefined, ColorAttachment, DepthAttachment,
+                           ShaderRead, Present };

@@ ResourceEntry — track current state (.h) @@
 struct ResourceEntry {
     ...
+    ResourceState currentState = ResourceState::Undefined;
 };

@@ ImportResource — now accepts an initial state (.h) @@
-    ResourceHandle ImportResource(const ResourceDesc& desc);
+    ResourceHandle ImportResource(const ResourceDesc& desc,
+                                  ResourceState initialState = ResourceState::Undefined);

@@ CreateResource / ImportResource — updated for ResourceState (.cpp) @@
 ResourceHandle FrameGraph::CreateResource(const ResourceDesc& desc) {
-    entries.push_back({ desc, {{}} });
+    entries.push_back({ desc, {{}}, ResourceState::Undefined, false });
     return { static_cast<uint32_t>(entries.size() - 1) };
 }

-ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc) {
-    entries.push_back({ desc, {{}}, /*imported=*/true });
+ResourceHandle FrameGraph::ImportResource(const ResourceDesc& desc,
+                                          ResourceState initialState) {
+    entries.push_back({ desc, {{}}, initialState, true });
     return { static_cast<uint32_t>(entries.size() - 1) };
 }

@@ InsertBarriers() — emit transitions where state changes (.cpp) @@
+void FrameGraph::InsertBarriers(uint32_t passIdx) {
+    auto StateForUsage = [](bool isWrite, Format fmt) {
+        if (isWrite)
+            return (fmt == Format::D32F) ? ResourceState::DepthAttachment
+                                         : ResourceState::ColorAttachment;
+        return ResourceState::ShaderRead;
+    };
+    for (auto& h : passes[passIdx].reads) {
+        ResourceState needed = ResourceState::ShaderRead;
+        if (entries[h.index].currentState != needed) {
+            entries[h.index].currentState = needed;
+        }
+    }
+    for (auto& h : passes[passIdx].writes) {
+        ResourceState needed = StateForUsage(true, entries[h.index].desc.format);
+        if (entries[h.index].currentState != needed) {
+            entries[h.index].currentState = needed;
+        }
+    }
+}

@@ Execute() — the full v2 pipeline (.cpp) @@
+void FrameGraph::Execute() {
+    BuildEdges();
+    auto sorted = TopoSort();
+    Cull(sorted);
+    for (uint32_t idx : sorted) {
+        if (!passes[idx].alive) continue;
+        InsertBarriers(idx);
+        passes[idx].Execute(/* &cmdList */);
+    }
+    passes.clear();
+    entries.clear();
+}
{{< /code-diff >}}

All four pieces — versioning, sorting, culling, barriers — compose into that `Execute()` body. Each step feeds the next: versioning creates edges, edges feed the sort, the sort enables culling, and the surviving sorted passes get automatic barriers.

---

### 🧩 Full v2 source

{{< include-code file="frame_graph_v2.h" lang="cpp" compact="true" >}}
{{< include-code file="frame_graph_v2.cpp" lang="cpp" compact="true" >}}
{{< include-code file="example_v2.cpp" lang="cpp" compile="true" deps="frame_graph_v2.h,frame_graph_v2.cpp" compact="true" >}}

That's three of the four intro promises delivered — automatic ordering, barrier insertion, and dead-pass culling. The only piece missing: resources still live for the entire frame. Version 3 fixes that with lifetime analysis and memory aliasing.

UE5's RDG does the same thing. When you call `FRDGBuilder::AddPass`, RDG builds the dependency graph from your declared reads/writes, topologically sorts it, culls dead passes, and inserts barriers — all before recording a single GPU command.

---

## 💾 MVP v3 — Lifetimes & Aliasing

<div style="margin:1em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid var(--ds-info);background:rgba(var(--ds-info-rgb),.04);font-size:.92em;line-height:1.6;">
🎯 <strong>Goal:</strong> Non-overlapping transient resources share physical memory — automatic VRAM aliasing with ~50% savings.
</div>

V2 gives us ordering, culling, and barriers — but every transient resource still gets its own VRAM for the entire frame. Resources whose lifetimes don’t overlap can share the same physical memory ([theory refresher](/posts/frame-graph-theory/#allocation-and-aliasing)). Time to implement that.

Two new structs — a `Lifetime` per resource and a `PhysicalBlock` per heap slot. The lifetime scan walks the sorted pass list, recording each transient resource's `firstUse` / `lastUse` indices:

{{< code-diff title="v2 → v3 — Lifetime structs & scan" >}}
@@ New structs (.h) @@
+struct PhysicalBlock {              // physical memory slot
+    uint32_t sizeBytes  = 0;
+    uint32_t availAfter = 0;        // free after this pass index
+};
+
+struct Lifetime {                   // per-resource timing
+    uint32_t firstUse = UINT32_MAX;
+    uint32_t lastUse  = 0;
+    bool     isTransient = true;
+};

@@ BytesPerPixel helper (.h) @@
+inline uint32_t BytesPerPixel(Format fmt) {
+    switch (fmt) {
+        case Format::R8:      return 1;
+        case Format::RGBA8:   return 4;
+        case Format::D32F:    return 4;
+        case Format::RGBA16F: return 8;
+        default:              return 4;
+    }
+}

@@ ScanLifetimes() — walk sorted passes, record first/last use (.cpp) @@
+std::vector<Lifetime> FrameGraph::ScanLifetimes(const std::vector<uint32_t>& sorted) {
+    std::vector<Lifetime> life(entries.size());
+
+    // Imported resources are externally owned — exclude from aliasing.
+    for (uint32_t i = 0; i < entries.size(); i++) {
+        if (entries[i].imported) life[i].isTransient = false;
+    }
+
+    for (uint32_t order = 0; order < sorted.size(); order++) {
+        uint32_t passIdx = sorted[order];
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
@@ FrameGraph — v3 public additions (.h) @@
+    struct CompiledPlan {
+        std::vector<uint32_t> sorted;
+        std::vector<uint32_t> mapping;   // mapping[virtualIdx] → physicalBlock
+    };
+
+    CompiledPlan Compile();
+    void Execute(const CompiledPlan& plan);
     void Execute();  // now: convenience wrapper — compile + execute in one call

@@ FrameGraph — v3 private additions (.h) @@
+    std::vector<Lifetime> ScanLifetimes(const std::vector<uint32_t>& sorted);
+    std::vector<uint32_t> AliasResources(const std::vector<Lifetime>& lifetimes);

@@ AliasResources() — greedy free-list scan (.cpp) @@
+std::vector<uint32_t> FrameGraph::AliasResources(const std::vector<Lifetime>& lifetimes) {
+    std::vector<PhysicalBlock> freeList;
+    std::vector<uint32_t> mapping(entries.size(), UINT32_MAX);
+
+    // Sort resources by firstUse.
+    std::vector<uint32_t> indices(entries.size());
+    std::iota(indices.begin(), indices.end(), 0);
+    std::sort(indices.begin(), indices.end(), [&](uint32_t a, uint32_t b) {
+        return lifetimes[a].firstUse < lifetimes[b].firstUse;
+    });
+
+    for (uint32_t resIdx : indices) {
+        if (!lifetimes[resIdx].isTransient) continue;
+        if (lifetimes[resIdx].firstUse == UINT32_MAX) continue;
+
+        uint32_t needed = entries[resIdx].desc.width
+                        * entries[resIdx].desc.height
+                        * BytesPerPixel(entries[resIdx].desc.format);
+        bool reused = false;
+
+        for (uint32_t b = 0; b < freeList.size(); b++) {
+            if (freeList[b].availAfter < lifetimes[resIdx].firstUse
+                && freeList[b].sizeBytes >= needed) {
+                mapping[resIdx] = b;         // reuse this block
+                freeList[b].availAfter = lifetimes[resIdx].lastUse;
+                reused = true;
+                break;
+            }
+        }
+
+        if (!reused) {
+            mapping[resIdx] = static_cast<uint32_t>(freeList.size());
+            freeList.push_back({ needed, lifetimes[resIdx].lastUse });
+        }
+    }
+    return mapping;
+}

@@ Compile() — v3 separates compile from execute (.cpp) @@
+FrameGraph::CompiledPlan FrameGraph::Compile() {
+    BuildEdges();
+    auto sorted   = TopoSort();
+    Cull(sorted);
+    auto lifetimes = ScanLifetimes(sorted);      // NEW v3
+    auto mapping   = AliasResources(lifetimes);  // NEW v3
+    return { std::move(sorted), std::move(mapping) };
+}

@@ Execute() — v3 replaces monolithic Execute with Compile + Execute split (.cpp) @@
-void FrameGraph::Execute() {
-    BuildEdges();
-    auto sorted = TopoSort();
-    Cull(sorted);
-    for (uint32_t idx : sorted) {
-        if (!passes[idx].alive) continue;
-        InsertBarriers(idx);
-        passes[idx].Execute(/* &cmdList */);
-    }
-    passes.clear();
-    entries.clear();
-}
+void FrameGraph::Execute(const CompiledPlan& plan) {
+    for (uint32_t idx : plan.sorted) {
+        if (!passes[idx].alive) continue;
+        InsertBarriers(idx);
+        passes[idx].Execute(/* &cmdList */);
+    }
+    passes.clear();
+    entries.clear();
+}
+
+void FrameGraph::Execute() { Execute(Compile()); }
{{< /code-diff >}}

~70 new lines on top of v2. Aliasing runs once per frame in O(R log R) — sort, then linear scan of the free list. Sub-microsecond for 15 transient resources.

That's the full value prop — automatic memory aliasing *and* automatic barriers from a single `FrameGraph` class. UE5's transient resource allocator does the same thing: any `FRDGTexture` created through `FRDGBuilder::CreateTexture` (vs `RegisterExternalTexture`) is transient and eligible for aliasing, using the same lifetime analysis and free-list scan we just built.

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
      • <strong>compute barriers</strong>
    </div>
    <div style="margin-top:.5em;padding:.3em .5em;border-radius:5px;background:rgba(var(--ds-code-rgb),.08);font-size:.76em;line-height:1.4;border:1px solid rgba(var(--ds-code-rgb),.12);">
      Everything linear or near-linear — all data fits in L1 cache.
    </div>
  </div>
  <div style="padding:.8em 1em;border-radius:10px;border-top:3px solid var(--ds-success);background:rgba(var(--ds-success-rgb),.04);">
    <div style="font-weight:800;font-size:.88em;margin-bottom:.5em;color:var(--ds-success);">③ Execute</div>
    <div style="font-size:.84em;line-height:1.6;opacity:.85;">
      Walk sorted, living passes:<br>
      • insert automatic barriers<br>
      • call execute lambda<br>
      • resources already aliased &amp; bound
    </div>
    <div style="margin-top:.5em;padding:.3em .5em;border-radius:5px;background:rgba(var(--ds-success-rgb),.08);font-size:.76em;line-height:1.4;border:1px solid rgba(var(--ds-success-rgb),.12);">
      <strong>Lambdas see a fully resolved environment.</strong> No manual barriers, no manual memory.
    </div>
  </div>
</div>

**Compile cost by step:**

<div style="overflow-x:auto;margin:.6em 0 1em">
<table style="width:100%;border-collapse:collapse;font-size:.88em">
  <thead>
    <tr>
      <th style="padding:.5em .8em;text-align:left;border-bottom:2px solid rgba(var(--ds-code-rgb),.3);color:var(--ds-code);width:30%">Compile step</th>
      <th style="padding:.5em .8em;text-align:center;border-bottom:2px solid rgba(var(--ds-code-rgb),.3);width:18%">Complexity</th>
      <th style="padding:.5em .8em;text-align:left;border-bottom:2px solid rgba(var(--ds-code-rgb),.3)">Algorithm</th>
    </tr>
  </thead>
  <tbody>
    <tr><td style="padding:.4em .8em;font-weight:600;">Topological sort</td><td style="padding:.4em .8em;text-align:center;font-family:ui-monospace,monospace;color:var(--ds-code)">O(V + E)</td><td style="padding:.4em .8em;font-size:.9em;opacity:.8">Kahn's — passes + edges</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.4em .8em;font-weight:600;">Pass culling</td><td style="padding:.4em .8em;text-align:center;font-family:ui-monospace,monospace;color:var(--ds-code)">O(V + E)</td><td style="padding:.4em .8em;font-size:.9em;opacity:.8">Backward reachability from output</td></tr>
    <tr><td style="padding:.4em .8em;font-weight:600;">Lifetime scan</td><td style="padding:.4em .8em;text-align:center;font-family:ui-monospace,monospace;color:var(--ds-code)">O(V + E)</td><td style="padding:.4em .8em;font-size:.9em;opacity:.8">Walk sorted passes and their read/write edges</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.4em .8em;font-weight:600;">Aliasing</td><td style="padding:.4em .8em;text-align:center;font-family:ui-monospace,monospace;color:var(--ds-code)">O(R log R)</td><td style="padding:.4em .8em;font-size:.9em;opacity:.8">Sort by first-use, greedy free-list scan</td></tr>
    <tr><td style="padding:.4em .8em;font-weight:600;">Barrier computation</td><td style="padding:.4em .8em;text-align:center;font-family:ui-monospace,monospace;color:var(--ds-code)">O(V + E)</td><td style="padding:.4em .8em;font-size:.9em;opacity:.8">Walk passes and their read/write edges with state lookup</td></tr>
  </tbody>
</table>
</div>
<div style="font-size:.84em;line-height:1.5;opacity:.7;margin:-.3em 0 1em 0">V = passes (~25), E = dependency edges (~50), R = transient resources (~15). Everything linear or near-linear.</div>

That's the full MVP — a single `FrameGraph` class that handles dependency-driven ordering, culling, aliasing, and barriers. Every concept from [Part I](/posts/frame-graph-theory/) now exists as running code.

---

<div style="margin:2em 0 0;padding:1em 1.2em;border-radius:10px;border:1px solid rgba(var(--ds-indigo-rgb),.2);background:rgba(var(--ds-indigo-rgb),.03);display:flex;justify-content:space-between;align-items:center;">
  <a href="../frame-graph-theory/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    ← Previous: Part I — Theory
  </a>
  <a href="../frame-graph-advanced/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    Next: Part III — Beyond MVP →
  </a>
</div>
