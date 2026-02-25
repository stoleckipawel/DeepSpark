---
title: "Frame Graph — Theory"
date: 2026-02-09
lastmod: 2026-02-23
draft: false
authors: ["Pawel Stolecki"]
description: "The theory behind frame graphs — how a DAG of passes and resources gives the compiler enough information to automate scheduling, barriers, and memory aliasing."
tags: ["rendering", "frame-graph", "gpu", "architecture"]
categories: ["analysis"]
summary: "Declare → Compile → Execute. How a directed acyclic graph of render passes and virtual resources lets an engine automate topological sorting, barrier insertion, pass culling, and VRAM aliasing."
showTableOfContents: false
keywords: ["frame graph", "render graph", "render pass", "DAG", "topological sort", "GPU barriers", "resource aliasing", "VRAM", "Vulkan", "D3D12"]
---

{{< article-nav >}}

<div class="ds-series-nav">
📖 <strong>Part I of IV.</strong>&ensp; <em>Theory</em> → <a href="../frame-graph-build-it/">Build It</a> → <a href="../frame-graph-advanced/">Beyond MVP</a> → <a href="../frame-graph-production/">Production Engines</a>
</div>

---

Behind every smooth frame is a brutal scheduling problem — which passes can run in parallel, which buffers can reuse the same memory, and which barriers are actually necessary. Frame graphs solve it: declare what each pass reads and writes, and the graph handles the rest. This series breaks down the theory, builds a real implementation in C++, and shows how the same ideas scale to production engines like UE5's RDG.

<div class="fg-reveal" style="margin:1.5em 0;border-radius:12px;overflow:hidden;border:1.5px solid rgba(var(--ds-indigo-rgb),.25);background:linear-gradient(135deg,rgba(var(--ds-indigo-rgb),.04),transparent);">
  <div style="display:grid;grid-template-columns:repeat(3,1fr);gap:0;">
    <div style="padding:1em;text-align:center;border-right:1px solid rgba(var(--ds-indigo-rgb),.12);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.12);">
      <div style="font-size:1.6em;margin-bottom:.15em;">📖</div>
      <div style="font-weight:800;font-size:.95em;">Learn Theory</div>
      <div style="font-size:.82em;opacity:.7;line-height:1.4;margin-top:.2em;">What a frame graph is, why every engine uses one, and how each piece works</div>
    </div>
    <div style="padding:1em;text-align:center;border-right:1px solid rgba(var(--ds-indigo-rgb),.12);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.12);">
      <div style="font-size:1.6em;margin-bottom:.15em;">🔨</div>
      <div style="font-weight:800;font-size:.95em;">Build MVP</div>
      <div style="font-size:.82em;opacity:.7;line-height:1.4;margin-top:.2em;">Working C++ frame graph, from scratch to prototype in ~500 lines</div>
    </div>
    <div style="padding:1em;text-align:center;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.12);">
      <div style="font-size:1.6em;margin-bottom:.15em;">🗺</div>
      <div style="font-weight:800;font-size:.95em;">Map to UE5</div>
      <div style="font-size:.82em;opacity:.7;line-height:1.4;margin-top:.2em;">Every piece maps to RDG — read the source with confidence</div>
    </div>
  </div>
</div>

---

## 🔥 The Problem

<div class="fg-reveal" style="position:relative;margin:1.4em 0;">

  <div style="margin-bottom:1.6em;">
    <div style="font-weight:800;font-size:1.05em;color:var(--ds-success);margin-bottom:.3em;">Month 1 — 3 passes, everything's fine</div>
    <div style="font-size:.92em;line-height:1.6;">
      Depth prepass → GBuffer → lighting. Two barriers, hand-placed. Two textures, both allocated at init. Code is clean, readable, correct.
    </div>
    <div class="ds-callout ds-callout--success" style="margin-top:.4em;padding:.4em .8em;font-size:.88em;font-style:italic;">
      At this scale, manual management works. You know every resource by name.
    </div>
  </div>

  <div style="margin-bottom:1.6em;">
    <div style="font-weight:800;font-size:1.05em;color:var(--ds-warn);margin-bottom:.3em;">Month 6 — 12 passes, cracks appear</div>
    <div style="font-size:.92em;line-height:1.6;">
      Same renderer, now with SSAO, SSR, bloom, TAA, shadow cascades. Three things going wrong simultaneously:
    </div>
    <div style="margin-top:.5em;display:grid;gap:.4em;">
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(var(--ds-warn-rgb),.2);background:rgba(var(--ds-warn-rgb),.04);font-size:.88em;line-height:1.5;">
        <strong>Invisible dependencies</strong> — someone adds SSAO but doesn't realize GBuffer needs an updated barrier. Visual artifacts on fresh build.
      </div>
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(var(--ds-warn-rgb),.2);background:rgba(var(--ds-warn-rgb),.04);font-size:.88em;line-height:1.5;">
        <strong>Wasted memory</strong> — SSAO and bloom textures never overlap, but aliasing them means auditing every pass that might touch them. Nobody does it.
      </div>
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(var(--ds-warn-rgb),.2);background:rgba(var(--ds-warn-rgb),.04);font-size:.88em;line-height:1.5;">
        <strong>Silent reordering</strong> — two branches touch the render loop. Git merges cleanly, but the shadow pass ends up after lighting. Subtly wrong output ships unnoticed.
      </div>
    </div>
    <div class="ds-callout ds-callout--warn" style="margin-top:.5em;padding:.4em .8em;font-size:.88em;font-style:italic;">
      No single change broke it. The accumulation broke it.
    </div>
  </div>

  <div>
    <div style="font-weight:800;font-size:1.05em;color:var(--ds-danger);margin-bottom:.3em;">Month 18 — 25 passes, nobody touches it</div>
    <div style="font-size:.92em;line-height:1.6;margin-bottom:.5em;">The renderer works, but:</div>
    <div style="display:grid;gap:.4em;">
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(var(--ds-danger-rgb),.2);background:rgba(var(--ds-danger-rgb),.04);font-size:.88em;line-height:1.5;">
        <strong>900 MB VRAM.</strong> Profiling shows 400 MB is aliasable — but the lifetime analysis would take a week and break the next time anyone adds a pass.
      </div>
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(var(--ds-danger-rgb),.2);background:rgba(var(--ds-danger-rgb),.04);font-size:.88em;line-height:1.5;">
        <strong>47 barrier calls.</strong> Three are redundant, two are missing, one is in the wrong queue. Nobody knows which.
      </div>
      <div style="padding:.5em .8em;border-radius:6px;border:1px solid rgba(var(--ds-danger-rgb),.2);background:rgba(var(--ds-danger-rgb),.04);font-size:.88em;line-height:1.5;">
        <strong>2 days to add a new pass.</strong> 30 minutes for the shader, the rest to figure out where to slot it and what barriers it needs.
      </div>
    </div>
    <div class="ds-callout ds-callout--danger" style="margin-top:.5em;padding:.4em .8em;font-size:.88em;font-style:italic;">
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
  <div style="color:var(--ds-success);font-weight:700">✓ manageable</div>
  <div style="color:var(--ds-warn);font-weight:700">⚠ fragile</div>
  <div style="color:var(--ds-danger);font-weight:700">✗ untouchable</div>
