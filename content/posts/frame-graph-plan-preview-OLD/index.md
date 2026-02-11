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

<div style="margin:1.2em 0 1.5em;padding:1.3em 1.5em;border-radius:12px;border:1.5px solid rgba(99,102,241,.18);background:linear-gradient(135deg,rgba(99,102,241,.04),rgba(34,197,94,.03));">
  <div style="display:grid;grid-template-columns:1fr auto 1fr;gap:.3em .8em;align-items:center;font-size:1em;line-height:1.6;">
    <span style="text-decoration:line-through;opacity:.4;text-align:right;">Passes run in whatever order you wrote them.</span>
    <span style="opacity:.35;">→</span>
    <strong>Sorted by dependencies.</strong>
    <span style="text-decoration:line-through;opacity:.4;text-align:right;">Every GPU sync point placed by hand.</span>
    <span style="opacity:.35;">→</span>
    <strong>Barriers inserted for you.</strong>
    <span style="text-decoration:line-through;opacity:.4;text-align:right;">Each pass allocates its own memory — 900 MB gone.</span>
    <span style="opacity:.35;">→</span>
    <strong style="color:#22c55e;">Resources shared safely — ~450 MB back.</strong>
  </div>
  <div style="margin-top:.8em;padding-top:.7em;border-top:1px solid rgba(99,102,241,.1);font-size:.88em;opacity:.7;line-height:1.5;text-align:center;">
    You describe <em>what</em> each pass needs — the graph figures out the <em>how</em>.
  </div>
</div>

Frostbite introduced it at GDC 2017. UE5 ships it as **RDG**. Unity has its own in SRP. Every major renderer uses one — this article shows you why, and walks you through building your own.

<div style="margin:1.5em 0;border-radius:12px;overflow:hidden;border:1.5px solid rgba(99,102,241,.25);background:linear-gradient(135deg,rgba(99,102,241,.04),transparent);">
  <div style="display:grid;grid-template-columns:repeat(3,1fr);gap:0;">
    <div style="padding:1em;text-align:center;border-right:1px solid rgba(99,102,241,.12);border-bottom:1px solid rgba(99,102,241,.12);">
      <div style="font-size:1.6em;margin-bottom:.15em;">🔨</div>
      <div style="font-weight:800;font-size:.95em;">Build</div>
      <div style="font-size:.82em;opacity:.7;line-height:1.4;margin-top:.2em;">Working C++ frame graph, from scratch to prototype</div>
    </div>
    <div style="padding:1em;text-align:center;border-right:1px solid rgba(99,102,241,.12);border-bottom:1px solid rgba(99,102,241,.12);">
      <div style="font-size:1.6em;margin-bottom:.15em;">🗺️</div>
      <div style="font-weight:800;font-size:.95em;">Map to UE5</div>
      <div style="font-size:.82em;opacity:.7;line-height:1.4;margin-top:.2em;">Every piece maps to RDG — read the source with confidence</div>
    </div>
    <div style="padding:1em;text-align:center;border-bottom:1px solid rgba(99,102,241,.12);">
      <div style="font-size:1.6em;margin-bottom:.15em;">🚀</div>
      <div style="font-weight:800;font-size:.95em;">Go further</div>
      <div style="font-size:.82em;opacity:.7;line-height:1.4;margin-top:.2em;">Aliasing, pass merging, async compute, split barriers</div>
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

A frame graph is a **directed acyclic graph (DAG)** — each node is a render pass, each edge is a resource one pass hands to the next. Here's what a typical deferred frame looks like:

<!-- DAG flow diagram -->
<div style="margin:1.2em 0 .3em;">
<div class="diagram-flow" style="justify-content:center">
  <div class="df-step df-primary">Depth<br>Prepass<span class="df-sub">depth</span></div>
  <div class="df-arrow"></div>
  <div class="df-step df-primary">GBuffer<br>Pass<span class="df-sub">albedo · normals · depth</span></div>
  <div class="df-arrow"></div>
  <div class="df-step" style="display:flex;flex-direction:column;gap:.3em;padding:.5em .8em">
    <div class="df-step df-primary" style="border:none;padding:.3em .6em;font-size:.9em">SSAO<span class="df-sub">occlusion</span></div>
    <div style="opacity:.4;font-size:.75em;">↕</div>
    <div class="df-step df-primary" style="border:none;padding:.3em .6em;font-size:.9em">Lighting<span class="df-sub">HDR color</span></div>
  </div>
  <div class="df-arrow"></div>
  <div class="df-step df-success">Tonemap<span class="df-sub">→ present</span></div>
