---title: ""
description: "Graphics software engineering: real-time rendering notes, GPU experiments, and performance analysis."
keywords: ["real-time rendering", "GPU programming", "frame graph", "rendering architecture", "Vulkan", "D3D12", "UE5 RDG"]
---

<style>
/* ── Series card ──────────────────────────────────────────────── */
.posts-hero {
  position: relative;
}
.posts-hero::before {
  content: '';
  position: absolute; inset: -1.5px; border-radius: 15px;
  background: linear-gradient(90deg, transparent 25%, rgba(var(--ds-accent-rgb),.2) 50%, transparent 75%);
  background-size: 200% 100%;
  animation: shimmer 4s ease-in-out infinite;
  pointer-events: none;
  mask: linear-gradient(#fff 0 0) content-box, linear-gradient(#fff 0 0);
  mask-composite: exclude; -webkit-mask-composite: xor;
  padding: 1.5px;
}
.series-card {
  transition: transform .2s ease, box-shadow .2s ease;
}
.series-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 8px 24px rgba(var(--ds-accent-rgb),.1);
}
.part-link {
  transition: transform .18s ease, border-color .18s ease, box-shadow .18s ease;
  text-decoration: none !important;
  color: inherit !important;
  display: block;
}
.part-link:hover {
  transform: translateY(-2px);
  border-color: rgba(var(--ds-accent-rgb),.35) !important;
  box-shadow: 0 4px 16px rgba(var(--ds-accent-rgb),.08);
}
.part-link:hover .part-title {
  color: var(--ds-accent);
}
.part-num {
  transition: transform .2s ease, background .2s ease;
}
.part-link:hover .part-num {
  transform: scale(1.1);
  background: var(--ds-accent) !important;
}

@media (prefers-reduced-motion: reduce) {
  *, *::before, *::after {
    animation-duration: 0.01ms !important;
    animation-delay: 0ms !important;
    transition-duration: 0.01ms !important;
  }
}
</style>

## Rendering Architecture Series

