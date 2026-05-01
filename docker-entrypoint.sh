#!/usr/bin/env bash
set -euo pipefail

# ── git shim ──────────────────────────────────────────────────────────────────
# Benchmark scripts call `git rev-parse HEAD` to embed the commit hash in
# result filenames.  Inside the container there is no real git repo, so we
# intercept that call and return the GIT_COMMIT value baked in at build time
# (via --build-arg GIT_COMMIT=...).  All other git sub-commands are forwarded
# to the real git binary.
mkdir -p /tmp/git-shim
cat > /tmp/git-shim/git <<'GITSHIM'
#!/bin/sh
case "$*" in
  *rev-parse*) printf '%s\n' "${GIT_COMMIT:-unknown}" ;;
  *)           exec /usr/bin/git "$@" ;;
esac
GITSHIM
chmod +x /tmp/git-shim/git
export PATH="/tmp/git-shim:${PATH}"

# ── dispatch ──────────────────────────────────────────────────────────────────
case "${1:-}" in

  all)
    echo ""
    echo "============================================================"
    echo " loomrv — Running ALL benchmarks (dense + discrete)"
    echo " GIT_COMMIT       : ${GIT_COMMIT:-unknown}"
    echo " HYPERFINE_RUNS   : ${HYPERFINE_RUNS:-(using script default: 25)}"
    echo " HYPERFINE_WARMUP : ${HYPERFINE_WARMUP:-(using script default: 2 or 3)}"
    echo " Results dir      : /app/loomrv-misc/results/<timestamp>/"
    echo "============================================================"
    echo ""
    bash run_all_dense_benchmarks.sh
    bash run_all_discrete_benchmarks.sh
    ;;

  dense)
    echo ""
    echo "============================================================"
    echo " loomrv — Running ALL dense benchmarks"
    echo " GIT_COMMIT       : ${GIT_COMMIT:-unknown}"
    echo " HYPERFINE_RUNS   : ${HYPERFINE_RUNS:-(using script default: 25)}"
    echo " HYPERFINE_WARMUP : ${HYPERFINE_WARMUP:-(using script default: 2 or 3)}"
    echo " Results dir      : /app/loomrv-misc/results/<timestamp>/"
    echo "============================================================"
    echo ""
    exec bash run_all_dense_benchmarks.sh
    ;;

  discrete)
    echo ""
    echo "============================================================"
    echo " loomrv — Running ALL discrete benchmarks"
    echo " GIT_COMMIT       : ${GIT_COMMIT:-unknown}"
    echo " HYPERFINE_RUNS   : ${HYPERFINE_RUNS:-(using script default: 25)}"
    echo " HYPERFINE_WARMUP : ${HYPERFINE_WARMUP:-(using script default: 2 or 3)}"
    echo " Results dir      : /app/loomrv-misc/results/<timestamp>/"
    echo "============================================================"
    echo ""
    exec bash run_all_discrete_benchmarks.sh
    ;;

  --help|-h|"")
    cat <<'HELP'

Usage:
  docker run [OPTIONS] loomrv-bench <COMMAND>

Commands:
  all        Run dense + discrete benchmarks (both scripts)
  dense      Run all dense benchmarks        (run_all_dense_benchmarks.sh)
  discrete   Run all discrete benchmarks     (run_all_discrete_benchmarks.sh)

Options passed as environment variables (-e KEY=VALUE):

  HYPERFINE_RUNS
      Override the number of timed measurement runs passed to hyperfine.
      Default is the value hardcoded in each script (25).
      Use 1 for a quick sanity check:
        docker run -e HYPERFINE_RUNS=1 ... loomrv-bench dense

  HYPERFINE_WARMUP
      Override the number of warmup runs passed to hyperfine.
      Default is the value hardcoded in each script (2 or 3).
      Use 1 for a quick sanity check:
        docker run -e HYPERFINE_WARMUP=1 ... loomrv-bench dense

  GIT_COMMIT
      Overrides the commit hash embedded in result filenames at RUNTIME.
      Normally this is baked in at build time via --build-arg; set this
      env var only if you need an ad-hoc override.

Examples:

  # Full run (dense + discrete) — persist results to ./results on the host:
  docker run --rm \
    -v "$(pwd)/results:/app/loomrv-misc/results" \
    loomrv-bench all

  # Dense only:
  docker run --rm \
    -v "$(pwd)/results:/app/loomrv-misc/results" \
    loomrv-bench dense

  # Quick sanity check (1 warmup, 1 run):
  docker run --rm -e HYPERFINE_RUNS=1 -e HYPERFINE_WARMUP=1 \
    -v "$(pwd)/results:/app/loomrv-misc/results" \
    loomrv-bench dense

  # Interactive shell for ad-hoc exploration:
  docker run --rm -it loomrv-bench bash

HELP
    ;;

  *)
    # Any other argument is treated as a script name or arbitrary command.
    exec bash "$@"
    ;;

esac
