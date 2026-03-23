---
title: "HGDC 2025 Panel Promo Lab"
date: 2026-03-08
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

<style>
.panel-promo-lab {
  --panel-photo: url('/images/HDCPanelPhoto.png');
  max-width: 660px;
  margin: 0 auto;
}
.panel-promo-lab h2 {
  font-size: clamp(1.18rem, 1.25vw, 1.4rem);
  font-weight: 700;
  letter-spacing: .04em;
  text-transform: uppercase;
  color: #FF9F1C;
  margin: 2.6em 0 .7em;
  padding-bottom: .38em;
  border-bottom: 1px solid rgba(255,159,28,.22);
}
.panel-promo-lab h2:first-child { margin-top: 0; }
.panel-promo-lab .post-caption {
  font-size: clamp(0.94rem, 0.95vw, 1.1rem);
  opacity: .68;
  margin-bottom: 1.5em;
  line-height: 1.58;
}
.panel-promo-lab .work-note {
  background: linear-gradient(145deg, rgba(255,255,255,.05), rgba(255,255,255,.015));
  border-radius: 10px;
  padding: 1em 1.15em;
  margin: 0 0 1.4em;
  font-size: clamp(0.92rem, 0.92vw, 1.06rem);
  line-height: 1.56;
  color: rgba(255,255,255,.66);
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.panel-promo-lab .work-note strong { color: #fff; }

.panel-slide {
  position: relative;
  aspect-ratio: 4 / 5;
  width: 100%;
  background: linear-gradient(145deg, #0a0c12 0%, #0b0b0e 45%, #101522 100%);
  border-radius: 6px;
  overflow: hidden;
  margin-bottom: 1.4em;
  padding: 1.95em 1.75em;
  display: flex;
  flex-direction: column;
  justify-content: flex-start;
  border: 0;
  box-shadow: 0 24px 46px rgba(0,0,0,.38);
  isolation: isolate;
}
.panel-slide::before {
  content: '';
  position: absolute;
  inset: 0;
  background:
    radial-gradient(ellipse at 18% 14%, rgba(255,159,28,.11) 0%, transparent 52%),
    radial-gradient(ellipse at 88% 86%, rgba(94,198,255,.12) 0%, transparent 45%),
    linear-gradient(120deg, rgba(255,255,255,.03) 0%, rgba(255,255,255,0) 35%);
  pointer-events: none;
}
.panel-slide::after {
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
.panel-slide * { position: relative; z-index: 1; }

.panel-slide .slide-label {
  display: block;
  font-size: clamp(0.88rem, 0.84vw, 1rem);
  letter-spacing: .1em;
  text-transform: uppercase;
  color: #5ec6ff;
  font-weight: 800;
  margin-bottom: .65em;
}
.panel-slide .slide-num {
  position: absolute;
  top: 1.15em;
  right: 1.2em;
  font-size: clamp(0.82rem, 0.8vw, 0.94rem);
  color: rgba(255,255,255,.34);
  font-weight: 700;
  letter-spacing: .08em;
  text-transform: uppercase;
}
.panel-slide .hook {
  font-size: clamp(1.72rem, 2.2vw, 2.28rem);
  font-weight: 900;
  line-height: 1.08;
  letter-spacing: -.01em;
  color: #fff;
  margin-bottom: .4em;
}
.panel-slide .hook .accent { color: #FF9F1C; }
.panel-slide .hook .cyan { color: #5ec6ff; }
.panel-slide .hook .green { color: #4ade80; }
.panel-slide .sub {
  font-size: clamp(0.96rem, 0.92vw, 1.06rem);
  line-height: 1.42;
  color: rgba(255,255,255,.66);
}
.panel-slide .sub strong { color: rgba(255,255,255,.9); }
.panel-slide .micro {
  font-size: clamp(0.86rem, 0.82vw, 0.96rem);
  color: rgba(255,255,255,.42);
  margin-top: auto;
  padding-top: .8em;
  line-height: 1.36;
}
.panel-slide .hook-aside {
  font-size: clamp(0.82rem, 0.8vw, 0.92rem);
  color: rgba(255,255,255,.58);
  margin: -.05em 0 .65em;
  letter-spacing: .01em;
}
.panel-slide .accent-bar {
  width: 72px;
  height: 6px;
  border-radius: 999px;
  background: linear-gradient(90deg, #FF9F1C, #FFD166);
  margin-bottom: .8em;
}

.panel-hero-grid {
  display: grid;
  grid-template-columns: minmax(0, 1.08fr) minmax(0, .92fr);
  gap: 1.1em;
  align-items: stretch;
}
.panel-slide.hook-layout {
  justify-content: flex-start;
  padding: 1.75em 2em 1.45em;
}
.hook-copy-top {
  margin-bottom: .85em;
}
.hook-copy-bottom {
  margin-top: .75em;
}
.panel-photo-card {
  position: relative;
  min-height: 350px;
  border-radius: 14px;
  overflow: hidden;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.1), 0 16px 28px rgba(0,0,0,.24);
  background:
    linear-gradient(180deg, rgba(4,8,20,.08), rgba(4,8,20,.55)),
    var(--panel-photo) center/cover no-repeat,
    linear-gradient(145deg, rgba(255,255,255,.08), rgba(255,255,255,.02));
}
.panel-slide.hook-layout .panel-photo-card {
  min-height: 205px;
  width: 100%;
  background-position: center center;
}
.panel-slide.hook-layout .accent-bar {
  margin-bottom: .7em;
}
.panel-slide.hook-layout .hook {
  font-size: clamp(1.72rem, 2.25vw, 2.2rem);
  line-height: 1.1;
  margin-bottom: 0;
}
.panel-slide.variant-image-first {
  justify-content: flex-start;
}
.panel-slide.variant-editorial,
.panel-slide.variant-asym,
.panel-slide.variant-fullbleed,
.panel-slide.variant-minimal,
.panel-slide.variant-captioned {
  justify-content: flex-start;
}
.panel-slide.variant-editorial .panel-photo-card {
  min-height: 300px;
  margin: 1em 0 1.1em;
}
.variant-editorial .variant-footer-line {
  max-width: 540px;
}
.variant-asym-grid {
  display: grid;
  grid-template-columns: minmax(0, .9fr) minmax(0, 1.1fr);
  gap: 1em;
  align-items: stretch;
  min-height: 100%;
}
.variant-asym-copy {
  display: flex;
  flex-direction: column;
  justify-content: space-between;
}
.variant-asym-copy .sub {
  max-width: 280px;
}
.variant-asym-photo {
  position: relative;
  min-height: 440px;
}
.variant-asym-photo .panel-photo-card {
  min-height: 100%;
  height: 100%;
}
.variant-inline-speakers {
  font-size: clamp(0.95rem, 0.92vw, 1.05rem);
  line-height: 1.55;
  color: rgba(255,255,255,.66);
}
.variant-inline-speakers strong {
  color: #fff;
}
.panel-slide.variant-fullbleed {
  padding: 0;
  overflow: hidden;
}
.variant-fullbleed .panel-photo-card {
  min-height: 100%;
  height: 100%;
  border-radius: 0;
  box-shadow: none;
  background:
    linear-gradient(180deg, rgba(4,8,20,.9) 0%, rgba(4,8,20,.32) 32%, rgba(4,8,20,.72) 100%),
    var(--panel-photo) center/cover no-repeat,
    linear-gradient(145deg, rgba(255,255,255,.08), rgba(255,255,255,.02));
}
.variant-fullbleed-content {
  position: absolute;
  inset: 0;
  padding: 1.8em 1.7em;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
}
.variant-fullbleed-top {
  max-width: 440px;
}
.variant-fullbleed-top .hook {
  font-size: clamp(2.2rem, 2.7vw, 2.9rem);
  line-height: 1.02;
}
.variant-fullbleed-top .sub {
  margin-top: .8em;
  color: rgba(255,255,255,.82);
}
.variant-fullbleed-bottom {
  display: flex;
  justify-content: space-between;
  gap: 1em;
  align-items: end;
}
.variant-speaker-stack {
  display: flex;
  flex-direction: column;
  gap: .25em;
  font-size: clamp(0.9rem, 0.88vw, 1rem);
  color: rgba(255,255,255,.82);
}
.variant-speaker-stack strong { color: #fff; }
.variant-slide-mark {
  font-size: clamp(0.88rem, 0.84vw, .98rem);
  text-transform: uppercase;
  letter-spacing: .08em;
  color: rgba(255,255,255,.52);
}
.panel-slide.variant-minimal {
  padding: 1.8em 1.8em 1.55em;
}
.variant-minimal-head {
  display: grid;
  grid-template-columns: auto 1fr;
  gap: 1em;
  align-items: start;
}
.variant-minimal-index {
  font-size: clamp(0.92rem, 0.9vw, 1rem);
  color: rgba(255,255,255,.42);
  letter-spacing: .08em;
  text-transform: uppercase;
}
.variant-minimal .panel-photo-card {
  min-height: 210px;
  margin: 1.15em 0 .95em;
}
.variant-minimal-list {
  display: grid;
  grid-template-columns: 1fr;
  gap: .38em;
  font-size: clamp(0.96rem, 0.92vw, 1.05rem);
  color: rgba(255,255,255,.72);
}
.variant-minimal-list strong { color: #fff; }
.panel-slide.variant-captioned {
  padding-bottom: 2em;
}
.variant-captioned .panel-photo-card {
  min-height: 320px;
  margin: .75em 0 0;
}
.variant-floating-card {
  margin: -3.4em 1.2em 0 auto;
  max-width: 340px;
  padding: 1.1em 1.15em;
  border-radius: 16px;
  background: linear-gradient(145deg, rgba(9,12,20,.92), rgba(18,26,38,.86));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.08), 0 18px 38px rgba(0,0,0,.26);
}
.variant-floating-card .sub {
  font-size: clamp(0.98rem, 0.94vw, 1.06rem);
  line-height: 1.45;
}
.variant-floating-card .variant-inline-speakers {
  margin-top: .8em;
  font-size: clamp(0.88rem, 0.86vw, .96rem);
}
.panel-slide.variant-image-first .panel-photo-card,
.panel-slide.variant-myth .panel-photo-card,
.panel-slide.variant-value .panel-photo-card {
  min-height: 255px;
  margin: .9em 0 1em;
}
.variant-footer-line {
  font-size: clamp(0.98rem, 0.92vw, 1.08rem);
  line-height: 1.45;
  color: rgba(255,255,255,.72);
}
.variant-footer-line strong {
  color: #fff;
}
.panel-slide.variant-split {
  justify-content: flex-start;
}
.variant-split-top {
  display: grid;
  grid-template-columns: minmax(0, 1fr) auto;
  gap: 1em;
  align-items: start;
}
.variant-kicker {
  font-size: clamp(0.94rem, 0.88vw, 1.04rem);
  text-transform: uppercase;
  letter-spacing: .08em;
  font-weight: 800;
  color: #5ec6ff;
  margin-bottom: .7em;
}
.variant-side-note {
  max-width: 210px;
  font-size: clamp(0.92rem, 0.88vw, 1.02rem);
  line-height: 1.42;
  color: rgba(255,255,255,.62);
  text-align: right;
}
.panel-slide.variant-split .panel-photo-card {
  min-height: 235px;
  margin: .9em 0 1em;
}
.panel-slide.variant-question {
  padding-top: 1.4em;
}
.panel-slide.variant-question .panel-photo-card {
  min-height: 360px;
  margin: .8em -2em 1em;
  border-radius: 0;
  box-shadow: none;
  background:
    linear-gradient(180deg, rgba(4,8,20,.78) 0%, rgba(4,8,20,.22) 34%, rgba(4,8,20,.58) 100%),
    var(--panel-photo) center/cover no-repeat,
    linear-gradient(145deg, rgba(255,255,255,.08), rgba(255,255,255,.02));
}
.variant-overlay-copy {
  margin-top: -330px;
  padding: 0 0 1.2em;
}
.variant-overlay-copy .hook {
  max-width: 480px;
}
.variant-overlay-copy .sub {
  max-width: 500px;
  color: rgba(255,255,255,.8);
}
.panel-slide.variant-question .panel-chip-row {
  margin-top: auto;
}
.panel-slide.variant-myth {
  justify-content: flex-start;
}
.panel-slide.variant-myth .hook .muted {
  color: rgba(255,255,255,.72);
}
.panel-slide.variant-myth .sub,
.panel-slide.variant-value .sub {
  max-width: 560px;
}
.panel-slide.variant-value {
  justify-content: flex-start;
}
.variant-bottom-note {
  margin-top: auto;
  padding-top: .9em;
  font-size: clamp(0.96rem, 0.9vw, 1.06rem);
  line-height: 1.44;
  color: rgba(255,255,255,.66);
}
.panel-slide.hook-layout .sub {
  font-size: clamp(1rem, 0.96vw, 1.08rem);
  line-height: 1.4;
}
.panel-slide.hook-layout .panel-chip-row {
  gap: .42em;
  margin-top: .62em;
}
.panel-slide.hook-layout .panel-chip {
  font-size: clamp(0.82rem, 0.8vw, 0.9rem);
  padding: .3em .58em;
}
.panel-slide.hook-layout .micro {
  font-size: clamp(0.84rem, 0.8vw, 0.94rem);
  line-height: 1.35;
  padding-top: .65em;
}
.panel-info-stack {
  display: flex;
  flex-direction: column;
  justify-content: center;
}
.panel-chip-row {
  display: flex;
  flex-wrap: wrap;
  gap: .5em;
  margin-top: .85em;
}
.panel-chip {
  font-size: clamp(0.92rem, 0.88vw, 1rem);
  letter-spacing: .015em;
  font-weight: 700;
  color: rgba(255,255,255,.9);
  border-radius: 12px;
  padding: .38em .72em;
  background: rgba(255,255,255,.035);
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.09);
}
.panel-chip.orange { background: rgba(255,159,28,.08); color: #ffb347; box-shadow: inset 0 0 0 1px rgba(255,159,28,.18); }
.panel-chip.cyan { background: rgba(94,198,255,.08); color: #7fd2ff; box-shadow: inset 0 0 0 1px rgba(94,198,255,.18); }
.panel-chip.green { background: rgba(74,222,128,.08); color: #67e39c; box-shadow: inset 0 0 0 1px rgba(74,222,128,.18); }
.panel-chip.indigo { background: rgba(129,140,248,.08); color: #9ca6ff; box-shadow: inset 0 0 0 1px rgba(129,140,248,.18); }

.persona-grid {
  display: grid;
  grid-template-columns: repeat(3, minmax(0, 1fr));
  gap: .5em;
  margin: .8em 0;
}
.persona-card,
.benefit-card,
.compare-card,
.quote-card {
  background: linear-gradient(145deg, rgba(255,255,255,.06), rgba(255,255,255,.02));
  border-radius: 10px;
  padding: .75em .75em .7em;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.06);
}
.persona-card .tag,
.benefit-card .tag,
.quote-card .tag {
  font-size: clamp(0.8rem, 0.78vw, 0.92rem);
  letter-spacing: .07em;
  text-transform: uppercase;
  font-weight: 800;
  margin-bottom: .3em;
}
.persona-card .tag.render { color: #FF9F1C; }
.persona-card .tag.story { color: #5ec6ff; }
.persona-card .tag.anim { color: #4ade80; }
.benefit-card .tag.gold { color: #FFD166; }
.benefit-card .tag.indigo { color: #818cf8; }
.benefit-card .tag.orange { color: #FF9F1C; }
.benefit-card .tag.green { color: #4ade80; }
.persona-card .title,
.benefit-card .title,
.compare-card .title,
.quote-card .title {
  font-size: clamp(0.92rem, 0.9vw, 1.02rem);
  font-weight: 800;
  color: #fff;
  line-height: 1.22;
}
.persona-card .body,
.benefit-card .body,
.compare-card .body,
.quote-card .body {
  margin-top: .28em;
  font-size: clamp(0.86rem, 0.84vw, 0.96rem);
  line-height: 1.38;
  color: rgba(255,255,255,.63);
}

.step-list {
  list-style: none;
  padding: 0;
  margin: .7em 0 0;
}
.step-list li {
  display: flex;
  gap: .55em;
  align-items: flex-start;
  margin-bottom: .6em;
  font-size: clamp(0.9rem, 0.88vw, 1rem);
  line-height: 1.38;
  color: rgba(255,255,255,.66);
}
.step-num {
  flex-shrink: 0;
  width: 24px;
  height: 24px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: .84rem;
  font-weight: 800;
  background: rgba(255,159,28,.12);
  color: #FF9F1C;
  box-shadow: inset 0 0 0 1px rgba(255,159,28,.26);
}
.step-list li strong { color: rgba(255,255,255,.92); }

.compare-grid {
  display: grid;
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap: .55em;
  margin-top: .8em;
}
.compare-card.bad .title { color: #f87171; }
.compare-card.good .title { color: #4ade80; }

.cta-block {
  margin-top: .9em;
  padding: .9em 1em;
  border-radius: 10px;
  background: linear-gradient(135deg, rgba(255,159,28,.08), rgba(255,159,28,.02));
  box-shadow: inset 0 0 0 1px rgba(255,159,28,.24), 0 10px 22px rgba(0,0,0,.2);
}
.cta-block .cta-title {
  font-size: clamp(0.96rem, 0.94vw, 1.08rem);
  font-weight: 800;
  color: #FF9F1C;
  margin-bottom: .25em;
}
.cta-block .cta-body {
  font-size: clamp(0.86rem, 0.84vw, 0.96rem);
  line-height: 1.42;
  color: rgba(255,255,255,.66);
}

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
  font-size: clamp(1rem, 0.96vw, 1.12rem);
  margin-top: .6em;
}

@media (max-width: 600px) {
  .panel-slide {
    aspect-ratio: auto;
    padding: 2.1em 1.7em;
  }
  .panel-hero-grid,
  .persona-grid,
  .compare-grid { grid-template-columns: 1fr; }
  .panel-photo-card { min-height: 270px; }
  .panel-slide.hook-layout {
    padding: 1.65em 1.45em 1.3em;
  }
  .panel-slide.hook-layout .panel-photo-card { min-height: 185px; }
  .variant-asym-grid,
  .variant-split-top { grid-template-columns: 1fr; }
  .variant-side-note { max-width: none; text-align: left; }
  .panel-slide.variant-question { padding-top: 1.2em; }
  .panel-slide.variant-question .panel-photo-card {
    min-height: 300px;
    margin: .7em -1.7em .9em;
  }
  .variant-overlay-copy {
    margin-top: -280px;
  }
  .variant-asym-photo { min-height: 250px; }
  .variant-fullbleed-top .hook { font-size: clamp(1.9rem, 7vw, 2.5rem); }
  .variant-fullbleed-bottom {
    flex-direction: column;
    align-items: start;
  }
  .variant-floating-card {
    margin: -2.4em .4em 0;
    max-width: none;
  }
}

.hook8-speakers {
  font-size: clamp(0.88rem, 0.86vw, .98rem);
  line-height: 1.5;
  color: rgba(255,255,255,.66);
}
.hook8-speakers strong { color: #fff; }
.hook8-bullets {
  display: grid;
  gap: .38em;
  margin: .9em 0 1em;
}
.hook8-bullets span {
  font-size: clamp(0.98rem, 0.94vw, 1.08rem);
  color: rgba(255,255,255,.78);
}
.hook8-bullets strong { color: #fff; }
.hook8-poster .hook {
  font-size: clamp(2.3rem, 3vw, 3.2rem);
  line-height: .96;
  max-width: 540px;
}
.hook8-poster .panel-photo-card {
  min-height: 300px;
  margin: 1em 0 .95em;
}
.hook8-overlay {
  padding: 0;
  overflow: hidden;
}
.hook8-overlay .panel-photo-card {
  height: 100%;
  min-height: 100%;
  border-radius: 0;
  box-shadow: none;
  background:
    linear-gradient(180deg, rgba(6,10,18,.15) 0%, rgba(6,10,18,0) 28%, rgba(6,10,18,.88) 100%),
    var(--panel-photo) center/cover no-repeat,
    linear-gradient(145deg, rgba(255,255,255,.08), rgba(255,255,255,.02));
}
.hook8-overlay-shell {
  position: absolute;
  inset: 0;
  padding: 1.6em;
  display: flex;
  flex-direction: column;
  justify-content: flex-end;
}
.hook8-overlay-shell .hook {
  font-size: clamp(2.45rem, 3.1vw, 3.35rem);
  line-height: .94;
  max-width: 470px;
}
.hook8-overlay-shell .sub { color: rgba(255,255,255,.84); max-width: 520px; }
.hook8-bulletslide .hook { max-width: 520px; }
.hook8-bulletslide .panel-photo-card { min-height: 190px; margin-top: auto; }
.hook8-sidecar {
  padding: 1.45em;
}
.hook8-sidecar-grid {
  display: grid;
  grid-template-columns: minmax(0, .82fr) minmax(0, 1.18fr);
  gap: 1em;
  min-height: 100%;
}
.hook8-sidecar-grid .panel-photo-card { min-height: 100%; height: 100%; }
.hook8-sidecar-copy {
  display: flex;
  flex-direction: column;
  justify-content: center;
  padding-left: 1.05em;
  border-left: 1px solid rgba(255,255,255,.08);
}
.hook8-sidecar-copy .hook { font-size: clamp(2rem, 2.5vw, 2.75rem); line-height: .98; }
.hook8-quiet .hook {
  font-size: clamp(2.2rem, 2.8vw, 3rem);
  line-height: .96;
  max-width: 400px;
}
.hook8-quiet .sub { max-width: 360px; }
.hook8-quiet .panel-photo-card { width: 62%; min-height: 220px; margin: 1.35em 0 0 auto; }
.hook8-captionband .panel-photo-card {
  min-height: 275px;
  margin-top: .9em;
  border-bottom-left-radius: 0;
  border-bottom-right-radius: 0;
}
.hook8-band {
  padding: 1em 1.05em;
  background: linear-gradient(145deg, rgba(12,17,27,.95), rgba(20,33,52,.88));
  border-bottom-left-radius: 16px;
  border-bottom-right-radius: 16px;
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.08);
}
.hook8-signal {
  padding: 0;
  overflow: hidden;
}
.hook8-signal-top {
  padding: 1.3em 1.5em 1.15em;
  background: linear-gradient(135deg, rgba(255,159,28,.18), rgba(94,198,255,.12));
}
.hook8-signal-top .hook { font-size: clamp(2.05rem, 2.5vw, 2.65rem); line-height: .98; }
.hook8-signal .panel-photo-card { min-height: 245px; margin: 1.05em 1.35em .85em; }
.hook8-signal-footer { padding: 0 1.5em 1.45em; }
.hook8-spotlight {
  padding: 1.5em;
}
.hook8-spotlight-stage {
  position: relative;
  min-height: 100%;
}
.hook8-spotlight-stage .panel-photo-card {
  min-height: 100%;
  height: 100%;
  background:
    radial-gradient(circle at 20% 18%, rgba(255,159,28,.25), rgba(255,159,28,0) 22%),
    linear-gradient(180deg, rgba(6,10,18,.24) 0%, rgba(6,10,18,.62) 100%),
    var(--panel-photo) center/cover no-repeat,
    linear-gradient(145deg, rgba(255,255,255,.08), rgba(255,255,255,.02));
}
.hook8-spotlight-card {
  position: absolute;
  left: 1.05em;
  right: 1.05em;
  bottom: 1.05em;
  padding: 1.05em 1.1em;
  border-radius: 18px;
  background: linear-gradient(145deg, rgba(7,10,18,.92), rgba(12,18,28,.84));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.08), 0 20px 38px rgba(0,0,0,.28);
}
@media (max-width: 600px) {
  .hook8-sidecar-grid { grid-template-columns: 1fr; }
  .hook8-sidecar-copy { border-left: 0; padding-left: 0; }
  .hook8-quiet .panel-photo-card { width: 100%; }
  .hook8-signal .panel-photo-card { margin: .9em 1em .8em; }
}

.hookrules-photo {
  position: relative;
  margin-left: -2em;
  margin-right: -2em;
  width: calc(100% + 4em);
  border-radius: 0;
  box-shadow: none;
}
.hookrules-photo::before {
  content: '';
  position: absolute;
  inset: 0;
  pointer-events: none;
}
.hookrules-xs { min-height: 150px; }
.hookrules-sm { min-height: 220px; }
.hookrules-md { min-height: 320px; }
.hookrules-lg { min-height: 455px; }
.hookrules-subtle::before {
  background: linear-gradient(180deg, rgba(6,10,18,.18), rgba(6,10,18,.38));
}
.hookrules-overlay::before {
  background: linear-gradient(180deg, rgba(6,10,18,.12) 0%, rgba(6,10,18,0) 26%, rgba(6,10,18,.82) 100%);
}
.hookrules-strong::before {
  background: linear-gradient(180deg, rgba(6,10,18,.74) 0%, rgba(6,10,18,.22) 32%, rgba(6,10,18,.9) 100%);
}
.hookrules-glow::before {
  background:
    radial-gradient(circle at 18% 18%, rgba(255,159,28,.22), rgba(255,159,28,0) 24%),
    linear-gradient(180deg, rgba(6,10,18,.2), rgba(6,10,18,.72));
}
.hookrules-tight {
  font-size: clamp(0.96rem, 0.92vw, 1.06rem);
  line-height: 1.45;
  color: rgba(255,255,255,.72);
}
.hookrules-kicker {
  font-size: clamp(0.9rem, 0.86vw, 1rem);
  text-transform: uppercase;
  letter-spacing: .08em;
  font-weight: 800;
  color: #5ec6ff;
  margin-bottom: .65em;
}
.hookrules-speakerline {
  font-size: clamp(0.88rem, 0.86vw, .98rem);
  line-height: 1.5;
  color: rgba(255,255,255,.66);
}
.hookrules-speakerline strong { color: #fff; }
.hookrules-band {
  margin-left: -2em;
  margin-right: -2em;
  padding: 1.05em 2em 1.15em;
  background: linear-gradient(145deg, rgba(10,14,23,.96), rgba(16,28,44,.9));
  box-shadow: inset 0 1px 0 rgba(255,255,255,.06);
}
.hookrules-band.warm {
  background: linear-gradient(145deg, rgba(44,21,4,.94), rgba(17,27,44,.92));
}
.hookrules-band.cool {
  background: linear-gradient(145deg, rgba(7,16,28,.96), rgba(14,24,38,.92));
}
.hookrules-card {
  position: absolute;
  left: 1.1em;
  right: 1.1em;
  bottom: 1.1em;
  padding: 1em 1.05em;
  border-radius: 18px;
  background: linear-gradient(145deg, rgba(7,10,18,.9), rgba(12,18,28,.82));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.08), 0 18px 34px rgba(0,0,0,.28);
}
.hookrules-topcard {
  position: absolute;
  top: 1.1em;
  left: 1.1em;
  right: 1.1em;
  padding: .95em 1em;
  border-radius: 18px;
  background: linear-gradient(145deg, rgba(7,10,18,.86), rgba(12,18,28,.76));
  box-shadow: inset 0 0 0 1px rgba(255,255,255,.08);
}
.hookrules-mega {
  font-size: clamp(2.35rem, 3vw, 3.25rem);
  line-height: .95;
}
.hookrules-medium {
  font-size: clamp(2rem, 2.5vw, 2.7rem);
  line-height: .98;
}
.hookrules-small {
  font-size: clamp(1.7rem, 2vw, 2.15rem);
  line-height: 1.02;
}
@media (max-width: 600px) {
  .hookrules-photo {
    margin-left: -1.7em;
    margin-right: -1.7em;
    width: calc(100% + 3.4em);
  }
  .hookrules-band {
    margin-left: -1.7em;
    margin-right: -1.7em;
    padding-left: 1.7em;
    padding-right: 1.7em;
  }
  .hookrules-lg { min-height: 370px; }
}
</style>

<div class="panel-promo-lab">

## HGDC 2025 Panel

<div class="post-caption">
Draft-only LinkedIn carousel workspace for the HGDC 2025 panel. Built as a standalone summary first, with the recording as optional follow-up context.
</div>

<div class="work-note">
  <strong>Working assumption:</strong> this page is for visual iteration and export, not for public indexing. The hook is now wired to the panel image at <strong>/images/HDCPanelPhoto.png</strong>.
</div>

## Post Text Draft

<div class="li-post-text">
  <div class="li-hook">If Unreal already gives you so much, why do teams still need engine programmers?</div>
  <div class="li-body">
    At HGDC 2025, Paweł Stolecki, Ilya Semikolennykh, and Adrianna "Ada" Bielak answered that question from three perspectives: <strong>rendering</strong>, <strong>story tech</strong>, and <strong>animation tech</strong>.
    <br><br>
    We talked about where engine work still matters in Unreal, how content and engine programmers stay aligned through production, when a real game need pushes you to extend the engine, and how one use case can grow into technology with value far beyond a single feature.
    <br><br>
    There are also concrete examples: pushing beyond the recognizable Unreal look, building context-aware story systems, improving animation workflows, and turning production problems into technology that unlocks better games. If you want the full discussion afterward, the recording is linked separately.
  </div>
  <div class="li-cta">Full panel recording in first comment</div>
  <div class="li-tags">#gamedev #unrealengine #engineprogramming #rendering #animationtech #storytech #toolsprogramming</div>
</div>

## Slide 1 — Hook

<div class="panel-slide">
  <span class="slide-num">1 / 9</span>
  <div class="hookrules-kicker">Inside Production</div>
  <div class="accent-bar"></div>
  <div class="hook hookrules-medium">What Engine Programmers Do in <span class="accent">Unreal Engine?</span></div>
  <div class="hook-aside">...and why you need one.</div>
  <div class="hook8-bullets">
    <span><strong>Role overlap</strong></span>
    <span><strong>Engine extension</strong></span>
    <span><strong>Production tradeoffs</strong></span>
  </div>
  <div class="panel-photo-card hookrules-photo hookrules-md hookrules-subtle"></div>
</div>

## Slide 2 — What The Panel Covers

<div class="panel-slide">
  <span class="slide-label">Panel Scope</span>
  <span class="slide-num">2 / 9</span>
  <div class="accent-bar"></div>
  <div class="hook"><span class="green">Three ways engine work shows up in Unreal.</span></div>
  <div class="sub">Rendering, story tech, and animation tech are three examples from a much wider set of engine specializations, but they already show how much engine work can shape what players feel and what teams can build.</div>
  <div class="persona-grid">
    <div class="persona-card">
      <div class="tag render">Rendering</div>
      <div class="title">Extend rendering for real game demands</div>
      <div class="body">Build, adopt, or extend visual features while working through performance limits, budgets, and engine changes.</div>
    </div>
    <div class="persona-card">
      <div class="tag story">Story Tech</div>
      <div class="title">Build story systems that react to the world</div>
      <div class="body">Create tools for context-aware open-world quests so teams can build stories players care about.</div>
    </div>
    <div class="persona-card">
      <div class="tag anim">Animation Tech</div>
      <div class="title">Turn motion into better tools for animators</div>
      <div class="body">Use math to describe motion so animators get more control and more room to excel.</div>
    </div>
  </div>
  <div class="micro">Not the full map of engine work, just three strong angles for seeing where engine programming creates real leverage.</div>
</div>

## Slide 3 — Collaboration

<div class="panel-slide">
  <span class="slide-label">Collaboration</span>
  <span class="slide-num">3 / 9</span>
  <div class="accent-bar"></div>
  <div class="hook">Game needs become clear when <span class="cyan">content and engine programmers</span> work together.</div>
  <div class="sub">Content and engine programmers have different roles, and the best results come from working together. That collaboration is what turns ambitious game ideas into systems and workflows teams can actually ship with.</div>
  <div class="persona-grid">
    <div class="benefit-card">
      <div class="tag orange">Constraints</div>
      <div class="title">Real use reveals what the engine does not cover</div>
      <div class="body">Many limitations appear only when content teams try to do something specific and the workflow breaks, slows down, or exposes a missing capability.</div>
    </div>
    <div class="benefit-card">
      <div class="tag indigo">Enablement</div>
      <div class="title">The work is iterative, not one-directional</div>
      <div class="body">The collaboration stays active through the whole process: from the first problem description, through solution review and early MVPs, all the way to final handoff.</div>
    </div>
    <div class="benefit-card">
      <div class="tag green">Alignment</div>
      <div class="title">The goal is alignment, not isolation</div>
      <div class="body">The job is to translate the needs of the game into technical choices without reducing engine work to ticket support or isolating technology from the people using it.</div>
    </div>
  </div>
</div>

## Slide 4 — Why It Still Matters

<div class="panel-slide">
  <span class="slide-label">Why It Still Matters</span>
  <span class="slide-num">4 / 9</span>
  <div class="accent-bar"></div>
  <div class="hook">If you already have <span class="accent">Unreal</span>,<br>why do you still need engine programmers?</div>
  <div class="sub">No technology is ideal for every game. Unreal gives you a strong base, but engine work is still where teams adapt it, remove barriers, and make space for the things that make a game feel truly its own: a look beyond the recognizable “Unreal look,” stronger animation workflows, or story systems Unreal does not provide out of the box.</div>
  <div class="compare-grid">
    <div class="compare-card bad">
      <div class="title">Custom Engine</div>
      <div class="body">If you do not build a capability, it simply is not there. You gain freedom over architecture and direction, but you also own the missing foundations and the cost of creating them.</div>
    </div>
    <div class="compare-card good">
      <div class="title">Unreal Engine</div>
      <div class="body">You start from an existing base and focus on the areas where you want to shine. You still extend source, integrate deeply, and handle process costs such as merge conflicts and long-term ownership.</div>
    </div>
  </div>
  <div class="micro">Unreal does not remove engine work. It focuses that work on the places where the game gains the most value.</div>
</div>

## Slide 5 — Extending Unreal

<div class="panel-slide">
  <span class="slide-label">When Extension Starts</span>
  <span class="slide-num">5 / 9</span>
  <div class="accent-bar"></div>
  <div class="hook">You extend the engine when <span class="accent">the game exposes a gap</span>.</div>
  <div class="sub">That moment usually appears when the needs of the game uncover parts of the stock engine that are unsupported, insufficient, or simply headed in a different direction than the game requires. This is where engine work stops being abstract and starts unlocking concrete capabilities.</div>
  <ul class="step-list">
    <li><span class="step-num">1</span><span><strong>Production exposes the gap.</strong> Often the limitation becomes visible only after content teams try to build something concrete.</span></li>
    <li><span class="step-num">2</span><span><strong>Engine programmers choose the response.</strong> Sometimes that means a plugin, sometimes direct source changes, depending on what has to be extended and how deeply it touches the engine.</span></li>
    <li><span class="step-num">3</span><span><strong>Process matters as much as code.</strong> Conventions for comments, change descriptions, tagging, and merge discipline become part of making engine development sustainable.</span></li>
  </ul>
  <div class="micro">The exciting part is not changing the engine for its own sake. It is unlocking what the game could not do before.</div>
</div>

## Slide 6 — In Practice

<div class="panel-slide">
  <span class="slide-label">In Practice</span>
  <span class="slide-num">6 / 9</span>
  <div class="accent-bar"></div>
  <div class="hook">Different domains.<br><span class="green">Same kind of leverage.</span></div>
  <div class="sub">The domains differ, but the pattern is similar: find a real production need, build or adapt the right system around it, and suddenly whole classes of work become easier, faster, or newly possible.</div>
  <div class="persona-grid">
    <div class="persona-card">
      <div class="tag render">Rendering</div>
      <div class="title">Rendering can mean both source changes and new modules</div>
      <div class="body">Examples include higher bounce light fidelity in Lumen, faster ray tracing scene updates, Nanite foliage-style breakthroughs, or standalone systems such as deformable geometry or fluid simulation.</div>
    </div>
    <div class="persona-card">
      <div class="tag story">Story Tech</div>
      <div class="title">Story tech can become a new real engine layer</div>
      <div class="body">A good example is framework-level support for open-world narrative: systems for world awareness and storytelling that Unreal does not provide out of the box, plus tools for authors to use them.</div>
    </div>
    <div class="persona-card">
      <div class="tag anim">Animation Tech</div>
      <div class="title">Animation tech includes building, integrating, and supporting systems</div>
      <div class="body">Motion matching, UAF-style workflows, runtime behavior, and external technology integration all matter if the final system has to support seamless gameplay, cinematic work, and production use.</div>
    </div>
  </div>
  <div class="micro">This is where engine work becomes easy to care about: you can point to something on screen, in a workflow, or in a shipped feature and say, this exists because the technology changed.</div>
</div>

## Slide 7 — Beyond One Game

<div class="panel-slide">
  <span class="slide-label">Beyond One Game</span>
  <span class="slide-num">7 / 9</span>
  <div class="accent-bar"></div>
  <div class="hook">One use case can expose a need.<br><span class="accent">Engine work often has to scale beyond it.</span></div>
  <div class="sub">Engine work often has to balance the needs of one use case against the value of building reusable technology, stable foundations, and workflows that can keep paying off across many users or many games.</div>
  <ul class="step-list">
    <li><span class="step-num">1</span><span><strong>A single game sets the pressure.</strong> That is often where the requirement becomes visible and where the technology first proves its value.</span></li>
    <li><span class="step-num">2</span><span><strong>Engine ownership raises the bar.</strong> The system should stay predictable, performant, and reusable beyond one narrow use case.</span></li>
    <li><span class="step-num">3</span><span><strong>Budgets still matter.</strong> The best technical direction is not the fanciest one. It has to fit time, performance, hardware, and the actual users it serves.</span></li>
    <li><span class="step-num">4</span><span><strong>Reuse is leverage.</strong> Closing repeated production problems inside stable systems is what turns one game's needs into long-term value.</span></li>
  </ul>
  <div class="micro">The value is leverage: solve a hard problem once, then let many teams move faster because of it.</div>
</div>

## Slide 8 — Paths Into Engine Work

<div class="panel-slide">
  <span class="slide-label">Career Paths</span>
  <span class="slide-num">8 / 9</span>
  <div class="accent-bar"></div>
  <div class="hook">There is no single path into <span class="accent">engine programming</span>.</div>
  <div class="sub">Some people start close to engine work from the beginning. Others arrive through gameplay, tools, or content-facing systems. Both paths build the kind of context that later turns into deep technical intuition.</div>
  <ul class="step-list">
    <li><span class="step-num">1</span><span><strong>Starting outside engine work still helps.</strong> If you used the tools first, you often understand better how real users think, where workflows break, and what support they actually need.</span></li>
    <li><span class="step-num">2</span><span><strong>Starting in engine work builds depth fast.</strong> You learn to read internals, debug deeper systems, and take responsibility for foundations other teams depend on.</span></li>
    <li><span class="step-num">3</span><span><strong>Specialization usually grows from curiosity and repetition.</strong> People often begin broadly, then go deeper where their interests and real production problems keep meeting.</span></li>
  </ul>
  <div class="micro">That is part of what makes this path exciting: curiosity compounds, and real production problems keep opening deeper doors.</div>
</div>

## Slide 9 — Why Learn It

<div class="panel-slide">
  <span class="slide-label">Why Learn It</span>
  <span class="slide-num">9 / 9</span>
  <div class="accent-bar"></div>
  <div class="hook">Engine work plugs you into a much wider <span class="accent">shared research space</span>.</div>
  <div class="sub">Unreal itself is built on decades of techniques, papers, and engine work shared across the industry. Learning how engines work, including engines other than Unreal, gives you more confidence, more transferability, and more freedom to solve harder problems later.</div>
  <div class="cta-block">
    <div class="cta-title">If This Interests You</div>
    <div class="cta-body">Study how things work from first principles, look beyond one engine, and do not be afraid of the internals. The full panel recording is linked here if you want the longer Polish discussion: <a href="https://www.youtube.com/watch?v=8tEdoAzL0c8&t=11s" style="color:#FF9F1C;text-decoration:none;">Paweł, Ilya, Adrianna - Rola programisty silnika w Unrealu - HGDC 2025 panel</a><br><br><strong>Recording language: Polish.</strong></div>
  </div>
  <div class="micro">Learn broadly, stay curious, and treat engine work as a way to unlock better games, better tools, and bigger creative ambition.</div>
</div>

## Alternate Directions

<div class="work-note">
  <strong>Variant ideas for iteration:</strong><br>
  1. A more personal hook slide focused on “how each panelist found their specialization.”<br>
  2. A more tactical hook slide focused on “what different engine roles end up shipping in Unreal.”<br>
  3. A second hook using a tighter crop on the panel image and fewer names on-canvas.<br>
  4. A bonus slide on book recommendations if the carousel needs a softer closing before the CTA.
</div>

</div>