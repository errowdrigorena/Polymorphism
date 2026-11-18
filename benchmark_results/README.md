# Benchmark results

Measured on `d2288f6610bf` at 2026-11-18T22:10:07+00:00 with the `gcc16-benchmarks` preset. CPU frequency scaling was disabled.

Each bar is the CPU time of the named operation. A cycle bar includes construction, the calls, and destruction. One pass walks 12288 objects.

## Cycle

Construct, draw, compute the area, and destroy.

![Cycle](cycle.svg)

The chart below leaves out direct. 100% is the fastest of the four, and the others are that time scaled up.

![Cycle, relative](cycle_relative.svg)

## Cycle, 5 calls

Construct, alternate draw() and area() five times (three draws and two areas), and destroy.

![Cycle, 5 calls](cycle_5.svg)

The chart below leaves out direct. 100% is the fastest of the four, and the others are that time scaled up.

![Cycle, 5 calls, relative](cycle_5_relative.svg)

## Cycle, 50 calls

Construct, alternate draw() and area() fifty times, and destroy.

![Cycle, 50 calls](cycle_50.svg)

The chart below leaves out direct. 100% is the fastest of the four, and the others are that time scaled up.

![Cycle, 50 calls, relative](cycle_50_relative.svg)

## Cycle, 500 calls

Construct, alternate draw() and area() five hundred times, and destroy.

![Cycle, 500 calls](cycle_500.svg)

The chart below leaves out direct. 100% is the fastest of the four, and the others are that time scaled up.

![Cycle, 500 calls, relative](cycle_500_relative.svg)

## Cycle, 5000 calls

Construct, alternate draw() and area() five thousand times, and destroy.

![Cycle, 5000 calls](cycle_5000.svg)

The chart below leaves out direct. 100% is the fastest of the four, and the others are that time scaled up.

![Cycle, 5000 calls, relative](cycle_5000_relative.svg)

## Draw

One draw() call. The string fits in the small buffer.

![Draw](draw.svg)

The chart below leaves out direct. 100% is the fastest of the four, and the others are that time scaled up.

![Draw, relative](draw_relative.svg)

## Area

One area() call. traditional_polymorphism, type_erasure, and std::polymorphic use a local base because the shapes hierarchy has no numeric method.

![Area](area.svg)

The chart below leaves out direct. 100% is the fastest of the four, and the others are that time scaled up.

![Area, relative](area_relative.svg)

## Copy

Copy one owning object. traditional_polymorphism clones through a virtual function because unique_ptr does not copy the derived object.

![Copy](copy.svg)

The chart below leaves out direct. 100% is the fastest of the four, and the others are that time scaled up.

![Copy, relative](copy_relative.svg)

## Build

Construct one object. direct stores it in the array; the others allocate on the heap.

![Build](build.svg)

The chart below leaves out direct. 100% is the fastest of the four, and the others are that time scaled up.

![Build, relative](build_relative.svg)
