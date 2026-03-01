---
title: "About"
description: "About Pawel Stolecki — rendering engineer focused on real-time graphics, GPU architecture, and performance-driven systems."
summary: "Background, projects, and engineering focus in real-time rendering."
keywords: ["about", "rendering engineer", "real-time graphics", "GPU", "graphics programming"]
date: 2026-02-09
showDate: false
showReadingTime: false
showWordCount: false
showShare: false
sharingLinks: false
showTitle: false
---

<style>
/* ── About page – clean layout ──────────────────────────────── */

/* Wrapper: kill default article max-width for full-bleed sections */
.about-page { margin: 0; padding-bottom: 0; }

/* Kill ALL bottom spacing the theme injects below article content */
.article-content { margin-bottom: 0 !important; }
.article-content.mb-20 { margin-bottom: 0 !important; }
article { padding-bottom: 0 !important; margin-bottom: 0 !important; }
article > section { padding-bottom: 0 !important; margin-bottom: 0 !important; }
article > section > div { padding-bottom: 0 !important; margin-bottom: 0 !important; }
/* Article inner footer (pagination area) adds pt-8 even when empty */
article > footer { padding-top: 0 !important; margin-top: 0 !important; }
/* Stop main from growing beyond its content (h-screen + grow creates gap) */
main { flex-grow: 0 !important; }

/* ── Section rhythm ─────────────────────────────────────────── */
.about-section {
  margin: 0 0 0;
  padding: 3.5em 0;
}

/* ── Below-fold sections: skip render until near viewport ───────── */
.about-band {
  width: 100vw;
  position: relative;
  left: 50%;
  transform: translateX(-50%);
  padding: 2.5em 2rem;
  box-sizing: border-box;
  content-visibility: auto;
  contain-intrinsic-size: 0 500px;
}
.about-band-dim {
  background: rgba(255,255,255,.03);
  border-top: 1px solid rgba(255,255,255,.06);
  border-bottom: 1px solid rgba(255,255,255,.06);
}
.about-band-inner {
  max-width: 900px;
  margin: 0 auto;
}

/* Section heading with accent bar */
.about-section h2,
.about-band h2 {
  font-size: 1.5em;
  font-weight: 800;
  margin: 0 0 1.2em;
  letter-spacing: -.01em;
  padding-left: .75em;
  border-left: 3px solid var(--ds-accent);
  line-height: 1.2;
}
.about-section .section-sub {
  font-size: .92em;
  opacity: .5;
  margin: 0 0 1.8em;
  line-height: 1.6;
}

/* ── Hero two-column (compact, centered like a card) ────────── */
.about-hero {
  display: flex;
  gap: 2.5em;
  align-items: center;
  max-width: 760px;
  margin: 0 auto 2em;
  line-height: 1.7;
}
.about-hero-text { flex: 1; min-width: 0; }
.about-hero-text p {
  font-size: .93em;
  margin: 0 0 .9em;
  opacity: .78;
}
.about-hero-text p:last-child { margin-bottom: 0; }
.about-hero-photo {
  flex-shrink: 0;
  width: 240px;
}
.about-hero-photo img {
  width: 100%;
  aspect-ratio: 3 / 4;
  object-fit: cover;
  object-position: center top;
  border-radius: 14px;
  display: block;
}
@media (max-width: 720px) {
  .about-hero { flex-direction: column-reverse; align-items: center; gap: 1.5em; max-width: 400px; }
  .about-hero-photo { width: 200px; }
}