<div class="series-card" style="margin:1.2em 0 2em;border-radius:12px;border:1.5px solid rgba(var(--ds-accent-rgb),.18);overflow:hidden;background:linear-gradient(135deg,rgba(var(--ds-accent-rgb),.03),transparent);">
<div style="padding:1em 1.3em;border-bottom:1px solid rgba(var(--ds-accent-rgb),.1);display:flex;align-items:center;gap:.8em;flex-wrap:wrap;">
<span style="font-size:1.3em;">🏗️</span>
<div>
<div style="font-weight:800;font-size:1.05em;">Frame Graph</div>
<div style="font-size:.82em;opacity:.55;">4 parts · theory → implementation → beyond MVP → production engines</div>
</div>
<div style="margin-left:auto;display:flex;gap:.4em;">
<span style="font-size:.72em;padding:.2em .5em;border-radius:5px;border:1px solid rgba(var(--ds-accent-rgb),.15);background:rgba(var(--ds-accent-rgb),.05);font-weight:600;">rendering</span>
<span style="font-size:.72em;padding:.2em .5em;border-radius:5px;border:1px solid rgba(var(--ds-accent-rgb),.15);background:rgba(var(--ds-accent-rgb),.05);font-weight:600;">gpu</span>
<span style="font-size:.72em;padding:.2em .5em;border-radius:5px;border:1px solid rgba(var(--ds-accent-rgb),.15);background:rgba(var(--ds-accent-rgb),.05);font-weight:600;">architecture</span>
</div>
</div>
<div style="display:grid;gap:0;">
<div class="part-link" onclick="window.location.href='posts/frame-graph-theory/'" style="cursor:pointer;padding:.9em 1.3em;border-bottom:1px solid rgba(var(--ds-accent-rgb),.07);display:flex;align-items:center;gap:1em;">
<div class="part-num" style="width:2em;height:2em;border-radius:50%;background:var(--ds-accent);display:flex;align-items:center;justify-content:center;color:#fff;font-weight:800;font-size:.85em;flex-shrink:0;">I</div>
<div style="flex:1;min-width:0;">
<div class="part-title" style="font-weight:700;font-size:.95em;transition:color .18s ease;">Theory</div>
<div style="font-size:.82em;opacity:.6;line-height:1.5;margin-top:.1em;">The theory behind frame graphs: how a DAG of passes and resources automates scheduling, barriers, and memory aliasing.</div>
</div>
<div style="font-size:.78em;opacity:.4;flex-shrink:0;">12 min read</div>
</div>
<div class="part-link" onclick="window.location.href='posts/frame-graph-build-it/'" style="cursor:pointer;padding:.9em 1.3em;border-bottom:1px solid rgba(var(--ds-accent-rgb),.07);display:flex;align-items:center;gap:1em;">
<div class="part-num" style="width:2em;height:2em;border-radius:50%;background:var(--ds-warm);display:flex;align-items:center;justify-content:center;color:#fff;font-weight:800;font-size:.85em;flex-shrink:0;">II</div>
<div style="flex:1;min-width:0;">
<div class="part-title" style="font-weight:700;font-size:.95em;transition:color .18s ease;">Build It</div>
<div style="font-size:.82em;opacity:.6;line-height:1.5;margin-top:.1em;">Three iterations from blank file to working frame graph with automatic barriers and memory aliasing.</div>
</div>
<div style="font-size:.78em;opacity:.4;flex-shrink:0;">30 min read</div>
</div>
<div class="part-link" onclick="window.location.href='posts/frame-graph-advanced/'" style="cursor:pointer;padding:.9em 1.3em;border-bottom:1px solid rgba(var(--ds-accent-rgb),.07);display:flex;align-items:center;gap:1em;">
<div class="part-num" style="width:2em;height:2em;border-radius:50%;background:var(--ds-info);display:flex;align-items:center;justify-content:center;color:#fff;font-weight:800;font-size:.85em;flex-shrink:0;">III</div>
<div style="flex:1;min-width:0;">
<div class="part-title" style="font-weight:700;font-size:.95em;transition:color .18s ease;">Beyond MVP</div>
<div style="font-size:.82em;opacity:.6;line-height:1.5;margin-top:.1em;">Async compute and split barriers: how the compiler squeezes more performance from the same DAG.</div>
</div>
<div style="font-size:.78em;opacity:.4;flex-shrink:0;">10 min read</div>
</div>
<div class="part-link" onclick="window.location.href='posts/frame-graph-production/'" style="cursor:pointer;padding:.9em 1.3em;display:flex;align-items:center;gap:1em;">
<div class="part-num" style="width:2em;height:2em;border-radius:50%;background:var(--ds-highlight);display:flex;align-items:center;justify-content:center;color:#fff;font-weight:800;font-size:.85em;flex-shrink:0;">IV</div>
<div style="flex:1;min-width:0;">
<div class="part-title" style="font-weight:700;font-size:.95em;transition:color .18s ease;">Production Engines</div>
<div style="font-size:.82em;opacity:.6;line-height:1.5;margin-top:.1em;">How production engines implement frame graphs at scale.</div>
</div>
<div style="font-size:.78em;opacity:.4;flex-shrink:0;">9 min read</div>
</div>
</div>
<div style="padding:.6em 1.3em;border-top:1px solid rgba(var(--ds-accent-rgb),.1);background:rgba(var(--ds-accent-rgb),.02);display:flex;align-items:center;gap:.6em;">
<div style="width:100%;height:4px;border-radius:2px;background:rgba(var(--ds-accent-rgb),.08);overflow:hidden;">
<div style="width:100%;height:100%;border-radius:2px;background:linear-gradient(90deg,var(--ds-accent),var(--ds-warm),var(--ds-info),var(--ds-highlight));"></div>
</div>
<span style="font-size:.75em;opacity:.45;white-space:nowrap;font-weight:600;">4 / 4</span>
</div>
</div>

---

## All Posts
