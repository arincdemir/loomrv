#!/usr/bin/env bash
# rybinx-benchmark-multi-and.sh
# AND-merged multi-property binary benchmark for rybinx.
# Mirrors ryjson-benchmark-multi-and.sh but uses a .row.bin trace.
#
# Usage: ./rybinx-benchmark-multi-and.sh [TRACE_SIZE [MAX_STEP]]
set -xeuo pipefail
mkdir -p traces tmp
RUN_TS="${RUN_TS:-$(date '+%Y-%m-%d_%H-%M-%S')}"
RESULTS_DIR="results/${RUN_TS}"
mkdir -p "${RESULTS_DIR}"

commit_hash=$(git rev-parse HEAD)
echo "Running rybinx single AND multi-property binary benchmark for commit ${commit_hash}"

TRACE_SIZE="${1:-1000000}"
MAX_STEP="${2:-8}"
JSONL_TRACE="traces/generated_trace_${TRACE_SIZE}_${MAX_STEP}_s.jsonl"
BIN_TRACE="traces/generated_trace_${TRACE_SIZE}_${MAX_STEP}_s.row.bin"

# 1. Generate JSONL trace if needed
if [ ! -f "${JSONL_TRACE}" ]; then
    echo "Generating JSONL trace (size=${TRACE_SIZE}, max_step=${MAX_STEP})..."
    python3 tools/generate_traces.py -n "${TRACE_SIZE}" --max-step "${MAX_STEP}" --with-s -o "${JSONL_TRACE}"
fi

# 2. Convert to binary if needed
if [ ! -f "${BIN_TRACE}" ]; then
    echo "Converting ${JSONL_TRACE} to binary..."
    python3 tools/to_binary_row.py --file "${JSONL_TRACE}"
fi

PROPS_FILE="tmp/multi_properties_and.txt"

# 3. Write properties file (all 30 formulas merged with AND into one)
cat << 'EOF' > "${PROPS_FILE}"
(historically((once[:10]{q}) -> ((not{p}) since {q}))) and (historically((once[:100]{q}) -> ((not{p}) since {q}))) and (historically((once[:1000]{q}) -> ((not{p}) since {q}))) and (historically({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q})) and (historically({r} && !{q} && once{q}) -> ((not{p}) since[30:100] {q})) and (historically({r} && !{q} && once{q}) -> ((not{p}) since[300:1000] {q})) and (historically({r} -> (historically[:10](not{p})))) and (historically({r} -> (historically[:100](not{p})))) and (historically({r} -> (historically[:1000](not{p})))) and (historically((once[:10]{q}) -> ({p} since {q}))) and (historically((once[:100]{q}) -> ({p} since {q}))) and (historically((once[:1000]{q}) -> ({p} since {q}))) and (historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))) and (historically(({r} && !{q} && once{q}) -> ({p} since[30:100] {q}))) and (historically(({r} && !{q} && once{q}) -> ({p} since[300:1000] {q}))) and (historically({r} -> (historically[:10]{p}))) and (historically({r} -> (historically[:100]{p}))) and (historically({r} -> (historically[:1000]{p}))) and (historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))) and (historically(({r} && !{q} && once{q}) -> ((once[:100]({p} or {q})) since {q}))) and (historically(({r} && !{q} && once{q}) -> ((once[:1000]({p} or {q})) since {q}))) and (historically(once[:10]{p})) and (historically(once[:100]{p})) and (historically(once[:1000]{p})) and (historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))) and (historically(({r} && !{q} && once{q}) -> ( (({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p})) since {q}))) and (historically(({r} && !{q} && once{q}) -> ( (({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})) since {q}))) and (historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))) and (historically(({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p}))) and (historically(({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})))
EOF

echo "Generated ${PROPS_FILE} with 1 AND-merged formula."

COUNT_NODES_BIN="../build/count-nodes"
if [ ! -f "${COUNT_NODES_BIN}" ]; then
    COUNT_NODES_BIN="./build/count-nodes"
fi
if [ -f "${COUNT_NODES_BIN}" ]; then
    "${COUNT_NODES_BIN}" "${PROPS_FILE}"
    echo "-----------------------------------------------------"
fi

RYBINX_FLAGS=${RYBINX_FLAGS:-"-v"}

hyperfine \
    --warmup 3 \
    --runs 25 \
    --export-json "${RESULTS_DIR}/$(basename \"$0\" .sh).${commit_hash}.results.json" \
    --command-name "rybinx_Combined_AND_30" \
        "./run_rybinx_single.sh \"${RYBINX_FLAGS}\" \"${BIN_TRACE}\" \"${PROPS_FILE}\""
