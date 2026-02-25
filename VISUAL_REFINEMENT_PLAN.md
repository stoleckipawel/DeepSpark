# Visual Refinement Plan — DeepSpark

> **Goal**: Every visual element should have a clear purpose. Colour should guide
> comprehension, not decorate. Educational content demands navigability, consistency,
> and minimal cognitive overhead.

---

## Quick Reference — All Steps

| # | Step | What changes | Where |
|---|------|-------------|-------|
| 16 | Reduce decorative borders | Strip redundant layers (dual borders, invisible overlays) | `custom.css`, `about.md` |
| 17 | Review connector styling | Remove or replace `.fg-connector` if no longer meaningful | `custom.css`, articles |
| 18 | Extract inline styles to classes | Move repeated grid/badge/divider patterns into CSS classes | `custom.css`, all articles |
| 19 | Image optimisation | Check sizes, convert to WebP if >200 KB | `static/images/` |
| 20 | Reduce visual density | Simplify card backgrounds, comparison grids, decorative overlays | articles, `custom.css`, `about.md` |

| 21 | Reduce rainbow / colour saturation | Downgrade decorative borders to neutrals, lower flow opacity | articles, `custom.css`, `deepspark.css` |
| 22 | Add collapsible code-diffs | `collapsed` param on `code-diff` shortcode → `<details>` wrapper | `code-diff.html`, `frame-graph-build-it/index.md` |
| 23 | Split long diffs with bridging prose | Max ~40 lines per diff; split at `@@` markers, add 1–3 sentences | `frame-graph-build-it/index.md` |

---

## Current State Assessment

### What Works Well
- **Colour token system** (`--ds-*` in `deepspark.css`) is thorough and almost
  universally used. Semantic mapping (`accent`, `highlight`, `warm`, `soft`) keeps
  the palette maintainable.
- **Structural consistency** across all 4 articles — each opens with `article-nav`,
  `ds-series-nav`, closes with `ds-article-footer`.
- **Interactive widgets** share a unified structure (`.interactive-widget` →
  `.widget-header` → controls → canvas → explain area).
- **Status colours** (`success`, `danger`, `warn`, `info`, `code`) provide a
  clear semantic language for readers — green = good, red = bad, blue = info,
  purple = code.

### What Needs Refinement
Visual inconsistencies, orphaned patterns, and a few accessibility gaps
undermine the otherwise solid foundation.

---

## Phase 1 — Consistency Fixes (Quick Wins)

These are mechanical changes that unify the existing visual language.

### 1.1 Unify Widget Active-Button Colour

| Widget | Active colour | Correct? |
|--------|--------------|----------|
| DAG | `--ds-info` (blue) | ✅ |
| Toposort | `--ds-info` | ✅ |
| Async | `--ds-info` | ✅ |
| Barriers | `--ds-info` | ✅ |
| Aliasing | `--ds-info` | ✅ |
| **Full Pipeline** | **`--ds-code` (purple)** | ❌ |

**Action**: Change `.pipe-ex-active` and `.pipe-focus-active` in
`interactive-full-pipeline.html` from `--ds-code` / `--ds-code-rgb` to
`--ds-info` / `--ds-info-rgb`. All widgets should use the same active colour
so readers recognise the pattern instantly.

**Purpose**: *Consistency reduces cognitive load — users learn the UI once and
apply it everywhere.*

### 1.2 Unify Async Example-Button Base Class

The Async widget defines `.async-ex-btn` with its **own** base styling
(padding, border, background, font-size, dark mode) instead of extending
`.widget-btn` like every other widget.

**Action**: Add `class="widget-btn async-ex-btn ..."` to the three Async
example `<button>` elements and strip the redundant base properties from the
`.async-ex-btn` CSS rule (keep only the `border-radius:999px` pill override).

**Purpose**: *Identical buttons teach the reader "these are all the same kind
of control" without any extra visual parsing.*

### 1.3 Deduplicate `@keyframes shimmer`

Both `_index.md` and `about.md` define `@keyframes shimmer` independently.

**Action**: Move the `shimmer` keyframe into `custom.css` (one definition) and
remove the duplicates from both markdown files.

**Purpose**: *Single source of truth — a change to the animation speed or
easing propagates everywhere.*

### 1.4 Widget Header Style Ownership

`.widget-header` base styles (border, padding, dark mode) are defined
**inside** `interactive-dag.html`. If a page uses a widget that isn't DAG
(e.g., only Full Pipeline), these styles may not be loaded.