/* ── Portfolio grid: 2 featured + rest ──────────────────────── */
.portfolio-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 1em;
}
.portfolio-grid .featured {
  grid-column: span 2;
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 1em;
}
@media (max-width: 600px) {
  .portfolio-grid { grid-template-columns: 1fr; }
  .portfolio-grid .featured { grid-column: span 1; grid-template-columns: 1fr; }
}
.portfolio-card {
  position: relative;
  border-radius: 12px;
  overflow: hidden;
  display: block;
  text-decoration: none !important;
  background: #0B0B0E;
  transition: transform .22s ease, box-shadow .22s ease;
}
.portfolio-card:hover {
  transform: translateY(-3px);
  box-shadow: 0 8px 28px rgba(0,0,0,.45);
}
.portfolio-card img {
  width: 100%;
  aspect-ratio: 16 / 10;
  object-fit: cover;
  display: block;
}
/* ── Slideshow container ─────────────────────────────────────── */
.pf-slides {
  position: relative;
  width: 100%;
  aspect-ratio: 16 / 10;
  overflow: hidden;
  background: #0B0B0E;
  display: block;
  contain: strict;
  will-change: transform; /* single compositor layer for the whole card */
}
.pf-slide {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
  object-fit: cover;
  opacity: 0;
  animation: pf-fade 7.5s linear infinite;
  animation-fill-mode: both;
}
/* Pause all slideshow animations when user prefers reduced motion */
@media (prefers-reduced-motion: reduce) {
  .pf-slide {
    animation: none;
    opacity: 0;
  }
  .pf-slide:first-child { opacity: 1; }
}
.portfolio-card:hover .pf-slide { animation-play-state: paused; }
/*
  7.5s cycle = 3 slides × 2.5s each.
  Each slide: fade-in 0.75s, hold 3.5s, fade-out 0.75s, idle 2.5s.
  Slides overlap during transitions so there is never a dark gap.
  Each card gets a unique time offset so they never all cross-fade
  at the same moment.
  Base delays (offset 0): -0.75s, 1.75s, 4.25s
*/
@keyframes pf-fade {
  0%      { opacity: 0; }
  10%     { opacity: 1; }
  56.67%  { opacity: 1; }
  66.67%  { opacity: 0; }
  100%    { opacity: 0; }
}
.portfolio-card .overlay {
  position: absolute;
  inset: 0;
  background: linear-gradient(0deg, rgba(0,0,0,.85) 0%, transparent 50%);
  display: flex;
  flex-direction: column;
  justify-content: flex-end;
  padding: 1em 1.1em;
}
.portfolio-card .overlay .title {
  font-size: .88em;
  font-weight: 700;
  color: #fff;
  line-height: 1.3;
}
.portfolio-card .overlay .tag {
  font-size: .7em;
  color: rgba(255,255,255,.45);
  margin-top: .1em;
}

/* ── Games showcase ────────────────────────────────────────── */
.games-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 1em;
}
@media (max-width: 700px) {
  .games-grid { grid-template-columns: 1fr; }
}
.game-card {
  position: relative;
  border-radius: 12px;
  overflow: hidden;
  display: block;
  text-decoration: none !important;
  background: #0B0B0E;
  transition: transform .22s ease, box-shadow .22s ease;
}
.game-card:hover {
  transform: translateY(-3px);
  box-shadow: 0 8px 28px rgba(0,0,0,.5);
}
.game-card img {
  width: 100%;
  aspect-ratio: 16 / 9;
  object-fit: cover;
  display: block;
  transition: transform .4s ease;
}
.game-card:hover img { transform: scale(1.04); }
.game-card .game-overlay {
  position: absolute;
  inset: 0;
  background: linear-gradient(0deg, rgba(0,0,0,.92) 0%, rgba(0,0,0,.3) 45%, transparent 70%);
  display: flex;
  flex-direction: column;
  justify-content: flex-end;
  padding: 1.1em 1.2em;
}
.game-card .game-title {
  font-size: 1em;
  font-weight: 800;
  color: #fff;
  line-height: 1.25;
  margin-bottom: .35em;
}
.game-card .game-role {
  font-size: .72em;
  color: rgba(255,255,255,.7);
  line-height: 1.5;
}
.game-card .game-play {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -60%);
  opacity: 0;
  transition: opacity .25s, transform .25s;
}
.game-card:hover .game-play {
  opacity: 1;
  transform: translate(-50%, -50%);
}
.game-card .game-play svg {
  width: 52px; height: 52px;
  fill: #fff;
  filter: drop-shadow(0 2px 8px rgba(0,0,0,.6));
}

