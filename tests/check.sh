#!/bin/sh
set -eu
folder=$(mktemp -d)
trap 'rm -rf "$folder"' EXIT HUP INT TERM
FILE="$folder/proc.csv" ./build/monitor -p -t 0 -n 3 > "$folder/console.txt"
[ "$(wc -l < "$folder/proc.csv")" -eq 4 ]
awk -F';' 'NR>1 { if ($6 < 30 || $6 > 100 || $7 != "" || $8 != "") exit 1 }' "$folder/proc.csv"
FILE="$folder/both.csv" ./build/monitor_shared --video --hard --time=0 --number=2 > /dev/null
[ "$(wc -l < "$folder/both.csv")" -eq 3 ]
awk -F';' 'NR>1 { if ($6 != "" || $7 < 30 || $7 > 120 || $8 < 30 || $8 > 50) exit 1 }' "$folder/both.csv"
if ./build/monitor -n 0 > /dev/null 2>&1; then exit 1; fi
if ./build/monitor --time=wrong > /dev/null 2>&1; then exit 1; fi
printf '%s\n' 'Перевірки пройдено.'
