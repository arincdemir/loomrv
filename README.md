# LoomRV — Multi-Property Temporal Logic Monitoring

LoomRV is a high-performance runtime verification framework for the simultaneous
evaluation of multiple past-time Metric Temporal Logic (Past-MTL) properties.
It compiles property specifications into a structurally deduplicated, linearized
execution schedule and employs a zero-allocation double-buffered arena for state
management.  This is the artifact accompanying the paper
*"Multi-Property Temporal Logic Monitoring"*.

The object-oriented baseline implementation by @doganulus can be seen at
[reelay](https://github.com/doganulus/reelay).

---

## Artifact Structure

```
loomrv/
├── LICENSE                         MPL 2.0
├── README.md                       This file
├── Dockerfile                      Multi-stage build for the benchmarking container
├── docker-entrypoint.sh            Container entrypoint (dispatches benchmark suites)
├── BENCHMARKS.md                   Condensed benchmark running instructions
│
├── src/                            LoomRV C++ source code
├── include/                        LoomRV C++ headers
├── benchmarks/                     Benchmark harness (CMake sub-project)
├── tests/                          Unit tests
├── CMakeLists.txt                  Top-level CMake build
│
├── loomrv-misc/                    Benchmark scripts and helper tools
│   ├── run_all_dense_benchmarks.sh     Runs all 16 dense-time scripts
│   ├── run_all_discrete_benchmarks.sh  Runs all 16 discrete-time scripts
│   ├── benchmark_synthetic_*.sh        CSE sensitivity benchmarks
│   ├── loomrv-*.sh / ryjson-*.sh / rybinx-*.sh   Per-experiment scripts
│   └── tools/
│       ├── generate_tables.py          Reproduce paper tables from result JSONs
│       ├── extract_min_times.py        Extract min times from hyperfine output
│       ├── generate_test_formulas.py   Generate synthetic formula sets
│       └── generate_traces.py          Generate synthetic trace data
│
├── results/                        Pre-computed benchmark results
│   ├── 2026-05-01_23-08-06/        Dense-time results (hyperfine JSON)
│   └── 2026-05-02_00-58-23/        Discrete-time results (hyperfine JSON)
│
└── multi-property-temporal-logic-monitoring-latex/
    └── main.tex                    Paper source
```

---

## Resource Requirements

| Resource | Requirement |
|----------|-------------|
| **Disk space** | ~12 GB (Docker image + test data + results) |
| **RAM** | 4 GB minimum |
| **CPU** | Any architecture supported by Docker; 1 core is sufficient (benchmarks are single-threaded) |
| **OS** | Any OS with Docker support (Linux, macOS, Windows via WSL2) |
| **Network** | Required during `docker build` (fetches source dependencies and test data) |

---

## Prerequisites

### Installing Docker

If Docker is not already installed, follow the official instructions for your
platform:

- **Ubuntu / Debian**: <https://docs.docker.com/engine/install/ubuntu/>
- **macOS**: <https://docs.docker.com/desktop/install/mac-install/>
- **Windows (WSL2)**: <https://docs.docker.com/desktop/install/windows-install/>

Verify your installation:

```bash
docker --version   # Should print Docker version 20.10 or newer
```

---

## 1 — Build the Image

> [!IMPORTANT]
> **Run all commands from the artfiact root** (`loomrv/`).

```bash
docker build -t loomrv-bench .
```

What happens during the build:

| Stage | What it does |
|-------|-------------|
| `reelay-builder` | Clones `doganulus/reelay` at commit `2aae575` and builds `rybinx` + `ryjson` |
| `loomrv-builder` | Builds `loomrv`, `count-nodes`, and `check-grammar` from source (C++20, Release) |
| `runtime` | Assembles a clean Ubuntu 24.04 image with all binaries, `hyperfine`, Python 3, and the test dataset downloaded from the GitHub release |

---

## 2 — Run the Benchmarks

Create a local `results/` directory so Docker can write results to the host:

```bash
mkdir -p results
```


### Dense benchmarks

```bash
docker run --rm \
  -v "$(pwd)/results:/app/loomrv-misc/results" \
  loomrv-bench dense
```

### Discrete benchmarks

```bash
docker run --rm \
  -v "$(pwd)/results:/app/loomrv-misc/results" \
  loomrv-bench discrete
```

Results are written as JSON files to `results/<timestamp>/` on the host.

---

## 3 — Time Estimates

The following estimates are based on an Intel Core i7-10750H. Actual times may
vary depending on your hardware.

| Mode | Command | Estimated time |
|------|---------|---------------|
| **Quick sanity check** | See §4 below | ~10–15 minutes |
| **Dense benchmarks** | `loomrv-bench dense` | ~40 minutes |
| **Discrete benchmarks** | `loomrv-bench discrete` | ~30 minutes |
| **Full reproduction** | `loomrv-bench all` | ~1.5 hours |

---

## 4 — Quick Sanity Check

The full suites use `hyperfine --runs 25 --warmup 2` (or `--warmup 3`), which
takes about 1.5 hours.  To verify the setup quickly, override both counters:

```bash
docker run --rm \
  -e HYPERFINE_RUNS=1 \
  -e HYPERFINE_WARMUP=1 \
  -v "$(pwd)/results:/app/loomrv-misc/results" \
  loomrv-bench dense
```

This replaces every `--runs` and `--warmup` argument passed to `hyperfine`
with `1`, without modifying any benchmark script.

---

## 5 — Quick Example

To verify the tool works and see multi-property monitoring in action, run a
one-liner inside the container.  Three formulas are monitored simultaneously
over the same trace:

```bash
docker run --rm --entrypoint bash loomrv-bench -c '
  printf "{\"time\":1,\"p\":true,\"q\":false}\n{\"time\":2,\"p\":true,\"q\":false}\n{\"time\":3,\"p\":false,\"q\":true}\n{\"time\":4,\"p\":true,\"q\":true}\n" > /tmp/trace.jsonl
  printf "historically({p})\nonce({q})\n{p} since {q}\n" > /tmp/props.txt
  /app/build/loomrv --discrete --print /tmp/trace.jsonl /tmp/props.txt'
```

**Expected output** (three comma-separated verdicts per timestep, one per
formula):

```
2:true,false,false
3:false,true,true
4:false,true,true
```

The first event (t=1) initialises the monitor state; verdicts are produced from
t=2 onward.

| Formula | Meaning | t=2 | t=3 | t=4 |
|---------|---------|-----|-----|-----|
| `historically({p})` | Has `p` been true at every step so far? | **true** — `p` true at t=1,2 | **false** — `p` false at t=3 | **false** — still violated |
| `once({q})` | Has `q` ever been true? | **false** — `q` false so far | **true** — `q` true at t=3 | **true** — already witnessed |
| `{p} since {q}` | Has `p` held at every step since the last `q`? | **false** — `q` never held | **true** — `q` holds now | **true** — `q` at t=3, `p` since |

LoomRV evaluates all three properties in a single pass over the trace.

---

## 6 — Verifying the Paper's Results

The `results/` directory contains the raw hyperfine JSON logs from the
experiments reported in the paper.  To reproduce the paper's tables:

```bash
docker run --rm --entrypoint bash loomrv-bench -c \
  'python3 tools/generate_tables.py \
      --dense-dir    results/2026-05-01_23-08-06 \
      --discrete-dir results/2026-05-02_00-58-23'
```

This prints all benchmark tables (Tables 2–7 from the paper) with minimum
wall-clock times and computed speedup ratios.

### Mapping: Result Files → Paper Tables

| Paper Table | Description | Result Files |
|---|---|---|
| Table 2 | CSE sensitivity (discrete) | `bench_discrete_*.json`, `bench_binary_discrete_*.json` |
| Table 3 | CSE sensitivity (dense) | `bench_*.json`, `bench_binary_*.json` (in dense dir) |
| Table 4 | Single-property (discrete) | `loomrv-discrete-benchmark-json.sh.*`, `ryjson-discrete-benchmark.sh.*`, `loomrv-discrete-benchmark-bin.sh.*`, `rybinx-discrete-benchmark.sh.*` |
| Table 5 | Single-property dense (JSON) | `loomrv-benchmark-dense.sh.*`, `ryjson-benchmark-dense.sh.*` |
| Table 5b | Single-property dense (binary) | `loomrv-benchmark-dense-binary.sh.*`, `rybinx-benchmark-dense.sh.*` |
| Table 6 | Multi-property (discrete) | `*-discrete-benchmark-multi*.sh.*`, `*-discrete-benchmark-single*.sh.*` |
| Table 7 | Multi-property (dense) | `*-benchmark-dense-multi*.sh.*`, `*-benchmark-dense-single*.sh.*` |

---

## 7 — Interactive Exploration

```bash
docker run --rm -it --entrypoint bash loomrv-bench
```

Inside the container, the working directory is `/app/loomrv-misc/`.
Relevant paths:

| Path | Contents |
|------|----------|
| `/app/build/loomrv` | Main loomrv binary |
| `/app/build/count-nodes` | Formula node-count utility |
| `/app/build/check-grammar` | Formula grammar checker |
| `/usr/local/bin/rybinx` | Reelay binary-format monitor |
| `/usr/local/bin/ryjson` | Reelay JSON-format monitor |
| `/app/data/fullsuite/` | Pre-generated test data (10 pattern families) |
| `/app/loomrv-misc/tools/` | Python helper scripts |

---

## 8 — Result Format

Each benchmark script writes one JSON file:

```
results/<timestamp>/<script-name>.<commit-hash>.results.json
```

The JSON is in [hyperfine's export format](https://github.com/sharkdp/hyperfine#export-results),
containing per-command timing statistics (mean, median, standard deviation, min,
max, and individual run times).

---

## License

This project is licensed under the [Mozilla Public License 2.0](LICENSE).
