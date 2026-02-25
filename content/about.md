---
title: "About"
description: "Personal notes on real-time rendering and software engineering."
date: 2026-02-09
showDate: false
showReadingTime: false
showWordCount: false
---

<!-- Animations -->
<style>
/* Hero */
.about-hero {
}
.about-hero::before {
  content: '';
  position: absolute; inset: -1.5px; border-radius: 15px;
  background: linear-gradient(90deg, transparent 15%, rgba(var(--ds-accent-rgb),.3) 50%, transparent 85%);
  background-size: 200% 100%;
  animation: shimmer 5s ease-in-out infinite;
  pointer-events: none;
  mask: linear-gradient(#fff 0 0) content-box, linear-gradient(#fff 0 0);
  mask-composite: exclude; -webkit-mask-composite: xor;
  padding: 1.5px;
}
/* Topic cards */
.about-card {
  transition: transform .22s ease, box-shadow .22s ease, border-color .22s ease;
}
.about-card:hover {
  transform: translateY(-3px);
  box-shadow: 0 6px 24px rgba(var(--ds-accent-rgb),.12);
  border-color: rgba(var(--ds-accent-rgb),.4);
  background: rgba(var(--ds-accent-rgb),.06);
}
.about-card .card-icon {
  display: inline-block;
  transition: transform .25s ease;
}
.about-card:hover .card-icon {
  transform: scale(1.2);
}

/* Author card */
.about-author {
}
.about-author::before {
  content: '';
  position: absolute; left: 0; top: 12%; bottom: 12%;
  width: 3px; border-radius: 2px;
  background: var(--ds-accent);
}

/* Timeline entries */
.tl-entry {
}

/* Pulsing current-role dot — uses pseudo-element so box-shadow doesn’t shift the dot */
.tl-dot-active {
  position: relative;
}
.tl-dot-active::after {
  content: '';
  position: absolute; inset: -3px;
  border-radius: 50%;
}

/* Skill tags */
.tl-tags span {
}

/* Current badge */
.badge-current {
  position: relative;
}
.badge-current::after {
  content: '';
  position: absolute; inset: -2px; border-radius: 7px;
  background: rgba(var(--ds-highlight-rgb),.12);
}

/* Section headings — warm underline accent */
.article-content h2 {
  position: relative;
  padding-bottom: .35em;
}
.article-content h2::after {
  content: '';
  position: absolute; left: 0; bottom: 0;
  width: 2.5em; height: 2.5px; border-radius: 2px;
  background: var(--ds-accent);
  opacity: .7;
}

/* Disclaimer */
.about-disclaimer {
}

</style>

<!-- Hero intro -->
<div class="about-hero" style="position:relative;margin:0 0 2em;padding:1.6em 1.8em;border-radius:14px;border:1.5px solid rgba(var(--ds-accent-rgb),.2);background:rgba(var(--ds-accent-rgb),.04);line-height:1.7;overflow:hidden;">
  <div style="font-size:1.3em;font-weight:800;margin-bottom:.35em;background:linear-gradient(90deg,var(--ds-soft),var(--ds-accent),var(--ds-highlight));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text;display:inline-block;">Deep Spark</div>
  <div style="font-size:1em;">
    A compact notebook for <strong>engine design</strong>, <strong>GPU/CPU/memory profiling</strong>, <strong>PBR</strong>, <strong>ray tracing</strong>, and <strong>practical performance work</strong>.
  </div>
</div>

---

## About the Author

<div class="about-author" style="position:relative;margin:1.2em 0 1.5em;padding:1.3em 1.5em 1.3em 1.8em;border-radius:12px;border:1.5px solid rgba(var(--ds-accent-rgb),.18);background:rgba(var(--ds-accent-rgb),.03);line-height:1.7;overflow:hidden;">
  <div style="font-size:1.12em;font-weight:800;margin-bottom:.4em;color:var(--ds-highlight);">Pawel Stolecki</div>
  <div style="font-size:.95em;">
    3D graphics software engineer focused on real-time rendering. I enjoy the intersection where visual-quality research meets hands-on optimization — pushing shading closer to ground truth while keeping systems efficient and shippable.
  </div>
</div>

### Experience

<div style="position:relative;margin:1.4em 0 2em;padding-left:2.2em;border-left:3px solid rgba(var(--ds-accent-rgb),.25);">

  <!-- CD PROJEKT RED -->
  <div class="tl-entry">
    <div class="tl-dot tl-dot-active"><div class="tl-dot__inner"></div></div>
    <div style="display:flex;align-items:baseline;gap:.6em;flex-wrap:wrap;">
      <span class="tl-company">CD PROJEKT RED</span>
      <span class="badge-current">Current</span>
    </div>
    <div class="tl-role">Rendering Engineer</div>
    <div class="tl-body">
      Driving lighting technology improvements. The work is hands-on and investigative — prototype a change, measure it on hardware, trace what the GPU, CPU, memory is actually doing, and ship what moves the needle.
    </div>
    <div class="tl-tags">
      <span class="ds-tag ds-tag--accent">Direct Lighting</span>
      <span class="ds-tag ds-tag--accent">Indirect Lighting</span>
      <span class="ds-tag ds-tag--highlight">GPU/CPU Profiling</span>
      <span class="ds-tag ds-tag--highlight">Memory Profiling</span>
      <span class="ds-tag ds-tag--warm">Platform Optimization</span>
      <span class="ds-tag ds-tag--warm">Cross-Platform</span>
    </div>
  </div>

  <!-- Techland -->
  <div class="tl-entry">
    <div class="tl-dot tl-dot-past"><div class="tl-dot__inner"></div></div>
    <div class="tl-company">Techland</div>
    <div class="tl-role">Rendering | Technical Artist</div>
    <div class="tl-body">
      Worked across the full rasterization pipeline — GBuffer generation, lighting, and post-processing — while scaling the renderer to ship on everything from handhelds to high-end PC. Began as a Technical Artist, which built a lasting focus on artist-facing tools, visual debugging, and workflows that keep content creators unblocked.
    </div>
    <div class="tl-tags">
      <span class="ds-tag ds-tag--accent">GBuffer Fill</span>
      <span class="ds-tag ds-tag--accent">Lighting</span>
      <span class="ds-tag ds-tag--highlight">Post-Processing</span>
      <span class="ds-tag ds-tag--highlight">Multi-Platform Scalability</span>
      <span class="ds-tag ds-tag--warm">Artist Tooling</span>
      <span class="ds-tag ds-tag--warm">Visual Debugging</span>
    </div>
  </div>

  <!-- The Farm 51 -->
  <div class="tl-entry">
    <div class="tl-dot tl-dot-early"><div class="tl-dot__inner"></div></div>
    <div class="tl-company">The Farm 51</div>
    <div class="tl-role">Technical Artist</div>
    <div class="tl-body">
      Built procedural shader systems for environmental effects — forest fire, stormy ocean, wind-driven foliage — alongside a landscape production pipeline and general-purpose shader library. Owned R&D for simulation systems and drove performance optimization across the board.
    </div>
    <div class="tl-tags">
      <span class="ds-tag ds-tag--accent">Procedural Shaders</span>
      <span class="ds-tag ds-tag--accent">Wind Simulation</span>
      <span class="ds-tag ds-tag--highlight">Landscape Pipeline</span>
      <span class="ds-tag ds-tag--highlight">Ocean Rendering</span>
      <span class="ds-tag ds-tag--warm">R&amp;D</span>
      <span class="ds-tag ds-tag--warm">Optimization</span>
    </div>
  </div>

</div>

---

<div class="about-disclaimer" style="margin-top:1em;padding:.9em 1.2em;border-radius:8px;border:1px solid rgba(var(--ds-accent-rgb),.08);background:rgba(var(--ds-accent-rgb),.02);font-size:.82em;line-height:1.6;opacity:.6;">
<strong>Disclaimer:</strong> All content on this site reflects my personal opinions and experiences only. Nothing here represents the views, positions, or endorsements of any company I currently work for or have previously worked for.
</div>



