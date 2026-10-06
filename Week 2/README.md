# Multi-Process Simulator & IPC

 Project Overview

This project is a multi-process CPU and memory simulator implemented in C on Linux.

The system is divided into three independent processes:

- **UI Process** — accepts commands from the user.
- **Core Process** — processes commands and manages CPU, Memory, Stack, and Queue operations.
- **Logging Process** — receives log messages from the Core Process and stores them in `simulator.log`.

The processes communicate using **POSIX Message Queues (IPC)**.

 System Architecture

```text
                         User
                           |
                           v
                   +---------------+
                   |   UI Process  |
                   | ui_process.c  |
                   +---------------+
                           |
                      POSIX MQ
                           |
                           v
                   +----------------+
                   |  Core Process  |
                   | core_process.c |
                   +----------------+
                    |      |      |
                    |      |      +----------+
                    |      |                 |
                    v      v                 v
                   CPU   Memory          Stack / Queue

                           |
                      POSIX MQ
                           |
                           v
                 +-------------------+
                 | Logging Process   |
                 | logging_process.c |
                 +-------------------+
                           |
                           v
                     simulator.log
```

## Components

| Component | File | Responsibility |
|---|---|---|
| UI Process | `ui_process.c` | Accepts user commands and displays Core responses |
| Core Process | `core_process.c` | Processes commands and coordinates simulator modules |
| CPU | `cpu.c` | Performs arithmetic and CPU instructions |
| Memory | `Memory.c` | Handles memory read and write operations |
| Stack | `Stack.c` | Handles push, pop, and peek operations |
| Queue | `Queue.c` | Handles enqueue, dequeue, and peek operations |
| Logging Process | `logging_process.c` | Receives and stores log messages |
| IPC Header | `ipc.h` | Shared IPC definitions |

## IPC Method

The project uses **POSIX Message Queues** for communication between processes.

Main communication paths:

```text
UI Process  ------------->  Core Process
UI Process  <-------------  Core Process
Core Process -------------> Logging Process
```

Message queues allow the processes to remain independent while exchanging commands, responses, and log messages.

Detailed IPC selection and justification are documented in:

`IPC_JUSTIFICATION.md`

Supported Commands

```text
ADD a b
SUB a b
MUL a b
DIV a b
PUSH value
POP
ENQUEUE value
DEQUEUE
STORE address value
READ address
exit
```

How to Run

Open three terminal windows in the project directory.

Terminal 1 — Logging Process

```bash
./logging_process
```

Terminal 2 — Core Process

```bash
./core_process
```

Terminal 3 — UI Process

```bash
./ui_process
```

Start the processes in this order so that the Core Process can connect to the Logging Process before accepting UI commands.

Example Test

```text
UI > STORE 0 10
CORE: Memory[0] = 10

UI > READ 0
CORE: Memory[0] = 10

UI > PUSH 25
CORE: Pushed 25

UI > POP
CORE: Popped 25

UI > ENQUEUE 30
CORE: Enqueued 30

UI > DEQUEUE
CORE: Dequeued 30

UI > READ 101
CORE: Invalid memory address

UI > exit
```

The corresponding operations and error condition are also received by the Logging Process and stored in `simulator.log`.

IPC Testing

The integrated system was tested for:

- UI to Core communication
- Core to UI responses
- Core to Logging Process communication
- Error message communication
- Stack and Queue operations through the Core Process
- Multiple sequential commands
- Process shutdown communication

All tested communication paths completed successfully.

Detailed test cases are available in:

`IPC_TEST_CASES.md`

Performance Analysis

The project also compares the standalone simulator with the multi-process version.

The analysis considers:

- Execution time
- IPC overhead
- CPU usage
- Memory usage

The benchmark results and observations are documented in:

`PERFORMANCE_ANALYSIS.md`

Logging

The Logging Process writes system activity to:

```text
simulator.log
```

Example log entry:

```text
[INFO] Command: STORE 0 10 | Result: Memory[0] = 10
```

Example error log entry:

```text
[ERROR] Command: READ 101 | Result: Invalid memory address
```

## Team Responsibilities

| Member | Responsibility |
|---|---|
| Student 1 | UI Process |
| Student 2 | Core Process |
| Student 3 | Logging Process |
| Team Leader | Integration, IPC, testing, architecture, and performance analysis |

## Documentation

| File | Description |
|---|---|
| `architecture_diagram` | Overall system architecture |
| `ipc_justification.md` | Reason for selecting POSIX Message Queues |
| `ipc_test_cases.txt` | IPC integration test cases and results |
| `Performance_analysis.txt` | Standalone vs multi-process performance comparison |

## Conclusion

The project successfully integrates the UI, Core, and Logging components as separate processes using POSIX Message Queues.

The final system demonstrates:

- Inter-process communication
- CPU and memory simulator operations
- Stack and Queue operations
- Error handling
- Logging
- Performance comparison

The project is designed and tested for a Linux-based C environment.