</div>

The pattern is always the same: manual resource management works at small scale and fails at compound scale. Not because engineers are sloppy — because *no human tracks 25 lifetimes and 47 transitions in their head every sprint*. You need a system that sees the whole frame at once.

---

## 💡 The Core Idea

A frame graph models an entire frame as a **directed acyclic graph (DAG)**. Each node is a render pass; each edge carries a resource — a texture, a buffer, an attachment — from the pass that writes it to every pass that reads it. Here's what a typical deferred-rendering frame looks like:

<!-- DAG flow diagram -->
<div style="margin:1.6em 0 .5em;text-align:center;">
<svg viewBox="0 0 1050 210" width="100%" style="max-width:1050px;display:block;margin:0 auto;font-family:inherit;" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <!-- grB: --ds-info-light (#60a5fa) → --ds-info-dark (#2563eb) -->
    <linearGradient id="grB" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#60a5fa"/><stop offset="100%" stop-color="#2563eb"/></linearGradient>
    <!-- grO: --ds-warn-light (#fbbf24) → --ds-warn-dark (#d97706) -->
    <linearGradient id="grO" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#fbbf24"/><stop offset="100%" stop-color="#d97706"/></linearGradient>
    <!-- grG: --ds-success-light (#4ade80) → --ds-success-dark (#16a34a) -->
    <linearGradient id="grG" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#4ade80"/><stop offset="100%" stop-color="#16a34a"/></linearGradient>
    <!-- grR: red-400 (#f87171) → --ds-danger-dark (#dc2626) -->
    <linearGradient id="grR" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#f87171"/><stop offset="100%" stop-color="#dc2626"/></linearGradient>
    <marker id="ah" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse"><path d="M1,1 L9,5 L1,9" fill="none" stroke="rgba(255,255,255,.35)" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"/></marker>
  </defs>
  <!-- base edges -->
  <path d="M115,100 L155,100 L155,42 L195,42" fill="none" stroke="rgba(255,255,255,.12)" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" marker-end="url(#ah)"/>
  <path d="M115,120 L155,120 L155,160 L200,160" fill="none" stroke="rgba(255,255,255,.12)" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" marker-end="url(#ah)"/>
  <path d="M300,160 L380,160" fill="none" stroke="rgba(255,255,255,.12)" stroke-width="2" stroke-linecap="round" marker-end="url(#ah)"/>
  <path d="M320,42 L520,42 L520,96 L548,96" fill="none" stroke="rgba(255,255,255,.12)" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" marker-end="url(#ah)"/>
  <path d="M462,160 L548,118" fill="none" stroke="rgba(255,255,255,.12)" stroke-width="2" stroke-linecap="round" marker-end="url(#ah)"/>
  <path d="M300,176 C370,205 480,205 548,125" fill="none" stroke="rgba(255,255,255,.12)" stroke-width="2" stroke-linecap="round" marker-end="url(#ah)"/>
  <path d="M650,106 L720,106" fill="none" stroke="rgba(255,255,255,.12)" stroke-width="2" stroke-linecap="round" marker-end="url(#ah)"/>
  <path d="M830,106 L910,106" fill="none" stroke="rgba(255,255,255,.12)" stroke-width="2" stroke-linecap="round" marker-end="url(#ah)"/>
  <!-- flow particles (CSS animated — classes from custom.css flow system) -->
  <path class="flow flow-lg flow-d1" d="M115,100 L155,100 L155,42 L195,42"/>
  <path class="flow flow-lg flow-d2" d="M115,120 L155,120 L155,160 L200,160"/>
  <path class="flow flow-lg flow-d3" d="M300,160 L380,160"/>
  <path class="flow flow-lg flow-d4" d="M320,42 L520,42 L520,96 L548,96"/>
  <path class="flow flow-lg flow-d5" d="M462,160 L548,118"/>
  <path class="flow flow-lg flow-d6" d="M300,176 C370,205 480,205 548,125"/>
  <path class="flow flow-lg flow-d7" d="M650,106 L720,106"/>
  <path class="flow flow-lg flow-d8" d="M830,106 L910,106"/>
  <!-- nodes -->
  <rect x="10" y="86" width="105" height="44" rx="22" fill="url(#grB)"/>
  <text x="62" y="113" text-anchor="middle" fill="#fff" font-weight="700" font-size="12" letter-spacing=".5">Z-Prepass</text>
  <rect x="195" y="20" width="125" height="44" rx="22" fill="url(#grB)"/>
  <text x="257" y="47" text-anchor="middle" fill="#fff" font-weight="700" font-size="12" letter-spacing=".5">Shadows</text>
  <rect x="200" y="138" width="100" height="44" rx="22" fill="url(#grB)"/>
  <text x="250" y="165" text-anchor="middle" fill="#fff" font-weight="700" font-size="12" letter-spacing=".5">GBuffer</text>
  <rect x="380" y="138" width="82" height="44" rx="22" fill="url(#grO)"/>
  <text x="421" y="165" text-anchor="middle" fill="#fff" font-weight="700" font-size="12" letter-spacing=".5">SSAO</text>
  <rect x="548" y="82" width="102" height="50" rx="25" fill="url(#grO)"/>
  <text x="599" y="112" text-anchor="middle" fill="#fff" font-weight="700" font-size="12" letter-spacing=".5">Lighting</text>
  <rect x="720" y="84" width="110" height="44" rx="22" fill="url(#grG)"/>
  <text x="775" y="111" text-anchor="middle" fill="#fff" font-weight="700" font-size="12" letter-spacing=".5">PostProcess</text>
  <rect x="910" y="86" width="70" height="40" rx="20" fill="url(#grR)"/>
  <text x="945" y="111" text-anchor="middle" fill="#fff" font-weight="600" font-size="11" letter-spacing=".3">Present</text>
  <!-- edge labels -->
  <text x="155" y="68" text-anchor="middle" fill="rgba(255,255,255,.4)" font-size="9.5" font-style="italic" letter-spacing=".3">depth</text>
  <text x="155" y="145" text-anchor="middle" fill="rgba(255,255,255,.4)" font-size="9.5" font-style="italic" letter-spacing=".3">depth</text>
  <text x="340" y="152" text-anchor="middle" fill="rgba(255,255,255,.4)" font-size="9.5" font-style="italic" letter-spacing=".3">normals</text>
  <text x="420" y="34" text-anchor="middle" fill="rgba(255,255,255,.4)" font-size="9.5" font-style="italic" letter-spacing=".3">shadow map</text>
  <text x="505" y="130" text-anchor="middle" fill="rgba(255,255,255,.4)" font-size="9.5" font-style="italic" letter-spacing=".3">AO</text>
  <text x="420" y="205" text-anchor="middle" fill="rgba(255,255,255,.4)" font-size="9.5" font-style="italic" letter-spacing=".3">GBuffer MRTs</text>
  <text x="685" y="98" text-anchor="middle" fill="rgba(255,255,255,.4)" font-size="9.5" font-style="italic" letter-spacing=".3">HDR</text>
  <text x="870" y="98" text-anchor="middle" fill="rgba(255,255,255,.4)" font-size="9.5" font-style="italic" letter-spacing=".3">LDR</text>
</svg>
<div style="margin-top:.4em;">
  <span style="display:inline-block;font-size:.72em;opacity:.4;letter-spacing:.03em;padding:.25em .7em;">nodes = render passes &nbsp;·&nbsp; edges = resource dependencies &nbsp;·&nbsp; forks = GPU parallelism</span>
</div>
</div>

The GPU never sees this graph. It exists only on the CPU, long enough for the system to inspect **every** pass and **every** resource before a single GPU command is recorded. That global view is what makes automatic scheduling, memory aliasing, and barrier insertion possible — exactly the things that break when done by hand at scale.

Every frame follows a three-phase lifecycle:

<!-- 3-step lifecycle — distinct style from the DAG above -->
<div style="margin:.8em auto 1.2em;max-width:560px;">
  <div class="fg-lifecycle" style="display:flex;align-items:stretch;gap:0;border-radius:10px;overflow:hidden;border:1.5px solid rgba(var(--ds-indigo-rgb),.2);">
    <a href="#-the-declare-step" aria-label="Jump to Declare section" style="flex:1;padding:.7em .6em;text-align:center;background:rgba(var(--ds-info-rgb),.06);border-right:1px solid rgba(var(--ds-indigo-rgb),.12);text-decoration:none;color:inherit;transition:background .2s ease;cursor:pointer;" onmouseover="this.style.background='rgba(var(--ds-info-rgb),.14)'" onmouseout="this.style.background='rgba(var(--ds-info-rgb),.06)'">
      <div style="font-weight:800;font-size:.88em;letter-spacing:.04em;color:var(--ds-info-light);">①&ensp;DECLARE</div>
      <div style="font-size:.75em;opacity:.6;margin-top:.2em;">passes &amp; dependencies</div>
    </a>
    <span style="display:flex;align-items:center;flex-shrink:0;"><svg viewBox="0 0 28 20" width="20" height="14" fill="none"><line x1="2" y1="10" x2="17" y2="10" stroke="currentColor" stroke-width="2" stroke-linecap="round" opacity=".15"/><polyline points="15,4 24,10 15,16" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" fill="none" opacity=".35"/></svg></span>
    <a href="#-the-compile-step" aria-label="Jump to Compile section" style="flex:1;padding:.7em .6em;text-align:center;background:rgba(var(--ds-code-rgb),.06);border-right:1px solid rgba(var(--ds-indigo-rgb),.12);text-decoration:none;color:inherit;transition:background .2s ease;cursor:pointer;" onmouseover="this.style.background='rgba(var(--ds-code-rgb),.14)'" onmouseout="this.style.background='rgba(var(--ds-code-rgb),.06)'">
      <div style="font-weight:800;font-size:.88em;letter-spacing:.04em;color:var(--ds-code-light);">&ensp;COMPILE</div>
      <div style="font-size:.75em;opacity:.6;margin-top:.2em;">order · aliases · barriers</div>
    </a>
    <span style="display:flex;align-items:center;flex-shrink:0;"><svg viewBox="0 0 28 20" width="20" height="14" fill="none"><line x1="2" y1="10" x2="17" y2="10" stroke="currentColor" stroke-width="2" stroke-linecap="round" opacity=".15"/><polyline points="15,4 24,10 15,16" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" fill="none" opacity=".35"/></svg></span>
    <a href="#-the-execute-step" aria-label="Jump to Execute section" style="flex:1;padding:.7em .6em;text-align:center;background:rgba(var(--ds-success-rgb),.06);text-decoration:none;color:inherit;transition:background .2s ease;cursor:pointer;" onmouseover="this.style.background='rgba(var(--ds-success-rgb),.14)'" onmouseout="this.style.background='rgba(var(--ds-success-rgb),.06)'">
      <div style="font-weight:800;font-size:.88em;letter-spacing:.04em;color:var(--ds-success);">③&ensp;EXECUTE</div>
      <div style="font-size:.75em;opacity:.6;margin-top:.2em;">record GPU commands</div>
    </a>
  </div>
</div>

The three phases are distinct for a reason: declaration is lightweight and descriptive, compilation is where the graph is analyzed and optimized, and execution simply follows the plan. Let's break down what happens in each.

---

## 📋 The Declare Step

The declare step is pure CPU work — you're building a **description** of what this frame needs, not executing it. The key principle: separate *what to do* from *doing it*, because the compiler needs to see everything before it can optimize anything.

### 🔧 Registering a pass

A pass is a logical unit of GPU work — it might contain a single compute dispatch or hundreds of draw calls. To add one you give the graph two things:

<div style="margin:1em 0 1.2em;border-radius:10px;border:1px solid rgba(var(--ds-info-rgb),.12);background:rgba(var(--ds-info-rgb),.02);overflow:hidden;">

  <div style="display:grid;grid-template-columns:2.2em 1fr;gap:0 .8em;padding:.75em 1em;">
    <div style="grid-row:1/3;font-size:1em;color:var(--ds-info);opacity:.35;text-align:center;padding-top:.2em;">①</div>
    <div><span style="font-weight:700;color:var(--ds-info);font-size:.88em;letter-spacing:.02em;">SETUP CALLBACK</span></div>
    <div style="font-size:.84em;line-height:1.55;opacity:.6;">Runs <strong>now</strong>, on the CPU. Declares which resources the pass will touch and how (read, write, render target, UAV, etc.). This is where graph edges come from.</div>
  </div>

  <div style="border-top:1px solid rgba(var(--ds-info-rgb),.08);display:grid;grid-template-columns:2.2em 1fr;gap:0 .8em;padding:.75em 1em;">
    <div style="grid-row:1/3;font-size:1em;color:var(--ds-info);opacity:.35;text-align:center;padding-top:.2em;">②</div>
    <div><span style="font-weight:700;color:var(--ds-info);font-size:.88em;letter-spacing:.02em;">EXECUTE CALLBACK</span></div>
    <div style="font-size:.84em;line-height:1.55;opacity:.6;">Stored for <strong>later</strong>. Records actual GPU commands (draw calls, dispatches, copies) into a command list — only invoked during the execute phase, potentially on a worker thread.</div>
  </div>

</div>

The setup callback is where everything that matters for the compiler happens — read, write, and create declarations build the edges and resource descriptors that drive sorting, barriers, and aliasing. The execute callback is opaque to the compiler; it just gets invoked at the right time with the right resources bound.

### 📦 Virtual resources

When a pass creates a resource, the graph stores only a **descriptor** — dimensions, format, usage flags. No GPU memory is allocated. The resource is *virtual*: an opaque handle the compiler tracks, backed by nothing until the compile step decides where it lives in physical memory.

<div style="margin:.6em 0 1em;padding:.65em 1em;border-radius:8px;border:1px dashed rgba(var(--ds-indigo-rgb),.14);background:rgba(var(--ds-indigo-rgb),.02);display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:.5em;">
  <div>
    <span style="font-weight:700;font-size:.88em;">Handle #3</span>
    <span style="font-size:.78em;opacity:.45;margin-left:.5em;">1920×1080 · RGBA8 · render target</span>
  </div>
  <span style="font-size:.75em;color:var(--ds-warn);opacity:.7;font-weight:600;">description only — no GPU memory yet</span>
</div>

This is deliberate — the compiler needs to see **every** resource across the entire frame before it can decide which ones can share physical memory.

Virtual resources fall into two categories:

<div class="fg-grid-stagger ds-grid-2col">
  <div class="fg-hoverable ds-card ds-card--info">
    <div class="ds-card-header ds-card-header--info">🔀 Transient</div>
    <div class="ds-card-body">
      <strong>Lifetime:</strong> single frame — created and destroyed within the graph<br>
      <strong>Declared as:</strong> descriptor (size, format, usage flags)<br>
      <strong>GPU memory:</strong> allocated at compile, freed at frame end<br>
      <strong>Aliasable:</strong> <span style="color:var(--ds-success);font-weight:700;">Yes</span> — non-overlapping lifetimes share physical memory<br>
      <strong>Examples:</strong> GBuffer MRTs, SSAO scratch, bloom scratch
    </div>
  </div>
  <div class="fg-hoverable ds-card ds-card--code">
    <div class="ds-card-header ds-card-header--code">📌 Imported (external)</div>
    <div class="ds-card-body">
      <strong>Lifetime:</strong> spans multiple frames — owned by an external system<br>
      <strong>Declared as:</strong> existing GPU handle registered into the graph<br>
      <strong>GPU memory:</strong> already allocated; the graph only tracks state<br>
      <strong>Aliasable:</strong> <span style="color:var(--ds-danger);font-weight:700;">No</span> — lifetime extends beyond the frame<br>
      <strong>Examples:</strong> backbuffer, TAA history, shadow atlas, blue noise LUT
    </div>
  </div>
</div>

### 🔗 Reads, writes, and edges

Each read or write you declare in a setup callback forms a connection in the dependency graph:

<div style="margin:1em 0;padding:.85em 1.1em;border-radius:10px;border:1.5px solid rgba(var(--ds-info-rgb),.18);background:linear-gradient(135deg,rgba(var(--ds-info-rgb),.04),transparent);font-size:.88em;line-height:1.65;">
  <div style="display:grid;grid-template-columns:auto 1fr;gap:.3em .9em;align-items:start;">
    <span style="font-weight:700;color:var(--ds-info);">Read</span><span>Connects this pass to the last writer of a resource, specifying how the resource will be accessed.</span>
    <span style="font-weight:700;color:var(--ds-success);">Write</span><span>Advances the resource to a new version, making future reads depend on this pass instead of earlier writers, and defines the intended access.</span>
    <span style="font-weight:700;color:var(--ds-code);">Create</span><span>Introduces a new virtual resource, tracked by the graph but not yet backed by memory.</span>
  </div>
</div>

The mechanism behind these edges is **versioning**: every time a pass writes a resource, the version number increments. Readers attach to whatever version existed when they were declared. Multiple passes can read the same version without conflict, but only a write creates a new version — and a new dependency. Here's how that plays out across a real frame:

<div style="margin:1.2em 0;font-size:.85em;">
  <div style="border-radius:10px;overflow:hidden;border:1.5px solid rgba(var(--ds-indigo-rgb),.15);">
    <div style="padding:.5em .8em;background:rgba(var(--ds-indigo-rgb),.06);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);font-weight:700;font-size:.9em;text-align:center;">Resource versioning — HDR target through the frame</div>
    <div style="display:grid;grid-template-columns:auto auto 1fr;gap:0;">
      <div style="padding:.45em .6em;background:rgba(var(--ds-info-rgb),.06);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-weight:700;text-align:center;color:var(--ds-info-light);font-size:.82em;">v1</div>
      <div style="padding:.45em .6em;background:rgba(var(--ds-info-rgb),.12);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-weight:700;text-align:center;color:var(--ds-info-light);font-size:.75em;">WRITE</div>
      <div style="padding:.45em .8em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);font-size:.86em;">
        <span style="font-weight:700;">Lighting</span> — renders lit color into HDR target
      </div>
      <div style="padding:.35em .6em;background:rgba(var(--ds-info-rgb),.03);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-size:.7em;opacity:.4;text-align:center;">v1</div>
      <div style="padding:.35em .6em;background:rgba(var(--ds-code-rgb),.08);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-weight:600;text-align:center;color:var(--ds-code-light);font-size:.75em;">read</div>
      <div style="padding:.35em .8em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);font-size:.84em;opacity:.85;">
        <span style="font-weight:600;">Bloom</span> — samples bright pixels <span style="opacity:.4;font-size:.88em;">(still v1)</span>
      </div>
      <div style="padding:.35em .6em;background:rgba(var(--ds-info-rgb),.03);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-size:.7em;opacity:.4;text-align:center;">v1</div>
      <div style="padding:.35em .6em;background:rgba(var(--ds-code-rgb),.08);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-weight:600;text-align:center;color:var(--ds-code-light);font-size:.75em;">read</div>
      <div style="padding:.35em .8em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);font-size:.84em;opacity:.85;">
        <span style="font-weight:600;">Reflections</span> — samples for SSR <span style="opacity:.4;font-size:.88em;">(still v1)</span>
      </div>
      <div style="padding:.35em .6em;background:rgba(var(--ds-info-rgb),.03);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-size:.7em;opacity:.4;text-align:center;">v1</div>
      <div style="padding:.35em .6em;background:rgba(var(--ds-code-rgb),.08);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-weight:600;text-align:center;color:var(--ds-code-light);font-size:.75em;">read</div>
      <div style="padding:.35em .8em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);font-size:.84em;opacity:.85;">
        <span style="font-weight:600;">Fog</span> — reads scene color for aerial blending <span style="opacity:.4;font-size:.88em;">(still v1)</span>
      </div>
      <div style="padding:.45em .6em;background:rgba(var(--ds-success-rgb),.06);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-weight:700;text-align:center;color:var(--ds-success);font-size:.82em;">v2</div>
      <div style="padding:.45em .6em;background:rgba(var(--ds-success-rgb),.12);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-weight:700;text-align:center;color:var(--ds-success);font-size:.75em;">WRITE</div>
      <div style="padding:.45em .8em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);font-size:.86em;">
        <span style="font-weight:700;">Composite</span> — overwrites with final blended result <span style="opacity:.4;font-size:.88em;">(bumps to v2)</span>
      </div>
      <div style="padding:.35em .6em;background:rgba(var(--ds-success-rgb),.03);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-size:.7em;opacity:.4;text-align:center;">v2</div>
      <div style="padding:.35em .6em;background:rgba(var(--ds-code-rgb),.08);border-right:1px solid rgba(var(--ds-indigo-rgb),.08);font-weight:600;text-align:center;color:var(--ds-code-light);font-size:.75em;">read</div>
      <div style="padding:.35em .8em;font-size:.84em;opacity:.85;">
        <span style="font-weight:600;">Tonemap</span> — maps HDR → SDR for display <span style="opacity:.4;font-size:.88em;">(reads v2, not v1)</span>
      </div>
    </div>
  </div>
  <div style="margin-top:.4em;font-size:.82em;opacity:.6;">Reads never bump the version — three passes read v1 without conflict. Only a write creates v2. Tonemap depends on Composite (the v2 writer), with <strong>no edge</strong> to Lighting or any v1 reader.</div>
</div>

These versioned edges are the raw material the compiler works with. Every step that follows — sorting, culling, barrier insertion — operates on this edge set.

---

## ⚙ The Compile Step

The declared DAG goes in; an optimized execution plan comes out — all on the CPU, in microseconds.

<div style="margin:1.2em 0;border-radius:12px;overflow:hidden;border:1.5px solid rgba(var(--ds-code-rgb),.25);">
  <!-- INPUT -->
  <div style="padding:.7em 1.1em;background:rgba(var(--ds-code-rgb),.08);border-bottom:1px solid rgba(var(--ds-code-rgb),.15);display:flex;align-items:center;gap:.8em;">
    <span style="font-weight:800;font-size:.85em;color:var(--ds-code-light);text-transform:uppercase;letter-spacing:.04em;">📥 In</span>
    <span style="font-size:.88em;opacity:.8;">declared passes + virtual resources + read/write edges</span>
  </div>
  <!-- PIPELINE -->
  <div style="padding:.8em 1.3em;background:rgba(var(--ds-code-rgb),.03);">
    <div style="display:grid;grid-template-columns:auto 1fr;gap:.35em 1em;align-items:center;font-size:.88em;">
      <span style="font-weight:700;color:var(--ds-code-light);">①</span><span><strong>Sort</strong> passes into dependency order</span>
      <span style="font-weight:700;color:var(--ds-code);">②</span><span><strong>Cull</strong> passes whose outputs are never read</span>
      <span style="font-weight:700;color:var(--ds-code);">③</span><span><strong>Scan lifetimes</strong> — record each transient resource's first and last use</span>
      <span style="font-weight:700;color:var(--ds-code);">④</span><span><strong>Alias</strong> — assign non-overlapping resources to shared memory slots</span>
      <span style="font-weight:700;color:var(--ds-code-light);">⑤</span><span><strong>Compute barriers</strong> — insert transitions at every resource state change</span>
    </div>
  </div>
  <!-- OUTPUT -->
  <div style="padding:.7em 1.1em;background:rgba(var(--ds-success-rgb),.06);border-top:1px solid rgba(var(--ds-success-rgb),.15);display:flex;align-items:center;gap:.8em;">
    <span style="font-weight:800;font-size:.85em;color:var(--ds-success);text-transform:uppercase;letter-spacing:.04em;">📤 Out</span>
    <span style="font-size:.88em;opacity:.8;">ordered passes · aliased memory · barrier list · physical bindings</span>
  </div>
</div>

###  Sorting

Before the GPU can execute anything, the compiler needs to turn the DAG into an ordered schedule. The rule is simple: **no pass runs before the passes it depends on**. This is called a **topological sort**.

The algorithm most compilers use is **Kahn's algorithm**. Think of it like a to-do list where you can only start a task once all its prerequisites are done:

<div style="margin:1em 0;padding:.9em 1.1em;border-radius:10px;border:1.5px solid rgba(var(--ds-code-rgb),.18);background:linear-gradient(135deg,rgba(var(--ds-code-rgb),.04),transparent);font-size:.9em;line-height:1.7;">
  <div style="display:grid;grid-template-columns:auto 1fr;gap:.4em .9em;align-items:start;">
    <span style="font-weight:700;color:var(--ds-code-light);">①</span><span><strong>Find passes with no prerequisites.</strong> Look at every pass and count how many other passes feed into it. Any pass with a count of zero is ready to go — nothing is blocking it.</span>
    <span style="font-weight:700;color:var(--ds-code);">②</span><span><strong>Run one of those ready passes</strong> and add it to the output list. It's "done" now.</span>
    <span style="font-weight:700;color:var(--ds-code);">③</span><span><strong>Update the counts.</strong> For every pass that was waiting on the one we just ran, subtract one from its count. If that drops a pass to zero, it's now unblocked — add it to the ready queue.</span>
    <span style="font-weight:700;color:var(--ds-code);">④</span><span><strong>Repeat</strong> until nothing is left. If some passes never reached zero, there's a circular dependency — the graph is broken.</span>
  </div>
</div>

{{< interactive-toposort >}}

<div class="fg-reveal" style="margin:1em 0;padding:.85em 1.1em;border-radius:10px;border:1.5px solid rgba(var(--ds-code-rgb),.18);background:linear-gradient(135deg,rgba(var(--ds-code-rgb),.04),transparent);font-size:.9em;line-height:1.65;">
<strong>Sorting bonus — fewer state switches.</strong> Kahn's algorithm often has <strong>several passes ready at the same time</strong>, which gives the compiler freedom to choose among them. A sort-time heuristic can use that freedom to group passes that share GPU state, reducing the number of expensive context rolls. The exact strategy varies by engine and hardware, but the key insight is the same: a topological sort doesn't just guarantee correctness — it creates scheduling slack the compiler can exploit for performance.
</div>

### ✂ Culling

Once the sort gives us a valid execution order, the compiler can ask a powerful question: **does every pass actually contribute to the final image?**

In a hand-built renderer, you'd need to manually toggle passes with feature flags or `#ifdef` blocks. Miss one, and the GPU silently burns cycles on work nobody sees. The frame graph compiler does this automatically — it walks the DAG **backward** from the final output (usually the swapchain image) and marks every pass that contributes to it, directly or indirectly. Any pass that *isn't* on a path to the output gets removed.

This is the same idea as dead-code elimination in a regular compiler: if a function's return value is never used, the compiler strips it out. Here, if a render pass writes to a texture that no downstream pass ever reads, the entire pass — and its resource allocations — disappear.

**Why this matters in practice:**
- **Feature toggling is free.** Disable bloom by not reading its output, and the bloom pass plus its textures vanish automatically — no `if (bloomEnabled)` checks scattered through your code.
- **Debug passes cost nothing in release.** A visualization pass that only feeds a debug overlay gets culled the moment the overlay is turned off.
- **Artists and designers can experiment** with pass configurations without worrying about leftover GPU cost from unused passes.

The algorithm is simple: start from every output the frame needs (typically just the final composite), walk backward along edges, and flag each pass you visit as "alive." Anything not flagged is dead — skip it entirely.

{{< interactive-dag >}}

### 💾 Allocation and aliasing

The sorted order tells the compiler exactly when each resource is first written and last read — its **lifetime**. Two resources whose lifetimes don't overlap can share the same physical memory, even if they're completely different formats or sizes. The GPU allocates one large heap and places multiple resources at different offsets within it.

Without aliasing, every transient texture gets its own allocation for the entire frame — even if it's only alive for 2–3 passes. With aliasing, a GBuffer that's done by pass 3 and a bloom buffer that starts at pass 4 can sit in the same memory. Real-world deferred pipelines commonly see **40–50% transient VRAM reduction** once aliasing is enabled.

The allocator works in two passes: first, walk the sorted pass list and record each transient resource's first write and last read. Then scan resources in order of first-use — for each one, check if an existing heap block is free (its previous occupant has finished). If so, reuse it. If not, allocate a new block.

**A few things to watch out for:** aliased memory contains stale data, so the first use of any aliased resource must be a full clear or overwrite. Only transient (single-frame) resources qualify — imported resources that persist across frames can't be aliased. Placed resources must respect the GPU's alignment requirements (typically 64 KB), and the previous occupant must have finished all GPU access before new work touches the same memory.

{{< interactive-aliasing >}}

### 🚧 Barriers

A GPU resource can't be a render target and a shader input at the same time — the hardware needs to flush caches, change memory layout, and switch access modes between those uses. That transition is a **barrier**. Miss one and you get rendering corruption or a GPU crash; add an unnecessary one and the GPU stalls waiting for nothing.

Barriers follow the same rule as everything else in a frame graph: **compile analyzes and decides, execute submits and runs.** The compile stage is static analysis, the execute stage is command playback.

During compile, the compiler walks every pass in topological order and builds a per-resource usage timeline — which passes touch which resource, and in what state (color attachment, shader read, transfer destination, etc.). For each resource it tracks a `currentState`, starting at `Undefined`. Whenever a pass needs the resource in a different state, the compiler records a transition — say, `ColorAttachment → ShaderRead` when GBuffer's output becomes Lighting's input — and updates the current state. No GPU work happens; this is purely analysis over the declared reads and writes.

A concrete example: suppose GBuffer writes Albedo as a color attachment, then Lighting and PostProcess both read it as a shader resource. The compiler emits one barrier after GBuffer (`ColorAttachment → ShaderRead`) and nothing between Lighting and PostProcess — consecutive reads in the same state don't need a transition. A production compiler goes further: it merges multiple transitions into a single barrier call per pass, removes redundant transitions, and eliminates barriers for resources that are about to be aliased anyway.

The result is a `CompiledPlan` where each pass carries a list of pre-barriers alongside its execute callback. At execution time the loop is trivial — for each pass, submit its precomputed barriers (`vkCmdPipelineBarrier` / `ResourceBarrier`), begin the render pass, call the execute lambda, end the render pass. No graph walking, no state comparison, no decisions. The GPU receives exactly what was precomputed.

{{< interactive-barriers >}}

---

## ▶ The Execute Step

The plan is ready — now the GPU gets involved. Every decision has already been made during compile: pass order, memory layout, barriers, physical resource bindings. Execute just walks the plan.

This is deliberate. The entire point of the declare/compile split is to front-load all the analysis so that execution becomes a trivial loop — no graph traversal, no state comparisons, no dependency checks. The system iterates through the compiled pass list, submits the precomputed barriers for each pass (`vkCmdPipelineBarrier` in Vulkan, `ResourceBarrier` in D3D12), begins the render pass, invokes the execute lambda, and ends the render pass. That pattern repeats until every pass has been recorded into the command buffer.

<div class="diagram-box">
  <div class="db-title">▶ EXECUTE — recording GPU commands</div>
  <div class="db-body">
    <div class="diagram-pipeline">
      <div class="dp-stage">
        <div class="dp-title">FOR EACH PASS</div>
        <ul><li>submit precomputed barriers</li><li>begin render pass</li><li>call <code>execute()</code> lambda — draw calls, dispatches, copies</li><li>end render pass</li></ul>
      </div>
    </div>
    <div style="text-align:center;font-size:.82em;opacity:.6;margin-top:.3em">The only phase that touches the GPU API — resources already bound</div>
  </div>
</div>

Each execute lambda sees a fully resolved environment — barriers already computed and stored in the plan, memory already allocated, resources ready to bind. The lambda just records draw calls, dispatches, and copies. All the intelligence lives in the compile step.

### 🧵 Parallel command recording

The compiled plan doesn't just decide *what* runs — it reveals *what can run at the same time*. Because each lambda only touches its own declared resources and all barriers are precomputed, the engine knows exactly which passes are independent and can record them on separate CPU threads simultaneously.

<div style="margin:1.2em 0;border-radius:10px;overflow:hidden;border:1.5px solid rgba(var(--ds-code-rgb),.2);">
  <div style="padding:.55em 1em;background:rgba(var(--ds-code-rgb),.08);border-bottom:1px solid rgba(var(--ds-code-rgb),.12);font-weight:700;font-size:.85em;text-transform:uppercase;letter-spacing:.04em;color:var(--ds-code-light);">Parallel recording — conceptual flow</div>
  <div style="padding:.8em 1em;font-size:.88em;line-height:1.7;">
    <div style="display:grid;grid-template-columns:auto 1fr;gap:.35em .9em;align-items:start;">
      <span style="font-weight:700;color:var(--ds-code-light);">①</span><span>The compiled plan identifies <strong>groups</strong> of passes with no dependencies between them — they appear at the same depth in the sorted order.</span>
      <span style="font-weight:700;color:var(--ds-code);">②</span><span>Each independent pass is dispatched to a <strong>worker thread</strong>, which records GPU commands into its own secondary command buffer (Vulkan) or command list (D3D12).</span>
      <span style="font-weight:700;color:var(--ds-code);">③</span><span>Once all threads finish, the engine <strong>merges</strong> the recorded buffers into the primary command buffer in the correct order and submits to the GPU.</span>
    </div>
  </div>
</div>

The scalability gain comes from the DAG itself: passes at the same depth in the topological order have no edges between them, so recording them in parallel requires no additional synchronization. The more independent passes a frame has, the more CPU cores stay busy — and modern frames have plenty of independence (shadow cascades, GBuffer, SSAO, and bloom often share a depth level).

### 🧹 Cleanup and reset

After every pass has been recorded, cleanup is trivial. The frame graph was designed around single-frame lifetimes, so there's nothing to track individually — the system just resets the transient memory pool in one shot (every GBuffer, scratch texture, and temporary buffer vanishes together). Imported resources like the swapchain, TAA history, or shadow atlas aren't touched — they belong to external systems and persist across frames. The graph object itself clears its pass list and resource table, leaving it empty and ready for the next frame's declare phase to start fresh. This reset-and-rebuild cycle is what lets engines add or remove passes freely without any teardown logic.

---

## 🔄 Rebuild Strategies

How often should the graph recompile? Three approaches, each a valid tradeoff:

<div class="fg-grid-stagger" style="display:grid;grid-template-columns:repeat(3,1fr);gap:1em;margin:1.2em 0;">
  <div class="fg-hoverable" style="border-radius:10px;border:1.5px solid var(--ds-success);overflow:hidden;">
    <div style="padding:.5em .8em;background:rgba(var(--ds-success-rgb),.1);font-weight:800;font-size:.95em;border-bottom:1px solid rgba(var(--ds-success-rgb),.2);">
      🔄 Dynamic
    </div>
    <div style="padding:.7em .8em;font-size:.88em;line-height:1.6;">
      Rebuild every frame.<br>
      <strong>Cost:</strong> microseconds<br>
      <strong>Flexibility:</strong> total — passes can appear, disappear, or change every frame
    </div>
  </div>
  <div class="fg-hoverable" style="border-radius:10px;border:1.5px solid var(--ds-info);overflow:hidden;">
    <div style="padding:.5em .8em;background:rgba(var(--ds-info-rgb),.1);font-weight:800;font-size:.95em;border-bottom:1px solid rgba(var(--ds-info-rgb),.2);">
       Hybrid
    </div>
    <div style="padding:.7em .8em;font-size:.88em;line-height:1.6;">
      Cache compiled result, invalidate on change.<br>
      <strong>Cost:</strong> near-zero on hit<br>
      <strong>Flexibility:</strong> total, but requires dirty-tracking to know when to invalidate the cache
    </div>
  </div>
  <div class="fg-hoverable" style="border-radius:10px;border:1.5px solid var(--color-neutral-400,#9ca3af);overflow:hidden;">
    <div style="padding:.5em .8em;background:rgba(var(--ds-slate-rgb),.1);font-weight:800;font-size:.95em;border-bottom:1px solid rgba(var(--ds-slate-rgb),.2);">
      🔒 Static
    </div>
    <div style="padding:.7em .8em;font-size:.88em;line-height:1.6;">
      Compile once at init, replay forever.<br>
      <strong>Cost:</strong> zero<br>
      <strong>Flexibility:</strong> none — the pipeline is locked at startup<br>
      <span style="opacity:.6;font-size:.9em;">Rare in practice</span>
    </div>
  </div>
</div>

**Dynamic** is the simplest approach and the most common starting point. The compile cost is low — sorting, culling, aliasing, and barrier computation are all CPU-side integer work over small arrays — but it isn't zero. It scales with the number of passes and resources, and on CPU-constrained platforms (consoles, mobile) or graphs with hundreds of passes, the per-frame cost can become noticeable.

**Hybrid** exists precisely because of that cost. When the graph topology is mostly stable frame-to-frame — same passes, same connections — there's no reason to recompute the same plan 60 times per second. A hybrid approach caches the compiled result and only invalidates when the declared graph actually changes (typically detected by hashing the pass + resource set). The tradeoff is complexity: you need reliable dirty-tracking and must guarantee a stale plan is never replayed against a changed graph.

**Static** compiles once at init and replays the same plan forever. It's rarely useful because the whole point of a frame graph is flexibility — feature toggles, dynamic quality scaling, debug overlays. A locked pipeline can't adapt. It appears occasionally in fixed-function scenarios like a VR compositor that always runs the same few passes.

---

## 💰 The Payoff

<div class="fg-compare ds-grid-2col" style="gap:0;border-radius:10px;overflow:hidden;border:2px solid rgba(var(--ds-indigo-rgb),.25);box-shadow:0 2px 8px rgba(0,0,0,.08);">
  <div class="ds-card-header ds-card-header--danger" style="border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);">❌ Without Graph</div>
  <div class="ds-card-header ds-card-header--success">✅ With Graph</div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);background:rgba(var(--ds-danger-rgb),.02);">
    <strong>Memory aliasing</strong><br><span style="opacity:.65">Opt-in, fragile, rarely done</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);background:rgba(var(--ds-success-rgb),.02);">
    <strong>Memory aliasing</strong><br>Automatic — compiler sees all lifetimes. <strong style="color:var(--ds-success);">Significant VRAM savings</strong> <span style="font-size:.85em;opacity:.7">(topology-dependent)</span>
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);background:rgba(var(--ds-danger-rgb),.02);">
    <strong>Lifetimes</strong><br><span style="opacity:.65">Manual create/destroy, leaked or over-retained</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);background:rgba(var(--ds-success-rgb),.02);">
    <strong>Lifetimes</strong><br>Scoped to first..last use. Zero waste.
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);background:rgba(var(--ds-danger-rgb),.02);">
    <strong>Barriers</strong><br><span style="opacity:.65">Manual, per-pass</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);background:rgba(var(--ds-success-rgb),.02);">
    <strong>Barriers</strong><br>Precomputed at compile from declared read/write
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);background:rgba(var(--ds-danger-rgb),.02);">
    <strong>Pass reordering</strong><br><span style="opacity:.65">Breaks silently</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);background:rgba(var(--ds-success-rgb),.02);">
    <strong>Pass reordering</strong><br>Safe — compiler respects dependencies
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);background:rgba(var(--ds-danger-rgb),.02);">
    <strong>Pass culling</strong><br><span style="opacity:.65">Manual ifdef / flag checks</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);background:rgba(var(--ds-success-rgb),.02);">
    <strong>Pass culling</strong><br>Automatic — unused outputs = dead pass
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);background:rgba(var(--ds-danger-rgb),.02);">
    <strong>Context rolls</strong><br><span style="opacity:.65">Hard-coded pass order — unnecessary state switches</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);background:rgba(var(--ds-success-rgb),.02);">
    <strong>Context rolls</strong><br>Sort heuristic groups compatible passes — <strong style="color:var(--ds-success);">fewer switches</strong> <span style="font-size:.85em;opacity:.7">(gain varies by topology)</span>
  </div>

  <!-- Advanced features divider -->
  <div style="grid-column:1/-1;padding:.4em 1em;background:rgba(var(--ds-indigo-rgb),.06);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.1);text-align:center;">
    <span style="font-size:.75em;text-transform:uppercase;letter-spacing:.06em;font-weight:700;opacity:.5;">Advanced — covered in <a href="../frame-graph-advanced/" style="color:var(--ds-info);text-decoration:none;border-bottom:1px dashed rgba(var(--ds-info-rgb),.4);">Part III</a></span>
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);background:rgba(var(--ds-danger-rgb),.02);opacity:.55;">
    <strong>Async compute</strong><br><span style="opacity:.65">Manual queue sync</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;background:rgba(var(--ds-success-rgb),.02);opacity:.55;">
    <strong>Async compute</strong><br>Compiler schedules independent passes across queues
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);background:rgba(var(--ds-danger-rgb),.02);opacity:.55;">
    <strong>Split barriers</strong><br><span style="opacity:.65">Manual begin/end placement</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;background:rgba(var(--ds-success-rgb),.02);opacity:.55;">
    <strong>Split barriers</strong><br>Compiler overlaps flushes with unrelated work
  </div>
</div>

---

### 🔮 What's next

That's the full theory — sorting, culling, barriers, aliasing — everything a frame graph compiler does. [Part II — Build It](../frame-graph-build-it/) turns every concept from this article into running C++, three iterations from blank file to a working `FrameGraph` class with automatic barriers and memory aliasing.

---

<div class="ds-article-footer" style="justify-content:flex-end;">
  <a href="../frame-graph-build-it/">
    Next: Part II — Build It →
  </a>
</div>
