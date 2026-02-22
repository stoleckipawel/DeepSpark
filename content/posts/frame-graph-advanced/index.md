---
title: "Frame Graph — Beyond MVP"
date: 2026-02-10T12:00:00
draft: false
description: "Async compute and split barriers — how the frame graph compiler squeezes more performance from the same DAG."
tags: ["rendering", "frame-graph", "gpu", "architecture"]
categories: ["analysis"]
series: ["Rendering Architecture"]
showTableOfContents: false
---

{{< article-nav >}}

<div style="margin:0 0 1.5em;padding:.7em 1em;border-radius:8px;background:rgba(var(--ds-indigo-rgb),.04);border:1px solid rgba(var(--ds-indigo-rgb),.12);font-size:.88em;line-height:1.6;opacity:.85;">
📖 <strong>Part III of IV.</strong>&ensp; <a href="../frame-graph-theory/">Theory</a> → <a href="../frame-graph-build-it/">Build It</a> → <em>Beyond MVP</em> → <a href="../frame-graph-production/">Production Engines</a>
</div>

[Part I](/posts/frame-graph-theory/) covered the core — sorting, culling, barriers, aliasing — and [Part II](/posts/frame-graph-build-it/) built it in C++. The same DAG enables the compiler to go further. It can schedule independent work across GPU queues and split barrier transitions to hide cache-flush latency.

---

## ⚡ Async Compute

Barriers optimize work on a single GPU queue. But modern GPUs expose at least two: a **graphics queue** and a **compute queue**. If two passes have **no dependency path between them** in the DAG, the compiler can schedule them on different queues simultaneously.

### 🔍 Finding parallelism

The compiler needs to answer one question for every pair of passes: **can these run at the same time?** Two passes can overlap only if neither depends on the other — directly or indirectly. A pass that writes the GBuffer can't overlap with lighting (which reads it), but it *can* overlap with SSAO if they share no resources.

The algorithm is called **reachability analysis** — for each pass, the compiler figures out every other pass it can eventually reach by following edges forward through the DAG. If pass A can reach pass B (or B can reach A), they're dependent. If neither can reach the other, they're **independent** — safe to run on separate queues.

### 🚧 Minimizing fences

Cross-queue work needs **GPU fences** — one queue signals, the other waits. Each fence adds dead GPU time — the exact cost varies by architecture (older GCN-era GPUs could hit 10–15 µs; Ada Lovelace and RDNA 3 are often lower). Move SSAO, volumetrics, and particle sim to compute and you can create several fences, and if each costs even a few microseconds, the accumulated idle time can erase the overlap gain. The compiler applies **transitive reduction** to collapse those down:

<div class="fg-grid-stagger" style="display:grid;grid-template-columns:1fr 1fr;gap:1em;margin:1.2em 0;">
  <div class="fg-hoverable" style="border-radius:10px;border:1.5px solid rgba(var(--ds-danger-rgb),.25);overflow:hidden;">
    <div style="padding:.55em .9em;background:rgba(var(--ds-danger-rgb),.06);border-bottom:1px solid rgba(var(--ds-danger-rgb),.12);font-weight:800;font-size:.85em;text-transform:uppercase;letter-spacing:.04em;color:var(--ds-danger);">Naive — 4 fences</div>
    <div style="padding:.8em .9em;font-family:ui-monospace,monospace;font-size:.82em;line-height:1.8;">
      <span style="opacity:.5;">Graphics:</span> [A] ──<span style="color:var(--ds-danger);font-weight:600;">fence</span>──→ [C]<br>
      <span style="opacity:.3;">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>└──<span style="color:var(--ds-danger);font-weight:600;">fence</span>──→ [D]<br>
      <br>
      <span style="opacity:.5;">Compute:</span>&nbsp; [B] ──<span style="color:var(--ds-danger);font-weight:600;">fence</span>──→ [C]<br>
      <span style="opacity:.3;">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>└──<span style="color:var(--ds-danger);font-weight:600;">fence</span>──→ [D]
    </div>
    <div style="padding:.5em .9em;border-top:1px solid rgba(var(--ds-danger-rgb),.1);font-size:.78em;opacity:.7;">Every cross-queue edge gets its own fence</div>
  </div>
  <div class="fg-hoverable" style="border-radius:10px;border:1.5px solid rgba(var(--ds-success-rgb),.25);overflow:hidden;">
    <div style="padding:.55em .9em;background:rgba(var(--ds-success-rgb),.06);border-bottom:1px solid rgba(var(--ds-success-rgb),.12);font-weight:800;font-size:.85em;text-transform:uppercase;letter-spacing:.04em;color:var(--ds-success);">Reduced — 1 fence</div>
    <div style="padding:.8em .9em;font-family:ui-monospace,monospace;font-size:.82em;line-height:1.8;">
      <span style="opacity:.5;">Graphics:</span> [A] ─────────→ [C] → [D]<br>
      <span style="opacity:.3;">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>↑<br>
      <span style="opacity:.5;">Compute:</span>&nbsp; [B] ──<span style="color:var(--ds-success);font-weight:600;">fence</span>──┘<br>
      <br>
      B's fence covers both C and D<br>
      <span style="opacity:.6;">(D is after C on graphics queue)</span>
    </div>
    <div style="padding:.5em .9em;border-top:1px solid rgba(var(--ds-success-rgb),.1);font-size:.78em;color:var(--ds-success);font-weight:600;">Redundant fences removed transitively</div>
  </div>
