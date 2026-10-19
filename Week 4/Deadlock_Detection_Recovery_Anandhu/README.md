# Student 3 — Deadlock Detection and Recovery

This mini-project uses **C for the deadlock logic** and **Python Tkinter for the GUI**.

## Files
- `deadlock.c` — deadlock detection, Resource Allocation Graph (RAG) edge generation, and recovery logic.
- `gui.py` — graphical interface. It compiles and runs `deadlock.c` when you click an operation.

## Requirements
- Python 3
- GCC compiler available in PATH
- Tkinter (usually included with standard Python on Windows)

### Check requirements
Open a terminal in this folder and run:
```bash
python --version
gcc --version
```
On Windows, install a GCC toolchain such as MinGW-w64 or MSYS2 if `gcc` is not recognized. Ensure its `bin` folder is in PATH.

## Run
```bash
python gui.py
```
If your system uses `python3`, run `python3 gui.py`.

Click **Load deadlock demo**, then **Detect deadlock**. You should see P1 and P2 identified as deadlocked.
Click **Generate RAG data** to list allocation edges (`R -> P`) and request edges (`P -> R`).
Select process 2 and click **Recover**. It should report that P2 was terminated and its allocated resources released.

## Demo input
Processes: 2
Resource types: 2
Available: `0 0`
Allocation:
```text
P1: 1 0
P2: 0 1
```
Request:
```text
P1: 0 1
P2: 1 0
```

## Test cases

### Test 1 — Deadlock exists
Use **Load deadlock demo**.
Expected: `STATUS: DEADLOCK`; `DEADLOCKED: P1 P2`.

### Test 2 — No deadlock
Set Available to `1 1`, Allocation to `1 0` and `0 1`, Request to `0 1` and `1 0`.
Expected: `STATUS: NO_DEADLOCK`, because available resources let the processes' requests be satisfied and resources can be released.

### Test 3 — Recovery
Load the deadlock demo, choose process 2, click **Recover**.
Expected: P2 is terminated, Available becomes `0 1`, and the deadlock is removed because P1 can receive R2 and finish.
Note: the GUI does not persist a recovered state after the operation; it reports the updated state produced by the C backend. If you want to continue interacting with the changed state, manually update the matrices using the displayed Available vector and set the terminated process's Allocation and Request rows to zero.

### Test 4 — Invalid input
Enter a negative number or leave a matrix cell empty.
Expected: the GUI shows an input validation error.

## What the algorithm does
1. Copies Available into Work.
2. Marks terminated processes and processes holding no resources as finished.
3. Finds an unfinished process whose Request is no greater than Work.
4. Pretends that process finishes and adds its Allocation back to Work.
5. Repeats until no further process can finish.
6. Remaining unfinished processes are reported as deadlocked.

## Important limitation
This module expects **Request** to mean the currently outstanding request for each process, not the Banker's Algorithm `Need` matrix (`Maximum - Allocation`). It detects deadlock for the state supplied to it. In systems with multiple instances of resource types, the detection algorithm is more informative than checking for a graph cycle alone.
