#!/usr/bin/env bash
set -euo pipefail

# Set group-level RUN_TS and construct the dir
export RUN_TS=$(date '+%Y-%m-%d_%H-%M-%S')
export RESULTS_DIR="results/${RUN_TS}"

# Ensure RESULTS_DIR is created and owned by the target user
mkdir -p "${RESULTS_DIR}"

# Note: loomrv-benchmark.sh is excluded as it is a legacy version clashing with loomrv-benchmark-dense-binary.sh and its command formatting is out of date.
SCRIPTS=(
    "loomrv-benchmark-dense.sh"
    "loomrv-benchmark-dense-binary.sh"
    "loomrv-benchmark-dense-single-binary.sh"
    "loomrv-benchmark-dense-single-seq.sh"
    "loomrv-benchmark-dense-multi.sh"
    "loomrv-benchmark-dense-multi-binary.sh"
    "loomrv-benchmark-dense-multi-and.sh"
    "loomrv-benchmark-dense-multi-and-binary.sh"
    "rybinx-benchmark-dense.sh"
    "rybinx-benchmark-dense-multi.sh"
    "rybinx-benchmark-dense-multi-and.sh"
    "ryjson-benchmark-dense.sh"
    "ryjson-benchmark-dense-multi.sh"
    "ryjson-benchmark-dense-multi-and.sh"
    "benchmark_synthetic_dense.sh"
    "benchmark_synthetic_binary_dense.sh"
)

echo ""
echo "========================================="
echo "Running ${#SCRIPTS[@]} Dense Benchmark scripts back to back"
echo "========================================="
for script in "${SCRIPTS[@]}"; do
    if [ ! -f "${script}" ]; then
        echo "Warning: ${script} not found. Skipping."
        continue
    fi
    echo ""
    echo ">> Executing ${script} ..."
    
    env PATH="$PATH" RUN_TS="$RUN_TS" RESULTS_DIR="$RESULTS_DIR" bash "./${script}"
done

echo ""
echo "All benchmarks finished! Results exported to: ${RESULTS_DIR}"