</div>

### ⚖ What makes overlap good or bad

Solving fences is the easy part — the compiler handles that. The harder question is whether overlapping two specific passes actually helps:

<div class="fg-grid-stagger" style="display:grid;grid-template-columns:1fr 1fr;gap:1em;margin:1.2em 0;">
  <div class="fg-hoverable" style="border-radius:10px;border:1.5px solid rgba(var(--ds-success-rgb),.25);overflow:hidden;">
    <div style="padding:.6em .9em;background:rgba(var(--ds-success-rgb),.05);border-bottom:1px solid rgba(var(--ds-success-rgb),.12);font-weight:800;font-size:.85em;text-transform:uppercase;letter-spacing:.04em;color:var(--ds-success);">✓ Complementary</div>
    <div style="padding:.8em .9em;font-size:.88em;line-height:1.6;">
      Graphics is <strong>ROP/rasterizer-bound</strong> (shadow rasterization, geometry-dense passes) while compute runs <strong>ALU-heavy</strong> shaders (SSAO, volumetrics). Different hardware units stay busy — real parallelism, measurable frame time reduction.
    </div>
  </div>
  <div class="fg-hoverable" style="border-radius:10px;border:1.5px solid rgba(var(--ds-danger-rgb),.25);overflow:hidden;">
    <div style="padding:.6em .9em;background:rgba(var(--ds-danger-rgb),.05);border-bottom:1px solid rgba(var(--ds-danger-rgb),.12);font-weight:800;font-size:.85em;text-transform:uppercase;letter-spacing:.04em;color:var(--ds-danger);">✗ Competing</div>
    <div style="padding:.8em .9em;font-size:.88em;line-height:1.6;">
      Both passes are <strong>bandwidth-bound</strong> or both <strong>ALU-heavy</strong> — they thrash each other's L2 cache and fight for CU time. The frame gets <em>slower</em> than running them sequentially. Common trap: overlapping two fullscreen post-effects.
    </div>
  </div>
</div>

### 🧭 Should this pass go async?

