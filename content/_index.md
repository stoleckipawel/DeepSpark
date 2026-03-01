---title: ""
description: "Graphics software engineering: real-time rendering notes, GPU experiments, and performance analysis."
keywords: ["real-time rendering", "GPU programming", "frame graph", "rendering architecture", "Vulkan", "D3D12", "UE5 RDG"]
---

## Rendering Architecture Series

<div class="sc ds-reveal">
<div class="sc-header">
<div>
<div class="sc-title">Frame Graph</div>
<div class="sc-subtitle">4 parts · theory → implementation → beyond MVP → production engines</div>
</div>
<div class="sc-tags">
<span class="sc-tag">rendering</span>
<span class="sc-tag">gpu</span>
<span class="sc-tag">architecture</span>
</div>
</div>
<div class="sc-parts">
<a class="sc-part" href="posts/frame-graph-theory/">
<div class="sc-num" style="background:var(--ds-accent);">I</div>
<div class="sc-body">
<div class="sc-part-title">Theory</div>
<div class="sc-desc">The theory behind frame graphs: how a DAG of passes and resources automates scheduling, barriers, and memory aliasing.</div>
</div>
<span class="sc-read">12 min read</span>
</a>
<a class="sc-part" href="posts/frame-graph-build-it/">
<div class="sc-num" style="background:var(--ds-warm);">II</div>
<div class="sc-body">
<div class="sc-part-title">Build It</div>
<div class="sc-desc">Three iterations from blank file to working frame graph with automatic barriers and memory aliasing.</div>
</div>
<span class="sc-read">30 min read</span>
</a>
<a class="sc-part" href="posts/frame-graph-advanced/">
<div class="sc-num" style="background:var(--ds-info);">III</div>
<div class="sc-body">
<div class="sc-part-title">Beyond MVP</div>
<div class="sc-desc">Async compute and split barriers: how the compiler squeezes more performance from the same DAG.</div>
</div>
<span class="sc-read">10 min read</span>
</a>
<a class="sc-part" href="posts/frame-graph-production/">
<div class="sc-num" style="background:var(--ds-highlight);">IV</div>
<div class="sc-body">
<div class="sc-part-title">Production Engines</div>
<div class="sc-desc">How production engines implement frame graphs at scale.</div>
</div>
<span class="sc-read">9 min read</span>
</a>
</div>
<div class="sc-progress">
<div class="sc-bar"><div class="sc-bar-fill"></div></div>
<span class="sc-count">4 / 4</span>
</div>
</div>

---

## All Posts
