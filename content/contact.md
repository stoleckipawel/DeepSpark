---
title: "Contact"
description: "Contact Pawel Stolecki for rendering engineering collaboration, graphics programming consulting, and technical discussion."
summary: "Ways to get in touch regarding rendering and graphics engineering."
keywords: ["contact", "rendering engineer", "graphics programming", "collaboration"]
date: 2026-02-28
showDate: false
showReadingTime: false
showWordCount: false
showShare: false
sharingLinks: false
showTitle: false
---

<style>
/* ── Contact page ── */
.contact-hero {
  width: 100vw;
  position: relative;
  left: 50%;
  transform: translateX(-50%);
  min-height: auto;
  display: flex;
  align-items: flex-start;
  padding: 2.5em 4vw 3em;
  box-sizing: border-box;
  gap: 4em;
  flex-wrap: wrap;
  overflow: hidden;
}
/* subtle large letterform bg */
.contact-hero::before {
  content: '@';
  position: absolute;
  right: 5vw;
  top: 50%;
  transform: translateY(-50%);
  font-size: clamp(14em, 28vw, 22em);
  font-weight: 900;
  line-height: 1;
  color: rgba(var(--ds-accent-rgb), .045);
  pointer-events: none;
  user-select: none;
  letter-spacing: -.05em;
}
.contact-left {
  flex: 1 1 280px;
  position: relative;
  z-index: 1;
}
.contact-label {
  font-size: .7em;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: .12em;
  opacity: .4;
  margin-bottom: .9em;
}
.contact-hero .contact-heading {
  font-size: clamp(2.4em, 5.5vw, 4em);
  font-weight: 800;
  line-height: 1.05;
  margin: 0 0 .7em;
  padding-top: 0;
  border-top: none;
}
.contact-hero .contact-heading span {
  color: var(--ds-accent);
}
.contact-hero .contact-heading::after {
  display: none;
}
.contact-divider {
  width: 2.4em;
  height: 2px;
  background: var(--ds-accent);
  border-radius: 2px;
  opacity: .7;
  margin-bottom: 1.6em;
}
.contact-tagline {
  font-size: .88em;
  opacity: .45;
  line-height: 1.7;
  max-width: 340px;
  margin: 0;
}
.contact-right {
  flex: 1 1 300px;
  display: flex;
  flex-direction: column;
  gap: 2.4em;
  position: relative;
  z-index: 1;
}
/* email card */
.contact-email-card {
  display: flex;
  align-items: center;
  gap: 1.1em;
  padding: 1.4em 1.8em;
  border: 1px solid rgba(255,255,255,.08);
  border-radius: 10px;
  background: rgba(255,255,255,.03);
  transition: border-color .2s, background .2s;
  text-decoration: none;
  color: inherit;
}
.contact-email-card:hover {
  border-color: rgba(var(--ds-accent-rgb), .5);
  background: rgba(var(--ds-accent-rgb), .05);
}
.contact-email-card-icon {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 42px;
  height: 42px;
  border-radius: 8px;
  background: rgba(var(--ds-accent-rgb), .12);
  flex-shrink: 0;
}
.contact-email-card-icon svg {
  width: 19px;
  height: 19px;
  fill: var(--ds-accent);
}
.contact-email-card-text {
  display: flex;
  flex-direction: column;
  gap: .15em;
}
.contact-email-card-label {
  font-size: .7em;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: .1em;
  opacity: .4;
}
.contact-email-card-value {
  font-size: .95em;
  font-weight: 600;
  opacity: .85;
}
/* socials row */
.contact-socials {
  display: flex;
  gap: .7em;
  flex-wrap: wrap;
}
.contact-social-pill {
  display: inline-flex;
  align-items: center;
  gap: .45em;
  padding: .45em .9em;
  border-radius: 50px;
  border: 1px solid rgba(255,255,255,.1);
  background: rgba(255,255,255,.04);
  color: inherit;
  text-decoration: none;
  font-size: .78em;
  font-weight: 500;
  opacity: .7;
  transition: opacity .15s, border-color .15s, background .15s;
}
.contact-social-pill:hover {
  opacity: 1;
  border-color: rgba(255,255,255,.22);
  background: rgba(255,255,255,.09);
}
.contact-social-pill svg {
  width: 13px;
  height: 13px;
  fill: currentColor;
}
</style>

<div class="contact-hero ds-reveal">
  <div class="contact-left">
    <h2 class="contact-heading">Get in<br><span>touch.</span></h2>
    <div class="contact-divider"></div>
    <p class="contact-tagline">Collaborations, content inquiries, or just a good conversation about graphics engineering.</p>
  </div>
  <div class="contact-right">
    <a class="contact-email-card" href="mailto:stoleckipawel@deepspark.dev">
      <div class="contact-email-card-icon">
        <svg viewBox="0 0 24 24"><path d="M20 4H4c-1.1 0-2 .9-2 2v12c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V6c0-1.1-.9-2-2-2zm0 4l-8 5-8-5V6l8 5 8-5v2z"/></svg>
      </div>
      <div class="contact-email-card-text">
        <span class="contact-email-card-label">Email</span>
        <span class="contact-email-card-value">stoleckipawel@deepspark.dev</span>
      </div>
    </a>

  </div>
</div>
