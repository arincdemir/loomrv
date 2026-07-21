# Getting Started

The Docker workflow is the simplest way to reproduce the complete artifact. A native CMake build is more convenient for development.

## Get The Source

```bash
git clone https://github.com/arincdemir/loomrv.git
cd loomrv
```

The commands below are run from this repository root.

## Docker Workflow

### Requirements

- Docker 20.10 or newer
- Approximately 12 GB free disk space
- At least 4 GB RAM
- Network access while building the image

### Build

From the repository root:

```bash
docker build -t loomrv-bench .
```

The multi-stage image builds LoomRV and Reelay, installs benchmark tools, includes bundled result files, and downloads the test dataset.

### Run A Quick Example

```bash
docker run --rm --entrypoint /app/build/loomrv loomrv-bench \
  --discrete --print /app/examples/trace.jsonl /app/examples/properties.txt
```

Expected output:

```text
2:true,false,false
3:false,true,true
4:false,true,true
```

## Native Build

### Requirements

- CMake 3.15 or newer
- C++20 compiler
- Git and network access during initial configuration

CMake fetches `simdjson`, `cpp-peglib`, and Catch2.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The build also downloads the test dataset if `data/fullsuite` is absent.

Confirm the version compiled from the CMake project metadata:

```bash
./build/loomrv --version
# loomrv 1.0.0
```

Run the same quick example natively:

```bash
./build/loomrv --discrete --print \
  examples/trace.jsonl examples/properties.txt
```

Compare its output with `examples/expected-discrete.txt`.

### Run Tests

Run the registered Catch2 tests through CTest:

```bash
ctest --test-dir build --output-on-failure
```

The Catch2 executable remains available for filters and direct runs:

```bash
./build/tests/unit_tests
```

### Embed With CMake

Until LoomRV provides an installation target, a CMake project can include the repository as a subdirectory:

```cmake
add_subdirectory(external/loomrv)

add_executable(my_monitor main.cpp)
target_link_libraries(my_monitor PRIVATE loomrv_lib)
```

The `loomrv_lib` target exports the `include` directory and its required parser and JSON dependencies. Application code can then include headers such as:

```cpp
#include <loomrv/MTLEngine.hpp>
#include <loomrv/ptl.hpp>
```

## CLI Usage

```text
loomrv [OPTION...] TRACE_FILE PROPERTIES_FILE
```

| Option | Description |
|---|---|
| `-v`, `--dense` | Use dense time; this is the default |
| `-x`, `--discrete` | Use discrete time |
| `-b`, `--binary` | Read the project-specific `.row.bin` format instead of NDJSON |
| `-p`, `--print` | Print per-timestep or per-interval verdicts |
| `--version` | Print the CMake project version and exit |
| `-a N`, `--arena-capacity N` | Set each interval arena buffer's capacity; default `3000` |

Examples:

```bash
./build/loomrv --discrete --print trace.jsonl properties.txt
./build/loomrv --dense --print trace.jsonl properties.txt
./build/loomrv --discrete --binary trace.row.bin properties.txt
./build/loomrv --arena-capacity 6000 --dense trace.jsonl properties.txt
```

Without `--print`, LoomRV evaluates the entire trace without writing verdicts. This mode is useful for benchmarking.

Arena capacity must be a positive integer. Choose a value large enough for the
compiled properties and the interval fragmentation expected from the trace;
the CLI does not automatically resize the preallocated arena.

## Helper Utilities

```bash
./build/check-grammar "once[:10]({p})"
./build/count-nodes properties.txt
./build/count-nodes --json properties.txt
./build/verify-dedup properties.txt
```

- `check-grammar` validates one formula.
- `count-nodes` compares independent node counts with the shared multi-property graph; `--json` emits the same analysis as a stable machine-readable report.
- `verify-dedup` reports deduplication within each individual formula.

## Next Steps

- Review [Temporal Logic Syntax](Temporal-Logic-Syntax.md).
- Learn trace schemas in [Input and Output Formats](Input-and-Output-Formats.md).
- Embed the engine using the [Library API](Library-API.md).
- Apply the recommendations in the [Performance Guide](Performance-Guide.md).
- Review common questions in [FAQ and Design Rationale](FAQ-and-Design-Rationale.md).
- Reproduce measurements from [Benchmarks and Results](Benchmarks-and-Results.md).

## Troubleshooting

### Configuration cannot download dependencies or test data

The initial native build fetches C++ dependencies and, when absent, the Timescales test dataset. Confirm that GitHub is reachable, then rerun the CMake configuration or build. The Docker build likewise requires network access.

### A property fails to parse

Validate it separately to obtain a focused grammar error, then compare it with the supported syntax:

```bash
./build/check-grammar "historically({p} implies once[:10]({q}))"
```

See [Temporal Logic Syntax](Temporal-Logic-Syntax.md) for operator spelling, bounds, and precedence.

### The first CLI result appears later than expected

Dense evaluation needs two timestamps to define its first interval. In the current JSON CLI workflow, the first discrete row also initializes feeder state, so printed discrete verdicts begin with the second row. Direct discrete library calls return a verdict on every evaluation call.
