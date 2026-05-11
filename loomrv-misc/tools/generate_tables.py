#!/usr/bin/env python3
"""generate_tables.py — Reproduce the paper's benchmark tables from raw
hyperfine JSON result files.

Usage:
    python3 generate_tables.py \\
        --dense-dir  ../../results/2026-05-01_23-08-06 \\
        --discrete-dir ../../results/2026-05-02_00-58-23

Each paper table is printed in a human-readable format.  Values are the
minimum wall-clock time (seconds) from hyperfine's JSON output, matching
the paper's reporting methodology.
"""

import argparse
import json
import os
import sys
from collections import OrderedDict


# ── helpers ──────────────────────────────────────────────────────────────

def load_results(directory):
    """Load all JSON result files in *directory* into a dict keyed by filename."""
    data = {}
    if not directory or not os.path.isdir(directory):
        return data
    for fname in sorted(os.listdir(directory)):
        if not fname.endswith(".json"):
            continue
        path = os.path.join(directory, fname)
        with open(path) as f:
            data[fname] = json.load(f)
    return data


def find_file(data, prefix):
    """Return the parsed JSON for the first file whose name starts with *prefix*.

    Handles two filename conventions:
      - Original results  : ``script.sh".COMMIT.results.json``  (stray quote)
      - Docker-generated  : ``script.COMMIT.results.json``      (`basename` strips .sh)

    The function tries matching with the literal prefix first, then falls back
    to a version with ``.sh`` stripped so both conventions are supported.
    When using the stripped prefix, we additionally verify the next character
    is ``"`` or ``.`` to avoid false positives (e.g. ``foo-bar`` matching
    ``foo-bar-extra``).
    """

    def _matches(name, pfx, exact):
        """Check if *name* starts with *pfx*.

        When *exact* is False (i.e. we stripped ``.sh`` from the prefix),
        require the character immediately after the prefix to be a boundary
        character (``.`` or ``"`` or end-of-string) so that ``foo-bar`` does
        not match ``foo-bar-extra``.
        """
        if not name.startswith(pfx):
            return False
        if exact:
            return True
        # Check boundary: next char must be '.', '"', or end of string
        rest = name[len(pfx):]
        return not rest or rest[0] in ('.', '"')

    for fname, content in data.items():
        # Strip escaped quotes from filenames for matching (original results)
        clean = fname.replace('"', "")

        # Try exact prefix first
        if _matches(fname, prefix, exact=True) or _matches(clean, prefix, exact=True):
            return content

        # Try with .sh stripped
        if prefix.endswith(".sh"):
            stripped = prefix[:-3]
            if _matches(fname, stripped, exact=False) or _matches(clean, stripped, exact=False):
                return content

    return None


def get_min(results_json, command_name):
    """Extract the 'min' time for a given command name from a hyperfine JSON."""
    if results_json is None:
        return None
    for r in results_json.get("results", []):
        if r["command"] == command_name:
            return r["min"]
    return None


def fmt(val, decimals=3):
    """Format a float or return '—' if missing."""
    if val is None:
        return "—"
    return f"{val:.{decimals}f}"


def fmt_speedup(baseline, value):
    """Compute and format a speedup ratio."""
    if baseline is None or value is None or value == 0:
        return "—"
    return f"{baseline / value:.1f}×"


# ── Benchmarks list (30 timescales formulas) ─────────────────────────────

BENCHMARKS = [
    "AbsentAQ10", "AbsentAQ100", "AbsentAQ1000",
    "AbsentBQR10", "AbsentBQR100", "AbsentBQR1000",
    "AbsentBR10", "AbsentBR100", "AbsentBR1000",
    "AlwaysAQ10", "AlwaysAQ100", "AlwaysAQ1000",
    "AlwaysBQR10", "AlwaysBQR100", "AlwaysBQR1000",
    "AlwaysBR10", "AlwaysBR100", "AlwaysBR1000",
    "RecurBQR10", "RecurBQR100", "RecurBQR1000",
    "RecurGLB10", "RecurGLB100", "RecurGLB1000",
    "RespondBQR10", "RespondBQR100", "RespondBQR1000",
    "RespondGLB10", "RespondGLB100", "RespondGLB1000",
]

# Node compression ratios (from count-nodes, hardcoded)
CSE_SCENARIOS = OrderedDict([
    ("Best-case shared",  3.57),
    ("Nested best-case",  4.09),
    ("Worst-case unique", 1.67),
    ("Nested worst-case", 1.31),
])

# Mapping: scenario display name → filename fragment
CSE_FILE_FRAGMENTS = {
    "Best-case shared":  "best_case_shared_core",
    "Nested best-case":  "nested_best_case",
    "Worst-case unique": "worst_case_unique_leaves",
    "Nested worst-case": "nested_worst_case",
}


