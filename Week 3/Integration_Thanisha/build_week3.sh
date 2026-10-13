#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/.week3_build"
OUTPUT="$PROJECT_DIR/scheduler"

required_files=(
    integration.c
    fcfs.c
    sjf_non_preemptive.c
    sjf_preemptive.c
    priority.c
    round_robin.c
    scheduling.h
)

for file in "${required_files[@]}"; do
    if [[ ! -f "$PROJECT_DIR/$file" ]]; then
        printf 'Missing required file: %s\n' "$file" >&2
        printf 'Put integration.c, this script, and the required project files in the same folder.\n' >&2
        exit 1
    fi
done

mkdir -p "$BUILD_DIR"
CFLAGS=(-Wall -Wextra -std=c11 -I"$PROJECT_DIR")

printf 'Compiling scheduling programs...\n'
gcc "${CFLAGS[@]}" -Dmain=fcfs_main -c "$PROJECT_DIR/fcfs.c" -o "$BUILD_DIR/fcfs.o"
gcc "${CFLAGS[@]}" -Dmain=sjf_non_preemptive_main -c "$PROJECT_DIR/sjf_non_preemptive.c" -o "$BUILD_DIR/sjf_non_preemptive.o"
gcc "${CFLAGS[@]}" -Dmain=sjf_preemptive_main -c "$PROJECT_DIR/sjf_preemptive.c" -o "$BUILD_DIR/sjf_preemptive.o"
gcc "${CFLAGS[@]}" -c "$PROJECT_DIR/priority.c" -o "$BUILD_DIR/priority.o"
gcc "${CFLAGS[@]}" -Dmain=round_robin_main -c "$PROJECT_DIR/round_robin.c" -o "$BUILD_DIR/round_robin.o"
gcc "${CFLAGS[@]}" -c "$PROJECT_DIR/integration.c" -o "$BUILD_DIR/integration.o"

gcc "$BUILD_DIR/integration.o" \
    "$BUILD_DIR/fcfs.o" \
    "$BUILD_DIR/sjf_non_preemptive.o" \
    "$BUILD_DIR/sjf_preemptive.o" \
    "$BUILD_DIR/priority.o" \
    "$BUILD_DIR/round_robin.o" \
    -o "$OUTPUT"

printf 'Build successful: %s\n' "$OUTPUT"
printf 'Run it with: ./scheduler\n'
