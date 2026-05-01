#!/usr/bin/env python3
import argparse
import json
import random
import os

def generate_trace(length, filename, step_mode='random', max_step=8, with_s=False):
    current_time = 0
    
    with open(filename, 'w') as f:
        for _ in range(length):
            # Create the dictionary in the exact order requested
            event = {
                "time": current_time,
                "q": random.choice([True, False]),
                "p": random.choice([True, False]),
                "r": random.choice([True, False])
            }
            if with_s:
                event["s"] = random.choice([True, False])
            # Write to JSONL
            f.write(json.dumps(event) + "\n")
            
            # Determine the next time step
            if step_mode == 'constant':
                # Dense time tracing, increments exactly by 1 each time
                current_time += 1
            elif step_mode == 'random':
                # Uniform random step: realistically simulates events arriving at variable intervals
                current_time += random.randint(1, max_step)

    print(f"Generated trace with {length} events -> {filename}")

def main():
    parser = argparse.ArgumentParser(description="Generate synthetic JSONL traces with p, q, r boolean states.")
    parser.add_argument("--length", "-n", type=int, default=10000, help="Number of events in the trace.")
    parser.add_argument("--out", "-o", type=str, default="synthetic_trace.jsonl", help="Output file path.")
    parser.add_argument("--step-mode", "-s", type=str, choices=["constant", "random"], default="random",
                        help="How time advances between events. 'constant': +1, 'random': +1 to max_step.")
    parser.add_argument("--max-step", type=int, default=3, help="Maximum time leap between events for 'random' mode.")
    parser.add_argument("--with-s", action="store_true", help="Include boolean state 's' in the generated trace.")
    
    args = parser.parse_args()

    # Ensure output directory exists if provided
    outdir = os.path.dirname(args.out)
    if outdir:
        os.makedirs(outdir, exist_ok=True)
        
    generate_trace(args.length, args.out, args.step_mode, args.max_step, args.with_s)

if __name__ == "__main__":
    main()
