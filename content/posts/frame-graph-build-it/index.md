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
📖 <strong>Part II of IV.</strong>&ensp; <a href="../frame-graph-theory/">Theory</a> → <em>Build It</em> → <a href="../frame-graph-advanced/">Advanced Features</a> → <a href="../frame-graph-production/">Production Engines</a>
</div>

*Part I laid out the theory — declare, compile, execute. Now we turn that blueprint into code. Three iterations, each one unlocking a capability that wasn't there before: first a topological sort gives us dependency-driven execution order, then the compiler injects barriers automatically, and finally memory aliasing lets resources share the same heap. Each version builds on the last — time to get our hands dirty.*

<!-- MVP progression — stacked power-up bars -->
<div style="margin:1.6em 0 1.2em;position:relative;padding-left:2.6em;">
  <!-- header -->
  <div style="margin-left:-2.6em;margin-bottom:1.2em;font-size:1.1em;font-weight:800;text-align:center;letter-spacing:.02em;">🧬 MVP Progression</div>
  <!-- vertical connector line -->
  <div style="position:absolute;left:1.05em;top:.4em;bottom:.4em;width:2px;background:linear-gradient(to bottom, var(--ds-info), var(--ds-code), var(--ds-success));border-radius:1px;opacity:.5;"></div>

  <!-- v1 -->
  <a href="#-v1--the-scaffold" style="text-decoration:none;color:inherit;display:block;position:relative;margin-bottom:1.2em;cursor:pointer;" onmouseover="this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-info-rgb),.5)'" onmouseout="this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-info-rgb),.25)'">
    <div style="position:absolute;left:-2.6em;top:.15em;width:2.1em;height:2.1em;border-radius:50%;background:var(--ds-info);display:flex;align-items:center;justify-content:center;font-weight:900;font-size:.7em;color:#fff;box-shadow:0 0 0 3px rgba(var(--ds-info-rgb),.2);z-index:1;">v1</div>
    <div class="mvp-card" style="padding:.7em .9em;border-radius:8px;border:1.5px solid rgba(var(--ds-info-rgb),.25);background:linear-gradient(90deg,rgba(var(--ds-info-rgb),.06) 0%,transparent 100%);transition:border-color .15s;">
      <div style="font-weight:800;font-size:.95em;color:var(--ds-info);margin-bottom:.25em;">The Scaffold</div>
      <div style="font-size:.84em;line-height:1.5;opacity:.85;">Pass declaration, virtual resources, execute in order. The skeleton everything else plugs into.</div>
      <!-- power bar -->
      <div style="margin-top:.5em;height:6px;border-radius:3px;background:rgba(127,127,127,.1);overflow:hidden;">
        <div style="width:20%;height:100%;border-radius:3px;background:var(--ds-info);"></div>
      </div>
      <div style="display:flex;justify-content:space-between;margin-top:.2em;font-size:.65em;opacity:.5;"><span>declare</span><span>compile</span><span>execute</span></div>
    </div>
  </a>

  <!-- v2 -->
  <a href="#-mvp-v2--dependencies--barriers" style="text-decoration:none;color:inherit;display:block;position:relative;margin-bottom:1.2em;cursor:pointer;" onmouseover="this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-code-rgb),.5)'" onmouseout="this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-code-rgb),.25)'">
    <div style="position:absolute;left:-2.6em;top:.15em;width:2.1em;height:2.1em;border-radius:50%;background:var(--ds-code);display:flex;align-items:center;justify-content:center;font-weight:900;font-size:.7em;color:#fff;box-shadow:0 0 0 3px rgba(var(--ds-code-rgb),.2);z-index:1;">v2</div>
    <div class="mvp-card" style="padding:.7em .9em;border-radius:8px;border:1.5px solid rgba(var(--ds-code-rgb),.25);background:linear-gradient(90deg,rgba(var(--ds-code-rgb),.06) 0%,transparent 100%);transition:border-color .15s;">
      <div style="font-weight:800;font-size:.95em;color:var(--ds-code);margin-bottom:.25em;">Dependencies & Barriers</div>
      <div style="font-size:.84em;line-height:1.5;opacity:.85;">Resource versioning → edges → topo-sort → dead-pass culling → automatic barrier insertion. The core value of a render graph.</div>
      <!-- power bar -->
      <div style="margin-top:.5em;height:6px;border-radius:3px;background:rgba(127,127,127,.1);overflow:hidden;">
        <div style="width:65%;height:100%;border-radius:3px;background:linear-gradient(90deg,var(--ds-info),var(--ds-code));"></div>
      </div>
      <div style="display:flex;justify-content:space-between;margin-top:.2em;font-size:.65em;opacity:.5;"><span>declare</span><span>compile</span><span>execute</span></div>
    </div>
  </a>

  <!-- v3 -->
  <a href="#-mvp-v3--lifetimes--aliasing" style="text-decoration:none;color:inherit;display:block;position:relative;cursor:pointer;" onmouseover="this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-success-rgb),.5)'" onmouseout="this.querySelector('.mvp-card').style.borderColor='rgba(var(--ds-success-rgb),.25)'">
    <div style="position:absolute;left:-2.6em;top:.15em;width:2.1em;height:2.1em;border-radius:50%;background:var(--ds-success);display:flex;align-items:center;justify-content:center;font-weight:900;font-size:.7em;color:#fff;box-shadow:0 0 0 3px rgba(var(--ds-success-rgb),.2), 0 0 12px rgba(var(--ds-success-rgb),.3);z-index:1;">v3</div>
    <div class="mvp-card" style="padding:.7em .9em;border-radius:8px;border:1.5px solid rgba(var(--ds-success-rgb),.25);background:linear-gradient(90deg,rgba(var(--ds-success-rgb),.06) 0%,transparent 100%);transition:border-color .15s;">
      <div style="font-weight:800;font-size:.95em;color:var(--ds-success);margin-bottom:.25em;">Lifetimes & Aliasing <span style="font-size:.75em;font-weight:600;opacity:.7;margin-left:.4em;">★ production-ready</span></div>
      <div style="font-size:.84em;line-height:1.5;opacity:.85;">Lifetime scan + greedy free-list allocator. Non-overlapping resources share physical memory — <strong>~50% VRAM saved</strong>.</div>
      <!-- power bar -->
      <div style="margin-top:.5em;height:6px;border-radius:3px;background:rgba(127,127,127,.1);overflow:hidden;">
        <div style="width:100%;height:100%;border-radius:3px;background:linear-gradient(90deg,var(--ds-info),var(--ds-code),var(--ds-success));"></div>
      </div>
      <div style="display:flex;justify-content:space-between;margin-top:.2em;font-size:.65em;opacity:.5;"><span>declare</span><span>compile</span><span>execute</span></div>
    </div>
  </a>