</div>
<div style="text-align:center;margin-top:.1em;">
  <span style="display:inline-block;font-size:.76em;opacity:.55;border:1px solid rgba(99,102,241,.15);border-radius:6px;padding:.25em .7em;">nodes = passes &nbsp;·&nbsp; edges = resource flow &nbsp;·&nbsp; arrows = write → read</span>
</div>
</div>

You don't execute this graph directly. Every frame goes through three steps — first you **declare** all the passes and what they read/write, then the system **compiles** an optimized plan (ordering, memory, barriers), and finally it **executes** the result:

<!-- 3-step lifecycle — distinct style from the DAG above -->
<div style="margin:.8em auto 1.2em;max-width:560px;">
  <div style="display:flex;align-items:stretch;gap:0;border-radius:10px;overflow:hidden;border:1.5px solid rgba(99,102,241,.2);">
    <div style="flex:1;padding:.7em .6em;text-align:center;background:rgba(59,130,246,.06);border-right:1px solid rgba(99,102,241,.12);">
      <div style="font-weight:800;font-size:.88em;letter-spacing:.04em;color:#3b82f6;">①&ensp;DECLARE</div>
      <div style="font-size:.75em;opacity:.6;margin-top:.2em;">passes &amp; dependencies</div>
    </div>
    <div style="flex:1;padding:.7em .6em;text-align:center;background:rgba(139,92,246,.06);border-right:1px solid rgba(99,102,241,.12);">
      <div style="font-weight:800;font-size:.88em;letter-spacing:.04em;color:#8b5cf6;">②&ensp;COMPILE</div>
      <div style="font-size:.75em;opacity:.6;margin-top:.2em;">order · aliases · barriers</div>
    </div>
    <div style="flex:1;padding:.7em .6em;text-align:center;background:rgba(34,197,94,.06);">
      <div style="font-weight:800;font-size:.88em;letter-spacing:.04em;color:#22c55e;">③&ensp;EXECUTE</div>
      <div style="font-size:.75em;opacity:.6;margin-top:.2em;">record GPU commands</div>
    </div>
  </div>
</div>

Let's look at each step.

---

## The Declare Step

Each frame starts on the CPU. You register passes, describe the resources they need, and declare who reads or writes what. No GPU work happens yet — you're building a description of the frame.

<div class="diagram-box">
  <div class="db-title">📋 DECLARE — building the graph</div>
  <div class="db-body">
    <div class="diagram-pipeline">
      <div class="dp-stage">
        <div class="dp-title">ADD PASSES</div>
        <ul><li><code>addPass(setup, execute)</code></li><li>setup runs now, execute runs later</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">DECLARE RESOURCES</div>
        <ul><li><code>create({1920,1080, RGBA8})</code></li><li>returns a handle — no allocation yet</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">WIRE DEPENDENCIES</div>
        <ul><li><code>read(h)</code> / <code>write(h)</code></li><li>these edges form the DAG</li></ul>
      </div>
    </div>
    <div style="text-align:center;font-size:.82em;opacity:.6;margin-top:.3em">CPU only — the GPU is idle during this phase</div>
  </div>
</div>

<div style="margin:1.2em 0;padding:1.1em 1.3em;border-radius:10px;border:1.5px dashed rgba(99,102,241,.3);background:rgba(99,102,241,.04);display:flex;align-items:center;gap:1.2em;flex-wrap:wrap;">
  <div style="flex:1;min-width:180px;">
    <div style="font-size:1.15em;font-weight:800;margin:.1em 0;">Handle #3</div>
    <div style="font-size:.82em;opacity:.6;">1920×1080 · RGBA8 · render target</div>
  </div>
  <div style="flex-shrink:0;padding:.4em .8em;border-radius:6px;background:rgba(245,158,11,.1);color:#f59e0b;font-weight:700;font-size:.8em;">description only — no GPU memory yet</div>
