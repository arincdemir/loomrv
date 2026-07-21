# Architecture and Design

LoomRV separates property compilation from runtime trace evaluation.

## End-To-End Pipeline

| Stage | Responsibility |
|---|---|
| Parser | Converts Past-MTL text into typed computation nodes |
| Runtime builder | Deduplicates nodes, records property roots, and prepares the execution order |
| Input processor | Converts JSON, binary rows, or application values into proposition valuations |
| Execution engine | Evaluates every node in topological order |
| Output processor | Reads each property's root-node result |

## Shared Computation Graph

Each node is identified by:

- operator type,
- left operand index,
- right operand index,
- lower temporal bound,
- upper temporal bound.

This identity is stored as a `NodeKey`. When another formula creates the same key, LoomRV reuses the existing node. `AND` and `OR` operand indexes are normalized so commutative equivalents can also share nodes.

Temporal bounds are part of the key. Consequently, two otherwise identical temporal subformulas share a node only when their bounds are identical. LoomRV does not currently reuse computation between overlapping but non-identical ranges such as `[0:10]` and `[3:7]`.

The parser appends one root-node index for every property. The engine therefore returns multiple independent verdicts without adding conjunction nodes between properties.

### Example

For the properties:

```text
historically({p} implies once({q}))
once({q})
```

the proposition `{q}` and the subformula `once({q})` are represented once and used by both property roots.

## Linearized Execution

Nodes are appended after their dependencies are known. The resulting node vector is already in evaluation order.

At each timestep, the execution engine walks this vector once:

1. proposition nodes read current inputs,
2. Boolean nodes read previously evaluated child outputs,
3. temporal nodes combine child outputs with their saved state,
4. property-root outputs are collected in the original formula order.

This design avoids recursive evaluation and object-oriented graph traversal in the hot loop.

## Discrete-Time Runtime

Discrete-time nodes store:

- a Boolean output,
- any temporal state required by the operator,
- operand indexes and temporal bounds.

Every input row produces one Boolean verdict per property.

## Dense-Time Runtime

Dense-time nodes store and produce interval sets. A dense input row defines proposition values beginning at its timestamp. Those values are evaluated over the interval ending at the next row's timestamp.

### Double-Buffered Arena

The interval-set holder owns two preallocated buffers:

- the read buffer contains state from the previous evaluation,
- the write buffer receives current outputs and updated state.

Before an evaluation, the buffers swap roles and the write cursor resets. Nodes then read earlier current outputs and any required historical state while appending their new interval data.

This provides contiguous storage and avoids dynamic allocation during evaluation.

## Input Feeders

### JSON feeder

The JSON feeder uses `simdjson` to parse newline-delimited JSON. Proposition names are matched against the monitor's proposition map.

JSON rows may be sparse updates: a referenced proposition omitted from a row retains its most recently supplied value. Fields that are not referenced by any compiled property are ignored.

### Binary feeder

The binary feeder reads packed rows into memory and maps fields in fixed order:

```text
p = 0, q = 1, r = 2, s = 3
```

It automatically finalizes the monitor using that order.

## Important Implementation Types

| Type | Role |
|---|---|
| `ptl_parser` | Parses formulas and performs node deduplication |
| `ParsedNode` | Compilation-time node representation |
| `DiscreteNode` | Runtime node with Boolean output |
| `DenseNode` | Runtime node with interval-set state and output |
| `DiscreteMultiPropertyMonitor` | Owns discrete nodes, roots, proposition map, and output vector |
| `DenseMultiPropertyMonitor` | Owns dense nodes, roots, proposition map, arena, and outputs |
| `IntervalSetHolder` | Owns double-buffered interval storage |

See [Library API](Library-API.md) for embedding details.
