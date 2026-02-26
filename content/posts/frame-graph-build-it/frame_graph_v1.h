#pragma once
// Frame Graph MVP v1 -- Declare & Execute
// No dependency tracking, no barriers, no aliasing.
// Passes execute in declaration order.
//
// Compile: clang++ -std=c++17 -o example_v1 example_v1.cpp
//     or: g++ -std=c++17 -o example_v1 example_v1.cpp

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// == Resource description (virtual until compile) ==============
enum class Format
{
	RGBA8,
	RGBA16F,
	R8,
	D32F
};

struct ResourceDesc
{
	uint32_t width = 0;
	uint32_t height = 0;
	Format format = Format::RGBA8;
};

using ResourceIndex = uint32_t;

// Handle = typed index into the graph's resource array.
// No GPU memory behind it yet -- just a number.
struct ResourceHandle
{
	ResourceIndex index = UINT32_MAX;
	bool IsValid() const { return index != UINT32_MAX; }
};

// == Render pass ===============================================
struct RenderPass
{
	std::string name;
	std::function<void()> Setup;                // build the DAG (v1: unused)
	std::function<void(/*cmd list*/)> Execute;  // record GPU commands
};

// == Frame graph ===============================================
class FrameGraph
{
  public:
	// Create a virtual resource -- returns a handle, not GPU memory.
	ResourceHandle CreateResource(const ResourceDesc& desc);

	// Import an external resource (e.g. swapchain backbuffer).
	// Barriers are tracked, but the graph does not own its memory.
	ResourceHandle ImportResource(const ResourceDesc& desc);

	// Register a pass. Setup runs now; execute is stored for later.
	template <typename SetupFn, typename ExecFn> void AddPass(const std::string& name, SetupFn&& setup, ExecFn&& exec)
	{
		passes.push_back({name, std::forward<SetupFn>(setup), std::forward<ExecFn>(exec)});
		passes.back().Setup();  // run setup immediately
	}

	// Compile + execute. v1 is trivial -- just run in declaration order.
	void Execute();

  private:
	std::vector<RenderPass> passes;
	std::vector<ResourceDesc> resources;
};
