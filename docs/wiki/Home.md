# LoomRV

**LoomRV** is a high-performance runtime verification framework for monitoring multiple past-time Metric Temporal Logic (Past-MTL) properties over a shared execution trace.

Instead of constructing and running one independent monitor per property, LoomRV compiles all properties into a single deduplicated computation graph. Shared subformulas are evaluated once, and every property retains its own verdict.

## Documentation Version

**Documentation release:** 2026.07, validated against the `main` branch on July 21, 2026.

A snapshot is stored under `docs/wiki` in the source repository so that releases and archival artifacts retain the documentation applicable to their exact source revision. The GitHub wiki remains the living copy for ongoing updates.

## Why LoomRV?

Systems commonly need to satisfy many related temporal requirements at the same time. Independent monitors repeatedly:

- parse the same trace,
- evaluate equivalent subformulas,
- allocate and update separate intermediate state.

LoomRV reduces this duplicated work using:

| Technique | Purpose |
|---|---|
| Shared directed acyclic graph | Reuses equivalent subformulas across properties |
| Linearized execution schedule | Evaluates nodes in dependency order without pointer-based traversal |
| Double-buffered arena | Stores dense-time interval state in contiguous preallocated memory |
| Multi-property output roots | Returns one verdict per original property |
| JSON and binary feeders | Supports readable and low-overhead trace ingestion |

LoomRV supports both **discrete-time** Boolean verdicts and **dense-time** interval-set verdicts.

## Quick Example

Create a trace:

```json
{"time":1,"p":true,"q":false}
{"time":2,"p":true,"q":false}
{"time":3,"p":false,"q":true}
{"time":4,"p":true,"q":true}
```

Create a property file containing one formula per line:

```text
historically({p})
once({q})
{p} since {q}
```

The repository stores these inputs under `examples/`. Run all three properties
in one discrete-time monitor:

```bash
./build/loomrv --discrete --print \
  examples/trace.jsonl examples/properties.txt
```

Output:

```text
2:true,false,false
3:false,true,true
4:false,true,true
```

Each output row contains the timestamp followed by verdicts in property-file order. In the current CLI JSON workflow, the first trace row initializes feeder state and printed verdicts begin at the second row.

## Results At A Glance

Using the bundled benchmark results, LoomRV achieved:

- approximately **2x to 4.5x** per-property throughput improvement over Reelay,
- approximately **6x to 12x** end-to-end speedup in multi-property configurations,
- a **7.3x** dense-time JSON speedup and **11.7x** discrete-time JSON speedup when monitoring all 30 benchmark properties together.

See [Benchmarks and Results](Benchmarks-and-Results.md) for the methodology and reproducible commands.

## Navigate The Wiki

- [Project Overview](Project-Overview.md)
- [Architecture and Design](Architecture-and-Design.md)
- [Temporal Logic Syntax](Temporal-Logic-Syntax.md)
- [Getting Started](Getting-Started.md)
- [Input and Output Formats](Input-and-Output-Formats.md)
- [Library API](Library-API.md)
- [Testing and Validation](Testing-and-Validation.md)
- [Benchmarks and Results](Benchmarks-and-Results.md)
- [Project Structure and Utilities](Project-Structure-and-Utilities.md)
- [References and Credits](References-and-Credits.md)

## Repository

Source code, tests, benchmark scripts, and bundled results are available in the [LoomRV repository](https://github.com/arincdemir/loomrv).
