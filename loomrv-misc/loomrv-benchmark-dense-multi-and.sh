#!/usr/bin/env bash
set -xeuo pipefail
mkdir -p traces tmp
RUN_TS="${RUN_TS:-$(date '+%Y-%m-%d_%H-%M-%S')}"
RESULTS_DIR="results/${RUN_TS}"
mkdir -p "${RESULTS_DIR}"

commit_hash=$(git rev-parse HEAD)
echo "Running multi-property 'AND' benchmark for commit ${commit_hash}"

TRACE_SIZE="${1:-1000000}"
MAX_STEP="${2:-8}"
TEST_TRACE="traces/generated_trace_${TRACE_SIZE}_${MAX_STEP}_s.jsonl"

if [ ! -f "${TEST_TRACE}" ]; then
    echo "Generating trace with size ${TRACE_SIZE} and max_step ${MAX_STEP}..."
    python3 tools/generate_traces.py -n "${TRACE_SIZE}" --max-step "${MAX_STEP}" --with-s -o "${TEST_TRACE}"
fi
PROPS_FILE="tmp/multi_properties_and.txt"

# Create a single properties file with all 30 formulas merged with AND
cat << 'EOF' > "${PROPS_FILE}"
(historically((once[:10]{q}) -> ((not{p}) since {q}))) and (historically((once[:100]{q}) -> ((not{p}) since {q}))) and (historically((once[:1000]{q}) -> ((not{p}) since {q}))) and (historically({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q})) and (historically({r} && !{q} && once{q}) -> ((not{p}) since[30:100] {q})) and (historically({r} && !{q} && once{q}) -> ((not{p}) since[300:1000] {q})) and (historically({r} -> (historically[:10](not{p})))) and (historically({r} -> (historically[:100](not{p})))) and (historically({r} -> (historically[:1000](not{p})))) and (historically((once[:10]{q}) -> ({p} since {q}))) and (historically((once[:100]{q}) -> ({p} since {q}))) and (historically((once[:1000]{q}) -> ({p} since {q}))) and (historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))) and (historically(({r} && !{q} && once{q}) -> ({p} since[30:100] {q}))) and (historically(({r} && !{q} && once{q}) -> ({p} since[300:1000] {q}))) and (historically({r} -> (historically[:10]{p}))) and (historically({r} -> (historically[:100]{p}))) and (historically({r} -> (historically[:1000]{p}))) and (historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))) and (historically(({r} && !{q} && once{q}) -> ((once[:100]({p} or {q})) since {q}))) and (historically(({r} && !{q} && once{q}) -> ((once[:1000]({p} or {q})) since {q}))) and (historically(once[:10]{p})) and (historically(once[:100]{p})) and (historically(once[:1000]{p})) and (historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))) and (historically(({r} && !{q} && once{q}) -> ( (({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p})) since {q}))) and (historically(({r} && !{q} && once{q}) -> ( (({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})) since {q}))) and (historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))) and (historically(({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p}))) and (historically(({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})))
EOF

echo "Generated multi_properties_and.txt with 1 merged formula."

# Make sure we're using the updated path for the binary
BIN_PATH="../build/loomrv"
if [ ! -f "${BIN_PATH}" ]; then
    BIN_PATH="./build/loomrv"
fi

COUNT_NODES_BIN="$(dirname "$BIN_PATH")/count-nodes"
if [ -f "${COUNT_NODES_BIN}" ]; then
    "${COUNT_NODES_BIN}" "${PROPS_FILE}"
    echo "-----------------------------------------------------"
fi

hyperfine \
    --warmup 3 \
    --runs 25 \
    --export-json "${RESULTS_DIR}/$(basename \"$0\" .sh).${commit_hash}.results.json" \
    --command-name "Combined_MultiProperty_AND_30" \
        "${BIN_PATH} --dense ${TEST_TRACE} ${PROPS_FILE}"
