# DeepSpark — Analytics Integration Plan

> **Hosting:** GitHub Pages (static, no server-side processing)  
> **Theme:** Blowfish (built-in Umami support)  
> **Constraints:** Free only. No self-hosting. No always-on server. Privacy-first (no cookies, no consent banners).

---

## Decisions (locked)

| # | Decision | Answer |
|---|---|---|
| 1 | Provider | **Umami Cloud Hobby** (free, 10K events/mo, no cookies, Blowfish built-in) |
| 2 | Hosting | **Umami Cloud** (free tier, no self-hosting) |
| 3 | Consent banner | **None needed** (no cookies, no personal data) |
| 4 | Tracking | Page views + scroll depth + code-copy + collapsible opens |
| 5 | Timing | **Now** |

---

## What you'll see when this is done

| Question | Where to find the answer | How it's tracked |
|---|---|---|
| **How many people visit my site?** | Umami → Dashboard → Visitors graph | Automatic |
| **Which articles are most popular?** | Umami → Pages → sorted by views | Automatic |
| **Where do visitors come from?** | Umami → Referrers (Google, Twitter/X, Reddit, HN, direct) | Automatic |
| **What countries are they in?** | Umami → Countries map | Automatic |
| **What device/browser do they use?** | Umami → Devices / Browsers | Automatic |
| **Where do readers stop reading?** | Umami → Events → `scroll-50%`, `scroll-75%`, `scroll-100%` | Custom script |
| **Do readers finish my articles?** | Umami → Events → `scroll-100%` count vs page views | Custom script |
| **Which H2 sections get reached?** | Umami → Events → `section` with heading name | Custom script (off by default — budget) |
| **How long do readers spend?** | Umami → Dashboard → Average visit time | Automatic |
| **Do people copy my code?** | Umami → Events → `code-copy` | Custom script |
| **Do people open collapsed code-diffs?** | Umami → Events → `details-open` | Custom script |

---

## Step-by-step implementation

### Step 1 — Create Umami Cloud account (you do this, ~3 min)