# ── Table generators ────────────────────────────────────────────────────

def print_table_header(title):
    print()
    print("=" * 80)
    print(f"  {title}")
    print("=" * 80)


def table_cse(data, time_label, file_prefix_json, file_prefix_bin,
              cmd_reelay_json, cmd_loomrv_seq_json, cmd_loomrv_multi_json,
              cmd_reelay_bin, cmd_loomrv_seq_bin, cmd_loomrv_multi_bin):
    """Print a CSE sensitivity table (Tables 2/3)."""

    header = f"{'Scenario':<20s} {'Comp.':>6s}  {'Reelay Seq':>11s} {'LoomRV Seq':>11s} {'LoomRV Multi':>13s}"
    print()
    print("  JSON feeder")
    print(f"  {header}")
    print(f"  {'-'*len(header)}")

    for scenario, comp in CSE_SCENARIOS.items():
        frag = CSE_FILE_FRAGMENTS[scenario]
        prefix = f"{file_prefix_json}{frag}"
        content = find_file(data, prefix)
        reelay = get_min(content, cmd_reelay_json)
        seq = get_min(content, cmd_loomrv_seq_json)
        multi = get_min(content, cmd_loomrv_multi_json)
        print(f"  {scenario:<20s} {comp:>5.2f}×  {fmt(reelay):>11s} {fmt(seq):>11s} {fmt(multi):>13s}")

    print()
    print("  Binary feeder")
    print(f"  {header}")
    print(f"  {'-'*len(header)}")

    for scenario, comp in CSE_SCENARIOS.items():
        frag = CSE_FILE_FRAGMENTS[scenario]
        prefix = f"{file_prefix_bin}{frag}"
        content = find_file(data, prefix)
        reelay = get_min(content, cmd_reelay_bin)
        seq = get_min(content, cmd_loomrv_seq_bin)
        multi = get_min(content, cmd_loomrv_multi_bin)
        print(f"  {scenario:<20s} {comp:>5.2f}×  {fmt(reelay):>11s} {fmt(seq):>11s} {fmt(multi):>13s}")


def table_cse_discrete(data):
    """Table 2: CSE sensitivity, discrete."""
    print_table_header("Table 2 — Impact of Cross-Property Sharing (Discrete)")
    table_cse(
        data,
        time_label="discrete",
        file_prefix_json="bench_discrete_",
        file_prefix_bin="bench_binary_discrete_",
        cmd_reelay_json="ryjson (Sequential)",
        cmd_loomrv_seq_json="loomrv (Sequential)",
        cmd_loomrv_multi_json="loomrv (Multi-Property)",
        cmd_reelay_bin="rybinx (Sequential)",
        cmd_loomrv_seq_bin="loomrv (Sequential, binary)",
        cmd_loomrv_multi_bin="loomrv (Multi-Property, binary)",
    )


def table_cse_dense(data):
    """Table 3: CSE sensitivity, dense."""
    print_table_header("Table 3 — Impact of Cross-Property Sharing (Dense)")
    table_cse(
        data,
        time_label="dense",
        file_prefix_json="bench_",
        file_prefix_bin="bench_binary_",
        cmd_reelay_json="ryjson (Sequential)",
        cmd_loomrv_seq_json="loomrv (Sequential)",
        cmd_loomrv_multi_json="loomrv (Multi-Property)",
        cmd_reelay_bin="rybinx (Sequential)",
        cmd_loomrv_seq_bin="loomrv (Sequential, binary)",
        cmd_loomrv_multi_bin="loomrv (Multi-Property, binary)",
    )


def table_single_discrete(data):
    """Table 4: Single-property discrete-time."""
    print_table_header("Table 4 — Single-Property Discrete-Time Monitoring")

    loomrv_json = find_file(data, "loomrv-discrete-benchmark-json.sh")
    reelay_json = find_file(data, "ryjson-discrete-benchmark.sh")
    loomrv_bin  = find_file(data, "loomrv-discrete-benchmark-bin.sh")
    reelay_bin  = find_file(data, "rybinx-discrete-benchmark.sh")

    header = f"{'Benchmark':<18s} {'Reelay':>8s} {'LoomRV':>8s}  {'Reelay':>8s} {'LoomRV':>8s}"
    sub    = f"{'':18s} {'JSON':>8s} {'JSON':>8s}  {'Binary':>8s} {'Binary':>8s}"
    print(f"  {header}")
    print(f"  {sub}")
    print(f"  {'-'*len(header)}")

    for bench in BENCHMARKS:
        rj = get_min(reelay_json, bench)
        lj = get_min(loomrv_json, bench)
        rb = get_min(reelay_bin, bench)
        lb = get_min(loomrv_bin, bench)
        print(f"  {bench:<18s} {fmt(rj):>8s} {fmt(lj):>8s}  {fmt(rb):>8s} {fmt(lb):>8s}")


