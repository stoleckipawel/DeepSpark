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

## 🎯 Why You Want One

<div style="margin:1.5em 0;border-radius:14px;overflow:hidden;border:1.5px solid rgba(var(--ds-indigo-rgb),.18);box-shadow:0 2px 16px rgba(0,0,0,.06);">
  <!-- Header gradient bar -->
  <div style="height:4px;background:linear-gradient(90deg,var(--ds-danger),var(--ds-warn),var(--ds-success));"></div>
  <!-- Rows -->
  <div style="display:grid;grid-template-columns:1fr auto 1fr;gap:0;">
    <!-- Headers -->
    <div style="padding:.6em 1em;font-size:.72em;text-transform:uppercase;letter-spacing:.06em;font-weight:700;color:var(--ds-danger);background:rgba(var(--ds-danger-rgb),.04);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);border-right:1px solid rgba(var(--ds-indigo-rgb),.06);">❌ Manual</div>
    <div style="padding:.6em .5em;background:rgba(var(--ds-indigo-rgb),.02);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);border-right:1px solid rgba(var(--ds-indigo-rgb),.06);"></div>
    <div style="padding:.6em 1em;font-size:.72em;text-transform:uppercase;letter-spacing:.06em;font-weight:700;color:var(--ds-success);background:rgba(var(--ds-success-rgb),.04);border-bottom:1px solid rgba(var(--ds-indigo-rgb),.08);">✅ Frame Graph</div>
    <!-- Row 1 — Execution order -->
    <div style="padding:.65em 1em;font-size:.88em;opacity:.5;text-decoration:line-through;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);border-right:1px solid rgba(var(--ds-indigo-rgb),.06);text-align:right;">
      Passes run in whatever order you wrote them.
    </div>
    <div style="padding:.65em .3em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);border-right:1px solid rgba(var(--ds-indigo-rgb),.06);display:flex;align-items:center;justify-content:center;">
      <svg viewBox="0 0 24 16" width="20" height="13" fill="none"><path d="M4 8h12m-4-4l5 4-5 4" stroke="var(--ds-success)" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" opacity=".5"/></svg>
    </div>
    <div style="padding:.65em 1em;font-size:.88em;font-weight:700;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);">
      Auto-sorted — dependency order, zero ordering bugs.
    </div>
    <!-- Row 2 — Barriers -->
    <div style="padding:.65em 1em;font-size:.88em;opacity:.5;text-decoration:line-through;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);border-right:1px solid rgba(var(--ds-indigo-rgb),.06);text-align:right;">
      Every resource transition tracked and placed by hand.
    </div>
    <div style="padding:.65em .3em;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);border-right:1px solid rgba(var(--ds-indigo-rgb),.06);display:flex;align-items:center;justify-content:center;">
      <svg viewBox="0 0 24 16" width="20" height="13" fill="none"><path d="M4 8h12m-4-4l5 4-5 4" stroke="var(--ds-success)" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" opacity=".5"/></svg>
    </div>
    <div style="padding:.65em 1em;font-size:.88em;font-weight:700;border-bottom:1px solid rgba(var(--ds-indigo-rgb),.06);">
      Barriers derived and batched from declared reads &amp; writes.
    </div>
    <!-- Row 3 — Memory aliasing -->
    <div style="padding:.65em 1em;font-size:.88em;opacity:.5;text-decoration:line-through;border-right:1px solid rgba(var(--ds-indigo-rgb),.06);text-align:right;">
      Every resource gets its own allocation for the full frame.
    </div>
    <div style="padding:.65em .3em;border-right:1px solid rgba(var(--ds-indigo-rgb),.06);display:flex;align-items:center;justify-content:center;">
      <svg viewBox="0 0 24 16" width="20" height="13" fill="none"><path d="M4 8h12m-4-4l5 4-5 4" stroke="var(--ds-success)" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" opacity=".5"/></svg>
    </div>
    <div style="padding:.65em 1em;font-size:.88em;font-weight:700;color:var(--ds-success);">
      Non-overlapping lifetimes share memory — significant VRAM savings.
    </div>
  </div>
  <!-- Footer -->
  <div style="padding:.55em 1em;background:rgba(var(--ds-indigo-rgb),.04);border-top:1px solid rgba(var(--ds-indigo-rgb),.08);text-align:center;font-size:.88em;opacity:.65;">
    You describe <em>what</em> each pass needs — the graph figures out the <em>how</em>.
  </div>