/* ── Talks grid ─────────────────────────────────────────────── */
.talks-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
  gap: 1em;
}
.talk-card {
  display: block;
  text-decoration: none !important;
  border-radius: 12px;
  overflow: hidden;
  background: #0B0B0E;
  transition: transform .22s ease, box-shadow .22s ease;
}
.talk-card:hover {
  transform: translateY(-3px);
  box-shadow: 0 8px 28px rgba(0,0,0,.45);
}
.talk-thumb {
  position: relative;
  overflow: hidden;
}
.talk-thumb img {
  width: 100%;
  aspect-ratio: 16 / 9;
  object-fit: cover;
  display: block;
  transition: transform .4s ease;
}
.talk-card:hover .talk-thumb img { transform: scale(1.04); }
.talk-play {
  position: absolute;
  inset: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(0,0,0,.2);
  opacity: 0;
  transition: opacity .25s ease;
}
.talk-card:hover .talk-play { opacity: 1; }
.talk-play svg {
  width: 48px; height: 48px;
  fill: #fff;
  filter: drop-shadow(0 2px 6px rgba(0,0,0,.5));
}
.talk-body {
  padding: 1em 1.1em 1.15em;
}
.talk-body h4 {
  font-size: .9em;
  font-weight: 700;
  margin: 0 0 .3em;
  line-height: 1.35;
}
.talk-body .meta {
  font-size: .75em;
  opacity: .45;
  line-height: 1.5;
}

/* ── GIF poster + play button overlay ────────────────────────── */
.portfolio-card { position: relative; }
.pf-gif-poster {
  width: 100%;
  aspect-ratio: 16 / 10;
  object-fit: cover;
  display: block;
  transition: filter .3s ease;
}
.pf-gif-play {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  opacity: .85;
  transition: opacity .25s ease, transform .25s ease;
  pointer-events: none;
  z-index: 2;
}
.portfolio-card:hover .pf-gif-play {
  opacity: 1;
  transform: translate(-50%, -50%) scale(1.1);
}
.portfolio-card.pf-gif-loaded .pf-gif-play { display: none; }
/* Slideshow: deferred images start invisible until src is set */
.pf-slide:not([src]) { visibility: hidden; }

/* Off-screen slideshows: pause animation to save GPU */
.pf-slides.pf-paused .pf-slide { animation-play-state: paused !important; }


</style>

<div class="about-page">

<!-- ── About me ──────────────────────────────────────────────── -->


<div class="about-hero ds-reveal">
  <div style="flex:1;min-width:0;">
    <h1 style="font-size:2.3em;font-weight:800;margin:0 0 .35em;line-height:1.1;">About me</h1>
    <div class="about-hero-text">
      <p>
        I'm Pawel Stolecki, a rendering engineer focused on real-time graphics and performance-critical systems,
        building scalable rendering systems that deliver production value while fully utilizing modern hardware under strict frame budgets.
      </p>
      <p>
        My work spans physically based rendering, real-time ray tracing, low-level graphics programming across PC and console platforms.
      </p>
      <p>
        I enjoy working close to the metal: analyzing hardware utilization, data structures &amp; algorithms ensuring every fraction of a millisecond is accounted for.
      </p>
    </div>
  </div>
  <div class="about-hero-photo">
    <img src="/images/author.jpg" alt="Pawel Stolecki" fetchpriority="high" decoding="async" />
  </div>
</div>

<!-- ── Games shipped ───────────────────────────────────────── -->

<div class="about-band about-band-dim"><div class="about-band-inner">

<h2>Games I worked on</h2>