def table_single_dense(data, feeder_type, table_num, table_label,
                       loomrv_prefix, reelay_prefix):
    """Tables 5/6: Single-property dense-time."""
    print_table_header(f"Table {table_num} — {table_label}")

    loomrv = find_file(data, loomrv_prefix)
    reelay = find_file(data, reelay_prefix)

    densities = ["Dense1", "Dense10", "Dense100"]
    header_parts = ["Benchmark".ljust(18)]
    for d in densities:
        header_parts.append(f"{'Reelay':>8s} {'LoomRV':>8s}")
    header = "  ".join(header_parts)

    sub_parts = ["".ljust(18)]
    for d in densities:
        sub_parts.append(f"{d:>8s} {d:>8s}")
    sub = "  ".join(sub_parts)

    print(f"  {header}")
    print(f"  {'-'*len(header)}")

    for bench in BENCHMARKS:
        parts = [f"{bench:<18s}"]
        for density in densities:
            cmd = f"{bench} {density}"
            r = get_min(reelay, cmd)
            l = get_min(loomrv, cmd)
            parts.append(f"{fmt(r):>8s} {fmt(l):>8s}")
        print(f"  {'  '.join(parts)}")


def table_multi_property(data_dense, data_discrete):
    """Tables 6/7: Multi-property monitoring performance."""

    # ── Discrete (Table 6 in the paper) ──
    print_table_header("Table 6 — Multi-Property Monitoring (Discrete)")

    configs_discrete = [
        ("Reelay-Sequential (JSON)",    "ryjson-discrete-benchmark-multi.sh",        "ryjson_Discrete_Sequential_30"),
        ("Reelay-AND (JSON)",           "ryjson-discrete-benchmark-multi-and.sh",    "ryjson_Discrete_Combined_AND_30"),
        ("LoomRV Sequential (JSON)",    "loomrv-discrete-benchmark-single-seq.sh",   "loomrv_Discrete_Sequential_30"),
        ("LoomRV-AND (JSON)",           "loomrv-discrete-benchmark-multi-and.sh",    "Combined_Discrete_MultiProperty_AND_30"),
        ("LoomRV Multi (JSON)",         "loomrv-discrete-benchmark-multi.sh",        "Combined_Discrete_MultiProperty_30"),
    ]
    configs_discrete_bin = [
        ("Reelay-Sequential (binary)",  "rybinx-discrete-benchmark-multi.sh",        "rybinx_Discrete_Sequential_30"),
        ("Reelay-AND (binary)",         "rybinx-discrete-benchmark-multi-and.sh",    "rybinx_Discrete_Combined_AND_30"),
        ("LoomRV Sequential (binary)",  "loomrv-discrete-benchmark-single-binary.sh","loomrv_Discrete_Sequential_Binary_30"),
        ("LoomRV-AND (binary)",         "loomrv-discrete-benchmark-multi-and-binary.sh","Combined_Discrete_MultiProperty_AND_30_binary"),
        ("LoomRV Multi (binary)",       "loomrv-discrete-benchmark-multi-binary.sh", "Combined_Discrete_MultiProperty_30_binary"),
    ]

    header = f"  {'Configuration':<35s} {'Time (s)':>10s} {'Speedup':>10s}"
    print(header)
    print(f"  {'-'*len(header.strip())}")

    # JSON section
    print(f"  {'JSON feeder':35s}")
    baseline_json = None
    for label, file_prefix, cmd_name in configs_discrete:
        content = find_file(data_discrete, file_prefix)
        val = get_min(content, cmd_name)
        if "Sequential" in label and "LoomRV" not in label:
            baseline_json = val
        speedup = fmt_speedup(baseline_json, val) if baseline_json else "—"
        print(f"  {label:<35s} {fmt(val):>10s} {speedup:>10s}")

    # Binary section
    print(f"  {'Binary feeder':35s}")
    baseline_bin = None
    for label, file_prefix, cmd_name in configs_discrete_bin:
        content = find_file(data_discrete, file_prefix)
        val = get_min(content, cmd_name)
        if "Sequential" in label and "LoomRV" not in label:
            baseline_bin = val
        speedup = fmt_speedup(baseline_bin, val) if baseline_bin else "—"
        print(f"  {label:<35s} {fmt(val):>10s} {speedup:>10s}")

    # ── Dense (Table 7 in the paper) ──
    print_table_header("Table 7 — Multi-Property Monitoring (Dense)")

    configs_dense = [
        ("Reelay-Sequential (JSON)",    "ryjson-benchmark-dense-multi.sh",        "ryjson_Sequential_30"),
        ("Reelay-AND (JSON)",           "ryjson-benchmark-dense-multi-and.sh",    "ryjson_Combined_AND_30"),
        ("LoomRV Sequential (JSON)",    "loomrv-benchmark-dense-single-seq.sh",   "loomrv_Sequential_30"),
        ("LoomRV-AND (JSON)",           "loomrv-benchmark-dense-multi-and.sh",    "Combined_MultiProperty_AND_30"),
        ("LoomRV Multi (JSON)",         "loomrv-benchmark-dense-multi.sh",        "Combined_MultiProperty_30"),
    ]
    configs_dense_bin = [
        ("Reelay-Sequential (binary)",  "rybinx-benchmark-dense-multi.sh",        "rybinx_Sequential_30"),
        ("Reelay-AND (binary)",         "rybinx-benchmark-dense-multi-and.sh",    "rybinx_Combined_AND_30"),
        ("LoomRV Sequential (binary)",  "loomrv-benchmark-dense-single-binary.sh","loomrv_Sequential_Binary_30"),
        ("LoomRV-AND (binary)",         "loomrv-benchmark-dense-multi-and-binary.sh","Combined_MultiProperty_AND_30_binary"),
        ("LoomRV Multi (binary)",       "loomrv-benchmark-dense-multi-binary.sh", "Combined_MultiProperty_30_binary"),
    ]

    print(header)
    print(f"  {'-'*len(header.strip())}")

    print(f"  {'JSON feeder':35s}")
    baseline_json = None
    for label, file_prefix, cmd_name in configs_dense:
        content = find_file(data_dense, file_prefix)
        val = get_min(content, cmd_name)
        if "Sequential" in label and "LoomRV" not in label:
            baseline_json = val
        speedup = fmt_speedup(baseline_json, val) if baseline_json else "—"
        print(f"  {label:<35s} {fmt(val):>10s} {speedup:>10s}")

    print(f"  {'Binary feeder':35s}")
    baseline_bin = None
    for label, file_prefix, cmd_name in configs_dense_bin:
        content = find_file(data_dense, file_prefix)
        val = get_min(content, cmd_name)
        if "Sequential" in label and "LoomRV" not in label:
            baseline_bin = val
        speedup = fmt_speedup(baseline_bin, val) if baseline_bin else "—"
        print(f"  {label:<35s} {fmt(val):>10s} {speedup:>10s}")


