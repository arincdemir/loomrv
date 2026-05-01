#!/usr/bin/env bash
# loomrv-discrete-benchmark-json.sh
# Head-to-head counterpart of ryjson-discrete-benchmark.sh.
# Runs loomrv on the same fullsuite .jsonl files as ryjson,
# covering all 10 pattern families × 3 timing values (30 benchmarks).
#
# Usage: ./loomrv-discrete-benchmark-json.sh [TESTDATA_DIR]
#   TESTDATA_DIR  path to the fullsuite directory (default: ../data/fullsuite)
set -xeuo pipefail

commit_hash=$(git rev-parse HEAD)
echo "Running loomrv single-formula discrete benchmark (JSON) for commit ${commit_hash}"
RUN_TS="${RUN_TS:-$(date '+%Y-%m-%d_%H-%M-%S')}"
RESULTS_DIR="results/${RUN_TS}"
mkdir -p "${RESULTS_DIR}"

TESTDATA_DIR="${TESTDATA_DIR:-${1:-../data/fullsuite}}"

BIN_PATH="../build/loomrv"
if [ ! -f "${BIN_PATH}" ]; then
    BIN_PATH="./build/loomrv"
fi
if [ ! -f "${BIN_PATH}" ]; then
    echo "Error: loomrv not found." >&2
    exit 1
fi

# Pre-write all 30 formula files into a temp directory.
# Use a trap so they are always cleaned up, even on error.
FORMULA_TMP=$(mktemp -d)
trap 'rm -rf -- "${FORMULA_TMP}"' EXIT

cat > "${FORMULA_TMP}/AbsentAQ10.txt"     <<'EOF'
historically((once[:10]{q}) -> ((not{p}) since {q}))
EOF
cat > "${FORMULA_TMP}/AbsentAQ100.txt"    <<'EOF'
historically((once[:100]{q}) -> ((not{p}) since {q}))
EOF
cat > "${FORMULA_TMP}/AbsentAQ1000.txt"   <<'EOF'
historically((once[:1000]{q}) -> ((not{p}) since {q}))
EOF
cat > "${FORMULA_TMP}/AbsentBQR10.txt"    <<'EOF'
historically({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q})
EOF
cat > "${FORMULA_TMP}/AbsentBQR100.txt"   <<'EOF'
historically({r} && !{q} && once{q}) -> ((not{p}) since[30:100] {q})
EOF
cat > "${FORMULA_TMP}/AbsentBQR1000.txt"  <<'EOF'
historically({r} && !{q} && once{q}) -> ((not{p}) since[300:1000] {q})
EOF
cat > "${FORMULA_TMP}/AbsentBR10.txt"     <<'EOF'
historically({r} -> (historically[:10](not{p})))
EOF
cat > "${FORMULA_TMP}/AbsentBR100.txt"    <<'EOF'
historically({r} -> (historically[:100](not{p})))
EOF
cat > "${FORMULA_TMP}/AbsentBR1000.txt"   <<'EOF'
historically({r} -> (historically[:1000](not{p})))
EOF
cat > "${FORMULA_TMP}/AlwaysAQ10.txt"     <<'EOF'
historically((once[:10]{q}) -> ({p} since {q}))
EOF
cat > "${FORMULA_TMP}/AlwaysAQ100.txt"    <<'EOF'
historically((once[:100]{q}) -> ({p} since {q}))
EOF
cat > "${FORMULA_TMP}/AlwaysAQ1000.txt"   <<'EOF'
historically((once[:1000]{q}) -> ({p} since {q}))
EOF
cat > "${FORMULA_TMP}/AlwaysBQR10.txt"    <<'EOF'
historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))
EOF
cat > "${FORMULA_TMP}/AlwaysBQR100.txt"   <<'EOF'
historically(({r} && !{q} && once{q}) -> ({p} since[30:100] {q}))
EOF
cat > "${FORMULA_TMP}/AlwaysBQR1000.txt"  <<'EOF'
historically(({r} && !{q} && once{q}) -> ({p} since[300:1000] {q}))
EOF
cat > "${FORMULA_TMP}/AlwaysBR10.txt"     <<'EOF'
historically({r} -> (historically[:10]{p}))
EOF
cat > "${FORMULA_TMP}/AlwaysBR100.txt"    <<'EOF'
historically({r} -> (historically[:100]{p}))
EOF
cat > "${FORMULA_TMP}/AlwaysBR1000.txt"   <<'EOF'
historically({r} -> (historically[:1000]{p}))
EOF
cat > "${FORMULA_TMP}/RecurBQR10.txt"     <<'EOF'
historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))
EOF
cat > "${FORMULA_TMP}/RecurBQR100.txt"    <<'EOF'
historically(({r} && !{q} && once{q}) -> ((once[:100]({p} or {q})) since {q}))
EOF
cat > "${FORMULA_TMP}/RecurBQR1000.txt"   <<'EOF'
historically(({r} && !{q} && once{q}) -> ((once[:1000]({p} or {q})) since {q}))
EOF
cat > "${FORMULA_TMP}/RecurGLB10.txt"     <<'EOF'
historically(once[:10]{p})
EOF
cat > "${FORMULA_TMP}/RecurGLB100.txt"    <<'EOF'
historically(once[:100]{p})
EOF
cat > "${FORMULA_TMP}/RecurGLB1000.txt"   <<'EOF'
historically(once[:1000]{p})
EOF
cat > "${FORMULA_TMP}/RespondBQR10.txt"   <<'EOF'
historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))
EOF
cat > "${FORMULA_TMP}/RespondBQR100.txt"  <<'EOF'
historically(({r} && !{q} && once{q}) -> ( (({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p})) since {q}))
EOF
cat > "${FORMULA_TMP}/RespondBQR1000.txt" <<'EOF'
historically(({r} && !{q} && once{q}) -> ( (({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})) since {q}))
EOF
cat > "${FORMULA_TMP}/RespondGLB10.txt"   <<'EOF'
historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))
EOF
cat > "${FORMULA_TMP}/RespondGLB100.txt"  <<'EOF'
historically(({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p}))
EOF
cat > "${FORMULA_TMP}/RespondGLB1000.txt" <<'EOF'
historically(({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p}))
EOF