<div style="margin:1.5em 0;display:flex;align-items:stretch;gap:0;font-size:.88em;">
  <div style="flex:1;min-width:0;display:flex;flex-direction:column;align-items:center;">
    <div style="width:100%;flex:1;display:flex;align-items:center;justify-content:center;padding:.7em .6em;border-radius:8px;background:rgba(var(--ds-indigo-rgb),.06);border:1.5px solid rgba(var(--ds-indigo-rgb),.15);text-align:center;font-weight:700;">Is Compute Shader?</div>
    <div style="margin-top:.4em;font-size:.78em;color:var(--ds-danger);text-align:center;line-height:1.35;opacity:.85;">✗ requires raster pipeline</div>
  </div>
  <div style="display:flex;align-items:center;padding:0 .2em;flex-shrink:0;"><svg viewBox="0 0 44 28" width="44" height="28" fill="none"><line x1="4" y1="14" x2="28" y2="14" stroke="var(--ds-success)" stroke-width="2" stroke-linecap="round" opacity=".15"/><line x1="4" y1="14" x2="28" y2="14" class="flow flow-md flow-green" style="animation-duration:1.8s"/><polyline points="24,5 38,14 24,23" stroke="var(--ds-success)" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round" fill="none" opacity=".35"/></svg></div>
  <div style="flex:1;min-width:0;display:flex;flex-direction:column;align-items:center;">
    <div style="width:100%;flex:1;display:flex;align-items:center;justify-content:center;padding:.7em .6em;border-radius:8px;background:rgba(var(--ds-indigo-rgb),.06);border:1.5px solid rgba(var(--ds-indigo-rgb),.15);text-align:center;font-weight:700;">Zero Resource Contention with Graphics?</div>
    <div style="margin-top:.4em;font-size:.78em;color:var(--ds-danger);text-align:center;line-height:1.35;opacity:.85;">✗ data hazard with graphics</div>
  </div>
  <div style="display:flex;align-items:center;padding:0 .2em;flex-shrink:0;"><svg viewBox="0 0 44 28" width="44" height="28" fill="none"><line x1="4" y1="14" x2="28" y2="14" stroke="var(--ds-success)" stroke-width="2" stroke-linecap="round" opacity=".15"/><line x1="4" y1="14" x2="28" y2="14" class="flow flow-md flow-green"/><polyline points="24,5 38,14 24,23" stroke="var(--ds-success)" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round" fill="none" opacity=".35"/></svg></div>
  <div style="flex:1;min-width:0;display:flex;flex-direction:column;align-items:center;">
    <div style="width:100%;flex:1;display:flex;align-items:center;justify-content:center;padding:.7em .6em;border-radius:8px;background:rgba(var(--ds-indigo-rgb),.06);border:1.5px solid rgba(var(--ds-indigo-rgb),.15);text-align:center;font-weight:700;">Has Complementary Resource Usage?</div>
    <div style="margin-top:.4em;font-size:.78em;color:var(--ds-danger);text-align:center;line-height:1.35;opacity:.85;">✗ same HW units — no overlap</div>
  </div>
  <div style="display:flex;align-items:center;padding:0 .2em;flex-shrink:0;"><svg viewBox="0 0 44 28" width="44" height="28" fill="none"><line x1="4" y1="14" x2="28" y2="14" stroke="var(--ds-success)" stroke-width="2" stroke-linecap="round" opacity=".15"/><line x1="4" y1="14" x2="28" y2="14" class="flow flow-md flow-green" style="animation-duration:2.2s"/><polyline points="24,5 38,14 24,23" stroke="var(--ds-success)" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round" fill="none" opacity=".35"/></svg></div>
  <div style="flex:1;min-width:0;display:flex;flex-direction:column;align-items:center;">
    <div style="width:100%;flex:1;display:flex;align-items:center;justify-content:center;padding:.7em .6em;border-radius:8px;background:rgba(var(--ds-indigo-rgb),.06);border:1.5px solid rgba(var(--ds-indigo-rgb),.15);text-align:center;font-weight:700;">Has Enough Work Between Fences?</div>
    <div style="margin-top:.4em;font-size:.78em;color:var(--ds-danger);text-align:center;line-height:1.35;opacity:.85;">✗ sync cost exceeds gain</div>
  </div>
  <div style="display:flex;align-items:center;padding:0 .2em;flex-shrink:0;"><svg viewBox="0 0 44 28" width="44" height="28" fill="none"><line x1="4" y1="14" x2="28" y2="14" stroke="var(--ds-success)" stroke-width="2" stroke-linecap="round" opacity=".15"/><line x1="4" y1="14" x2="28" y2="14" class="flow flow-md flow-green" style="animation-duration:1.7s"/><polyline points="24,5 38,14 24,23" stroke="var(--ds-success)" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round" fill="none" opacity=".35"/></svg></div>
  <div style="flex:1;min-width:0;display:flex;flex-direction:column;align-items:center;">
    <div style="width:100%;flex:1;display:flex;align-items:center;justify-content:center;padding:.7em .6em;border-radius:8px;background:rgba(var(--ds-success-rgb),.08);border:1.5px solid rgba(var(--ds-success-rgb),.25);text-align:center;font-weight:800;color:var(--ds-success);">ASYNC COMPUTE ✓</div>
  </div>
</div>
<div style="font-size:.82em;opacity:.6;margin-top:-.2em;text-align:center;">Good candidates: SSAO alongside ROP-bound geometry, volumetrics during shadow rasterization, particle sim during UI.</div>

Try it yourself — move compute-eligible passes between queues and see how fence count and frame time change:

{{< interactive-async >}}

