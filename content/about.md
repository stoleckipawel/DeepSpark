---
title: "About"
description: "Personal notes on real-time rendering and software engineering."
date: 2026-02-09
showDate: false
showReadingTime: false
showWordCount: false
---

<!-- Hero intro -->
<div style="margin:0 0 2em;padding:1.4em 1.6em;border-radius:14px;border:1.5px solid rgba(99,102,241,.18);background:linear-gradient(135deg,rgba(99,102,241,.05),rgba(34,197,94,.03));line-height:1.7;">
  <div style="font-size:1.25em;font-weight:800;margin-bottom:.3em;">LightThreads</div>
  <div style="font-size:1em;">
    A compact notebook for <strong>engine design</strong>, <strong>GPU/CPU/memory profiling</strong>, <strong>PBR</strong>, <strong>ray tracing</strong>, and <strong>practical performance work</strong>.
  </div>
</div>

---

## What I Write About

<div style="display:grid;grid-template-columns:repeat(auto-fill,minmax(260px,1fr));gap:.8em;margin:1.2em 0 2em;">
  <div style="padding:.85em 1em;border-radius:10px;border:1px solid rgba(99,102,241,.15);background:rgba(99,102,241,.03);">
    <div style="font-size:1.15em;margin-bottom:.25em;">🛠️</div>
    <div style="font-weight:700;font-size:.92em;margin-bottom:.2em;">Rendering Engine Design</div>
    <div style="font-size:.84em;opacity:.7;line-height:1.5;">APIs, modular architecture, maintainability, and pragmatic release strategies.</div>
  </div>
  <div style="padding:.85em 1em;border-radius:10px;border:1px solid rgba(99,102,241,.15);background:rgba(99,102,241,.03);">
    <div style="font-size:1.15em;margin-bottom:.25em;">⚙️</div>
    <div style="font-weight:700;font-size:.92em;margin-bottom:.2em;">Engine Systems & Trade-offs</div>
    <div style="font-size:.84em;opacity:.7;line-height:1.5;">High-performance game systems and real-time application architecture.</div>
  </div>
  <div style="padding:.85em 1em;border-radius:10px;border:1px solid rgba(99,102,241,.15);background:rgba(99,102,241,.03);">
    <div style="font-size:1.15em;margin-bottom:.25em;">⚡</div>
    <div style="font-weight:700;font-size:.92em;margin-bottom:.2em;">GPU-Aware Performance</div>
    <div style="font-size:.84em;opacity:.7;line-height:1.5;">Memory layout, bandwidth vs compute trade-offs, batching, and caching.</div>
  </div>
  <div style="padding:.85em 1em;border-radius:10px;border:1px solid rgba(99,102,241,.15);background:rgba(99,102,241,.03);">
    <div style="font-size:1.15em;margin-bottom:.25em;">🎨</div>
    <div style="font-weight:700;font-size:.92em;margin-bottom:.2em;">PBR & Light Transport</div>
    <div style="font-size:.84em;opacity:.7;line-height:1.5;">Useful patterns for physically based rendering, sampling, and denoising.</div>
  </div>
  <div style="padding:.85em 1em;border-radius:10px;border:1px solid rgba(99,102,241,.15);background:rgba(99,102,241,.03);">
    <div style="font-size:1.15em;margin-bottom:.25em;">✨</div>
    <div style="font-weight:700;font-size:.92em;margin-bottom:.2em;">Ray Tracing Pipelines</div>
    <div style="font-size:.84em;opacity:.7;line-height:1.5;">Getting quality where it matters without breaking the frame budget.</div>
  </div>
  <div style="padding:.85em 1em;border-radius:10px;border:1px solid rgba(99,102,241,.15);background:rgba(99,102,241,.03);">
    <div style="font-size:1.15em;margin-bottom:.25em;">🔍</div>
    <div style="font-weight:700;font-size:.92em;margin-bottom:.2em;">Profiling & Shader Tuning</div>
    <div style="font-size:.84em;opacity:.7;line-height:1.5;">Small tooling that makes performance work repeatable and measurable.</div>
  </div>
</div>

---

## About the Author

<div style="margin:1.2em 0 1.5em;padding:1.3em 1.5em;border-radius:12px;border:1.5px solid rgba(99,102,241,.18);background:linear-gradient(135deg,rgba(99,102,241,.04),transparent);line-height:1.7;">
  <div style="font-size:1.1em;font-weight:800;margin-bottom:.4em;">Paweł Stołecki</div>
  <div style="font-size:.95em;">
    3D graphics software engineer focused on real-time rendering. I enjoy the intersection where visual-quality research meets hands-on optimization — pushing shading closer to ground truth while keeping systems efficient and shippable.
  </div>
</div>

