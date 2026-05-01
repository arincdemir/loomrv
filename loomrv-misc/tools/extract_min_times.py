#!/usr/bin/env python3
import json
import sys
import os

def process_file(file_path):
    try:
        with open(file_path, 'r') as f:
            data = json.load(f)
            
        print(f"\n=== Results from: {os.path.basename(file_path)} ===")
        print(f"{'Benchmark Name':<50} | {'Min Time (s)':<15}")
        print("-" * 70)
        
        for result in data.get('results', []):
            command_name = result.get('command', 'Unknown')
            min_time = result.get('min', 0.0)
            
            # Format time to 5 decimal places for precision
            print(f"{command_name:<50} | {min_time:.5f}")
            
    except Exception as e:
        print(f"Error reading {file_path}: {e}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 extract_min_times.py <path/to/results.json> [more_results.json...]")
        print("       python3 extract_min_times.py results/2026-04-19_17-06-02/")
        sys.exit(1)

    for arg in sys.argv[1:]:
        if os.path.isfile(arg):
            if arg.endswith('.json'):
                process_file(arg)
        elif os.path.isdir(arg):
            # Process all json files in the directory
            for root, _, files in os.walk(arg):
                for file in sorted(files):
                    if file.endswith('.json'):
                        process_file(os.path.join(root, file))
        else:
            print(f"Path not found: {arg}")

if __name__ == "__main__":
    main()