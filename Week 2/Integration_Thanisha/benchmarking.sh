#!/bin/bash

RUNS=5
REPEAT=1000

echo "========================================"
echo "STANDALONE VS MULTI-PROCESS BENCHMARK"
echo "========================================"
echo
echo "Runs: $RUNS"
echo "Workload repetitions per run: $REPEAT"
echo

# Temporary standalone program
cat > /tmp/standalone.c <<'EOF'
#include <stdio.h>
#include <string.h>

typedef struct {
    int accumulator;
    int program_counter;
    int running;
} CPU;

void process_command(CPU *, char [], char [], int);

int main()
{
    CPU cpu = {0, 0, 1};
    char instruction[256];
    char response[256];

    while (fgets(instruction, sizeof(instruction), stdin))
    {
        instruction[strcspn(instruction, "\n")] = '\0';

        if (strcmp(instruction, "exit") == 0)
            break;

        process_command(&cpu, instruction, response, sizeof(response));
    }

    return 0;
}
EOF

gcc /tmp/standalone.c core.c cpu.c Memory.c Stack.c Queue.c \
    -o /tmp/core_standalone

# Create workload
WORKLOAD=/tmp/workload.txt
> "$WORKLOAD"

for i in $(seq 1 $REPEAT)
do
    echo "LOAD 10" >> "$WORKLOAD"
    echo "ADD 5" >> "$WORKLOAD"
    echo "STORE 10 100" >> "$WORKLOAD"
    echo "READ 10" >> "$WORKLOAD"
    echo "PUSH 20" >> "$WORKLOAD"
    echo "POP" >> "$WORKLOAD"
    echo "ENQUEUE 30" >> "$WORKLOAD"
    echo "DEQUEUE" >> "$WORKLOAD"
done

# ------------------------------------------------
# STANDALONE
# ------------------------------------------------

echo "--- Standalone Core ---"

standalone_total=0

for i in 1 2 3 4 5
do
    time=$(/usr/bin/time -f "%e" \
        sh -c 'cat /tmp/workload.txt | /tmp/core_standalone >/dev/null' \
        2>&1)

    echo "Run $i: $time sec"

    standalone_total=$(awk \
        "BEGIN {print $standalone_total + $time}")
done

standalone_avg=$(awk \
    "BEGIN {print $standalone_total / $RUNS}")

echo
echo "Standalone total   : $standalone_total sec"
echo "Standalone average : $standalone_avg sec"

# ------------------------------------------------
# MULTI-PROCESS
# ------------------------------------------------

echo
echo "--- Multi-Process Simulator ---"

./logging_process >/dev/null 2>&1 &
LOGGER_PID=$!

sleep 1

./core_process >/dev/null 2>&1 &
CORE_PID=$!

sleep 1

multi_total=0

for i in 1 2 3 4 5
do
    time=$(/usr/bin/time -f "%e" \
        sh -c 'cat /tmp/workload.txt | ./ui_process >/dev/null' \
        2>&1)

    echo "Run $i: $time sec"

    multi_total=$(awk \
        "BEGIN {print $multi_total + $time}")
done

multi_avg=$(awk \
    "BEGIN {print $multi_total / $RUNS}")

echo
echo "Multi-process total   : $multi_total sec"
echo "Multi-process average : $multi_avg sec"

# ------------------------------------------------
# IPC OVERHEAD
# ------------------------------------------------

ipc=$(awk \
    "BEGIN {print $multi_avg - $standalone_avg}")

ipc_percent=$(awk \
    "BEGIN {print ($ipc / $standalone_avg) * 100}")

echo
echo "--- IPC OVERHEAD ---"
echo "IPC overhead            : $ipc sec"
echo "IPC overhead percentage : $ipc_percent %"

# ------------------------------------------------
# CPU AND MEMORY
# ------------------------------------------------

echo
echo "--- CPU AND MEMORY USAGE ---"

echo
echo "Standalone:"
/usr/bin/time -v \
    sh -c 'cat /tmp/workload.txt | /tmp/core_standalone >/dev/null' \
    2>&1 | grep -E \
    "User time|System time|Percent of CPU|Maximum resident"

echo
echo "Multi-process Core:"
ps -p $CORE_PID -o pid,%cpu,%mem,rss,cmd

echo
echo "Multi-process Logger:"
ps -p $LOGGER_PID -o pid,%cpu,%mem,rss,cmd

# ------------------------------------------------
# STOP PROCESSES
# ------------------------------------------------

printf "exit\n" | ./ui_process >/dev/null 2>&1

wait $CORE_PID 2>/dev/null
wait $LOGGER_PID 2>/dev/null

echo
echo "========================================"
echo "BENCHMARK COMPLETE"
echo "========================================"