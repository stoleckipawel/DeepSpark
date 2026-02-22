// Frame Graph MVP v2 -- Usage Example
// Compile: clang++ -std=c++17 -o example_v2 example_v2.cpp
#include "frame_graph_v2.h"
#include "frame_graph_v2.cpp"  // single-TU build (Godbolt)
#include <cstdio>

int main() {
    printf("=== Frame Graph v2: Dependencies & Barriers ===\n");

    FrameGraph fg;

    // Import the swapchain backbuffer — externally owned.
    auto backbuffer = fg.ImportResource({1920, 1080, Format::RGBA8},
                                        ResourceState::Present);

    auto depth = fg.CreateResource({1920, 1080, Format::D32F});
    auto gbufA = fg.CreateResource({1920, 1080, Format::RGBA8});
    auto hdr   = fg.CreateResource({1920, 1080, Format::RGBA16F});
    auto debug = fg.CreateResource({1920, 1080, Format::RGBA8});

    fg.AddPass("DepthPrepass",
        [&]() { fg.Write(0, depth); },
        [&](/*cmd*/) { printf("  >> exec: DepthPrepass\n"); });

    fg.AddPass("GBuffer",
        [&]() { fg.Read(1, depth); fg.Write(1, gbufA); },
        [&](/*cmd*/) { printf("  >> exec: GBuffer\n"); });

    fg.AddPass("Lighting",
        [&]() { fg.Read(2, gbufA); fg.Write(2, hdr); },
        [&](/*cmd*/) { printf("  >> exec: Lighting\n"); });

    // Present — writes to the imported backbuffer.
    fg.AddPass("Present",
        [&]() { fg.Read(3, hdr); fg.Write(3, backbuffer); },
        [&](/*cmd*/) { printf("  >> exec: Present\n"); });

    // Dead pass — nothing reads debug, so the graph will cull it.
    fg.AddPass("DebugOverlay",
        [&]() { fg.Write(4, debug); },
        [&](/*cmd*/) { printf("  >> exec: DebugOverlay\n"); });

    fg.Execute();
    return 0;
}
