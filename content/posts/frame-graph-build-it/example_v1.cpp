// Frame Graph MVP v1 -- Usage Example
// Compile: g++ -std=c++17 -o example_v1 example_v1.cpp
#include "frame_graph_v1.h"
#include <cstdio>

int main() {
    printf("=== Frame Graph v1: Declare & Execute ===\n");
    printf("No dependencies, no barriers, no aliasing.\n");
    printf("Passes execute in declaration order.\n\n");

    FrameGraph fg;

    auto depth  = fg.createResource({1920, 1080, Format::D32F});
    auto gbufA  = fg.createResource({1920, 1080, Format::RGBA8});
    auto gbufN  = fg.createResource({1920, 1080, Format::RGBA8});
    auto hdr    = fg.createResource({1920, 1080, Format::RGBA16F});
    printf("Resources created:\n");
    printf("  [0] depth   1920x1080 D32F\n");
    printf("  [1] gbufA   1920x1080 RGBA8\n");
    printf("  [2] gbufN   1920x1080 RGBA8\n");
    printf("  [3] hdr     1920x1080 RGBA16F\n\n");

    fg.addPass("DepthPrepass",
        [&]()          { printf("  setup: DepthPrepass registered\n"); },
        [&](/*cmd*/)   { printf("  [pass 0] DepthPrepass  -> writes depth\n"); });

    fg.addPass("GBuffer",
        [&]()          { printf("  setup: GBuffer registered\n"); },
        [&](/*cmd*/)   { printf("  [pass 1] GBuffer       -> reads depth, writes gbufA, gbufN\n"); });

    fg.addPass("Lighting",
        [&]()          { printf("  setup: Lighting registered\n"); },
        [&](/*cmd*/)   { printf("  [pass 2] Lighting      -> reads gbufA, gbufN, writes hdr\n"); });

    printf("\nPasses added (setup phase):\n");
    printf("\nExecuting (declaration order -- no sorting):\n");
    fg.execute();

    printf("\n[OK] v1 complete. Passes ran in order, but:\n");
    printf("  - No dependency tracking -- order is manual\n");
    printf("  - No barrier insertion -- GPU hazards possible\n");
    printf("  - No dead-pass culling\n");
    printf("  - No memory aliasing\n");
    printf("  -> v2 will fix the first three.\n");
    return 0;
}
