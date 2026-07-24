# FAQ and Design Rationale

## Why Not Conjoin Every Property?

A conjunction is sufficient when an application needs only one aggregate verdict. LoomRV's multi-property interface instead preserves one verdict per input property and records one root index for each property.

Exposing every conjunct from a conjunction monitor requires retaining those internal values, which approaches a multi-output computation graph. A conjunction also adds internal conjunction nodes that are unnecessary when the property roots can be returned directly. The artifact includes both conjunction and multi-root configurations so this distinction is visible in the existing measurements.

## Why Use A DAG Instead Of Constructing One Automaton?

An automaton-based monitor is possible, but it uses a different construction and runtime representation. LoomRV follows the compositional sequential-network approach: each subformula becomes a node, shared subformulas become one node, and the finalized DAG is evaluated in topological order.

This design keeps the original property roots visible, makes sharing explicit, and maps directly to a compact execution schedule. It avoids requiring a product automaton merely to combine properties. This is an implementation rationale for LoomRV, not a claim that DAG monitors dominate every automata-based technique or workload.

## Are Temporal Operators With Overlapping Bounds Shared?

No. A temporal node's structural identity includes the operator, child identifiers, and interval bounds. Identical temporal subformulas are shared only when their bounds also match. For example, `once[0:10]({p})` and `once[3:7]({p})` remain distinct nodes.

Reusing partially overlapping ranges would require different semantic state and update rules; it is not a hash-key relaxation in the current implementation. The `count-nodes` report reflects only sharing that LoomRV actually performs.

## How Representative Are The Synthetic Workloads?

The synthetic suites are controlled experiments. They intentionally vary structural overlap from high-sharing to low-sharing cases so the effect of shared computation can be observed. They do not establish how much overlap every industrial requirement set contains.

The 30-property Timescales suite exercises a broader collection of temporal patterns, but it is still a benchmark corpus rather than a survey of deployed specifications. Use `count-nodes --json` on the intended property set to characterize its exact sharing before extrapolating the published speedups.

## Does Node Compression Equal Runtime Speedup?

No. Compression measures how many computation nodes are removed. Runtime also includes trace ingestion, node-specific operation costs, initialization, and optional verdict formatting. Discrete Boolean nodes and dense-time interval operations have different costs, so two property sets with the same compression ratio can have different throughput.

## Why Is Reelay The Main Baseline?

Reelay is the closest like-for-like baseline because it shares the sequential-network lineage and supports the relevant online temporal-logic setting. Other tools use different logics, compilation pipelines, output semantics, or deployment assumptions. See [Tool Compatibility and Baseline Choice](Tool-Compatibility.md) for the source-backed comparison.

## Does LoomRV Add Diagnostics To Evaluation?

No. Version reporting and `count-nodes` are setup or standalone utility operations. They do not add branches, counters, logging, or allocation to the per-event evaluator.

See [Performance Guide](Performance-Guide.md) for practical measurement guidance and [Architecture and Design](Architecture-and-Design.md) for the compilation and execution pipeline.
