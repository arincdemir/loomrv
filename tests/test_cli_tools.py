#!/usr/bin/env python3
"""Integration checks for LoomRV's standalone command-line utilities."""

import argparse
import json
import math
import subprocess
import tempfile
from pathlib import Path


NODE_TYPES = {
    "proposition",
    "and",
    "or",
    "not",
    "implies",
    "once",
    "historically",
    "since",
    "test",
}


def run(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(args, check=True, capture_output=True, text=True)


def write_formulas(directory: Path, name: str, formulas: list[str]) -> Path:
    path = directory / name
    path.write_text("\n".join(formulas) + ("\n" if formulas else ""), encoding="utf-8")
    return path


def check_report(
    executable: str,
    formula_file: Path,
    *,
    properties: int,
    independent: int,
    shared: int,
    eliminated: int,
    roots: int,
    ratio: float | None,
    node_counts: dict[str, int],
) -> None:
    completed = run(executable, "--json", str(formula_file))
    assert completed.stderr == "", completed.stderr
    report = json.loads(completed.stdout)
    assert report["schema_version"] == 1
    assert report["property_count"] == properties
    assert report["independent_node_count"] == independent
    assert report["shared_dag_node_count"] == shared
    assert report["eliminated_node_count"] == eliminated
    assert report["property_root_count"] == roots
    assert set(report["node_type_counts"]) == NODE_TYPES
    assert report["node_type_counts"] == {
        node_type: node_counts.get(node_type, 0) for node_type in NODE_TYPES
    }
    if ratio is None:
        assert report["compression_ratio"] is None
    else:
        assert math.isclose(report["compression_ratio"], ratio)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--loomrv", required=True)
    parser.add_argument("--count-nodes", required=True)
    parser.add_argument("--expected-version", required=True)
    args = parser.parse_args()

    version = run(args.loomrv, "--version")
    assert version.stdout == f"loomrv {args.expected_version}\n"
    assert version.stderr == ""

    with tempfile.TemporaryDirectory() as tmp:
        directory = Path(tmp)
        shared = write_formulas(
            directory, "shared.txt", ["once({p})", "historically({p})"]
        )
        unshared = write_formulas(directory, "unshared.txt", ["{p}", "{q}"])
        duplicate = write_formulas(
            directory, "duplicate.txt", ["once({p})", "once({p})"]
        )
        empty = write_formulas(directory, "empty.txt", [])

        check_report(
            args.count_nodes,
            shared,
            properties=2,
            independent=4,
            shared=3,
            eliminated=1,
            roots=2,
            ratio=4 / 3,
            node_counts={"proposition": 1, "once": 1, "historically": 1},
        )
        check_report(
            args.count_nodes,
            unshared,
            properties=2,
            independent=2,
            shared=2,
            eliminated=0,
            roots=2,
            ratio=1,
            node_counts={"proposition": 2},
        )
        check_report(
            args.count_nodes,
            duplicate,
            properties=2,
            independent=4,
            shared=2,
            eliminated=2,
            roots=2,
            ratio=2,
            node_counts={"proposition": 1, "once": 1},
        )
        check_report(
            args.count_nodes,
            empty,
            properties=0,
            independent=0,
            shared=0,
            eliminated=0,
            roots=0,
            ratio=None,
            node_counts={},
        )

        text = run(args.count_nodes, str(shared)).stdout
        expected_text = (
            f"=== Node Count Report: {shared} ===\n"
            "Sequential AST Nodes (Sum) : 4\n"
            "Deduplicated AST Nodes     : 3\n"
            "Reduction                  : 1 nodes (25%)\n"
            "AST Size Compression       : 1.33333x smaller\n"
        )
        assert text == expected_text


if __name__ == "__main__":
    main()
