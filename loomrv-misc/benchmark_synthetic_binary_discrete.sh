#!/usr/bin/env bash
# benchmark_synthetic_binary_discrete.sh
# Binary-format discrete counterpart of benchmark_synthetic_discrete.sh.
# Compares rybinx (sequential, one formula at a time) against
# loomrv --binary (all formulas in one multi-property call)
# across the same four synthetic formula scenarios.
#
# Usage: ./benchmark_synthetic_binary_discrete.sh [FORMULA_COUNT [TRACE_LENGTH]]
set -euo pipefail

# All paths are relative to loomrv-misc/ — run this script from that directory.
RUN_TS="${RUN_TS:-$(date '+%Y-%m-%d_%H-%M-%S')}"
RESULTS_DIR="${RESULTS_DIR:-results/${RUN_TS}}"
mkdir -p "${RESULTS_DIR}" tmp traces

echo "============================================================"
echo "   Synthetic Multi-Property Binary Benchmark (rybinx vs loomrv --binary) [Discrete]"
echo "============================================================"

FORMULA_COUNT=${1:-10}
TRACE_LENGTH=${2:-1000000}

JSONL_TRACE="traces/synthetic_discrete_${TRACE_LENGTH}.jsonl"
BIN_TRACE="traces/synthetic_discrete_${TRACE_LENGTH}.row.bin"
FORMULA_DIR="synthetic_${FORMULA_COUNT}"

# 1. Setup Binaries
BIN_PATH="../build/loomrv"
if [ ! -f "${BIN_PATH}" ]; then
    BIN_PATH="./build/loomrv"
fi
if [ ! -f "${BIN_PATH}" ]; then
    echo "Error: loomrv not found in build directory." >&2
    exit 1
fi

COUNT_NODES_BIN="$(dirname "$BIN_PATH")/count-nodes"

# 2. Generate JSONL trace (if needed)
if [ ! -f "${JSONL_TRACE}" ]; then
    echo "Generating discrete trace with ${TRACE_LENGTH} events..."
    python3 tools/generate_traces.py --length "${TRACE_LENGTH}" --out "${JSONL_TRACE}" --step-mode constant
else
    echo "Using existing discrete trace: ${JSONL_TRACE}"
fi

# 3. Convert to binary (if needed)
if [ ! -f "${BIN_TRACE}" ]; then
    echo "Converting ${JSONL_TRACE} to binary..."
    python3 tools/to_binary_row.py --file "${JSONL_TRACE}"
else
    echo "Using existing binary trace: ${BIN_TRACE}"
fi

# 4. Generate Formulas (if needed)
if [ ! -d "${FORMULA_DIR}" ]; then
    echo "Generating synthetic formulas (N=${FORMULA_COUNT})..."
    python3 tools/generate_test_formulas.py --count "${FORMULA_COUNT}" -o "${FORMULA_DIR}" --checker "$(dirname "$BIN_PATH")/check-grammar"
else
    echo "Using existing formulas in: ${FORMULA_DIR}"
fi

# 5. Run Benchmarks
SCENARIOS=(
    "best_case_shared_core.txt"
    "worst_case_unique_leaves.txt"
    "nested_best_case.txt"
    "nested_worst_case.txt"
)

RYBINX_FLAGS=${RYBINX_FLAGS:-"-x"}
commit_hash=$(git rev-parse HEAD)

# Sequential loomrv wrapper (one formula at a time, binary input)
DOVERIFY_BIN_WRAPPER="tmp/run_doverify_bin_seq_synthetic_discrete.sh"
cat << 'EOF' > "${DOVERIFY_BIN_WRAPPER}"
#!/usr/bin/env bash
BIN=$1
TRACE=$2
PROPS=$3
TMP_DIR=$(mktemp -d)
trap 'rm -rf -- "$TMP_DIR"' EXIT

i=0
while IFS= read -r formula; do
    [ -z "$formula" ] && continue
    echo "$formula" > "$TMP_DIR/prop_$i.txt"
    $BIN --binary --discrete "$TRACE" "$TMP_DIR/prop_$i.txt" > /dev/null
    i=$((i+1))
done < "$PROPS"
EOF
chmod +x "${DOVERIFY_BIN_WRAPPER}"

for SCENARIO in "${SCENARIOS[@]}"; do
    PROPS_FILE="${FORMULA_DIR}/${SCENARIO}"

    if [ ! -f "${PROPS_FILE}" ]; then
        echo "Warning: ${PROPS_FILE} not found. Skipping."
        continue
    fi

    echo ""
    echo "-----------------------------------------------------"
    echo " Benchmarking Scenario: ${SCENARIO}"
    echo " Trace:    ${BIN_TRACE}"
    echo " Formulas: ${PROPS_FILE}"
    echo "-----------------------------------------------------"

    if [ -f "${COUNT_NODES_BIN}" ]; then
        "${COUNT_NODES_BIN}" "${PROPS_FILE}"
        echo "-----------------------------------------------------"
    fi

    SCENARIO_NAME=$(basename "${SCENARIO}" .txt)
    RES_FILE="${RESULTS_DIR}/bench_binary_discrete_${SCENARIO_NAME}_${FORMULA_COUNT}props_${TRACE_LENGTH}ev_${commit_hash}.json"

    hyperfine \
        --warmup 2 \
        --runs 10 \
        --export-json "${RES_FILE}" \
        --command-name "rybinx (Sequential)" \
            "./run_rybinx_seq.sh \"${RYBINX_FLAGS}\" \"${BIN_TRACE}\" \"${PROPS_FILE}\"" \
        --command-name "loomrv (Sequential, binary)" \
            "./${DOVERIFY_BIN_WRAPPER} \"${BIN_PATH}\" \"${BIN_TRACE}\" \"${PROPS_FILE}\"" \
        --command-name "loomrv (Multi-Property, binary)" \
            "${BIN_PATH} --binary --discrete ${BIN_TRACE} ${PROPS_FILE}"
done

echo ""
echo "Done! Benchmark results exported to ${RESULTS_DIR}/"