<div class="games-grid">

  <a class="game-card" href="https://www.youtube.com/watch?v=Nthv4xF_zHU" target="_blank" rel="noopener">
    <img src="https://img.youtube.com/vi/Nthv4xF_zHU/hqdefault.jpg" alt="The Witcher 4" loading="lazy" decoding="async" />
    <div class="game-overlay">
      <div class="game-title">The Witcher 4: Tech Demo</div>
      <div class="game-role">CD PROJEKT RED · Delivering visual quality within a 60 fps PS5 budget</div>
    </div>
    <div class="game-play"><svg viewBox="0 0 24 24"><path d="M8 5v14l11-7z"/></svg></div>
  </a>

  <a class="game-card" href="https://www.youtube.com/watch?v=km7pc3P6PE8" target="_blank" rel="noopener">
    <img src="https://img.youtube.com/vi/km7pc3P6PE8/hqdefault.jpg" alt="Dying Light 2" loading="lazy" decoding="async" />
    <div class="game-overlay">
      <div class="game-title">Dying Light 2: Stay Human</div>
      <div class="game-role">Techland · GBuffer &amp; lighting rendering improvements</div>
    </div>
    <div class="game-play"><svg viewBox="0 0 24 24"><path d="M8 5v14l11-7z"/></svg></div>
  </a>

  <a class="game-card" href="https://www.youtube.com/watch?v=ysgzR2jMwNE" target="_blank" rel="noopener">
    <img src="https://img.youtube.com/vi/ysgzR2jMwNE/hqdefault.jpg" alt="World War 3" loading="lazy" decoding="async" />
    <div class="game-overlay">
      <div class="game-title">World War 3</div>
      <div class="game-role">The Farm 51 · Environmental shaders &amp; rendering tooling</div>
    </div>
    <div class="game-play"><svg viewBox="0 0 24 24"><path d="M8 5v14l11-7z"/></svg></div>
  </a>

</div>
</div></div>

<!-- ── Rendering showcase ────────────────────────────────────── -->

<div class="about-band"><div class="about-band-inner">

<h2>Contribution Examples</h2>

