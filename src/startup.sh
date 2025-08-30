#!/usr/bin/env bash

set -euo pipefail

port="${1:-4848}"
cd "$(dirname "$0")/../area"

echo "Working directory: $(pwd)"
echo "Starting server on port $port"

ulimit -s 1024
ulimit -c unlimited

if [[ -e shutdown.txt ]]; then
    echo "Removing existing shutdown.txt"
    rm -f shutdown.txt
fi

while true; do
    index=1000
    while [[ -e "../log/${index}.log" ]]; do
        ((index++))
    done
    logfile="../log/${index}.log"

    date > "$logfile"
    date > boot.txt

    echo "Launching ../src/swr $port, logging to $logfile"
    "../src/swr" "$port" &>> "$logfile"
    echo "Process exited with code $?"

    if [[ -e shutdown.txt ]]; then
        echo "Detected shutdown.txt; exiting."
        rm -f shutdown.txt
        exit 0
    fi

    echo "Sleeping 10 seconds before restart"
    sleep 10
done
