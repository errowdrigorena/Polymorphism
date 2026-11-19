# Documents

The root [README](../README.md) is how to build, test, and run this repository. A document here is for one mechanism that a summary cannot state accurately.

Each document follows the code that implements the mechanism: the predicates, the data layout, and the call. Build commands stay in the root README.

| Document | What it covers |
| --- | --- |
| [Reflection](reflection.md) | How `reflection_type_erasure` turns a model class into `Poly<Model>` |
| [Benchmarks](benchmarks.md) | Why the timings do not cover type erasure, after the machine and the flags |

A new document belongs here when a later change introduces another mechanism with the same kind of detail. A paragraph in the root README can point at it.
