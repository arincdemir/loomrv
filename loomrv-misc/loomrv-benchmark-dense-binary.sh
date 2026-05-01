#!/usr/bin/env bash
# loomrv-benchmark-single-binary.sh
# Head-to-head counterpart of rybinx-benchmark-dense.sh.
# Runs loomrv --binary on the same fullsuite .row.bin files as rybinx,
# covering all 10 pattern families × 3 timing values × 3 condensations (90 benchmarks).
#
# Usage: ./loomrv-benchmark-single-binary.sh [TESTDATA_DIR]
#   TESTDATA_DIR  path to the fullsuite directory (default: ../data/fullsuite)
set -xeuo pipefail

commit_hash=$(git rev-parse HEAD)
echo "Running loomrv single-formula binary benchmark for commit ${commit_hash}"
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
    --command-name "AbsentAQ10 Dense1"       "${B} --binary --dense ${D}/AbsentAQ/Dense1/1M/AbsentAQ10.row.bin     ${F}/AbsentAQ10.txt" \
    --command-name "AbsentAQ10 Dense10"      "${B} --binary --dense ${D}/AbsentAQ/Dense10/1M/AbsentAQ10.row.bin    ${F}/AbsentAQ10.txt" \
    --command-name "AbsentAQ10 Dense100"     "${B} --binary --dense ${D}/AbsentAQ/Dense100/1M/AbsentAQ10.row.bin   ${F}/AbsentAQ10.txt" \
    --command-name "AbsentAQ100 Dense1"      "${B} --binary --dense ${D}/AbsentAQ/Dense1/1M/AbsentAQ100.row.bin    ${F}/AbsentAQ100.txt" \
    --command-name "AbsentAQ100 Dense10"     "${B} --binary --dense ${D}/AbsentAQ/Dense10/1M/AbsentAQ100.row.bin   ${F}/AbsentAQ100.txt" \
    --command-name "AbsentAQ100 Dense100"    "${B} --binary --dense ${D}/AbsentAQ/Dense100/1M/AbsentAQ100.row.bin  ${F}/AbsentAQ100.txt" \
    --command-name "AbsentAQ1000 Dense1"     "${B} --binary --dense ${D}/AbsentAQ/Dense1/1M/AbsentAQ1000.row.bin   ${F}/AbsentAQ1000.txt" \
    --command-name "AbsentAQ1000 Dense10"    "${B} --binary --dense ${D}/AbsentAQ/Dense10/1M/AbsentAQ1000.row.bin  ${F}/AbsentAQ1000.txt" \
    --command-name "AbsentAQ1000 Dense100"   "${B} --binary --dense ${D}/AbsentAQ/Dense100/1M/AbsentAQ1000.row.bin ${F}/AbsentAQ1000.txt" \
    --command-name "AbsentBQR10 Dense1"      "${B} --binary --dense ${D}/AbsentBQR/Dense1/1M/AbsentBQR10.row.bin     ${F}/AbsentBQR10.txt" \
    --command-name "AbsentBQR10 Dense10"     "${B} --binary --dense ${D}/AbsentBQR/Dense10/1M/AbsentBQR10.row.bin    ${F}/AbsentBQR10.txt" \
    --command-name "AbsentBQR10 Dense100"    "${B} --binary --dense ${D}/AbsentBQR/Dense100/1M/AbsentBQR10.row.bin   ${F}/AbsentBQR10.txt" \
    --command-name "AbsentBQR100 Dense1"     "${B} --binary --dense ${D}/AbsentBQR/Dense1/1M/AbsentBQR100.row.bin    ${F}/AbsentBQR100.txt" \
    --command-name "AbsentBQR100 Dense10"    "${B} --binary --dense ${D}/AbsentBQR/Dense10/1M/AbsentBQR100.row.bin   ${F}/AbsentBQR100.txt" \
    --command-name "AbsentBQR100 Dense100"   "${B} --binary --dense ${D}/AbsentBQR/Dense100/1M/AbsentBQR100.row.bin  ${F}/AbsentBQR100.txt" \
    --command-name "AbsentBQR1000 Dense1"    "${B} --binary --dense ${D}/AbsentBQR/Dense1/1M/AbsentBQR1000.row.bin   ${F}/AbsentBQR1000.txt" \
    --command-name "AbsentBQR1000 Dense10"   "${B} --binary --dense ${D}/AbsentBQR/Dense10/1M/AbsentBQR1000.row.bin  ${F}/AbsentBQR1000.txt" \
    --command-name "AbsentBQR1000 Dense100"  "${B} --binary --dense ${D}/AbsentBQR/Dense100/1M/AbsentBQR1000.row.bin ${F}/AbsentBQR1000.txt" \
    --command-name "AbsentBR10 Dense1"       "${B} --binary --dense ${D}/AbsentBR/Dense1/1M/AbsentBR10.row.bin     ${F}/AbsentBR10.txt" \
    --command-name "AbsentBR10 Dense10"      "${B} --binary --dense ${D}/AbsentBR/Dense10/1M/AbsentBR10.row.bin    ${F}/AbsentBR10.txt" \
    --command-name "AbsentBR10 Dense100"     "${B} --binary --dense ${D}/AbsentBR/Dense100/1M/AbsentBR10.row.bin   ${F}/AbsentBR10.txt" \
    --command-name "AbsentBR100 Dense1"      "${B} --binary --dense ${D}/AbsentBR/Dense1/1M/AbsentBR100.row.bin    ${F}/AbsentBR100.txt" \
    --command-name "AbsentBR100 Dense10"     "${B} --binary --dense ${D}/AbsentBR/Dense10/1M/AbsentBR100.row.bin   ${F}/AbsentBR100.txt" \
    --command-name "AbsentBR100 Dense100"    "${B} --binary --dense ${D}/AbsentBR/Dense100/1M/AbsentBR100.row.bin  ${F}/AbsentBR100.txt" \
    --command-name "AbsentBR1000 Dense1"     "${B} --binary --dense ${D}/AbsentBR/Dense1/1M/AbsentBR1000.row.bin   ${F}/AbsentBR1000.txt" \
    --command-name "AbsentBR1000 Dense10"    "${B} --binary --dense ${D}/AbsentBR/Dense10/1M/AbsentBR1000.row.bin  ${F}/AbsentBR1000.txt" \
    --command-name "AbsentBR1000 Dense100"   "${B} --binary --dense ${D}/AbsentBR/Dense100/1M/AbsentBR1000.row.bin ${F}/AbsentBR1000.txt" \
    --command-name "AlwaysAQ10 Dense1"       "${B} --binary --dense ${D}/AlwaysAQ/Dense1/1M/AlwaysAQ10.row.bin     ${F}/AlwaysAQ10.txt" \
    --command-name "AlwaysAQ10 Dense10"      "${B} --binary --dense ${D}/AlwaysAQ/Dense10/1M/AlwaysAQ10.row.bin    ${F}/AlwaysAQ10.txt" \
    --command-name "AlwaysAQ10 Dense100"     "${B} --binary --dense ${D}/AlwaysAQ/Dense100/1M/AlwaysAQ10.row.bin   ${F}/AlwaysAQ10.txt" \
    --command-name "AlwaysAQ100 Dense1"      "${B} --binary --dense ${D}/AlwaysAQ/Dense1/1M/AlwaysAQ100.row.bin    ${F}/AlwaysAQ100.txt" \
    --command-name "AlwaysAQ100 Dense10"     "${B} --binary --dense ${D}/AlwaysAQ/Dense10/1M/AlwaysAQ100.row.bin   ${F}/AlwaysAQ100.txt" \
    --command-name "AlwaysAQ100 Dense100"    "${B} --binary --dense ${D}/AlwaysAQ/Dense100/1M/AlwaysAQ100.row.bin  ${F}/AlwaysAQ100.txt" \
    --command-name "AlwaysAQ1000 Dense1"     "${B} --binary --dense ${D}/AlwaysAQ/Dense1/1M/AlwaysAQ1000.row.bin   ${F}/AlwaysAQ1000.txt" \
    --command-name "AlwaysAQ1000 Dense10"    "${B} --binary --dense ${D}/AlwaysAQ/Dense10/1M/AlwaysAQ1000.row.bin  ${F}/AlwaysAQ1000.txt" \
    --command-name "AlwaysAQ1000 Dense100"   "${B} --binary --dense ${D}/AlwaysAQ/Dense100/1M/AlwaysAQ1000.row.bin ${F}/AlwaysAQ1000.txt" \
    --command-name "AlwaysBQR10 Dense1"      "${B} --binary --dense ${D}/AlwaysBQR/Dense1/1M/AlwaysBQR10.row.bin     ${F}/AlwaysBQR10.txt" \
    --command-name "AlwaysBQR10 Dense10"     "${B} --binary --dense ${D}/AlwaysBQR/Dense10/1M/AlwaysBQR10.row.bin    ${F}/AlwaysBQR10.txt" \
    --command-name "AlwaysBQR10 Dense100"    "${B} --binary --dense ${D}/AlwaysBQR/Dense100/1M/AlwaysBQR10.row.bin   ${F}/AlwaysBQR10.txt" \
    --command-name "AlwaysBQR100 Dense1"     "${B} --binary --dense ${D}/AlwaysBQR/Dense1/1M/AlwaysBQR100.row.bin    ${F}/AlwaysBQR100.txt" \
    --command-name "AlwaysBQR100 Dense10"    "${B} --binary --dense ${D}/AlwaysBQR/Dense10/1M/AlwaysBQR100.row.bin   ${F}/AlwaysBQR100.txt" \
    --command-name "AlwaysBQR100 Dense100"   "${B} --binary --dense ${D}/AlwaysBQR/Dense100/1M/AlwaysBQR100.row.bin  ${F}/AlwaysBQR100.txt" \
    --command-name "AlwaysBQR1000 Dense1"    "${B} --binary --dense ${D}/AlwaysBQR/Dense1/1M/AlwaysBQR1000.row.bin   ${F}/AlwaysBQR1000.txt" \
    --command-name "AlwaysBQR1000 Dense10"   "${B} --binary --dense ${D}/AlwaysBQR/Dense10/1M/AlwaysBQR1000.row.bin  ${F}/AlwaysBQR1000.txt" \
    --command-name "AlwaysBQR1000 Dense100"  "${B} --binary --dense ${D}/AlwaysBQR/Dense100/1M/AlwaysBQR1000.row.bin ${F}/AlwaysBQR1000.txt" \
    --command-name "AlwaysBR10 Dense1"       "${B} --binary --dense ${D}/AlwaysBR/Dense1/1M/AlwaysBR10.row.bin     ${F}/AlwaysBR10.txt" \
    --command-name "AlwaysBR10 Dense10"      "${B} --binary --dense ${D}/AlwaysBR/Dense10/1M/AlwaysBR10.row.bin    ${F}/AlwaysBR10.txt" \
    --command-name "AlwaysBR10 Dense100"     "${B} --binary --dense ${D}/AlwaysBR/Dense100/1M/AlwaysBR10.row.bin   ${F}/AlwaysBR10.txt" \
    --command-name "AlwaysBR100 Dense1"      "${B} --binary --dense ${D}/AlwaysBR/Dense1/1M/AlwaysBR100.row.bin    ${F}/AlwaysBR100.txt" \
    --command-name "AlwaysBR100 Dense10"     "${B} --binary --dense ${D}/AlwaysBR/Dense10/1M/AlwaysBR100.row.bin   ${F}/AlwaysBR100.txt" \
    --command-name "AlwaysBR100 Dense100"    "${B} --binary --dense ${D}/AlwaysBR/Dense100/1M/AlwaysBR100.row.bin  ${F}/AlwaysBR100.txt" \
    --command-name "AlwaysBR1000 Dense1"     "${B} --binary --dense ${D}/AlwaysBR/Dense1/1M/AlwaysBR1000.row.bin   ${F}/AlwaysBR1000.txt" \
    --command-name "AlwaysBR1000 Dense10"    "${B} --binary --dense ${D}/AlwaysBR/Dense10/1M/AlwaysBR1000.row.bin  ${F}/AlwaysBR1000.txt" \
    --command-name "AlwaysBR1000 Dense100"   "${B} --binary --dense ${D}/AlwaysBR/Dense100/1M/AlwaysBR1000.row.bin ${F}/AlwaysBR1000.txt" \
    --command-name "RecurBQR10 Dense1"       "${B} --binary --dense ${D}/RecurBQR/Dense1/1M/RecurBQR10.row.bin     ${F}/RecurBQR10.txt" \
    --command-name "RecurBQR10 Dense10"      "${B} --binary --dense ${D}/RecurBQR/Dense10/1M/RecurBQR10.row.bin    ${F}/RecurBQR10.txt" \
    --command-name "RecurBQR10 Dense100"     "${B} --binary --dense ${D}/RecurBQR/Dense100/1M/RecurBQR10.row.bin   ${F}/RecurBQR10.txt" \
    --command-name "RecurBQR100 Dense1"      "${B} --binary --dense ${D}/RecurBQR/Dense1/1M/RecurBQR100.row.bin    ${F}/RecurBQR100.txt" \
    --command-name "RecurBQR100 Dense10"     "${B} --binary --dense ${D}/RecurBQR/Dense10/1M/RecurBQR100.row.bin   ${F}/RecurBQR100.txt" \
    --command-name "RecurBQR100 Dense100"    "${B} --binary --dense ${D}/RecurBQR/Dense100/1M/RecurBQR100.row.bin  ${F}/RecurBQR100.txt" \
    --command-name "RecurBQR1000 Dense1"     "${B} --binary --dense ${D}/RecurBQR/Dense1/1M/RecurBQR1000.row.bin   ${F}/RecurBQR1000.txt" \
    --command-name "RecurBQR1000 Dense10"    "${B} --binary --dense ${D}/RecurBQR/Dense10/1M/RecurBQR1000.row.bin  ${F}/RecurBQR1000.txt" \
    --command-name "RecurBQR1000 Dense100"   "${B} --binary --dense ${D}/RecurBQR/Dense100/1M/RecurBQR1000.row.bin ${F}/RecurBQR1000.txt" \
    --command-name "RecurGLB10 Dense1"       "${B} --binary --dense ${D}/RecurGLB/Dense1/1M/RecurGLB10.row.bin     ${F}/RecurGLB10.txt" \
    --command-name "RecurGLB10 Dense10"      "${B} --binary --dense ${D}/RecurGLB/Dense10/1M/RecurGLB10.row.bin    ${F}/RecurGLB10.txt" \
    --command-name "RecurGLB10 Dense100"     "${B} --binary --dense ${D}/RecurGLB/Dense100/1M/RecurGLB10.row.bin   ${F}/RecurGLB10.txt" \
    --command-name "RecurGLB100 Dense1"      "${B} --binary --dense ${D}/RecurGLB/Dense1/1M/RecurGLB100.row.bin    ${F}/RecurGLB100.txt" \
    --command-name "RecurGLB100 Dense10"     "${B} --binary --dense ${D}/RecurGLB/Dense10/1M/RecurGLB100.row.bin   ${F}/RecurGLB100.txt" \
    --command-name "RecurGLB100 Dense100"    "${B} --binary --dense ${D}/RecurGLB/Dense100/1M/RecurGLB100.row.bin  ${F}/RecurGLB100.txt" \
    --command-name "RecurGLB1000 Dense1"     "${B} --binary --dense ${D}/RecurGLB/Dense1/1M/RecurGLB1000.row.bin   ${F}/RecurGLB1000.txt" \
    --command-name "RecurGLB1000 Dense10"    "${B} --binary --dense ${D}/RecurGLB/Dense10/1M/RecurGLB1000.row.bin  ${F}/RecurGLB1000.txt" \
    --command-name "RecurGLB1000 Dense100"   "${B} --binary --dense ${D}/RecurGLB/Dense100/1M/RecurGLB1000.row.bin ${F}/RecurGLB1000.txt" \
    --command-name "RespondBQR10 Dense1"     "${B} --binary --dense ${D}/RespondBQR/Dense1/1M/RespondBQR10.row.bin     ${F}/RespondBQR10.txt" \
    --command-name "RespondBQR10 Dense10"    "${B} --binary --dense ${D}/RespondBQR/Dense10/1M/RespondBQR10.row.bin    ${F}/RespondBQR10.txt" \
    --command-name "RespondBQR10 Dense100"   "${B} --binary --dense ${D}/RespondBQR/Dense100/1M/RespondBQR10.row.bin   ${F}/RespondBQR10.txt" \
    --command-name "RespondBQR100 Dense1"    "${B} --binary --dense ${D}/RespondBQR/Dense1/1M/RespondBQR100.row.bin    ${F}/RespondBQR100.txt" \
    --command-name "RespondBQR100 Dense10"   "${B} --binary --dense ${D}/RespondBQR/Dense10/1M/RespondBQR100.row.bin   ${F}/RespondBQR100.txt" \
    --command-name "RespondBQR100 Dense100"  "${B} --binary --dense ${D}/RespondBQR/Dense100/1M/RespondBQR100.row.bin  ${F}/RespondBQR100.txt" \
    --command-name "RespondBQR1000 Dense1"   "${B} --binary --dense ${D}/RespondBQR/Dense1/1M/RespondBQR1000.row.bin   ${F}/RespondBQR1000.txt" \
    --command-name "RespondBQR1000 Dense10"  "${B} --binary --dense ${D}/RespondBQR/Dense10/1M/RespondBQR1000.row.bin  ${F}/RespondBQR1000.txt" \
    --command-name "RespondBQR1000 Dense100" "${B} --binary --dense ${D}/RespondBQR/Dense100/1M/RespondBQR1000.row.bin ${F}/RespondBQR1000.txt" \
    --command-name "RespondGLB10 Dense1"     "${B} --binary --dense ${D}/RespondGLB/Dense1/1M/RespondGLB10.row.bin     ${F}/RespondGLB10.txt" \
    --command-name "RespondGLB10 Dense10"    "${B} --binary --dense ${D}/RespondGLB/Dense10/1M/RespondGLB10.row.bin    ${F}/RespondGLB10.txt" \
    --command-name "RespondGLB10 Dense100"   "${B} --binary --dense ${D}/RespondGLB/Dense100/1M/RespondGLB10.row.bin   ${F}/RespondGLB10.txt" \
    --command-name "RespondGLB100 Dense1"    "${B} --binary --dense ${D}/RespondGLB/Dense1/1M/RespondGLB100.row.bin    ${F}/RespondGLB100.txt" \
    --command-name "RespondGLB100 Dense10"   "${B} --binary --dense ${D}/RespondGLB/Dense10/1M/RespondGLB100.row.bin   ${F}/RespondGLB100.txt" \
    --command-name "RespondGLB100 Dense100"  "${B} --binary --dense ${D}/RespondGLB/Dense100/1M/RespondGLB100.row.bin  ${F}/RespondGLB100.txt" \
    --command-name "RespondGLB1000 Dense1"   "${B} --binary --dense ${D}/RespondGLB/Dense1/1M/RespondGLB1000.row.bin   ${F}/RespondGLB1000.txt" \
    --command-name "RespondGLB1000 Dense10"  "${B} --binary --dense ${D}/RespondGLB/Dense10/1M/RespondGLB1000.row.bin  ${F}/RespondGLB1000.txt" \
    --command-name "RespondGLB1000 Dense100" "${B} --binary --dense ${D}/RespondGLB/Dense100/1M/RespondGLB1000.row.bin ${F}/RespondGLB1000.txt"