### Experience

<div style="position:relative;margin:1.4em 0 2em;padding-left:2.2em;border-left:3px solid rgba(99,102,241,.2);">

  <!-- CD PROJEKT RED -->
  <div style="margin-bottom:2.2em;">
    <div style="position:absolute;left:-0.8em;width:1.4em;height:1.4em;border-radius:50%;background:#6366f1;display:flex;align-items:center;justify-content:center;">
      <div style="width:.5em;height:.5em;border-radius:50%;background:#fff;"></div>
    </div>
    <div style="display:flex;align-items:baseline;gap:.6em;flex-wrap:wrap;">
      <span style="font-weight:800;font-size:1.08em;">CD PROJEKT RED</span>
      <span style="font-size:.78em;padding:.18em .6em;border-radius:5px;background:rgba(34,197,94,.12);color:#22c55e;font-weight:600;">Current</span>
    </div>
    <div style="font-size:.82em;opacity:.55;margin:.15em 0 .5em;">Rendering Engineer</div>
    <div style="font-size:.92em;line-height:1.7;">
      Driving lighting technology improvements. The work is hands-on and investigative — prototype a change, measure it on hardware, trace what the GPU, CPU, memory is actually doing, and ship what moves the needle.
    </div>
    <div style="display:flex;flex-wrap:wrap;gap:.4em;margin-top:.6em;">
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(59,130,246,.18);background:rgba(59,130,246,.06);color:rgba(59,130,246,.85);font-weight:600;">Direct Lighting</span>
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(59,130,246,.18);background:rgba(59,130,246,.06);color:rgba(59,130,246,.85);font-weight:600;">Indirect Lighting</span>
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(139,92,246,.18);background:rgba(139,92,246,.06);color:rgba(139,92,246,.85);font-weight:600;">GPU/CPU Profiling</span>
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(139,92,246,.18);background:rgba(139,92,246,.06);color:rgba(139,92,246,.85);font-weight:600;">Memory Profiling</span>
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(34,197,94,.18);background:rgba(34,197,94,.06);color:rgba(34,197,94,.85);font-weight:600;">Platform Optimization</span>
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(34,197,94,.18);background:rgba(34,197,94,.06);color:rgba(34,197,94,.85);font-weight:600;">Cross-Platform</span>
    </div>
  </div>

  <!-- Techland -->
  <div>
    <div style="position:absolute;left:-0.8em;width:1.4em;height:1.4em;border-radius:50%;background:rgba(99,102,241,.3);display:flex;align-items:center;justify-content:center;">
      <div style="width:.5em;height:.5em;border-radius:50%;background:#fff;"></div>
    </div>
    <div style="font-weight:800;font-size:1.08em;">Techland</div>
    <div style="font-size:.82em;opacity:.55;margin:.15em 0 .5em;">Rendering | Technical Artist</div>
    <div style="font-size:.92em;line-height:1.7;">
      Worked across the full rasterization pipeline — GBuffer generation, lighting, and post-processing — while scaling the renderer to ship on everything from handhelds to high-end PC. Began as a Technical Artist, which built a lasting focus on artist-facing tools, visual debugging, and workflows that keep content creators unblocked.
    </div>
    <div style="display:flex;flex-wrap:wrap;gap:.4em;margin-top:.6em;">
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(59,130,246,.18);background:rgba(59,130,246,.06);color:rgba(59,130,246,.85);font-weight:600;">GBuffer Fill</span>
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(59,130,246,.18);background:rgba(59,130,246,.06);color:rgba(59,130,246,.85);font-weight:600;">Lighting</span>
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(139,92,246,.18);background:rgba(139,92,246,.06);color:rgba(139,92,246,.85);font-weight:600;">Post-Processing</span>
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(139,92,246,.18);background:rgba(139,92,246,.06);color:rgba(139,92,246,.85);font-weight:600;">Multi-Platform Scalability</span>
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(34,197,94,.18);background:rgba(34,197,94,.06);color:rgba(34,197,94,.85);font-weight:600;">Artist Tooling</span>
      <span style="font-size:.75em;padding:.2em .55em;border-radius:5px;border:1px solid rgba(34,197,94,.18);background:rgba(34,197,94,.06);color:rgba(34,197,94,.85);font-weight:600;">Visual Debugging</span>
    </div>
  </div>

</div>

---

<div style="margin-top:1em;padding:.9em 1.2em;border-radius:8px;border:1px solid rgba(156,163,175,.15);background:rgba(156,163,175,.03);font-size:.82em;line-height:1.6;opacity:.65;">
<strong>Disclaimer:</strong> All content on this site reflects my personal opinions and experiences only. Nothing here represents the views, positions, or endorsements of any company I currently work for or have previously worked for.
</div>



