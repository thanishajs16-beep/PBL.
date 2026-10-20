
#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"
if [[ "${1:-gui}" == "test" ]]; then
    echo "Building integration tests..."

    gcc -Wall -Wextra -std=c11 \
        test_core.c deadlock_engine.c -o test_core

    gcc -Wall -Wextra -std=c11 \
        test_detection.c deadlock_engine.c -o test_detection

    echo ""
    echo "=== Banker's Algorithm Test ==="
    ./test_core

    echo ""
    echo "=== Detection, RAG, and Recovery Test ==="
    ./test_detection

    echo ""
    echo "All core integration tests passed!"
    exit 0
fi

if [[ "${1:-gui}" == "core" ]]; then
    echo "Checking the deadlock engine..."

    gcc -Wall -Wextra -std=c11 \
        -c deadlock_engine.c \
        -o deadlock_engine.o

    echo "Core engine compiled successfully!"
    exit 0
fi

if ! command -v pkg-config >/dev/null 2>&1; then
    echo "GTK build unavailable: pkg-config is not installed."
    echo "Use './build.sh core' to check the core engine first."
    exit 1
fi

if ! pkg-config --exists gtk+-3.0; then
    echo "GTK 3 development files are missing."
    echo "Install them before building the GUI."
    exit 1
fi

echo "Building the deadlock simulation GUI..."

gcc -Wall -Wextra -std=c11 \
    $(pkg-config --cflags gtk+-3.0) \
    main.c deadlock_engine.c gui.c \
    -o deadlock_simulator \
    $(pkg-config --libs gtk+-3.0) -lm

echo "GUI build successful!"
