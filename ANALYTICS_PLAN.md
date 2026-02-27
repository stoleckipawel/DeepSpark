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

Eliminated options:
- **GA4** — cookies, consent banner, blocked by ~30-40% of dev audience
- **Fathom** — no free tier ($14/mo)
- **Plausible** — no free cloud tier (€9/mo), self-hosted needs a VPS
- **Simple Analytics** — no free tier (€9/mo)
- **Seline / GoatCounter** — no scroll depth / limited custom events

**Winner: Umami Cloud Hobby** — free tier (10K events/mo), no cookies, custom events for scroll tracking, Blowfish built-in support (just fill `params.toml`).

### ✅ DECIDED: Umami

---

## Decision 2 — Hosting? → **Umami Cloud — Free Hobby tier**

| Option | Events/month | Cost | Fits? |
|---|---|---|---|
| **Umami Cloud Hobby** | 10,000 | **Free** | ✅ Yes |

### Event budget math
- 1 page view = 1 event
- Scroll milestones (25%, 50%, 75%, 100%) = up to 4 events per read
- Code-copy click = 1 event (rare)
- Worst case per visitor: ~5-6 events

**10,000 events ÷ 6 = ~1,600 full article reads/month.** For a growing tech blog, this is comfortably enough. If you outgrow it, that's a great problem to have — upgrade to Growth ($9/mo) when traffic justifies it.

### Smart budgeting we'll implement
- Only fire scroll events on article pages (not homepage/about)
- Combine 25%+50% into just 50% (halves scroll events) → **3 milestones: 50%, 75%, 100%**
- Debounce to prevent duplicates
- Result: ~4 events per full read → supports **~2,500 reads/month** on free tier

### ✅ DECIDED: Umami Cloud Hobby (free, no hosting, no maintenance)

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
| **Scroll milestones** (50%, 75%, 100%) | Where readers stop — do they finish? Drop off at the midpoint? | IntersectionObserver on sentinel elements (3 events, budget-optimized) |
| **Reading completion** | % of articles fully read | Included in the 100% milestone above |
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

## Decision 5 — When to go live? → **Now**

Every day without analytics is lost data. We'll test locally with `hugo server`, verify the script loads and events fire, then push.

### ✅ DECIDED: Wire it up now

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

## All decisions locked ✅

| # | Decision | Answer |
|---|---|---|
| 1 | Provider | **Umami** (privacy-first, custom events, Blowfish built-in) |
| 2 | Hosting | **Umami Cloud Hobby** (free, no self-hosting) |
| 3 | Consent banner | **None needed** (no cookies) |
| 4 | Tracking | **Layer 1 + Layer 2** (page views + scroll/reading behavior) |
| 5 | Timing | **Now** |

