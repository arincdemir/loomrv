#!/usr/bin/env bash
set -euo pipefail

# All paths are relative to loomrv-misc/ — run this script from that directory.
RUN_TS="${RUN_TS:-$(date '+%Y-%m-%d_%H-%M-%S')}"
RESULTS_DIR="${RESULTS_DIR:-results/${RUN_TS}}"
mkdir -p "${RESULTS_DIR}" tmp traces

echo "====================================================="
echo "   Synthetic Multi-Property Deduplication Benchmark (Discrete) "
echo "====================================================="

FORMULA_COUNT=${1:-10}
TRACE_LENGTH=${2:-1000000}
TRACE_FILE="traces/synthetic_discrete_${TRACE_LENGTH}.jsonl"
FORMULA_DIR="synthetic_${FORMULA_COUNT}"

# 1. Setup Binaries
BIN_PATH="../build/loomrv"
if [ ! -f "${BIN_PATH}" ]; then
    BIN_PATH="./build/loomrv"
fi
if [ ! -f "${BIN_PATH}" ]; then
    echo "Error: loomrv not found in build directory."
    exit 1
fi

COUNT_NODES_BIN="$(dirname "$BIN_PATH")/count-nodes"

# 2. Generate Trace (if needed)
if [ ! -f "${TRACE_FILE}" ]; then
    echo "Generating discrete trace with ${TRACE_LENGTH} events..."
    python3 tools/generate_traces.py --length "${TRACE_LENGTH}" --out "${TRACE_FILE}" --step-mode constant
else
    echo "Using existing discrete trace: ${TRACE_FILE}"
fi

# 3. Generate Formulas (if needed)
if [ ! -d "${FORMULA_DIR}" ]; then
    echo "Generating synthetic formulas (N=${FORMULA_COUNT})..."
    python3 tools/generate_test_formulas.py --count "${FORMULA_COUNT}" -o "${FORMULA_DIR}" --checker "$(dirname "$BIN_PATH")/check-grammar"
else
    echo "Using existing formulas in: ${FORMULA_DIR}"
fi

# 4. Create Sequential Wrappers (in tmp/ so they don't clutter the root)
RYJSON_WRAPPER="tmp/run_ryjson_seq_synthetic_discrete.sh"
cat << 'EOF' > "${RYJSON_WRAPPER}"
#!/usr/bin/env bash
TRACE=$1
PROPS=$2
# ryjson takes trace first, then formula. We read line by line.
while IFS= read -r formula; do
    [ -z "$formula" ] && continue
    ryjson -x "$formula" "$TRACE" > /dev/null
done < "$PROPS"
EOF
chmod +x "${RYJSON_WRAPPER}"

# Wrapper for loomrv seq
DOVERIFY_WRAPPER="tmp/run_doverify_seq_synthetic_discrete.sh"
cat << 'EOF' > "${DOVERIFY_WRAPPER}"
#!/usr/bin/env bash
BIN=$1
TRACE=$2
PROPS=$3
# loomrv takes formula files, so we make a temp file for each line.
TMP_DIR=$(mktemp -d)
trap 'rm -rf -- "$TMP_DIR"' EXIT

i=0
while IFS= read -r formula; do
    [ -z "$formula" ] && continue
    echo "$formula" > "$TMP_DIR/prop_$i.txt"
    $BIN --discrete "$TRACE" "$TMP_DIR/prop_$i.txt" > /dev/null
    i=$((i+1))
done < "$PROPS"
EOF
chmod +x "${DOVERIFY_WRAPPER}"


# 5. Run Benchmarks
SCENARIOS=(
    "best_case_shared_core.txt"
    "worst_case_unique_leaves.txt"
    "nested_best_case.txt"
    "nested_worst_case.txt"
)

commit_hash=$(git rev-parse HEAD)

for SCENARIO in "${SCENARIOS[@]}"; do
    PROPS_FILE="${FORMULA_DIR}/${SCENARIO}"
    
    if [ ! -f "${PROPS_FILE}" ]; then
        echo "Warning: ${PROPS_FILE} not found. Skipping."
        continue
    fi
    
    echo ""
    echo "-----------------------------------------------------"
    echo " Benchmarking Scenario: ${SCENARIO}"
    echo " Trace: ${TRACE_FILE}"
    echo " Formulas: ${PROPS_FILE}"
    echo "-----------------------------------------------------"
    
    if [ -f "${COUNT_NODES_BIN}" ]; then
        "${COUNT_NODES_BIN}" "${PROPS_FILE}"
        echo "-----------------------------------------------------"
    fi
    
    SCENARIO_NAME=$(basename "${SCENARIO}" .txt)
    RES_FILE="${RESULTS_DIR}/bench_discrete_${SCENARIO_NAME}_${FORMULA_COUNT}props_${TRACE_LENGTH}ev_${commit_hash}.json"

    hyperfine \
        --warmup 2 \
        --runs 10 \
        --export-json "${RES_FILE}" \
        --command-name "ryjson (Sequential)" \
            "./${RYJSON_WRAPPER} \"${TRACE_FILE}\" \"${PROPS_FILE}\"" \
        --command-name "loomrv (Sequential)" \
            "./${DOVERIFY_WRAPPER} \"${BIN_PATH}\" \"${TRACE_FILE}\" \"${PROPS_FILE}\"" \
        --command-name "loomrv (Multi-Property)" \
            "${BIN_PATH} --discrete ${TRACE_FILE} ${PROPS_FILE}"
done

echo ""
echo "Done! Benchmark results exported to ${RESULTS_DIR}/"
