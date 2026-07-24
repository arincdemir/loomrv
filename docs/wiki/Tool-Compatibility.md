# Tool Compatibility and Baseline Choice

Runtime-verification tools differ in specification language, time semantics, verdict representation, input model, and deployment target. These differences matter when deciding whether a timing comparison is direct.

## Compatibility Matrix

| Tool | Primary specification and interface | Execution or output model | Relationship to LoomRV |
|---|---|---|---|
| [Reelay](https://github.com/doganulus/reelay) | Online LTL, MTL, and related temporal-logic monitoring with discrete- and dense-time support | C++/Python computation-graph monitors over event streams | The direct baseline: LoomRV extends the same sequential-network lineage, accepts the Reelay expression format, and can be evaluated with aligned trace semantics and feeders. The artifact pins the compared revision. |
| [MonPoly](https://bitbucket.org/monpoly/monpoly/) | Metric First-Order Temporal Logic policies over timestamped logs | Log monitor with first-order data variables and policy-oriented output | Not a drop-in baseline for propositional Past-LTL/Past-MTL throughput because its language, data model, and target workloads include first-order policy monitoring. |
| [Aerial](https://traytel.bitbucket.io/papers/rvcubes17-aerial_tool/index.html) | Online MTL and the more expressive Metric Dynamic Logic over timed event streams | Uses an event-rate-independent approach with a different verdict representation | Relevant metric-monitoring work, but not directly interchangeable with LoomRV's past-time, per-property Boolean or interval-set interface. |
| [Copilot](https://github.com/Copilot-Language/copilot) | A Haskell stream DSL with temporal-logic libraries | Generates hard-real-time C monitors and connects properties to trigger handlers | Targets compiled embedded stream programs rather than LoomRV's file/library interface and shared collection of Past-MTL formulas. |
| [R2U2](https://r2u2.github.io/r2u2/index.html) | Mission-time LTL specifications compiled through the C2PO toolchain | Stream-based observers designed for constrained safety- and mission-critical systems | Its language, compiler pipeline, resource model, and verdict timing differ from LoomRV's online Past-MTL evaluation model. |

## Why Reelay Is The Executable Baseline

Reelay is used because it is the closest controlled comparison, not because the other tools are unimportant. It shares the underlying compositional monitoring approach and supports the relevant online discrete- and dense-time setting. This lets the artifact compare execution organization while avoiding a translation between unrelated languages or verdict semantics.

The benchmark artifact therefore builds a pinned Reelay revision and does not include adapters for the other tools in the matrix. Adding an adapter would require a separately validated semantic translation, compatible trace encoding, equivalent output policy, and tool-specific tuning before its timings could support a fair claim.

## What The Matrix Does Not Claim

The matrix is a scope guide, not a ranking. It does not claim that LoomRV is faster than tools that were not executed, or that one monitoring formalism is generally preferable. Each adjacent tool addresses workloads or deployment constraints that may be more appropriate than LoomRV for a particular application.

For the comparisons that are included, see [Benchmarks and Results](Benchmarks-and-Results.md). For architectural rationale, see [FAQ and Design Rationale](FAQ-and-Design-Rationale.md).
