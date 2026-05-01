#!/usr/bin/env bash
set -xeuo pipefail
mkdir -p traces tmp
RUN_TS="${RUN_TS:-$(date '+%Y-%m-%d_%H-%M-%S')}"
RESULTS_DIR="results/${RUN_TS}"
mkdir -p "${RESULTS_DIR}"

commit_hash=$(git rev-parse HEAD)
echo "Running ryjson sequential multi-property benchmark for commit ${commit_hash}"

TRACE_SIZE="${1:-1000000}"
MAX_STEP="${2:-8}"
TEST_TRACE="traces/generated_trace_${TRACE_SIZE}_${MAX_STEP}_s.jsonl"

if [ ! -f "${TEST_TRACE}" ]; then
    echo "Generating trace with size ${TRACE_SIZE} and max_step ${MAX_STEP}..."
    python3 tools/generate_traces.py -n "${TRACE_SIZE}" --max-step "${MAX_STEP}" --with-s -o "${TEST_TRACE}"
fi
PROPS_FILE="tmp/multi_properties.txt"

# Create a single properties file with all 30 formulas from the benchmarking suite
cat << 'EOF' > "${PROPS_FILE}"
historically((once[:10]{q}) -> ((not{p}) since {q}))
historically((once[:100]{q}) -> ((not{p}) since {q}))
historically((once[:1000]{q}) -> ((not{p}) since {q}))
historically({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q})
historically({r} && !{q} && once{q}) -> ((not{p}) since[30:100] {q})
historically({r} && !{q} && once{q}) -> ((not{p}) since[300:1000] {q})
historically({r} -> (historically[:10](not{p})))
historically({r} -> (historically[:100](not{p})))
historically({r} -> (historically[:1000](not{p})))
historically((once[:10]{q}) -> ({p} since {q}))
historically((once[:100]{q}) -> ({p} since {q}))
historically((once[:1000]{q}) -> ({p} since {q}))
historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))
historically(({r} && !{q} && once{q}) -> ({p} since[30:100] {q}))
historically(({r} && !{q} && once{q}) -> ({p} since[300:1000] {q}))
historically({r} -> (historically[:10]{p}))
historically({r} -> (historically[:100]{p}))
historically({r} -> (historically[:1000]{p}))
historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))
historically(({r} && !{q} && once{q}) -> ((once[:100]({p} or {q})) since {q}))
historically(({r} && !{q} && once{q}) -> ((once[:1000]({p} or {q})) since {q}))
historically(once[:10]{p})
historically(once[:100]{p})
historically(once[:1000]{p})
historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))
historically(({r} && !{q} && once{q}) -> ( (({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p})) since {q}))
historically(({r} && !{q} && once{q}) -> ( (({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})) since {q}))
historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))
historically(({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p}))
historically(({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p}))
EOF

echo "Generated multi_properties.txt with 30 formulas."

COUNT_NODES_BIN="../build/count-nodes"
if [ ! -f "${COUNT_NODES_BIN}" ]; then
    COUNT_NODES_BIN="./build/count-nodes"
fi
if [ -f "${COUNT_NODES_BIN}" ]; then
    "${COUNT_NODES_BIN}" "${PROPS_FILE}"
    echo "-----------------------------------------------------"
fi

RYJSON_FLAGS=${RYJSON_FLAGS:-"-v"}

hyperfine \
    --warmup 3 \
    --runs 25 \
    --export-json "${RESULTS_DIR}/$(basename \"$0\" .sh).${commit_hash}.results.json" \
    --command-name "ryjson_Sequential_30" \
        "./run_ryjson_seq.sh \"${RYJSON_FLAGS}\" \"${TEST_TRACE}\" \"${PROPS_FILE}\""
