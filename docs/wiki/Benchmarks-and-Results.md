# Benchmarks and Results

The repository contains scripts, raw Hyperfine JSON output, and a table generator for reproducing the project's performance evaluation.

## Compared Systems

- **Reelay Sequential:** one Reelay monitor per property
- **Reelay-AND:** properties combined into one conjunction
- **LoomRV Sequential:** one LoomRV monitor per property
- **LoomRV-AND:** properties combined into one LoomRV conjunction
- **LoomRV Multi:** one shared LoomRV graph with one root per property

LoomRV and Reelay are evaluated with both JSON and binary feeders.

## Bundled Result Sets

| Directory | Contents |
|---|---|
| `results/2026-05-01_23-08-06` | Dense-time results |
| `results/2026-05-02_00-58-23` | Discrete-time results |

Values reported below are minimum wall-clock times from Hyperfine.

## Multi-Property Summary

Thirty properties are monitored together.

### Discrete Time

| Configuration | JSON time | JSON speedup | Binary time | Binary speedup |
|---|---:|---:|---:|---:|
| Reelay Sequential | 9.345 s | 1.0x | 4.657 s | 1.0x |
| LoomRV Sequential | 4.415 s | 2.1x | 1.577 s | 3.0x |
| LoomRV-AND | 0.804 s | 11.6x | 0.704 s | 6.6x |
| LoomRV Multi | **0.797 s** | **11.7x** | 0.708 s | 6.6x |

### Dense Time

| Configuration | JSON time | JSON speedup | Binary time | Binary speedup |
|---|---:|---:|---:|---:|
| Reelay Sequential | 27.491 s | 1.0x | 22.275 s | 1.0x |
| LoomRV Sequential | 9.628 s | 2.9x | 5.801 s | 3.8x |
| LoomRV-AND | 4.053 s | 6.8x | 3.722 s | 6.0x |
| LoomRV Multi | **3.773 s** | **7.3x** | **3.454 s** | **6.4x** |

## Cross-Property Sharing

Synthetic property sets vary the amount of reusable structure. At maximum sharing, the shared graph reaches approximately `4.09x` node compression.

For the nested best-case scenario:

| Time model | Feeder | Reelay Sequential | LoomRV Multi | Speedup |
|---|---|---:|---:|---:|
| Discrete | JSON | 3.316 s | 0.206 s | 16.1x |
| Discrete | Binary | 1.341 s | 0.127 s | 10.6x |
| Dense | JSON | 7.436 s | 0.604 s | 12.3x |
| Dense | Binary | 5.627 s | 0.490 s | 11.5x |

## Reproduce Tables From Bundled Results

No benchmark rerun is required:

```bash
python3 loomrv-misc/tools/generate_tables.py \
  --dense-dir results/2026-05-01_23-08-06 \
  --discrete-dir results/2026-05-02_00-58-23
```

## Run Benchmarks With Docker

Create a host results directory:

```bash
mkdir -p results
```

Run all suites:

```bash
docker run --rm \
  -v "$(pwd)/results:/app/loomrv-misc/results" \
  loomrv-bench all
```

Run only one time model by replacing `all` with `dense` or `discrete`.

### Quick Sanity Run

```bash
docker run --rm \
  -e HYPERFINE_RUNS=1 \
  -e HYPERFINE_WARMUP=1 \
  -v "$(pwd)/results:/app/loomrv-misc/results" \
  loomrv-bench dense
```

## Interpretation

The configurations separate several major effects:

1. LoomRV's linearized, arena-based execution improves even single-property monitoring.
2. The shared graph and single trace pass provide additional multi-property speedup.

Discrete monitoring benefits strongly from avoiding repeated parsing and initialization. Dense monitoring also benefits from structural sharing because each node performs more expensive interval-set operations.

These are end-to-end measurements rather than a complete component-level decomposition. In particular, the multi-property configurations also benefit from parsing and feeding the trace once, and the current experiments do not independently measure every contribution of parsing, scheduling, structural sharing, and arena management.
