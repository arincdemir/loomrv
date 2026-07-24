# Input and Output Formats

## Property Files

Property files contain one formula per non-empty line:

```text
historically({p})
once({q})
{p} since {q}
```

The output vector preserves this line order.

## NDJSON Traces

Each line is one JSON object with:

- a non-negative integer `time` field,
- zero or more Boolean proposition updates.

```json
{"time":1,"p":true,"q":false}
{"time":2,"p":true,"q":false}
{"time":3,"p":false,"q":true}
```

Proposition names are matched by name. Fields not referenced by the compiled properties are ignored.

Rows use sparse-update semantics. A referenced proposition that is omitted from a row retains the last value supplied for that proposition. Before its first appearance, a proposition's value is `false`. This allows unchanged values to be omitted:

```json
{"time":1,"p":true,"q":false}
{"time":2,"q":true}
{"time":3,"p":false}
```

At time 2, `p` remains `true`; at time 3, `q` remains `true`. This behavior applies to named inputs in both discrete and dense time. Positional C++ inputs are complete valuations and must provide every proposition on every call.

### Discrete-Time Interpretation

Every line is evaluated independently at its timestamp and produces a Boolean verdict per property.

In the current command-line JSON workflow, the first row initializes the feeder and printed verdicts begin at the second row. Direct calls to the discrete library API produce a verdict for every evaluation call.

With `--discrete --print`, output rows use:

```text
TIME:VERDICT_1,VERDICT_2,...
```

Example:

```text
3:false,true,true
```

### Dense-Time Interpretation

The proposition values established by one row hold over the half-open interval ending at the next row's timestamp. Dense timestamps must be strictly increasing. Therefore, a trace with `n` rows produces at most `n - 1` evaluated intervals.

With `--dense --print`, each output row is JSON:

```json
{"time":[START,END],"verdicts":[[[A,B]],[]]}
```

Each property verdict is an array of intervals where that property holds inside the evaluated time range.

All dense-time intervals use the half-open convention `[START, END)`: the start is included and the end is excluded. The JSON encoding uses two-element arrays, so `[3,7]` in the serialized output represents the mathematical interval `[3,7)`.

## Binary Row Format

The binary feeder uses this packed C++ layout:

```cpp
#pragma pack(push, 1)
struct TimescalesInput {
    int32_t time;
    bool p;
    bool q;
    bool r;
    bool s;
};
#pragma pack(pop)
```

The fixed proposition order is:

```text
p, q, r, s
```

The binary feeder automatically finalizes the monitor using this order. Do not call `finalize_monitor` before creating a binary feeder.

This format is intended for project benchmarks. It is not a portable general-purpose interchange format because Boolean representation and endianness can vary between platforms.

## C++ Input Forms

Applications embedding LoomRV can evaluate:

- positional `std::vector<bool>` values after explicitly finalizing proposition order,
- named `std::vector<std::pair<std::string_view, bool>>` values,
- dense `TimescalesInput` records containing start time, end time, and named propositions.

Positional values are faster but require their order to match the finalized monitor mapping.

Named inputs use the same sparse-update semantics as NDJSON: omitted known propositions retain their previous values and unknown names are ignored.

## Result Lifetime

Feeder `feed_next` functions return pointers to internal monitor output vectors. These pointers remain valid only until the next call to `feed_next`.

Similarly, direct `eval_multi_property` calls return references to monitor-owned output vectors. Copy results that must be retained across later evaluations.

## Error Handling

The CLI exits with an error when:

- the property file cannot be opened,
- no non-empty formulas are present,
- a formula cannot be parsed,
- the trace feeder cannot be created.

Malformed NDJSON rows that cannot be converted to objects are skipped by the JSON feeder.
