# DeepSpark — Analytics Integration Plan

> **Hosting:** GitHub Pages (static, no server-side processing)
> **Theme:** Blowfish (built-in support for GA4, Fathom, Umami, Seline)
> **Status:** No analytics currently active

---

## Your priorities (confirmed)

1. **Reader comfort first** — no cookies, no consent banners, no creepy tracking
2. **Deep behavioral data** — page views over time, scroll depth, where readers stop, time on page
3. **Goal:** understand reader behaviour to improve writing quality and structure

---

## Decision 1 — Which analytics provider? → **Umami**

GA4 is eliminated by priority #1 (cookies, consent banner, blocked by ad blockers on ~30-40% of your dev audience). Fathom, Seline, and GoatCounter are eliminated by priority #2 (no scroll depth / limited custom events). That leaves two real contenders:

| | Umami | Plausible |
|---|---|---|
| Privacy (no cookies) | ✅ | ✅ |
| Custom events (scroll depth, reading milestones) | ✅ `umami.track()` | ✅ `plausible()` |
| Blowfish built-in | ✅ **Yes** — just fill `params.toml` | ❌ Manual script tag |
| Cost (cloud) | $9/mo (free self-host) | €9/mo (free self-host) |
| Dashboard | Functional, improving fast | More polished |
| Data export / API | ✅ Full API | ✅ Full API |
| Event properties (e.g. which article, which section) | ✅ Key-value pairs | ✅ Key-value pairs |

**Winner: Umami** — same privacy and event capabilities as Plausible, but zero-friction setup since Blowfish has it built in. If you later want to switch to Plausible, the custom scroll-tracking script works with either.

### ✅ DECIDED: Umami

---

## Decision 2 — Self-hosted vs. SaaS? → **YOUR CALL**

| Option | Pros | Cons |
|---|---|---|
| **Umami Cloud ($9/mo)** | Zero maintenance, 5-min setup, automatic updates, always online | Monthly cost, data on Umami's servers (still privacy-respecting) |
| **Self-hosted (free)** | Completely free, full data ownership, custom domain | Need a VPS (~$5/mo anyway), PostgreSQL setup, maintenance, backups |

**My lean:** Start with **Umami Cloud** to get data flowing immediately. Migrate to self-hosted later if cost matters — the tracking script stays the same, only the `domain` in config changes.

### 👉 YOUR DECISION:
> `[ ] Umami Cloud` · `[ ] Self-hosted`

---

## Decision 3 — Cookie consent banner? → **Not needed**

Umami uses no cookies and collects no personal data. GDPR/ePrivacy compliant out of the box. No banner required.

### ✅ DECIDED: No consent banner

---

## Decision 4 — What to track

Based on your goals ("understand where readers stop, what gets read, improve my writing"), here's the tracking plan:

### Layer 1 — Automatic (provided by Umami out of the box)
| Metric | What it tells you |
|---|---|
| Page views over time | Which articles attract traffic, growth trends |
| Unique visitors | Actual audience size (not inflated by refreshes) |
| Referral sources | Where readers find you (Google, Twitter/X, HN, Reddit, direct) |
| Country / language | Audience geography |
| Device / browser / OS | Mobile vs. desktop reading patterns |
| Time on page | Which articles hold attention vs. get bounced |
| Entry / exit pages | Where people land, where they leave |

### Layer 2 — Custom events (requires a small JS script we'll add)
| Event | What it tells you | How |
|---|---|---|
| **Scroll milestones** (25%, 50%, 75%, 100%) | Where readers stop — do they finish? Drop off at the midpoint? | IntersectionObserver on sentinel elements |
| **Reading completion** | % of articles fully read | Fire event when footer enters viewport |
| **Section engagement** | Which H2 sections are actually reached | Track heading visibility |
| **Time-to-milestone** | How long to reach 50%, 75%, 100% — indicates engagement speed | Timestamp deltas |
| **Code-copy clicks** | Which code blocks are useful enough to copy | Hook the existing copy button |
| **Collapsible expand** | Do readers open collapsed code-diffs? | Hook `<details>` toggle |

### Layer 3 — Nice-to-have (add later)
| Event | Value |
|---|---|
| External link clicks | Which references/resources readers follow |
| Search queries (site search) | What readers look for that you haven't written yet |
| Dark/light mode preference | Minor — design feedback |

### ✅ DECIDED: Layer 1 + Layer 2 now, Layer 3 later

---

## Decision 5 — When to go live? → **YOUR CALL**

| Option | Pros | Cons |
|---|---|---|
| **Immediate** | Start collecting data today; every day without analytics is data lost | Scroll tracking needs testing |
| **After testing** | Verify everything works on preview before pushing to prod | Delays by a few days |

**My lean:** Wire it up now, test locally with `hugo server`, then push. Effectively immediate.

### 👉 YOUR DECISION:
> `[ ] Wire it up now` · `[ ] Wait`

---

## Implementation plan

### Step 1 — Umami account setup (you do this)
1. Go to [cloud.umami.is](https://cloud.umami.is) (or set up self-hosted)
2. Create a site for `stoleckipawel.dev`
3. Copy your **Website ID** (looks like `a1b2c3d4-...`)

### Step 2 — Enable Umami in Blowfish config (I do this)
```toml
# config/_default/params.toml
[umamiAnalytics]
  websiteid = "YOUR-WEBSITE-ID"
  domain = "cloud.umami.is"
  enableTrackEvent = true
```

### Step 3 — Add scroll & reading tracker script (I do this)
A lightweight script in `layouts/partials/extend-footer.html` that:
- Places invisible sentinel `<div>`s at 25%, 50%, 75%, 100% of article body
- Uses `IntersectionObserver` (no scroll event spam, very performant)
- Fires `umami.track('scroll-25', { article: 'frame-graph-build-it' })` etc.
- Tracks time-to-milestone
- Hooks code-copy button clicks
- Hooks `<details>` toggle for collapsible code-diffs

Zero impact on page load — the script is ~40 lines, deferred, no external dependencies.

### Step 4 — Verify locally
```bash
hugo server -D
# Open browser → check Network tab → confirm umami script loads
# Scroll an article → check Umami dashboard for events
```

### Step 5 — Deploy
Commit and push. GitHub Actions builds and deploys. Done.

---

## Remaining decisions for you

| # | Question | Options |
|---|---|---|
| 2 | Hosting model | `Umami Cloud` / `Self-hosted` |
| 5 | Timing | `Now` / `Wait` |
| — | Provide Website ID | (after creating Umami account) |

Once you give me the Website ID, I'll wire up Steps 2–3 in under a minute.
