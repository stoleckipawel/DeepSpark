// Frame Graph — API demo (4 passes, imported backbuffer)
#include "frame_graph_v3.h"

int main()
{
	FrameGraph fg;

	// [1] Declare — describe resources and register passes

	// Import the swapchain backbuffer — externally owned, not aliased.
	auto backbuffer = fg.ImportResource({1920, 1080, Format::RGBA8}, ResourceState::Present);

	// Transient resources — graph-owned, eligible for aliasing.
	auto depth = fg.CreateResource({1920, 1080, Format::D32F});
	auto gbufA = fg.CreateResource({1920, 1080, Format::RGBA8});
	auto gbufN = fg.CreateResource({1920, 1080, Format::RGBA8});
	auto hdr = fg.CreateResource({1920, 1080, Format::RGBA16F});

	fg.AddPass(
	    "DepthPrepass",
	    [&]()
	    {
		    fg.Write(0, depth);
	    },
	    [&](/*cmd*/) { /* draw scene depth-only */ });

	fg.AddPass(
	    "GBuffer",
	    [&]()
	    {
		    fg.Read(1, depth);
		    fg.Write(1, gbufA);
		    fg.Write(1, gbufN);
	    },
	    [&](/*cmd*/) { /* draw scene to GBuffer MRTs */ });

	fg.AddPass(
	    "Lighting",
	    [&]()
	    {
		    fg.Read(2, gbufA);
		    fg.Read(2, gbufN);
		    fg.Write(2, hdr);
	    },
	    [&](/*cmd*/) { /* fullscreen lighting pass */ });

	fg.AddPass(
	    "Present",
	    [&]()
	    {
		    fg.Read(3, hdr);
		    fg.Write(3, backbuffer);
	    },
	    [&](/*cmd*/) { /* copy to backbuffer, present */ });

	// [2] Compile — topo-sort, cull, alias memory, compute barriers
	auto plan = fg.Compile();

	// [3] Execute — replay precomputed barriers + run each pass
	fg.Execute(plan);
}