Meanwhile, `custom.css` also has `.widget-header` rules
(the `::after` badge + `border-image` gradient) — creating a split definition.

**Action**: Consolidate **all** `.widget-header` and `.widget-btn` base
styles into `custom.css`. Remove duplicates from `interactive-dag.html`. Each
shortcode should only contain its **own** prefixed styles (e.g., `.dag-ex-btn`).

**Purpose**: *Prevents broken styling when a widget appears on a page without
the DAG shortcode. Single source of truth for shared UI.*

---

## Phase 2 — Purposeful Colour Coding

Ensure every colour carries meaning. Document the semantic intent.

### 2.1 Establish a Colour Legend for Content

The site uses colours semantically but never tells the reader the mapping.
Adding a brief visual legend to the series navigation or the first article
would anchor the colour coding in the reader's mind.

| Colour | Token | Educational meaning |
|--------|-------|---------------------|
| 🟠 Orange | `--ds-accent` | Brand / primary indicators |
| 🟡 Gold | `--ds-highlight` | Highlights, emphasis, takeaways |
| 🟤 Burnt | `--ds-warm` | Warm secondary, caution-adjacent |
| 🔵 Blue | `--ds-info` | Information, widget interactivity |
| 🟢 Green | `--ds-success` | Correct, optimised, passing |
| 🔴 Red | `--ds-danger` | Error, problem, cost |
| 🟡 Amber | `--ds-warn` | Warning, fence, caution |
| 🟣 Purple | `--ds-code` | Code references, implementation detail |
| 🔵 Indigo | `--ds-indigo` | Structural chrome (series nav, footer) |

**Action**: Consider adding a one-line tooltip or legend to the series nav
banner (`ds-series-nav`) that says *"Colour guide: 🟢 good · 🔴 bad ·
🔵 interactive · 🟡 note"* — only if it can be done without clutter.

**Purpose**: *Explicit colour semantics make diagrams self-documenting. Readers
won't wonder "why is this node green?"*

### 2.2 Replace Off-Palette Hex Colours

