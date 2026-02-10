---
title: "Frame Graph — MVP to Real Engines"
date: 2026-02-09
draft: true
description: "Article plan preview — Frame Graph implementation & usage, from MVP to production engines."
tags: ["rendering", "frame-graph", "gpu", "architecture"]
categories: ["analysis"]
series: ["Rendering Architecture"]
showTableOfContents: false
---

{{< article-nav >}}

<!-- This is a preview-only file generated from ARTICLE_PLAN.md. Not the final article. -->

# Part I — Theory

*What a render graph is, what problems it solves, and why every major engine uses one.*

---

## Why You Want One

<div style="display:grid;grid-template-columns:1fr 1fr;gap:.8em;margin:1.2em 0;">
  <div style="border-radius:10px;border:1.5px solid rgba(59,130,246,.3);padding:.8em 1em;background:rgba(59,130,246,.04);">
    <div style="font-weight:800;font-size:.95em;color:#3b82f6;margin-bottom:.3em;">🧩 Composability</div>
    <div style="font-size:.9em;line-height:1.6;">Add a pass, remove a pass, reorder passes. Nothing breaks. The graph recompiles.</div>
  </div>
  <div style="border-radius:10px;border:1.5px solid rgba(34,197,94,.3);padding:.8em 1em;background:rgba(34,197,94,.04);">
    <div style="font-weight:800;font-size:.95em;color:#22c55e;margin-bottom:.3em;">💾 Memory</div>
    <div style="font-size:.9em;line-height:1.6;">30–50% VRAM saved. The compiler alias-packs transient resources. No human does this reliably at 20+ passes.</div>
  </div>
  <div style="border-radius:10px;border:1.5px solid rgba(139,92,246,.3);padding:.8em 1em;background:rgba(139,92,246,.04);">
    <div style="font-weight:800;font-size:.95em;color:#8b5cf6;margin-bottom:.3em;">🔒 Barriers</div>
    <div style="font-size:.9em;line-height:1.6;">Automatic. Correct. The graph handles the majority of cases — you rarely write one by hand.</div>
  </div>
  <div style="border-radius:10px;border:1.5px solid rgba(245,158,11,.3);padding:.8em 1em;background:rgba(245,158,11,.04);">
    <div style="font-weight:800;font-size:.95em;color:#f59e0b;margin-bottom:.3em;">🔍 Inspectability</div>
    <div style="font-size:.9em;line-height:1.6;">The frame is data. Export it, diff it, visualize it.</div>
  </div>
</div>

Frostbite introduced the frame graph at GDC 2017. UE5 ships it as **RDG**. Unity has its own in SRP. Every major renderer now uses one — here's why, and how to build your own.