---

## ✂ Split Barriers

Async compute hides latency by overlapping work across *queues*. Split barriers achieve the same effect on a *single queue* — by spreading one resource transition across multiple passes instead of stalling on it.

A **regular barrier** does a cache flush, state change, and cache invalidate in one blocking command — the GPU finishes the source pass, stalls while the transition completes, then starts the next pass. Every microsecond of that stall is wasted.

A **split barrier** breaks the transition into two halves and spreads them apart:

<div style="margin:1.4em 0;font-size:.88em;">
  <div style="display:flex;align-items:stretch;gap:0;border-radius:10px;overflow:hidden;border:1.5px solid rgba(var(--ds-indigo-rgb),.15);">
    <div style="background:rgba(var(--ds-info-rgb),.08);padding:.8em 1em;border-right:3px solid var(--ds-info);min-width:130px;text-align:center;">
      <div style="font-weight:800;font-size:.95em;">Source pass</div>
      <div style="font-size:.78em;opacity:.6;margin-top:.2em;">writes texture</div>
    </div>
    <div style="background:rgba(var(--ds-info-rgb),.15);padding:.5em .8em;display:flex;align-items:center;min-width:50px;border-right:1px dashed rgba(var(--ds-indigo-rgb),.3);">
      <div style="text-align:center;width:100%;">
        <div style="font-size:.7em;font-weight:700;color:var(--ds-info);text-transform:uppercase;letter-spacing:.04em;">BEGIN</div>
        <div style="font-size:.68em;opacity:.5;">flush caches</div>
      </div>
    </div>
    <div style="flex:1;background:repeating-linear-gradient(90deg,rgba(var(--ds-indigo-rgb),.04) 0,rgba(var(--ds-indigo-rgb),.04) 50%,rgba(var(--ds-indigo-rgb),.08) 50%,rgba(var(--ds-indigo-rgb),.08) 100%);background-size:50% 100%;display:flex;align-items:stretch;min-width:200px;">
      <div style="flex:1;padding:.6em .7em;text-align:center;border-right:1px dashed rgba(var(--ds-indigo-rgb),.15);">
        <div style="font-weight:700;font-size:.85em;">Pass C</div>
        <div style="font-size:.72em;opacity:.5;">unrelated work</div>
      </div>
      <div style="flex:1;padding:.6em .7em;text-align:center;">
        <div style="font-weight:700;font-size:.85em;">Pass D</div>
        <div style="font-size:.72em;opacity:.5;">unrelated work</div>
      </div>
    </div>
    <div style="background:rgba(var(--ds-success-rgb),.15);padding:.5em .8em;display:flex;align-items:center;min-width:50px;border-left:1px dashed rgba(var(--ds-indigo-rgb),.3);">
      <div style="text-align:center;width:100%;">
        <div style="font-size:.7em;font-weight:700;color:var(--ds-success);text-transform:uppercase;letter-spacing:.04em;">END</div>
        <div style="font-size:.68em;opacity:.5;">invalidate</div>
      </div>
    </div>
    <div style="background:rgba(var(--ds-success-rgb),.08);padding:.8em 1em;border-left:3px solid var(--ds-success);min-width:130px;text-align:center;">
      <div style="font-weight:800;font-size:.95em;">Dest pass</div>
      <div style="font-size:.78em;opacity:.6;margin-top:.2em;">reads texture</div>
    </div>
  </div>
  <div style="display:flex;margin-top:.25em;">
    <div style="min-width:130px;"></div>
    <div style="min-width:50px;"></div>
    <div style="flex:1;min-width:200px;text-align:center;font-size:.78em;opacity:.6;">
      ↑ cache flush runs in background while these execute ↑
    </div>
    <div style="min-width:50px;"></div>
    <div style="min-width:130px;"></div>
  </div>
</div>

The passes between begin and end are the **overlap gap** — they execute while the cache flush happens in the background. The compiler places these automatically: begin immediately after the source pass, end immediately before the destination.

### 📏 How much gap is enough?