1. Go to **[cloud.umami.is](https://cloud.umami.is)**
2. Click **Sign up** → use GitHub or email (free Hobby plan is selected by default)
3. Once logged in → click **Settings** → **Websites** → **Add website**
4. Fill in:
   - **Name:** `DeepSpark`
   - **Domain:** `stoleckipawel.dev`
5. Click **Save**
6. You'll see a **Website ID** like `a1b2c3d4-e5f6-7890-abcd-ef1234567890`
7. **Copy that Website ID** → give it to me for Step 2

> The free Hobby tier gives you 10,000 events/month with 3-month data retention. No credit card needed.

---

### Step 2 — Enable Umami in site config (I do this, ~30 sec)

I edit `config/_default/params.toml`:

```toml
[umamiAnalytics]
  websiteid = "YOUR-WEBSITE-ID"       # ← your ID goes here
  enableTrackEvent = true              # enables umami.track() for custom events
```

**What this does:**
- Blowfish's built-in `analytics/umami.html` partial injects the Umami script into `<head>` on every page
- The script loads async (~1.5 KB gzipped) — zero render blocking
- `enableTrackEvent = true` fires a built-in `article:Title` event on each page load

**Data you get from this alone (no custom code):**
- ✅ Page views over time (daily/weekly/monthly)
- ✅ Unique visitors (hashed IP, no cookies)
- ✅ Referral sources (Google, Twitter/X, Reddit, HN, direct)
- ✅ Countries/regions
- ✅ Device type (desktop/mobile/tablet)
- ✅ Browser & OS
- ✅ Entry & exit pages
- ✅ Average visit duration
- ✅ Most popular pages ranked by views
- ✅ Real-time visitor count
- ✅ UTM campaign tracking (`?utm_source=twitter` etc.)

---

### Step 3 — Add scroll & behavior tracking script (I do this, ~40 lines)

I add a `<script>` at the end of `layouts/partials/extend-footer.html`. Here's exactly what it does:

#### 3a. Scroll milestones — "Where do readers stop?"

**Mechanism:** On article pages (detected by `.article-content` element), the script:
1. Calculates the article body height
2. Creates 3 invisible markers at 50%, 75%, 100% of the article
3. An `IntersectionObserver` watches the markers (no scroll event spam — performant)
4. When a marker enters the viewport → fires `umami.track()`
5. Each milestone fires **only once** per page load (deduped)

**Events fired:**

| Event name | Fires when | Properties sent |
|---|---|---|
| `scroll-50` | Reader passes the halfway point | `{ article: "frame-graph-theory" }` |
| `scroll-75` | Reader reaches the 3/4 mark | `{ article: "frame-graph-theory" }` |
| `scroll-100` | Reader reaches the end | `{ article: "frame-graph-theory" }` |

**Budget cost:** 0–3 events per page visit (only milestones actually reached fire)

#### 3b. Code-copy tracking — "What code is useful?"

**Mechanism:** Listens for clicks on Blowfish's `.copy-button` elements (the "Copy" button on code blocks).

| Event name | Fires when | Properties sent |
|---|---|---|
| `code-copy` | Reader clicks copy on a code block | `{ article: "frame-graph-build-it" }` |

**Budget cost:** Rare. ~1 event per 10-20 article reads.

#### 3c. Collapsible expand tracking — "Do readers open collapsed diffs?"

**Mechanism:** Listens for `toggle` events on `<details class="code-diff-collapsible">` elements. Only fires on open (not close).

| Event name | Fires when | Properties sent |
|---|---|---|
| `details-open` | Reader opens a collapsed code-diff | `{ article: "frame-graph-build-it" }` |

**Budget cost:** Low. Only fires when someone actively expands a block.

#### 3d. Section tracking (OFF by default — enable later)

**Mechanism:** Watches `<h2>` headings inside `.article-content` with IntersectionObserver. Fires when each heading becomes visible.

| Event name | Fires when | Properties sent |
|---|---|---|
| `section` | H2 heading enters viewport | `{ article: "slug", name: "The Compile Step" }` |

**Why off by default:** Your articles have 5-8 H2s each. A full read would fire 5-8 extra events, eating into the 10K/month budget fast. Enable it when you understand your traffic volume.

---

### Step 4 — Event budget math

| Event | Avg per read | Est. per 100 reads |
|---|---|---|
| Page view (automatic) | 1 | 100 |
| `article:Title` (Blowfish built-in) | 1 | 100 |
| `scroll-50` | ~0.7 | ~70 |
| `scroll-75` | ~0.45 | ~45 |
| `scroll-100` | ~0.25 | ~25 |
| `code-copy` | ~0.05 | ~5 |
| `details-open` | ~0.1 | ~10 |
| **Total avg** | **~3.55** | **~355** |

**Free tier: 10,000 events ÷ 3.55 = ~2,800 article reads/month** ✅

---

### Step 5 — Test locally (you + me, ~5 min)

1. I build with `hugo server -D`
2. Open `http://localhost:1313/posts/frame-graph-theory/`
3. **DevTools → Network** → filter `umami` → verify script loads (or 404s if no ID yet)
4. **DevTools → Console** → scroll the article → verify tracking logs appear
5. The script detects `localhost` and logs to console instead of firing real events (no budget waste)

---

### Step 6 — Deploy (automatic, ~2 min)

```bash
git add config/_default/params.toml layouts/partials/extend-footer.html
git commit -m "feat: add Umami analytics + scroll depth tracking"
git push
```

GitHub Actions builds and deploys. Analytics goes live on `stoleckipawel.dev` within ~2 minutes.

---

### Step 7 — First-week checklist

| Day | What to check | Where |
|---|---|---|
| Day 1 | Script loads? Console errors? | Browser DevTools on live site |
| Day 1 | Page views appearing? | Umami → Dashboard → Real-time |
| Day 2-3 | Referrers showing up? | Umami → Referrers |
| Day 3 | Scroll events firing? | Umami → Events |
| Day 7 | Event budget usage | Umami → Settings → Usage |

---

### Step 8 — How to read the data

#### "Which article is most popular?"
→ Umami → **Pages** → sort by views

#### "Where do readers come from?"
→ Umami → **Referrers**
- `google.com` = organic search
- `t.co` / `twitter.com` = Twitter/X
- `reddit.com` = Reddit post
- `news.ycombinator.com` = Hacker News
- `(direct)` = typed URL, bookmarks, RSS readers

#### "Where do readers stop?" (the key insight)
→ Umami → **Events** → compare scroll counts vs page views:

| Article | Views | scroll-50 | scroll-75 | scroll-100 | Completion |
|---|---|---|---|---|---|
| Frame Graph — Theory | 200 | 140 (70%) | 90 (45%) | 50 (25%) | 25% |
| Frame Graph — Build It | 150 | 120 (80%) | 100 (67%) | 85 (57%) | 57% |

**How to interpret:**
- **scroll-50 is low** → intro/hook isn't compelling enough, readers bounce early
- **Sharp drop 50→75** → middle section loses readers, tighten or restructure
- **High scroll-100** → readers are very engaged, this article format works — do more like it
- **Build It has 57% completion vs Theory at 25%** → practical content retains better than abstract

#### "What code do readers copy?"
→ Umami → **Events** → filter `code-copy` → which articles generate copies = which code is practically useful

---

## Files modified (total: 2)

| File | Change |
|---|---|
| `config/_default/params.toml` | Fill `[umamiAnalytics]` section (~3 lines) |
| `layouts/partials/extend-footer.html` | Append scroll tracking script (~40 lines) |

No new dependencies. No build changes. No CSS changes. No npm packages.

---

## What's next?

**Give me your Umami Website ID and I'll implement Steps 2 + 3 in about 60 seconds.**

> ✅ **IMPLEMENTED** — Website ID `7a9b3579-eefc-45f3-9358-ea450ce0a0ac` configured. Scroll tracking script added. Ready to deploy.

---

## Future additions (after you have traffic data)

| Addition | When | What it gives you |
|---|---|---|
| Enable section-level H2 tracking | When you know monthly event usage | Per-section drop-off data |
| Google Search Console | Anytime | What search queries bring readers (free, no cookies) |
| Cloudflare Web Analytics | Anytime | Core Web Vitals RUM data (free, complements Umami) |
| Bing Webmaster Tools | Anytime | Covers Bing/Yahoo/DDG search data + SEO audit |
| Giscus comments | When you want engagement | GitHub-backed comments = reader signal |
| Lighthouse CI | When you have 10+ pages | Performance regression detection in CI |