</div>

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

<div class="fg-reveal" style="position:relative;margin:1.4em 0;padding-left:2.2em;border-left:3px solid var(--color-neutral-300,#d4d4d4);">

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

The separation is deliberate: declaration is cheap, compilation is where the optimization happens, and execution is a straight walk through an already-optimal plan. Let's look at each phase.

---

## 📋 The Declare Step

Each frame starts on the CPU. You register passes, describe the resources they need, and declare who reads or writes what. No GPU work happens yet — you're building a description of the frame.

<div style="margin:1em 0 1.2em;border-radius:10px;border:1px solid rgba(var(--ds-info-rgb),.12);background:rgba(var(--ds-info-rgb),.02);overflow:hidden;">

  <div style="display:grid;grid-template-columns:2.2em 1fr;gap:0 .8em;padding:.75em 1em;">
    <div style="grid-row:1/3;font-size:1em;color:var(--ds-info);opacity:.35;text-align:center;padding-top:.2em;">●</div>
    <div><span style="font-weight:700;color:var(--ds-info);font-size:.88em;letter-spacing:.02em;">REGISTER PASSES</span></div>
    <div style="font-size:.84em;line-height:1.55;opacity:.6;">Tell the graph <em>what work</em> this frame needs — each pass gets a setup callback to declare resources and an execute callback for later GPU recording. &ensp;<code style="font-size:.9em;">addPass(setup, execute)</code></div>
  </div>

  <div style="border-top:1px solid rgba(var(--ds-info-rgb),.08);display:grid;grid-template-columns:2.2em 1fr;gap:0 .8em;padding:.75em 1em;">
    <div style="grid-row:1/3;font-size:1em;color:var(--ds-info);opacity:.35;text-align:center;padding-top:.2em;">●</div>
    <div><span style="font-weight:700;color:var(--ds-info);font-size:.88em;letter-spacing:.02em;">DESCRIBE RESOURCES</span></div>
    <div style="font-size:.84em;line-height:1.55;opacity:.6;">Declare every texture and buffer a pass will touch — size, format, usage — without allocating GPU memory. Everything stays virtual. &ensp;<code style="font-size:.9em;">create({1920,1080, RGBA8})</code></div>
  </div>

  <div style="border-top:1px solid rgba(var(--ds-info-rgb),.08);display:grid;grid-template-columns:2.2em 1fr;gap:0 .8em;padding:.75em 1em;">
    <div style="grid-row:1/3;font-size:1em;color:var(--ds-info);opacity:.35;text-align:center;padding-top:.2em;">●</div>
    <div><span style="font-weight:700;color:var(--ds-info);font-size:.88em;letter-spacing:.02em;">CONNECT READS &amp; WRITES</span></div>
    <div style="font-size:.84em;line-height:1.55;opacity:.6;">Wire up the edges: <em>this pass reads that texture, that pass writes this buffer.</em> These connections drive execution order, barriers, and memory aliasing. &ensp;<code style="font-size:.9em;">read(h)</code> / <code style="font-size:.9em;">write(h)</code></div>
  </div>

</div>
<div style="text-align:center;font-size:.78em;opacity:.4;">CPU only — the GPU is idle during this phase</div>

What does a "virtual resource" actually look like at this point? Just a lightweight descriptor and a handle — no GPU memory behind it yet:

<div style="margin:.6em 0 1em;padding:.65em 1em;border-radius:8px;border:1px dashed rgba(var(--ds-indigo-rgb),.14);background:rgba(var(--ds-indigo-rgb),.02);display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:.5em;">
  <div>
    <span style="font-weight:700;font-size:.88em;">Handle #3</span>
    <span style="font-size:.78em;opacity:.45;margin-left:.5em;">1920×1080 · RGBA8 · render target</span>
  </div>
  <span style="font-size:.75em;color:var(--ds-warn);opacity:.7;font-weight:600;">description only — no GPU memory yet</span>
