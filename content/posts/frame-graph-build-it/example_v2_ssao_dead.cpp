// Frame Graph v2 — SSAO Disconnected (output NOT read → pass gets culled)
// Compile: g++ -std=c++17 -o example_v2_ssao_dead example_v2_ssao_dead.cpp frame_graph_v2.cpp
#include "frame_graph_v2.h"
#include <cstdio>

int main() {
    printf("=== v2 Variant B: SSAO Disconnected ===\n");

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

    // Lighting does NOT read ssao — only reads gbufA
    fg.AddPass("Lighting",
        [&]() { fg.Read(3, gbufA); fg.Write(3, hdr); },
        [&](/*cmd*/) { printf("  >> exec: Lighting\n"); });

    printf("Lighting does NOT read SSAO -> SSAO gets culled.\n");
    fg.Execute();
    return 0;
}