<div class="portfolio-grid">

  <div class="featured">
    <!-- Volumetric Fog — 3-image left→right wipe slideshow -->
    <a class="portfolio-card" href="https://www.artstation.com/artwork/OGER3g" target="_blank" rel="noopener">
      <div class="pf-slides">
        <img class="pf-slide" src="https://cdnb.artstation.com/p/assets/images/images/062/270/817/medium/pawel-stolecki-1.jpg?1682718299"   alt="Volumetric Fog 1" loading="lazy" style="animation-delay:-0.75s" />
        <img class="pf-slide" data-src="https://cdnb.artstation.com/p/assets/images/images/062/270/829/medium/pawel-stolecki-5.jpg?1682718331"   alt="Volumetric Fog 2" loading="lazy" style="animation-delay:1.75s" />
        <img class="pf-slide" data-src="https://cdna.artstation.com/p/assets/images/images/062/270/844/medium/pawel-stolecki-9.jpg?1682718362"   alt="Volumetric Fog 3" loading="lazy" style="animation-delay:4.25s" />
      </div>
      <div class="overlay">
        <div class="title">Volumetric Fog</div>
        <div class="tag">Dying Light 2 · Techland</div>
      </div>
    </a>
    <!-- Ocean R&D — deferred GIF with poster -->
    <a class="portfolio-card" href="https://www.artstation.com/artwork/N5K5qb" target="_blank" rel="noopener">
      <img class="pf-gif-poster" src="/images/water-poster.jpg" data-gif="/images/water.gif" alt="Gerstner Ocean R&amp;D" loading="lazy" decoding="async" />
      <div class="pf-gif-play" aria-hidden="true"><svg viewBox="0 0 24 24" width="40" height="40" fill="#fff"><circle cx="12" cy="12" r="11" fill="rgba(0,0,0,.45)" stroke="#fff" stroke-width="1.2"/><path d="M9.5 7.5v9l7-4.5z"/></svg></div>
      <div class="overlay">
        <div class="title">Gerstner Ocean</div>
        <div class="tag">World War 3 · The Farm 51</div>
      </div>
    </a>
  </div>

  <!-- SSAO — 3-image left→right wipe slideshow -->
  <a class="portfolio-card" href="https://www.artstation.com/artwork/PXOBQy" target="_blank" rel="noopener">
    <div class="pf-slides">
      <img class="pf-slide" src="https://cdnb.artstation.com/p/assets/images/images/062/268/265/medium/pawel-stolecki-1.jpg?1682713064"  alt="SSAO 1" loading="lazy" style="animation-delay:4.05s" />
      <img class="pf-slide" data-src="https://cdna.artstation.com/p/assets/images/images/062/268/810/medium/pawel-stolecki-222.jpg?1682714090" alt="SSAO compare" loading="lazy" style="animation-delay:6.55s" />
      <img class="pf-slide" data-src="https://cdnb.artstation.com/p/assets/images/images/062/268/621/medium/pawel-stolecki-2.jpg?1682713769"  alt="SSAO 3" loading="lazy" style="animation-delay:9.05s" />
    </div>
    <div class="overlay">
      <div class="title">Screen Space Ambient Occlusion</div>
      <div class="tag">Dying Light 2 · Techland</div>
    </div>
  </a>

  <!-- Water Rendering — 3-image slideshow -->
  <a class="portfolio-card" href="https://www.artstation.com/artwork/8bmLQQ" target="_blank" rel="noopener">
    <div class="pf-slides">
      <img class="pf-slide" src="https://cdna.artstation.com/p/assets/images/images/062/269/266/medium/pawel-stolecki-1.jpg?1682714949"  alt="Water Rendering 1" loading="lazy" style="animation-delay:1.45s" />
      <img class="pf-slide" data-src="https://cdna.artstation.com/p/assets/images/images/062/269/286/medium/pawel-stolecki-4.jpg?1682714973"  alt="Water Rendering 2" loading="lazy" style="animation-delay:3.95s" />
      <img class="pf-slide" data-src="https://cdna.artstation.com/p/assets/images/images/062/269/308/medium/pawel-stolecki-7.jpg?1682715005"  alt="Water Rendering 3" loading="lazy" style="animation-delay:6.45s" />
    </div>
    <div class="overlay">
      <div class="title">Water Rendering</div>
      <div class="tag">Dying Light 2 · Techland</div>
    </div>
  </a>

  <!-- Motion Blur — 3-image left→right wipe slideshow -->
  <a class="portfolio-card" href="https://www.artstation.com/artwork/m83KDd" target="_blank" rel="noopener">
    <div class="pf-slides">
      <img class="pf-slide" src="https://cdnb.artstation.com/p/assets/images/images/062/270/439/medium/pawel-stolecki-1.jpg?1682717446"  alt="Motion Blur 1" loading="lazy" style="animation-delay:0.35s" />
      <img class="pf-slide" data-src="https://cdnb.artstation.com/p/assets/images/images/062/270/447/medium/pawel-stolecki-3.jpg?1682717455"  alt="Motion Blur 2" loading="lazy" style="animation-delay:2.85s" />
      <img class="pf-slide" data-src="https://cdna.artstation.com/p/assets/images/images/062/270/454/medium/pawel-stolecki-5.jpg?1682717470"  alt="Motion Blur 3" loading="lazy" style="animation-delay:5.35s" />
    </div>
    <div class="overlay">
      <div class="title">Motion Blur</div>
      <div class="tag">Dying Light 2 · Techland</div>
    </div>
  </a>

  <!-- Wind Simulation — deferred GIF with poster -->
  <a class="portfolio-card" href="https://www.artstation.com/artwork/4XJLvn" target="_blank" rel="noopener">
    <img class="pf-gif-poster" src="/images/simulation-poster.jpg" data-gif="/images/simulation.gif" alt="Wind Simulation" loading="lazy" decoding="async" />
    <div class="pf-gif-play" aria-hidden="true"><svg viewBox="0 0 24 24" width="40" height="40" fill="#fff"><circle cx="12" cy="12" r="11" fill="rgba(0,0,0,.45)" stroke="#fff" stroke-width="1.2"/><path d="M9.5 7.5v9l7-4.5z"/></svg></div>
    <div class="overlay">
      <div class="title">Wind Simulation</div>
      <div class="tag">World War 3 · The Farm 51</div>
    </div>
  </a>

</div>
</div></div>

<!-- ── Talks & presentations ─────────────────────────────────── -->

<div class="about-band about-band-dim"><div class="about-band-inner">

<h2>Talks &amp; presentations</h2>


