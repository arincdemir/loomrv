#!/usr/bin/env python3
import argparse
import os
import subprocess
import sys

def generate_best_case(n):
    """
    Generates formulas with a large shared core. The core subformula is identical
    and will be perfectly deduplicated. Only the outermost bounds change.
    Using only props: {p}, {q}, {r}
    """
    formulas = []
    core = "historically((once[:10]{q}) -> ((not{p}) since {r}))"
    for i in range(1, n + 1):
        formulas.append(f"({core}) or (once[:{i}]{{p}})")
    return formulas

def generate_worst_case(n):
    """
    Changes bounds at the leaf level. Because the leaf node's key changes,
    every ancestor node's key changes up to the root, guaranteeing near-zero
    deduplication (only the atoms {p}, {q}, {r} are shared).
    """
    formulas = []
    for i in range(1, n + 1):
        formulas.append(f"historically(({{r}} and once[:{i}]{{q}}) -> ((not{{p}}) since[{i}:{i+10}] {{r}}))")
    return formulas

def generate_nested_best_case(n):
    """
    Successively wraps the previous formula. F_i fully contains F_{i-1}.
    This means the AST for F_i only adds to the AST of F_{i-1}, leading 
    to perfect incremental deduplication.
    """
    formulas = []
    current = "{p}"
    props = ["{p}", "{q}", "{r}"]
    for i in range(1, n + 1):
        prop = props[i % 3]
        current = f"({current} or {prop})"
        formulas.append(f"historically({current})")
    return formulas

def generate_nested_worst_case(n):
    """
    Builds deeply nested formulas, but the VERY INNERMOST bound varies by formula.
    Because the deepest subformula changes, none of the wrapping layers can be 
    deduplicated across formulas.
    """
    formulas = []
    props = ["{p}", "{q}", "{r}"]
    for i in range(1, n + 1):
        # Varying the leaf invalidates the entire parent chain
        current = f"once[:{i}]{{p}}"
        for j in range(i):
            prop = props[j % 3]
            current = f"({current} and {prop})"
        formulas.append(f"historically({current})")
    return formulas

def main():
    parser = argparse.ArgumentParser(description="Generate synthetic PTL formulas for deduplication benchmarking.")
    parser.add_argument("--count", "-n", type=int, default=100, help="Number of formulas to generate per category.")
    parser.add_argument("--outdir", "-o", type=str, default="synthetic_formulas", help="Output directory for the generated files.")
    parser.add_argument("--checker", "-c", type=str, default="./build/check-grammar", help="Path to the check-grammar executable.")
    
    args = parser.parse_args()

    os.makedirs(args.outdir, exist_ok=True)

    scenarios = {
        "best_case_shared_core.txt": generate_best_case(args.count),
        "worst_case_unique_leaves.txt": generate_worst_case(args.count),
        "nested_best_case.txt": generate_nested_best_case(args.count),
        "nested_worst_case.txt": generate_nested_worst_case(args.count)
    }

    print(f"Generating formula sets (N={args.count}) into '{args.outdir}/' ...")
    
    if not os.path.isfile(args.checker):
        print(f"Warning: Grammar checker not found at '{args.checker}'. Skipping validation.")
        checker_path = None
    else:
        checker_path = args.checker
        print(f"Using grammar checker at: {checker_path}")

    for filename, formulas in scenarios.items():
        filepath = os.path.join(args.outdir, filename)
        
        if checker_path:
            for formula in formulas:
                res = subprocess.run([checker_path, formula], capture_output=True, text=True)
                if res.returncode != 0:
                    print(f"\nError: Generated invalid formula '{formula}'")
                    print(f"Checker error:\n{res.stderr.strip()}\n{res.stdout.strip()}")
                    sys.exit(1)

        with open(filepath, "w") as f:
            for formula in formulas:
                f.write(formula + "\n")
        print(f"  -> Validated and created {filepath}")

if __name__ == "__main__":
    main()