### Next step
1. Go to [cloud.umami.is](https://cloud.umami.is) → sign up (free) → add site `stoleckipawel.dev` → copy your **Website ID**
2. Give me the Website ID and I'll wire up everything in ~1 minute

---
---

# Appendix: Complete Analytics Opportunity Map

> **Scope:** Every free (or free-tier) analytics opportunity for a Hugo static site on GitHub Pages at `stoleckipawel.dev`, using the Blowfish theme, deployed via GitHub Actions.
>
> **Date researched:** 2026-02-27

---

## 1. GitHub-Native Analytics

### 1a. GitHub Repository Traffic Insights

- **URL:** `https://github.com/<user>/<repo>/graphs/traffic`
- **Cost:** Free (included with every repo)
- **What it provides:**
  - Unique visitors & page views to the **repo page** (not the live site) — last 14 days only
  - Top referral sources to the repo
  - Popular content (which repo files/paths get views)
- **Privacy impact:** None on your readers — this tracks GitHub.com visitors to the repo, not stoleckipawel.dev visitors
- **Setup:** Zero — already available in your repo's Insights → Traffic tab
- **Gotchas:**
  - Only 14 days of data, no historical export (use a GitHub Action to scrape & store — see Section 10)
  - Tracks repo visitors, **not** site visitors — useful only as a proxy for dev interest
  - Requires push access to view

### 1b. GitHub Actions Workflow Metrics

- **URL:** `https://github.com/<user>/<repo>/actions`
- **Cost:** Free (2,000 mins/month on free tier)
- **What it provides:** Build times, deployment success/failure, frequency of deploys
- **Privacy impact:** None
- **Setup:** Already in place via `deploy-pages.yml`
- **Gotchas:** Not visitor analytics — deployment health only

---

## 2. Free Third-Party Analytics Services (No Server Required)

### 2a. Umami Cloud (Hobby tier) ✅ SELECTED

- **URL:** https://cloud.umami.is
- **Cost:** Free — 10,000 events/month
- **What it provides:** Page views, unique visitors, referrers, UTM tracking, country/language/device/browser/OS, time on page, entry/exit pages, custom events (scroll depth, clicks, etc.), real-time dashboard, API access, data export
- **Privacy impact:** **No cookies, no personal data, GDPR/CCPA compliant out of the box.** No consent banner needed.
- **Setup for Hugo/Blowfish:** Native support — just fill `[umamiAnalytics]` in `params.toml`. Zero-friction.
- **Gotchas:**
  - 10K events/month is tight if you fire many custom events (budget carefully)
  - Cloud Hobby has 3-month data retention (Growth = unlimited)
  - Some ad blockers block `cloud.umami.is` — can use custom domain with paid tier

### 2b. GoatCounter (free hosted)

- **URL:** https://www.goatcounter.com
- **Cost:** Free for non-commercial use (hosted at `yourname.goatcounter.com`). Self-hosted is also free.
- **What it provides:** Page views, referrers, browser/OS/screen size, location (country), campaign tracking, basic paths. No unique visitor count (privacy choice). Public dashboard option.
- **Privacy impact:** **No cookies, no personal data.** GDPR compliant without consent. Possibly the most privacy-respecting option available.
- **Setup:** Add a single `<script>` tag to `extend-head.html` or `extend-footer.html`. No Blowfish native support — manual tag.
- **Gotchas:**
  - No custom events — page views only (scroll depth not possible here)
  - No real-time view
  - Dashboard is functional but spartan
  - Free hosted tier has fair-use limit (~100K+ pageviews should be fine for a blog)

### 2c. Tinylytics

- **URL:** https://tinylytics.app
- **Cost:** Free tier: 1 site, basic stats. Paid: $5/month for more features.
- **What it provides:** Page views, referrers, countries, hit counter badge you can embed, "kudos" (like button), webring features
- **Privacy impact:** No cookies, privacy-friendly
- **Setup:** Single `<script>` tag. Manual addition to Hugo templates.
- **Gotchas:**
  - Very basic — no custom events, no scroll tracking
  - Indie project (one developer) — longevity risk
  - Free tier is extremely limited

### ~~2d. Simple Analytics~~ — REMOVED (no free tier, starts €9/mo)

### 2e. Cabin

- **URL:** https://withcabin.com
- **Cost:** Free for personal sites / open-source projects
- **What it provides:** Page views, unique visitors, referrers, country, device type, UTM tracking, carbon tracking (unique feature)
- **Privacy impact:** No cookies, no personal data, GDPR compliant
- **Setup:** Single script tag in `extend-head.html`
- **Gotchas:**
  - No custom events on free tier
  - Must apply and be approved for free tier (personal/open-source)
  - Limited feature set compared to Umami/Plausible

### 2f. Counter.dev

- **URL:** https://counter.dev
- **Cost:** Free (completely free, open source)
- **What it provides:** Page views, referrers, screen sizes, OS, browser, country — very basic dashboard
- **Privacy impact:** No cookies, privacy-friendly. Uses the `window.location` and User-Agent only.
- **Setup:** Tiny inline script + account email. Add to `extend-footer.html`.
- **Gotchas:**
  - Extremely minimal — no custom events, no scroll tracking
  - Single-developer project
  - Data can be delayed
  - Dashboard is very basic
  - No API for data export

### 2g. Pirsch Analytics

- **URL:** https://pirsch.io
- **Cost:** Free tier: 10,000 page views/month, 1 dashboard (as of 2025 — verify current offering)
- **What it provides:** Page views, unique visitors, referrers, UTM, device/browser/OS/country, entry/exit pages, events, goals, conversion funnels
- **Privacy impact:** **No cookies**, server-side-ish approach (uses hashing). GDPR compliant.
- **Setup:** Script tag or server-side API (script tag for static site). Manual Hugo integration.
- **Gotchas:**
  - Free tier limits may have changed — was briefly removed
  - Events cost against pageview quota
  - Not as mature as Plausible/Umami ecosystem

### 2h. Beam Analytics

- **URL:** https://beamanalytics.io
- **Cost:** Free tier: 5,000 page views/month, 1 site
- **What it provides:** Page views, visitors, referrers, browser/device/OS, country, UTM tracking
- **Privacy impact:** No cookies, GDPR/CCPA compliant
- **Setup:** Script tag injection
- **Gotchas:**
  - 5K pageview limit is low
  - No custom events on free tier
  - Relatively new service

### 2i. Microanalytics.io

- **URL:** https://microanalytics.io
- **Cost:** Free tier: 10,000 page views/month, 1 website
- **What it provides:** Page views, unique visitors, referrers, country, device/browser, session duration
- **Privacy impact:** No cookies, privacy-first, EU-hosted
- **Setup:** Script tag
- **Gotchas:**
  - No custom events
  - Limited integrations
  - Small team

### ~~2j. Plausible Analytics~~ — REMOVED (no free cloud tier, self-hosted needs VPS)

### ~~2k. Fathom Analytics~~ — REMOVED (no free tier, starts $14/mo)

### 2l. Seline Analytics

- **URL:** https://seline.so
- **Cost:** Free tier available (limits vary — check current)
- **What it provides:** Page views, visitors, referrers, events, session replay (limited)
- **Privacy impact:** Privacy-focused, cookie-free basics
- **Setup:** Blowfish has native support via `[selineAnalytics]` in params.toml
- **Gotchas:**
  - Newer service — maturity/longevity uncertain
  - Free tier limits may be restrictive
  - Event capabilities less proven than Umami

---

## 3. Cloudflare-Based Options

### 3a. Cloudflare Web Analytics (free)

- **URL:** https://www.cloudflare.com/web-analytics/
- **Cost:** **Completely free**, unlimited page views, even without using Cloudflare as CDN/DNS
- **What it provides:** Page views, unique visitors, referrers, paths, country, browser/OS/device, Core Web Vitals (LCP, FID, CLS), page load time
- **Privacy impact:** **No cookies**, privacy-first. Cloudflare processes data but doesn't sell it. No consent banner needed.
- **Setup:**
  - If NOT using Cloudflare DNS: just add their JS beacon (`<script src='https://static.cloudflareinsights.com/beacon.min.js' data-cf-beacon='{"token":"YOUR_TOKEN"}'></script>`) to `extend-footer.html`
  - If using Cloudflare DNS/proxy: auto-injected, no code needed
- **Gotchas:**
  - **No custom events** — page views and Web Vitals only
  - Data sampling at very high volumes (not an issue for a blog)
  - Dashboard is limited — no filtering by UTM, no goals, no funnels
  - Can stack with Umami since they measure different things (Umami = behavior, CF = performance)
  - **Excellent complement to Umami** — gives Core Web Vitals for free

### 3b. Cloudflare DNS Analytics

- **URL:** Available in Cloudflare dashboard if domain uses CF nameservers
- **Cost:** Free with any Cloudflare plan
- **What it provides:** DNS query volume, query types, response codes, top queried records
- **Privacy impact:** None on visitors (infrastructure-level data)
- **Setup:** Switch nameservers to Cloudflare (stoleckipawel.dev must use CF DNS)
- **Gotchas:**
  - GitHub Pages custom domains work fine with Cloudflare DNS
  - Only tells you DNS query volume, not meaningful visitor analytics
  - Useful for detecting DDoS or misconfiguration

---

## 4. Search Engine Tools

### 4a. Google Search Console (GSC)

- **URL:** https://search.google.com/search-console
- **Cost:** **Free**
- **What it provides:**
  - Search queries that lead to your site (impressions, clicks, CTR, average position)
  - Index coverage (which pages are indexed, errors)
  - Core Web Vitals report (mobile + desktop)
  - Mobile usability issues
  - Sitemaps monitoring
  - Backlinks (who links to you)
  - Manual actions / security issues
- **Privacy impact:** **None on visitors** — data comes from Google's side
- **Setup:**
  1. Add property for `stoleckipawel.dev`
  2. Verify ownership via DNS TXT record, HTML file, or meta tag
  3. For meta tag: add to `extend-head.html`: `<meta name="google-site-verification" content="YOUR_CODE">`
  4. Or use Blowfish's built-in: `[verification] google = "YOUR_CODE"` in `params.toml`
  5. Submit sitemap: `https://stoleckipawel.dev/sitemap.xml`
- **Gotchas:**
  - Data delayed ~2-3 days
  - **Essential** even if not "analytics" — tells you what search terms bring readers
  - Free, zero privacy impact — no reason not to set up

### 4b. Bing Webmaster Tools

- **URL:** https://www.bing.com/webmasters
- **Cost:** **Free**
- **What it provides:** Similar to GSC — search queries, impressions, clicks, index coverage, SEO reports, backlinks, keyword research tool
- **Privacy impact:** None on visitors
- **Setup:**
  1. Add site, verify via DNS/meta tag/CNAME
  2. Blowfish built-in: `[verification] bing = "YOUR_CODE"` in `params.toml`
  3. Can import from GSC directly
- **Gotchas:**
  - Bing is ~3-6% of search traffic but its webmaster tools are excellent
  - Has a built-in SEO audit tool that GSC doesn't
  - Also covers Yahoo and DuckDuckGo (they use Bing's index)

### 4c. Yandex Webmaster

- **URL:** https://webmaster.yandex.com
- **Cost:** Free
- **What it provides:** Search analytics for Yandex, index status, site quality metrics
- **Privacy impact:** None
- **Setup:** Meta verification tag — Blowfish supports `[verification] yandex = "..."`
- **Gotchas:** Only relevant if you have Russian/CIS audience. Low priority.

---

## 5. Social / Referral Tracking

### 5a. Open Graph & Twitter Cards (already partially configured)

- **URL:** Built into Hugo/Blowfish
- **Cost:** Free
- **What it provides:** Rich previews when your articles are shared on Twitter/X, LinkedIn, Facebook, Slack, Discord, etc. Not analytics per se, but **dramatically increases click-through from social shares**.
- **Privacy impact:** None
- **Current status:** Blowfish generates OG tags automatically. `defaultSocialImage` is set to `images/DeepSparkIcon.png`.
- **Setup:** Already done by theme. Validate with:
  - https://cards-dev.twitter.com/validator (Twitter Card Validator)
  - https://developers.facebook.com/tools/debug/ (Facebook Sharing Debugger)
  - https://www.opengraph.xyz (general OG preview)
- **Gotchas:**
  - Each article should have a unique `featured_image` or `thumbnail` for best social previews
  - Without per-article images, all shares look the same (generic icon)

### 5b. Reddit / Hacker News Referral Detection

- **URL:** N/A — tracked via referrer data in your analytics tool (Umami)
- **Cost:** Free
- **What it provides:** Umami's referrer report will show `reddit.com`, `news.ycombinator.com`, etc. as traffic sources
- **Setup:** Automatic — Umami captures `document.referrer`
- **Gotchas:**
  - Reddit referrers sometimes come through as `old.reddit.com`, `www.reddit.com`, or `out.reddit.com`
  - HN traffic spikes are massive but fleeting (track hourly if possible)
  - Some Reddit apps strip referrer headers

### 5c. UTM Parameter Tracking

- **URL:** Built into Umami
- **Cost:** Free
- **What it provides:** Track campaigns — when you share links as `?utm_source=twitter&utm_medium=social&utm_campaign=frame-graph`, Umami breaks these out
- **Setup:** Just use UTM parameters in your shared links. Umami picks them up automatically.
- **Gotchas:** Requires discipline to use UTM links when sharing

---

## 6. Performance Monitoring

### 6a. Google PageSpeed Insights

- **URL:** https://pagespeed.web.dev
- **Cost:** **Free** (web tool), API is free (limited to 25,000 queries/day)
- **What it provides:** Lighthouse scores (Performance, Accessibility, Best Practices, SEO), Core Web Vitals (LCP, CLS, INP), actionable recommendations
- **Privacy impact:** None
- **Setup:** Just enter your URL. For API: get a key from Google Cloud Console (free).
- **Gotchas:**
  - Point-in-time measurement — not continuous monitoring
  - Can automate via API + GitHub Actions (see 6c)

### 6b. Web Vitals Tracking (via Cloudflare Web Analytics)

- **URL:** Included in Cloudflare Web Analytics (see 3a)
- **Cost:** Free
- **What it provides:** Real User Monitoring (RUM) for Core Web Vitals — LCP, FID/INP, CLS — from actual visitors over time
- **Privacy impact:** No cookies (Cloudflare beacon)
- **Setup:** Same as Cloudflare Web Analytics setup
- **Gotchas:** Best way to get continuous Web Vitals data for free

### 6c. Lighthouse CI in GitHub Actions

- **URL:** https://github.com/GoogleChrome/lighthouse-ci
- **Cost:** **Free** (runs in GitHub Actions)
- **What it provides:** Automated Lighthouse audits on every deploy. Performance regression detection. Can set thresholds ("fail build if performance < 90").
- **Privacy impact:** None
- **Setup:** Add a step to `deploy-pages.yml`:
  ```yaml
  - name: Lighthouse CI
    uses: treosh/lighthouse-ci-action@v12
    with:
      urls: |
        https://stoleckipawel.dev/
        https://stoleckipawel.dev/posts/frame-graph-theory/
      budgetPath: ./lighthouse-budget.json  # optional
      uploadArtifacts: true
  ```
- **Gotchas:**
  - Runs in CI, not against real user traffic
  - Scores may differ from real-world (CI environment vs user devices)
  - Adds ~60-90 seconds to build time
  - Can store historical results in Lighthouse CI Server (free self-hosted) or as artifacts

### 6d. Unlighthouse (full-site Lighthouse audit)

- **URL:** https://unlighthouse.dev
- **Cost:** Free (open source)
- **What it provides:** Runs Lighthouse against every page on your site, generates a report. Catches per-page performance issues.
- **Setup:** `npx unlighthouse --site https://stoleckipawel.dev`
- **Gotchas:** Slow for large sites (fine for a blog). Can run in CI.

---

## 7. Reader Engagement Proxies

### 7a. GitHub Stars on the Repo

- **Cost:** Free
- **What it provides:** Social proof / interest signal. Can track growth over time with tools like https://star-history.com
- **Privacy impact:** None
- **Setup:** Already available. Make repo public if it isn't.
- **Gotchas:** Vanity metric — doesn't correlate with readership directly

### 7b. RSS Subscriber Counts

- **URL:** Your feed is at `https://stoleckipawel.dev/index.xml`
- **Cost:** Free
- **What it provides:** RSS subscribers can't be tracked directly (RSS is pull-based, anonymous). But:
  - **FeedPress** (https://feed.press) — free tier: wraps your feed, gives subscriber estimates
  - **Follow.it** (https://follow.it) — free: adds email/RSS subscription widget, gives subscriber counts
  - **RSS analytics via Umami** — track clicks on the RSS icon as a custom event
- **Privacy impact:** Depends on service
- **Setup:** Either redirect your RSS URL through a tracking service, or just track RSS link clicks
- **Gotchas:**
  - RSS subscriber counting is inherently imprecise
  - Many readers use feed readers that batch-fetch (inflating counts)

### 7c. Newsletter Signups

- **Cost:** Free tiers available on Buttondown (free under 100 subscribers), Substack (free), Mailchimp (free under 500)
- **What it provides:** Email subscriber count, open rates, click rates — direct engagement measurement
- **Setup:** Add a signup form to the site (Blowfish templates or custom HTML in `extend-footer.html`)
- **Gotchas:** Requires ongoing effort (sending newsletters). But subscriber count is a high-signal engagement metric.

### 7d. Blowfish Views/Likes (Firebase-backed)

- **URL:** Built into Blowfish theme
- **Cost:** Free (Firebase free tier: Spark plan)
- **What it provides:** Per-article view count and "like" button with count, displayed on the page
- **Setup:** Fill `[firebase]` section in `params.toml`, set `showViews = true` and `showLikes = true` in `[article]`
- **Privacy impact:** Uses Firebase — may set cookies, needs consent consideration
- **Gotchas:**
  - Firebase free tier limits: 50K reads/day, 20K writes/day (plenty for a blog)
  - Adds visible social proof to articles
  - View counting is separate from analytics (just a counter, no behavioral data)
  - Currently disabled in your config (`showViews = false`, `showLikes = false`)

---

## 8. Heatmap / Session Replay (Free Tiers)

### 8a. Microsoft Clarity ⭐ STRONGEST OPTION

- **URL:** https://clarity.microsoft.com
- **Cost:** **Completely free. No traffic limits. Unlimited sites. Forever.**
- **What it provides:**
  - **Session recordings** — watch exactly how visitors interact with your pages
  - **Heatmaps** — click maps, scroll maps, area maps
  - **Dead click detection** — where users click expecting something to happen
  - **Rage click detection** — frustrated repeated clicks
  - **Quick-back detection** — users who navigate then immediately return
  - **Scroll depth** — percentage of page scrolled (aggregate)
  - **Engagement metrics** — active time, scroll reach
  - **Copilot AI** — Microsoft's AI summarizes session patterns
  - **Filtering** — by page, country, device, browser, OS, referrer, UTM
  - **Google Analytics integration** — optional (irrelevant for you)
- **Privacy impact:**
  - **⚠️ Uses cookies** (first-party `_clck`, `_clsk`)
  - **⚠️ Consent banner likely needed** under GDPR/ePrivacy
  - Automatically masks sensitive text/inputs
  - Microsoft processes data (US-based)
  - This conflicts with your "no cookies, no consent banners" priority
- **Setup:** Add a `<script>` snippet to `extend-head.html` (~4 lines)
- **Gotchas:**
  - **Cookie issue is the dealbreaker** for your privacy-first approach
  - However: the data (session replays, heatmaps) is extremely valuable for your "understand where readers stop" goal
  - **Possible compromise:** Add Clarity behind a cookie consent banner, or use it temporarily during a research phase then remove it
  - Adds ~15-20KB to page load
  - Session replay data retained 30 days

### ~~8b. Hotjar~~ — REMOVED (Clarity is strictly better on free tier, 35 sessions/day is useless)

### ~~8c. FullStory~~ — REMOVED (enterprise overkill, heavy 70KB+ script, restrictive free tier)

---

## 9. A/B Testing on Static Sites

### 9a. Google Optimize → Discontinued (Sept 2023)

No longer available.

### ~~9b. Cloudflare Workers (manual A/B)~~ — REMOVED (complex, requires Cloudflare proxy setup)

### 9c. Client-Side A/B with JavaScript

- **Cost:** Free (DIY)
- **What it provides:** Simple variant testing (e.g., different headlines, CTA text)
- **Setup:** Small JS in `extend-footer.html` that assigns visitors to variants (using `Math.random()` + `localStorage`), then tracks variant via Umami custom event
- **Gotchas:**
  - Flash of original content before JS swaps variant
  - Not statistically rigorous without effort
  - localStorage-based — cleared if user clears cache

### 9d. PostHog (free tier)

- **URL:** https://posthog.com
- **Cost:** Free: 1M events/month (generous), includes feature flags and A/B testing
- **What it provides:** Full product analytics, feature flags, A/B testing, session replay
- **Privacy impact:** Can be configured cookie-free. Self-hostable.
- **Setup:** Script tag. Feature flags require code integration.
- **Gotchas:**
  - Powerful but heavy — designed for web apps, not blogs
  - 1M events free is very generous
  - Cookie-free mode available but less accurate

### ~~9e. Growthbook~~ — REMOVED (overkill for a blog)

**Bottom line on A/B testing:** For a blog, the practical approach is client-side JS + Umami custom events. Formal A/B platforms are overkill.

---

## 10. GitHub Actions-Based Analytics (Proxy Metrics)

### 10a. Deployment Frequency & Build Times

- **URL:** Your workflow: `.github/workflows/deploy-pages.yml`
- **Cost:** Free
- **What it provides:** How often you publish, build duration trends
- **Setup:** Already happening. View in Actions tab. For historical tracking, use:
  - **GitHub Actions workflow metrics API:** `GET /repos/{owner}/{repo}/actions/runs` — extract timing data
  - **Step Timing:** Each step in the workflow logs its duration
- **Gotchas:** Meta-analytics, not visitor data

### 10b. Repo Traffic Archiver (GitHub Action)

- **URL:** https://github.com/marketplace/actions/traffic-to-badge or custom script
- **Cost:** Free
- **What it provides:** Archives the 14-day repo traffic data on a schedule so you get historical coverage
- **Setup:**
  ```yaml
  - cron: '0 0 */13 * *'  # Run every 13 days to capture before expiry
  ```
  Store traffic data as JSON in the repo or a gist.
- **Gotchas:** Only tracks repo traffic, not site traffic. Clutter in repo history.

### 10c. Build Size Tracking

- **Cost:** Free (GitHub Actions)
- **What it provides:** Track total size of `public/` directory over time. Catch unintended bloat.
- **Setup:** Add a step:
  ```yaml
  - name: Report build size
    run: du -sh public/ && find public -name '*.html' | wc -l
  ```
- **Gotchas:** Useful for performance hygiene, not visitor analytics

### 10d. Lighthouse Score Tracking over Time

- Combine with 6c — store Lighthouse scores as GitHub Action artifacts or in a JSON file. Track performance regressions across deploys.

---

## 11. Comments-as-Analytics (Engagement Signals)

### 11a. Giscus (GitHub Discussions-backed) ⭐ RECOMMENDED

- **URL:** https://giscus.app
- **Cost:** **Free** (uses GitHub Discussions as backend)
- **What it provides:**
  - Comment system on each article
  - Reactions (👍 ❤️ 🚀 etc.) — lightweight engagement signal
  - Discussion threads — deeper engagement signal
  - All data stored in your GitHub repo's Discussions tab
- **Privacy impact:** Requires GitHub login to comment (readers must have a GitHub account). No cookies from Giscus itself. GitHub's cookies apply only to logged-in participants.
- **Setup:** Blowfish may have built-in support (check `params.toml` — not currently configured). Otherwise:
  1. Enable Discussions on your repo
  2. Install the Giscus GitHub App
  3. Add the `<script>` to article template or `extend-footer.html`
- **Gotchas:**
  - Audience must have GitHub accounts (fine for a dev blog)
  - No anonymous comments
  - Engagement is a proxy — only the most motivated readers comment
  - **Excellent fit** for a technical blog on GitHub Pages

### 11b. Utterances (GitHub Issues-backed)

- **URL:** https://utteranc.es
- **Cost:** Free (uses GitHub Issues as backend)
- **What it provides:** Comments stored as GitHub Issues. Simpler than Giscus.
- **Privacy impact:** Same as Giscus — requires GitHub login
- **Setup:** Single script tag
- **Gotchas:**
  - Giscus is newer and better (uses Discussions which are purpose-built for this)
  - Issues can clutter your repo's Issues tab
  - **Giscus is preferred** over Utterances

### 11c. Disqus (free tier)

- **URL:** https://disqus.com
- **Cost:** Free tier (ad-supported)
- **What it provides:** Full-featured comment system with moderation, reactions, engagement analytics
- **Privacy impact:** **⚠️ Heavy tracking, ads, cookies, third-party data sharing.** The antithesis of privacy-first.
- **Setup:** Script injection or Hugo built-in Disqus support
- **Gotchas:** **Hard no** — completely violates your privacy principles. Mentioned only for completeness.

---

## 12. Uptime Monitoring

### 12a. UptimeRobot

- **URL:** https://uptimerobot.com
- **Cost:** Free: 50 monitors, 5-minute intervals
- **What it provides:** Uptime/downtime alerts, response time tracking, status page
- **Privacy impact:** None on visitors (pings from UptimeRobot's servers)
- **Setup:**
  1. Sign up
  2. Add HTTP(s) monitor for `https://stoleckipawel.dev`
  3. Configure alerts (email, Slack, Discord, etc.)
- **Gotchas:**
  - 5-minute check interval on free tier (fine for a blog)
  - GitHub Pages has ~99.9% uptime anyway
  - Status page can be public (share with readers)
  - Alerts you if GitHub Pages has an outage

### 12b. Better Uptime (now Better Stack)

- **URL:** https://betterstack.com/uptime
- **Cost:** Free: 10 monitors, 3-minute intervals, beautiful status page
- **What it provides:** Uptime monitoring, incident management, status page, response time graphs
- **Privacy impact:** None on visitors
- **Setup:** Similar to UptimeRobot
- **Gotchas:**
  - Nicer UI than UptimeRobot
  - Free tier is slightly more limited (10 vs 50 monitors) but enough for one site
  - Includes incident timelines

### 12c. Freshping (by Freshworks)

- **URL:** https://www.freshworks.com/website-monitoring/
- **Cost:** Free: 50 monitors, 1-minute intervals, 5 status pages
- **What it provides:** Uptime, response time, multi-location checks
- **Privacy impact:** None
- **Setup:** Standard URL monitor
- **Gotchas:** Best free check interval (1 minute). Part of Freshworks ecosystem.

### 12d. GitHub's Own Status

- **URL:** https://www.githubstatus.com
- **Cost:** Free
- **What it provides:** GitHub Pages platform status. Subscribe to updates.
- **Setup:** Just subscribe via email/RSS
- **Gotchas:** Tells you about platform-wide issues, not your specific site

---

## 13. SEO Tools

### 13a. Google Rich Results Test

- **URL:** https://search.google.com/test/rich-results
- **Cost:** Free
- **What it provides:** Validates structured data (JSON-LD, Microdata) on your pages. Shows which rich result types are eligible.
- **Setup:** Paste URL, run test.
- **Gotchas:** Blowfish generates some structured data. `enableStructuredBreadcrumbs = true` is already set in your config.

### 13b. Schema.org Validator

- **URL:** https://validator.schema.org
- **Cost:** Free
- **What it provides:** Validates structured data markup
- **Setup:** Paste URL or markup
- **Gotchas:** Complementary to Google's tool

### 13c. Broken Link Checkers

- **W3C Link Checker:** https://validator.w3.org/checklink (free, online)
- **Ahrefs Broken Link Checker:** https://ahrefs.com/broken-link-checker (free, limited)
- **htmltest** (GitHub Action): https://github.com/wjdp/htmltest — run in CI to catch broken links on every deploy
  ```yaml
  - name: Check links
    uses: wjdp/htmltest-action@master
    with:
      path: ./public
  ```
- **Gotchas:** `htmltest` is excellent for CI — catches broken internal links before they go live. Free, fast, runs on the built HTML.

### 13d. Ahrefs Webmaster Tools (free)

- **URL:** https://ahrefs.com/webmaster-tools
- **Cost:** Free for site owners (verify ownership)
- **What it provides:** Backlink profile, organic keywords, site audit (SEO health), internal linking analysis
- **Setup:** Verify site ownership (DNS/meta/file)
- **Gotchas:**
  - Only shows data for your verified site (not competitor analysis)
  - Site audit is excellent — finds SEO issues Lighthouse misses
  - Backlink data is industry-leading

### 13e. Screaming Frog SEO Spider (free tier)

- **URL:** https://www.screamingfrog.co.uk/seo-spider/
- **Cost:** Free: crawl up to 500 URLs
- **What it provides:** Full site crawl — response codes, meta titles/descriptions, H1s, canonical tags, hreflang, structured data, image alt text, internal linking
- **Setup:** Desktop app — enter URL, crawl
- **Gotchas:** 500 URL limit on free tier (plenty for a blog). Desktop only (Windows/Mac/Linux).

### 13f. Sitemap Validators

- **XML Sitemaps Validator:** https://www.xml-sitemaps.com/validate-xml-sitemap.html
- **Your sitemap:** Already configured at `https://stoleckipawel.dev/sitemap.xml`
- **Gotchas:** Hugo generates this automatically. Just validate it's well-formed and submitted to GSC/Bing.

---

## Summary: Recommended Stack for stoleckipawel.dev

### Tier 1 — Implement Now (high value, zero/low effort)

| Tool | Category | Why |
|---|---|---|
| **Umami Cloud Hobby** | Visitor analytics | Already decided. No cookies, custom events, Blowfish built-in. |
| **Google Search Console** | Search analytics | Free, zero privacy impact, tells you what queries bring readers. Essential. |
| **Cloudflare Web Analytics** | Performance + basic analytics | Free, no cookies, gives Core Web Vitals RUM data. Perfect complement to Umami. |
| **Bing Webmaster Tools** | Search analytics | Free, covers Bing/Yahoo/DDG, has a great SEO audit tool. |

### Tier 2 — Implement Soon (moderate value, low effort)

| Tool | Category | Why |
|---|---|---|
| **Giscus** | Engagement | Free, GitHub-backed comments, perfect for a dev blog audience. |
| **UptimeRobot** | Uptime monitoring | Free, 5-min checks, alerts you if the site goes down. |
| **Lighthouse CI** | Performance CI | Free, catches performance regressions on every deploy. |
| **htmltest** | SEO / link health | Free, catches broken links in CI before they're live. |
| **Open Graph validation** | Social/referral | Already working, just validate once. |

### Tier 3 — Consider Later (valuable but not urgent)

| Tool | Category | Why |
|---|---|---|
| **Microsoft Clarity** | Heatmaps/session replay | Incredible free tool but uses cookies — conflicts with privacy goals. Could use temporarily for research. |
| **Ahrefs Webmaster Tools** | SEO | Free site audit and backlink monitoring. |
| **Lighthouse score tracking** | Perf trending | Track scores over time in CI artifacts. |
| **Newsletter** (Buttondown/etc.) | Engagement | High-signal metric but requires commitment to write. |

### Tier 4 — Eliminated (paid, self-hosted, or conflicts with goals)

| Tool | Why removed |
|---|---|
| Google Analytics (GA4) | Cookies, consent banner, blocked by ad blockers |
| Fathom | No free tier ($14/mo) |
| Plausible Cloud | No free tier (€9/mo), self-hosted needs always-on VPS |
| Simple Analytics | No free tier (€9/mo) |
| Hotjar / FullStory | Clarity is strictly better for free; all need cookies |
| Disqus | Privacy nightmare, ads, heavy tracking |
| Yandex Webmaster | Not relevant unless targeting CIS audience |
| Growthbook / A/B platforms | Overkill for a blog |
| Any self-hosted option | Requires always-on server/VPS |

---

## Quick Reference: Privacy Impact Matrix

| Tool | Cookies? | Consent needed? | Personal data? |
|---|---|---|---|
| Umami | ❌ No | ❌ No | ❌ No |
| Cloudflare Web Analytics | ❌ No | ❌ No | ❌ No |
| Google Search Console | N/A (server-side) | ❌ No | ❌ No |
| Bing Webmaster Tools | N/A (server-side) | ❌ No | ❌ No |
| Giscus | Only GitHub's (for logged-in commenters) | ❌ No (GitHub handles it) | Only GitHub username |
| UptimeRobot | N/A (server-side ping) | ❌ No | ❌ No |
| Lighthouse CI | N/A (CI only) | ❌ No | ❌ No |
| Microsoft Clarity | ⚠️ **Yes** | ⚠️ **Yes** | ⚠️ Session recordings |
| GoatCounter | ❌ No | ❌ No | ❌ No |
| Cabin | ❌ No | ❌ No | ❌ No |

---

## Blowfish Theme Built-In Support Summary

Your theme natively supports these analytics (just fill in config values):

| Provider | Config key | Free? | Viable? |
|---|---|---|---|
| **Umami** | `params.toml → [umamiAnalytics]` | ✅ 10K events | ✅ **SELECTED** |
| Seline | `params.toml → [selineAnalytics]` | ✅ Limited | ⚠️ Backup |
| Firebase views/likes | `params.toml → [firebase]` | ✅ Spark plan | ⚠️ Cookies |
| Site verification | `params.toml → [verification]` | ✅ Free | ✅ Use for GSC/Bing |
| ~~Google Analytics~~ | ~~`hugo.toml → googleAnalytics`~~ | ~~Yes~~ | ❌ Cookies |
| ~~Fathom~~ | ~~`params.toml → [fathomAnalytics]`~~ | ~~No~~ | ❌ Paid |

Everything else requires manual script injection via `layouts/partials/extend-head.html` or `extend-footer.html`.
