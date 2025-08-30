#!/usr/bin/env bash

# Fail hard on script errors, but allow diagnostics to be best-effort
set -euo pipefail

port="${1:-4848}"
cd "$(dirname "$0")/../area"

echo "Working directory: $(pwd)"
echo "Starting server on port $port"

ulimit -c unlimited

# The email address to send crash alerts to.
ALERT_EMAIL="crashalert@renegadeinc.net"

# Define a function to send the email alert.
send_crash_alert() {
    # $1=coredump path, $2=logfile, $3=exe path, $4=pid
    local corefile="$1"
    local logfile="$2"
    local exe="$3"
    local pid="$4"


    local subject="CRITICAL: MUD Crash — PID ${pid}"


    # Extract the last player command from the log: prefer the line after "SEGMENTATION VIOLATION",
    # otherwise fall back to the last "** player: command" entry.
    local last_cmd=""
    if [[ -f "$logfile" ]]; then
        last_cmd=$(awk '/SEGMENTATION VIOLATION/{flag=1;next} flag && NF {print;flag=0}' "$logfile" | tail -n 1 || true)
        if [[ -z "$last_cmd" ]]; then
            last_cmd=$(grep -a "^\*\* " "$logfile" | tail -n 1 || true)
        fi
    fi


    # Obtain a concise crash location using gdb if available.
    local crash_loc=""
    local bt_summary=""
    if command -v gdb >/dev/null 2>&1; then
        # Full backtrace (all threads), then parse frame #0 from the crashing thread.
        bt_summary=$(gdb -q -n -batch \
            -ex "set pagination off" \
            -ex "thread apply all bt full" \
            --args "$exe" "$corefile" 2>&1)
        crash_loc=$(printf "%s\n" "$bt_summary" | awk '/^#0 /{print;exit}')
        # If #0 not present, fallback to first line containing " at .*:[0-9]+"
        if [[ -z "$crash_loc" ]]; then
            crash_loc=$(printf "%s\n" "$bt_summary" | grep -E " at .*:[0-9]+" | head -n1)
        fi
    else
        crash_loc="gdb not installed; unable to resolve source line"
        bt_summary="gdb not installed on host; detailed backtrace unavailable"
    fi


    local message_body="\
MUD server crash detected.\n\n
Timestamp: $(date)\n
Working Directory: $(pwd)\n
Executable: ${exe}\n
PID: ${pid}\n
Coredump File: ${corefile}\n
Log File: ${logfile}\n\n
Last Command: ${last_cmd}\n\nTop Frame: ${crash_loc}\n\n
--- Recent log tail ---\n$(tail -n 60 "$logfile" 2>/dev/null)\n
--- GDB backtrace ---\n${bt_summary}
"


    # Send the email, fallback to text file
    if command -v mail >/dev/null 2>&1; then
        printf "%s\n" "$message_body" | mail -s "$subject" "$ALERT_EMAIL" || true
    else
        (echo "$message_body" > "$corefile-report-${pid}.txt" || true)
    fi
}

check_for_coredump() {
    local pid="$1"
    local exe="$2"
    local logfile="$3"

    wait $pid

    # Core dump files are named "core.<exe>.<pid>"
    local latest_core=$(ls -t core.* 2>/dev/null | head -n 1 || true)

    # Send alert and exit if coredump detected
    if [[ -n "$latest_core" ]]; then
        send_crash_alert "$latest_core" "$logfile" "$exe" "$pid"
        exit 1
    fi
}

while true; do
    # find next log index
    i=1000
    while [[ -f $i.log ]]; do
        i=$((i + 1))
    done

    logfile="$i.log"

    if [[ -f shutdown.txt ]]; then
        rm shutdown.txt
        exit 0
    fi

    # Launch server
    ./swr $port >"$logfile" 2>&1 &
    pid=$!

    check_for_coredump $pid ./swr "$logfile"
done
# Note: we do not loop on crashes, to allow for debugging. The user can restart the script if desired.