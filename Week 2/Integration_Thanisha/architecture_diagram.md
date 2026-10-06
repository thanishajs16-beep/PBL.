# System Architecture

## 1. Overview

The Multi-Process Simulator consists of three independent processes:

- **Student 1 – UI Process:** Handles user input and displays results.
- **Student 4 – Core Process:** Processes commands and integrates CPU, Memory, Stack, and Queue.
- **Student 3 – Logging Process:** Receives log messages and stores them in `simulator.log`.

The processes communicate using **POSIX Message Queues**.

## 2. Architecture Diagram

```text
                         USER
                          |
                          v
                  +---------------+
                  |   UI PROCESS  |
                  | ui_process.c  |
                  +---------------+
                          |
                          | POSIX Message Queue
                          | UI → Core
                          v
                  +------------------+
                  |   CORE PROCESS   |
                  | core_process.c   |
                  +------------------+
                    /    |    |    \
                   v     v    v     v
                +-----+ +------+ +-----+ +-------+
                | CPU | |Memory| |Stack| | Queue |
                |.c   | |.c    | |.c   | |.c    |
                +-----+ +------+ +-----+ +-------+
                          |
                          | POSIX Message Queue
                          | Core → Logger
                          v
                  +-------------------+
                  | LOGGING PROCESS   |
                  |logging_process.c  |
                  +-------------------+
                          |
                          v
                    simulator.log
```

## 3. Process Responsibilities

| Process         | Responsibility                                                  |
| --------------- | --------------------------------------------------------------- |
| UI Process      | Accepts commands and displays Core responses                    |
| Core Process    | Processes commands and integrates CPU, Memory, Stack, and Queue |
| Logging Process | Receives and stores command results and errors                  |

## 4. IPC Flow

```text
UI Process
    |
    | Command
    v
Core Process
    |
    +----> CPU / Memory / Stack / Queue
    |
    +----> Response ----> UI Process
    |
    +----> Log Message
                |
                v
        Logging Process
                |
                v
          simulator.log
```

## 5. Integration

The Team Leader integrates the UI, Core, and Logging processes using POSIX Message Queues. The Core Process acts as the central component connecting the user commands with the simulator modules and logging system.
