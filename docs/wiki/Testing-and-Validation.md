# Testing and Validation

LoomRV uses Catch2 for unit and integration tests.

## Build And Run Tests

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/tests/unit_tests
```

On the documented repository state, this command passes:

```text
All tests passed (4602936 assertions in 52 test cases)
```

The tests consume the dataset under `data/fullsuite`. CMake downloads it automatically when it is absent.

## Current CTest Note

Although CTest files are generated, `ctest --test-dir build` currently reports no registered tests. Run the Catch2 test binary directly:

```bash
./build/tests/unit_tests
```

## Test Areas

| Test file | Coverage |
|---|---|
| `test_parse.cpp` | Formula parsing, bounds, invalid syntax, proposition maps, and node counts |
| `test_discrete.cpp` | Discrete operator semantics and Timescales benchmark traces |
| `test_dense.cpp` | Dense interval semantics and Timescales benchmark traces |
| `test_interval_set.cpp` | Arena lifecycle and interval-set operations |
| `test_json_input.cpp` | JSON and binary feeders |
| `test_readers.cpp` | Low-level row readers |

## Useful Targeted Runs

Catch2 filters can focus a test category:

```bash
./build/tests/unit_tests "[parse]"
./build/tests/unit_tests "[json]"
./build/tests/unit_tests "[binary]"
./build/tests/unit_tests "[interval_set]"
```

## Manual Validation

Validate a formula:

```bash
./build/check-grammar "historically({p})"
```

Inspect cross-property sharing:

```bash
./build/count-nodes properties.txt
```

Run a CLI trace and inspect verdicts:

```bash
./build/loomrv --discrete --print trace.jsonl properties.txt
```

## Coverage

Configure coverage instrumentation with:

```bash
cmake -S . -B build-coverage -DENABLE_COVERAGE=ON
cmake --build build-coverage -j
cmake --build build-coverage --target coverage
```

The coverage target requires `gcovr` and emits HTML and XML reports under the build directory.