<div class="fg-grid-stagger" style="display:grid;grid-template-columns:repeat(4,1fr);gap:.6em;margin:1.2em 0;">
  <div class="fg-hoverable" style="border-radius:8px;border:1.5px solid rgba(var(--ds-danger-rgb),.2);background:rgba(var(--ds-danger-rgb),.03);padding:.7em .8em;text-align:center;">
    <div style="font-weight:800;font-size:1.3em;color:var(--ds-danger);">0</div>
    <div style="font-size:.8em;font-weight:600;margin:.25em 0;">passes</div>
    <div style="font-size:.78em;opacity:.7;">No gap — degenerates into a regular barrier with extra API cost</div>
  </div>
  <div class="fg-hoverable" style="border-radius:8px;border:1.5px solid rgba(234,179,8,.2);background:rgba(234,179,8,.03);padding:.7em .8em;text-align:center;">
    <div style="font-weight:800;font-size:1.3em;color:#eab308;">1</div>
    <div style="font-size:.8em;font-weight:600;margin:.25em 0;">pass</div>
    <div style="font-size:.78em;opacity:.7;">Marginal — might not cover the full flush latency</div>
  </div>
  <div class="fg-hoverable" style="border-radius:8px;border:1.5px solid rgba(var(--ds-success-rgb),.25);background:rgba(var(--ds-success-rgb),.03);padding:.7em .8em;text-align:center;">
    <div style="font-weight:800;font-size:1.3em;color:var(--ds-success);">2+</div>
    <div style="font-size:.8em;font-weight:600;margin:.25em 0;">passes</div>
    <div style="font-size:.78em;opacity:.7;">Cache flush fully hidden — measurable frame time reduction</div>
  </div>
  <div class="fg-hoverable" style="border-radius:8px;border:1.5px solid rgba(var(--ds-indigo-rgb),.2);background:rgba(var(--ds-indigo-rgb),.03);padding:.7em .8em;text-align:center;">
    <div style="font-weight:800;font-size:1.3em;color:var(--ds-indigo);">⚡</div>
    <div style="font-size:.8em;font-weight:600;margin:.25em 0;">cross-queue</div>
    <div style="font-size:.78em;opacity:.7;">Can't split across queues — use an async fence instead</div>
  </div>
</div>

---

## 🎛 Putting It All Together

You've now seen every piece the compiler works with — topological sorting, pass culling, barrier computation, async compute scheduling, memory aliasing, split barriers. In a simple 5-pass pipeline these feel manageable. In a production renderer? You're looking at **15–25 passes, 30+ resource edges, and dozens of implicit dependencies** — all inferred from `read()` and `write()` calls that no human can hold in their head at once.

<div class="fg-reveal" style="margin:1.2em 0;padding:.85em 1.1em;border-radius:10px;border:1.5px solid rgba(var(--ds-code-rgb),.2);background:linear-gradient(135deg,rgba(var(--ds-code-rgb),.05),transparent);font-size:.92em;line-height:1.65;">
<strong>This is the trade-off at the heart of every render graph.</strong> Dependencies become <em>implicit</em> — the graph infers ordering from data flow, which means you never declare "pass A must run before pass B." That's powerful: the compiler can reorder, cull, and parallelize freely. But it also means <strong>dependencies are hidden</strong>. Miss a <code>read()</code> call and the graph silently reorders two passes that shouldn't overlap. Add an assert and you'll catch the <em>symptom</em> — but not the missing edge that caused it.
</div>

Since the frame graph is a DAG, every dependency is explicitly encoded in the structure. That means you can build tools to **visualize** the entire pipeline — every pass, every resource edge, every implicit ordering decision — something that's impossible when barriers and ordering are scattered across hand-written render code.

The explorer below is a production-scale graph. Toggle each compiler feature on and off to see exactly what it contributes. Click any pass to inspect its dependencies — every edge was inferred from `read()` and `write()` calls, not hand-written.

{{< interactive-full-pipeline >}}

### 🔮 What's next

Async compute and split barriers are compiler features — they plug into the same DAG we built in Part II. But how do production engines actually ship all of this at scale? [Part IV — Production Engines](../frame-graph-production/) examines UE5's RDG and Frostbite's FrameGraph side by side, covering parallel command recording, legacy migration, and the engineering trade-offs that only matter at 700+ passes per frame.

---

<div style="margin:2em 0 0;padding:1em 1.2em;border-radius:10px;border:1px solid rgba(var(--ds-info-rgb),.2);background:rgba(var(--ds-info-rgb),.03);display:flex;justify-content:space-between;">
  <a href="../frame-graph-build-it/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    ← Previous: Part II — Build It
  </a>
  <a href="../frame-graph-production/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    Next: Part IV — Production Engines →
  </a>
</div>