</div>

---

## 🏗️ API Design & First Prototype

We start from the API you *want* to write, then build toward it — starting with bare scaffolding and ending with automatic barriers and memory aliasing.

### 🎯 Design principles

<div style="margin:1em 0 1.4em;display:grid;grid-template-columns:repeat(3,1fr);gap:.8em;">
  <div style="padding:1em;border-radius:10px;border-top:3px solid var(--ds-info);background:rgba(var(--ds-info-rgb),.04);">
    <div style="font-weight:800;font-size:.92em;margin-bottom:.4em;color:var(--ds-info);">λ² &ensp;Two lambdas</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      <strong>Setup</strong> — runs at declaration. Declares reads &amp; writes. No GPU work.<br>
      <strong>Execute</strong> — runs later. Records GPU commands into a fully resolved environment.
    </div>
  </div>
  <div style="padding:1em;border-radius:10px;border-top:3px solid var(--ds-code);background:rgba(var(--ds-code-rgb),.04);">
    <div style="font-weight:800;font-size:.92em;margin-bottom:.4em;color:var(--ds-code);">📐 &ensp;Virtual resources</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      Requested by description (<code>{1920, 1080, RGBA8}</code>), not GPU handle. Virtual until the compiler maps them to memory.
    </div>
  </div>
  <div style="padding:1em;border-radius:10px;border-top:3px solid var(--ds-success);background:rgba(var(--ds-success-rgb),.04);">
    <div style="font-weight:800;font-size:.92em;margin-bottom:.4em;color:var(--ds-success);">♻️ &ensp;Owned lifetimes</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      The graph owns every transient resource from first use to last. You never call create or destroy.
    </div>
  </div>