</div>

### 📦 Transient vs. imported

When you declare a resource, the graph needs to know one thing: **does it live inside this frame, or does it come from outside?**

<div class="fg-grid-stagger ds-grid-2col">
  <div class="fg-hoverable ds-card ds-card--info">
    <div class="ds-card-header ds-card-header--info">🔀 Transient</div>
    <div class="ds-card-body">
      <strong>Lifetime:</strong> single frame<br>
      <strong>Declared as:</strong> description (size, format)<br>
      <strong>GPU memory:</strong> allocated and aliased at compile<br>
      <strong>Aliasable:</strong> <span style="color:var(--ds-success);font-weight:700;">Yes</span> — non-overlapping lifetimes share physical memory<br>
      <strong>Examples:</strong> GBuffer MRTs, SSAO scratch, bloom scratch
    </div>
  </div>
  <div class="fg-hoverable ds-card ds-card--code">
    <div class="ds-card-header ds-card-header--code">📌 Imported</div>
    <div class="ds-card-body">
      <strong>Lifetime:</strong> across frames<br>
      <strong>Declared as:</strong> existing GPU handle<br>
      <strong>GPU memory:</strong> already allocated externally<br>
      <strong>Aliasable:</strong> <span style="color:var(--ds-danger);font-weight:700;">No</span> — lifetime extends beyond the frame<br>
      <strong>Examples:</strong> backbuffer, TAA history, shadow atlas, blue noise LUT
    </div>
  </div>
</div>

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

### 🔗 How edges form — resource versioning

Before the compiler can sort anything, it needs edges. Edges come from **resource versioning**: every time a pass writes a resource, the version number increments. Readers attach to whatever version existed when they were declared. Multiple passes can read the same version without conflict — only a write creates a new one.

This is how the graph knows *exactly* which pass depends on which, even when the same resource is written more than once per frame:

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

### 📊 Sorting

With edges in place, the compiler flattens the DAG into a linear execution order. Every edge says *"pass B depends on something pass A produced,"* so the compiler needs a **topological sort** — an ordering where every pass comes after the passes it depends on.

The standard approach is **Kahn's algorithm**:

<div style="margin:1em 0;padding:.9em 1.1em;border-radius:10px;border:1.5px solid rgba(var(--ds-code-rgb),.18);background:linear-gradient(135deg,rgba(var(--ds-code-rgb),.04),transparent);font-size:.9em;line-height:1.7;">
  <div style="display:grid;grid-template-columns:auto 1fr;gap:.4em .9em;align-items:start;">
    <span style="font-weight:700;color:var(--ds-code-light);">①</span><span><strong>Count</strong> incoming edges for every pass. A pass with zero incoming edges has no unmet dependencies.</span>
    <span style="font-weight:700;color:var(--ds-code);">②</span><span><strong>Emit</strong> any zero-count pass — its dependencies are already satisfied.</span>
    <span style="font-weight:700;color:var(--ds-code);">③</span><span><strong>Decrement</strong> the counts of its successors. If a successor hits zero, it's now ready — add it to the queue.</span>
    <span style="font-weight:700;color:var(--ds-code);">④</span><span><strong>Repeat</strong> until the queue drains. If fewer passes come out than went in, there's a cycle — the graph is invalid.</span>
  </div>
</div>

The whole walk is **O(V + E)** — linear in passes and edges. It runs once per frame and finishes in microseconds.

