---
title: "Frame Graph — Production Engines"
date: 2026-02-11
draft: true
description: "How UE5, Frostbite, and Unity implement frame graphs at scale — plus the upgrade roadmap from MVP to production."
tags: ["rendering", "frame-graph", "gpu", "architecture", "ue5"]
categories: ["analysis"]
series: ["Rendering Architecture"]
showTableOfContents: false
---

{{< article-nav >}}

<div style="margin:0 0 1.5em;padding:.7em 1em;border-radius:8px;background:rgba(99,102,241,.04);border:1px solid rgba(99,102,241,.12);font-size:.88em;line-height:1.6;opacity:.85;">
📖 <strong>Part III of III.</strong>&ensp; <a href="/posts/frame-graph-theory/">Theory</a> → <a href="/posts/frame-graph-build-it/">Build It</a> → <em>Production Engines</em>
</div>

<div style="border-left:4px solid #6366f1;background:linear-gradient(135deg,rgba(99,102,241,.06),transparent);border-radius:0 10px 10px 0;padding:1em 1.3em;margin:1em 0;font-size:.95em;font-style:italic;line-height:1.55">
How UE5, Frostbite, and Unity implement the same ideas at scale — what they added, what they compromised, and where they still differ.
</div>

[Part II](/posts/frame-graph-build-it/) left us with a working frame graph — automatic barriers, pass culling, and memory aliasing in ~300 lines of C++. That's a solid MVP, but production engines face problems we didn't: parallel command recording, subpass merging for mobile GPUs, async compute scheduling, and managing thousands of passes across legacy codebases. This article examines how three major engines solved those problems, then lays out an upgrade roadmap for taking the MVP further.

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

The MVP from [Part II](/posts/frame-graph-build-it/) already handles automatic barriers, pass culling, and basic memory aliasing. Here's what to add next, in what order, and why — bridging the gap between our prototype and what ships in production.

### 1. Production-grade memory aliasing
**Priority: HIGH · Difficulty: Medium**

[Part II's MVP v3](/posts/frame-graph-build-it/) implements the core algorithm — lifetime scanning, greedy free-list allocation, and interval-graph coloring. That's enough to prove the 30–50% VRAM savings. But production engines refine it in three ways our MVP skips:

<div class="diagram-steps">
  <div class="ds-step">
    <div class="ds-num">1</div>
    <div><strong>Placed resources / heap sub-allocation.</strong> Our MVP assigns logical "blocks" — production engines allocate <code>ID3D12Heap</code> (D3D12) or <code>VkDeviceMemory</code> (Vulkan) and bind resources at offsets within them. This is what makes aliasing real on the GPU. Without placed resources, each allocation gets its own memory and aliasing is impossible.</div>
  </div>
  <div class="ds-step">
    <div class="ds-num">2</div>
    <div><strong>Power-of-two bucketing.</strong> Round resource sizes up to the nearest power of two before matching against the free list. Reduces fragmentation at the cost of slight over-allocation. UE5 does this in its transient allocator.</div>
  </div>
  <div class="ds-step">
    <div class="ds-num">3</div>
    <div><strong>Cross-frame pooling.</strong> Instead of allocating and freeing heaps every frame, maintain a persistent pool sized to peak usage over the last N frames. Amortizes allocation cost to near zero. Both UE5 and Frostbite pool aggressively.</div>
  </div>
</div>

**What to watch out for:**

<div class="diagram-warn">
  <div class="dw-title">⚠ Aliasing pitfalls</div>
  <div class="dw-row"><div class="dw-label">Format compat</div><div>depth/stencil metadata may conflict with color targets on same VkMemory → skip aliasing for depth formats</div></div>
  <div class="dw-row"><div class="dw-label">Initialization</div><div>reused memory = garbage contents → first use <strong>MUST</strong> be a full clear or fullscreen write</div></div>
  <div class="dw-row"><div class="dw-label">Imported res</div><div>survive across frames — <strong>never alias</strong>. Only transient resources qualify.</div></div>
</div>

UE5's transient allocator does all three refinements. The core algorithm from Part II is correct — these additions make it production-ready.

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

Across these three articles, we covered the full arc: [Part I](/posts/frame-graph-theory/) laid out the theory — why manual resource management breaks at scale and how the declare/compile/execute lifecycle solves it. [Part II](/posts/frame-graph-build-it/) turned that theory into working C++ — three iterations from scaffolding to automatic barriers, pass culling, and memory aliasing. And this article mapped those ideas onto what ships in UE5, Frostbite, and Unity, then charted the path from MVP to production: placed-resource heaps, subpass merging, async compute, and split barriers.

You can now open `RenderGraphBuilder.h` in UE5 and *read* it, not reverse-engineer it. You know what `FRDGBuilder::AddPass` builds, how the transient allocator aliases memory, why `ERDGPassFlags::AsyncCompute` exists, and where the RDG boundary with legacy code still leaks.

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

---

<div style="margin:2em 0 0;padding:1em 1.2em;border-radius:10px;border:1px solid rgba(99,102,241,.2);background:rgba(99,102,241,.03);display:flex;justify-content:flex-start;">
  <a href="/posts/frame-graph-build-it/" style="text-decoration:none;font-weight:700;font-size:.95em;">
    ← Previous: Part II — Build It
  </a>
</div>
