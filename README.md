# Deep Spark

Deep Spark is a graphics engineering blog by [Pawel Stolecki](https://stoleckipawel.dev/about/), a rendering engineer focused on real-time graphics and performance-critical systems.

The articles are about scalable rendering systems, GPU architecture, and low-level graphics programming under real frame-budget constraints.

## What you'll find here

- Articles on frame graphs, render scheduling, GPU barriers, memory aliasing, async compute, and production rendering architecture.
- A four-part frame graph series that starts with the core model, builds a working C++ version, then moves into queue scheduling and production-scale systems.
- Context from real rendering work across shipped games, engine work, and performance-focused graphics systems.

Current core series:

1. Theory: why frame graphs exist and how they handle scheduling, barriers, culling, and VRAM aliasing.
2. Build It: a C++ implementation built in three iterations.
3. Beyond MVP: async compute, queue scheduling, and split barriers.
4. Production Engines: how the same ideas scale toward systems such as UE5 RDG.

## Who it's for

This blog is for people working in or moving toward:

- graphics programming
- rendering engineering
- engine architecture
- GPU performance and optimization
- low-level real-time systems

## Read it

The blog is available online at [stoleckipawel.dev](https://stoleckipawel.dev).

## Notes

All written content belongs to Pawel Stolecki. Theme components remain under their respective upstream licensing.
