#!/usr/bin/env bash
set -xeuo pipefail

commit_hash=$(git rev-parse HEAD)
echo "Running time series discrete benchmark for commit ${commit_hash}"
RUN_TS="${RUN_TS:-$(date '+%Y-%m-%d_%H-%M-%S')}"
RESULTS_DIR="results/${RUN_TS}"
mkdir -p "${RESULTS_DIR}"

RYBINX_FLAGS=${RYBINX_FLAGS:-"-x"}
TESTDATA_DIR="${TESTDATA_DIR:-${1:-../data/fullsuite}}"

hyperfine \
    --warmup 3 \
    --runs 25 \
    --export-json "${RESULTS_DIR}/$(basename \"$0\" .sh).${commit_hash}.results.json" \
    --command-name AbsentAQ10 \
        "rybinx ${RYBINX_FLAGS} 'historically((once[:10]{q}) -> ((not{p}) since {q}))' ${TESTDATA_DIR}/AbsentAQ/Discrete/1M/AbsentAQ10.row.bin" \
    --command-name AbsentAQ100 \
        "rybinx ${RYBINX_FLAGS} 'historically((once[:100]{q}) -> ((not{p}) since {q}))' ${TESTDATA_DIR}/AbsentAQ/Discrete/1M/AbsentAQ100.row.bin" \
    --command-name AbsentAQ1000 \
        "rybinx ${RYBINX_FLAGS} 'historically((once[:1000]{q}) -> ((not{p}) since {q}))' ${TESTDATA_DIR}/AbsentAQ/Discrete/1M/AbsentAQ1000.row.bin" \
    --command-name AbsentBQR10 \
        "rybinx ${RYBINX_FLAGS} 'historically({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q})' ${TESTDATA_DIR}/AbsentBQR/Discrete/1M/AbsentBQR10.row.bin" \
    --command-name AbsentBQR100 \
        "rybinx ${RYBINX_FLAGS} 'historically({r} && !{q} && once{q}) -> ((not{p}) since[30:100] {q})' ${TESTDATA_DIR}/AbsentBQR/Discrete/1M/AbsentBQR100.row.bin" \
    --command-name AbsentBQR1000 \
        "rybinx ${RYBINX_FLAGS} 'historically({r} && !{q} && once{q}) -> ((not{p}) since[300:1000] {q})' ${TESTDATA_DIR}/AbsentBQR/Discrete/1M/AbsentBQR1000.row.bin" \
    --command-name AbsentBR10 \
        "rybinx ${RYBINX_FLAGS} 'historically({r} -> (historically[:10](not{p})))' ${TESTDATA_DIR}/AbsentBR/Discrete/1M/AbsentBR10.row.bin" \
    --command-name AbsentBR100 \
        "rybinx ${RYBINX_FLAGS} 'historically({r} -> (historically[:100](not{p})))' ${TESTDATA_DIR}/AbsentBR/Discrete/1M/AbsentBR100.row.bin" \
    --command-name AbsentBR1000 \
        "rybinx ${RYBINX_FLAGS} 'historically({r} -> (historically[:1000](not{p})))' ${TESTDATA_DIR}/AbsentBR/Discrete/1M/AbsentBR1000.row.bin" \
    --command-name AlwaysAQ10 \
        "rybinx ${RYBINX_FLAGS} 'historically((once[:10]{q}) -> ({p} since {q}))' ${TESTDATA_DIR}/AlwaysAQ/Discrete/1M/AlwaysAQ10.row.bin" \
    --command-name AlwaysAQ100 \
        "rybinx ${RYBINX_FLAGS} 'historically((once[:100]{q}) -> ({p} since {q}))' ${TESTDATA_DIR}/AlwaysAQ/Discrete/1M/AlwaysAQ100.row.bin" \
    --command-name AlwaysAQ1000 \
        "rybinx ${RYBINX_FLAGS} 'historically((once[:1000]{q}) -> ({p} since {q}))' ${TESTDATA_DIR}/AlwaysAQ/Discrete/1M/AlwaysAQ1000.row.bin" \
    --command-name AlwaysBQR10 \
        "rybinx ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))' ${TESTDATA_DIR}/AlwaysBQR/Discrete/1M/AlwaysBQR10.row.bin" \
    --command-name AlwaysBQR100 \
        "rybinx ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ({p} since[30:100] {q}))' ${TESTDATA_DIR}/AlwaysBQR/Discrete/1M/AlwaysBQR100.row.bin" \
    --command-name AlwaysBQR1000 \
        "rybinx ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ({p} since[300:1000] {q}))' ${TESTDATA_DIR}/AlwaysBQR/Discrete/1M/AlwaysBQR1000.row.bin" \
    --command-name AlwaysBR10 \
        "rybinx ${RYBINX_FLAGS} 'historically({r} -> (historically[:10]{p}))' ${TESTDATA_DIR}/AlwaysBR/Discrete/1M/AlwaysBR10.row.bin" \
    --command-name AlwaysBR100 \
        "rybinx ${RYBINX_FLAGS} 'historically({r} -> (historically[:100]{p}))' ${TESTDATA_DIR}/AlwaysBR/Discrete/1M/AlwaysBR100.row.bin" \
    --command-name AlwaysBR1000 \
        "rybinx ${RYBINX_FLAGS} 'historically({r} -> (historically[:1000]{p}))' ${TESTDATA_DIR}/AlwaysBR/Discrete/1M/AlwaysBR1000.row.bin" \
    --command-name RecurBQR10 \
        "rybinx ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))' ${TESTDATA_DIR}/RecurBQR/Discrete/1M/RecurBQR10.row.bin" \
    --command-name RecurBQR100 \
        "rybinx ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ((once[:100]({p} or {q})) since {q}))' ${TESTDATA_DIR}/RecurBQR/Discrete/1M/RecurBQR100.row.bin" \
    --command-name RecurBQR1000 \
        "rybinx ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ((once[:1000]({p} or {q})) since {q}))' ${TESTDATA_DIR}/RecurBQR/Discrete/1M/RecurBQR1000.row.bin" \
    --command-name RecurGLB10 \
        "rybinx ${RYBINX_FLAGS} 'historically(once[:10]{p})' ${TESTDATA_DIR}/RecurGLB/Discrete/1M/RecurGLB10.row.bin" \
    --command-name RecurGLB100 \
        "rybinx ${RYBINX_FLAGS} 'historically(once[:100]{p})' ${TESTDATA_DIR}/RecurGLB/Discrete/1M/RecurGLB100.row.bin" \
    --command-name RecurGLB1000 \
        "rybinx ${RYBINX_FLAGS} 'historically(once[:1000]{p})' ${TESTDATA_DIR}/RecurGLB/Discrete/1M/RecurGLB1000.row.bin" \
    --command-name RespondBQR10 \
        "rybinx ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))' ${TESTDATA_DIR}/RespondBQR/Discrete/1M/RespondBQR10.row.bin" \
    --command-name RespondBQR100 \
        "rybinx ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p})) since {q}))' ${TESTDATA_DIR}/RespondBQR/Discrete/1M/RespondBQR100.row.bin" \
    --command-name RespondBQR1000 \
        "rybinx ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})) since {q}))' ${TESTDATA_DIR}/RespondBQR/Discrete/1M/RespondBQR1000.row.bin" \
    --command-name RespondGLB10 \
        "rybinx ${RYBINX_FLAGS} 'historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))' ${TESTDATA_DIR}/RespondGLB/Discrete/1M/RespondGLB10.row.bin" \
    --command-name RespondGLB100 \
        "rybinx ${RYBINX_FLAGS} 'historically(({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p}))' ${TESTDATA_DIR}/RespondGLB/Discrete/1M/RespondGLB100.row.bin" \
    --command-name RespondGLB1000 \
        "rybinx ${RYBINX_FLAGS} 'historically(({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p}))' ${TESTDATA_DIR}/RespondGLB/Discrete/1M/RespondGLB1000.row.bin"
