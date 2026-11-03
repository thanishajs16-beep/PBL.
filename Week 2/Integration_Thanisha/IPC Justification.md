# IPC Justification

## 1. Selected IPC Mechanisms

The Multi-Process Simulator uses a **combination of IPC mechanisms** to support communication between its processes:

- **POSIX Message Queues** for communication between the UI and Core processes.
- **Unix Domain Sockets** for communication between the Core process and the CPU, Memory, Stack, and Queue worker processes.
- **Pipes** for communication between the Core process and the Logging process.

Each mechanism is used for a specific communication path in the simulator.

## 2. Why These IPC Mechanisms Were Selected

- **POSIX Message Queues:** Allow the UI and Core processes to exchange commands and results as separate messages through request and response queues.
- **Unix Domain Sockets:** Provide two-way communication between Core and each worker process. Separate socket pairs are created for CPU, Memory, Stack, and Queue.
- **Pipes:** Provide a simple way for Core to send log messages to the Logging process.
- **Process independence:** The UI, Core, worker, and Logging processes can perform their assigned tasks separately and communicate through IPC.

## 3. IPC Communication in the Project

### UI to Core and Core to UI — POSIX Message Queues

The UI sends commands to Core through the request queue. Core sends the processed result back through the response queue.

```text
UI Process
    |
    | Command
    v
/Yenkbachind_request
    |
    v
Core Process
    |
    | Result
    v
/Yenkbachind_response
    |
    v
UI Process
```

### Core to CPU, Memory, Stack, and Queue — Unix Domain Sockets

Core creates a separate socket pair for each worker process. Commands are sent to the appropriate worker, and the worker returns its response through the socket.

```text
                 Core Process
                /     |      \
               /      |       \
          Socket   Socket    Socket ...
             |        |         |
            CPU    Memory     Stack    Queue
          Worker   Worker    Worker   Worker
```

### Core to Logging Process — Pipe

Core sends log messages, command details, and responses to the Logging process through a pipe.

```text
Core Process
     |
     | Log messages
     v
    Pipe
     |
     v
Logging Process
```

## 4. Comparison with Other IPC Methods

| IPC Method | Use in This Project | Main Advantage |
|---|---|---|
| Unix Domain Sockets | Core and CPU, Memory, Stack, and Queue workers | Supports two-way communication through separate socket pairs. |
| Pipes | Core and Logging process | Simple stream-based communication for log messages. |
| POSIX Message Queues | UI and Core processes | Exchanges commands and results as separate messages. |
| Shared Memory | Not used in the supplied `core_process.c` | Allows fast data sharing, but requires synchronization. |

## 5. Conclusion

The simulator uses different IPC mechanisms for different communication paths:

- **UI ↔ Core:** POSIX Message Queues
- **Core ↔ CPU, Memory, Stack, and Queue:** Unix Domain Sockets
- **Core → Logging:** Pipe

This combination allows the processes to communicate while keeping their responsibilities separate.

**Note:** The supplied `core_process.c` uses POSIX Message Queues for UI-to-Core communication. It does not show a shared-memory connection for the UI.
