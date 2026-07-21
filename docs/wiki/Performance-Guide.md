# Performance Guide

LoomRV is designed for high-throughput monitoring. The execution engine evaluates a finalized node schedule without adding diagnostic work to the per-event path. Use the standalone inspection and benchmark tools when you need visibility into a property set.

## Build For Measurement

Use a Release build for throughput measurements:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Debug and sanitizer builds are useful for validation, but their timings are not comparable with the bundled results.

## Keep Output Outside The Timed Path

The CLI evaluates a trace without producing verdict text unless `--print` is supplied. Leave printing disabled for throughput measurements. Formatting dense-time interval sets and writing output can otherwise dominate a short run.

Applications embedding `loomrv_lib` should build and finalize a monitor once, then reuse it for the full stream. Property parsing and graph construction are setup operations rather than part of normal event evaluation.

## Choose The Input Path Deliberately

- **NDJSON** is readable, accepts named proposition fields, and is the general CLI interchange format. Its parsing cost is part of end-to-end measurements.
- **Binary rows** reduce ingestion overhead in the supplied experiments, but the format is benchmark-specific, uses fixed proposition fields, and is not a portable interchange format.
- Direct library evaluation avoids file decoding when an application already has proposition valuations in memory.

Comparisons should use the same input representation for both systems. A speedup measured against repeated JSON parsing is not solely a measure of graph evaluation.

## Inspect Structural Sharing

Use `count-nodes` before running a large property set:

```bash
./build/count-nodes properties.txt
./build/count-nodes --json properties.txt
```

The text report is intended for interactive use. The JSON report is stable input for scripts and contains:

- the number of properties and property roots,
- the sum of nodes in independent monitors,
- the number of nodes in the shared DAG,
- the eliminated-node count and compression ratio,
- shared-DAG counts for propositions and each Boolean or temporal node type.

For the bundled three-property example, the independent monitors contain seven nodes and the shared graph contains five:

```json
{
  "schema_version": 1,
  "property_count": 3,
  "independent_node_count": 7,
  "shared_dag_node_count": 5,
  "eliminated_node_count": 2,
  "compression_ratio": 1.4,
  "property_root_count": 3,
  "node_type_counts": {
    "proposition": 2,
    "and": 0,
    "or": 0,
    "not": 0,
    "implies": 0,
    "once": 1,
    "historically": 1,
    "since": 1,
    "test": 0
  }
}
```

## Interpret Compression Carefully

The compression ratio describes graph structure, not a guaranteed runtime speedup. Observed performance also depends on node types, trace density, ingestion cost, output cost, and the amount of interval-set work performed by dense-time nodes.

Temporal nodes are merged only when their operator, operands, and bounds are identical. Related formulas with different bounds therefore appear as distinct temporal nodes in the report.

For the supplied measurement methodology and raw results, see [Benchmarks and Results](Benchmarks-and-Results.md).