<div class="wyg-card" style="
  margin: 1.8em 0;
  border-radius: 10px;
  overflow: hidden;
  border: 1.5px solid var(--color-neutral-300, #d4d4d4);
  background: var(--color-neutral-50, #fafafa);
">
  <div style="
    padding: .65em 1.1em;
    font-weight: 800;
    font-size: 1.05em;
    letter-spacing: .02em;
    background: var(--color-primary-100, #e0e7ff);
    color: var(--color-neutral-800, #1e1e1e);
    border-bottom: 1.5px solid var(--color-neutral-300, #d4d4d4);
  ">🎯 What you'll get</div>
  <div style="display: grid; grid-template-columns: auto 1fr; gap: 0;">
    <div style="padding: .7em 1em; font-weight: 700; font-size: .95em; border-bottom: 1px solid var(--color-neutral-200, #e5e5e5); display:flex; align-items:center; gap:.45em;">
      <span style="font-size:1.15em;">🔨</span> Build
    </div>
    <div style="padding: .7em 1em; border-bottom: 1px solid var(--color-neutral-200, #e5e5e5); border-left: 1px solid var(--color-neutral-200, #e5e5e5);">
      A working frame graph in C++, from blank file to functional prototype
    </div>
    <div style="padding: .7em 1em; font-weight: 700; font-size: .95em; border-bottom: 1px solid var(--color-neutral-200, #e5e5e5); display:flex; align-items:center; gap:.45em;">
      <span style="font-size:1.15em;">🗺️</span> Map
    </div>
    <div style="padding: .7em 1em; border-bottom: 1px solid var(--color-neutral-200, #e5e5e5); border-left: 1px solid var(--color-neutral-200, #e5e5e5);">
      Every piece to UE5's RDG — read the source with confidence
    </div>
    <div style="padding: .7em 1em; font-weight: 700; font-size: .95em; display:flex; align-items:center; gap:.45em;">
      <span style="font-size:1.15em;">🚀</span> Beyond MVP
    </div>
    <div style="padding: .7em 1em; border-left: 1px solid var(--color-neutral-200, #e5e5e5);">
      Aliasing, pass merging, async compute, split barriers
    </div>
  </div>
</div>

If you've watched VRAM spike from non-overlapping textures or chased a black screen after reordering a pass — this is for you.

---

## The Problem

<div style="position:relative;margin:1.4em 0;padding-left:2.2em;border-left:3px solid var(--color-neutral-300,#d4d4d4);">

  <div style="margin-bottom:1.6em;">
    <div style="position:absolute;left:-0.8em;width:1.4em;height:1.4em;border-radius:50%;background:#22c55e;display:flex;align-items:center;justify-content:center;font-size:.7em;color:#fff;font-weight:700;">1</div>
    <div style="font-weight:800;font-size:1.05em;color:#22c55e;margin-bottom:.3em;">Month 1 — 3 passes, everything's fine</div>
    <div style="font-size:.92em;line-height:1.6;">
      Depth prepass → GBuffer → lighting. Two barriers, hand-placed. Two textures, both allocated at init. Code is clean, readable, correct.
    </div>
    <div style="margin-top:.4em;padding:.4em .8em;border-radius:6px;background:rgba(34,197,94,.06);font-size:.88em;font-style:italic;border-left:3px solid #22c55e;">
      At this scale, manual management works. You know every resource by name.
    </div>
  </div>

  <div style="margin-bottom:1.6em;">
    <div style="position:absolute;left:-0.8em;width:1.4em;height:1.4em;border-radius:50%;background:#f59e0b;display:flex;align-items:center;justify-content:center;font-size:.7em;color:#fff;font-weight:700;">6</div>
    <div style="font-weight:800;font-size:1.05em;color:#f59e0b;margin-bottom:.3em;">Month 6 — 12 passes, cracks appear</div>
    <div style="font-size:.92em;line-height:1.6;">
      Same renderer, now with SSAO, SSR, bloom, TAA, shadow cascades. Three things going wrong simultaneously:
    </div>
    <div style="margin-top:.5em;display:grid;gap:.4em;">
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(245,158,11,.2);background:rgba(245,158,11,.04);font-size:.88em;line-height:1.5;">
        <strong>Invisible dependencies</strong> — someone adds SSAO but doesn't realize GBuffer needs an updated barrier. Visual artifacts on fresh build.
      </div>
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(245,158,11,.2);background:rgba(245,158,11,.04);font-size:.88em;line-height:1.5;">
        <strong>Wasted memory</strong> — SSAO and bloom textures never overlap, but aliasing them means auditing every pass that might touch them. Nobody does it.
      </div>
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(245,158,11,.2);background:rgba(245,158,11,.04);font-size:.88em;line-height:1.5;">
        <strong>Silent reordering</strong> — two branches touch the render loop. Git merges cleanly, but the shadow pass ends up after lighting. Subtly wrong output ships unnoticed.
      </div>
    </div>
    <div style="margin-top:.5em;padding:.4em .8em;border-radius:6px;background:rgba(245,158,11,.06);font-size:.88em;font-style:italic;border-left:3px solid #f59e0b;">
      No single change broke it. The accumulation broke it.
    </div>
  </div>

  <div>
    <div style="position:absolute;left:-0.8em;width:1.4em;height:1.4em;border-radius:50%;background:#ef4444;display:flex;align-items:center;justify-content:center;font-size:.65em;color:#fff;font-weight:700;">18</div>
    <div style="font-weight:800;font-size:1.05em;color:#ef4444;margin-bottom:.3em;">Month 18 — 25 passes, nobody touches it</div>
    <div style="font-size:.92em;line-height:1.6;margin-bottom:.5em;">The renderer works, but:</div>
    <div style="display:grid;gap:.4em;">
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(239,68,68,.2);background:rgba(239,68,68,.04);font-size:.88em;line-height:1.5;">
        <strong>900 MB VRAM.</strong> Profiling shows 400 MB is aliasable — but the lifetime analysis would take a week and break the next time anyone adds a pass.
      </div>
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(239,68,68,.2);background:rgba(239,68,68,.04);font-size:.88em;line-height:1.5;">
        <strong>47 barrier calls.</strong> Three are redundant, two are missing, one is in the wrong queue. Nobody knows which.
      </div>
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(239,68,68,.2);background:rgba(239,68,68,.04);font-size:.88em;line-height:1.5;">
        <strong>2 days to add a new pass.</strong> 30 minutes for the shader, the rest to figure out where to slot it and what barriers it needs.
      </div>
    </div>
    <div style="margin-top:.5em;padding:.4em .8em;border-radius:6px;background:rgba(239,68,68,.06);font-size:.88em;font-style:italic;border-left:3px solid #ef4444;">
      The renderer isn't wrong. It's <em>fragile</em>. Every change is a risk.
    </div>
  </div>

</div>

<div class="diagram-bars" style="grid-template-columns:110px 1fr 1fr 1fr;gap:0.3em 0.6em;font-size:.8em">
  <div class="db-label"></div>
  <div style="font-weight:700;text-align:center">Month 1</div>
  <div style="font-weight:700;text-align:center">Month 6</div>
  <div style="font-weight:700;text-align:center">Month 18</div>
  <div class="db-label">Passes</div>
  <div><div class="db-bar" style="width:12%;min-width:18px"></div><span class="db-val">3</span></div>
  <div><div class="db-bar db-warn" style="width:48%"></div><span class="db-val">12</span></div>
  <div><div class="db-bar db-danger" style="width:100%"></div><span class="db-val">25</span></div>
  <div class="db-label">Barriers</div>
  <div><div class="db-bar" style="width:4%;min-width:18px"></div><span class="db-val">2</span></div>
  <div><div class="db-bar db-warn" style="width:38%"></div><span class="db-val">18</span></div>
  <div><div class="db-bar db-danger" style="width:100%"></div><span class="db-val">47</span></div>
  <div class="db-label">VRAM</div>
  <div><div class="db-bar" style="width:4%;min-width:18px"></div><span class="db-val">~40 MB</span></div>
  <div><div class="db-bar db-warn" style="width:42%"></div><span class="db-val">380 MB</span></div>
  <div><div class="db-bar db-danger" style="width:100%"></div><span class="db-val">900 MB</span></div>
  <div class="db-label">Aliasable</div>
  <div><div class="db-bar" style="width:0%;min-width:3px;opacity:.3"></div><span class="db-val">0</span></div>
  <div><div class="db-bar db-warn" style="width:20%"></div><span class="db-val">~80 MB</span></div>
  <div><div class="db-bar db-danger" style="width:100%"></div><span class="db-val">400 MB</span></div>
  <div class="db-label">Status</div>
  <div style="color:#22c55e;font-weight:700">✓ manageable</div>
  <div style="color:#f59e0b;font-weight:700">⚠ fragile</div>
  <div style="color:#ef4444;font-weight:700">✗ untouchable</div>
</div>

The pattern is always the same: manual resource management works at small scale and fails at compound scale. Not because engineers are sloppy — because *no human tracks 25 lifetimes and 47 transitions in their head every sprint*. You need a system that sees the whole frame at once.

---

## The Core Idea

<div style="display:grid;grid-template-columns:1fr 1fr;gap:1.2em;margin:1.4em 0;">
  <div style="border-radius:10px;border:1.5px solid var(--color-neutral-300,#d4d4d4);padding:1em;background:var(--color-neutral-50,#fafafa);">
    <div style="font-weight:800;font-size:.95em;margin-bottom:.5em;">📦 What it is</div>
    <ul style="margin:0;padding-left:1.2em;line-height:1.7;font-size:.92em;">
      <li><strong>Directed Acyclic Graph (DAG)</strong> of render passes</li>
      <li><strong>Edges</strong> = resource dependencies (read/write)</li>
      <li>Each frame: declare → compile → execute</li>
    </ul>
  </div>
  <div style="border-radius:10px;border:1.5px solid var(--color-neutral-300,#d4d4d4);padding:1em;background:var(--color-neutral-50,#fafafa);">
    <div style="font-weight:800;font-size:.95em;margin-bottom:.5em;">💡 Analogy</div>
    <div style="font-size:.92em;line-height:1.7;">Think of it like a build system for GPU work. Each pass declares its inputs and outputs. The compiler resolves the dependency order, finds where resources can be reused, and inserts synchronization — the same way <code>make</code> figures out which targets to rebuild and in what order.</div>
  </div>
</div>

<div class="diagram-flow" style="justify-content:center">
  <div class="df-step df-primary">Depth<br>Prepass<span class="df-sub">depth tex</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-primary">GBuf<br>Pass<span class="df-sub">GBuffer</span></div>
  <div class="df-arrow"></div>
  <div class="df-step" style="display:flex;flex-direction:column;gap:.3em;padding:.5em .8em">
    <div style="display:flex;gap:.5em;align-items:center">
      <div class="df-step df-primary" style="border:none;padding:.3em .6em;font-size:.9em">SSAO<span class="df-sub">SSAO tex</span></div>
    </div>
    <div style="opacity:.5;font-size:.8em">↕</div>
    <div class="df-step df-primary" style="border:none;padding:.3em .6em;font-size:.9em">Lighting<span class="df-sub">HDR</span></div>
  </div>
  <div class="df-arrow"></div>
  <div class="df-step df-success">Tonemap<span class="df-sub">→ Present</span></div>
</div>
<div style="text-align:center;font-size:.78em;opacity:.6;margin-top:-.5em">Nodes = passes &nbsp;&nbsp; Edges = resource dependencies &nbsp;&nbsp; Arrow = data flow (write → read)</div>

Resources in the graph come in two kinds:

<div style="display:grid;grid-template-columns:1fr 1fr;gap:1em;margin:1.2em 0;">
  <div style="border-radius:10px;border:1.5px solid rgba(59,130,246,.3);overflow:hidden;">
    <div style="padding:.6em .9em;font-weight:800;font-size:.95em;background:rgba(59,130,246,.08);border-bottom:1px solid rgba(59,130,246,.15);color:#3b82f6;">⚡ Transient</div>
    <div style="padding:.7em .9em;font-size:.88em;line-height:1.7;">
      <strong>Lifetime:</strong> single frame<br>
      <strong>Declared as:</strong> description (size, format)<br>
      <strong>GPU memory:</strong> allocated at compile time — <em>virtual until then</em><br>
      <strong>Aliasable:</strong> <span style="color:#22c55e;font-weight:700;">Yes</span> — non-overlapping lifetimes share physical memory<br>
      <strong>Examples:</strong> GBuffer MRTs, SSAO scratch, bloom scratch
    </div>
  </div>
  <div style="border-radius:10px;border:1.5px solid rgba(139,92,246,.3);overflow:hidden;">
    <div style="padding:.6em .9em;font-weight:800;font-size:.95em;background:rgba(139,92,246,.08);border-bottom:1px solid rgba(139,92,246,.15);color:#8b5cf6;">📌 Imported</div>
    <div style="padding:.7em .9em;font-size:.88em;line-height:1.7;">
      <strong>Lifetime:</strong> across frames<br>
      <strong>Declared as:</strong> existing GPU handle<br>
      <strong>GPU memory:</strong> already allocated externally<br>
      <strong>Aliasable:</strong> <span style="color:#ef4444;font-weight:700;">No</span> — lifetime extends beyond the frame<br>
      <strong>Examples:</strong> backbuffer, TAA history, shadow atlas, blue noise LUT
    </div>
  </div>
</div>

This split is what makes aliasing possible. Transient resources are just descriptions until the compiler maps them to real memory — so two that never overlap can land on the same allocation. Imported resources are already owned by something else; the graph tracks their barriers but leaves their memory alone.

<div class="diagram-flow" style="justify-content:center">
  <div class="df-step">DECLARE<span class="df-sub">passes & deps</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-primary" style="min-width:180px">COMPILE<span class="df-sub">schedule order · compute aliases · insert barriers</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-success">EXECUTE<span class="df-sub">record cmds</span></div>
</div>

Let's walk through each stage.

---

## The Declare Step

<div class="diagram-box">
  <div class="db-title">📋 DECLARE — building the graph</div>
  <div class="db-body">
    <div class="diagram-pipeline">
      <div class="dp-stage">
        <div class="dp-title">ADD PASSES</div>
        <ul><li>register setup + execute lambdas</li><li>each pass gets a unique ID</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">DECLARE RESOURCES</div>
        <ul><li>request by description (size, format)</li><li>virtual handle — no GPU memory yet</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">WIRE DEPENDENCIES</div>
        <ul><li><code>read(handle)</code> / <code>write(handle)</code></li><li>edges form the DAG</li></ul>
      </div>
    </div>
    <div style="text-align:center;font-size:.82em;opacity:.6;margin-top:.3em">CPU only — no GPU work happens here</div>
  </div>
</div>

- **Add passes** — each `addPass(setup, execute)` call registers a node in the graph. The setup lambda runs *immediately* to declare what resources it touches.
- **Declare resources** — `createTexture({1920, 1080, RGBA8})` returns a handle, not a GPU allocation. The resource is virtual.
- **Wire dependencies** — calling `read(handle)` or `write(handle)` inside setup creates edges. This is what the compiler uses to determine order, barriers, and aliasability.

Nothing executes on the GPU during this phase. You're describing *intent* — the compiler turns that intent into a plan.

---

## The Compile Step

This is where the heavy lifting happens — the compiler reads the declared DAG and produces an optimized execution plan.

<div class="diagram-box">
  <div class="db-title">🔍 COMPILE — inside the black box</div>
  <div class="db-body">
    <div class="diagram-pipeline">
      <div class="dp-stage">
        <div class="dp-title">SCHEDULE</div>
        <ul><li>topo-sort DAG (Kahn's alg)</li><li>detect cycles</li><li>→ pass order</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">ALLOCATE</div>
        <ul><li>map virtual resources to physical memory</li><li>alias overlaps</li><li>→ memory map</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">SYNCHRONIZE</div>
        <ul><li>insert barriers between passes that hand off resources</li><li>→ barrier list</li></ul>
      </div>
    </div>
    <div style="text-align:center;font-size:.82em;opacity:.6;margin-top:.3em">Schedule + Sync: O(V + E) &nbsp;&nbsp; Allocate: O(R log R)</div>
  </div>
</div>

- **Schedule** — topological sort (Kahn's algorithm). If a cycle exists, the compiler catches it here.
- **Allocate** — two transient resources with non-overlapping lifetimes share the same physical block. Sort by first use, then greedy scan — O(R log R) where R = transient resource count.
- **Synchronize** — the compiler knows who wrote what and who reads it next — minimal barriers, no over-sync.

<div style="margin:1.2em 0;padding:.7em 1em;border-radius:8px;background:linear-gradient(135deg,rgba(59,130,246,.06),rgba(139,92,246,.06));border:1px solid rgba(59,130,246,.2);font-size:.9em;">
⚡ All three are <strong>near-linear</strong>. For a typical 25-pass frame the entire compile takes <strong>microseconds</strong>. Exact cost breakdown in <a href="#a-real-frame">A Real Frame</a>.
</div>

<div style="margin:1.4em 0;padding:1em 1.2em;border-left:4px solid #8b5cf6;border-radius:0 8px 8px 0;background:rgba(139,92,246,.05);font-size:.95em;line-height:1.6;">
<strong style="color:#8b5cf6;font-size:1.05em;">🧠 The key insight</strong><br>
The renderer doesn't <em>run</em> passes — it <em>submits a plan</em>. The graph compiler sees every resource lifetime in the frame at once, so it can pack transient resources into the minimum memory footprint, place every barrier automatically, and cull passes whose outputs nobody reads. This is the <strong>inversion of control</strong> that makes everything else possible.
</div>

---

## The Execute Step

<div class="diagram-box">
  <div class="db-title">▶️ EXECUTE — recording GPU work</div>
  <div class="db-body">
    <div class="diagram-pipeline">
      <div class="dp-stage">
        <div class="dp-title">ALLOCATE MEMORY</div>
        <ul><li>create/reuse physical resources</li><li>apply alias mappings from compile</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">RUN PASSES</div>
        <ul><li>iterate compiled order</li><li>insert barriers before each pass</li><li>call execute lambda</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">CLEANUP</div>
        <ul><li>free transient resources</li><li>reset frame allocator</li></ul>
      </div>
    </div>
    <div style="text-align:center;font-size:.82em;opacity:.6;margin-top:.3em">GPU command recording — the only phase that touches the API</div>
  </div>
</div>

- **Allocate memory** — the compiled alias map tells the executor which resources share physical memory. Create only what's needed.
- **Run passes** — walk the passes in compiled order. Before each pass, insert the barriers the compiler computed. Then call the execute lambda — this is where the user records draw calls, dispatches, copies.
- **Cleanup** — transient resources are freed (or returned to a pool). The linear allocator resets. The frame is done.

The execute phase is intentionally simple — all the intelligence lives in the compile step. Each execute lambda sees a fully resolved environment: barriers placed, memory allocated, resources ready.

---

**How often does it rebuild?** Three strategies, each a valid tradeoff:

<div style="display:grid;grid-template-columns:repeat(3,1fr);gap:1em;margin:1.2em 0;">
  <div style="border-radius:10px;border:1.5px solid #22c55e;overflow:hidden;">
    <div style="padding:.5em .8em;background:rgba(34,197,94,.1);font-weight:800;font-size:.95em;border-bottom:1px solid rgba(34,197,94,.2);">
      🔄 Dynamic
    </div>
    <div style="padding:.7em .8em;font-size:.88em;line-height:1.6;">
      Rebuild every frame.<br>
      <strong>Cost:</strong> microseconds<br>
      <strong>Flex:</strong> full — passes appear/disappear freely<br>
      <span style="opacity:.6;font-size:.9em;">Used by: Frostbite</span>
    </div>
  </div>
  <div style="border-radius:10px;border:1.5px solid #3b82f6;overflow:hidden;">
    <div style="padding:.5em .8em;background:rgba(59,130,246,.1);font-weight:800;font-size:.95em;border-bottom:1px solid rgba(59,130,246,.2);">
      ⚡ Hybrid
    </div>
    <div style="padding:.7em .8em;font-size:.88em;line-height:1.6;">
      Cache compiled result, invalidate on change.<br>
      <strong>Cost:</strong> near-zero on hit<br>
      <strong>Flex:</strong> full + bookkeeping<br>
      <span style="opacity:.6;font-size:.9em;">Used by: UE5</span>
    </div>
  </div>
  <div style="border-radius:10px;border:1.5px solid var(--color-neutral-400,#9ca3af);overflow:hidden;">
    <div style="padding:.5em .8em;background:rgba(156,163,175,.1);font-weight:800;font-size:.95em;border-bottom:1px solid rgba(156,163,175,.2);">
      🔒 Static
    </div>
    <div style="padding:.7em .8em;font-size:.88em;line-height:1.6;">
      Compile once at init, replay forever.<br>
      <strong>Cost:</strong> zero<br>
      <strong>Flex:</strong> none — fixed pipeline<br>
      <span style="opacity:.6;font-size:.9em;">Rare in practice</span>
    </div>
  </div>
</div>

Most engines use **dynamic** or **hybrid**. The compile is so cheap that caching buys little — but some engines do it anyway to skip redundant barrier recalculation.

---

## The Payoff

<div style="margin:1.2em 0;display:grid;grid-template-columns:1fr 1fr;gap:0;border-radius:10px;overflow:hidden;border:1.5px solid var(--color-neutral-300,#d4d4d4);">
  <div style="padding:.6em 1em;font-weight:800;font-size:.95em;background:rgba(239,68,68,.08);border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-right:1px solid var(--color-neutral-200,#e5e5e5);color:#ef4444;">❌ Without Graph</div>
  <div style="padding:.6em 1em;font-weight:800;font-size:.95em;background:rgba(34,197,94,.08);border-bottom:1px solid var(--color-neutral-200,#e5e5e5);color:#22c55e;">✅ With Graph</div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-right:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Memory aliasing</strong><br><span style="opacity:.75">Opt-in, fragile, rarely done</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Memory aliasing</strong><br>Automatic — compiler sees all lifetimes. <strong style="color:#22c55e;">30–50% VRAM saved.</strong>
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-right:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Lifetimes</strong><br><span style="opacity:.75">Manual create/destroy, leaked or over-retained</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Lifetimes</strong><br>Scoped to first..last use. Zero waste.
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-right:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Barriers</strong><br><span style="opacity:.75">Manual, per-pass</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Barriers</strong><br>Automatic from declared read/write
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-right:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Pass reordering</strong><br><span style="opacity:.75">Breaks silently</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Pass reordering</strong><br>Safe — compiler respects dependencies
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-right:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Pass culling</strong><br><span style="opacity:.75">Manual ifdef / flag checks</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Pass culling</strong><br>Automatic — unused outputs = dead pass
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-right:1px solid var(--color-neutral-200,#e5e5e5);">
    <strong>Async compute</strong><br><span style="opacity:.75">Manual queue sync</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;">
    <strong>Async compute</strong><br>Compiler schedules across queues
  </div>
</div>

<div style="margin:1.2em 0;padding:.8em 1em;border-radius:8px;background:linear-gradient(135deg,rgba(34,197,94,.06),rgba(59,130,246,.06));border:1px solid rgba(34,197,94,.2);font-size:.92em;line-height:1.6;">
🏭 <strong>Not theoretical.</strong> Frostbite reported <strong>50% VRAM reduction</strong> from aliasing at GDC 2017. UE5's RDG ships the same optimization today — every <code>FRDGTexture</code> marked as transient goes through the same aliasing pipeline we're about to build.<br>
<span style="opacity:.7;font-size:.9em;">The MVP gives you automatic lifetimes + aliasing by Section 8, automatic barriers by Section 7. After that, we map everything to UE5's RDG.</span>
</div>

---

## API Design

We start from the API you *want* to write — a minimal `FrameGraph` setup that declares a depth prepass, GBuffer pass, and lighting pass in ~20 lines of C++.

<div style="margin:1.4em 0;border-radius:10px;border:1.5px solid var(--color-neutral-300,#d4d4d4);overflow:hidden;background:var(--color-neutral-50,#fafafa);">
  <div style="padding:.65em 1em;font-weight:800;font-size:1em;background:var(--color-primary-100,#e0e7ff);border-bottom:1.5px solid var(--color-neutral-300,#d4d4d4);color:var(--color-neutral-800,#1e1e1e);">
    🔑 Key design choices
  </div>
  <div style="display:grid;grid-template-columns:auto 1fr;gap:0;">
    <div style="padding:.7em 1em;font-weight:700;font-size:.92em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);display:flex;align-items:center;gap:.4em;">
      <span style="font-size:1.1em;">λ²</span> Two lambdas
    </div>
    <div style="padding:.7em 1em;font-size:.9em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-left:1px solid var(--color-neutral-200,#e5e5e5);line-height:1.5;">
      <strong style="color:#3b82f6;">Setup</strong> runs at declaration time — declares "I read A, I write B." No GPU work.<br>
      <strong style="color:#22c55e;">Execute</strong> runs later — records actual GPU commands. Barriers & memory already resolved.
    </div>
    <div style="padding:.7em 1em;font-weight:700;font-size:.92em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);display:flex;align-items:center;gap:.4em;">
      <span style="font-size:1.1em;">📐</span> Virtual resources
    </div>
    <div style="padding:.7em 1em;font-size:.9em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-left:1px solid var(--color-neutral-200,#e5e5e5);line-height:1.5;">
      Requested by description (<code>{1920, 1080, RGBA8}</code>), not by GPU handle. Virtual until the compiler maps them.
    </div>
    <div style="padding:.7em 1em;font-weight:700;font-size:.92em;display:flex;align-items:center;gap:.4em;">
      <span style="font-size:1.1em;">♻️</span> Owned lifetimes
    </div>
    <div style="padding:.7em 1em;font-size:.9em;border-left:1px solid var(--color-neutral-200,#e5e5e5);line-height:1.5;">
      The graph owns transient lifetimes — the user never calls create/destroy.
    </div>
  </div>
</div>

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

If you've seen UE5 code, this should look familiar:

<div style="margin:1.2em 0;border-radius:10px;overflow:hidden;border:1.5px solid var(--color-neutral-300,#d4d4d4);">
  <div style="display:grid;grid-template-columns:1fr 1fr;gap:0;">
    <div style="padding:.55em .8em;font-weight:800;font-size:.92em;background:rgba(59,130,246,.08);border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-right:1px solid var(--color-neutral-200,#e5e5e5);text-align:center;">🛠️ Our API</div>
    <div style="padding:.55em .8em;font-weight:800;font-size:.92em;background:rgba(139,92,246,.08);border-bottom:1px solid var(--color-neutral-200,#e5e5e5);text-align:center;">🎮 UE5 Equivalent</div>
    <div style="padding:.5em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-right:1px solid var(--color-neutral-200,#e5e5e5);font-family:ui-monospace,monospace;">addPass(setup, execute)</div>
    <div style="padding:.5em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);font-family:ui-monospace,monospace;">FRDGBuilder::AddPass</div>
    <div style="padding:.5em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-right:1px solid var(--color-neutral-200,#e5e5e5);font-family:ui-monospace,monospace;">ResourceHandle</div>
    <div style="padding:.5em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);font-family:ui-monospace,monospace;">FRDGTextureRef / FRDGBufferRef</div>
    <div style="padding:.5em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);border-right:1px solid var(--color-neutral-200,#e5e5e5);font-family:ui-monospace,monospace;">setup lambda</div>
    <div style="padding:.5em .8em;font-size:.88em;border-bottom:1px solid var(--color-neutral-200,#e5e5e5);font-family:ui-monospace,monospace;">BEGIN_SHADER_PARAMETER_STRUCT</div>
    <div style="padding:.5em .8em;font-size:.88em;border-right:1px solid var(--color-neutral-200,#e5e5e5);font-family:ui-monospace,monospace;">execute lambda</div>
    <div style="padding:.5em .8em;font-size:.88em;font-family:ui-monospace,monospace;">Execute lambda <span style="font-family:inherit;opacity:.6">(same concept)</span></div>
  </div>
</div>

<div style="margin:1em 0;padding:.7em 1em;border-radius:8px;border-left:4px solid #f59e0b;background:rgba(245,158,11,.05);font-size:.9em;line-height:1.6;">
⚠️ <strong>UE5's macro tradeoff:</strong> The <code>BEGIN_SHADER_PARAMETER_STRUCT</code> approach is opaque, hard to debug, and impossible to compose dynamically. Our explicit two-lambda API is simpler and more flexible — UE5 traded that flexibility for compile-time validation and reflection.
</div>

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

**What it proves:** The lambda-based pass declaration pattern works. You can already compose passes without manual barrier calls (even though barriers are no-ops here).

**What it lacks:** This version executes passes in declaration order and creates every resource upfront. It's correct but wasteful. Version 2 adds the graph.

---

## MVP v2 — Dependencies & Barriers

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

<div class="diagram-zoo">
  <div class="dz-title">Barrier zoo — the transitions a real frame actually needs</div>
  <div class="dz-item">
    <span class="dz-num">1</span> <span class="dz-name">Render target → Shader read</span> (most common)<br>
    GBuffer pass writes albedo → Lighting pass samples it
    <div class="dz-apis">Vulkan: COLOR_ATTACHMENT_OUTPUT → FRAGMENT_SHADER<br>D3D12: RENDER_TARGET → PIXEL_SHADER_RESOURCE</div>
  </div>
  <div class="dz-item">
    <span class="dz-num">2</span> <span class="dz-name">Depth write → Depth read</span> (shadow sampling)<br>
    Shadow pass writes depth → Lighting reads as texture
    <div class="dz-apis">Vulkan: LATE_FRAGMENT_TESTS → FRAGMENT_SHADER<br>D3D12: DEPTH_WRITE → PIXEL_SHADER_RESOURCE<br>Layout: DEPTH_ATTACHMENT → SHADER_READ_ONLY</div>
  </div>
  <div class="dz-item">
    <span class="dz-num">3</span> <span class="dz-name">Compute UAV write → Compute UAV read</span> (ping-pong)<br>
    Bloom downsample writes mip N → reads mip N to write mip N+1
    <div class="dz-apis">Vulkan: COMPUTE_SHADER (WRITE) → COMPUTE_SHADER (READ)<br>D3D12: UAV barrier (same resource, same state — still needed to flush compute caches!)</div>
  </div>
  <div class="dz-item">
    <span class="dz-num">4</span> <span class="dz-name">Shader read → Render target</span> (reuse after sampling)<br>
    Lighting sampled HDR buffer → Tonemap now writes to it
    <div class="dz-apis">Vulkan: FRAGMENT_SHADER → COLOR_ATTACHMENT_OUTPUT<br>Layout: SHADER_READ_ONLY → COLOR_ATTACHMENT_OPTIMAL</div>
  </div>
  <div class="dz-item">
    <span class="dz-num">5</span> <span class="dz-name">Render target → Present</span> (every frame, easy to forget)<br>
    Final composite → swapchain present
    <div class="dz-apis">Vulkan: COLOR_ATTACHMENT_OUTPUT → BOTTOM_OF_PIPE<br>D3D12: RENDER_TARGET → PRESENT</div>
  </div>
  <div class="dz-item">
    <span class="dz-num">6</span> <span class="dz-name">Aliasing barrier</span> (two resources, same memory)<br>
    GBuffer dies → HDR lighting reuses same physical block
    <div class="dz-apis">D3D12: explicit D3D12_RESOURCE_BARRIER_TYPE_ALIASING<br>Vulkan: handled via image layout UNDEFINED (discard)</div>
  </div>
</div>

<div class="diagram-card">
  <div class="dc-title">Note on D3D12 Enhanced Barriers</div>
  The legacy <code>D3D12_RESOURCE_BARRIER</code> API shown above uses per-resource state tracking. The newer Enhanced Barriers API (Agility SDK 1.7+) replaces this with per-subresource <strong>access-based</strong> barriers (<code>D3D12_BARRIER_GROUP</code>, <code>D3D12_TEXTURE_BARRIER</code>). Enhanced Barriers are more expressive — they support simultaneous read access from multiple stages, eliminate promotion/decay rules, and allow independent subresource transitions. If you're targeting D3D12 in 2026, prefer Enhanced Barriers for new code.
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

**See culling in action:** Compare these two variants — identical pipeline, but in variant B, Lighting doesn't read SSAO's output. The graph automatically culls the dead pass:

{{< compile-compare fileA="example_v2_ssao_alive.cpp" fileB="example_v2_ssao_dead.cpp" labelA="SSAO Connected (alive)" labelB="SSAO Disconnected (culled)" deps="frame_graph_v2.h" >}}

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

- **D3D12:** Create a `ID3D12Heap` (e.g., 64 MB device-local). Call `CreatePlacedResource` to bind each virtual resource at an offset within the heap. Two resources at different offsets (or the same offset with non-overlapping lifetimes) share the heap.
- **Vulkan:** Allocate a `VkDeviceMemory` block. Create each `VkImage`/`VkBuffer` normally, then call `vkBindImageMemory`/`vkBindBufferMemory` pointing into the same `VkDeviceMemory` at different (or overlapping) offsets.

Without placed resources, each `CreateCommittedResource` (D3D12) or dedicated allocation (Vulkan) gets its own memory — aliasing is impossible. This is why the graph's allocator works with heaps, not individual allocations.

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

**What it proves:** The full value prop — automatic memory aliasing *and* automatic barriers. The MVP is now feature-equivalent to Frostbite's 2017 GDC demo (minus async compute).

In UE5, this is handled by the transient resource allocator. Any `FRDGTexture` created through `FRDGBuilder::CreateTexture` (as opposed to `RegisterExternalTexture`) is transient and eligible for aliasing. The RDG compiler runs the same lifetime analysis and free-list scan we just built.

A limitation worth noting: UE5 only aliases *transient* resources. Imported resources — even when their lifetimes are fully known within the frame — are never aliased. Frostbite's original design was more aggressive here, aliasing across a broader set of resources by tracking GPU-timeline lifetimes rather than just graph-declared lifetimes. If your renderer has large imported resources with predictable per-frame usage patterns, UE5's approach leaves VRAM on the table.

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

Two kinds of resources in play:
- **Transient:** GBuffer MRTs, SSAO scratch, HDR lighting buffer, bloom scratch — created and destroyed within this frame. Aliased by the graph.
- **Imported:** Backbuffer (acquired from swapchain, presented at end), TAA history (read from last frame, written this frame for next frame), shadow atlas (persistent, updated incrementally). The graph tracks their barriers but doesn't own their memory.

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

<div class="diagram-phases">
  <div class="dph-col" style="border-color:#3b82f6">
    <div class="dph-title" style="color:#3b82f6">Render thread (setup)</div>
    <div class="dph-body">
      <code>FRDGBuilder::AddPass(...)</code><br>
      <code>FRDGBuilder::AddPass(...)</code><br>
      <code>FRDGBuilder::AddPass(...)</code><br>
      <code>CreateTexture(...)</code><br>
      <code>RegisterExternalTexture(...)</code><br>
      <span style="opacity:.6">↓ accumulates full DAG<br>(passes, resources, edges)</span>
    </div>
  </div>
  <div style="display:flex;align-items:center;font-size:1.6em;color:#3b82f6;font-weight:700">→</div>
  <div class="dph-col" style="border-color:#22c55e">
    <div class="dph-title" style="color:#22c55e">Render thread (execute)</div>
    <div class="dph-body">
      <code>FRDGBuilder::Execute()</code><br>
      ├ compile<br>
      ├ allocate<br>
      ├ barriers<br>
      └ record cmds
    </div>
  </div>
</div>

**Pass declaration.** Each `AddPass` takes a parameter struct + execute lambda. The struct *is* the setup phase:

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

<div class="diagram-ftable">
<table>
  <tr><th>Feature</th><th>Our MVP</th><th>UE5 RDG</th></tr>
  <tr><td><strong>Transient alloc</strong></td><td>free-list scan per frame</td><td>pooled allocator amortized across frames</td></tr>
  <tr><td><strong>Barriers</strong></td><td>one-at-a-time</td><td>batched + split begin/end</td></tr>
  <tr><td><strong>Pass culling</strong></td><td>backward walk from output</td><td>refcount-based + skip allocation</td></tr>
  <tr><td><strong>Cmd recording</strong></td><td>single thread</td><td>parallel FRHICmdList</td></tr>
  <tr><td><strong>Rebuild</strong></td><td>dynamic</td><td>hybrid (cached)</td></tr>
</table>
</div>

**Debugging.** `RDG Insights` in the Unreal editor visualizes the full pass graph, resource lifetimes, and barrier placement. The frame is data — export it, diff it, analyze offline.

**What RDG gets wrong (or leaves on the table):**

<div class="diagram-limits">
  <div class="dl-title">RDG Limitations</div>
  <div class="dl-item"><span class="dl-x">✗</span> <strong>Incomplete migration</strong> — Legacy FRHICommandList ←→ RDG boundary = manual barriers at the seam</div>
  <div class="dl-item"><span class="dl-x">✗</span> <strong>Macro-heavy API</strong> — BEGIN_SHADER_PARAMETER_STRUCT → opaque, no debugger stepping, fights dynamic composition</div>
  <div class="dl-item"><span class="dl-x">✗</span> <strong>Transient-only aliasing</strong> — Imported resources never aliased, even when lifetime is fully known within the frame</div>
  <div class="dl-item"><span class="dl-x">✗</span> <strong>No automatic subpass merging</strong> — Delegated to RHI — graph can't optimize tiles</div>
  <div class="dl-item"><span class="dl-x">✗</span> <strong>Async compute is opt-in</strong> — Manual ERDGPassFlags::AsyncCompute tagging. Compiler trusts, doesn't discover.</div>
</div>

### Where Frostbite started

Frostbite's frame graph (O'Donnell & Barczak, GDC 2017: *"FrameGraph: Extensible Rendering Architecture in Frostbite"*) is where the modern render graph concept originates.

<div class="diagram-innovations">
  <div class="di-title">Frostbite innovations that shaped every later engine</div>
  <div class="di-row"><div class="di-label">Transient resources</div><div>First production aliasing. 50% VRAM saved on Battlefield 1.</div></div>
  <div class="di-row"><div class="di-label">Split barriers</div><div>begin/end placement for GPU overlap. UE5 adopted this.</div></div>
  <div class="di-row"><div class="di-label">Graph export</div><div>DOT-format debug. Every engine since has built equivalent.</div></div>
  <div class="di-row"><div class="di-label">Dynamic rebuild</div><div>Full rebuild every frame. "Compile cost so low, caching adds complexity for nothing."</div></div>
</div>

**Frostbite vs UE5 — design spectrum:**

<div class="diagram-spectrum">
  <div class="ds-labels"><span>More aggressive</span><span>More conservative</span></div>
  <div class="ds-bar"></div>
  <div class="ds-cards">
    <div class="ds-card" style="border-color:#22c55e">
      <div class="ds-name" style="color:#22c55e">Frostbite</div>
      <span style="color:#22c55e">✓</span> fully dynamic<br>
      <span style="color:#22c55e">✓</span> alias everything<br>
      <span style="color:#22c55e">✓</span> subpass merging<br>
      <span style="color:#22c55e">✓</span> auto async<br>
      <span style="color:#ef4444">✗</span> no legacy support<br>
      <span style="color:#ef4444">✗</span> closed engine
    </div>
    <div class="ds-card" style="border-color:#3b82f6">
      <div class="ds-name" style="color:#3b82f6">UE5 RDG</div>
      <span style="color:#22c55e">✓</span> hybrid/cached<br>
      <span style="color:#22c55e">✓</span> transient only<br>
      <span style="color:#ef4444">✗</span> RHI-delegated<br>
      <span style="color:#ef4444">✗</span> opt-in async<br>
      <span style="color:#22c55e">✓</span> legacy compat<br>
      <span style="color:#22c55e">✓</span> 3P game code
    </div>
  </div>
  <div style="font-size:.78em;opacity:.6;margin-top:.5em">Frostbite controls the full engine. UE5 must support 25 years of existing code.</div>
</div>

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

<div class="diagram-steps">
  <div class="ds-step">
    <div class="ds-num">1</div>
    <div><strong>Process GBuf</strong> (16MB, pass 2–4): free list empty → no match → allocate <strong>Block A</strong> (16MB). Free list: [A: 16MB, avail after pass 4]</div>
  </div>
  <div class="ds-step">
    <div class="ds-num">2</div>
    <div><strong>Process SSAO</strong> (2MB, pass 3–5): Block A avail after 4, but SSAO starts at 3 → <span style="color:#ef4444;font-weight:600">overlap!</span> → allocate <strong>Block B</strong> (2MB). Free list: [A: avail after 4] [B: avail after 5]</div>
  </div>
  <div class="ds-step">
    <div class="ds-num">3</div>
    <div><strong>Process HDR</strong> (16MB, pass 5–7): Block A: 16MB, avail after 4, HDR starts at 5 → <span style="color:#22c55e;font-weight:600">✓ fits!</span> → reuse Block A. Free list: [A: avail after 7] [B: avail after 5]</div>
  </div>
</div>
<div class="diagram-card dc-success" style="font-weight:600">Result: 3 virtual resources → 2 physical blocks (34 MB → 18 MB)</div>

The greedy first-fit approach is provably optimal for interval graphs. For more aggressive packing, see **linear-scan register allocation** from compiler literature — same problem, different domain. Round up to power-of-two buckets to reduce fragmentation (UE5 does this).

**What to watch out for:**

<div class="diagram-warn">
  <div class="dw-title">⚠ Aliasing pitfalls</div>
  <div class="dw-row"><div class="dw-label">Format compat</div><div>depth/stencil metadata may conflict with color targets on same VkMemory → skip aliasing for depth formats</div></div>
  <div class="dw-row"><div class="dw-label">Initialization</div><div>reused memory = garbage contents → first use <strong>MUST</strong> be a full clear or fullscreen write</div></div>
  <div class="dw-row"><div class="dw-label">Imported res</div><div>survive across frames — <strong>never alias</strong>. Only transient resources qualify.</div></div>
</div>

UE5's transient allocator does exactly this. Add immediately after the MVP works.

### 2. Pass merging / subpass folding
**Priority: HIGH on mobile · Difficulty: Medium**

Critical for tile-based GPUs (Mali, Adreno, Apple). Merge compatible passes into Vulkan subpasses or Metal render pass load/store actions.

<div class="diagram-tiles">
  <div class="dt-col">
    <div class="dt-col-title">Without merging (tile-based GPU)</div>
    <div class="dt-col-body">
      <strong>Pass A (GBuffer)</strong><br>
      ├ render to tile<br>
      ├ <span class="dt-cost-bad">flush tile → main memory ✗ slow</span><br>
      └ done<br><br>
      <strong>Pass B (Lighting)</strong><br>
      ├ <span class="dt-cost-bad">load from main memory ✗ slow</span><br>
      ├ render to tile<br>
      ├ flush tile → main memory<br>
      └ done
    </div>
  </div>
  <div class="dt-col" style="border-color:#22c55e">
    <div class="dt-col-title" style="color:#22c55e">With merging</div>
    <div class="dt-col-body">
      <strong>Pass A+B (merged subpass)</strong><br>
      ├ render A to tile<br>
      ├ <span class="dt-cost-good">B reads tile directly (subpass input — free!)</span><br>
      └ flush once → main memory<br><br>
      <strong>Saves:</strong> 1 flush + 1 load per merged pair<br>
      = <span class="dt-cost-good">massive bandwidth savings on mobile</span>
    </div>
  </div>
</div>

**How the algorithm works:**

The algorithm walks the sorted pass list and identifies **merge candidates** — adjacent passes that pass a checklist:

<div class="diagram-tree">
  <div class="dt-node"><strong>Can Pass A and Pass B merge?</strong></div>
  <div class="dt-branch">
    <strong>Same RT dimensions?</strong>
    <span class="dt-no"> — no →</span> <span class="dt-result dt-fail">SKIP</span> <span style="opacity:.6">(tile sizes differ)</span>
    <div class="dt-branch">
      <span class="dt-yes">yes ↓</span><br>
      <strong>Compatible attachments?</strong>
      <span class="dt-no"> — no →</span> <span class="dt-result dt-fail">SKIP</span> <span style="opacity:.6">(B samples A at arbitrary UVs)</span>
      <div class="dt-branch">
        <span class="dt-yes">yes</span> <span style="opacity:.6">(B reads A's output at current pixel only)</span> <span class="dt-yes">↓</span><br>
        <strong>No external deps?</strong>
        <span class="dt-no"> — no →</span> <span class="dt-result dt-fail">SKIP</span> <span style="opacity:.6">(buffer dep leaves render pass)</span>
        <div class="dt-branch">
          <span class="dt-yes">yes ↓</span><br>
          <span class="dt-result dt-pass">MERGE A + B → 1 subpass</span><br>
          <span style="font-size:.85em;opacity:.7">union-find groups them → one VkRenderPass with N subpasses</span>
        </div>
      </div>
    </div>
  </div>
</div>

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

The interactive tool below lets you drag compute-eligible passes between queues and see fence costs in real time. After that, we'll cover the theory behind reachability analysis and fence minimization.

{{< interactive-async >}}

**How the reachability analysis works:**

<div class="diagram-card" style="margin-bottom:.5em"><strong>DAG:</strong> Depth → GBuf → Lighting → Tonemap, with GBuf → SSAO → Lighting</div>

<div class="diagram-bitset">
<table>
  <tr><th>Pass</th><th>Depth</th><th>GBuf</th><th>SSAO</th><th>Lighting</th><th>Tonemap</th><th>Reaches</th></tr>
  <tr><td><strong>Depth</strong></td><td class="bit-0">0</td><td class="bit-1">1</td><td class="bit-1">1</td><td class="bit-1">1</td><td class="bit-1">1</td><td>everything</td></tr>
  <tr><td><strong>GBuf</strong></td><td class="bit-0">0</td><td class="bit-0">0</td><td class="bit-1">1</td><td class="bit-1">1</td><td class="bit-1">1</td><td>SSAO, Light, Tone</td></tr>
  <tr><td><strong>SSAO</strong></td><td class="bit-0">0</td><td class="bit-0">0</td><td class="bit-0">0</td><td class="bit-1">1</td><td class="bit-1">1</td><td>Light, Tone</td></tr>
  <tr><td><strong>Lighting</strong></td><td class="bit-0">0</td><td class="bit-0">0</td><td class="bit-0">0</td><td class="bit-0">0</td><td class="bit-1">1</td><td>Tonemap</td></tr>
  <tr><td><strong>Tonemap</strong></td><td class="bit-0">0</td><td class="bit-0">0</td><td class="bit-0">0</td><td class="bit-0">0</td><td class="bit-0">0</td><td>leaf</td></tr>
</table>
</div>

<div class="diagram-card dc-success">
  <strong>Can Shadows overlap SSAO?</strong><br>
  reachable[Shadows].test(SSAO) = false, reachable[SSAO].test(Shadows) = false<br>
  → <strong>independent!</strong> → can run on different queues
</div>

**Steps:**

1. **Build reachability** — walk in reverse topological order. Each pass's bitset = union of successors' bitsets + successors themselves.
2. **Query independence** — two passes overlap iff neither can reach the other. One AND + one compare per query.
3. **Partition** — greedily assign compute-eligible passes to the compute queue whenever they're independent from the current graphics tail.

**Fence placement:**

Wherever a dependency edge crosses queue boundaries, you need a GPU fence (semaphore signal + wait). Walk the DAG edges: if source pass is on queue A and dest pass is on queue B, insert a fence. Minimize fence count by transitively reducing: if pass C already waits on a fence that pass B signaled, and pass B is after pass A on the same queue, pass C doesn't need a separate fence from pass A.

<div class="diagram-tiles">
  <div class="dt-col">
    <div class="dt-col-title"><span class="dt-cost-bad">Without transitive reduction</span></div>
    <div class="dt-col-body" style="font-family:ui-monospace,monospace;font-size:.9em">
      Graphics: [A] ──fence──→ [C]<br>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;└──fence──→ [D]<br><br>
      Compute: &nbsp;[B] ──fence──→ [C]<br>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;└──fence──→ [D]<br><br>
      <span class="dt-cost-bad">4 fences</span>
    </div>
  </div>
  <div class="dt-col" style="border-color:#22c55e">
    <div class="dt-col-title"><span class="dt-cost-good">With transitive reduction</span></div>
    <div class="dt-col-body" style="font-family:ui-monospace,monospace;font-size:.9em">
      Graphics: [A] ──────────→ [C]<br>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;↑<br>
      Compute: &nbsp;[B] ──fence──┘<br><br>
      B's fence covers both C and D<br>
      (D is after C on graphics queue)<br><br>
      <span class="dt-cost-good">1 fence</span>
    </div>
  </div>
</div>

**What to watch out for:**

<div class="diagram-tree">
  <div class="dt-node"><strong>Should this pass go async?</strong></div>
  <div class="dt-branch">
    <strong>Is it compute-only?</strong>
    <span class="dt-no"> — no →</span> <span class="dt-result dt-fail">can't</span> <span style="opacity:.6">(needs rasterization)</span>
    <div class="dt-branch">
      <span class="dt-yes">yes ↓</span><br>
      <strong>Duration > 0.5ms?</strong>
      <span class="dt-no"> — no →</span> <span class="dt-result dt-fail">don't bother</span> <span style="opacity:.6">(fence overhead ≈ 5–15µs eats the savings)</span>
      <div class="dt-branch">
        <span class="dt-yes">yes ↓</span><br>
        <strong>Independent from graphics tail?</strong>
        <span class="dt-no"> — no →</span> <span class="dt-result dt-fail">can't</span> <span style="opacity:.6">(DAG dependency)</span>
        <div class="dt-branch">
          <span class="dt-yes">yes ↓</span><br>
          <span class="dt-result dt-pass">ASYNC COMPUTE ✓</span><br>
          <span style="font-size:.85em;opacity:.7">Good candidates: SSAO, volumetrics, particle sim, light clustering</span>
        </div>
      </div>
    </div>
  </div>
</div>

| Concern | Detail |
|---------|--------|
| **Queue ownership** | Vulkan: explicit `srcQueueFamilyIndex`/`dstQueueFamilyIndex` transfer. D3D12: `ID3D12Fence`. Both expensive — only if overlap wins exceed transfer cost. |
| **HW contention** | NVIDIA: separate async engines. AMD: more independent CUs. Some GPUs just time-slice — profile to confirm real overlap. |

In UE5, you opt in per pass with `ERDGPassFlags::AsyncCompute`; the RDG compiler handles fence insertion and cross-queue synchronization. Add after you have GPU-bound workloads that can genuinely overlap (e.g., SSAO while shadow maps render).

### 4. Split barriers
**Priority: LOW · Difficulty: High**

Place "begin" barrier as early as possible (right after the source pass finishes), "end" barrier as late as possible (right before the destination pass starts) → GPU has more room to overlap work between them. Drag the BEGIN marker in the interactive tool below to see how the overlap gap changes:

{{< interactive-split-barriers >}}

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

<div class="diagram-steps">
  <div class="ds-step">
    <div class="ds-num" style="background:#3b82f6">3</div>
    <div><strong>Pass 3: GBuffer write</strong> ← <span style="color:#3b82f6;font-weight:600">begin barrier here</span> (flush COLOR_ATTACHMENT caches)</div>
  </div>
  <div class="ds-step">
    <div class="ds-num" style="background:#6b7280">4</div>
    <div>Pass 4: SSAO (unrelated) &nbsp; <span style="opacity:.5">↕ GPU freely executes pass 4 & 5</span></div>
  </div>
  <div class="ds-step">
    <div class="ds-num" style="background:#6b7280">5</div>
    <div>Pass 5: Bloom (unrelated)</div>
  </div>
  <div class="ds-step">
    <div class="ds-num" style="background:#22c55e">6</div>
    <div><strong>Pass 6: Lighting read</strong> ← <span style="color:#22c55e;font-weight:600">end barrier here</span> (invalidate SHADER_READ caches)</div>
  </div>
</div>
<div class="diagram-card dc-success">Gap of 2 passes = 2 passes of free GPU overlap</div>

**What to watch out for:**

<div class="diagram-ftable">
<table>
  <tr><th>Gap size</th><th>Action</th><th>Why</th></tr>
  <tr><td><strong>0 passes</strong></td><td>regular barrier</td><td>begin/end adjacent → no benefit</td></tr>
  <tr><td><strong>1 pass</strong></td><td>maybe</td><td>marginal overlap</td></tr>
  <tr><td><strong>2+ passes</strong></td><td>split</td><td>measurable GPU overlap</td></tr>
  <tr><td><strong>cross-queue</strong></td><td>fence instead</td><td>can't split across queues</td></tr>
</table>
</div>

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

---

## Resources

Further reading, ordered from "start here" to deep dives.

<div class="diagram-nav">
  <div class="dn-col">
    <div class="dn-title" style="color:#22c55e">Start here</div>
    Wijiler video (15 min)<br>
    Loggini overview<br>
    GPUOpen render graphs
  </div>
  <div class="dn-col">
    <div class="dn-title" style="color:#3b82f6">Go deeper</div>
    Frostbite GDC talk<br>
    themaister blog<br>
    D3D12 barriers doc
  </div>
  <div class="dn-col">
    <div class="dn-title" style="color:#8b5cf6">Go deepest</div>
    UE5 RDG source<br>
    Vulkan sync blog<br>
    AMD RPS SDK
  </div>
</div>

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

