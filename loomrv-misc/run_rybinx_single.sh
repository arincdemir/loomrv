#!/usr/bin/env bash
# run_rybinx_single.sh — run rybinx with the entire properties file as one formula.
# Usage: run_rybinx_single.sh <rybinx_flags> <trace.row.bin> <props_file>
FORMULA=$(cat "$3")
rybinx $1 "$FORMULA" "$2" > /dev/null
