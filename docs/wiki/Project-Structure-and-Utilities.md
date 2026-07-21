# Project Structure and Utilities

## Repository Layout

| Path | Purpose |
|---|---|
| `include/loomrv` | Public headers and parser grammar |
| `src` | Engine, feeders, readers, CLI, and utilities |
| `tests` | Catch2 tests |
| `examples` | Reusable CLI trace, properties, and expected output |
| `benchmarks` | C++ benchmark harnesses |
| `loomrv-misc` | Benchmark scripts and Python tools |
| `results` | Bundled raw Hyperfine results |
| `data/fullsuite` | Timescales benchmark traces |
| `docs/wiki` | Authoritative, versioned project documentation |
| `Dockerfile` | Reproducible benchmark image |

## Main Components

| Component | Relevant files |
|---|---|
| Formula parser and deduplication | `include/loomrv/ptl.hpp`, `include/loomrv/ptl_grammar.hpp` |
| Multi-property engine | `include/loomrv/MTLEngine.hpp`, `src/MTLEngine.cpp` |
| Interval arena | `include/loomrv/interval_set.hpp`, `src/interval_set.cpp` |
| JSON input | `include/loomrv/json_feeder.hpp`, `src/json_feeder.cpp` |
| Binary input | `include/loomrv/binary_feeder.hpp`, `src/binary_feeder.cpp` |
| CLI | `src/main.cpp` |

## Built Executables

| Executable | Purpose |
|---|---|
| `loomrv` | Main trace-monitoring CLI |
| `check-grammar` | Validate one property formula |
| `count-nodes` | Compare sequential and shared graph node counts |
| `verify-dedup` | Inspect intra-formula deduplication |
| `unit_tests` | Run the Catch2 test suite |
| `loomrv-perf` | Performance-oriented executable |

## Benchmark Tools

The `loomrv-misc` directory includes:

| Tool | Purpose |
|---|---|
| `run_all_dense_benchmarks.sh` | Run dense benchmark scripts |
| `run_all_discrete_benchmarks.sh` | Run discrete benchmark scripts |
| `benchmark_synthetic_*.sh` | Generate cross-property sharing measurements |
| `tools/generate_tables.py` | Convert raw result JSON into paper tables |
| `tools/generate_test_formulas.py` | Generate synthetic property sets |
| `tools/generate_traces.py` | Generate synthetic traces |
| `tools/to_binary_row.py` | Convert traces to the binary row format |
| `tools/extract_min_times.py` | Extract minimum timings |

Documentation is authored in `docs/wiki`. The `docs/wiki_tool.py` command validates the manual and synchronizes known pages to a local `loomrv.wiki` checkout while adapting internal link syntax.

## Dependencies

| Dependency | Use |
|---|---|
| C++20 | Core implementation |
| CMake | Build system and dependency fetching |
| cpp-peglib | Temporal-logic parser |
| simdjson | NDJSON ingestion |
| Catch2 | Tests |
| Hyperfine | Benchmark timing |
| Python 3 | Benchmark data processing |
| Docker | Reproducible artifact environment |

Dependencies fetched by CMake are pinned to specific versions in the top-level `CMakeLists.txt`.

## License

LoomRV is distributed under the Mozilla Public License 2.0. See the repository's [LICENSE](https://github.com/arincdemir/loomrv/blob/main/LICENSE).