| File | Colour | Current | Palette equivalent |
|------|--------|---------|-------------------|
| `frame-graph-build-it/index.md` | `#ca8a04` (amber) | UML v1 accent | `--ds-warn-dark` (#d97706) — close enough |
| `frame-graph-build-it/index.md` | `#059669` (emerald) | UML v3 accent | `--ds-success-dark` (#16a34a) — same family |

**Action**: Replace `#ca8a04` → `#d97706` (== `--ds-warn-dark`) and
`#059669` → `#16a34a` (== `--ds-success-dark`) in the 3 legend swatches
and ~6 Mermaid `style` directives. These contexts don't support CSS variables,
but the hex values can still **match** the palette tokens.

The existing `#6366f1` is already exactly `--ds-indigo` — no change needed.

**Purpose**: *Even where CSS variables can't be used, keeping hex values
aligned with the palette prevents "off-brand" colours.*

### 2.3 SVG Gradient-Stop Alignment Audit

The large DAG diagram in `frame-graph-theory/index.md` uses hardcoded gradient
stops that already match `--ds-*` values (e.g., `#60a5fa` == `--ds-info-light`).
No action needed — these are correct. Document this in a code comment at the
SVG definition.

**Action**: Add a comment above the `<linearGradient>` definitions noting
which `--ds-*` token each hex maps to, so future edits stay aligned.

---

## Phase 3 — Navigation Polish

### 3.1 Unify Progress Bar Gradients

| Nav system | Gradient |
|-----------|----------|
| Article nav (`anav-bar`) | 2-colour: `--ds-accent → --ds-highlight` |
| Site nav (`snav-bar`) | 3-colour: `--ds-warm → --ds-accent → --ds-highlight` |

**Action**: Align both to the **same** gradient. Recommendation: use the
2-colour gradient (`--ds-accent → --ds-highlight`) for both, as it's simpler
and the warm tone at the leading edge of the 3-colour version is barely
visible at low scroll percentages.

**Purpose**: *Two progress bars on the same site should look the same to avoid
confusing readers about whether they're seeing the same control.*

### 3.2 Unify Mobile FAB Styling

| Nav system | FAB background |
|-----------|----------------|
| Article nav | Solid `--ds-accent` |
| Site nav | Gradient `--ds-accent → --ds-warm` |

**Action**: Use the solid `--ds-accent` for both. The gradient on a 40px
circle is imperceptible.

**Purpose**: *Consistent brand touch-point across pages.*

### 3.3 Rail Backdrop Colours

Both nav rails use hardcoded `rgba(255,255,255,.92)` (light) and
`rgba(23,23,23,.92)` (dark) for the backdrop blur background.

**Action**: Replace with `rgba(var(--color-neutral-50),.92)` and
`rgba(var(--color-neutral-900),.92)` respectively, so the backdrop adapts if
the neutral scale is ever adjusted.

**Purpose**: *Keeps the nav chrome connected to the colour system.*

---

## Phase 4 — Accessibility & Motion

### 4.1 Add `prefers-reduced-motion` to About Page

The about page defines `@keyframes shimmer` but **does not** include a
`prefers-reduced-motion` media query. The landing page does.

**Action**: Add the reduced-motion block to `about.md`'s `<style>`:

```css
@media (prefers-reduced-motion: reduce) {
  *, *::before, *::after {
    animation-duration: 0.01ms !important;
    transition-duration: 0.01ms !important;
  }
}
```

If shimmer is moved to `custom.css` (per 1.3), this block should go there
instead — one place, covers everything.

**Purpose**: *Users with vestibular disorders or motion sensitivity should
never see infinite animations.*

### 4.2 Flow Animation Reduced-Motion

The flow-charging system (`.flow-lg`, `.flow-md`, `.flow-sm`, `.flow-mini`)
defines multi-second looping animations on SVG edges. These should also be
disabled under `prefers-reduced-motion`.

**Action**: Add to `custom.css`:

```css
@media (prefers-reduced-motion: reduce) {
  .flow-lg, .flow-md, .flow-sm, .flow-mini {
    animation: none !important;
  }
}
```

**Purpose**: *Animated data-flow arrows are decorative — the article is
readable without them.*

### 4.3 Review Colour Contrast on Status Tokens

Some status colours on dark backgrounds may have marginal contrast:

| Token | Hex | WCAG on `#0B0B0E` | Check |
|-------|-----|-------------------|-------|
| `--ds-success` | `#22c55e` | ~6.7:1 | ✅ AA |
| `--ds-danger` | `#ef4444` | ~4.7:1 | ⚠️ Borderline AA-large |
| `--ds-warn` | `#f59e0b` | ~7.2:1 | ✅ AA |
| `--ds-info` | `#3b82f6` | ~3.8:1 | ⚠️ Below AA |
| `--ds-code` | `#8b5cf6` | ~3.6:1 | ⚠️ Below AA |

**Action**: For text usage, swap to the `-light` variants already defined
(`--ds-info-light` = `#60a5fa` ≈ 5.5:1, `--ds-code-light` = `#a78bfa` ≈ 5.8:1).
The base tokens should only be used for backgrounds, borders, or large text
(18pt+), not for body-size inline text on the dark `#0B0B0E` background.

**Purpose**: *WCAG 2.1 Level AA compliance for text readability.*

---

## Phase 5 — Class Naming Consolidation

### 5.1 Document the Naming Conventions

Three prefixing patterns coexist:

| Prefix | Used in | Examples |
|--------|---------|----------|
| `ds-` | Custom.css shared components | `ds-callout`, `ds-card`, `ds-series-nav` |
| `fg-` | Articles (Theory, Advanced) | `fg-hoverable`, `fg-connector` |
| `diagram-` | Production article | `diagram-flow`, `diagram-bars`, `diagram-struct` |

**Action**: No class renaming (too risky for breaking inline HTML in articles).
Instead, add a **comment block** at the top of `custom.css` documenting the
convention:

```css
/*
 * Class-naming conventions:
 *   ds-*       Shared components (callout, card, badge, nav, etc.)
 *   fg-*       Frame Graph article-specific elements
 *   diagram-*  Static diagram styles (bars, tiles, struct, etc.)
 *   widget-*   Interactive widget shared UI (header, btn)
 *   [name]-*   Widget-specific (dag-, topo-, async-, barrier-, alias-, pipe-)
 */
```

**Purpose**: *New contributors (or future-you) can find and understand classes
instantly.*

### 5.2 Audit Unused Classes

After all glow/fade cleanup, some classes may be orphaned in `custom.css`.

**Action**: Run a grep for each `custom.css` class against all content and
layout files. Remove any class that has zero references outside `custom.css`
itself.

**Purpose**: *Dead CSS adds load time and confusion.*

---

## Phase 6 — Visual Weight & Hierarchy

### 6.1 Reduce Decorative Borders

Several elements stack multiple visual cues (border + background gradient +
opacity shift + icon). In educational content, each visual layer should earn
its place.

Candidates for simplification:

| Element | Current cues | Suggestion |
|---------|-------------|------------|
| `.ds-callout` | Border-left (4px) + full border + gradient BG | Keep border-left only; drop the full border |
| `.ds-mvp-card` | Border + background + title highlight | OK — card needs enclosure |
| `.about-hero` | Border + shimmer pseudo + radial pseudo + gradient BG | Consider dropping the radial `::after` — it's invisible unless you know it's there |
| Widget header | `border-top` gradient (3px) + `::after` badge + `border-bottom` | Three borders on one element. Consider removing `border-top` since the gradient `border-image` overrides it visually |

**Action**: Evaluate each on the live site. Remove the layer that adds the
least visual information.

**Purpose**: *Fewer visual layers = faster scanning for readers.*

### 6.2 Connector Styling

`.fg-connector` is now a static element (opacity `.2`, `height: 28px`). It
exists as a visual bridge between sections.

**Action**: Review whether these connectors are still needed post-animation-
removal. If they read as thin grey lines without clear meaning, consider
removing them entirely or replacing with a subtle `<hr>`.

**Purpose**: *A connector that doesn't connect is visual noise.*

---

## Phase 7 — Content-Level Visual Hygiene

### 7.1 Inline Style Reduction

All four articles use heavy inline `style=""` attributes (grid layouts, color,
padding, etc.). While functional, this makes visual changes require editing
markdown, not CSS.

**Action** (low-priority): Gradually extract frequently repeated inline
patterns into `custom.css` classes.

Priority patterns to extract:
- Comparison grids (`display:grid;grid-template-columns:1fr 1fr;gap:...`)
- Tag/badge spans (`font-size:.75em;padding:.2em .55em;border-radius:5px;...`)
- Section dividers / icon-label rows

**Purpose**: *CSS classes are reusable and theme-switchable. Inline styles are
not.*

### 7.2 Image Optimisation Check

Verify that all images in `static/images/` are served at appropriate sizes.
Hugo can generate responsive variants via `resources.Resize`.

**Action**: Check if any images exceed 200 KB uncompressed. Consider WebP
conversion for the cover images.

**Purpose**: *Page load performance directly affects educational UX.*

---

## Implementation Priority

> Superseded — see **Updated Implementation Priority** at the bottom of this
> file (includes Phase 8 feedback items).

---

## Phase 8 — Reader Feedback (Jacek, Feb 2025)

Raw feedback from a real reader. These observations are independent from
the automated audit above and carry extra weight because they reflect first
impressions.

### 8.1 "Pages generally don't look like this — there's probably a reason"

The overall visual density — heavy inline styling, coloured borders, gradients,
animated flow particles, custom cards — creates a "this doesn't look like a
normal article" feeling. Normal tech articles are mostly typography + code
blocks + the occasional diagram.

**Root cause**: Many visual layers were added for polish, but each one
moves the page further from the conventions readers expect.

**Action**: Phase 6 (Reduce Decorative Borders) and Phase 1 (Consistency
Fixes) directly address this. Also:
- Remove or simplify `.about-hero::after` (radial overlay)
- Consider making comparison grids simpler (plain 2-col with minimal border)
- Reduce the number of gradient backgrounds on cards — a solid subtle
  background is enough

**Guiding principle**: *Match reader expectations. Save visual complexity for
the interactive widgets, where it earns its keep.*

### 8.2 "Too much rainbow" — reduce colour saturation

The page uses ~10 distinct accent colours across tokens, badges, borders,
and flow animations. While each has a semantic meaning, the cumulative
effect reads as "rainbow."

**Action**:
- Audit every inline `border: ... var(--ds-*)` and `background: ... var(--ds-*)`
  usage. Where colour doesn't encode meaning (e.g., decorative card borders),
  downgrade to `--color-neutral-300` / `--color-neutral-600`.
- Reserve saturated brand colours (`--ds-accent`, `--ds-highlight`, `--ds-warm`)
  for **headings, navigation, and interactive controls only**.
- Status colours (`success`, `danger`, `warn`, `info`) should appear
  **only** in contexts where they carry semantic weight (pass/fail, before/after,
  caution).
- Reduce opacity of flow-particle animations (`--ds-flow-opacity: 0.55` →
  `0.35`) so they feel like subtle motion rather than neon tracks.

**Purpose**: *Fewer colours on screen means the colours that remain carry
more meaning.*

### 8.3 Part 2 (Build It) is "one big diff" — feels like reading P4 history

The Build It article contains **12 `code-diff` blocks** and **9 `include-code`
blocks** across 3 iterations (v1 → v2 → v3). Each code-diff renders fully
expanded with coloured +/- lines. A reader scrolling through sees an
unbroken wall of green-and-red code.

Current diff inventory:

| Iteration | Diff blocks | Approximate lines of diff |
|-----------|------------|--------------------------|
| v1 (scaffold) | 3 diffs | ~80 lines |
| v1 → v2 | 4 diffs | ~200 lines |
| v2 → v3 | 5 diffs | ~200 lines |
| **Total** | **12 diffs** | **~480 lines of diff** |

Plus 9 `include-code` blocks (full source + examples), most already collapsed
via `compact="true"`.

**Problems identified**:
1. `code-diff` shortcode has **no collapse/expand** — all diffs are always
   open. There is no way for a reader to skip a diff.
2. The article tries to be **both** a conceptual explanation and a
   hands-on implementation tutorial. This dual purpose means readers who want
   theory are forced through diffs, and implementers have to wade through
   prose.
3. Jacek: *"where there's a shitload of code and it's maybe not crucial,
   give people the ability to skip."*

**Action** (approach options, pick one or combine):

**Option A — Collapsible code-diffs** (recommended, least disruptive):
Add a `collapsed` parameter to the `code-diff` shortcode. When set, the diff
renders inside a `<details>` element with a summary showing the diff title +
line count. The reader clicks to expand. This matches how `include-code`
already works with `compact="true"`.

Implementation sketch for `code-diff.html`:
```html
{{ $collapsed := .Get "collapsed" | default "" }}
{{ if eq $collapsed "true" }}
<details class="code-diff code-diff-collapsed">
  <summary class="cd-title">{{ $title }} <span class="cd-hint">(click to expand diff)</span></summary>
  ...lines...
</details>
{{ else }}
<div class="code-diff">
  <div class="cd-title">{{ $title }}</div>
  ...lines...
</div>
{{ end }}
```

Then in the article, mark the **long** diffs (>30 lines) as collapsed:
```
{{</* code-diff title="v2 — Barriers, CompiledPlan & compile/execute split" collapsed="true" */>}}
```

Short diffs (≤20 lines) stay open since they're quick to scan.

**Option B — Theory/Implementation split** (bigger restructuring):
Split Part 2 into two sub-parts:
- **Part 2a: "Build It — Concepts"** — explains what v1, v2, v3 add
  (resource versioning, topology sort, barriers, aliasing) with small
  pseudocode snippets and diagrams. No full diffs.
- **Part 2b: "Build It — Implementation"** — the full diff walkthrough
  for readers who want to code along.

This matches Jacek's suggestion (*"one purely theoretical, second
implementation"*) but requires restructuring content and updating navigation.

**Option C — Hybrid** (recommended combination):
Keep the single article but add a prominent section toggle at the top:
*"This article includes step-by-step C++ diffs. To focus on concepts,
collapse all code."* with a single "Collapse All Diffs" / "Expand All Diffs"
button that toggles every `<details class="code-diff-collapsed">` at once.
This way the article serves both audiences without splitting the content.

**Purpose**: *Let readers control their depth. Implementation-focused readers
expand everything; concept-focused readers skip the diffs.*

### 8.5 Maximum diff length — split long diffs with prose

Even with collapsibility (8.3), a single diff block should never exceed a
digestible length. When a diff is too long the reader loses track of *why*
each chunk exists — it becomes a wall of green lines with no narrative thread.

**Rule**: A single `code-diff` block should contain at most **~40 lines of
diff content**. If it exceeds that, split it into smaller blocks and insert
a sentence or two of prose between them explaining what the next chunk does
and why it matters.

Current offenders (measured from the article source):

| Diff block | Lines | Action |
|-----------|-------|--------|
| v1→v2 — Resource versioning & dependency tracking | 82 | Split into 2–3 blocks: (1) new data types, (2) FrameGraph class changes, (3) Read/Write/ReadWrite impl |
| v2 — Barriers, CompiledPlan & compile/execute split | 113 | Split into 3–4 blocks: (1) ResourceState + Barrier types, (2) ImportResource + CompiledPlan, (3) ComputeBarriers(), (4) Compile() + Execute() |
| v3 — Allocation helpers & lifetime scan | 51 | Split into 2: (1) allocation helpers, (2) lifetime scan loop |
| v3 — Free-list allocator & header changes | 57 | Split into 2: (1) free-list logic, (2) header struct changes |
| v3 — ComputeBarriers() with aliasing + EmitBarriers() | 63 | Split into 2: (1) ComputeBarriers() body, (2) EmitBarriers() + Compile() wiring |
| v2 — Edge building + Kahn's topological sort | 44 | Borderline — split into 2 if natural break exists, otherwise OK |
| All others (≤29 lines) | — | Keep as-is |

The existing `@@` section markers inside each diff already indicate logical
boundaries — use those as natural split points.

**Prose between splits should**:
- Be 1–3 sentences max (not a paragraph)
- Name the concept the next block introduces (e.g., *"With resource versions
  in place, we can now wire Read() and Write() — each call looks up the
  current version's writer and adds a dependency edge."*)
- Never repeat what the code already says — point out *why*, not *what*

**Purpose**: *Short diffs with bridging prose create a narrative rhythm.
The reader processes one concept, reads 20–40 lines of code, absorbs it,
then moves to the next. Long unbroken diffs break this rhythm and become
a P4 changelist.*

### 8.4 Glows confirmed distracting

Jacek: *"I need to remove all those glows because you can actually go crazy"*

**Status**: ✅ Already handled — all glow and halo effects were removed in a
previous session. The shimmer border animation on the landing page and about
page remains as the only animated decorative element (subtle, single-colour).

---

## Updated Implementation Priority

| Priority | Phase | Effort | Impact |
|----------|-------|--------|--------|
| 🔴 High | **8.3 Collapsible code-diffs** | 30 min | Reader experience |
| 🔴 High | **8.5 Split long diffs with prose** | 1 hr | Readability |
| 🔴 High | **8.2 Reduce rainbow / colour saturation** | 1-2 hrs | Visual tone |
| 🔴 High | **8.1 Reduce visual density / match conventions** | 1 hr | First impression |
| 🔴 High | 1.1 Unify widget active colour | 5 min | Consistency |
| 🔴 High | 1.4 Consolidate widget-header CSS | 20 min | Prevents broken styles |
| 🔴 High | 4.1 Reduced-motion on about page | 2 min | Accessibility |
| 🟠 Medium | 1.2 Async button base class | 5 min | Consistency |
| 🟠 Medium | 1.3 Deduplicate shimmer keyframe | 10 min | Maintainability |
| 🟠 Medium | 2.2 Off-palette hex alignment | 10 min | Colour coherence |
| 🟠 Medium | 3.1 Unify progress bar gradient | 5 min | Visual unity |
| 🟠 Medium | 3.2 Unify FAB styling | 3 min | Visual unity |
| 🟠 Medium | 4.2 Flow animation reduced-motion | 3 min | Accessibility |
| 🟡 Low | 2.1 Colour legend in series nav | 15 min | Educational clarity |
| 🟡 Low | 2.3 SVG gradient comments | 5 min | Documentation |
| 🟡 Low | 3.3 Rail backdrop tokens | 5 min | Token adherence |
| 🟡 Low | 4.3 Contrast audit | 30 min | Accessibility |
| 🟡 Low | 5.1 Naming convention comment | 5 min | Documentation |
| 🟡 Low | 5.2 Dead class audit | 20 min | Cleanup |
| ⚪ Future | 6.1 Reduce decorative borders | 30 min | Visual clarity |
| ⚪ Future | 6.2 Connector review | 10 min | Visual clarity |
| ⚪ Future | 7.1 Inline style extraction | 2+ hrs | Maintainability |
| ⚪ Future | 7.2 Image optimisation | 30 min | Performance |
| ⚪ Future | **8.3B Theory/Implementation split** | 3+ hrs | Content architecture |

---

*Plan generated from full-site visual audit + reader feedback (Jacek, Feb 2025).
Each item includes its motivation so decisions can be revisited without
re-auditing.*
