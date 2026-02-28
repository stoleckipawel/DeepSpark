# Image Generation Prompts — Promotion Assets

> **Style rules for ALL prompts:**
> - No glow, no bloom, no lens flare
> - No textures, no grain, no noise
> - No gradients (or very subtle, max 2 colors)
> - No 3D renders
> - Flat colors, sharp edges, clean lines
> - Dark background (#1a1a2e or #0d1117)
> - Monospace or clean sans-serif font (Inter, JetBrains Mono)
> - Should look like a programmer made it in Photoshop in 30 minutes

---

## 1. OG / Social Cards (1200 × 630 px)

Used by: Discord embeds, Twitter cards, Reddit link previews, LinkedIn.
One per article. These are the most important assets.

---

### Part I — Theory

```
Flat technical diagram on a dark navy background (#0d1117). 

Center: five simple rounded rectangles arranged as a directed acyclic graph (DAG), connected by straight arrows. Boxes are labeled "GBuffer", "Lighting", "SSAO", "Composite", "Tonemap" in a small white monospace font. Each box has a flat muted color fill — steel blue, muted teal, dusty orange, slate purple, muted red. Arrows are thin white lines with small arrowheads.

Bottom-left corner: "DEEP SPARK" in small caps, clean white sans-serif, 14pt equivalent.
Top-right corner: "Part I of IV" in a small rounded pill badge, white text on dark grey (#2d2d2d) background.

Large title text centered above the diagram: "Frame Graph — Theory" in white, bold, clean sans-serif font.
Below the title in smaller grey (#8b949e) text: "How a DAG of render passes automates scheduling, barriers, and memory aliasing"

No glow. No shadows. No textures. No gradients. Flat and clean. 1200×630 pixels.
```

---

### Part II — Build It

```
Flat code snippet visualization on a dark navy background (#0d1117).

Center: three stacked horizontal bars representing code iterations, arranged vertically with small gaps. Each bar is a flat rounded rectangle with a different muted color:
- Top bar: small, steel blue fill, labeled "v1 — ~90 LOC" in white monospace
- Middle bar: medium width, muted teal fill, labeled "v2 — ~260 LOC" in white monospace  
- Bottom bar: widest, dusty orange fill, labeled "v3 — ~500 LOC" in white monospace

The bars grow progressively wider left-to-right, showing iteration growth. Small thin white arrows between bars pointing downward.

Bottom-left: "DEEP SPARK" in small caps, white sans-serif.
Top-right: "Part II of IV" pill badge, white on dark grey.

Large title: "Frame Graph — Build It" in white bold sans-serif.
Subtitle in grey (#8b949e): "Three C++ iterations from blank file to working frame graph"

No glow. No shadows. No textures. Completely flat. 1200×630 pixels.
```

---

### Part III — Beyond MVP

```
Flat technical diagram on dark navy background (#0d1117).

Center: two horizontal lanes (like swim lanes), one labeled "Graphics Queue" and one labeled "Compute Queue" in small grey (#8b949e) monospace text on the left side. 

Each lane contains 2-3 flat colored rounded rectangles (render passes) arranged left to right. Graphics lane: steel blue boxes. Compute lane: muted teal boxes. 

A single thin dashed white vertical line between two passes indicates a fence/sync point. Small white arrows show pass flow left to right within each lane.

Bottom-left: "DEEP SPARK" in small caps, white sans-serif.
Top-right: "Part III of IV" pill badge, white on dark grey.

Large title: "Frame Graph — Beyond MVP" in white bold sans-serif.
Subtitle in grey: "Async compute and split barriers"

No glow. No effects. Purely flat shapes and lines. 1200×630 pixels.
```

---

### Part IV — Production Engines

```
Flat comparison layout on dark navy background (#0d1117).

Center: two columns side by side, separated by a thin vertical white line.

Left column header: "UE5 RDG" in white monospace, with a small flat blue square icon.
Right column header: "Frostbite" in white monospace, with a small flat orange square icon.

Under each header: a stack of 4-5 short text labels in grey (#8b949e) monospace, listed vertically:
Left: "AddPass macros", "ERDGPassFlags", "Transient allocator", "700+ passes"
Right: "FrameGraph", "Pass callbacks", "Barrier batching", "Thread-pool recording"

Bottom-left: "DEEP SPARK" in small caps, white sans-serif.
Top-right: "Part IV of IV" pill badge, white on dark grey.

Large title: "Frame Graph — Production Engines" in white bold sans-serif.
Subtitle in grey: "How UE5 and Frostbite implement frame graphs at scale"

No glow. No textures. No logos. Clean flat layout. 1200×630 pixels.
```

---

## 2. Twitter Thread Images (1600 × 900 px)

Optional but high impact. Attach to specific tweets in the thread.

---

### For Tweet 3 (DAG concept)

```
Minimal flat DAG diagram on dark background (#0d1117). 1600×900 pixels.

Seven rounded rectangle nodes arranged in a clear DAG layout (top to bottom, some parallel). Labels in white monospace: "Depth Pre-pass", "GBuffer", "Shadow Map", "SSAO", "Lighting", "Post-FX", "Tonemap".

Each node has a flat solid fill — alternate between steel blue (#4a6fa5), muted teal (#458b74), and dusty orange (#c4764d). 

Edges are thin solid white lines with small triangular arrowheads. Layout flows roughly top-to-bottom with some parallel branches (e.g., Shadow Map and SSAO both feed into Lighting).

Below the diagram: three words in a row separated by arrows: "Declare → Compile → Execute" in white sans-serif, medium size.

No glow. No shadows. No background pattern. Pure flat vector look.
```

---

### For Tweet 4 (code iterations)

```
Flat side-by-side code block visualization on dark background (#0d1117). 1600×900 pixels.

Three panels arranged horizontally with thin grey borders and tiny gaps between them. Each panel has a dark slightly-lighter-than-background fill (#161b22).

Panel 1 header: "v1" in steel blue (#4a6fa5), with "~90 LOC" in grey below.
Panel 1 body: 4-5 lines of simplified pseudocode in white monospace, very small:
  AddPass("GBuffer", ...)
  CreateResource("Albedo", ...)
  Execute()

Panel 2 header: "v2" in muted teal (#458b74), "~260 LOC" in grey.
Panel 2 body: 4-5 lines:
  BuildEdges()
  TopologicalSort()  
  CullDeadPasses()
  ComputeBarriers()

Panel 3 header: "v3" in dusty orange (#c4764d), "~500 LOC" in grey.
Panel 3 body: 4-5 lines:
  ScanLifetimes()
  AssignMemoryBlocks()
  Compile() → Plan
  Execute(plan)

No syntax highlighting colors. All white text on dark panels. No glow, no effects.
```

---

### For Tweet 5 (async compute)

```
Flat two-lane timeline diagram on dark background (#0d1117). 1600×900 pixels.

Two horizontal swim lanes spanning the width:

Top lane labeled "Graphics" in grey (#8b949e) on the left. Contains 4 flat steel blue (#4a6fa5) rounded rectangles in a row labeled: "Depth", "GBuffer", "Lighting", "Tonemap".

Bottom lane labeled "Compute" in grey on the left. Contains 2 flat muted teal (#458b74) rounded rectangles: "SSAO", "Bloom". These are positioned to overlap in time with the Graphics passes (SSAO runs parallel to GBuffer/Lighting area).

One thin dashed white vertical line between the lanes marking a sync/fence point, with a tiny label "fence" in grey.

Simple thin white arrows showing data flow. 

No glow. No shadows. Pure flat shapes. Clean technical diagram style.
```

---

## 3. Reddit Post Preview (optional)

Reddit uses the OG image from above. No separate asset needed. The 1200×630 OG cards cover Reddit's link preview.

---

## 4. LinkedIn Post Image (1200 × 627 px)

If posting with an attached image (instead of link preview), use the Part I OG card or this variant:

```
Flat summary card on dark background (#0d1117). 1200×627 pixels.

Title: "Frame Graph — Deep Dive" in large white bold sans-serif, centered near the top.

Below: four horizontal rows, evenly spaced, each with:
- A Roman numeral (I, II, III, IV) in a flat colored circle (blue, teal, orange, red)
- A short description in white sans-serif next to it

Row 1: (I) "Theory — DAG, topological sort, barriers, aliasing"
Row 2: (II) "Build It — Three C++ iterations, ~500 LOC"  
Row 3: (III) "Beyond MVP — Async compute, split barriers"
Row 4: (IV) "Production — UE5 RDG & Frostbite at scale"

Bottom-left: "DEEP SPARK" small caps. Bottom-right: "stoleckipawel.dev" in grey.

No glow. No textures. Clean flat layout like a slide deck.
```

---

## 5. Discord Server Post (no special image)

Discord pulls the OG card from the URL automatically. No separate asset needed.

---

## Style reference summary

| Property | Value |
|---|---|
| Background | `#0d1117` (GitHub dark) or `#1a1a2e` |
| Primary text | `#ffffff` white |
| Secondary text | `#8b949e` grey |
| Accent blue | `#4a6fa5` steel blue |
| Accent teal | `#458b74` muted teal |
| Accent orange | `#c4764d` dusty orange |
| Accent purple | `#7c6f9e` slate purple |
| Accent red | `#a85454` muted red |
| Font (titles) | Inter Bold or similar clean sans-serif |
| Font (code/labels) | JetBrains Mono or similar monospace |
| Corners | Small radius (4-8px) on rectangles |
| Lines/arrows | 1-2px solid white, small triangular heads |
| **Banned** | Glow, bloom, noise, grain, textures, 3D, gradients, drop shadows |

---

## Generation tips

- **If using Midjourney/DALL-E:** Add `--no glow, bloom, lens flare, texture, grain, 3D render, photorealistic` to every prompt
- **If using Canva:** Use the "Presentation" template at custom size, dark background, shape tools only
- **If doing it manually in Photoshop/Figma:** The prompts above describe exactly what to draw — flat rectangles, lines, text
- **Test the result:** Paste your image temporarily in a Discord message to yourself — does it look like a programmer made it, or does it look AI-generated? If it looks AI, simplify further.