<div class="fg-reveal" style="margin:1em 0;padding:.85em 1.1em;border-radius:10px;border:1.5px solid rgba(var(--ds-code-rgb),.18);background:linear-gradient(135deg,rgba(var(--ds-code-rgb),.04),transparent);font-size:.9em;line-height:1.65;">
<strong>Sorting bonus — fewer context rolls.</strong> A <em>context roll</em> happens every time the GPU switches render targets: caches flush, attachments rebind, and the pipeline drains. In a naAZve hand-ordered renderer those switches are whatever order you hard-coded. With Kahn's algorithm the compiler often has <strong>multiple passes at zero in-degree simultaneously</strong> — it can pick the one that targets the <em>same</em> render target as the previous pass. That one tie-breaking heuristic groups passes by attachment and can cut render-target switches by 30–50%, turning a <em>correctness</em> tool (topological sort) into a <em>performance</em> tool (state-change minimiser).
</div>

{{< interactive-toposort >}}

### ✂ Culling

With the sorted order established, the compiler walks backward from the final outputs and removes any pass whose results are never read. Dead-code elimination for GPU work — entire passes vanish without a feature flag.

{{< interactive-dag >}}

### 💾 Allocation and aliasing

The sorted order tells the compiler exactly when each resource is first written and last read — its **lifetime**. Two resources that are never alive at the same time can share the same physical memory. Without aliasing, every transient resource holds its own allocation for the full frame — even if it's only used for 2–3 passes. The actual savings depend on pass topology, resolution, and how many transient resources have non-overlapping lifetimes — Frostbite's GDC 2017 talk reported roughly 50% transient VRAM reduction on Battlefield 1's deferred pipeline at full resolution. Your mileage will vary with different pass structures.

The allocator is a two-step process:

<div style="margin:1em 0;padding:.9em 1.1em;border-radius:10px;border:1.5px solid rgba(var(--ds-code-rgb),.18);background:linear-gradient(135deg,rgba(var(--ds-code-rgb),.04),transparent);font-size:.9em;line-height:1.7;">
  <div style="display:grid;grid-template-columns:auto 1fr;gap:.4em .9em;align-items:start;">
    <span style="font-weight:700;color:var(--ds-code-light);">①</span><span><strong>Scan lifetimes</strong> — walk the sorted pass list and record each transient resource's first and last use. Imported resources are excluded (externally owned, live across frames).</span>
    <span style="font-weight:700;color:var(--ds-code);">②</span><span><strong>Free-list scan</strong> — sort resources by first-use. For each one, try to fit it into an existing physical block whose previous user has finished. Fit → reuse that block. No fit → allocate a new one. This is <strong>greedy interval coloring</strong> — O(R log R) for the sort, then linear for the scan.</span>
  </div>
</div>

