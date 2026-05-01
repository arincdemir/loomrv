# ================================================================
# Stage 1 — Build reelay apps (rybinx + ryjson)
# ================================================================
FROM ubuntu:24.04 AS reelay-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        cmake \
        ninja-build \
        clang \
        git \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# REELAY_BRANCH controls which branch/tag of doganulus/reelay is used.
# The apps: Makefile target maps to -DREELAY_BUILD_APPS=ON + cmake --install.
ARG REELAY_BRANCH=main

RUN git clone --filter=blob:none --branch "${REELAY_BRANCH}" \
        https://github.com/doganulus/reelay.git /tmp/reelay

RUN cmake -S /tmp/reelay -B /tmp/reelay/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DREELAY_BUILD_APPS=ON \
        -GNinja \
    && cmake --build /tmp/reelay/build -j$(nproc) \
    && cmake --install /tmp/reelay/build \
    && rm -rf /tmp/reelay


# ================================================================
# Stage 2 — Build loomrv (only the binaries needed by benchmarks)
# ================================================================
FROM ubuntu:24.04 AS loomrv-builder

# build-essential provides g++ (GCC 13 on 24.04, full C++20 support).
# libboost-dev is required so that benchmarks/CMakeLists.txt can satisfy
# find_package(Boost CONFIG REQUIRED) at configure time — even though we
# don't build bench_runner at all (only loomrv, count-nodes, check-grammar).
RUN apt-get update && apt-get install -y --no-install-recommends \
        ninja-build \
        build-essential \
        git \
        ca-certificates \
        libboost-dev \
        python3-pip \
    && pip3 install --break-system-packages 'cmake>=3.30' \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Copy only source tree (no data/, no reelay/, no build/)
COPY CMakeLists.txt .
COPY src/           src/
COPY include/       include/
COPY tests/         tests/
COPY benchmarks/    benchmarks/

# simdjson, cpp-peglib, and Catch2 are fetched automatically by FetchContent.
# We only build the three executables that the benchmark shell scripts invoke;
# bench_runner (which needs Boost ICL) is intentionally excluded.
RUN cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -GNinja \
    && cmake --build build \
        --target loomrv count-nodes check-grammar \
        -j$(nproc)


# ================================================================
# Stage 3 — Runtime image
# ================================================================
FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        hyperfine \
        python3 \
        python3-pip \
        python-is-python3 \
        git \
        curl \
        unzip \
        ca-certificates \
    && pip3 install --no-cache-dir --break-system-packages jsonlines \
    && rm -rf /var/lib/apt/lists/*

# ── reelay binaries ────────────────────────────────────────────
COPY --from=reelay-builder /usr/local/bin/rybinx /usr/local/bin/rybinx
COPY --from=reelay-builder /usr/local/bin/ryjson  /usr/local/bin/ryjson

# ── loomrv binaries at /app/build/ ────────────────────────────
# The benchmark scripts (run from /app/loomrv-misc/) reference ../build/loomrv
# which resolves to /app/build/loomrv — matching the repo layout exactly.
COPY --from=loomrv-builder /src/build/loomrv        /app/build/loomrv
COPY --from=loomrv-builder /src/build/count-nodes   /app/build/count-nodes
COPY --from=loomrv-builder /src/build/check-grammar /app/build/check-grammar

# ── benchmark scripts and Python tools ────────────────────────
COPY loomrv-misc/ /app/loomrv-misc/

# ── test data ──────────────────────────────────────────────────
# Downloaded from the GitHub release so the image is self-contained.
# Extracted to /app/data/fullsuite/ — scripts reference ../data/fullsuite.
RUN curl -fL \
        https://github.com/arincdemir/loomrv/releases/download/timescales-data-v1/data.zip \
        -o /tmp/data.zip \
    && unzip -q /tmp/data.zip -d /app \
    && rm /tmp/data.zip

# ── hyperfine shim ─────────────────────────────────────────────
# Placed at /usr/local/bin/hyperfine (ahead of /usr/bin/hyperfine in PATH).
# When HYPERFINE_RUNS or HYPERFINE_WARMUP are set, they replace the values
# that follow --runs / --warmup so reviewers can do a quick sanity check
# without modifying any script.
# When both are empty (default) the real hyperfine is exec'd as-is.
RUN cat > /usr/local/bin/hyperfine <<'SH'
#!/bin/bash
if [ -z "${HYPERFINE_RUNS:-}" ] && [ -z "${HYPERFINE_WARMUP:-}" ]; then
    exec /usr/bin/hyperfine "$@"
fi
ARGS=()
NEXT_IS_RUNS=false
NEXT_IS_WARMUP=false
for a in "$@"; do
    if $NEXT_IS_RUNS; then
        ARGS+=("${HYPERFINE_RUNS:-$a}")
        NEXT_IS_RUNS=false
    elif $NEXT_IS_WARMUP; then
        ARGS+=("${HYPERFINE_WARMUP:-$a}")
        NEXT_IS_WARMUP=false
    elif [ "$a" = "--runs" ]; then
        ARGS+=("$a")
        NEXT_IS_RUNS=true
    elif [ "$a" = "--warmup" ]; then
        ARGS+=("$a")
        NEXT_IS_WARMUP=true
    else
        ARGS+=("$a")
    fi
done
exec /usr/bin/hyperfine "${ARGS[@]}"
SH
RUN chmod +x /usr/local/bin/hyperfine

# ── entrypoint ─────────────────────────────────────────────────
COPY docker-entrypoint.sh /usr/local/bin/docker-entrypoint.sh
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

# ── environment ────────────────────────────────────────────────
# GIT_COMMIT is baked in at build time via --build-arg.
# The entrypoint creates a git shim that returns this value for
# `git rev-parse HEAD` calls — used by benchmark scripts to name result files.
ARG  GIT_COMMIT=unknown
ENV  GIT_COMMIT=${GIT_COMMIT}

# Default data path expected by benchmark scripts (../data/fullsuite relative
# to the /app/loomrv-misc workdir = /app/data/fullsuite).
ENV  TESTDATA_DIR=../data/fullsuite

# /usr/local/bin must precede /usr/bin so the hyperfine shim takes priority.
ENV  PATH="/usr/local/bin:${PATH}"

WORKDIR /app/loomrv-misc
ENTRYPOINT ["docker-entrypoint.sh"]
CMD  ["--help"]
