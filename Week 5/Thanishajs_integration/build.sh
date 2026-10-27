
#!/bin/bash
set -e

echo "Building Page Replacement Simulator..."

gcc -Wall -Wextra -std=c11 \
    test_integration.c comparison.c fifo.c lru.c optimal.c \
    -o page_simulator

echo "Build successful!"