<div style="margin:1em 0;border-radius:12px;overflow:hidden;border:1.5px solid rgba(var(--ds-code-rgb),.2);">
  <!-- Timeline -->
  <div style="padding:.8em 1.2em;">
    <div style="display:grid;grid-template-columns:100px repeat(6,1fr);gap:2px;font-size:.72em;opacity:.45;margin-bottom:.3em;">
      <div></div>
      <div style="text-align:center;">Pass 1</div>
      <div style="text-align:center;">Pass 2</div>
      <div style="text-align:center;">Pass 3</div>
      <div style="text-align:center;">Pass 4</div>
      <div style="text-align:center;">Pass 5</div>
      <div style="text-align:center;">Pass 6</div>
    </div>
    <div style="display:grid;grid-template-columns:100px repeat(6,1fr);gap:2px;margin-bottom:3px;">
      <div style="font-size:.8em;font-weight:700;display:flex;align-items:center;">GBuffer</div>
      <div style="background:rgba(var(--ds-code-rgb),.2);border-radius:4px 0 0 4px;height:24px;"></div>
      <div style="background:rgba(var(--ds-code-rgb),.2);height:24px;"></div>
      <div style="background:rgba(var(--ds-code-rgb),.2);border-radius:0 4px 4px 0;height:24px;"></div>
      <div style="height:24px;"></div>
      <div style="height:24px;"></div>
      <div style="height:24px;"></div>
    </div>
    <div style="display:grid;grid-template-columns:100px repeat(6,1fr);gap:2px;margin-bottom:.5em;">
      <div style="font-size:.8em;font-weight:700;display:flex;align-items:center;">Bloom</div>
      <div style="height:24px;"></div>
      <div style="height:24px;"></div>
      <div style="height:24px;"></div>
      <div style="background:rgba(var(--ds-code-rgb),.2);border-radius:4px 0 0 4px;height:24px;"></div>
      <div style="background:rgba(var(--ds-code-rgb),.2);height:24px;"></div>
      <div style="background:rgba(var(--ds-code-rgb),.2);border-radius:0 4px 4px 0;height:24px;"></div>
    </div>
    <div style="border-top:1px solid rgba(var(--ds-code-rgb),.1);padding-top:.5em;display:flex;align-items:center;gap:.8em;">
      <span style="font-size:.8em;opacity:.5;">No overlap →</span>
      <span style="padding:.25em .65em;border-radius:6px;background:rgba(var(--ds-success-rgb),.1);color:var(--ds-success);font-weight:700;font-size:.82em;">same heap, two resources</span>
    </div>
  </div>

  <!-- How it works -->
  <div style="padding:.7em 1.2em;font-size:.88em;line-height:1.7;border-top:1px solid rgba(var(--ds-code-rgb),.08);">
    The graph allocates a large <code>ID3D12Heap</code> (or <code>VkDeviceMemory</code>) and <strong>places</strong> multiple resources at different offsets within it. This is the single biggest VRAM win the graph provides.
  </div>

  <!-- Pitfalls -->
  <div style="padding:.7em 1.2em;background:rgba(var(--ds-warn-rgb),.04);border-top:1px solid rgba(var(--ds-warn-rgb),.12);">
    <div style="font-size:.78em;font-weight:700;text-transform:uppercase;letter-spacing:.03em;color:var(--ds-warn-dark);margin-bottom:.4em;">⚠ Pitfalls</div>
    <div style="display:grid;grid-template-columns:auto 1fr;gap:.2em .8em;font-size:.85em;line-height:1.6;">
      <span style="font-weight:700;opacity:.7;">Garbage</span><span>Aliased memory has stale contents — first use must be a full clear or overwrite</span>
      <span style="font-weight:700;opacity:.7;">Transient only</span><span>Imported resources live across frames — only single-frame transients qualify</span>
      <span style="font-weight:700;opacity:.7;">Alignment</span><span>Placed resources must respect the GPU's placement alignment (commonly 64 KB) — sizes and offsets must be rounded up, or two resources that look like they fit will overlap at the hardware level</span>
      <span style="font-weight:700;opacity:.7;">Sync</span><span>The old resource must finish all GPU access before the new one touches the same memory</span>
    </div>
  </div>

  <!-- Production tricks -->
  <div style="padding:.7em 1.2em;background:rgba(var(--ds-code-rgb),.03);border-top:1px solid rgba(var(--ds-code-rgb),.08);">
    <div style="font-size:.78em;font-weight:700;text-transform:uppercase;letter-spacing:.03em;opacity:.45;margin-bottom:.4em;">Production optimizations</div>
    <div style="display:grid;grid-template-columns:auto 1fr;gap:.2em .8em;font-size:.85em;line-height:1.6;">
      <span style="font-weight:700;opacity:.7;">🪣 Bucketing</span><span>Round sizes to power-of-two (4, 8, 16 MB…) — fewer distinct sizes means heaps are reusable across resources</span>
      <span style="font-weight:700;opacity:.7;">♻ Pooling</span><span>Keep heaps across frames. Next frame's <code>compile()</code> pulls from the pool — allocation cost drops to near zero</span>
    </div>
  </div>

  <!-- Part IV link -->
  <div style="padding:.45em 1.2em;background:rgba(var(--ds-code-rgb),.06);border-top:1px solid rgba(var(--ds-code-rgb),.1);font-size:.8em;opacity:.6;text-align:center;">
    <a href="../frame-graph-production/">Part IV</a> covers how production engines implement these strategies.
  </div>
</div>

{{< interactive-aliasing >}}

### 🚧 Barriers

A GPU resource can't be a render target and a shader input at the same time — the hardware needs to flush caches, change memory layout, and switch access modes between those uses. That transition is a **barrier**. Miss one and you get rendering corruption or a GPU crash; add an unnecessary one and the GPU stalls waiting for nothing.

