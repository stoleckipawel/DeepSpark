// Frame Graph MVP v2 -- Usage Example
// Compile: g++ -std=c++17 -o example_v2 example_v2.cpp
#include "frame_graph_v2.h"
#include <cstdio>

int main() {
    printf("=== Frame Graph v2: Dependencies & Barriers ===\n");
    printf("Adds: dependency DAG, topo-sort, pass culling, auto barriers.\n\n");

    FrameGraph fg;
    auto depth = fg.createResource({1920, 1080, Format::D32F});
    auto gbufA = fg.createResource({1920, 1080, Format::RGBA8});
    auto hdr   = fg.createResource({1920, 1080, Format::RGBA16F});
    auto debug = fg.createResource({1920, 1080, Format::RGBA8});
    printf("Resources created:\n");
    printf("  [0] depth   1920x1080 D32F\n");
    printf("  [1] gbufA   1920x1080 RGBA8\n");
    printf("  [2] hdr     1920x1080 RGBA16F\n");
    printf("  [3] debug   1920x1080 RGBA8  (unused -- should be culled)\n\n");

    fg.addPass("DepthPrepass",
        [&]() { fg.write(0, depth); },
        [&](/*cmd*/) { printf("  >> exec: DepthPrepass  (writes depth)\n"); });

    fg.addPass("GBuffer",
        [&]() { fg.read(1, depth); fg.write(1, gbufA); },
        [&](/*cmd*/) { printf("  >> exec: GBuffer       (reads depth, writes gbufA)\n"); });

    fg.addPass("Lighting",
        [&]() { fg.read(2, gbufA); fg.write(2, hdr); },
        [&](/*cmd*/) { printf("  >> exec: Lighting      (reads gbufA, writes hdr)\n"); });

    // This pass writes debug but nothing reads it -- the graph will cull it.
    fg.addPass("DebugOverlay",
        [&]() { fg.write(3, debug); },
        [&](/*cmd*/) { printf("  >> exec: DebugOverlay  (writes debug)\n"); });

    printf("Passes added: DepthPrepass, GBuffer, Lighting, DebugOverlay\n");
    printf("Dependencies: GBuffer->DepthPrepass, Lighting->GBuffer\n");
    printf("DebugOverlay has no consumers -- it's a dead pass.\n\n");

    printf("Compiling graph...\n");
    fg.execute();

    printf("\n[OK] v2 complete. Automatic dependency sorting, dead-pass culling,\n");
    printf("  and barrier insertion -- no manual work needed.\n");
    printf("  But each resource gets its own GPU allocation (no reuse).\n");
    printf("  -> v3 will add lifetime analysis and memory aliasing.\n");
    return 0;
}
