#!/usr/bin/env python3
"""Validate the versioned manual and synchronize it to the GitHub Wiki checkout."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIRECTORY = REPOSITORY_ROOT / "docs" / "wiki"
DEFAULT_DESTINATION = REPOSITORY_ROOT / "loomrv.wiki"
MARKDOWN_LINK = re.compile(r"(?<!!)\[([^\]]+)\]\(([^)]+)\)")


class DocumentationError(RuntimeError):
    pass


def source_pages() -> list[Path]:
    return sorted(SOURCE_DIRECTORY.glob("*.md"))


def split_anchor(target: str) -> tuple[str, str]:
    path, separator, anchor = target.partition("#")
    suffix = f"#{anchor}" if separator else ""
    return path, suffix


def is_external(target: str) -> bool:
    return target.startswith(("http://", "https://", "mailto:", "#"))


def transform_for_github_wiki(text: str) -> str:
    def replace(match: re.Match[str]) -> str:
        label, target = match.groups()
        if is_external(target):
            return match.group(0)
        path, anchor = split_anchor(target)
        if path.endswith(".md"):
            path = path[:-3]
        return f"[{label}]({path}{anchor})"

    return MARKDOWN_LINK.sub(replace, text)


def validate_internal_links(pages: list[Path]) -> list[str]:
    errors: list[str] = []
    known_names = {page.name for page in pages}
    known_stems = {page.stem for page in pages}

    for page in pages:
        text = page.read_text(encoding="utf-8")
        for _, target in MARKDOWN_LINK.findall(text):
            if is_external(target):
                continue
            path, _ = split_anchor(target)
            if path in known_stems:
                errors.append(
                    f"{page.name}: canonical wiki link must include .md: {target}"
                )
            elif path.endswith(".md") and Path(path).name not in known_names:
                errors.append(f"{page.name}: missing linked page: {target}")
    return errors


def cmake_project_version() -> str:
    cmake = (REPOSITORY_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    match = re.search(
        r"project\s*\([^)]*\bVERSION\s+([0-9]+(?:\.[0-9]+){1,3})",
        cmake,
        flags=re.DOTALL,
    )
    if not match:
        raise DocumentationError("Could not read the project version from CMakeLists.txt")
    return match.group(1)


def validate_required_content(pages: list[Path]) -> list[str]:
    errors: list[str] = []
    combined = "\n".join(page.read_text(encoding="utf-8") for page in pages)
    getting_started = (SOURCE_DIRECTORY / "Getting-Started.md").read_text(
        encoding="utf-8"
    )
    main_source = (REPOSITORY_ROOT / "src" / "main.cpp").read_text(encoding="utf-8")

    source_flags = set(
        re.findall(r'\{"([a-z][a-z-]+)",\s*(?:no_argument|required_argument)', main_source)
    )
    documented_flags = set(re.findall(r"--([a-z][a-z-]+)", getting_started))
    missing_flags = sorted(source_flags - documented_flags)
    if missing_flags:
        errors.append(
            "Getting-Started.md does not document CLI flags: " + ", ".join(missing_flags)
        )

    expected_output = (REPOSITORY_ROOT / "examples" / "expected-discrete.txt").read_text(
        encoding="utf-8"
    ).strip()
    for page_name in ("Home.md", "Getting-Started.md"):
        text = (SOURCE_DIRECTORY / page_name).read_text(encoding="utf-8")
        if expected_output not in text:
            errors.append(f"{page_name}: bundled example output is out of date")

    required_snippets = (
        "cmake -S . -B build -DCMAKE_BUILD_TYPE=Release",
        "cmake --build build",
        "ctest --test-dir build --output-on-failure",
        "./build/loomrv --version",
        "./build/count-nodes --json",
        "docker build -t loomrv-bench .",
    )
    for snippet in required_snippets:
        if snippet not in combined:
            errors.append(f"Manual is missing required command example: {snippet}")

    version = cmake_project_version()
    if f"loomrv {version}" not in combined:
        errors.append(f"Manual does not show the CMake project version: loomrv {version}")

    return errors


def run_checked(command: list[str]) -> subprocess.CompletedProcess[str]:
    try:
        return subprocess.run(
            command,
            cwd=REPOSITORY_ROOT,
            check=True,
            capture_output=True,
            text=True,
        )
    except (OSError, subprocess.CalledProcessError) as error:
        raise DocumentationError(f"Command failed: {' '.join(command)}\n{error}") from error


def validate_built_commands(build_directory: Path) -> list[str]:
    errors: list[str] = []
    loomrv = build_directory / "loomrv"
    count_nodes = build_directory / "count-nodes"
    for executable in (loomrv, count_nodes):
        if not executable.is_file():
            errors.append(f"Missing built executable: {executable}")
    if errors:
        return errors

    expected_version = f"loomrv {cmake_project_version()}\n"
    version = run_checked([str(loomrv), "--version"])
    if version.stdout != expected_version or version.stderr:
        errors.append("loomrv --version does not match the documented CMake version")

    example = run_checked(
        [
            str(loomrv),
            "--discrete",
            "--print",
            "examples/trace.jsonl",
            "examples/properties.txt",
        ]
    )
    expected_output = (REPOSITORY_ROOT / "examples" / "expected-discrete.txt").read_text(
        encoding="utf-8"
    )
    if example.stdout != expected_output or example.stderr:
        errors.append("The documented CLI example differs from examples/expected-discrete.txt")

    report = run_checked([str(count_nodes), "--json", "examples/properties.txt"])
    try:
        parsed = json.loads(report.stdout)
    except json.JSONDecodeError as error:
        errors.append(f"count-nodes --json did not produce valid JSON: {error}")
    else:
        expected_counts = {
            "schema_version": 1,
            "property_count": 3,
            "independent_node_count": 7,
            "shared_dag_node_count": 5,
            "eliminated_node_count": 2,
            "property_root_count": 3,
        }
        for key, expected in expected_counts.items():
            if parsed.get(key) != expected:
                errors.append(
                    f"count-nodes example has {key}={parsed.get(key)!r}; expected {expected!r}"
                )
        if report.stderr:
            errors.append("count-nodes --json wrote diagnostics during a successful run")

    return errors


def validate(build_directory: Path | None) -> None:
    pages = source_pages()
    errors = validate_internal_links(pages)
    errors.extend(validate_required_content(pages))
    if build_directory is not None:
        errors.extend(validate_built_commands(build_directory))

    if errors:
        raise DocumentationError("\n".join(f"- {error}" for error in errors))
    print(f"Validated {len(pages)} canonical wiki pages.")


def resolve_destination(value: str) -> Path:
    destination = Path(value)
    return destination if destination.is_absolute() else REPOSITORY_ROOT / destination


def synchronize(destination: Path, check_only: bool) -> None:
    pages = source_pages()
    expected_names = {page.name for page in pages}

    if not destination.exists():
        if check_only:
            raise DocumentationError(f"Wiki checkout does not exist: {destination}")
        destination.mkdir(parents=True)

    unexpected = sorted(
        page.name for page in destination.glob("*.md") if page.name not in expected_names
    )
    if unexpected:
        raise DocumentationError(
            "Unexpected live-only wiki pages were left untouched: " + ", ".join(unexpected)
        )

    drift: list[str] = []
    for source in pages:
        expected = transform_for_github_wiki(source.read_text(encoding="utf-8"))
        target = destination / source.name
        actual = target.read_text(encoding="utf-8") if target.exists() else None
        if actual == expected:
            continue
        drift.append(source.name)
        if not check_only:
            target.write_text(expected, encoding="utf-8")

    if check_only and drift:
        raise DocumentationError("GitHub Wiki checkout is out of sync: " + ", ".join(drift))
    if drift:
        print("Synchronized: " + ", ".join(drift))
    else:
        print("GitHub Wiki checkout is synchronized.")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    validate_parser = subparsers.add_parser("validate", help="validate canonical pages")
    validate_parser.add_argument(
        "--build-dir",
        help="also exercise documented commands using executables in this build directory",
    )

    sync_parser = subparsers.add_parser("sync", help="publish canonical pages to a wiki checkout")
    sync_parser.add_argument("--destination", default=str(DEFAULT_DESTINATION))
    mode = sync_parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true", help="report drift without writing")
    mode.add_argument("--write", action="store_true", help="update known wiki pages")

    args = parser.parse_args()
    try:
        if args.command == "validate":
            build_directory = (
                resolve_destination(args.build_dir) if args.build_dir is not None else None
            )
            validate(build_directory)
        else:
            synchronize(resolve_destination(args.destination), check_only=args.check)
    except DocumentationError as error:
        print(f"Documentation check failed:\n{error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
