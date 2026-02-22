// Frame Graph MVP v3 -- Usage Example
// Compile: g++ -std=c++17 -o example_v3 example_v3.cpp frame_graph_v3.cpp
#include "frame_graph_v3.h"
#include "frame_graph_v3.cpp"  // single-TU build (Godbolt)
#include <cstdio>

int main() {
    printf("=== Frame Graph v3: Lifetimes & Memory Aliasing ===\n");

    FrameGraph fg;

    // Import the swapchain backbuffer — externally owned.
    // The graph tracks barriers but won't alias it.
    auto backbuffer = fg.ImportResource({1920, 1080, Format::RGBA8},
                                        ResourceState::Present);

    auto depth = fg.CreateResource({1920, 1080, Format::D32F});
    auto gbufA = fg.CreateResource({1920, 1080, Format::RGBA8});
    auto gbufN = fg.CreateResource({1920, 1080, Format::RGBA8});
    auto hdr   = fg.CreateResource({1920, 1080, Format::RGBA16F});
    auto bloom = fg.CreateResource({960,  540,  Format::RGBA16F});
    auto debug = fg.CreateResource({1920, 1080, Format::RGBA8});

    fg.AddPass("DepthPrepass",
        [&]() { fg.Write(0, depth); },
        [&](/*cmd*/) { printf("  >> exec: DepthPrepass\n"); });

    fg.AddPass("GBuffer",
        [&]() { fg.Read(1, depth); fg.Write(1, gbufA); fg.Write(1, gbufN); },
        [&](/*cmd*/) { printf("  >> exec: GBuffer\n"); });

    fg.AddPass("Lighting",
        [&]() { fg.Read(2, gbufA); fg.Read(2, gbufN); fg.Write(2, hdr); },
        [&](/*cmd*/) { printf("  >> exec: Lighting\n"); });

    fg.AddPass("Bloom",
        [&]() { fg.Read(3, hdr); fg.Write(3, bloom); },
        [&](/*cmd*/) { printf("  >> exec: Bloom\n"); });

    fg.AddPass("Tonemap",
        [&]() { fg.Read(4, bloom); fg.Write(4, hdr); },
        [&](/*cmd*/) { printf("  >> exec: Tonemap\n"); });

    // Present — reads HDR, writes to imported backbuffer.
    fg.AddPass("Present",
        [&]() { fg.Read(5, hdr); fg.Write(5, backbuffer); },
        [&](/*cmd*/) { printf("  >> exec: Present\n"); });

    // Dead pass — nothing reads debug, so the graph will cull it.
    fg.AddPass("DebugOverlay",
        [&]() { fg.Write(6, debug); },
        [&](/*cmd*/) { printf("  >> exec: DebugOverlay\n"); });

    auto plan = fg.Compile();   // topo-sort, cull, alias
    fg.Execute(plan);             // barriers + run
    return 0;
}
