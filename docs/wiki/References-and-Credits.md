# References and Credits

## Project Team

- **Arınç Demir**: student researcher and primary LoomRV developer
- **Dogan Ulus**: project advisor and paper co-author

The project was developed at **Boğaziçi University**.

## Research Context

LoomRV is the artifact accompanying the work:

> *Multi-Property Temporal Logic Monitoring*

The project extends sequential-network-based temporal logic monitoring to a shared multi-property setting. It combines content-addressable subformula deduplication with a data-oriented runtime and zero-allocation interval-state management.

## Baseline

The primary comparison baseline is [Reelay](https://github.com/doganulus/reelay), an online monitoring library for metric temporal logic.

LoomRV's Docker image builds a pinned Reelay revision to keep benchmark comparisons reproducible.

The [Tool Compatibility and Baseline Choice](Tool-Compatibility.md) page relates Reelay to MonPoly, Aerial, Copilot, and R2U2 using links to each project's primary documentation.

## Supporting Tools

- [simdjson](https://github.com/simdjson/simdjson) for high-performance JSON parsing
- [cpp-peglib](https://github.com/yhirose/cpp-peglib) for parsing formula syntax
- [Catch2](https://github.com/catchorg/Catch2) for testing
- [Hyperfine](https://github.com/sharkdp/hyperfine) for benchmark timing

## Repository And License

- [LoomRV source repository](https://github.com/arincdemir/loomrv)
- [Mozilla Public License 2.0](https://github.com/arincdemir/loomrv/blob/main/LICENSE)
