# Visual Feedback Integration — Proposal

Based on Adam's review: diffs are "blob of white characters," too many colors/effects cause eyes to lose focus.

---

## Priority 1 — Diff Readability (Adam's main concern)

Affects **Part II — Build It** (~30 code-diffs). No diffs in Parts I, III, IV.

| # | What | Where | Details |
|---|------|-------|---------|
| 1 | **Stronger diff backgrounds** | `assets/css/custom.css` L720–727 | `.cd-add` and `.cd-remove` both use `.12` alpha — far too faint on dark backgrounds. Bump to `.20`–`.25` for green, `.18`–`.22` for red. |
| 2 | **Add gutter +/- column** | `layouts/shortcodes/code-diff.html` | The shortcode renders raw `+`/`-` prefixed text in a single `<div>`. No dedicated gutter column showing +/- symbols — readers can't parse it as a diff at a glance. Add a fixed-width gutter `<span>` with the sign. |
| 3 | **Syntax highlighting** | `layouts/shortcodes/code-diff.html` | Currently all code is monochrome `#c9d1d9`. Adam said "kod trochę nieczytelny" — this is the single biggest win. Either run each line through Hugo/Chroma highlighting or add manual CSS token coloring for keywords/types/strings. |
| 4 | **Dim context lines more** | `assets/css/custom.css` L728–730 | `.cd-context` is `opacity: 0.5` — too close to added lines. Drop to `0.35` so the eye is drawn to changed lines. |

---

## Priority 2 — Visual Noise ("too many colors/effects, eyes lose focus")

| # | What | Where | Details |
|---|------|-------|---------|
| 5 | **Kill button pulse** | `assets/css/custom.css` L929 | `.widget-btn-primary` has `animation: interact-pulse 1.4s ease-in-out infinite` — every interactive widget's primary button pulses forever. Remove or make it one-shot. |
| 6 | **Reduce flow animations** | Part I `theory/index.md` L150–157 — 8 animated `flow-lg` paths on the DAG diagram. Part III `advanced/index.md` L109 — 4 animated `flow-md flow-green` arrows on the decision flowchart. Options: (a) remove `flow` classes entirely (static dashes), (b) add `animation-play-state: paused` by default + play on hover, (c) reduce to just 1–2 key edges. |
| 7 | **Tone down hover lift** | `assets/css/custom.css` L944–950 | `.fg-hoverable:hover` does `translateY(-2px)` + `box-shadow 0 6px 20px`. Used on ~14 cards across Parts I, III, IV. Reduce to `1px` lift or remove entirely — cards don't need to feel like buttons. |
| 8 | **Remove inline hover JS (Part II)** | `content/posts/frame-graph-build-it/index.md` L48–90 | The MVP progression has 3 `<a>` blocks with `onmouseover`/`onmouseout` JS for card transforms and border-color changes. Replace with CSS `:hover` or remove. |
| 9 | **Remove inline hover JS (Part I)** | `content/posts/frame-graph-theory/index.md` L195–215 | The lifecycle bar (DECLARE → COMPILE → EXECUTE) has inline `onmouseover`/`onmouseout` on each link. Same treatment — CSS-only or remove. |
| 10 | **Part III: reduce animated arrows** | `content/posts/frame-graph-advanced/index.md` L108–119 | The "Should this pass go async?" flowchart has 4 animated green flow arrows between decision boxes, each with different durations. These compete with the text content. |

---

## Summary by Article

| Article | Diffs to fix | Flow animations | Hoverable cards | Inline JS hover |
|---------|-------------|-----------------|-----------------|-----------------|
| Part I — Theory | 0 | 8 (DAG diagram) | ~6 | 3 (lifecycle bar) |
| Part II — Build It | ~30 | 0 | 3 (MVP timeline) | 3 (MVP timeline) + 4 (v2 step bar) |
| Part III — Beyond MVP | 0 | 4 (flowchart) | ~8 | 0 |
| Part IV — Production | 0 | 0 | 0 | 0 |

---

## Implementation Notes

- Items 1–4 are the highest-impact changes — CSS/shortcode-level fixes that affect every diff at once.
- Item 3 (syntax highlighting) is the most complex — requires shortcode rewrite or JS-based tokenizer.
- Items 5–10 are surgical: CSS tweaks or inline HTML edits in specific articles.
