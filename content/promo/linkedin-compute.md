---
title: "LinkedIn Promo — Compute Article"
date: 2026-03-23
draft: true
robots: "noindex, nofollow"
sitemap:
  disable: true
showTableOfContents: false
showDate: false
showAuthor: false
showPagination: false
showTitle: false
---

<!-- ═══════════════════════════════════════════════════════════════
  LINKEDIN PROMO R&D — Compute Article
  Quiet, technical concepts for: "Making a Tiny Compute Copy
  Survive a Real Unreal Frame"
     ═══════════════════════════════════════════════════════════════ -->

<style>
.promo-lab { max-width: 660px; margin: 0 auto; }
.promo-lab h2 {
  font-size: clamp(1.18rem, 1.25vw, 1.4rem);
  font-weight: 700;
  letter-spacing: .04em;
  text-transform: uppercase;
  color: #FF9F1C;
  margin: 2.6em 0 .7em;
  padding-bottom: .38em;
  border-bottom: 1px solid rgba(255,159,28,.22);
}
.promo-lab h2:first-child { margin-top: 0; }
.promo-lab .post-caption {
  font-size: clamp(0.94rem, 0.95vw, 1.1rem);
  opacity: .66;
  margin-bottom: 1.5em;
  line-height: 1.58;
}

.slide {
  position: relative;
  aspect-ratio: 4 / 5;
  width: 100%;
  border-radius: 6px;
  overflow: hidden;
  margin-bottom: 1.4em;
  display: flex;
  flex-direction: column;
  justify-content: center;
  box-shadow: 0 24px 46px rgba(0,0,0,.38);
  isolation: isolate;
}
.slide::before {
  content: '';
  position: absolute;
  inset: 0;
  pointer-events: none;
}
.slide::after {
  content: '';
  position: absolute;
  pointer-events: none;
}
.slide * { position: relative; z-index: 1; }

