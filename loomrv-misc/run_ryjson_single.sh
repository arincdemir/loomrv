#!/usr/bin/env bash
FORMULA=$(cat "$3")
ryjson $1 "$FORMULA" "$2" > /dev/null