<div class="talks-grid">

  <a class="talk-card" href="https://www.youtube.com/watch?v=OwyMw2C4UfA" target="_blank" rel="noopener">
    <div class="talk-thumb">
      <img src="https://i.ytimg.com/vi/OwyMw2C4UfA/hqdefault.jpg" alt="PBR: Putting Pieces Together" loading="lazy" decoding="async" />
      <div class="talk-play"><svg viewBox="0 0 24 24"><path d="M8 5v14l11-7z"/></svg></div>
    </div>
    <div class="talk-body">
      <h4>Physically Based Rendering: Putting Pieces Together</h4>
      <div class="meta">Game Industry Conference · CD Projekt Red</div>
    </div>
  </a>

  <a class="talk-card" href="https://www.youtube.com/watch?v=UOsPC_V6uxA" target="_blank" rel="noopener">
    <div class="talk-thumb">
      <img src="https://i.ytimg.com/vi/UOsPC_V6uxA/hqdefault.jpg" alt="Illuminating the Villedor" loading="lazy" decoding="async" />
      <div class="talk-play"><svg viewBox="0 0 24 24"><path d="M8 5v14l11-7z"/></svg></div>
    </div>
    <div class="talk-body">
      <h4>Illuminating the Villedor: Lighting in Dying Light 2</h4>
      <div class="meta">Game Industry Conference · w/ Wojciech Zeler · Techland</div>
    </div>
  </a>

  <a class="talk-card" href="https://www.youtube.com/watch?v=z0JE_eqxrVQ" target="_blank" rel="noopener">
    <div class="talk-thumb">
      <img src="https://i.ytimg.com/vi/z0JE_eqxrVQ/hqdefault.jpg" alt="Pakiet Technicznej wiedzy" loading="lazy" decoding="async" />
      <div class="talk-play"><svg viewBox="0 0 24 24"><path d="M8 5v14l11-7z"/></svg></div>
    </div>
    <div class="talk-body">
      <h4>Pakiet Technicznej wiedzy dla 3D Artysty</h4>
      <div class="meta">Personal channel</div>
    </div>
  </a>

</div>
</div></div>



</div>

<script>
(function(){
  if (typeof IntersectionObserver === 'undefined') return;

  /* ── 1. Deferred slideshow images: load slides 2-3 when card is near viewport ── */
  var slideIO = new IntersectionObserver(function(entries){
    entries.forEach(function(e){
      if (!e.isIntersecting) return;
      var deferred = e.target.querySelectorAll('.pf-slide[data-src]');
      deferred.forEach(function(img){
        img.src = img.getAttribute('data-src');
        img.removeAttribute('data-src');
      });
      slideIO.unobserve(e.target);
    });
  }, { rootMargin: '200px 0px' });
  document.querySelectorAll('.pf-slides').forEach(function(el){ slideIO.observe(el); });

  /* ── 2. Pause slideshow animations when off-screen ─────────── */
  var pauseIO = new IntersectionObserver(function(entries){
    entries.forEach(function(e){
      e.target.classList.toggle('pf-paused', !e.isIntersecting);
    });
  }, { rootMargin: '50px 0px' });
  document.querySelectorAll('.pf-slides').forEach(function(el){
    el.classList.add('pf-paused');       /* start paused */
    pauseIO.observe(el);
  });

  /* ── 3. GIF deferred load: swap poster → animated GIF on hover/tap ── */
  document.querySelectorAll('.pf-gif-poster').forEach(function(poster){
    var gifURL = poster.getAttribute('data-gif');
    if (!gifURL) return;
    var card = poster.closest('.portfolio-card');
    var loaded = false;

    function loadGif() {
      if (loaded) return;
      loaded = true;
      var gif = new Image();
      gif.onload = function(){
        poster.src = gifURL;
        poster.removeAttribute('data-gif');
        card.classList.add('pf-gif-loaded');
      };
      gif.src = gifURL;
    }

    /* Desktop: hover loads the GIF */
    card.addEventListener('mouseenter', loadGif, { once: true });
    /* Mobile: tap loads the GIF (first tap loads, second follows href) */
    card.addEventListener('touchstart', function(ev){
      if (!loaded) { ev.preventDefault(); loadGif(); }
    }, { passive: false, once: true });
  });
})();
</script>