.li-post-text {
  background: linear-gradient(145deg, rgba(255,255,255,.05), rgba(255,255,255,.015));
  border-radius: 10px;
  padding: 1.4em 1.6em;
  margin-bottom: 1.4em;
  font-size: clamp(1rem, 1vw, 1.16rem);
  line-height: 1.62;
  color: rgba(255,255,255,.68);
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.li-post-text .li-hook {
  font-weight: 800;
  color: #fff;
  font-size: clamp(1.2rem, 1.3vw, 1.5rem);
  margin-bottom: .6em;
  line-height: 1.36;
}
.li-post-text .li-body { margin-bottom: .8em; }
.li-post-text .li-cta { color: #FF9F1C; font-weight: 700; }
.li-post-text .li-tags {
  color: rgba(255,159,28,.7);
  font-size: clamp(1.08rem, 1.02vw, 1.24rem);
  margin-top: .6em;
}

.compute-promo-note {
  background: linear-gradient(145deg, rgba(255,255,255,.045), rgba(255,255,255,.015));
  border-radius: 12px;
  padding: 1.05em 1.2em;
  margin: 0 0 1.15em;
  color: rgba(255,255,255,.64);
  line-height: 1.58;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.compute-promo-note strong { color: rgba(255,255,255,.9); }

.compute-slide {
  background: linear-gradient(160deg, #0b0d12 0%, #0d1016 58%, #10141c 100%);
  padding: 2em 1.85em;
}
.compute-slide::before {
  background:
    radial-gradient(ellipse at 18% 14%, rgba(255,255,255,.045) 0%, transparent 54%),
    radial-gradient(ellipse at 86% 78%, rgba(255,159,28,.08) 0%, transparent 42%),
    linear-gradient(120deg, rgba(255,255,255,.02) 0%, rgba(255,255,255,0) 35%);
}
.compute-slide::after {
  width: 240px;
  height: 240px;
  bottom: -90px;
  right: -70px;
  background: radial-gradient(circle at 30% 30%, rgba(255,159,28,.1), rgba(255,159,28,0) 70%);
}
.compute-slide .slide-label {
  display: block;
  font-size: .84rem;
  letter-spacing: .09em;
  text-transform: uppercase;
  color: rgba(255,255,255,.42);
  font-weight: 800;
  margin-bottom: .8em;
}
.compute-slide .slide-num {
  display: block;
  position: absolute;
  top: 1.5em;
  right: 1.6em;
  font-size: .82rem;
  color: rgba(255,255,255,.28);
  font-weight: 700;
  letter-spacing: .08em;
  text-transform: uppercase;
}
.compute-kicker {
  font-size: .9rem;
  text-transform: uppercase;
  letter-spacing: .08em;
  color: #ffb347;
  font-weight: 800;
  margin-bottom: .7em;
}
.compute-slide .hook {
  max-width: 100%;
  font-size: clamp(2rem, 2.55vw, 2.55rem);
  line-height: 1.07;
  margin-bottom: .45em;
  color: #fff;
  font-weight: 900;
  letter-spacing: -.01em;
}
.compute-slide .sub {
  max-width: 100%;
  font-size: clamp(1.02rem, 1.02vw, 1.18rem);
  line-height: 1.52;
  color: rgba(255,255,255,.6);
}
.compute-wire {
  display: grid;
  grid-template-columns: repeat(5, minmax(0, 1fr));
  gap: .45em;
  margin: 1.15em 0 .85em;
}
.compute-wire .node {
  border-radius: 10px;
  padding: .78em .72em;
  background: linear-gradient(145deg, rgba(255,255,255,.06), rgba(255,255,255,.02));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.07);
}
.compute-wire .node .tag {
  font-size: .72rem;
  text-transform: uppercase;
  letter-spacing: .08em;
  color: rgba(255,255,255,.4);
  font-weight: 800;
  margin-bottom: .45em;
}
.compute-wire .node .title {
  font-size: .96rem;
  line-height: 1.22;
  color: #fff;
  font-weight: 800;
}
.compute-wire .node .body {
  margin-top: .35em;
  font-size: .82rem;
  line-height: 1.36;
  color: rgba(255,255,255,.58);
}
.compute-wire .node.hot {
  box-shadow: inset 0 0 0 1px rgba(255,179,71,.28);
  background: linear-gradient(145deg, rgba(255,179,71,.09), rgba(255,179,71,.03));
}
.compute-arrow-row {
  display: grid;
  grid-template-columns: repeat(5, minmax(0, 1fr));
  gap: .45em;
  margin: -.15em 0 .55em;
  color: rgba(255,255,255,.18);
  text-align: center;
  font-weight: 700;
}
.compute-metric-row {
  display: grid;
  grid-template-columns: repeat(3, minmax(0, 1fr));
  gap: .55em;
  margin: 1.05em 0 .9em;
}
.compute-metric {
  border-radius: 10px;
  padding: .82em .82em;
  background: linear-gradient(145deg, rgba(255,255,255,.05), rgba(255,255,255,.02));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.compute-metric .k {
  font-size: .72rem;
  text-transform: uppercase;
  letter-spacing: .08em;
  color: rgba(255,255,255,.4);
  font-weight: 800;
}
.compute-metric .v {
  margin-top: .38em;
  font-size: 1rem;
  line-height: 1.34;
  color: #fff;
  font-weight: 800;
}
.compute-ladder {
  margin: 1.05em 0 .8em;
  border-radius: 12px;
  overflow: hidden;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.compute-ladder .row {
  display: grid;
  grid-template-columns: 146px 1fr;
}
.compute-ladder .row + .row { border-top: 1px solid rgba(255,255,255,.06); }
.compute-ladder .left {
  padding: .82em .9em;
  background: rgba(255,255,255,.035);
  font-size: .78rem;
  letter-spacing: .08em;
  text-transform: uppercase;
  color: rgba(255,255,255,.45);
  font-weight: 800;
}
.compute-ladder .right {
  padding: .82em .95em;
  font-size: .98rem;
  line-height: 1.4;
  color: rgba(255,255,255,.82);
}
.compute-ladder .right strong { color: #fff; }
.compute-mini-caption {
  margin-top: auto;
  padding-top: 1em;
  font-size: .9rem;
  color: rgba(255,255,255,.42);
}
.compute-carousel-outline {
  display: grid;
  gap: .55em;
  margin-top: .9em;
}
.compute-carousel-outline .item {
  display: grid;
  grid-template-columns: 64px 1fr;
  gap: .85em;
  align-items: start;
  border-radius: 10px;
  padding: .78em .85em;
  background: linear-gradient(145deg, rgba(255,255,255,.045), rgba(255,255,255,.015));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.compute-carousel-outline .num {
  font-size: .8rem;
  text-transform: uppercase;
  letter-spacing: .08em;
  color: #ffb347;
  font-weight: 800;
}
.compute-carousel-outline .title {
  font-size: .98rem;
  line-height: 1.32;
  color: #fff;
  font-weight: 800;
}
.compute-carousel-outline .body {
  margin-top: .2em;
  font-size: .9rem;
  line-height: 1.46;
  color: rgba(255,255,255,.58);
}
.compute-selected {
  margin-top: 2.2em;
}
.compute-selected .title {
  font-size: clamp(1.04rem, 1vw, 1.18rem);
  text-transform: uppercase;
  letter-spacing: .06em;
  color: #FF9F1C;
  font-weight: 800;
  margin-bottom: .5em;
}
.compute-bullet-list {
  display: grid;
  gap: .45em;
  margin: 1em 0 .75em;
}
.compute-bullet-item {
  display: grid;
  grid-template-columns: 12px 1fr;
  gap: .65em;
  align-items: start;
  font-size: .98rem;
  line-height: 1.42;
  color: rgba(255,255,255,.8);
}
.compute-bullet-item .dot {
  width: 7px;
  height: 7px;
  margin-top: .45em;
  border-radius: 50%;
  background: #FF9F1C;
}
.compute-bullet-item strong {
  color: #fff;
}
.compute-flow {
  display: grid;
  grid-template-columns: repeat(4, minmax(0, 1fr));
  gap: .55em;
  margin: 1em 0 .85em;
}
.compute-flow .stage {
  border-radius: 10px;
  padding: .82em .8em;
  background: linear-gradient(145deg, rgba(255,255,255,.05), rgba(255,255,255,.02));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.compute-flow .stage .step {
  font-size: .72rem;
  text-transform: uppercase;
  letter-spacing: .08em;
  color: rgba(255,255,255,.4);
  font-weight: 800;
  margin-bottom: .45em;
}
.compute-flow .stage .headline {
  font-size: .98rem;
  line-height: 1.25;
  color: #fff;
  font-weight: 800;
}
.compute-flow .stage .copy {
  margin-top: .3em;
  font-size: .82rem;
  line-height: 1.34;
  color: rgba(255,255,255,.58);
}
.compute-banner {
  margin: .95em 0 .8em;
  border-radius: 12px;
  padding: .95em 1em;
  background: linear-gradient(145deg, rgba(255,159,28,.08), rgba(255,255,255,.02));
  box-shadow: inset 0 0 0 1px rgba(255,159,28,.18);
  color: rgba(255,255,255,.82);
  font-size: 1rem;
  line-height: 1.42;
}
.compute-banner strong {
  color: #fff;
}
.carousel-version {
  margin-top: 1.4em;
  padding: 1em 1.05em 1.1em;
  border-radius: 14px;
  background: linear-gradient(145deg, rgba(255,255,255,.04), rgba(255,255,255,.015));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.carousel-version .version-head {
  display: grid;
  grid-template-columns: 120px 1fr;
  gap: .9em;
  align-items: start;
  margin-bottom: .9em;
}
.carousel-version .version-kicker {
  font-size: .82rem;
  text-transform: uppercase;
  letter-spacing: .08em;
  color: #ffb347;
  font-weight: 800;
}
.carousel-version .version-title {
  font-size: 1.02rem;
  color: #fff;
  font-weight: 800;
  line-height: 1.3;
}
.carousel-version .version-body {
  margin-top: .18em;
  color: rgba(255,255,255,.58);
  font-size: .9rem;
  line-height: 1.45;
}
.carousel-mini-grid {
  display: grid;
  grid-template-columns: repeat(4, minmax(0, 1fr));
  gap: .7em;
}
.mini-slide {
  position: relative;
  aspect-ratio: 4 / 5;
  border-radius: 10px;
  overflow: hidden;
  padding: 1.15em 1.05em;
  display: flex;
  flex-direction: column;
  justify-content: flex-start;
  background: linear-gradient(160deg, #0b0d12 0%, #0d1016 58%, #10141c 100%);
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.mini-slide::before {
  content: '';
  position: absolute;
  inset: 0;
  background:
    radial-gradient(ellipse at 18% 14%, rgba(255,255,255,.04) 0%, transparent 54%),
    radial-gradient(ellipse at 86% 78%, rgba(255,159,28,.07) 0%, transparent 42%);
  pointer-events: none;
}
.mini-slide * { position: relative; z-index: 1; }
.mini-slide .mini-num {
  font-size: .76rem;
  letter-spacing: .08em;
  text-transform: uppercase;
  color: rgba(255,255,255,.34);
  font-weight: 800;
  margin-bottom: .72em;
}
.mini-slide .mini-title {
  font-size: 1.58rem;
  line-height: 1.02;
  color: #fff;
  font-weight: 900;
  letter-spacing: -.02em;
  margin-bottom: .55em;
}
.mini-slide .mini-copy {
  font-size: .98rem;
  line-height: 1.48;
  color: rgba(255,255,255,.6);
}
.slide-compare-row {
  margin-top: 1.25em;
  padding: 1em 1.05em 1.1em;
  border-radius: 14px;
  background: linear-gradient(145deg, rgba(255,255,255,.04), rgba(255,255,255,.015));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.slide-compare-head {
  display: grid;
  grid-template-columns: 110px 1fr;
  gap: .9em;
  align-items: start;
  margin-bottom: .9em;
}
.slide-compare-kicker {
  font-size: .82rem;
  text-transform: uppercase;
  letter-spacing: .08em;
  color: #ffb347;
  font-weight: 800;
}
.slide-compare-title {
  font-size: 1.02rem;
  color: #fff;
  font-weight: 800;
  line-height: 1.3;
}
.slide-compare-body {
  margin-top: .18em;
  color: rgba(255,255,255,.58);
  font-size: .9rem;
  line-height: 1.45;
}
.slide-compare-grid {
  display: grid;
  grid-auto-flow: column;
  grid-auto-columns: minmax(280px, 320px);
  gap: .9em;
  overflow-x: auto;
  padding-bottom: .55em;
  scrollbar-width: thin;
  scrollbar-color: rgba(255,159,28,.35) rgba(255,255,255,.05);
}
.slide-compare-grid::-webkit-scrollbar {
  height: 10px;
}
.slide-compare-grid::-webkit-scrollbar-track {
  background: rgba(255,255,255,.05);
  border-radius: 999px;
}
.slide-compare-grid::-webkit-scrollbar-thumb {
  background: rgba(255,159,28,.35);
  border-radius: 999px;
}
.mini-slide .mini-variant {
  font-size: .7rem;
  letter-spacing: .08em;
  text-transform: uppercase;
  color: #ffb347;
  font-weight: 800;
  margin-bottom: .45em;
}
@media (min-width: 1200px) {
  .slide-compare-grid {
    grid-auto-columns: minmax(300px, 340px);
  }
}
@media (max-width: 820px) {
  .compute-wire,
  .compute-arrow-row,
  .compute-metric-row,
  .compute-flow,
  .carousel-mini-grid {
    grid-template-columns: 1fr;
  }
  .compute-arrow-row { display: none; }
  .compute-ladder .row,
  .slide-compare-head,
  .carousel-version .version-head,
  .compute-carousel-outline .item {
    grid-template-columns: 1fr;
  }
}
</style>

<div class="promo-lab">

## Compute Article — LinkedIn Promo

<div class="post-caption">
Final thumbnail direction for the compute article. The audience is Unreal-facing readers who want to understand what one familiar compute pass becomes underneath in D3D12.
</div>

<div class="compute-promo-note">
  <strong>Direction:</strong> keep the promise concrete and Unreal-oriented. One familiar pass on the surface, one explicit D3D12 path underneath.
</div>

<div class="compute-promo-note">
  <strong>Positioning:</strong> this is not selling compute in general. It is selling clarity about what the engine is hiding and why that helps when building or debugging a pass.
</div>

<div class="li-post-text">
<div class="li-hook">If you use Unreal, this is the layer under the pass you already know.</div>
<div class="li-body">
This article starts from a familiar Unreal-level goal: copy one texture into another with one small compute pass.
<br><br>
Then it walks one layer down and shows what that simple pass actually means in D3D12: source and destination resources, SRV and UAV bindings, a root signature that defines shader access, a Pipeline State Object (PSO), command-list recording, queue submission, and the hand-off to the next pass.
<br><br>
The value is practical: a clearer model for where compute passes usually go wrong, what each layer is responsible for, and why a pass that looks simple in Unreal still has real API machinery underneath it.
</div>
<div class="li-cta">Full article draft → link in first comment</div>
<div class="li-tags">#rendering #graphicsprogramming #gamedev #d3d12 #unrealengine #hlsl #compute</div>
</div>

<div class="slide compute-slide">
  <span class="slide-label">Compute Article / Final Thumbnail</span>
  <span class="slide-num">1 / 5</span>
  <div class="compute-kicker">Under The Pass</div>
  <div class="hook">What does one Unreal pass<br>really become in D3D12?</div>
  <div class="sub">Start from one simple Unreal copy pass, then unwrap the D3D12 path beneath it: source and destination resources, binding layout, executable pipeline state, Dispatch, and the next-pass transition.</div>
  <div class="compute-flow">
    <div class="stage">
      <div class="step">Start</div>
      <div class="headline">Copy pass</div>
      <div class="copy">Read one texture, write another.</div>
    </div>
    <div class="stage">
      <div class="step">Below</div>
      <div class="headline">D3D12 path</div>
      <div class="copy">Resources, SRV/UAV bindings, and executable state.</div>
    </div>
    <div class="stage">
      <div class="step">Execute</div>
      <div class="headline">Dispatch</div>
      <div class="copy">The command list turns setup into GPU work.</div>
    </div>
    <div class="stage">
      <div class="step">After</div>
      <div class="headline">Transition</div>
      <div class="copy">The output must still reach the next pass correctly.</div>
    </div>
  </div>
  <div class="compute-banner"><strong>Angle:</strong> this version frames the article as an unwrapping of the hidden D3D12 layer underneath one familiar Unreal pass.</div>
</div>

## 9-Slide Preview

<div class="post-caption">
Full-size preview of the selected carousel direction. Each slide now carries one core concept so the sequence teaches one step at a time.
</div>

<div class="slide compute-slide">
  <span class="slide-label">Compute Article / Carousel Preview</span>
  <span class="slide-num">1 / 9</span>
  <div class="compute-kicker">Pull Back</div>
  <div class="hook">What does one Unreal pass<br>really become in D3D12?</div>
  <div class="sub">Start with one familiar compute copy pass, then pull the curtain back on the layers Unreal quietly handles for you.</div>
  <div class="compute-flow">
    <div class="stage">
      <div class="step">Engine</div>
      <div class="headline">Simple pass</div>
      <div class="copy">Read one texture, write one texture.</div>
    </div>
    <div class="stage">
      <div class="step">D3D12</div>
      <div class="headline">Real setup</div>
      <div class="copy">SRV, UAV, descriptors, PSO.</div>
    </div>
    <div class="stage">
      <div class="step">GPU</div>
      <div class="headline">Real work</div>
      <div class="copy">Command list plus Dispatch.</div>
    </div>
    <div class="stage">
      <div class="step">Frame</div>
      <div class="headline">Real result</div>
      <div class="copy">The output still has to survive.</div>
    </div>
  </div>
  <div class="compute-banner"><strong>Idea:</strong> Unreal gives you a clean pass abstraction. The article shows the explicit D3D12 path underneath it without assuming you already speak D3D12 fluently.</div>
</div>

<div class="slide compute-slide">
  <span class="slide-label">Compute Article / Carousel Preview</span>
  <span class="slide-num">2 / 9</span>
  <div class="compute-kicker">Pass Intent</div>
  <div class="hook">In Unreal, this starts as<br>one small render pass.</div>
  <div class="sub">At the engine level the ask is simple: read one texture and copy it into another. But the moment that pass enters the frame, the renderer has to turn that intent into something the GPU can actually execute.</div>
  <div class="compute-ladder">
    <div class="row">
      <div class="left">Intent</div>
      <div class="right"><strong>One pass</strong> wants to read a source and write a destination</div>
    </div>
    <div class="row">
      <div class="left">Context</div>
      <div class="right"><strong>The frame accepts it</strong> as part of a larger render sequence</div>
    </div>
    <div class="row">
      <div class="left">Question</div>
      <div class="right"><strong>What has to exist</strong> before that pass can legally run?</div>
    </div>
  </div>
  <div class="compute-banner"><strong>Core concept:</strong> the journey starts with pass intent. The next step is giving that intent real source and destination resources.</div>
</div>

<div class="slide compute-slide">
  <span class="slide-label">Compute Article / Carousel Preview</span>
  <span class="slide-num">3 / 9</span>
  <div class="compute-kicker">Real Resources</div>
  <div class="hook">Before anything else, the pass needs<br>something to read and somewhere to write.</div>
  <div class="sub">The Unreal pass cannot stay abstract anymore. In D3D12 that means a real source resource in a readable state and a real destination resource created for unordered compute writes. If either one is missing or misconfigured, the copy path is broken before Dispatch even exists.</div>
  <div class="compute-metric-row">
    <div class="compute-metric">
      <div class="k">Read</div>
      <div class="v">Source texture</div>
    </div>
    <div class="compute-metric">
      <div class="k">Write</div>
      <div class="v">Destination texture</div>
    </div>
    <div class="compute-metric">
      <div class="k">Next</div>
      <div class="v">How will the shader see both?</div>
    </div>
  </div>
  <div class="compute-banner"><strong>Core concept:</strong> a copy-style compute pass needs both readable input and writable output. The next step is defining what kind of access the shader is allowed to have to each one.</div>
</div>

<div class="slide compute-slide">
  <span class="slide-label">Compute Article / Carousel Preview</span>
  <span class="slide-num">4 / 9</span>
  <div class="compute-kicker">Binding Contract</div>
  <div class="hook">Now the renderer defines<br>what the shader is allowed to touch.</div>
  <div class="sub">A compute shader cannot just reach into memory. In D3D12 this contract is the root signature: it declares which bindings exist, whether the shader receives an SRV for reads, a UAV for writes, constants, or descriptor-backed tables, and what layout the command list must satisfy before the shader is allowed to run.</div>
  <div class="compute-flow">
    <div class="stage">
      <div class="step">Contract</div>
      <div class="headline">Root signature</div>
      <div class="copy">Defines the binding layout the shader expects.</div>
    </div>
    <div class="stage">
      <div class="step">Input</div>
      <div class="headline">SRV slot</div>
      <div class="copy">An SRV slot exposes readable resource data.</div>
    </div>
    <div class="stage">
      <div class="step">Output</div>
      <div class="headline">UAV slot</div>
      <div class="copy">A UAV slot exposes writable resource data.</div>
    </div>
    <div class="stage">
      <div class="step">Question</div>
      <div class="headline">What binds there?</div>
      <div class="copy">The contract exists, but it still points at nothing.</div>
    </div>
  </div>
  <div class="compute-banner"><strong>Core concept:</strong> the root signature is the legal interface between shader code and GPU state. For a copy pass, it needs to describe both reads and writes before the bindings can be filled in.</div>
</div>

<div class="slide compute-slide">
  <span class="slide-label">Compute Article / Carousel Preview</span>
  <span class="slide-num">5 / 9</span>
  <div class="compute-kicker">Real Bindings</div>
  <div class="hook">Then those abstract slots become<br>real descriptors and real data.</div>
  <div class="sub">The source texture needs an SRV descriptor, the destination texture needs a UAV descriptor, and those pieces have to be placed where D3D12 can reference them. A descriptor is not the resource itself. It is the small record that tells the GPU which resource to access, which subresource range to use, and whether that binding is for reading or writing.</div>
  <div class="compute-ladder">
    <div class="row">
      <div class="left">Source</div>
      <div class="right"><strong>Create an SRV descriptor</strong> that points at the readable texture resource</div>
    </div>
    <div class="row">
      <div class="left">Destination</div>
      <div class="right"><strong>Create a UAV descriptor</strong> that points at the writable texture resource</div>
    </div>
    <div class="row">
      <div class="left">Bind</div>
      <div class="right"><strong>Descriptor tables or root bindings</strong> connect the root signature to both resources</div>
    </div>
  </div>
  <div class="compute-banner"><strong>Core concept:</strong> descriptors are how D3D12 names real GPU resources at bind time. In a copy pass, the important distinction is explicit: one descriptor is for reading, one is for writing.</div>
</div>

<div class="slide compute-slide">
  <span class="slide-label">Compute Article / Carousel Preview</span>
  <span class="slide-num">6 / 9</span>
  <div class="compute-kicker">Pipeline State</div>
  <div class="hook">The shader still needs<br>a legal executable form.</div>
  <div class="sub">Source code is not enough. The renderer has to turn the compute shader and its binding layout into a compute Pipeline State Object, or PSO. In practice, that PSO packages the compiled shader together with the root-signature expectations so the command list can bind one coherent piece of executable state instead of loosely assembled setup.</div>
  <div class="compute-metric-row">
    <div class="compute-metric">
      <div class="k">Pipeline</div>
      <div class="v">Shader + PSO</div>
    </div>
    <div class="compute-metric">
      <div class="k">Reason</div>
      <div class="v">Executable state</div>
    </div>
    <div class="compute-metric">
      <div class="k">Next</div>
      <div class="v">Record commands</div>
    </div>
  </div>
  <div class="compute-banner"><strong>Core concept:</strong> PSO creation freezes the compute shader into executable GPU state with a matching binding layout. The next step is placing that state onto a command list in the right order.</div>
</div>

<div class="slide compute-slide">
  <span class="slide-label">Compute Article / Carousel Preview</span>
  <span class="slide-num">7 / 9</span>
  <div class="compute-kicker">Command Recording</div>
  <div class="hook">Now the renderer finally binds state<br>and records the dispatch.</div>
  <div class="sub">This is the moment the pass becomes command-list work. The renderer records commands in the exact order the GPU must observe them: bind the descriptor heap, bind the root signature, bind the PSO, transition the source into a readable state and the destination into UAV state, set the root bindings, and only then call Dispatch.</div>
  <div class="compute-flow">
    <div class="stage">
      <div class="step">Bind</div>
      <div class="headline">Heap + root</div>
      <div class="copy">The command list gets the descriptor space and binding layout.</div>
    </div>
    <div class="stage">
      <div class="step">State</div>
      <div class="headline">Transition</div>
      <div class="copy">The source becomes readable and the destination becomes writable.</div>
    </div>
    <div class="stage">
      <div class="step">Execute</div>
      <div class="headline">Dispatch</div>
      <div class="copy">Dispatch defines how many thread groups copy source texels into destination texels.</div>
    </div>
    <div class="stage">
      <div class="step">Question</div>
      <div class="headline">Then what?</div>
      <div class="copy">The output still has to survive the frame.</div>
    </div>
  </div>
  <div class="compute-banner"><strong>Core concept:</strong> command recording is the ordered script the GPU follows. For a copy pass, both the read path and the write path have to be valid before Dispatch.</div>
</div>

<div class="slide compute-slide">
  <span class="slide-label">Compute Article / Carousel Preview</span>
  <span class="slide-num">8 / 9</span>
  <div class="compute-kicker">Pass Hand-Off</div>
  <div class="hook">The output matters only if the next pass<br>can safely consume it.</div>
  <div class="sub">After Dispatch, the renderer still has to preserve ordering, issue the right barrier or transition, and carry the result forward without breaking the frame graph. If the compute pass reads one texture and writes another, D3D12 still needs an explicit story for when the write becomes visible and which later pass is allowed to consume the destination.</div>
  <div class="compute-flow">
    <div class="stage">
      <div class="step">Pass A</div>
      <div class="headline">Read + write</div>
      <div class="copy">The compute pass samples the source and writes the destination.</div>
    </div>
    <div class="stage">
      <div class="step">State</div>
      <div class="headline">Transition</div>
      <div class="copy">The destination may need to move from UAV write state to an SRV or copy state.</div>
    </div>
    <div class="stage">
      <div class="step">Order</div>
      <div class="headline">Barrier</div>
      <div class="copy">A barrier tells the GPU when writes must be visible before later reads.</div>
    </div>
    <div class="stage">
      <div class="step">Pass B</div>
      <div class="headline">Consume</div>
      <div class="copy">The next pass has to see the result correctly.</div>
    </div>
  </div>
  <div class="compute-banner"><strong>Core concept:</strong> a pass is only useful if its written result survives into the next pass. In D3D12 that means explicit resource states, explicit visibility rules, and explicit synchronization rather than assuming the driver will infer intent.</div>
</div>

<div class="slide compute-slide">
  <span class="slide-label">Compute Article / Carousel Preview</span>
  <span class="slide-num">9 / 9</span>
  <div class="compute-kicker">Why This Helps</div>
  <div class="hook">By the end of the frame,<br>the hidden work is finally visible.</div>
  <div class="sub">The pass felt simple because Unreal carried the D3D12 mechanics for you: source and destination resources, SRV and UAV bindings, PSO setup, command-list recording, and pass-to-pass hand-off.</div>
  <div class="compute-bullet-list">
    <div class="compute-bullet-item">
      <div class="dot"></div>
      <div><strong>Clearer ordering:</strong> pass intent became source and destination resources, binding contract, descriptors, pipeline, dispatch, and then hand-off.</div>
    </div>
    <div class="compute-bullet-item">
      <div class="dot"></div>
      <div><strong>Clearer value:</strong> when a compute pass fails, you know which layer to inspect first.</div>
    </div>
    <div class="compute-bullet-item">
      <div class="dot"></div>
      <div><strong>Reader payoff:</strong> you leave with a reusable model for what Unreal is doing underneath one familiar pass.</div>
    </div>
  </div>
  <div class="compute-banner"><strong>Read the article:</strong> follow one Unreal-style compute copy pass from read/write intent, to resources, to bindings, to Dispatch, to the next pass, and the whole path stops feeling mysterious.</div>
</div>

</div>