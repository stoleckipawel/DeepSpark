// Frame Graph v2 — SSAO Connected (output IS read → pass stays alive)
// Compile: g++ -std=c++17 -o example_v2_ssao_alive example_v2_ssao_alive.cpp frame_graph_v2.cpp
#include "frame_graph_v2.h"
#include <cstdio>

int main() {
    printf("=== v2 Variant A: SSAO Connected ===\n");

    FrameGraph fg;
    auto depth = fg.CreateResource({1920, 1080, Format::D32F});
    auto gbufA = fg.CreateResource({1920, 1080, Format::RGBA8});
    auto ssao  = fg.CreateResource({1920, 1080, Format::R8});
    auto hdr   = fg.CreateResource({1920, 1080, Format::RGBA16F});

    fg.AddPass("DepthPrepass",
        [&]() { fg.Write(0, depth); },
        [&](/*cmd*/) { printf("  >> exec: DepthPrepass\n"); });

    fg.AddPass("GBuffer",
        [&]() { fg.Read(1, depth); fg.Write(1, gbufA); },
        [&](/*cmd*/) { printf("  >> exec: GBuffer\n"); });

    fg.AddPass("SSAO",
        [&]() { fg.Read(2, depth); fg.Write(2, ssao); },
        [&](/*cmd*/) { printf("  >> exec: SSAO\n"); });

    fg.AddPass("Lighting",
        [&]() { fg.Read(3, gbufA); fg.Read(3, ssao); fg.Write(3, hdr); },
        [&](/*cmd*/) { printf("  >> exec: Lighting\n"); });

    printf("Lighting reads SSAO -> SSAO stays alive.\n");
    fg.Execute();
    return 0;
}