Barriers follow the same rule as everything else in a frame graph: **compile analyzes and decides, execute submits and runs.** Think of it like a compiler — the compile stage is static analysis, the execute stage is command playback. Barriers are no different.

#### ⚙ What happens during compile (barrier computation)

The compiler analyzes the graph and builds the full transition plan. No GPU work — purely data analysis.

<div style="margin:1.2em 0;border-radius:10px;overflow:hidden;border:1.5px solid rgba(var(--ds-code-rgb),.2);">
  <!-- Step 1 -->
  <div style="padding:.7em 1.1em;border-bottom:1px solid rgba(var(--ds-code-rgb),.1);background:rgba(var(--ds-code-rgb),.04);">
    <div style="font-weight:800;font-size:.82em;color:var(--ds-code-light);margin-bottom:.3em;">① Build resource usage timeline</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      Every pass declared its reads and writes. The compiler walks those declarations and builds a per-resource usage history:
    </div>
    <pre style="margin:.5em 0 0;padding:.5em .8em;border-radius:6px;background:rgba(0,0,0,.04);font-size:.82em;line-height:1.5;overflow-x:auto;"><code>Resource: Albedo
  Pass 0 (GBuffer):   Write → ColorAttachment
  Pass 1 (Lighting):  Read  → ShaderRead
  Pass 2 (PostFX):    Read  → ShaderRead</code></pre>
  </div>
  <!-- Step 2 -->
  <div style="padding:.7em 1.1em;border-bottom:1px solid rgba(var(--ds-code-rgb),.1);">
    <div style="font-weight:800;font-size:.82em;color:var(--ds-code-light);margin-bottom:.3em;">① Track previous state</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      For each resource, start with <code>currentState = Undefined</code>, then walk passes in topological order.
    </div>
  </div>
  <!-- Step 3 -->
  <div style="padding:.7em 1.1em;border-bottom:1px solid rgba(var(--ds-code-rgb),.1);background:rgba(var(--ds-code-rgb),.04);">
    <div style="font-weight:800;font-size:.82em;color:var(--ds-code);margin-bottom:.3em;">③ Detect state transitions</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      At each pass, check what the pass needs vs. what the resource currently is:
    </div>
    <pre style="margin:.5em 0 0;padding:.5em .8em;border-radius:6px;background:rgba(0,0,0,.04);font-size:.82em;line-height:1.5;overflow-x:auto;"><code>if (requiredState != currentState):
    store barrier { resource, currentState → requiredState }
    currentState = requiredState</code></pre>
    <div style="font-size:.84em;line-height:1.6;opacity:.75;margin-top:.5em;">
      Example — GBuffer writes Albedo as <code>ColorAttachment</code>, then Lighting reads it as <code>ShaderRead</code>. The compiler stores:<br>
      <strong style="color:var(--ds-code-light)">Barrier: ColorAttachment → ShaderRead</strong><br>
      This is purely analysis. Nothing is sent to the GPU yet.
    </div>
  </div>
  <!-- Step 4 -->
  <div style="padding:.7em 1.1em;border-bottom:1px solid rgba(var(--ds-code-rgb),.1);">
    <div style="font-weight:800;font-size:.82em;color:var(--ds-code-light);margin-bottom:.3em;">① Optimize (production)</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      A production compiler will merge multiple transitions into one barrier call, batch transitions per pass, remove redundant transitions, and eliminate transitions for resources that are about to be aliased. Still compile-time — still no GPU work.
    </div>
  </div>
  <!-- Step 5 -->
  <div style="padding:.7em 1.1em;background:rgba(var(--ds-success-rgb),.04);">
    <div style="font-weight:800;font-size:.82em;color:var(--ds-code-light);margin-bottom:.3em;">⑤ Store in CompiledPlan</div>
    <div style="font-size:.86em;line-height:1.6;opacity:.85;">
      The output is a list of precomputed barriers per pass — stored alongside the execution callbacks:
    </div>
    <pre style="margin:.5em 0 0;padding:.5em .8em;border-radius:6px;background:rgba(0,0,0,.04);font-size:.82em;line-height:1.5;overflow-x:auto;"><code>CompiledPass {
    preBarriers[]       // transitions before this pass runs
    executeCallback     // the lambda — draw calls, dispatches
}</code></pre>
    <div style="font-size:.84em;line-height:1.6;opacity:.75;margin-top:.4em;">
      The frame graph is now fully "lowered" into a GPU execution plan. Every barrier is decided. Execute just replays it.
    </div>
  </div>
