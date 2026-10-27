# Page Replacement Simulator

## Project Overview

The Page Replacement Simulator is an Operating Systems project that demonstrates how page replacement algorithms manage pages in physical memory when the number of available page frames is limited.

The simulator implements three page replacement algorithms: FIFO (First-In, First-Out), LRU (Least Recently Used), and Optimal. It allows users to enter a page-reference string and the number of available frames, simulate page replacement, and compare the performance of the algorithms.

## Objectives

- Understand virtual memory and page replacement.
- Implement FIFO, LRU, and Optimal page replacement algorithms.
- Display frame contents after each page reference.
- Identify and count page hits and page faults.
- Calculate hit ratios and fault ratios.
- Compare algorithm performance using a graphical user interface.

## Features

- Graphical user interface (GUI).
- Input validation for page references and frame count.
- Separate simulation results for FIFO, LRU, and Optimal.
- Frame-by-frame memory state display.
- Page-hit and page-fault statistics.
- Comparison of algorithm performance.
- Console-based integration test.

## Technologies Used

- **Programming Language:** C
- **GUI Framework:** GTK 3
- **Development Environment:** Visual Studio Code
- **Build Tools:** GCC and `pkg-config`

## Project Files

| File | Description |
|---|---|
| `common.h` | Shared data structures, constants, and function declarations |
| `fifo.c` | FIFO page replacement algorithm |
| `lru.c` | LRU page replacement algorithm |
| `optimal.c` | Optimal page replacement algorithm |
| `comparison.c` | Compares the results of all three algorithms |
| `gui.c` | Graphical user interface |
| `test_integration.c` | Console-based integration test |
| `build.sh` | Script for building the console test and GUI |

## Requirements

- GCC compiler
- GTK 3 development libraries
- `pkg-config`
- Linux or a compatible environment such as WSL with a working graphical display

## Build Instructions

Open a terminal in the project directory.

Make the build script executable:

```bash
chmod +x build.sh
```

Build both the console test and GUI:

```bash
./build.sh
```

Build only the console test:

```bash
./build.sh core
```

Build only the GUI:

```bash
./build.sh gui
```

## Run Instructions

Run the graphical simulator:

```bash
./page_simulator_gui
```

Run the console integration test:

```bash
./page_simulator
```

## Sample Input

- Page-reference string: `7 0 1 2 0 3 0 4 2 3`
- Number of page frames: `3`
- Algorithm selection: Compare all algorithms

## Expected Results

The simulator displays the memory-frame contents after each page reference, identifies page hits and page faults, calculates performance statistics, and compares the results of FIFO, LRU, and Optimal.

The algorithm with the fewest page faults is identified as the best-performing algorithm for the selected input.

## Team Contributions

- **GUI Development:** Sarvani
- **FIFO Algorithm:** Anandhu
- **LRU and Optimal Algorithms:** Russell
- **Integration, Comparison, Testing, and Documentation:** Thanisha

## Conclusion

This project provides a practical demonstration of page replacement in operating systems. It helps users understand how different algorithms manage memory and compare their performance using the same page-reference string and frame count.
