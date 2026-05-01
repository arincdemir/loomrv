#!/usr/bin/env bash
set -euo pipefail

# Set group-level RUN_TS and construct the dir
export RUN_TS=$(date '+%Y-%m-%d_%H-%M-%S')
export RESULTS_DIR="results/${RUN_TS}"

# Ensure RESULTS_DIR is created and owned by the target user
mkdir -p "${RESULTS_DIR}"

SCRIPTS=(
    "loomrv-discrete-benchmark-bin.sh"
    "loomrv-discrete-benchmark-json.sh"
    "rybinx-discrete-benchmark.sh"
    "ryjson-discrete-benchmark.sh"
    "loomrv-discrete-benchmark-single-seq.sh"
    "loomrv-discrete-benchmark-single-binary.sh"
    "loomrv-discrete-benchmark-multi.sh"
    "loomrv-discrete-benchmark-multi-binary.sh"
    "loomrv-discrete-benchmark-multi-and.sh"
    "loomrv-discrete-benchmark-multi-and-binary.sh"
    "rybinx-discrete-benchmark-multi.sh"
    "rybinx-discrete-benchmark-multi-and.sh"
    "ryjson-discrete-benchmark-multi.sh"
    "ryjson-discrete-benchmark-multi-and.sh"
    "benchmark_synthetic_discrete.sh"
    "benchmark_synthetic_binary_discrete.sh"
)

echo ""
echo "========================================="
echo "Running ${#SCRIPTS[@]} Discrete Benchmark scripts back to back"
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