</div>

#### ▶ What happens during execute (barrier submission)

Execution is intentionally simple. It replays the compiled plan — no analysis, no graph walking, no state comparison, no decisions:

<pre style="margin:.6em 0;padding:.7em 1em;border-radius:8px;background:rgba(var(--ds-success-rgb),.04);border:1px solid rgba(var(--ds-success-rgb),.15);font-size:.84em;line-height:1.6;overflow-x:auto;"><code>for pass in compiledPlan:
    submit(pass.preBarriers)       // vkCmdPipelineBarrier / ResourceBarrier
    beginRenderPass()
    pass.executeCallback()         // draw calls, dispatches
    endRenderPass()</code></pre>

Just issuing commands. The GPU receives exactly what was precomputed.

#### 📌 Concrete example

<div class="ds-grid-2col" style="gap:0;border-radius:10px;overflow:hidden;border:1.5px solid rgba(var(--ds-indigo-rgb),.2);">
  <div style="padding:.8em 1em;background:rgba(var(--ds-code-rgb),.04);border-right:1px solid rgba(var(--ds-indigo-rgb),.15);">
    <div style="font-weight:800;font-size:.82em;color:var(--ds-code-light);margin-bottom:.5em;">COMPILE generates:</div>
    <div style="font-size:.82em;line-height:1.65;font-family:ui-monospace,monospace;">
      <strong>GBuffer</strong><br>
      &ensp;pre: Undefined → ColorAttachment<br><br>
      <strong>Lighting</strong><br>
      &ensp;pre: ColorAttachment → ShaderRead<br><br>
      <strong>PostProcess</strong><br>
      &ensp;pre: ColorAttachment → ShaderRead
    </div>
    <div style="font-size:.78em;opacity:.5;margin-top:.5em;">All stored in CompiledPlan.</div>
  </div>
  <div style="padding:.8em 1em;background:rgba(var(--ds-success-rgb),.04);">
    <div style="font-weight:800;font-size:.82em;color:var(--ds-success);margin-bottom:.5em;">EXECUTE submits:</div>
    <div style="font-size:.82em;line-height:1.65;font-family:ui-monospace,monospace;">
      vkCmdPipelineBarrier(…)<br>
      vkCmdBeginRenderPass(…)<br>
      &ensp;draw…<br>
      vkCmdEndRenderPass()<br><br>
      vkCmdPipelineBarrier(…)<br>
      vkCmdBeginRenderPass(…)<br>
      &ensp;draw…<br>
      vkCmdEndRenderPass()
    </div>
    <div style="font-size:.78em;opacity:.5;margin-top:.5em;">Exactly as precomputed.</div>
  </div>
</div>

{{< interactive-barriers >}}

---

## ▶ The Execute Step

The plan is ready — now the GPU gets involved. Every decision has already been made during compile: pass order, memory layout, barriers, physical resource bindings. Execute just walks the plan.

<div class="diagram-box">
  <div class="db-title">▶ EXECUTE — recording GPU commands</div>
  <div class="db-body">
    <div class="diagram-pipeline">
      <div class="dp-stage">
        <div class="dp-title">RUN PASSES</div>
        <ul><li>for each pass in compiled order:</li><li>submit precomputed barriers → call <code>execute()</code></li></ul>
      </div>
      <div class="dp-stage">
        <div class="dp-title">CLEANUP</div>
        <ul><li>release transients (or pool them)</li><li>reset the frame allocator</li></ul>
      </div>
    </div>
    <div style="text-align:center;font-size:.82em;opacity:.6;margin-top:.3em">The only phase that touches the GPU API — resources already bound</div>
  </div>
