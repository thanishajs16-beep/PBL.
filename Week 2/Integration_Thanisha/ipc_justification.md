 IPC Justification

1. Selected IPC Mechanism

The Multi-Process Simulator uses **POSIX Message Queues** for Inter-Process Communication (IPC).

POSIX Message Queues allow the independent UI, Core, and Logging processes to exchange messages without directly sharing their memory.

2. Why POSIX Message Queues Were Selected

POSIX Message Queues were selected because they are well suited to the message-based communication required by the simulator.

The main reasons are:

- **Clear communication:** Commands and results can be exchanged as individual messages.
- **Process independence:** UI, Core, and Logging run as separate processes.
- **Simple communication structure:** Different queues can be used for different communication paths.
- **Safe data exchange:** Processes communicate through messages instead of directly accessing each other's memory.
- **Suitable for this project:** The simulator mainly exchanges commands, results, and log messages, making message queues a suitable choice.

3. IPC Communication in the Project

The system uses three POSIX Message Queues:

```text
/Yenkbachind_request
/Yenkbachind_response
/expression_logger
```

UI → Core

The UI Process sends user commands to the Core Process through the request queue.

```text
UI Process
    |
    | Command
    v
/Yenkbachind_request
    |
    v
Core Process
```

 Core → UI

The Core Process sends the result of the processed command back to the UI Process through the response queue.

```text
Core Process
    |
    | Result
    v
/Yenkbachind_response
    |
    v
UI Process
```

Core → Logging Process

The Core Process sends command results and errors to the Logging Process through the logger queue.

```text
Core Process
    |
    | Log Message
    v
/expression_logger
    |
    v
Logging Process
```

4. Comparison with Other IPC Methods

| IPC Method           | Advantages                                                                       | Limitations for This Project                                   |
| -------------------- | -------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| Pipes                | Simple and useful for stream-based communication                                 | Less convenient for multiple message-based communication paths |
| Shared Memory        | Very fast for large amounts of shared data                                       | Requires synchronization mechanisms to avoid conflicts         |
| POSIX Message Queues | Provides structured message-based communication and separate communication paths | Has queue and message-size limits                              |

For this simulator, the amount of data exchanged between processes is small and mainly consists of commands, results, and log messages. Therefore, the advantages of POSIX Message Queues make them appropriate for this project.

5. Conclusion

POSIX Message Queues provide a suitable IPC mechanism for the Multi-Process Simulator. They allow the UI, Core, and Logging processes to communicate independently while maintaining clear separation of responsibilities.

The implementation successfully demonstrates:

- UI to Core communication
- Core to UI communication
- Core to Logging communication
- Error message communication
- Process shutdown communication

The IPC test cases confirm that these communication paths are working correctly.
