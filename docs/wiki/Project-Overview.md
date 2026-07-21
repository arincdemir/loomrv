# Project Overview

## Problem

Runtime verification checks a system execution against formal requirements as the execution occurs or after a trace has been recorded. Past-time temporal logic is well suited to online monitoring because each verdict depends only on the current and previous observations.

Existing tools commonly create an independent monitor for every property. This approach is simple, but it duplicates work when multiple properties:

- consume the same trace,
- reference the same propositions,
- contain equivalent subformulas,
- perform similar temporal-state updates.

The duplicated cost becomes significant as the property set grows.

## Project Goal

LoomRV investigates whether multiple Past-LTL and Past-MTL properties can be monitored more efficiently as one unified computation.

The framework must:

1. accept multiple properties while preserving one output per property,
2. identify and reuse equivalent subformulas,
3. evaluate both discrete-time and dense-time traces,
4. avoid dynamic allocation in the dense-time evaluation loop,
5. support practical trace formats and reproducible performance evaluation.

## Main Contributions

### Multi-property compilation

Every formula is parsed into nodes representing propositions, Boolean operators, and temporal operators. Nodes are stored in a content-addressable map. When an equivalent node already exists, the existing node is reused.

### Data-oriented execution

The shared graph is represented as an ordered node array. Dependencies always appear before their users, allowing sequential evaluation without recursive traversal or pointer chasing.

### Zero-allocation dense-time state

Dense-time nodes manipulate interval sets. LoomRV stores these sets in a preallocated, double-buffered arena. Each evaluation reads previous state from one buffer and writes current state to the other.

### Reproducible evaluation artifact

The repository includes Docker packaging, benchmark scripts, synthetic trace tools, and raw Hyperfine result files. These support comparison with the Reelay baseline.

## Supported Use Cases

- Monitoring several related temporal properties over one trace
- Discrete-time event streams with Boolean verdicts
- Dense-time event streams with interval-set verdicts
- Offline monitoring of NDJSON or the project-specific binary row format
- Embedding the monitor engine in C++ applications
- Measuring the effect of cross-property subformula sharing

## Current Limitations

- The command-line application processes trace files rather than live sockets or message queues.
- The binary feeder uses a project-specific packed row format with fixed proposition fields `p`, `q`, `r`, and `s`.
- Arena capacity is supplied when a monitor is created; callers embedding the library must select an adequate value.
- The grammar supports past-time operators only.
- Temporal nodes are shared only when their operator, children, and interval bounds match exactly. Overlapping but non-identical bounds are not currently shared.
- Formula constants such as `true` and `false` are present as grammar tokens but are not accepted as atoms.
- The project currently has no installation target or packaged public library release.

## Time Models

| Model | Input interpretation | Verdict |
|---|---|---|
| Discrete | Each row is one evaluation point | One Boolean per property and row |
| Dense | A row's proposition values hold until the next timestamp | One interval set per property and evaluated interval |

Dense-time feeding requires two rows before the first interval can be evaluated because the first row establishes the start of the interval.