</div>

<div class="fg-reveal" style="margin:1.2em 0;padding:1em 1.2em;border-radius:10px;border:1.5px solid rgba(var(--ds-success-rgb),.2);background:rgba(var(--ds-success-rgb),.04);font-size:.92em;line-height:1.6;">
  Each execute lambda sees a <strong>fully resolved environment</strong> — barriers already computed and stored in the plan, memory already allocated, resources ready to bind. The lambda just records draw calls, dispatches, and copies. All the intelligence lives in the compile step.
</div>

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
      <strong>Flex:</strong> full — passes appear/disappear freely<br>
      <span style="opacity:.6;font-size:.9em;">Used by: production engines</span>
    </div>
  </div>
  <div class="fg-hoverable" style="border-radius:10px;border:1.5px solid var(--ds-info);overflow:hidden;">
    <div style="padding:.5em .8em;background:rgba(var(--ds-info-rgb),.1);font-weight:800;font-size:.95em;border-bottom:1px solid rgba(var(--ds-info-rgb),.2);">
       Hybrid
    </div>
    <div style="padding:.7em .8em;font-size:.88em;line-height:1.6;">
      Cache compiled result, invalidate on change.<br>
      <strong>Cost:</strong> near-zero on hit<br>
      <strong>Flex:</strong> full + bookkeeping<br>
      <span style="opacity:.6;font-size:.9em;">Used by: UE5</span>
    </div>
  </div>
  <div class="fg-hoverable" style="border-radius:10px;border:1.5px solid var(--color-neutral-400,#9ca3af);overflow:hidden;">
    <div style="padding:.5em .8em;background:rgba(var(--ds-slate-rgb),.1);font-weight:800;font-size:.95em;border-bottom:1px solid rgba(var(--ds-slate-rgb),.2);">
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

  <div style="padding:.55em .8em;font-size:.88em;border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);background:rgba(var(--ds-danger-rgb),.02);">
    <strong>Async compute</strong><br><span style="opacity:.65">Manual queue sync</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;background:rgba(var(--ds-success-rgb),.02);">
    <strong>Async compute</strong><br>Compiler schedules across queues
  </div>

  <div style="padding:.55em .8em;font-size:.88em;border-top:1px solid rgba(var(--ds-indigo-rgb),.1);border-right:1.5px solid rgba(var(--ds-indigo-rgb),.15);background:rgba(var(--ds-danger-rgb),.02);">
    <strong>Context rolls</strong><br><span style="opacity:.65">Hard-coded pass order — frequent render-target switches</span>
  </div>
  <div style="padding:.55em .8em;font-size:.88em;border-top:1px solid rgba(var(--ds-indigo-rgb),.1);background:rgba(var(--ds-success-rgb),.02);">
    <strong>Context rolls</strong><br>Compiler groups passes by attachment — <strong style="color:var(--ds-success);">30–50% fewer RT switches</strong>
  </div>
</div>

<div class="fg-reveal" style="margin:1.2em 0;padding:.8em 1em;border-radius:8px;background:linear-gradient(135deg,rgba(var(--ds-success-rgb),.06),rgba(var(--ds-info-rgb),.06));border:1px solid rgba(var(--ds-success-rgb),.2);font-size:.92em;line-height:1.6;">
🏭 <strong>Not theoretical.</strong> Frostbite's <a href="https://www.gdcvault.com/play/1024612/FrameGraph-Extensible-Rendering-Architecture-in">GDC 2017 talk</a> reported ~50% transient VRAM reduction on Battlefield 1's deferred pipeline — a topology with many short-lived fullscreen targets at high resolution. Actual savings depend on pass count, resolution, and how many transient lifetimes overlap. UE5's RDG ships the same optimization today — every <code>FRDGTexture</code> marked as transient goes through the aliasing pipeline we build in <a href="../frame-graph-build-it/">Part II</a>.
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
