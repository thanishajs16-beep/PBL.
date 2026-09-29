# Inter-Process Communication (IPC)

IPC (Inter-Process Communication) is a mechanism used by processes to send and receive data or messages while they are running.

## 1. Need for IPC

- **Communication:** Processes communicate because different processes may perform different parts of a task.
- **Data sharing:** Processes share data because one process may produce data that another process needs.
- **Resource sharing:** Processes share resources because resources may need to be used by multiple processes.
- **Coordination:** Processes coordinate because their tasks may depend on each other.
- **Synchronization:** Processes synchronize because they must execute in the correct order and avoid conflicts.

## 2. Importance of IPC

- Improves efficiency by allowing processes to work together.
- Supports multitasking by enabling processes to work concurrently.
- Improves resource utilization through resource sharing.
- Ensures coordination between processes.
- Helps complete complex tasks by dividing work among processes.

## 3. Basic Working of IPC – Client-Server

1. **Client** sends a request to the **Server** through IPC.
2. **Server** receives and processes the request.
3. **Server** sends the response back through IPC.
4. **Client** receives the response and continues its work.

### Simple Flow

**Client → Request → IPC → Server → Response → IPC → Client**
