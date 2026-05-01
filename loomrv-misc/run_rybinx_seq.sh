#!/usr/bin/env bash
# run_rybinx_seq.sh — run rybinx sequentially for every formula in a properties file.
# Usage: run_rybinx_seq.sh <rybinx_flags> <trace.row.bin> <props_file>
while IFS= read -r formula; do
    # Skip empty lines
    [ -z "$formula" ] && continue
    rybinx $1 "$formula" "$2" > /dev/null
done < "$3"
