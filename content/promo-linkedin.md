---
title: "LinkedIn Promo R&D"
date: 2026-03-03
draft: true
robots: "noindex, nofollow"
sitemap:
  disable: true
showTableOfContents: false
showDate: false
showAuthor: false
showPagination: false
---

<!-- ═══════════════════════════════════════════════════════════════
     LINKEDIN PROMO R&D — Frame Graph Series
  Carousel slide mockups. Export each slide at 1080×1350 (LinkedIn 4:5 portrait).
     Each "Post" section = one LinkedIn post (carousel of slides).
     ═══════════════════════════════════════════════════════════════ -->

<style>
/* ── Global promo reset ──────────────────────────────────────── */
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
  opacity: .62;
  margin-bottom: 1.5em;
  line-height: 1.58;
  font-style: italic;
}

/* ── Slide canvas (LinkedIn portrait 4:5) ────────────────────── */
.slide {
  position: relative;
  aspect-ratio: 4 / 5;
  width: 100%;
  background: linear-gradient(145deg, #0a0c12 0%, #0b0b0e 45%, #101522 100%);
  border-radius: 6px;
  overflow: hidden;
  margin-bottom: 1.4em;
  padding: 2.45em 2.1em;
  display: flex; flex-direction: column; justify-content: center;
  border: 0;
  box-shadow: 0 24px 46px rgba(0,0,0,.38);
  isolation: isolate;
}
.slide::before {
  content: '';
  position: absolute; inset: 0;
  background: radial-gradient(ellipse at 18% 14%, rgba(255,159,28,.11) 0%, transparent 52%),
              radial-gradient(ellipse at 88% 86%, rgba(94,198,255,.12) 0%, transparent 45%),
              linear-gradient(120deg, rgba(255,255,255,.03) 0%, rgba(255,255,255,0) 35%);
  pointer-events: none;
}
.slide::after {
  content: '';
  position: absolute;
  top: -90px;
  right: -120px;
  width: 330px;
  height: 330px;
  border-radius: 50%;
  background: radial-gradient(circle at 30% 30%, rgba(129,140,248,.2), rgba(129,140,248,0) 65%);
  pointer-events: none;
}
.slide:nth-of-type(odd)::after {
  left: -120px;
  right: auto;
  background: radial-gradient(circle at 60% 45%, rgba(255,159,28,.18), rgba(255,159,28,0) 64%);
}
.slide * { position: relative; z-index: 1; }

/* ── Meta labels (no breadcrumb look) ────────────────────────── */
.slide-label {
  display: none;
}
.slide-num {
  display: none;
}

/* ── Typography ──────────────────────────────────────────────── */
.slide .hook {
  font-size: clamp(2rem, 2.65vw, 2.75rem);
  font-weight: 900;
  line-height: 1.16;
  letter-spacing: -.01em; color: #fff; margin-bottom: .6em;
  max-width: 86%;
}
.slide .hook em {
  color: #FF9F1C; font-style: normal;
}
.slide .hook .gold { color: #FFD166; }
.slide .hook .green { color: #22c55e; }
.slide .hook .red { color: #ef4444; }
.slide .hook .indigo { color: #818cf8; }
.slide .sub {
  font-size: clamp(1.16rem, 1.32vw, 1.46rem);
  line-height: 1.58;
  color: rgba(255,255,255,.66);
  max-width: 80%;
}
.slide .sub strong { color: rgba(255,255,255,.9); font-weight: 700; }
.slide .micro {
  font-size: clamp(1.02rem, 0.98vw, 1.14rem);
  color: rgba(255,255,255,.42);
  margin-top: auto;
  padding-top: 1.35em;
  opacity: .8;
}
.slide .micro a { color: #FF9F1C; text-decoration: none; }

/* ── Accent line ─────────────────────────────────────────────── */
.slide .accent-bar {
  width: 72px; height: 6px; border-radius: 999px;
  background: linear-gradient(90deg, #FF9F1C, #FFD166);
  margin-bottom: 1.2em;
  box-shadow: 0 0 0 1px rgba(255,255,255,.05), 0 6px 16px rgba(255,159,28,.28);
}
.slide .accent-bar.gold { background: #FFD166; }
.slide .accent-bar.green { background: #22c55e; }
.slide .accent-bar.indigo { background: #818cf8; }

/* ── Stat grid ───────────────────────────────────────────────── */
.stat-grid {
  display: grid; grid-template-columns: repeat(3, 1fr);
  gap: .8em; margin: 1.2em 0;
}
.stat-card {
  background: linear-gradient(145deg, rgba(255,255,255,.06), rgba(255,255,255,.02));
  border: 0;
  border-radius: 10px; padding: 1em .8em; text-align: center;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.stat-card .num {
  font-size: clamp(2.05rem, 2.15vw, 2.6rem);
  font-weight: 900;
  line-height: 1;
  color: #FF9F1C;
}
.stat-card .num.gold { color: #FFD166; }
.stat-card .num.green { color: #22c55e; }
.stat-card .num.red { color: #ef4444; }
.stat-card .num.indigo { color: #818cf8; }
.stat-card .label {
  font-size: clamp(0.98rem, 0.92vw, 1.12rem);
  text-transform: uppercase;
  letter-spacing: .06em;
  color: rgba(255,255,255,.4); margin-top: .4em; font-weight: 600;
}

/* ── Comparison row ──────────────────────────────────────────── */
.compare-row {
  display: grid; grid-template-columns: 1fr auto 1fr;
  align-items: center; gap: .6em; margin: 1em 0;
}
.compare-box {
  background: linear-gradient(145deg, rgba(255,255,255,.06), rgba(255,255,255,.02));
  border-radius: 10px; padding: 1.1em 1em;
  border: 0;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.compare-box .title {
  font-size: clamp(0.98rem, 0.92vw, 1.12rem);
  text-transform: uppercase;
  letter-spacing: .06em;
  font-weight: 700; margin-bottom: .5em;
}
.compare-box.bad .title { color: #ef4444; }
.compare-box.good .title { color: #22c55e; }
.compare-box .body {
  font-size: clamp(0.98rem, 0.98vw, 1.14rem);
  line-height: 1.56;
  color: rgba(255,255,255,.64);
}
.compare-arrow {
  font-size: clamp(2rem, 2vw, 2.5rem);
  color: rgba(255,255,255,.22);
  font-weight: 300;
}

/* ── Step list ───────────────────────────────────────────────── */
.step-list { list-style: none; padding: 0; margin: 1em 0; }
.step-list li {
  display: flex; align-items: flex-start; gap: .7em;
  margin-bottom: .9em;
  font-size: clamp(1.02rem, 1.03vw, 1.18rem);
  line-height: 1.52;
  color: rgba(255,255,255,.7);
}
.step-list .step-num {
  flex-shrink: 0; width: 28px; height: 28px;
  border-radius: 50%; display: flex; align-items: center; justify-content: center;
  font-weight: 800;
  font-size: clamp(1rem, 0.94vw, 1.14rem);
  background: rgba(255,159,28,.12); color: #FF9F1C;
  border: 1.5px solid rgba(255,159,28,.25);
}
.step-list .step-num.green {
  background: rgba(34,197,94,.12); color: #22c55e;
  border-color: rgba(34,197,94,.25);
}
.step-list .step-num.indigo {
  background: rgba(99,102,241,.12); color: #818cf8;
  border-color: rgba(99,102,241,.25);
}
.step-list li strong { color: rgba(255,255,255,.9); }

/* ── Pipeline diagram (ASCII-style) ─────────────────────────── */
.pipe-diagram {
  background: linear-gradient(145deg, rgba(255,255,255,.04), rgba(255,255,255,.01));
  border: 0;
  border-radius: 10px; padding: 1.2em 1.4em;
  font-family: ui-monospace, 'Cascadia Code', Consolas, monospace;
  font-size: clamp(0.92rem, 0.92vw, 1.06rem);
  line-height: 1.72;
  color: rgba(255,255,255,.6);
  margin: 1em 0;
  overflow-x: auto;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.pipe-diagram .pass { color: #fff; font-weight: 700; }
.pipe-diagram .barrier { color: #ef4444; font-weight: 600; }
.pipe-diagram .arrow { color: rgba(255,255,255,.2); }
.pipe-diagram .async { color: #818cf8; font-weight: 600; }
.pipe-diagram .fence { color: #FFD166; font-weight: 600; }
.pipe-diagram .alias { color: #22c55e; font-weight: 600; }

/* ── Tag pills ───────────────────────────────────────────────── */
.tag-row {
  display: flex; flex-wrap: wrap; gap: .4em; margin: .8em 0;
}
.tag-pill {
  font-size: clamp(0.96rem, 0.9vw, 1.1rem);
  font-weight: 700;
  letter-spacing: .04em;
  padding: .3em .7em; border-radius: 6px;
  text-transform: uppercase;
}
.tag-pill.orange {
  background: rgba(255,159,28,.1); color: #FF9F1C;
  border: 1px solid rgba(255,159,28,.2);
}
.tag-pill.green {
  background: rgba(34,197,94,.1); color: #22c55e;
  border: 1px solid rgba(34,197,94,.2);
}
.tag-pill.indigo {
  background: rgba(99,102,241,.1); color: #818cf8;
  border: 1px solid rgba(99,102,241,.2);
}
.tag-pill.gold {
  background: rgba(255,209,102,.1); color: #FFD166;
  border: 1px solid rgba(255,209,102,.2);
}
.tag-pill.red {
  background: rgba(239,68,68,.1); color: #ef4444;
  border: 1px solid rgba(239,68,68,.2);
}

/* ── CTA block ───────────────────────────────────────────────── */
.cta-block {
  margin-top: 1.5em; padding: 1.2em 1.4em;
  border-radius: 10px;
  background: linear-gradient(135deg, rgba(255,159,28,.08), rgba(255,159,28,.02));
  border: 0;
  box-shadow: inset 0 0 0 1px rgba(255,159,28,.24), 0 10px 22px rgba(0,0,0,.2);
}
.cta-block .cta-title {
  font-size: clamp(1.14rem, 1.2vw, 1.34rem);
  font-weight: 800;
  color: #FF9F1C;
  margin-bottom: .4em;
}
.cta-block .cta-body {
  font-size: clamp(0.98rem, 0.98vw, 1.14rem);
  line-height: 1.56;
  color: rgba(255,255,255,.64);
}

/* ── Discussion block ────────────────────────────────────────── */
.discuss-block {
  margin-top: auto; padding-top: 1.5em;
}
.discuss-block .discuss-q {
  font-size: clamp(1.2rem, 1.28vw, 1.48rem);
  font-weight: 800;
  color: #fff;
  line-height: 1.4; margin-bottom: .5em;
}
.discuss-block .discuss-hint {
  font-size: clamp(1.06rem, 1vw, 1.2rem);
  color: rgba(255,255,255,.5);
  line-height: 1.52;
}

/* ── Lifecycle visual ────────────────────────────────────────── */
.lifecycle-bar {
  display: flex; align-items: stretch; gap: 0;
  border-radius: 8px; overflow: hidden;
  border: 0;
  margin: 1em 0;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.08);
}
.lifecycle-bar .phase {
  flex: 1; padding: .8em .6em; text-align: center;
  font-size: clamp(1.04rem, 1vw, 1.2rem);
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: .04em;
}
.lifecycle-bar .phase:not(:last-child) { border-right: 1px solid rgba(255,255,255,.06); }
.lifecycle-bar .phase.declare {
  background: rgba(99,102,241,.1); color: #818cf8;
}
.lifecycle-bar .phase.compile {
  background: rgba(255,159,28,.1); color: #FF9F1C;
}
.lifecycle-bar .phase.execute {
  background: rgba(34,197,94,.1); color: #22c55e;
}
.lifecycle-bar .phase-sub {
  display: block;
  font-size: clamp(1rem, 0.94vw, 1.14rem);
  font-weight: 400;
  opacity: .6; margin-top: .2em; text-transform: none;
  letter-spacing: 0;
}

/* ── Post text mockup ────────────────────────────────────────── */
.li-post-text {
  background: linear-gradient(145deg, rgba(255,255,255,.05), rgba(255,255,255,.015));
  border: 0;
  border-radius: 10px; padding: 1.4em 1.6em;
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
.li-post-text .li-cta {
  color: #FF9F1C; font-weight: 700;
}
.li-post-text .li-tags {
  color: rgba(255,159,28,.7);
  font-size: clamp(1.08rem, 1.02vw, 1.24rem);
  margin-top: .6em;
}

/* ── Visual-first hook blocks ────────────────────────────────── */
.visual-rules {
  background: linear-gradient(145deg, rgba(255,255,255,.05), rgba(255,255,255,.015));
  border: 0;
  border-radius: 10px;
  padding: 1.1em 1.2em;
  margin: 0 0 1.6em;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.visual-rules .title {
  font-size: clamp(1.04rem, 0.98vw, 1.2rem);
  text-transform: uppercase;
  letter-spacing: .06em;
  color: #FF9F1C;
  font-weight: 800;
  margin-bottom: .55em;
}
.visual-rules .line {
  font-size: clamp(1.08rem, 1.04vw, 1.24rem);
  line-height: 1.56;
  color: rgba(255,255,255,.62);
}
.visual-rules .line strong { color: rgba(255,255,255,.9); }

.meme-chip {
  align-self: flex-start;
  font-size: clamp(0.98rem, 0.92vw, 1.12rem);
  letter-spacing: .04em;
  font-weight: 700;
  color: rgba(255,255,255,.72);
  background: linear-gradient(145deg, rgba(255,255,255,.12), rgba(255,255,255,.04));
  border: 0;
  border-radius: 999px;
  padding: .35em .7em;
  margin-bottom: .9em;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.12);
}

.focus-photo {
  margin: .3em 0 1.1em;
  min-height: 230px;
  border-radius: 14px;
  border: 0;
  background: radial-gradient(circle at 24% 38%, rgba(239,68,68,.22) 0%, transparent 38%),
              radial-gradient(circle at 80% 64%, rgba(255,159,28,.16) 0%, transparent 40%),
              linear-gradient(140deg, rgba(255,255,255,.04), rgba(255,255,255,.01));
  display: flex;
  align-items: center;
  justify-content: center;
  overflow: hidden;
  position: relative;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.1), 0 12px 24px rgba(0,0,0,.22);
}
.focus-photo::before {
  content: '';
  position: absolute;
  width: 180px;
  height: 180px;
  border-radius: 50%;
  border: 2px solid rgba(255,159,28,.35);
  box-shadow: 0 0 0 20px rgba(255,159,28,.08), 0 0 60px rgba(255,159,28,.2);
}
.focus-photo .subject {
  font-size: clamp(3.5rem, 4.8vw, 5.1rem);
  font-weight: 900;
  letter-spacing: -.02em;
  color: #fff;
  text-shadow: 0 4px 24px rgba(0,0,0,.35);
}
.focus-photo .caption {
  position: absolute;
  bottom: .85em;
  left: 1em;
  right: 1em;
  font-size: clamp(0.98rem, 0.92vw, 1.12rem);
  text-transform: uppercase;
  letter-spacing: .07em;
  color: rgba(255,255,255,.42);
  font-weight: 700;
}

.kpi-band {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: .6em;
  margin: .9em 0 1.1em;
}
.kpi {
  border-radius: 10px;
  border: 0;
  background: linear-gradient(145deg, rgba(255,255,255,.08), rgba(255,255,255,.03));
  text-align: center;
  padding: .9em .5em;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.05);
}
.kpi .v {
  font-size: clamp(1.95rem, 2vw, 2.45rem);
  line-height: 1;
  font-weight: 900;
  color: #fff;
}
.kpi .k {
  margin-top: .4em;
  font-size: clamp(0.96rem, 0.9vw, 1.08rem);
  text-transform: uppercase;
  letter-spacing: .06em;
  color: rgba(255,255,255,.45);
  font-weight: 700;
}

.duo-meter {
  margin: .8em 0 1em;
  border-radius: 12px;
  border: 0;
  overflow: hidden;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.09);
}
.duo-meter .row {
  display: grid;
  grid-template-columns: 130px 1fr;
  align-items: stretch;
}
.duo-meter .row + .row { border-top: 1px solid rgba(255,255,255,.06); }
.duo-meter .label {
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: clamp(0.98rem, 0.92vw, 1.12rem);
  text-transform: uppercase;
  letter-spacing: .07em;
  color: rgba(255,255,255,.6);
  font-weight: 800;
  background: rgba(255,255,255,.03);
}
.duo-meter .bar {
  padding: .65em .8em;
  font-weight: 800;
  font-size: clamp(1.02rem, 1.02vw, 1.18rem);
}
.duo-meter .bar.good {
  background: linear-gradient(90deg, rgba(34,197,94,.24), rgba(34,197,94,.07));
  color: #4ade80;
}
.duo-meter .bar.bad {
  background: linear-gradient(90deg, rgba(239,68,68,.25), rgba(239,68,68,.08));
  color: #f87171;
}

/* ── Benefit grid (non-expert readable) ─────────────────────── */
.benefit-grid {
  display: grid;
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap: .6em;
  margin: .95em 0 0;
}
.benefit-card {
  border-radius: 10px;
  border: 0;
  background: linear-gradient(145deg, rgba(255,255,255,.06), rgba(255,255,255,.02));
  padding: .85em .85em .8em;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.05);
}
.benefit-card .b-tag {
  font-size: clamp(0.94rem, 0.9vw, 1.08rem);
  text-transform: uppercase;
  letter-spacing: .07em;
  font-weight: 800;
  margin-bottom: .45em;
  color: rgba(255,255,255,.72);
}
.benefit-card .b-main {
  font-size: clamp(1.04rem, 1.06vw, 1.22rem);
  font-weight: 800;
  line-height: 1.3;
  color: #fff;
}
.benefit-card .b-sub {
  margin-top: .35em;
  font-size: clamp(1.06rem, 1vw, 1.2rem);
  line-height: 1.5;
  color: rgba(255,255,255,.62);
}
.benefit-card .b-tag.perf { color: #4ade80; }
.benefit-card .b-tag.stab { color: #5ec6ff; }
.benefit-card .b-tag.qual { color: #ffd166; }
.benefit-card .b-tag.prod { color: #818cf8; }
.benefit-card .b-tag.mem { color: #FF9F1C; }

/* ── Thumbnail-inspired motif strip (DAG + queue lanes) ─────── */
.thumb-motif {
  margin: .7em 0 1.1em;
  border-radius: 10px;
  border: 1px solid rgba(255,255,255,.08);
  background: linear-gradient(135deg, rgba(17,35,59,.75), rgba(20,21,30,.85));
  padding: .65em .75em;
}
.thumb-motif .lane {
  height: 9px;
  border-radius: 999px;
  background: rgba(255,255,255,.08);
  overflow: hidden;
  margin-bottom: .45em;
}
.thumb-motif .lane:last-child { margin-bottom: 0; }
.thumb-motif .fill {
  height: 100%;
  border-radius: 999px;
}
.thumb-motif .fill.cyan {
  width: 82%;
  background: linear-gradient(90deg, #5ec6ff, #8defff);
}
.thumb-motif .fill.green {
  width: 68%;
  background: linear-gradient(90deg, #4af0a4, #68ff9a);
}
.thumb-motif .fill.amber {
  width: 56%;
  background: linear-gradient(90deg, #ffb455, #ffd166);
}

/* ── Big-number hook visual ─────────────────────────────────── */
.hero-metric {
  margin: .45em 0 1.1em;
  border-radius: 14px;
  border: 0;
  background: radial-gradient(circle at 50% 20%, rgba(94,198,255,.2) 0%, transparent 55%),
              linear-gradient(145deg, rgba(17,35,59,.9), rgba(20,21,30,.92));
  text-align: center;
  padding: 1.35em 1em 1.1em;
  box-shadow: inset 0 0 0 1px rgba(94,198,255,.2), 0 16px 30px rgba(0,0,0,.34);
}
.hero-metric .value {
  font-size: clamp(4.2rem, 5.3vw, 5.6rem);
  font-weight: 900;
  line-height: .95;
  letter-spacing: -.03em;
  color: #4ade80;
  text-shadow: 0 0 28px rgba(74,222,128,.24);
}
.hero-metric .kicker {
  margin-top: .35em;
  font-size: clamp(1.02rem, 0.96vw, 1.2rem);
  letter-spacing: .08em;
  text-transform: uppercase;
  color: rgba(255,255,255,.72);
  font-weight: 800;
}
.hero-metric .meta {
  margin-top: .42em;
  font-size: clamp(0.98rem, 0.92vw, 1.12rem);
  color: rgba(255,255,255,.45);
  letter-spacing: .05em;
  text-transform: uppercase;
}

.hook-pills {
  display: flex;
  flex-wrap: wrap;
  gap: .45em;
  margin-top: .9em;
}
.hook-pill {
  font-size: clamp(0.96rem, 0.9vw, 1.1rem);
  text-transform: uppercase;
  letter-spacing: .06em;
  font-weight: 800;
  color: rgba(255,255,255,.82);
  border: 0;
  background: linear-gradient(145deg, rgba(255,255,255,.13), rgba(255,255,255,.05));
  border-radius: 999px;
  padding: .34em .68em;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.1);
}

/* Creative composition: stagger internal blocks instead of boxed alignment */
.slide .hook,
.slide .sub,
.slide .benefit-grid,
.slide .step-list,
.slide .kpi-band,
.slide .lifecycle-bar,
.slide .duo-meter,
.slide .pipe-diagram,
.slide .compare-row {
  transform: translateX(var(--slide-shift, 0px));
}
.slide:nth-of-type(3n + 1) { --slide-shift: 0px; }
.slide:nth-of-type(3n + 2) { --slide-shift: 8px; }
.slide:nth-of-type(3n + 3) { --slide-shift: -8px; }

/* Remove hard framed look from inline-styled blocks used in slides */
.slide [style*="border:1px"],
.slide [style*="border: 1px"],
.slide [style*="border:1.5px"],
.slide [style*="border: 1.5px"],
.slide [style*="border-left:2px"],
.slide [style*="border-right:1px"],
.slide [style*="border-right: 1px"] {
  border: 0 !important;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.08);
}

.slide [style*="border-right:1px dashed"],
.slide [style*="border-right: 1px dashed"] {
  border-right: 0 !important;
}

.hook-variant-tag {
  align-self: flex-start;
  margin-bottom: .7em;
  font-size: clamp(0.94rem, 0.88vw, 1.08rem);
  font-weight: 800;
  letter-spacing: .08em;
  text-transform: uppercase;
  color: #5ec6ff;
  border: 1px solid rgba(94,198,255,.35);
  background: rgba(94,198,255,.08);
  border-radius: 999px;
  padding: .34em .64em;
}

/* ── Post 1 scoped redesign (asymmetric, low-frame) ─────────── */
.post1-hero {
  display: grid;
  grid-template-columns: minmax(0, .84fr) minmax(0, 1.16fr);
  gap: 1.25em;
  align-items: stretch;
  margin-bottom: 1.5em;
}
.post1-head {
  display: flex;
  flex-direction: column;
  gap: .7em;
}
.post1-head .post-caption {
  margin-bottom: 0;
  opacity: .62;
  font-style: normal;
}
.post1-head .visual-rules,
.post1-head .li-post-text {
  background: transparent;
  border: 0;
  border-radius: 0;
  box-shadow: none;
  padding: 0;
  margin: 0;
}
.post1-head .visual-rules .title {
  font-size: clamp(1rem, 0.94vw, 1.14rem);
  letter-spacing: .08em;
  margin-bottom: .35em;
}
.post1-head .visual-rules .line {
  font-size: clamp(1.04rem, 0.98vw, 1.2rem);
  line-height: 1.4;
}
.post1-head .li-post-text {
  font-size: clamp(0.96rem, 0.94vw, 1.12rem);
  line-height: 1.46;
}
.post1-head .li-post-text .li-hook {
  font-size: clamp(1.12rem, 1.12vw, 1.3rem);
  line-height: 1.3;
  margin-bottom: .4em;
}
.post1-head .li-post-text .li-body { margin-bottom: .52em; }

.slide.post1-slide {
  margin-bottom: 0;
  padding: 2.8em 2.4em;
  justify-content: center;
  align-items: center;
  text-align: center;
}
.post1-slide .slide-label {
  display: block;
  font-size: clamp(1.06rem, 1.04vw, 1.24rem);
  font-weight: 800;
  letter-spacing: .14em;
  text-transform: uppercase;
  color: #5ec6ff;
  margin-bottom: 1em;
}
.post1-slide .meme-chip {
  background: transparent;
  border: 0;
  box-shadow: none;
  padding: 0;
  margin-bottom: .5em;
  color: rgba(255,255,255,.56);
}
.post1-slide .hero-metric {
  background: transparent;
  border: 0;
  box-shadow: none;
  padding: 0;
  margin: 0 0 .3em;
  text-align: center;
}
.post1-slide .hero-metric .value {
  font-size: clamp(5.2rem, 6.8vw, 7.2rem);
  line-height: .88;
}
.post1-slide .hero-metric .kicker {
  margin-top: .35em;
  text-align: center;
  font-size: clamp(1.1rem, 1.08vw, 1.32rem);
  color: rgba(255,255,255,.82);
}
.post1-slide .hero-metric .meta {
  margin-top: .28em;
  text-align: center;
  font-size: clamp(1rem, 0.96vw, 1.18rem);
}
.post1-slide .hook {
  max-width: 94%;
  margin: .15em auto .3em;
  text-align: center;
}
.post1-slide .hook-pills {
  margin-top: .6em;
  gap: .25em .55em;
  justify-content: center;
}
.post1-slide .hook-pill {
  background: transparent;
  border: 0;
  box-shadow: none;
  padding: 0;
  color: rgba(255,255,255,.74);
  font-size: clamp(0.94rem, 0.88vw, 1.08rem);
  letter-spacing: .08em;
}
.post1-slide .hook-pill:not(:last-child)::after {
  content: '·';
  margin-left: .5em;
  color: rgba(255,255,255,.36);
}
.post1-slide .micro { margin-top: 1.1em; }

/* ── Responsive ──────────────────────────────────────────────── */
@media (max-width: 600px) {
  .post1-hero { grid-template-columns: 1fr; gap: .75em; }
  .slide.post1-slide { padding: 2em 1.6em; }
  .post1-slide .hero-metric .value { font-size: 4.25rem; }
  .post1-slide .hook { max-width: 100%; }
  .slide { padding: 2.15em 1.75em; }
  .slide .hook { font-size: 2.1rem; }
  .slide .sub { font-size: 1.35rem; }
  .slide .micro { font-size: 1.12rem; }
  .hero-metric .value { font-size: 4.1rem; }
  .benefit-card .b-main { font-size: 1.34rem; }
  .benefit-card .b-sub { font-size: 1.2rem; }
  .benefit-card .b-tag { font-size: 1.08rem; }
  .step-list li { font-size: 1.24rem; }
  .duo-meter .bar { font-size: 1.24rem; }
  .duo-meter .label { font-size: 1.08rem; }
  .compare-box .body { font-size: 1.2rem; }
  .compare-box .title { font-size: 1.08rem; }
  .pipe-diagram { font-size: 1.04rem; line-height: 1.62; }
  .li-post-text { font-size: 1.18rem; }
  .li-post-text .li-hook { font-size: 1.42rem; }
  .li-post-text .li-tags { font-size: 1.14rem; }
  .visual-rules .line { font-size: 1.16rem; }
  .visual-rules .title { font-size: 1.08rem; }
  .hook-pill { font-size: 1.08rem; }
  .kpi .k { font-size: 1.08rem; }
  .stat-card .label { font-size: 1.08rem; }
  .slide .hook,
  .slide .sub,
  .slide .benefit-grid,
  .slide .step-list,
  .slide .kpi-band,
  .slide .lifecycle-bar,
  .slide .duo-meter,
  .slide .pipe-diagram,
  .slide .compare-row { transform: none; }
  .focus-photo .subject { font-size: 2.2em; }
  .stat-grid { grid-template-columns: 1fr; }
  .benefit-grid { grid-template-columns: 1fr; }
  .compare-row { grid-template-columns: 1fr; }
  .kpi-band { grid-template-columns: 1fr; }
  .duo-meter .row { grid-template-columns: 1fr; }
  .compare-arrow { transform: rotate(90deg); text-align: center; }
}
</style>

<div class="promo-lab">

<!-- ═══════════════════════════════════════════════════════════════
     POST 1 — SERIES HOOK (Overview post — the one that appears on the feed)
     ═══════════════════════════════════════════════════════════════ -->

## Post 1 — Series Hook

<div class="post-caption">
The main LinkedIn post. This is the carousel people scroll through on their feed.
The post text goes below, slides are the images.
</div>

<div class="visual-rules">
  <div class="title">Distance Test Rule</div>
  <div class="line"><strong>If you need to squint, the slide fails.</strong> Each hook slide below uses one dominant subject, 4–7 words max, and one clear contrast zone so it reads from phone distance in under 1 second.</div>
</div>

<!-- LinkedIn post text mockup -->
<div class="li-post-text">
<div class="li-hook">Up to <strong>50% less transient memory</strong>.<br>Fewer GPU waits and fewer pass-order bugs.</div>
<div class="li-body">
If you've never heard of frame graphs, this is the short version of why to care.
<br><br>
They help teams <strong>save memory</strong> (often up to <strong>50% less transient VRAM</strong>), <strong>save GPU time</strong>, <strong>save developer time</strong>, and <strong>avoid concrete bugs</strong> like missing transitions or wrong pass ordering.
<br><br>
This carousel is a sneak peek of outcomes. The full series explains how those outcomes happen, with practical code and production patterns.
</div>
<div class="li-cta">Full series → link in first comment</div>
<div class="li-tags">#rendering #gpu #graphicsprogramming #gamedev #vulkan #d3d12 #ue5 #framegraph</div>
</div>

<!-- SLIDE 1: Unified Hook -->
<div class="slide post1-slide">
  <span class="slide-label">Frame Graph Series</span>
  <span class="slide-num">1 / 6</span>
  <div class="hero-metric">
    <div class="value">50%</div>
    <div class="kicker">Less transient VRAM (up to)</div>
    <div class="meta">Automatic aliasing · zero manual tracking</div>
  </div>
  <div class="hook" style="font-size:1.15em;">Fewer GPU stalls. Less crashes.<br>Faster iteration. Ship with confidence.</div>
  <div class="hook-pills">
    <span class="hook-pill">Memory</span>
    <span class="hook-pill">GPU Performance</span>
    <span class="hook-pill">Productivity</span>
    <span class="hook-pill">Stability</span>
  </div>
  <div class="micro">Swipe →</div>
</div>

<!-- SLIDE 2: Proof of benefits -->
<div class="slide">
  <span class="slide-label">Proof Of Value</span>
  <span class="slide-num">2 / 6</span>
  <div class="accent-bar green"></div>
  <div class="hook" style="font-size:1.15em;">What this fixes:</div>
  <div class="benefit-grid">
    <div class="benefit-card">
      <div class="b-tag mem">Memory</div>
      <div class="b-main">Up to 50% less transient VRAM</div>
      <div class="b-sub">Resource aliasing reclaims memory across passes automatically.</div>
    </div>
    <div class="benefit-card">
      <div class="b-tag perf">GPU Performance</div>
      <div class="b-main">Fewer pipeline stalls</div>
      <div class="b-sub">Split barriers and async compute keep queues saturated.</div>
    </div>
    <div class="benefit-card">
      <div class="b-tag prod">Productivity</div>
      <div class="b-main">No manual pass wiring</div>
      <div class="b-sub">The compiler orders passes and places barriers for you.</div>
    </div>
    <div class="benefit-card">
      <div class="b-tag stab">Stability</div>
      <div class="b-main">Bugs caught at build time</div>
      <div class="b-sub">Dependency tracking and resource versioning replace guesswork.</div>
    </div>
    <div class="benefit-card" style="grid-column:1 / -1;">
      <div class="b-tag qual">Reliability</div>
      <div class="b-main">Ship features, not sync fixes</div>
      <div class="b-sub">Developers focus on rendering logic — the graph handles the rest.</div>
    </div>
  </div>
  <div class="sub">But how does one system deliver all of that?</div>
</div>

<!-- SLIDE 3: The Solution (Declare → Compile → Execute) -->
<div class="slide">
  <span class="slide-label">The Architecture</span>
  <span class="slide-num">3 / 6</span>
  <div class="accent-bar indigo"></div>
  <div class="hook" style="font-size:1.3em;">You describe intent once.<br><em>The compiler handles scheduling.</em></div>
  <div class="lifecycle-bar">
    <div class="phase declare">
      Declare
      <span class="phase-sub">passes + resources</span>
    </div>
    <div class="phase compile">
      Compile
      <span class="phase-sub">sort, cull, alias, sync</span>
    </div>
    <div class="phase execute">
      Execute
      <span class="phase-sub">record GPU commands</span>
    </div>
  </div>
  <div class="sub" style="margin-top:.8em;">From the same declarations, the compiler derives order, synchronization, memory reuse, and overlap.</div>
  <div class="sub" style="margin-top:.6em;">Sounds great — but without it, here's what your team hits:</div>
</div>

<!-- SLIDE 4: Sound familiar? -->
<div class="slide">
  <span class="slide-label">Why This Matters</span>
  <span class="slide-num">4 / 6</span>
  <div class="accent-bar green"></div>
  <div class="hook" style="font-size:1.15em;">Sound familiar?</div>
  <ul class="step-list">
    <li>
      <span class="step-num" style="background:rgba(239,68,68,.12);color:#ef4444;border-color:rgba(239,68,68,.25);">&#x2717;</span>
      <div><strong>&ldquo;We&rsquo;re out of VRAM &mdash; again.&rdquo;</strong> Every new pass adds pressure. Manual aliasing doesn&rsquo;t scale.</div>
    </li>
    <li>
      <span class="step-num" style="background:rgba(239,68,68,.12);color:#ef4444;border-color:rgba(239,68,68,.25);">&#x2717;</span>
      <div><strong>&ldquo;Why did frame time spike?&rdquo;</strong> Over-broad barriers stall queues nobody predicted from the code.</div>
    </li>
    <li>
      <span class="step-num" style="background:rgba(239,68,68,.12);color:#ef4444;border-color:rgba(239,68,68,.25);">&#x2717;</span>
      <div><strong>&ldquo;Adding a pass broke three others.&rdquo;</strong> Implicit ordering turns every changelist into a regression risk.</div>
    </li>
    <li>
      <span class="step-num" style="background:rgba(239,68,68,.12);color:#ef4444;border-color:rgba(239,68,68,.25);">&#x2717;</span>
      <div><strong>&ldquo;Device lost &mdash; no repro.&rdquo;</strong> Silent read-before-write only crashes on some GPUs, some drivers.</div>
    </li>
  </ul>
  <div class="compare-row" style="grid-template-columns:1fr;margin-top:.6em;">
    <div class="compare-box good" style="text-align:center;">
      <div class="title">Frame graphs fix all four.</div>
      <div class="body">Declare intent &rarr; the compiler handles ordering, sync, aliasing, and overlap.</div>
    </div>
  </div>
  <div class="sub" style="margin-top:.6em;">The series walks you through building it:</div>
</div>
<!-- SLIDE 5: What the series covers -->
<div class="slide">
  <span class="slide-label">The Series</span>
  <span class="slide-num">5 / 6</span>
  <div class="accent-bar gold"></div>
  <div class="hook" style="font-size:1.15em;">Four articles. Each one builds on the last.</div>
  <ul class="step-list">
    <li>
      <span class="step-num indigo">I</span>
      <div><strong>Theory</strong> — Why frame pacing, stability, and memory behavior improve.</div>
    </li>
    <li>
      <span class="step-num">II</span>
      <div><strong>Build It</strong> — 3 practical C++ iterations from prototype to functional MVP.</div>
    </li>
    <li>
      <span class="step-num green">III</span>
      <div><strong>Beyond MVP</strong> — Async compute, split barriers, and better GPU utilization from the same hardware.</div>
    </li>
    <li>
      <span class="step-num indigo">IV</span>
      <div><strong>Production Engines</strong> — How teams ship this model reliably at scale.</div>
    </li>
  </ul>
</div>

<!-- SLIDE 6: CTA + Discussion -->
<div class="slide">
  <span class="slide-label">Deep Spark</span>
  <span class="slide-num">6 / 6</span>
  <div class="accent-bar"></div>
  <div class="hook" style="font-size:1.3em;">The full series is live.</div>
  <div class="sub">4 articles. Theory → code → optimization → production. Each one builds on the last.</div>
  <div class="cta-block">
    <div class="cta-title">→ Link in the first comment</div>
    <div class="cta-body">Start with Part I (Theory) — it takes ~20 min and sets up everything else.</div>
  </div>
  <div class="discuss-block">
    <div class="discuss-q">What's the hardest part of render graph implementation you've hit?</div>
    <div class="discuss-hint">Barrier debugging? Aliasing edge cases? Async compute profiling? Let's hear it — I'll respond to every comment.</div>
  </div>
</div>

---

<!-- ═══════════════════════════════════════════════════════════════
     POST 2 — REMINDER POST
     ═══════════════════════════════════════════════════════════════ -->

## Post 2 — Reminder (Read The Series)

<div class="post-caption">
Reminder post for people who saw/skipped the entry carousel.
</div>

<div class="li-post-text">
<div class="li-hook">If your renderer still has pass-order crashes, queue stalls, and VRAM spikes — this is your reminder.</div>
<div class="li-body">
Post 1 explained the value. This one is the practical reminder: what you get when you implement it.
<br><br>
You get the 3-step C++ path, crash-prevention mechanism, stall-reduction mechanism, and production patterns used in large engines.
<br><br>
If you postponed reading the series, start now — this is the fastest way to reduce rendering chaos without rewriting everything at once.
</div>
<div class="li-cta">Full series → first comment</div>
<div class="li-tags">#rendering #gpu #graphicsprogramming #gamedev #framegraph #cpp #vulkan #d3d12 #ue5</div>
</div>

<!-- Slide 1: Reminder hook -->
<div class="slide">
  <span class="slide-label">Frame Graph — Reminder</span>
  <span class="slide-num">1 / 6</span>
  <div class="hero-metric">
    <div class="value">50%</div>
    <div class="kicker">Less transient VRAM (up to)</div>
    <div class="meta">If you implement lifetime aliasing</div>
  </div>
  <div class="hook" style="font-size:1.25em;">Reminder: this is the same system that cuts crashes, stalls, and memory spikes.</div>
  <div class="sub">Not hype — these are implementation outcomes from the 4-part series.</div>
  <div class="micro">Swipe →</div>
</div>

<!-- Slide 2: Build path -->
<div class="slide">
  <span class="slide-label">Build Path</span>
  <span class="slide-num">2 / 6</span>
  <div class="accent-bar gold"></div>
  <div class="hook" style="font-size:1.18em;">You can ship this incrementally in 3 steps.</div>
  <div class="kpi-band">
    <div class="kpi">
      <div class="v" style="color:#818cf8;">v1</div>
      <div class="k">Scaffold</div>
    </div>
    <div class="kpi">
      <div class="v" style="color:#FF9F1C;">v2</div>
      <div class="k">Deps + barriers</div>
    </div>
    <div class="kpi">
      <div class="v" style="color:#22c55e;">v3</div>
      <div class="k">Aliasing</div>
    </div>
  </div>
  <div class="sub">Part II shows complete C++ iterations from simple scaffold to memory savings.</div>
</div>

<!-- Slide 3: Crash prevention mechanism -->
<div class="slide">
  <span class="slide-label">Crash Prevention</span>
  <span class="slide-num">3 / 6</span>
  <div class="accent-bar red" style="background:#ef4444;"></div>
  <div class="hook" style="font-size:1.2em;">How crashes get prevented:</div>
  <div class="pipe-diagram">
    pass(<span class="pass">"GBuffer"</span>).write(albedo).write(depth)<br>
    pass(<span class="pass">"Lighting"</span>).read(albedo).read(depth).write(hdr)<br>
    pass(<span class="pass">"Tone"</span>).read(hdr).write(backbuffer)<br><br>
    <span style="color:rgba(255,255,255,.35);">compile → edge validation + state chain + required transitions</span>
  </div>
  <div class="sub">Missing read-before-write edges and invalid transition paths are caught before they become device-loss or frame-corruption crashes.</div>
</div>

<!-- Slide 4: Stall reduction mechanism -->
<div class="slide">
  <span class="slide-label">Stall Reduction</span>
  <span class="slide-num">4 / 6</span>
  <div class="accent-bar green"></div>
  <div class="hook" style="font-size:1.15em;">How long-frame spikes get reduced:</div>
  <div class="benefit-grid">
    <div class="benefit-card">
      <div class="b-tag perf">Scheduling</div>
      <div class="b-main">Run independent passes earlier</div>
      <div class="b-sub">Queue overlap reduces idle bubbles.</div>
    </div>
    <div class="benefit-card">
      <div class="b-tag stab">Synchronization</div>
      <div class="b-main">Insert only required barriers</div>
      <div class="b-sub">Avoid full-queue forced waits.</div>
    </div>
    <div class="benefit-card" style="grid-column:1 / -1;">
      <div class="b-tag qual">Result</div>
      <div class="b-main">Fewer long-frame spikes</div>
      <div class="b-sub">Cleaner compile decisions lead to more stable frame pacing.</div>
    </div>
  </div>
</div>

<!-- Slide 5: Production proof -->
<div class="slide">
  <span class="slide-label">Production Proof</span>
  <span class="slide-num">5 / 6</span>
  <div class="accent-bar indigo"></div>
  <div class="hook" style="font-size:1.15em;">Same model, production scale.</div>
  <div class="compare-row">
    <div class="compare-box bad">
      <div class="title">MVP</div>
      <div class="body">Single thread, fresh allocs each frame, per-resource barriers, 5-10 passes.</div>
    </div>
    <div class="compare-arrow">→</div>
    <div class="compare-box good">
      <div class="title">Production</div>
      <div class="body">Parallel recording, cross-frame pooling, barrier batching, hundreds of passes.</div>
    </div>
  </div>
  <div class="sub">Part IV maps the same fundamentals to UE5/Frostbite-style production constraints.</div>
</div>

<!-- Slide 6: Reminder CTA -->
<div class="slide">
  <span class="slide-label">Reminder CTA</span>
  <span class="slide-num">6 / 6</span>
  <div class="accent-bar"></div>
  <div class="hook" style="font-size:1.2em;">Entry + implementation + optimization + production — all live.</div>
  <div class="sub">If your pipeline still suffers from crashes, memory pressure, and stalls, this is your reminder to run through the series now.</div>
  <div class="cta-block">
    <div class="cta-title">→ Link in the first comment</div>
    <div class="cta-body">Start with Entry (Post 1) if you need the why, then continue through Parts II–IV for code and production patterns.</div>
  </div>
  <div class="discuss-block">
    <div class="discuss-q">What is your current bottleneck: crashes, memory, or stalls?</div>
    <div class="discuss-hint">Drop your case and I’ll point to the exact part that helps first.</div>
  </div>
</div>

---

<!-- ═══════════════════════════════════════════════════════════════
     TWITTER/X — THREAD IMAGES (16:9 landscape, 1600×900)
     Adapted from LinkedIn carousel. 5 images + 1 text-only CTA tweet.
     ═══════════════════════════════════════════════════════════════ -->

<style>
/* ── Twitter 16:9 slide override ─────────────────────────────── */
.slide.tw-slide {
  aspect-ratio: 16 / 9;
  padding: 1.8em 3em;
  justify-content: center;
  align-items: center;
  text-align: center;
  margin-bottom: 1.4em;
  overflow: hidden;
}

/* Slide label for Twitter */
.tw-slide .slide-label {
  display: block;
  font-size: clamp(0.88rem, 0.86vw, 1.02rem);
  font-weight: 800;
  letter-spacing: .14em;
  text-transform: uppercase;
  color: #5ec6ff;
  margin-bottom: .4em;
}

/* Hero metric in landscape */
.tw-slide .hero-metric {
  background: transparent;
  border: 0;
  box-shadow: none;
  padding: 0;
  margin: 0 0 .2em;
}
.tw-slide .hero-metric .value {
  font-size: clamp(4.5rem, 5.8vw, 6.5rem);
  line-height: .88;
}
.tw-slide .hero-metric .kicker {
  font-size: clamp(1rem, 0.96vw, 1.2rem);
  margin-top: .2em;
  color: rgba(255,255,255,.82);
}
.tw-slide .hero-metric .meta {
  font-size: clamp(0.9rem, 0.88vw, 1.06rem);
  margin-top: .15em;
}

/* Hook text in landscape */
.tw-slide .hook {
  max-width: 88%;
  margin: .05em auto .1em;
  text-align: center;
  font-size: clamp(1.5rem, 2vw, 2.15rem);
}

/* Sub text */
.tw-slide .sub {
  max-width: 80%;
  margin: .3em auto 0;
  text-align: center;
  font-size: clamp(0.96rem, 0.94vw, 1.12rem);
}

/* Pills inline */
.tw-slide .hook-pills {
  margin-top: .5em;
  gap: .2em .5em;
  justify-content: center;
}
.tw-slide .hook-pill {
  background: transparent;
  border: 0;
  box-shadow: none;
  padding: 0;
  color: rgba(255,255,255,.74);
  font-size: clamp(0.94rem, 0.88vw, 1.08rem);
  letter-spacing: .08em;
}
.tw-slide .hook-pill:not(:last-child)::after {
  content: '·';
  margin-left: .45em;
  color: rgba(255,255,255,.36);
}

/* No "Swipe" on Twitter */
.tw-slide .micro { display: none; }

/* Benefit grid: 2×2 for landscape */
.tw-slide .benefit-grid {
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap: .35em;
  max-width: 92%;
  margin: .4em auto 0;
}
.tw-slide .benefit-card {
  padding: .45em .6em .4em;
}
.tw-slide .benefit-card .b-tag {
  font-size: clamp(0.82rem, 0.8vw, 0.96rem);
  margin-bottom: .2em;
}
.tw-slide .benefit-card .b-main {
  font-size: clamp(0.92rem, 0.92vw, 1.08rem);
}
.tw-slide .benefit-card .b-sub {
  font-size: clamp(0.86rem, 0.84vw, 1rem);
  margin-top: .15em;
  line-height: 1.35;
}

/* Lifecycle bar: horizontal fills better in 16:9 */
.tw-slide .lifecycle-bar {
  max-width: 88%;
  margin: .5em auto;
}
.tw-slide .lifecycle-bar .phase {
  padding: .55em .7em;
  font-size: clamp(0.96rem, 0.94vw, 1.12rem);
}

/* Step list */
.tw-slide .step-list {
  max-width: 82%;
  margin: .4em auto;
  text-align: left;
}

/* Pain point list */
.tw-slide .step-list li {
  margin-bottom: .4em;
  font-size: clamp(0.94rem, 0.92vw, 1.08rem);
}

/* Compare box */
.tw-slide .compare-row {
  max-width: 72%;
  margin: .3em auto;
}
.tw-slide .compare-box .title {
  font-size: clamp(0.96rem, 0.94vw, 1.12rem);
}

/* URL watermark in corner */
.tw-slide .tw-url {
  position: absolute;
  bottom: .6em;
  right: 1em;
  font-size: clamp(0.78rem, 0.76vw, 0.92rem);
  color: rgba(255,255,255,.3);
  letter-spacing: .04em;
  font-weight: 600;
  z-index: 2;
}

/* Series roadmap: horizontal timeline for 16:9 */
.tw-timeline {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: .4em;
  max-width: 92%;
  margin: .4em auto 0;
}
.tw-timeline .tw-step {
  text-align: center;
  padding: .5em .4em;
  border-radius: 10px;
  background: linear-gradient(145deg, rgba(255,255,255,.06), rgba(255,255,255,.02));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.05);
  position: relative;
}
.tw-timeline .tw-step:not(:last-child)::after {
  content: '→';
  position: absolute;
  right: -.65em;
  top: 50%;
  transform: translateY(-50%);
  color: rgba(255,255,255,.2);
  font-size: 1.2rem;
  font-weight: 300;
  z-index: 2;
}
.tw-timeline .tw-step .tw-num {
  font-size: clamp(1.3rem, 1.5vw, 1.7rem);
  font-weight: 900;
  line-height: 1;
  margin-bottom: .15em;
}
.tw-timeline .tw-step .tw-num.indigo { color: #818cf8; }
.tw-timeline .tw-step .tw-num.orange { color: #FF9F1C; }
.tw-timeline .tw-step .tw-num.green { color: #22c55e; }
.tw-timeline .tw-step .tw-title {
  font-size: clamp(1.02rem, 1vw, 1.18rem);
  font-weight: 800;
  color: #fff;
  margin-bottom: .2em;
}
.tw-timeline .tw-step .tw-desc {
  font-size: clamp(0.92rem, 0.88vw, 1.06rem);
  color: rgba(255,255,255,.55);
  line-height: 1.4;
}

/* Tweet text preview */
.tw-tweet-text {
  background: linear-gradient(145deg, rgba(255,255,255,.05), rgba(255,255,255,.015));
  border: 0;
  border-radius: 10px;
  padding: 1.2em 1.4em;
  margin-bottom: 1em;
  font-size: clamp(1rem, 1vw, 1.16rem);
  line-height: 1.62;
  color: rgba(255,255,255,.68);
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
  max-width: 520px;
}
.tw-tweet-text .tw-handle {
  font-weight: 800;
  color: rgba(255,255,255,.9);
  font-size: clamp(1.04rem, 1vw, 1.2rem);
  margin-bottom: .4em;
}
.tw-tweet-text .tw-body {
  white-space: pre-line;
}
.tw-tweet-text .tw-tags {
  color: #5ec6ff;
  margin-top: .5em;
}

/* Responsive */
@media (max-width: 600px) {
  .slide.tw-slide { padding: 1.8em 1.4em; }
  .tw-slide .hero-metric .value { font-size: 4rem; }
  .tw-slide .hook { max-width: 100%; font-size: 1.6rem; }
  .tw-slide .benefit-grid { grid-template-columns: 1fr; max-width: 100%; }
  .tw-timeline { grid-template-columns: 1fr 1fr; }
  .tw-timeline .tw-step:not(:last-child)::after { display: none; }
}
</style>

## Twitter/X — Thread Images (16:9)

<div class="post-caption">
Adapted from LinkedIn carousel for Twitter thread. 5 images (1600×900 landscape) + text-only CTA tweet.
Post 30 min after LinkedIn (8:42). Full plan: <code>promotion/twitter-promo-plan.md</code>
</div>

<!-- Tweet 1 text -->
<div class="tw-tweet-text">
<div class="tw-handle">Tweet 1/6 — Hook 🧵</div>
<div class="tw-body">Frame graphs can cut transient VRAM in half.

Fewer GPU stalls. Less crashes. Faster iteration.

Here's why every renderer should have one — thread 🧵

🔗 stoleckipawel.dev/posts/series/frame-graph</div>
<div class="tw-tags">#gamedev #rendering</div>
</div>

<!-- TW SLIDE 1: Hook (16:9) -->
<div class="slide tw-slide">
  <span class="slide-label">Frame Graph Series</span>
  <div class="hero-metric">
    <div class="value">50%</div>
    <div class="kicker">Less transient VRAM (up to)</div>
    <div class="meta">Automatic aliasing &middot; zero manual tracking</div>
  </div>
  <div class="hook">Fewer GPU stalls. Less crashes.<br>Faster iteration. Ship with confidence.</div>
  <div class="hook-pills">
    <span class="hook-pill">Memory</span>
    <span class="hook-pill">GPU Performance</span>
    <span class="hook-pill">Productivity</span>
    <span class="hook-pill">Stability</span>
  </div>
  <span class="tw-url">stoleckipawel.dev/posts/series/frame-graph</span>
</div>

<!-- Tweet 2 text -->
<div class="tw-tweet-text">
<div class="tw-handle">Tweet 2/6 — Benefits</div>
<div class="tw-body">What a frame graph compiler actually gives you:

&bull; Memory — resource aliasing reclaims VRAM across passes
&bull; GPU perf — split barriers + async compute reduce stalls
&bull; Productivity — compiler orders passes and places barriers
&bull; Stability — dependency tracking catches bugs at build time</div>
</div>

<!-- TW SLIDE 2: What this fixes (16:9, 2×2 grid) -->
<div class="slide tw-slide">
  <span class="slide-label">What This Fixes</span>
  <div class="hook">What a frame graph compiler delivers:</div>
  <div class="benefit-grid">
    <div class="benefit-card">
      <div class="b-tag mem">Memory</div>
      <div class="b-main">Up to 50% less transient VRAM</div>
      <div class="b-sub">Resource aliasing reclaims memory across passes.</div>
    </div>
    <div class="benefit-card">
      <div class="b-tag perf">GPU Performance</div>
      <div class="b-main">Fewer pipeline stalls</div>
      <div class="b-sub">Split barriers + async compute keep queues full.</div>
    </div>
    <div class="benefit-card">
      <div class="b-tag prod">Productivity</div>
      <div class="b-main">No manual pass wiring</div>
      <div class="b-sub">Compiler orders passes and places barriers.</div>
    </div>
    <div class="benefit-card">
      <div class="b-tag stab">Stability</div>
      <div class="b-main">Bugs caught at build time</div>
      <div class="b-sub">Dependency tracking replaces guesswork.</div>
    </div>
  </div>
  <span class="tw-url">stoleckipawel.dev/posts/series/frame-graph</span>
</div>

<!-- Tweet 3 text -->
<div class="tw-tweet-text">
<div class="tw-handle">Tweet 3/6 — How it works</div>
<div class="tw-body">The core model is 3 phases:

1. Declare — register passes + resources
2. Compile — sort, cull, alias, sync
3. Execute — record GPU commands

You describe intent once. The compiler handles scheduling.</div>
</div>

<!-- TW SLIDE 3: Architecture (16:9) -->
<div class="slide tw-slide">
  <span class="slide-label">The Architecture</span>
  <div class="hook" style="font-size:1.3em;">You describe intent once.<br><em>The compiler handles scheduling.</em></div>
  <div class="lifecycle-bar">
    <div class="phase declare">
      Declare
      <span class="phase-sub">passes + resources</span>
    </div>
    <div class="phase compile">
      Compile
      <span class="phase-sub">sort, cull, alias, sync</span>
    </div>
    <div class="phase execute">
      Execute
      <span class="phase-sub">record GPU commands</span>
    </div>
  </div>
  <div class="sub" style="margin-top:.6em;">Same declarations &rarr; order, sync, memory reuse, and overlap.</div>
  <span class="tw-url">stoleckipawel.dev/posts/series/frame-graph</span>
</div>

<!-- Tweet 4 text -->
<div class="tw-tweet-text">
<div class="tw-handle">Tweet 4/6 — Pain points</div>
<div class="tw-body">Sound familiar?

✗ "We're out of VRAM — again"
✗ "Why did frame time spike?"
✗ "Adding a pass broke three others"
✗ "Device lost — no repro"

Frame graphs fix all four with one abstraction.</div>
</div>

<!-- TW SLIDE 4: Sound familiar? (16:9) -->
<div class="slide tw-slide">
  <div class="hook" style="font-size:1.15em;">Sound familiar?</div>
  <ul class="step-list">
    <li>
      <span class="step-num" style="background:rgba(239,68,68,.12);color:#ef4444;border-color:rgba(239,68,68,.25);">&#x2717;</span>
      <div><strong>&ldquo;We&rsquo;re out of VRAM &mdash; again.&rdquo;</strong></div>
    </li>
    <li>
      <span class="step-num" style="background:rgba(239,68,68,.12);color:#ef4444;border-color:rgba(239,68,68,.25);">&#x2717;</span>
      <div><strong>&ldquo;Why did frame time spike?&rdquo;</strong></div>
    </li>
    <li>
      <span class="step-num" style="background:rgba(239,68,68,.12);color:#ef4444;border-color:rgba(239,68,68,.25);">&#x2717;</span>
      <div><strong>&ldquo;Adding a pass broke three others.&rdquo;</strong></div>
    </li>
    <li>
      <span class="step-num" style="background:rgba(239,68,68,.12);color:#ef4444;border-color:rgba(239,68,68,.25);">&#x2717;</span>
      <div><strong>&ldquo;Device lost &mdash; no repro.&rdquo;</strong></div>
    </li>
  </ul>
  <div class="compare-row" style="grid-template-columns:1fr;margin-top:.4em;">
    <div class="compare-box good" style="text-align:center;">
      <div class="title">Frame graphs fix all four.</div>
    </div>
  </div>
  <span class="tw-url">stoleckipawel.dev/posts/series/frame-graph</span>
</div>

<!-- Tweet 5 text -->
<div class="tw-tweet-text">
<div class="tw-handle">Tweet 5/6 — Series roadmap</div>
<div class="tw-body">I wrote a 4-part deep dive:

I. Theory — why pacing, stability, and memory improve
II. Build It — 3 C++ iterations, prototype to MVP
III. Beyond MVP — async compute, split barriers
IV. Production — UE5/Frostbite-scale patterns

Each builds on the last.</div>
</div>

<!-- TW SLIDE 5: Series roadmap as horizontal timeline (16:9) -->
<div class="slide tw-slide">
  <span class="slide-label">The Series</span>
  <div class="hook" style="font-size:1.15em;">Four articles. Each one builds on the last.</div>
  <div class="tw-timeline">
    <div class="tw-step">
      <div class="tw-num indigo">I</div>
      <div class="tw-title">Theory</div>
      <div class="tw-desc">Why pacing, stability &amp; memory improve</div>
    </div>
    <div class="tw-step">
      <div class="tw-num orange">II</div>
      <div class="tw-title">Build It</div>
      <div class="tw-desc">3 C++ iterations, prototype to MVP</div>
    </div>
    <div class="tw-step">
      <div class="tw-num green">III</div>
      <div class="tw-title">Beyond MVP</div>
      <div class="tw-desc">Async compute &amp; split barriers</div>
    </div>
    <div class="tw-step">
      <div class="tw-num indigo">IV</div>
      <div class="tw-title">Production</div>
      <div class="tw-desc">UE5/Frostbite-scale patterns</div>
    </div>
  </div>
  <span class="tw-url">stoleckipawel.dev/posts/series/frame-graph</span>
</div>

<!-- Tweet 6 text (no image) -->
<div class="tw-tweet-text">
<div class="tw-handle">Tweet 6/6 — CTA (no image)</div>
<div class="tw-body">Full series is live.

Start with Part I (Theory, ~20 min read) — it sets up everything else.

🔗 stoleckipawel.dev/posts/series/frame-graph

What's the hardest part of render graph implementation you've hit? Barrier debugging? Aliasing edge cases?</div>
<div class="tw-tags">#gamedev #rendering #graphicsprogramming</div>
</div>

</div><!-- end promo-lab -->
