#!/usr/bin/env bash
while IFS= read -r formula; do
    # Skip empty lines
    [ -z "$formula" ] && continue
    ryjson $1 "$formula" "$2" > /dev/null
done < "$3"
