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
        "./loomrv ${RYBINX_FLAGS} 'historically((once[:10]{q}) -> ((not{p}) since {q}))' ${TESTDATA_DIR}/AbsentAQ10.row.bin" \
    --command-name AbsentAQ100 \
        "./loomrv ${RYBINX_FLAGS} 'historically((once[:100]{q}) -> ((not{p}) since {q}))' ${TESTDATA_DIR}/AbsentAQ100.row.bin" \
    --command-name AbsentAQ1000 \
        "./loomrv ${RYBINX_FLAGS} 'historically((once[:1000]{q}) -> ((not{p}) since {q}))' ${TESTDATA_DIR}/AbsentAQ1000.row.bin" \
    --command-name AbsentBQR10 \
        "./loomrv ${RYBINX_FLAGS} 'historically({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q})' ${TESTDATA_DIR}/AbsentBQR10.row.bin" \
    --command-name AbsentBQR100 \
        "./loomrv ${RYBINX_FLAGS} 'historically({r} && !{q} && once{q}) -> ((not{p}) since[30:100] {q})' ${TESTDATA_DIR}/AbsentBQR100.row.bin" \
    --command-name AbsentBQR1000 \
        "./loomrv ${RYBINX_FLAGS} 'historically({r} && !{q} && once{q}) -> ((not{p}) since[300:1000] {q})' ${TESTDATA_DIR}/AbsentBQR1000.row.bin" \
    --command-name AbsentBR10 \
        "./loomrv ${RYBINX_FLAGS} 'historically({r} -> (historically[:10](not{p})))' ${TESTDATA_DIR}/AbsentBR10.row.bin" \
    --command-name AbsentBR100 \
        "./loomrv ${RYBINX_FLAGS} 'historically({r} -> (historically[:100](not{p})))' ${TESTDATA_DIR}/AbsentBR100.row.bin" \
    --command-name AbsentBR1000 \
        "./loomrv ${RYBINX_FLAGS} 'historically({r} -> (historically[:1000](not{p})))' ${TESTDATA_DIR}/AbsentBR1000.row.bin" \
    --command-name AlwaysAQ10 \
        "./loomrv ${RYBINX_FLAGS} 'historically((once[:10]{q}) -> ({p} since {q}))' ${TESTDATA_DIR}/AlwaysAQ10.row.bin" \
    --command-name AlwaysAQ100 \
        "./loomrv ${RYBINX_FLAGS} 'historically((once[:100]{q}) -> ({p} since {q}))' ${TESTDATA_DIR}/AlwaysAQ100.row.bin" \
    --command-name AlwaysAQ1000 \
        "./loomrv ${RYBINX_FLAGS} 'historically((once[:1000]{q}) -> ({p} since {q}))' ${TESTDATA_DIR}/AlwaysAQ1000.row.bin" \
    --command-name AlwaysBQR10 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))' ${TESTDATA_DIR}/AlwaysBQR10.row.bin" \
    --command-name AlwaysBQR100 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ({p} since[30:100] {q}))' ${TESTDATA_DIR}/AlwaysBQR100.row.bin" \
    --command-name AlwaysBQR1000 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ({p} since[300:1000] {q}))' ${TESTDATA_DIR}/AlwaysBQR1000.row.bin" \
    --command-name AlwaysBR10 \
        "./loomrv ${RYBINX_FLAGS} 'historically({r} -> (historically[:10]{p}))' ${TESTDATA_DIR}/AlwaysBR10.row.bin" \
    --command-name AlwaysBR100 \
        "./loomrv ${RYBINX_FLAGS} 'historically({r} -> (historically[:100]{p}))' ${TESTDATA_DIR}/AlwaysBR100.row.bin" \
    --command-name AlwaysBR1000 \
        "./loomrv ${RYBINX_FLAGS} 'historically({r} -> (historically[:1000]{p}))' ${TESTDATA_DIR}/AlwaysBR1000.row.bin" \
    --command-name RecurBQR10 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))' ${TESTDATA_DIR}/RecurBQR10.row.bin" \
    --command-name RecurBQR100 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ((once[:100]({p} or {q})) since {q}))' ${TESTDATA_DIR}/RecurBQR100.row.bin" \
    --command-name RecurBQR1000 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ((once[:1000]({p} or {q})) since {q}))' ${TESTDATA_DIR}/RecurBQR1000.row.bin" \
    --command-name RecurGLB10 \
        "./loomrv ${RYBINX_FLAGS} 'historically(once[:10]{p})' ${TESTDATA_DIR}/RecurGLB10.row.bin" \
    --command-name RecurGLB100 \
        "./loomrv ${RYBINX_FLAGS} 'historically(once[:100]{p})' ${TESTDATA_DIR}/RecurGLB100.row.bin" \
    --command-name RecurGLB1000 \
        "./loomrv ${RYBINX_FLAGS} 'historically(once[:1000]{p})' ${TESTDATA_DIR}/RecurGLB1000.row.bin" \
    --command-name RespondBQR10 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))' ${TESTDATA_DIR}/RespondBQR10.row.bin" \
    --command-name RespondBQR100 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p})) since {q}))' ${TESTDATA_DIR}/RespondBQR100.row.bin" \
    --command-name RespondBQR1000 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})) since {q}))' ${TESTDATA_DIR}/RespondBQR1000.row.bin" \
    --command-name RespondGLB10 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))' ${TESTDATA_DIR}/RespondGLB10.row.bin" \
    --command-name RespondGLB100 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p}))' ${TESTDATA_DIR}/RespondGLB100.row.bin" \
    --command-name RespondGLB1000 \
        "./loomrv ${RYBINX_FLAGS} 'historically(({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p}))' ${TESTDATA_DIR}/RespondGLB1000.row.bin"
