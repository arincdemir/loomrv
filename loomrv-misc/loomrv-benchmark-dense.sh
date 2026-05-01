mkdir -p prop_files
echo 'historically((once[:10]{q}) -> ((not{p}) since {q}))' > prop_files/prop_0.txt
echo 'historically((once[:10]{q}) -> ((not{p}) since {q}))' > prop_files/prop_1.txt
echo 'historically((once[:10]{q}) -> ((not{p}) since {q}))' > prop_files/prop_2.txt
echo 'historically((once[:100]{q}) -> ((not{p}) since {q}))' > prop_files/prop_3.txt
echo 'historically((once[:100]{q}) -> ((not{p}) since {q}))' > prop_files/prop_4.txt
echo 'historically((once[:100]{q}) -> ((not{p}) since {q}))' > prop_files/prop_5.txt
echo 'historically((once[:1000]{q}) -> ((not{p}) since {q}))' > prop_files/prop_6.txt
echo 'historically((once[:1000]{q}) -> ((not{p}) since {q}))' > prop_files/prop_7.txt
echo 'historically((once[:1000]{q}) -> ((not{p}) since {q}))' > prop_files/prop_8.txt
echo 'historically({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q})' > prop_files/prop_9.txt
echo 'historically({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q})' > prop_files/prop_10.txt
echo 'historically({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q})' > prop_files/prop_11.txt
echo 'historically({r} && !{q} && once{q}) -> ((not{p}) since[30:100] {q})' > prop_files/prop_12.txt
echo 'historically({r} && !{q} && once{q}) -> ((not{p}) since[30:100] {q})' > prop_files/prop_13.txt
echo 'historically({r} && !{q} && once{q}) -> ((not{p}) since[30:100] {q})' > prop_files/prop_14.txt
echo 'historically({r} && !{q} && once{q}) -> ((not{p}) since[300:1000] {q})' > prop_files/prop_15.txt
echo 'historically({r} && !{q} && once{q}) -> ((not{p}) since[300:1000] {q})' > prop_files/prop_16.txt
echo 'historically({r} && !{q} && once{q}) -> ((not{p}) since[300:1000] {q})' > prop_files/prop_17.txt
echo 'historically({r} -> (historically[:10](not{p})))' > prop_files/prop_18.txt
echo 'historically({r} -> (historically[:10](not{p})))' > prop_files/prop_19.txt
echo 'historically({r} -> (historically[:10](not{p})))' > prop_files/prop_20.txt
echo 'historically({r} -> (historically[:100](not{p})))' > prop_files/prop_21.txt
echo 'historically({r} -> (historically[:100](not{p})))' > prop_files/prop_22.txt
echo 'historically({r} -> (historically[:100](not{p})))' > prop_files/prop_23.txt
echo 'historically({r} -> (historically[:1000](not{p})))' > prop_files/prop_24.txt
echo 'historically({r} -> (historically[:1000](not{p})))' > prop_files/prop_25.txt
echo 'historically({r} -> (historically[:1000](not{p})))' > prop_files/prop_26.txt
echo 'historically((once[:10]{q}) -> ({p} since {q}))' > prop_files/prop_27.txt
echo 'historically((once[:10]{q}) -> ({p} since {q}))' > prop_files/prop_28.txt
echo 'historically((once[:10]{q}) -> ({p} since {q}))' > prop_files/prop_29.txt
echo 'historically((once[:100]{q}) -> ({p} since {q}))' > prop_files/prop_30.txt
echo 'historically((once[:100]{q}) -> ({p} since {q}))' > prop_files/prop_31.txt
echo 'historically((once[:100]{q}) -> ({p} since {q}))' > prop_files/prop_32.txt
echo 'historically((once[:1000]{q}) -> ({p} since {q}))' > prop_files/prop_33.txt
echo 'historically((once[:1000]{q}) -> ({p} since {q}))' > prop_files/prop_34.txt
echo 'historically((once[:1000]{q}) -> ({p} since {q}))' > prop_files/prop_35.txt
echo 'historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))' > prop_files/prop_36.txt
echo 'historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))' > prop_files/prop_37.txt
echo 'historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))' > prop_files/prop_38.txt
echo 'historically(({r} && !{q} && once{q}) -> ({p} since[30:100] {q}))' > prop_files/prop_39.txt
echo 'historically(({r} && !{q} && once{q}) -> ({p} since[30:100] {q}))' > prop_files/prop_40.txt
echo 'historically(({r} && !{q} && once{q}) -> ({p} since[30:100] {q}))' > prop_files/prop_41.txt
echo 'historically(({r} && !{q} && once{q}) -> ({p} since[300:1000] {q}))' > prop_files/prop_42.txt
echo 'historically(({r} && !{q} && once{q}) -> ({p} since[300:1000] {q}))' > prop_files/prop_43.txt
echo 'historically(({r} && !{q} && once{q}) -> ({p} since[300:1000] {q}))' > prop_files/prop_44.txt
echo 'historically({r} -> (historically[:10]{p}))' > prop_files/prop_45.txt
echo 'historically({r} -> (historically[:10]{p}))' > prop_files/prop_46.txt
echo 'historically({r} -> (historically[:10]{p}))' > prop_files/prop_47.txt
echo 'historically({r} -> (historically[:100]{p}))' > prop_files/prop_48.txt
echo 'historically({r} -> (historically[:100]{p}))' > prop_files/prop_49.txt
echo 'historically({r} -> (historically[:100]{p}))' > prop_files/prop_50.txt
echo 'historically({r} -> (historically[:1000]{p}))' > prop_files/prop_51.txt
echo 'historically({r} -> (historically[:1000]{p}))' > prop_files/prop_52.txt
echo 'historically({r} -> (historically[:1000]{p}))' > prop_files/prop_53.txt
echo 'historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))' > prop_files/prop_54.txt
echo 'historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))' > prop_files/prop_55.txt
echo 'historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))' > prop_files/prop_56.txt
echo 'historically(({r} && !{q} && once{q}) -> ((once[:100]({p} or {q})) since {q}))' > prop_files/prop_57.txt
echo 'historically(({r} && !{q} && once{q}) -> ((once[:100]({p} or {q})) since {q}))' > prop_files/prop_58.txt
echo 'historically(({r} && !{q} && once{q}) -> ((once[:100]({p} or {q})) since {q}))' > prop_files/prop_59.txt
echo 'historically(({r} && !{q} && once{q}) -> ((once[:1000]({p} or {q})) since {q}))' > prop_files/prop_60.txt
echo 'historically(({r} && !{q} && once{q}) -> ((once[:1000]({p} or {q})) since {q}))' > prop_files/prop_61.txt
echo 'historically(({r} && !{q} && once{q}) -> ((once[:1000]({p} or {q})) since {q}))' > prop_files/prop_62.txt
echo 'historically(once[:10]{p})' > prop_files/prop_63.txt
echo 'historically(once[:10]{p})' > prop_files/prop_64.txt
echo 'historically(once[:10]{p})' > prop_files/prop_65.txt
echo 'historically(once[:100]{p})' > prop_files/prop_66.txt
echo 'historically(once[:100]{p})' > prop_files/prop_67.txt
echo 'historically(once[:100]{p})' > prop_files/prop_68.txt
echo 'historically(once[:1000]{p})' > prop_files/prop_69.txt
echo 'historically(once[:1000]{p})' > prop_files/prop_70.txt
echo 'historically(once[:1000]{p})' > prop_files/prop_71.txt
echo 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))' > prop_files/prop_72.txt
echo 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))' > prop_files/prop_73.txt
echo 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))' > prop_files/prop_74.txt
echo 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p})) since {q}))' > prop_files/prop_75.txt
echo 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p})) since {q}))' > prop_files/prop_76.txt
echo 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p})) since {q}))' > prop_files/prop_77.txt
echo 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})) since {q}))' > prop_files/prop_78.txt
echo 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})) since {q}))' > prop_files/prop_79.txt
echo 'historically(({r} && !{q} && once{q}) -> ( (({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p})) since {q}))' > prop_files/prop_80.txt
echo 'historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))' > prop_files/prop_81.txt
echo 'historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))' > prop_files/prop_82.txt
echo 'historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))' > prop_files/prop_83.txt
echo 'historically(({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p}))' > prop_files/prop_84.txt
echo 'historically(({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p}))' > prop_files/prop_85.txt
echo 'historically(({s} -> once[30:100]{p}) and not((not {s}) since[100:] {p}))' > prop_files/prop_86.txt
echo 'historically(({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p}))' > prop_files/prop_87.txt
echo 'historically(({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p}))' > prop_files/prop_88.txt
echo 'historically(({s} -> once[300:1000]{p}) and not((not {s}) since[1000:] {p}))' > prop_files/prop_89.txt

