# Library API

LoomRV can be embedded as a C++20 static library through the `loomrv_lib` CMake target.

## Core Headers

```cpp
#include <loomrv/MTLEngine.hpp>
#include <loomrv/ptl.hpp>
#include <loomrv/json_feeder.hpp>
#include <loomrv/binary_feeder.hpp>
#include <loomrv/interval_set.hpp>
```

## Discrete-Time Example

```cpp
#include <loomrv/MTLEngine.hpp>
#include <loomrv/ptl.hpp>

using namespace loomrv;

int main() {
    ptl_parser parser;
    auto monitor = createDiscreteMultiPropertyMonitor(1000);

    parser.parse_discrete("historically({p})", monitor);
    parser.parse_discrete("once({q})", monitor);
    finalize_monitor(monitor, {"p", "q"});

    const auto& verdicts = eval_multi_property(
        monitor, 1, std::vector<bool>{true, false});

    return verdicts[0] ? 0 : 1;
}
```

## Dense-Time Example

```cpp
ptl_parser parser;
auto monitor = createDenseMultiPropertyMonitor(3000);

parser.parse_dense("once[:10]({p})", monitor);
finalize_monitor(monitor, {"p"});

const auto& verdicts = eval_multi_property(
    monitor, 0, 5, std::vector<bool>{true});
```

Dense-time results are `IntervalSet` values. Convert a result for inspection with:

```cpp
auto intervals = db_interval_set::toVectorIntervals(verdicts[0]);
```

## Monitor Lifecycle

1. Create a dense or discrete monitor with an arena holder capacity.
2. Parse every property into the same monitor.
3. Finalize proposition ordering.
4. Evaluate inputs repeatedly.
5. Let the monitor destructor release arena resources.

Monitors are movable but not copyable.

## Formula Parsing

```cpp
ptl_parser parser;
parser.parse_discrete(formula, discrete_monitor);
parser.parse_dense(formula, dense_monitor);
```

Parsing multiple formulas into one monitor enables cross-property node sharing. Every successful parse records one property root.

## Finalization

Use explicit positional ordering when inputs will be supplied as `std::vector<bool>`:

```cpp
finalize_monitor(monitor, {"p", "q"});
```

Use automatic ordering when inputs are named:

```cpp
finalize_monitor(monitor);
```

For binary feeders, do not finalize manually. They finalize with the fixed order `p`, `q`, `r`, `s`.

## Direct Evaluation

Discrete positional input:

```cpp
const std::vector<bool>& eval_multi_property(
    DiscreteMultiPropertyMonitor&, int time, const std::vector<bool>&);
```

Discrete named input:

```cpp
const std::vector<bool>& eval_multi_property(
    DiscreteMultiPropertyMonitor&, int time,
    const std::vector<std::pair<std::string_view, bool>>&);
```

Dense positional input:

```cpp
const std::vector<IntervalSet>& eval_multi_property(
    DenseMultiPropertyMonitor&, int start, int end,
    const std::vector<bool>&);
```

Dense named input uses `loomrv::TimescalesInput`.

Named inputs are sparse updates. When a known proposition is omitted, the monitor retains its most recently supplied value; before the first update its value is `false`. Names that are not referenced by the compiled properties are ignored. Positional `std::vector<bool>` inputs are complete valuations and must contain every proposition in the order selected during finalization.

## Feeders

JSON feeders stream NDJSON and require a finalized monitor:

```cpp
auto* feeder = create_discrete_json_feeder(monitor, "trace.jsonl");
while (const auto* result = feed_next(feeder)) {
    // Use result before the next feed_next call.
}
destroy_feeder(feeder);
```

Binary feeders load binary rows and finalize the monitor automatically.

## Holder Capacity

The integer supplied to `createDenseMultiPropertyMonitor` and `createDiscreteMultiPropertyMonitor` determines the size of each interval arena buffer.

Choose a capacity large enough for the compiled formulas and expected interval fragmentation. The CLI currently uses `3000`. Tests and focused examples often use smaller values.

## Dependencies

The library links publicly against:

- `cpp-peglib` for parsing,
- `simdjson` for NDJSON ingestion.