</div>

These three ideas produce a natural pipeline — declare your intent, let the compiler optimize, then execute:

<!-- Timeline: declaration → compile → execution -->
<div class="diagram-phases">
  <div class="dph-col" style="border-color:var(--ds-info)">
    <div class="dph-title" style="color:var(--ds-info)">① Declaration <span style="font-weight:400;font-size:.75em;opacity:.7;">CPU</span></div>
    <div class="dph-body">
      <code>addPass(setup, execute)</code><br>
      ├ setup lambda runs<br>
      &nbsp;&nbsp;• declare reads / writes<br>
      &nbsp;&nbsp;• request resources<br>
      └ <strong>no GPU work, no allocation</strong>
      <div style="margin-top:.6em;padding:.35em .6em;border-radius:5px;background:rgba(var(--ds-info-rgb),.08);font-size:.82em;line-height:1.4;border:1px solid rgba(var(--ds-info-rgb),.12);">
        Resources are <strong>virtual</strong> — just a description + handle index. Zero bytes allocated.
      </div>
    </div>
  </div>
  <div class="dph-col" style="border-color:var(--ds-code)">
    <div class="dph-title" style="color:var(--ds-code)">② Compile <span style="font-weight:400;font-size:.75em;opacity:.7;">CPU</span></div>
    <div class="dph-body">
      ├ <strong>sort</strong> — topo order (Kahn's)<br>
      ├ <strong>cull</strong> — remove dead passes<br>
      ├ <strong>alias</strong> — map virtual → physical<br>
      └ <strong>barrier</strong> — emit transitions
      <div style="margin-top:.6em;padding:.35em .6em;border-radius:5px;background:rgba(var(--ds-code-rgb),.08);font-size:.82em;line-height:1.4;border:1px solid rgba(var(--ds-code-rgb),.12);">
        Aliasing and allocation <strong>happen</strong> here — non-overlapping lifetimes share the same heap, physical memory is bound before execute.
      </div>
    </div>
  </div>
  <div class="dph-col" style="border-color:var(--ds-success)">
    <div class="dph-title" style="color:var(--ds-success)">③ Execute <span style="font-weight:400;font-size:.75em;opacity:.7;">GPU</span></div>
    <div class="dph-body">
      for each pass in sorted order:<br>
      ├ insert automatic barriers<br>
      └ call execute lambda<br>
      &nbsp;&nbsp;→ draw / dispatch / copy
      <div style="margin-top:.6em;padding:.35em .6em;border-radius:5px;background:rgba(var(--ds-success-rgb),.08);font-size:.82em;line-height:1.4;border:1px solid rgba(var(--ds-success-rgb),.12);">
        Lambdas see a <strong>fully resolved</strong> environment — memory bound, barriers placed, resources ready.
      </div>
    </div>
  </div>
</div>

### 🚀 The Target API

Here's where we're headed — the final API in under 30 lines:

{{< include-code file="api_demo.cpp" lang="cpp" open="true" >}}

### 🧱 v1 — The Scaffold

Three types are all we need to start: a `RenderPass` with setup + execute lambdas, a `ResourceDesc` (width, height, format — no GPU handle yet), and a `ResourceHandle` that's just an index. The `FrameGraph` class owns arrays of both and runs passes in declaration order. No dependency tracking, no barriers — just the foundation that v2 and v3 build on.

{{< include-code file="frame_graph_v1.h" lang="cpp" compact="true" >}}
{{< include-code file="example_v1.cpp" lang="cpp" compile="true" deps="frame_graph_v1.h" compact="true" >}}

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
🎯 <strong>Goal:</strong> Automatic pass ordering, dead-pass culling, and barrier insertion — the core value of a render graph.
</div>

Four pieces, each feeding the next:

<div style="margin:.8em 0 1.2em;display:grid;grid-template-columns:repeat(4,1fr);gap:0;border-radius:10px;overflow:hidden;border:1.5px solid rgba(var(--ds-indigo-rgb),.2);">
  <a href="#v2-versioning" style="padding:.7em .6em .5em;background:rgba(var(--ds-info-rgb),.05);border-right:1px solid rgba(var(--ds-indigo-rgb),.12);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(var(--ds-info-rgb),.12)'" onmouseout="this.style.background='rgba(var(--ds-info-rgb),.05)'">
    <div style="font-size:1.2em;margin-bottom:.15em;">🔀</div>
    <div style="font-weight:800;font-size:.85em;color:var(--ds-info);">Versioning</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">reads/writes → edges</div>
  </a>
  <a href="#v2-toposort" style="padding:.7em .6em .5em;background:rgba(var(--ds-code-rgb),.05);border-right:1px solid rgba(var(--ds-indigo-rgb),.12);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(var(--ds-code-rgb),.12)'" onmouseout="this.style.background='rgba(var(--ds-code-rgb),.05)'">
    <div style="font-size:1.2em;margin-bottom:.15em;">📦</div>
    <div style="font-weight:800;font-size:.85em;color:var(--ds-code);">Topo Sort</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">edges → execution order</div>
  </a>
  <a href="#v2-culling" style="padding:.7em .6em .5em;background:rgba(var(--ds-warn-rgb),.05);border-right:1px solid rgba(var(--ds-indigo-rgb),.12);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(var(--ds-warn-rgb),.12)'" onmouseout="this.style.background='rgba(var(--ds-warn-rgb),.05)'">
    <div style="font-size:1.2em;margin-bottom:.15em;">✂️</div>
    <div style="font-weight:800;font-size:.85em;color:var(--ds-warn);">Pass Culling</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">kill unreachable passes</div>
  </a>
  <a href="#v2-barriers" style="padding:.7em .6em .5em;background:rgba(var(--ds-danger-rgb),.05);text-decoration:none;text-align:center;transition:background .15s;" onmouseover="this.style.background='rgba(var(--ds-danger-rgb),.12)'" onmouseout="this.style.background='rgba(var(--ds-danger-rgb),.05)'">
    <div style="font-size:1.2em;margin-bottom:.15em;">🚧</div>
    <div style="font-weight:800;font-size:.85em;color:var(--ds-danger);">Barriers</div>
    <div style="font-size:.72em;opacity:.6;margin-top:.15em;line-height:1.3;">emit GPU transitions</div>
  </a>
</div>

<span id="v2-versioning"></span>

### 🔀 Resource versioning — the data structure

[Part I](/posts/frame-graph-theory/#how-edges-form--resource-versioning) introduced resource versioning — each write bumps a version number, readers attach to the current version, and that's what creates precise dependency edges. Here we implement it.

The key data structure: each resource entry tracks its **current version** (incremented on write) and a **writer pass index** per version. When a pass calls `read(h)`, the graph looks up the current version's writer and adds a dependency edge from that writer to the reading pass.

Here's what changes from v1. The `ResourceDesc` array becomes `ResourceEntry` — each entry carries a version list. `RenderPass` gains dependency tracking fields. And two new methods, `read()` and `write()`, wire everything together:

{{< code-diff title="v1 → v2 — Resource versioning & dependency tracking" >}}
@@ New types (resource state + version tracking) @@
+enum class ResourceState { Undefined, ColorAttachment, DepthAttachment,
+                           ShaderRead, Present };
+
+struct ResourceVersion {                 // NEW v2
+    uint32_t writerPass = UINT32_MAX;    // which pass wrote this version
+    std::vector<uint32_t> readerPasses;  // which passes read it
+};
+
+struct ResourceEntry {
+    ResourceDesc desc;
+    std::vector<ResourceVersion> versions;  // version 0, 1, 2...
+    ResourceState currentState = ResourceState::Undefined;
+};

@@ RenderPass — new fields @@
 struct RenderPass {
     std::string name;
     std::function<void()>             setup;
     std::function<void(/*cmd list*/)> execute;
+    std::vector<ResourceHandle> reads;     // NEW v2
+    std::vector<ResourceHandle> writes;    // NEW v2
+    std::vector<uint32_t> dependsOn;       // NEW v2
+    std::vector<uint32_t> successors;      // NEW v2
+    uint32_t inDegree = 0;                 // NEW v2 (Kahn's)
+    bool     alive    = false;             // NEW v2 (culling)
 };

@@ FrameGraph — read/write methods @@
+    void read(uint32_t passIdx, ResourceHandle h) {
+        auto& ver = entries_[h.index].versions.back();
+        if (ver.writerPass != UINT32_MAX)
+            passes_[passIdx].dependsOn.push_back(ver.writerPass);
+        ver.readerPasses.push_back(passIdx);
+        passes_[passIdx].reads.push_back(h);
+    }
+
+    void write(uint32_t passIdx, ResourceHandle h) {
+        entries_[h.index].versions.push_back({});
+        entries_[h.index].versions.back().writerPass = passIdx;
+        passes_[passIdx].writes.push_back(h);
+    }

@@ Storage @@
-    std::vector<ResourceDesc>  resources_;
+    std::vector<ResourceEntry> entries_;  // now with versioning
{{< /code-diff >}}

Every `write()` pushes a new version. Every `read()` finds the current version's writer and records a `dependsOn` edge. Those edges feed the next three steps.

---

<span id="v2-toposort"></span>

### 📊 Topological sort (Kahn's algorithm)

[Part I](/posts/frame-graph-theory/#sorting-and-culling) walked through Kahn's algorithm step by step. Here's the implementation. `buildEdges()` deduplicates the raw `dependsOn` entries and builds the adjacency list; `topoSort()` does the zero-in-degree queue drain:

{{< code-diff title="v2 — Edge building + Kahn's topological sort" >}}
@@ buildEdges() — deduplicate and build adjacency list @@
+    void buildEdges() {
+        for (uint32_t i = 0; i < passes_.size(); i++) {
+            std::unordered_set<uint32_t> seen;
+            for (uint32_t dep : passes_[i].dependsOn) {
+                if (seen.insert(dep).second) {
+                    passes_[dep].successors.push_back(i);
+                    passes_[i].inDegree++;
+                }
+            }
+        }
+    }

@@ topoSort() — Kahn's algorithm, O(V + E) @@
+    std::vector<uint32_t> topoSort() {
+        std::queue<uint32_t> q;
+        std::vector<uint32_t> inDeg(passes_.size());
+        for (uint32_t i = 0; i < passes_.size(); i++) {
+            inDeg[i] = passes_[i].inDegree;
+            if (inDeg[i] == 0) q.push(i);
+        }
+        std::vector<uint32_t> order;
+        while (!q.empty()) {
+            uint32_t cur = q.front(); q.pop();
+            order.push_back(cur);
+            for (uint32_t succ : passes_[cur].successors) {
+                if (--inDeg[succ] == 0) q.push(succ);
+            }
+        }
+        assert(order.size() == passes_.size() && "Cycle detected!");
+        return order;
+    }
{{< /code-diff >}}

---

<span id="v2-culling"></span>

### ✂️ Pass culling

[Part I](/posts/frame-graph-theory/#sorting-and-culling) described culling as "dead-code elimination for GPU work." The implementation is a single backward walk — mark the final pass alive, then propagate backward through `dependsOn` edges:

{{< code-diff title="v2 — Pass culling" >}}
@@ cull() — backward reachability from output @@
+    void cull(const std::vector<uint32_t>& sorted) {
+        if (sorted.empty()) return;
+        passes_[sorted.back()].alive = true;   // last pass = output
+        for (int i = (int)sorted.size() - 1; i >= 0; i--) {
+            if (!passes_[sorted[i]].alive) continue;
+            for (uint32_t dep : passes_[sorted[i]].dependsOn)
+                passes_[dep].alive = true;
+        }
+    }
{{< /code-diff >}}

---

<span id="v2-barriers"></span>

### 🚧 Barrier insertion

[Part I](/posts/frame-graph-theory/#barriers) explained *why* barriers exist and how the compiler infers them from read/write edges. The implementation walks each pass's resources, comparing the tracked state to what the pass needs. If they differ — emit a barrier and update:

{{< code-diff title="v2 — Barrier insertion + execute() rewrite" >}}
@@ insertBarriers() — emit transitions where state changes @@
+    void insertBarriers(uint32_t passIdx) {
+        auto stateForUsage = [](bool isWrite, Format fmt) {
+            if (isWrite)
+                return (fmt == Format::D32F) ? ResourceState::DepthAttachment
+                                             : ResourceState::ColorAttachment;
+            return ResourceState::ShaderRead;
+        };
+        for (auto& h : passes_[passIdx].reads) {
+            ResourceState needed = ResourceState::ShaderRead;
+            if (entries_[h.index].currentState != needed) {
+                // emit barrier: old state → new state
+                entries_[h.index].currentState = needed;
+            }
+        }
+        for (auto& h : passes_[passIdx].writes) {
+            ResourceState needed = stateForUsage(true, entries_[h.index].desc.format);
+            if (entries_[h.index].currentState != needed) {
+                entries_[h.index].currentState = needed;
+            }
+        }
+    }

@@ execute() — the full v2 pipeline @@
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
{{< /code-diff >}}

All four pieces — versioning, sorting, culling, barriers — compose into that `execute()` body. Each step feeds the next: versioning creates edges, edges feed the sort, the sort enables culling, and the surviving sorted passes get automatic barriers.

---

### 🧩 Full v2 source

{{< include-code file="frame_graph_v2.h" lang="cpp" compact="true" >}}
{{< include-code file="example_v2.cpp" lang="cpp" compile="true" deps="frame_graph_v2.h" compact="true" >}}

That's three of the four intro promises delivered — automatic ordering, barrier insertion, and dead-pass culling. The only piece missing: resources still live for the entire frame. Version 3 fixes that with lifetime analysis and memory aliasing.

UE5's RDG does the same thing. When you call `FRDGBuilder::AddPass`, RDG builds the dependency graph from your declared reads/writes, topologically sorts it, culls dead passes, and inserts barriers — all before recording a single GPU command.

---

## 💾 MVP v3 — Lifetimes & Aliasing

V2 gives us ordering, culling, and barriers — but every transient resource still gets its own VRAM for the entire frame. [Part I](/posts/frame-graph-theory/#allocation-and-aliasing) showed how non-overlapping lifetimes let the allocator share physical memory (aliasing). Now we implement it.

Two new structs — a `Lifetime` per resource and a `PhysicalBlock` per heap slot. The lifetime scan walks the sorted pass list, recording each transient resource's `firstUse` / `lastUse` indices:

{{< code-diff title="v2 → v3 — Lifetime structs & scan" >}}
@@ New structs @@
+struct PhysicalBlock {              // physical memory slot
+    uint32_t sizeBytes  = 0;
+    Format   format     = Format::RGBA8;
+    uint32_t availAfter = 0;        // free after this pass index
+};
+
+struct Lifetime {                   // per-resource timing
+    uint32_t firstUse = UINT32_MAX;
+    uint32_t lastUse  = 0;
+    bool     isTransient = true;
+};

@@ scanLifetimes() — walk sorted passes, record first/last use @@
+    std::vector<Lifetime> scanLifetimes(const std::vector<uint32_t>& sorted) {
+        std::vector<Lifetime> life(entries_.size());
+        for (uint32_t order = 0; order < sorted.size(); order++) {
+            if (!passes_[sorted[order]].alive) continue;
+            for (auto& h : passes_[sorted[order]].reads) {
+                life[h.index].firstUse = std::min(life[h.index].firstUse, order);
+                life[h.index].lastUse  = std::max(life[h.index].lastUse,  order);
+            }
+            for (auto& h : passes_[sorted[order]].writes) {
+                life[h.index].firstUse = std::min(life[h.index].firstUse, order);
+                life[h.index].lastUse  = std::max(life[h.index].lastUse,  order);
+            }
+        }
+        return life;
+    }
{{< /code-diff >}}

This requires **placed resources** at the API level — GPU memory allocated from a heap, with resources bound to offsets within it. In D3D12, that means `ID3D12Heap` + `CreatePlacedResource`. In Vulkan, `VkDeviceMemory` + `vkBindImageMemory` at different offsets. Without placed resources (i.e., `CreateCommittedResource` or Vulkan dedicated allocations), each resource gets its own memory and aliasing is impossible — which is why the graph's allocator works with heaps.

The second half of the algorithm — the greedy free-list allocator. Sort resources by `firstUse`, then try to fit each one into an existing block whose previous user has finished:

{{< code-diff title="v3 — Greedy free-list aliasing + compile() integration" >}}
@@ aliasResources() — greedy free-list scan @@
+    std::vector<uint32_t> aliasResources(const std::vector<Lifetime>& lifetimes) {
+        std::vector<PhysicalBlock> freeList;
+        std::vector<uint32_t> mapping(entries_.size(), UINT32_MAX);
+
+        // sort resources by firstUse
+        std::vector<uint32_t> indices(entries_.size());
+        std::iota(indices.begin(), indices.end(), 0);
+        std::sort(indices.begin(), indices.end(), [&](uint32_t a, uint32_t b) {
+            return lifetimes[a].firstUse < lifetimes[b].firstUse;
+        });
+
+        for (uint32_t resIdx : indices) {
+            if (!lifetimes[resIdx].isTransient) continue;
+            if (lifetimes[resIdx].firstUse == UINT32_MAX) continue;
+            uint32_t needed = /* width * height * bpp */;
+            bool reused = false;
+            for (uint32_t b = 0; b < freeList.size(); b++) {
+                if (freeList[b].availAfter < lifetimes[resIdx].firstUse
+                    && freeList[b].sizeBytes >= needed) {
+                    mapping[resIdx] = b;         // reuse this block
+                    freeList[b].availAfter = lifetimes[resIdx].lastUse;
+                    reused = true; break;
+                }
+            }
+            if (!reused) {
+                mapping[resIdx] = freeList.size();
+                freeList.push_back({ needed, fmt, lifetimes[resIdx].lastUse });
+            }
+        }
+        return mapping;
+    }

@@ compile() — v3 adds lifetime scan + aliasing @@
     auto sorted = topoSort();
     cull(sorted);
+    auto lifetimes = scanLifetimes(sorted);     // NEW v3
+    auto mapping   = aliasResources(lifetimes); // NEW v3
+    // mapping[virtualIdx] → physicalBlock — execute just runs passes
{{< /code-diff >}}

~70 new lines on top of v2. Aliasing runs once per frame in O(R log R) — sort, then linear scan of the free list. Sub-microsecond for 15 transient resources.

That's the full value prop — automatic memory aliasing *and* automatic barriers from a single `FrameGraph` class. UE5's transient resource allocator does the same thing: any `FRDGTexture` created through `FRDGBuilder::CreateTexture` (vs `RegisterExternalTexture`) is transient and eligible for aliasing, using the same lifetime analysis and free-list scan we just built.

---

### 🧩 Full v3 source

{{< include-code file="frame_graph_v3.h" lang="cpp" compact="true" >}}
{{< include-code file="example_v3.cpp" lang="cpp" compile="true" deps="frame_graph_v3.h" compact="true" >}}

---

### ✅ What the MVP delivers

The finished `FrameGraph` class. Here's what it does every frame, broken down by phase — the same declare → compile → execute lifecycle from [Part I](/posts/frame-graph-theory/):

<div style="margin:1.2em 0;display:grid;grid-template-columns:repeat(3,1fr);gap:.8em;">
  <div style="padding:.8em 1em;border-radius:10px;border-top:3px solid var(--ds-info);background:rgba(var(--ds-info-rgb),.04);">
    <div style="font-weight:800;font-size:.88em;margin-bottom:.5em;color:var(--ds-info);">① Declare</div>
    <div style="font-size:.84em;line-height:1.6;opacity:.85;">
      Each <code>addPass</code> runs its setup lambda:<br>
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
    Next: Part III — Advanced Features →
  </a>
</div>