#!/usr/bin/env bash
set -xeuo pipefail

commit_hash=$(git rev-parse HEAD)
echo "Running time series discrete benchmark for commit ${commit_hash}"
RUN_TS="${RUN_TS:-$(date '+%Y-%m-%d_%H-%M-%S')}"
RESULTS_DIR="results/${RUN_TS}"
mkdir -p "${RESULTS_DIR}"

DOVERIFY_FLAGS=${DOVERIFY_FLAGS:-"-v"}
TESTDATA_DIR="${TESTDATA_DIR:-${1:-../data/fullsuite}}"

hyperfine \
    --warmup 3 \
    --runs 25 \
    --export-json "${RESULTS_DIR}/$(basename \"$0\" .sh).${commit_hash}.results.json" \
    --command-name "AbsentAQ10 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentAQ/Dense1/1M/AbsentAQ10.jsonl prop_files/prop_0.txt" \
    --command-name "AbsentAQ10 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentAQ/Dense10/1M/AbsentAQ10.jsonl prop_files/prop_1.txt" \
    --command-name "AbsentAQ10 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentAQ/Dense100/1M/AbsentAQ10.jsonl prop_files/prop_2.txt" \
    --command-name "AbsentAQ100 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentAQ/Dense1/1M/AbsentAQ100.jsonl prop_files/prop_3.txt" \
    --command-name "AbsentAQ100 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentAQ/Dense10/1M/AbsentAQ100.jsonl prop_files/prop_4.txt" \
    --command-name "AbsentAQ100 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentAQ/Dense100/1M/AbsentAQ100.jsonl prop_files/prop_5.txt" \
    --command-name "AbsentAQ1000 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentAQ/Dense1/1M/AbsentAQ1000.jsonl prop_files/prop_6.txt" \
    --command-name "AbsentAQ1000 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentAQ/Dense10/1M/AbsentAQ1000.jsonl prop_files/prop_7.txt" \
    --command-name "AbsentAQ1000 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentAQ/Dense100/1M/AbsentAQ1000.jsonl prop_files/prop_8.txt" \
    --command-name "AbsentBQR10 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBQR/Dense1/1M/AbsentBQR10.jsonl prop_files/prop_9.txt" \
    --command-name "AbsentBQR10 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBQR/Dense10/1M/AbsentBQR10.jsonl prop_files/prop_10.txt" \
    --command-name "AbsentBQR10 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBQR/Dense100/1M/AbsentBQR10.jsonl prop_files/prop_11.txt" \
    --command-name "AbsentBQR100 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBQR/Dense1/1M/AbsentBQR100.jsonl prop_files/prop_12.txt" \
    --command-name "AbsentBQR100 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBQR/Dense10/1M/AbsentBQR100.jsonl prop_files/prop_13.txt" \
    --command-name "AbsentBQR100 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBQR/Dense100/1M/AbsentBQR100.jsonl prop_files/prop_14.txt" \
    --command-name "AbsentBQR1000 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBQR/Dense1/1M/AbsentBQR1000.jsonl prop_files/prop_15.txt" \
    --command-name "AbsentBQR1000 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBQR/Dense10/1M/AbsentBQR1000.jsonl prop_files/prop_16.txt" \
    --command-name "AbsentBQR1000 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBQR/Dense100/1M/AbsentBQR1000.jsonl prop_files/prop_17.txt" \
    --command-name "AbsentBR10 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBR/Dense1/1M/AbsentBR10.jsonl prop_files/prop_18.txt" \
    --command-name "AbsentBR10 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBR/Dense10/1M/AbsentBR10.jsonl prop_files/prop_19.txt" \
    --command-name "AbsentBR10 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBR/Dense100/1M/AbsentBR10.jsonl prop_files/prop_20.txt" \
    --command-name "AbsentBR100 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBR/Dense1/1M/AbsentBR100.jsonl prop_files/prop_21.txt" \
    --command-name "AbsentBR100 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBR/Dense10/1M/AbsentBR100.jsonl prop_files/prop_22.txt" \
    --command-name "AbsentBR100 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBR/Dense100/1M/AbsentBR100.jsonl prop_files/prop_23.txt" \
    --command-name "AbsentBR1000 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBR/Dense1/1M/AbsentBR1000.jsonl prop_files/prop_24.txt" \
    --command-name "AbsentBR1000 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBR/Dense10/1M/AbsentBR1000.jsonl prop_files/prop_25.txt" \
    --command-name "AbsentBR1000 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AbsentBR/Dense100/1M/AbsentBR1000.jsonl prop_files/prop_26.txt" \
    --command-name "AlwaysAQ10 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysAQ/Dense1/1M/AlwaysAQ10.jsonl prop_files/prop_27.txt" \
    --command-name "AlwaysAQ10 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysAQ/Dense10/1M/AlwaysAQ10.jsonl prop_files/prop_28.txt" \
    --command-name "AlwaysAQ10 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysAQ/Dense100/1M/AlwaysAQ10.jsonl prop_files/prop_29.txt" \
    --command-name "AlwaysAQ100 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysAQ/Dense1/1M/AlwaysAQ100.jsonl prop_files/prop_30.txt" \
    --command-name "AlwaysAQ100 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysAQ/Dense10/1M/AlwaysAQ100.jsonl prop_files/prop_31.txt" \
    --command-name "AlwaysAQ100 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysAQ/Dense100/1M/AlwaysAQ100.jsonl prop_files/prop_32.txt" \
    --command-name "AlwaysAQ1000 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysAQ/Dense1/1M/AlwaysAQ1000.jsonl prop_files/prop_33.txt" \
    --command-name "AlwaysAQ1000 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysAQ/Dense10/1M/AlwaysAQ1000.jsonl prop_files/prop_34.txt" \
    --command-name "AlwaysAQ1000 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysAQ/Dense100/1M/AlwaysAQ1000.jsonl prop_files/prop_35.txt" \
    --command-name "AlwaysBQR10 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBQR/Dense1/1M/AlwaysBQR10.jsonl prop_files/prop_36.txt" \
    --command-name "AlwaysBQR10 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBQR/Dense10/1M/AlwaysBQR10.jsonl prop_files/prop_37.txt" \
    --command-name "AlwaysBQR10 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBQR/Dense100/1M/AlwaysBQR10.jsonl prop_files/prop_38.txt" \
    --command-name "AlwaysBQR100 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBQR/Dense1/1M/AlwaysBQR100.jsonl prop_files/prop_39.txt" \
    --command-name "AlwaysBQR100 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBQR/Dense10/1M/AlwaysBQR100.jsonl prop_files/prop_40.txt" \
    --command-name "AlwaysBQR100 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBQR/Dense100/1M/AlwaysBQR100.jsonl prop_files/prop_41.txt" \
    --command-name "AlwaysBQR1000 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBQR/Dense1/1M/AlwaysBQR1000.jsonl prop_files/prop_42.txt" \
    --command-name "AlwaysBQR1000 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBQR/Dense10/1M/AlwaysBQR1000.jsonl prop_files/prop_43.txt" \
    --command-name "AlwaysBQR1000 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBQR/Dense100/1M/AlwaysBQR1000.jsonl prop_files/prop_44.txt" \
    --command-name "AlwaysBR10 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBR/Dense1/1M/AlwaysBR10.jsonl prop_files/prop_45.txt" \
    --command-name "AlwaysBR10 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBR/Dense10/1M/AlwaysBR10.jsonl prop_files/prop_46.txt" \
    --command-name "AlwaysBR10 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBR/Dense100/1M/AlwaysBR10.jsonl prop_files/prop_47.txt" \
    --command-name "AlwaysBR100 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBR/Dense1/1M/AlwaysBR100.jsonl prop_files/prop_48.txt" \
    --command-name "AlwaysBR100 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBR/Dense10/1M/AlwaysBR100.jsonl prop_files/prop_49.txt" \
    --command-name "AlwaysBR100 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBR/Dense100/1M/AlwaysBR100.jsonl prop_files/prop_50.txt" \
    --command-name "AlwaysBR1000 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBR/Dense1/1M/AlwaysBR1000.jsonl prop_files/prop_51.txt" \
    --command-name "AlwaysBR1000 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBR/Dense10/1M/AlwaysBR1000.jsonl prop_files/prop_52.txt" \
    --command-name "AlwaysBR1000 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/AlwaysBR/Dense100/1M/AlwaysBR1000.jsonl prop_files/prop_53.txt" \
    --command-name "RecurBQR10 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurBQR/Dense1/1M/RecurBQR10.jsonl prop_files/prop_54.txt" \
    --command-name "RecurBQR10 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurBQR/Dense10/1M/RecurBQR10.jsonl prop_files/prop_55.txt" \
    --command-name "RecurBQR10 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurBQR/Dense100/1M/RecurBQR10.jsonl prop_files/prop_56.txt" \
    --command-name "RecurBQR100 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurBQR/Dense1/1M/RecurBQR100.jsonl prop_files/prop_57.txt" \
    --command-name "RecurBQR100 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurBQR/Dense10/1M/RecurBQR100.jsonl prop_files/prop_58.txt" \
    --command-name "RecurBQR100 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurBQR/Dense100/1M/RecurBQR100.jsonl prop_files/prop_59.txt" \
    --command-name "RecurBQR1000 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurBQR/Dense1/1M/RecurBQR1000.jsonl prop_files/prop_60.txt" \
    --command-name "RecurBQR1000 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurBQR/Dense10/1M/RecurBQR1000.jsonl prop_files/prop_61.txt" \
    --command-name "RecurBQR1000 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurBQR/Dense100/1M/RecurBQR1000.jsonl prop_files/prop_62.txt" \
    --command-name "RecurGLB10 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurGLB/Dense1/1M/RecurGLB10.jsonl prop_files/prop_63.txt" \
    --command-name "RecurGLB10 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurGLB/Dense10/1M/RecurGLB10.jsonl prop_files/prop_64.txt" \
    --command-name "RecurGLB10 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurGLB/Dense100/1M/RecurGLB10.jsonl prop_files/prop_65.txt" \
    --command-name "RecurGLB100 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurGLB/Dense1/1M/RecurGLB100.jsonl prop_files/prop_66.txt" \
    --command-name "RecurGLB100 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurGLB/Dense10/1M/RecurGLB100.jsonl prop_files/prop_67.txt" \
    --command-name "RecurGLB100 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurGLB/Dense100/1M/RecurGLB100.jsonl prop_files/prop_68.txt" \
    --command-name "RecurGLB1000 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurGLB/Dense1/1M/RecurGLB1000.jsonl prop_files/prop_69.txt" \
    --command-name "RecurGLB1000 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurGLB/Dense10/1M/RecurGLB1000.jsonl prop_files/prop_70.txt" \
    --command-name "RecurGLB1000 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RecurGLB/Dense100/1M/RecurGLB1000.jsonl prop_files/prop_71.txt" \
    --command-name "RespondBQR10 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondBQR/Dense1/1M/RespondBQR10.jsonl prop_files/prop_72.txt" \
    --command-name "RespondBQR10 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondBQR/Dense10/1M/RespondBQR10.jsonl prop_files/prop_73.txt" \
    --command-name "RespondBQR10 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondBQR/Dense100/1M/RespondBQR10.jsonl prop_files/prop_74.txt" \
    --command-name "RespondBQR100 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondBQR/Dense1/1M/RespondBQR100.jsonl prop_files/prop_75.txt" \
    --command-name "RespondBQR100 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondBQR/Dense10/1M/RespondBQR100.jsonl prop_files/prop_76.txt" \
    --command-name "RespondBQR100 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondBQR/Dense100/1M/RespondBQR100.jsonl prop_files/prop_77.txt" \
    --command-name "RespondBQR1000 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondBQR/Dense1/1M/RespondBQR1000.jsonl prop_files/prop_78.txt" \
    --command-name "RespondBQR1000 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondBQR/Dense10/1M/RespondBQR1000.jsonl prop_files/prop_79.txt" \
    --command-name "RespondBQR1000 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondBQR/Dense100/1M/RespondBQR1000.jsonl prop_files/prop_80.txt" \
    --command-name "RespondGLB10 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondGLB/Dense1/1M/RespondGLB10.jsonl prop_files/prop_81.txt" \
    --command-name "RespondGLB10 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondGLB/Dense10/1M/RespondGLB10.jsonl prop_files/prop_82.txt" \
    --command-name "RespondGLB10 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondGLB/Dense100/1M/RespondGLB10.jsonl prop_files/prop_83.txt" \
    --command-name "RespondGLB100 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondGLB/Dense1/1M/RespondGLB100.jsonl prop_files/prop_84.txt" \
    --command-name "RespondGLB100 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondGLB/Dense10/1M/RespondGLB100.jsonl prop_files/prop_85.txt" \
    --command-name "RespondGLB100 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondGLB/Dense100/1M/RespondGLB100.jsonl prop_files/prop_86.txt" \
    --command-name "RespondGLB1000 Dense1" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondGLB/Dense1/1M/RespondGLB1000.jsonl prop_files/prop_87.txt" \
    --command-name "RespondGLB1000 Dense10" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondGLB/Dense10/1M/RespondGLB1000.jsonl prop_files/prop_88.txt" \
    --command-name "RespondGLB1000 Dense100" \
        "../build/loomrv --dense ${TESTDATA_DIR}/RespondGLB/Dense100/1M/RespondGLB1000.jsonl prop_files/prop_89.txt"