</div>
<div style="text-align:center;font-size:.82em;opacity:.6;margin-top:-.2em;">
  Resources stay virtual at this stage — just a description and a handle. Memory comes later.
</div>

### Transient vs. imported

When you declare a resource, the graph needs to know one thing: **does it live inside this frame, or does it come from outside?**

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

---

## The Compile Step

The declared DAG goes in, an optimized execution plan comes out. Three things happen — all near-linear, all in microseconds for a typical frame.

<div class="diagram-box">
  <div class="db-title">🔍 COMPILE — turning the DAG into a plan</div>
  <div class="db-body">
    <div class="diagram-pipeline">
      <div class="dp-stage">
        <div class="dp-title">SCHEDULE</div>
        <ul><li>topological sort (Kahn's)</li><li>cycle detection</li><li>→ final pass order</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">ALLOCATE</div>
        <ul><li>sort transients by first use</li><li>greedy-fit into physical slots</li><li>→ alias map</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">SYNCHRONIZE</div>
        <ul><li>walk each resource handoff</li><li>emit minimal barriers</li><li>→ barrier list</li></ul>
      </div>
    </div>
    <div style="text-align:center;font-size:.82em;opacity:.6;margin-top:.3em">Still CPU — producing data structures for the execute phase</div>
  </div>
</div>

<!-- Visual: the virtual handle from Declare is now resolved -->
<div style="margin:1.2em 0;display:flex;align-items:center;gap:1em;flex-wrap:wrap;">
  <div style="flex:1;min-width:180px;padding:1em 1.2em;border-radius:10px;border:1.5px dashed rgba(99,102,241,.3);background:rgba(99,102,241,.04);">
    <div style="font-size:1.1em;font-weight:800;">Handle #3</div>
    <div style="font-size:.8em;opacity:.5;">1920×1080 · RGBA8</div>
    <div style="margin-top:.4em;font-size:.75em;padding:.25em .6em;border-radius:6px;background:rgba(245,158,11,.1);color:#f59e0b;font-weight:700;display:inline-block;">virtual</div>
  </div>
  <div style="font-size:1.4em;opacity:.3;flex-shrink:0;">→</div>
  <div style="flex:1;min-width:180px;padding:1em 1.2em;border-radius:10px;border:1.5px solid rgba(34,197,94,.3);background:rgba(34,197,94,.04);">
    <div style="font-size:1.1em;font-weight:800;">Handle #3 <span style="opacity:.35;">→</span> <span style="color:#22c55e;">Pool slot 0</span></div>
    <div style="font-size:.8em;opacity:.5;">shares 8 MB with Handle #7</div>
    <div style="margin-top:.4em;font-size:.75em;padding:.25em .6em;border-radius:6px;background:rgba(34,197,94,.1);color:#22c55e;font-weight:700;display:inline-block;">aliased</div>
  </div>
</div>
<div style="text-align:center;font-size:.82em;opacity:.6;margin-top:-.2em;">
  The compiler sees every resource lifetime at once — non-overlapping handles land on the same physical memory.
</div>

---

## The Execute Step

The plan is ready — now the GPU gets involved. This phase walks the compiled pass order, applies barriers, and calls your execute lambdas.

<div class="diagram-box">
  <div class="db-title">▶️ EXECUTE — recording GPU commands</div>
  <div class="db-body">
    <div class="diagram-pipeline">
      <div class="dp-stage">
        <div class="dp-title">BACK RESOURCES</div>
        <ul><li>create or reuse physical memory</li><li>apply the alias map</li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">RUN PASSES</div>
        <ul><li>for each pass in compiled order:</li><li>insert barriers → call <code>execute()</code></li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">CLEANUP</div>
        <ul><li>release transients (or pool them)</li><li>reset the frame allocator</li></ul>
      </div>
    </div>
    <div style="text-align:center;font-size:.82em;opacity:.6;margin-top:.3em">The only phase that touches the GPU API</div>
  </div>
</div>

<div style="margin:1.2em 0;padding:1em 1.2em;border-radius:10px;border:1.5px solid rgba(34,197,94,.2);background:rgba(34,197,94,.04);font-size:.92em;line-height:1.6;">
  Each execute lambda sees a <strong>fully resolved environment</strong> — barriers already placed, memory already allocated, resources ready to bind. The lambda just records draw calls, dispatches, and copies. All the intelligence lives in the compile step.
</div>

---

## Rebuild Strategies

How often should the graph recompile? Three approaches, each a valid tradeoff:

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

<div style="margin:1.2em 0;display:grid;grid-template-columns:1fr 1fr;gap:0;border-radius:10px;overflow:hidden;border:2px solid rgba(99,102,241,.25);box-shadow:0 2px 8px rgba(0,0,0,.08);">
  <div style="padding:.6em 1em;font-weight:800;font-size:.95em;background:rgba(239,68,68,.1);border-bottom:1.5px solid rgba(99,102,241,.15);border-right:1.5px solid rgba(99,102,241,.15);color:#ef4444;">❌ Without Graph</div>
  <div style="padding:.6em 1em;font-weight:800;font-size:.95em;background:rgba(34,197,94,.1);border-bottom:1.5px solid rgba(99,102,241,.15);color:#22c55e;">✅ With Graph</div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(99,102,241,.1);border-right:1.5px solid rgba(99,102,241,.15);background:rgba(239,68,68,.02);">
    <strong>Memory aliasing</strong><br><span style="opacity:.65">Opt-in, fragile, rarely done</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(99,102,241,.1);background:rgba(34,197,94,.02);">
    <strong>Memory aliasing</strong><br>Automatic — compiler sees all lifetimes. <strong style="color:#22c55e;">30–50% VRAM saved.</strong>
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(99,102,241,.1);border-right:1.5px solid rgba(99,102,241,.15);background:rgba(239,68,68,.02);">
    <strong>Lifetimes</strong><br><span style="opacity:.65">Manual create/destroy, leaked or over-retained</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(99,102,241,.1);background:rgba(34,197,94,.02);">
    <strong>Lifetimes</strong><br>Scoped to first..last use. Zero waste.
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(99,102,241,.1);border-right:1.5px solid rgba(99,102,241,.15);background:rgba(239,68,68,.02);">
    <strong>Barriers</strong><br><span style="opacity:.65">Manual, per-pass</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(99,102,241,.1);background:rgba(34,197,94,.02);">
    <strong>Barriers</strong><br>Automatic from declared read/write
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(99,102,241,.1);border-right:1.5px solid rgba(99,102,241,.15);background:rgba(239,68,68,.02);">
    <strong>Pass reordering</strong><br><span style="opacity:.65">Breaks silently</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(99,102,241,.1);background:rgba(34,197,94,.02);">
    <strong>Pass reordering</strong><br>Safe — compiler respects dependencies
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(99,102,241,.1);border-right:1.5px solid rgba(99,102,241,.15);background:rgba(239,68,68,.02);">
    <strong>Pass culling</strong><br><span style="opacity:.65">Manual ifdef / flag checks</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(99,102,241,.1);background:rgba(34,197,94,.02);">
    <strong>Pass culling</strong><br>Automatic — unused outputs = dead pass
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-right:1.5px solid rgba(99,102,241,.15);background:rgba(239,68,68,.02);">
    <strong>Async compute</strong><br><span style="opacity:.65">Manual queue sync</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;background:rgba(34,197,94,.02);">
    <strong>Async compute</strong><br>Compiler schedules across queues
  </div>
</div>

<div style="margin:1.2em 0;padding:.8em 1em;border-radius:8px;background:linear-gradient(135deg,rgba(34,197,94,.06),rgba(59,130,246,.06));border:1px solid rgba(34,197,94,.2);font-size:.92em;line-height:1.6;">
🏭 <strong>Not theoretical.</strong> Frostbite reported <strong>50% VRAM reduction</strong> from aliasing at GDC 2017. UE5's RDG ships the same optimization today — every <code>FRDGTexture</code> marked as transient goes through the same aliasing pipeline we're about to build.<br>
<span style="opacity:.7;font-size:.9em;">The MVP gives you automatic lifetimes + aliasing by Section 8, automatic barriers by Section 7. After that, we map everything to UE5's RDG.</span>
</div>

---

# Part II — Build It

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
⚠️ <strong>Caveat:</strong> UE5's migration to RDG is <em>incomplete</em>. Large parts of the renderer still use legacy immediate-mode <code>FRHICommandList</code> calls outside the graph. These “untracked” resources bypass RDG's barrier and aliasing systems entirely — you get the graph's benefits only for passes that have been ported. Legacy passes still need manual barriers at the RDG boundary. This is the cost of retrofitting a graph onto a 25-year-old codebase.
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
    <div style="font-size:.92em;line-height:1.5">Async compute, split barriers, pass merging, parallel recording. These are production features — covered in the Upgrade Roadmap below.</div>
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

# Part III — Production Engines

<div style="border-left:4px solid #6366f1;background:linear-gradient(135deg,rgba(99,102,241,.06),transparent);border-radius:0 10px 10px 0;padding:1em 1.3em;margin:1em 0;font-size:.95em;font-style:italic;line-height:1.55">
How UE5, Frostbite, and Unity implement the same ideas at scale — what they added, what they compromised, and where they still differ.
</div>

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

<div style="display:flex;gap:1em;flex-wrap:wrap;margin:1em 0">
  <div style="flex:1;min-width:260px;border:1px solid rgba(59,130,246,.25);border-radius:10px;overflow:hidden">
    <div style="background:linear-gradient(135deg,rgba(59,130,246,.12),rgba(59,130,246,.05));padding:.6em 1em;font-weight:700;font-size:.9em;color:#3b82f6;border-bottom:1px solid rgba(59,130,246,.15)">Pass Flags</div>
    <div style="padding:.6em 1em;font-size:.85em;line-height:1.8">
      <code>ERDGPassFlags::Raster</code> — Graphics queue, render targets<br>
      <code>ERDGPassFlags::Compute</code> — Graphics queue, compute dispatch<br>
      <code>ERDGPassFlags::AsyncCompute</code> — Async compute queue<br>
      <code>ERDGPassFlags::NeverCull</code> — Exempt from dead-pass culling
    </div>
  </div>
  <div style="flex:1;min-width:260px;border:1px solid rgba(139,92,246,.25);border-radius:10px;overflow:hidden">
    <div style="background:linear-gradient(135deg,rgba(139,92,246,.12),rgba(139,92,246,.05));padding:.6em 1em;font-weight:700;font-size:.9em;color:#8b5cf6;border-bottom:1px solid rgba(139,92,246,.15)">Resource Types</div>
    <div style="padding:.6em 1em;font-size:.85em;line-height:1.8">
      <code>FRDGTexture</code> / <code>FRDGTextureRef</code> — Render targets, SRVs, UAVs<br>
      <code>FRDGBuffer</code> / <code>FRDGBufferRef</code> — Structured, vertex/index, indirect args
    </div>
  </div>
</div>
<div style="font-size:.82em;opacity:.6;margin-top:-.3em">Both go through the same aliasing and barrier system.</div>

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

<div style="display:flex;align-items:flex-start;gap:.8em;border:1px solid rgba(34,197,94,.2);border-radius:10px;padding:1em 1.2em;margin:1em 0;background:linear-gradient(135deg,rgba(34,197,94,.05),transparent)">
  <span style="font-size:1.4em;line-height:1">🔍</span>
  <div style="font-size:.9em;line-height:1.55"><strong>Debugging.</strong> <code>RDG Insights</code> in the Unreal editor visualizes the full pass graph, resource lifetimes, and barrier placement. The frame is data — export it, diff it, analyze offline.</div>
</div>

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

<div style="border:1px solid rgba(34,197,94,.2);border-radius:10px;padding:1em 1.2em;margin:1em 0;background:linear-gradient(135deg,rgba(34,197,94,.05),transparent)">
  <div style="font-weight:700;color:#22c55e;margin-bottom:.3em">Unity — SRP Render Graph</div>
  <div style="font-size:.9em;line-height:1.55">Shipped as part of the Scriptable Render Pipeline. Handles pass culling and transient resource aliasing in URP/HDRP backends. Async compute support varies by platform. Designed for portability across mobile and desktop, so it avoids the more aggressive GPU-specific optimizations.</div>
</div>

### Comparison

<div style="overflow-x:auto;margin:1em 0">
<table style="width:100%;border-collapse:collapse;border-radius:10px;overflow:hidden;font-size:.9em">
  <thead>
    <tr style="background:linear-gradient(135deg,rgba(99,102,241,.1),rgba(59,130,246,.08))">
      <th style="padding:.7em 1em;text-align:left;border-bottom:2px solid rgba(99,102,241,.2)">Feature</th>
      <th style="padding:.7em 1em;text-align:center;border-bottom:2px solid rgba(59,130,246,.2);color:#3b82f6">UE5 RDG</th>
      <th style="padding:.7em 1em;text-align:center;border-bottom:2px solid rgba(34,197,94,.2);color:#22c55e">Frostbite</th>
      <th style="padding:.7em 1em;text-align:center;border-bottom:2px solid rgba(139,92,246,.2);color:#8b5cf6">Unity SRP</th>
    </tr>
  </thead>
  <tbody>
    <tr><td style="padding:.5em 1em;font-weight:600">Rebuild strategy</td><td style="padding:.5em 1em;text-align:center">hybrid (cached)</td><td style="padding:.5em 1em;text-align:center">dynamic</td><td style="padding:.5em 1em;text-align:center">dynamic</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.5em 1em;font-weight:600">Pass culling</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> auto</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> refcount</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> auto</td></tr>
    <tr><td style="padding:.5em 1em;font-weight:600">Memory aliasing</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> transient</td><td style="padding:.5em 1em;text-align:center;font-weight:600;color:#22c55e">✓ full</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> transient</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.5em 1em;font-weight:600">Async compute</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span> flag-based</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td><td style="padding:.5em 1em;text-align:center;opacity:.6">varies</td></tr>
    <tr><td style="padding:.5em 1em;font-weight:600">Split barriers</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td><td style="padding:.5em 1em;text-align:center"><span style="color:#ef4444">✗</span></td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.5em 1em;font-weight:600">Parallel recording</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td><td style="padding:.5em 1em;text-align:center;opacity:.6">limited</td></tr>
    <tr><td style="padding:.5em 1em;font-weight:600">Buffer tracking</td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td><td style="padding:.5em 1em;text-align:center"><span style="color:#22c55e">✓</span></td></tr>
  </tbody>
</table>
</div>

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

<div style="overflow-x:auto;margin:1em 0">
<table style="width:100%;border-collapse:collapse;font-size:.88em;border-radius:10px;overflow:hidden">
  <thead>
    <tr style="background:linear-gradient(135deg,rgba(59,130,246,.1),rgba(139,92,246,.08))">
      <th style="padding:.65em 1em;text-align:left;border-bottom:2px solid rgba(59,130,246,.15)">API</th>
      <th style="padding:.65em 1em;text-align:left;border-bottom:2px solid rgba(59,130,246,.15)">Merged group becomes</th>
      <th style="padding:.65em 1em;text-align:left;border-bottom:2px solid rgba(59,130,246,.15)">Intermediate attachments</th>
    </tr>
  </thead>
  <tbody>
    <tr><td style="padding:.5em 1em;font-weight:600;color:#ef4444">Vulkan</td><td style="padding:.5em 1em">Single <code>VkRenderPass</code> + N <code>VkSubpassDescription</code></td><td style="padding:.5em 1em">Subpass inputs (tile-local read)</td></tr>
    <tr style="background:rgba(127,127,127,.04)"><td style="padding:.5em 1em;font-weight:600;color:#6b7280">Metal</td><td style="padding:.5em 1em">One <code>MTLRenderPassDescriptor</code>, <code>storeAction = .dontCare</code></td><td style="padding:.5em 1em"><code>loadAction = .load</code></td></tr>
    <tr><td style="padding:.5em 1em;font-weight:600;color:#3b82f6">D3D12</td><td style="padding:.5em 1em"><code>BeginRenderPass</code>/<code>EndRenderPass</code> (Tier 1/2)</td><td style="padding:.5em 1em">No direct subpass — via render pass tiers</td></tr>
  </tbody>
</table>
</div>

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

