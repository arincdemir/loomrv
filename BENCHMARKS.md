# Reproducing the loomrv Benchmarks

This document describes how to reproduce the benchmark results using Docker.
No local C++ toolchain, Boost, or data downloads are required — everything is
self-contained inside the image.

## Prerequisites

- [Docker](https://docs.docker.com/get-docker/) ≥ 20.10
- ~4 GB free disk space (image includes ~1 GB of uncompressed test data)
- Internet access during `docker build` (fetches source deps and test data)

---

## 1 — Build the image

Run the following from the repository root.  Pass in the current commit hash
so result filenames reflect the exact revision being benchmarked:

```bash
docker build \
  --build-arg GIT_COMMIT=$(git rev-parse HEAD) \
  -t loomrv-bench \
  .
```

What happens during the build:

| Stage | What it does |
|-------|-------------|
| `reelay-builder` | Clones `doganulus/reelay` (branch `main`) and builds `rybinx` + `ryjson` with `-DREELAY_BUILD_APPS=ON`, mirroring the `apps:` Makefile target |
| `loomrv-builder` | Builds `loomrv`, `count-nodes`, and `check-grammar` from source (C++20, Release) |
| `runtime` | Assembles a clean Ubuntu 24.04 image with all binaries, `hyperfine`, Python 3, and the 274 MB test dataset downloaded from the GitHub release |

The final image is ~1.5–2 GB.

---

## 2 — Run the benchmarks

Create a local `results/` directory first so Docker can mount it:

```bash
mkdir -p results
```

### All benchmarks (dense + discrete)

```bash
docker run --rm \
  -v "$(pwd)/results:/app/loomrv-misc/results" \
  loomrv-bench all
```

Runs all 32 scripts (dense + discrete).

### Dense benchmarks only  (`run_all_dense_benchmarks.sh`)

```bash
docker run --rm \
  -v "$(pwd)/results:/app/loomrv-misc/results" \
  loomrv-bench dense
```

Runs 16 scripts covering dense (JSON + binary), single-property, multi-property,
and synthetic trace scenarios.

### Discrete benchmarks only  (`run_all_discrete_benchmarks.sh`)

```bash
docker run --rm \
  -v "$(pwd)/results:/app/loomrv-misc/results" \
  loomrv-bench discrete
```

Runs 16 scripts covering the discrete-time variants of all the above.

Results are written as JSON files to `results/<timestamp>/` on the host.

---

## 3 — Quick sanity check (1 warmup, 1 run)

The full suites use `hyperfine --runs 25 --warmup 2` (or `--warmup 3`), which
takes several hours.  To verify the setup quickly, override both counters:

```bash
docker run --rm \
  -e HYPERFINE_RUNS=1 \
  -e HYPERFINE_WARMUP=1 \
  -v "$(pwd)/results:/app/loomrv-misc/results" \
  loomrv-bench dense
```

This replaces every `--runs` and `--warmup` argument passed to `hyperfine`
with `1`, without modifying any benchmark script.  Set them independently
if you only want to reduce one (e.g. `-e HYPERFINE_RUNS=3` keeps the
default warmup count).

---

## 4 — Interactive exploration

```bash
docker run --rm -it loomrv-bench bash
```

Inside the container, the working directory is `/app/loomrv-misc/`.
Relevant paths:

| Path | Contents |
|------|----------|
| `/app/build/loomrv` | Main loomrv binary |
| `/app/build/count-nodes` | Formula node-count utility |
| `/app/build/check-grammar` | Formula grammar checker |
| `/usr/local/bin/rybinx` | reelay binary-format monitor |
| `/usr/local/bin/ryjson` | reelay JSON-format monitor |
| `/app/data/fullsuite/` | Pre-generated test data (10 pattern families) |
| `/app/loomrv-misc/tools/` | Python helper scripts |

---

## 5 — Result format

Each benchmark script writes one JSON file:

```
results/<timestamp>/<script-name>.<commit-hash>.results.json
```

The JSON is in [hyperfine's export format](https://github.com/sharkdp/hyperfine#export-results),
containing per-command timing statistics (mean, median, standard deviation, min,
max, and individual run times).

---

## Notes

- **Reproducibility**: The test data is pinned to the
  [`timescales-data-v1`](https://github.com/arincdemir/loomrv/releases/tag/timescales-data-v1)
  release. The reelay competitor tools are built from the `main` branch at the
  time the image is built; use `--build-arg REELAY_BRANCH=<tag>` to pin a
  specific reelay release.
- **CPU performance**: For publication-quality results, disable CPU frequency
  scaling on the host (see `loomrv-misc/benchmark_settings.txt`).  The Docker
  container uses the host CPU directly, so host-level tuning applies.
- **Platform**: The image is built for the host architecture (`linux/amd64` or
  `linux/arm64`).  Cross-platform builds are possible with `--platform` but
  timing results will not be comparable across architectures.
