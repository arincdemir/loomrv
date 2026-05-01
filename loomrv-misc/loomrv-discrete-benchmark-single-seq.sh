#!/usr/bin/env bash
set -xeuo pipefail
mkdir -p traces tmp
RUN_TS="${RUN_TS:-$(date '+%Y-%m-%d_%H-%M-%S')}"
RESULTS_DIR="results/${RUN_TS}"
mkdir -p "${RESULTS_DIR}"

commit_hash=$(git rev-parse HEAD)
echo "Running loomrv sequential multi-property discrete benchmark for commit ${commit_hash}"

TRACE_SIZE="${1:-1000000}"
TEST_TRACE="traces/generated_discrete_trace_${TRACE_SIZE}_s.jsonl"

if [ ! -f "${TEST_TRACE}" ]; then
    echo "Generating discrete trace with size ${TRACE_SIZE}..."
    python3 tools/generate_traces.py -n "${TRACE_SIZE}" --step-mode constant --with-s -o "${TEST_TRACE}"
fi
PROPS_FILE="tmp/multi_properties.txt"

# Create a single properties file with all 30 formulas
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

# Wrapper lives in tmp/ so it doesn't clutter the root
WRAPPER_SCRIPT="tmp/run_doverify_discrete_seq.sh"
cat << 'OEF' > "${WRAPPER_SCRIPT}"
#!/usr/bin/env bash
TRACE_FILE=$1
PROPS_FILE=$2
BIN_EXEC=$3

mkdir -p tmp
TEMP_PROP="tmp/temp_single_prop_$$.txt"
trap 'rm -f "${TEMP_PROP}"' EXIT

while IFS= read -r formula; do
    # Skip empty lines
    [ -z "$formula" ] && continue
    echo "$formula" > "${TEMP_PROP}"
    $BIN_EXEC --discrete "$TRACE_FILE" "${TEMP_PROP}" > /dev/null
done < "$PROPS_FILE"
OEF
chmod +x "${WRAPPER_SCRIPT}"

hyperfine \
    --warmup 3 \
    --runs 25 \
    --export-json "${RESULTS_DIR}/$(basename \"$0\" .sh).${commit_hash}.results.json" \
    --command-name "loomrv_Discrete_Sequential_30" \
        "./${WRAPPER_SCRIPT} \"${TEST_TRACE}\" \"${PROPS_FILE}\" \"${BIN_PATH}\""