# ── main ─────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="Reproduce the paper's benchmark tables from raw hyperfine JSON results.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""Examples:
  python3 generate_tables.py \\
      --dense-dir ../../results/2026-05-01_23-08-06 \\
      --discrete-dir ../../results/2026-05-02_00-58-23
""",
    )
    parser.add_argument(
        "--dense-dir",
        help="Path to the timestamped results directory containing dense-time results.",
    )
    parser.add_argument(
        "--discrete-dir",
        help="Path to the timestamped results directory containing discrete-time results.",
    )
    args = parser.parse_args()

    if not args.dense_dir and not args.discrete_dir:
        parser.error("At least one of --dense-dir or --discrete-dir must be provided.")

    data_dense = load_results(args.dense_dir)
    data_discrete = load_results(args.discrete_dir)

    if not data_dense and not data_discrete:
        print("Error: No JSON result files found in the specified directories.", file=sys.stderr)
        sys.exit(1)

    # ── Tables 2 & 3: CSE sensitivity ──
    if data_discrete:
        table_cse_discrete(data_discrete)
    if data_dense:
        table_cse_dense(data_dense)

    # ── Table 4: Single-property discrete ──
    if data_discrete:
        table_single_discrete(data_discrete)

    # ── Table 5: Single-property dense (JSON) ──
    if data_dense:
        table_single_dense(
            data_dense,
            feeder_type="JSON",
            table_num=5,
            table_label="Single-Property Dense-Time (Reelay JSON vs LoomRV JSON)",
            loomrv_prefix="loomrv-benchmark-dense.sh",
            reelay_prefix="ryjson-benchmark-dense.sh",
        )
        # ── Table 6*: Single-property dense (binary) — paper calls this Table 6 ──
        # Note: paper's Table 6 is dense-binary single-property. The multi-property
        # discrete table is labeled separately.
        table_single_dense(
            data_dense,
            feeder_type="Binary",
            table_num="5b",
            table_label="Single-Property Dense-Time (Reelay Binary vs LoomRV Binary)",
            loomrv_prefix="loomrv-benchmark-dense-binary.sh",
            reelay_prefix="rybinx-benchmark-dense.sh",
        )

    # ── Tables 6 & 7: Multi-property ──
    if data_dense or data_discrete:
        table_multi_property(data_dense, data_discrete)

    print()
    print("Done. All values are minimum wall-clock times (seconds) from hyperfine.")


if __name__ == "__main__":
    main()