F="${FORMULA_TMP}"   # shorthand
B="${BIN_PATH}"
D="${TESTDATA_DIR}"

hyperfine \
    --warmup 3 \
    --runs 25 \
    --export-json "${RESULTS_DIR}/$(basename \"$0\" .sh).${commit_hash}.results.json" \
    --command-name "AbsentAQ10"     "${B} --discrete ${D}/AbsentAQ/Discrete/1M/AbsentAQ10.jsonl     ${F}/AbsentAQ10.txt" \
    --command-name "AbsentAQ100"    "${B} --discrete ${D}/AbsentAQ/Discrete/1M/AbsentAQ100.jsonl    ${F}/AbsentAQ100.txt" \
    --command-name "AbsentAQ1000"   "${B} --discrete ${D}/AbsentAQ/Discrete/1M/AbsentAQ1000.jsonl   ${F}/AbsentAQ1000.txt" \
    --command-name "AbsentBQR10"    "${B} --discrete ${D}/AbsentBQR/Discrete/1M/AbsentBQR10.jsonl     ${F}/AbsentBQR10.txt" \
    --command-name "AbsentBQR100"   "${B} --discrete ${D}/AbsentBQR/Discrete/1M/AbsentBQR100.jsonl    ${F}/AbsentBQR100.txt" \
    --command-name "AbsentBQR1000"  "${B} --discrete ${D}/AbsentBQR/Discrete/1M/AbsentBQR1000.jsonl   ${F}/AbsentBQR1000.txt" \
    --command-name "AbsentBR10"     "${B} --discrete ${D}/AbsentBR/Discrete/1M/AbsentBR10.jsonl     ${F}/AbsentBR10.txt" \
    --command-name "AbsentBR100"    "${B} --discrete ${D}/AbsentBR/Discrete/1M/AbsentBR100.jsonl    ${F}/AbsentBR100.txt" \
    --command-name "AbsentBR1000"   "${B} --discrete ${D}/AbsentBR/Discrete/1M/AbsentBR1000.jsonl   ${F}/AbsentBR1000.txt" \
    --command-name "AlwaysAQ10"     "${B} --discrete ${D}/AlwaysAQ/Discrete/1M/AlwaysAQ10.jsonl     ${F}/AlwaysAQ10.txt" \
    --command-name "AlwaysAQ100"    "${B} --discrete ${D}/AlwaysAQ/Discrete/1M/AlwaysAQ100.jsonl    ${F}/AlwaysAQ100.txt" \
    --command-name "AlwaysAQ1000"   "${B} --discrete ${D}/AlwaysAQ/Discrete/1M/AlwaysAQ1000.jsonl   ${F}/AlwaysAQ1000.txt" \
    --command-name "AlwaysBQR10"    "${B} --discrete ${D}/AlwaysBQR/Discrete/1M/AlwaysBQR10.jsonl     ${F}/AlwaysBQR10.txt" \
    --command-name "AlwaysBQR100"   "${B} --discrete ${D}/AlwaysBQR/Discrete/1M/AlwaysBQR100.jsonl    ${F}/AlwaysBQR100.txt" \
    --command-name "AlwaysBQR1000"  "${B} --discrete ${D}/AlwaysBQR/Discrete/1M/AlwaysBQR1000.jsonl   ${F}/AlwaysBQR1000.txt" \
    --command-name "AlwaysBR10"     "${B} --discrete ${D}/AlwaysBR/Discrete/1M/AlwaysBR10.jsonl     ${F}/AlwaysBR10.txt" \
    --command-name "AlwaysBR100"    "${B} --discrete ${D}/AlwaysBR/Discrete/1M/AlwaysBR100.jsonl    ${F}/AlwaysBR100.txt" \
    --command-name "AlwaysBR1000"   "${B} --discrete ${D}/AlwaysBR/Discrete/1M/AlwaysBR1000.jsonl   ${F}/AlwaysBR1000.txt" \
    --command-name "RecurBQR10"     "${B} --discrete ${D}/RecurBQR/Discrete/1M/RecurBQR10.jsonl     ${F}/RecurBQR10.txt" \
    --command-name "RecurBQR100"    "${B} --discrete ${D}/RecurBQR/Discrete/1M/RecurBQR100.jsonl    ${F}/RecurBQR100.txt" \
    --command-name "RecurBQR1000"   "${B} --discrete ${D}/RecurBQR/Discrete/1M/RecurBQR1000.jsonl   ${F}/RecurBQR1000.txt" \
    --command-name "RecurGLB10"     "${B} --discrete ${D}/RecurGLB/Discrete/1M/RecurGLB10.jsonl     ${F}/RecurGLB10.txt" \
    --command-name "RecurGLB100"    "${B} --discrete ${D}/RecurGLB/Discrete/1M/RecurGLB100.jsonl    ${F}/RecurGLB100.txt" \
    --command-name "RecurGLB1000"   "${B} --discrete ${D}/RecurGLB/Discrete/1M/RecurGLB1000.jsonl   ${F}/RecurGLB1000.txt" \
    --command-name "RespondBQR10"   "${B} --discrete ${D}/RespondBQR/Discrete/1M/RespondBQR10.jsonl     ${F}/RespondBQR10.txt" \
    --command-name "RespondBQR100"  "${B} --discrete ${D}/RespondBQR/Discrete/1M/RespondBQR100.jsonl    ${F}/RespondBQR100.txt" \
    --command-name "RespondBQR1000" "${B} --discrete ${D}/RespondBQR/Discrete/1M/RespondBQR1000.jsonl   ${F}/RespondBQR1000.txt" \
    --command-name "RespondGLB10"   "${B} --discrete ${D}/RespondGLB/Discrete/1M/RespondGLB10.jsonl     ${F}/RespondGLB10.txt" \
    --command-name "RespondGLB100"  "${B} --discrete ${D}/RespondGLB/Discrete/1M/RespondGLB100.jsonl    ${F}/RespondGLB100.txt" \
    --command-name "RespondGLB1000" "${B} --discrete ${D}/RespondGLB/Discrete/1M/RespondGLB1000.jsonl   ${F}/RespondGLB1000.txt"
