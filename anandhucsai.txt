IPC – PERSON 4 BRIEF AND SIMPLE NOTES

1. REMOTE PROCEDURE CALL (RPC)

Meaning: RPC stands for Remote Procedure Call. It allows a program to
call a function that is running in another process or on another
computer.

Simple example: A banking app requests the account balance from the
bank’s server. The server checks it and sends the balance back.

Working: 1. Client calls a function. 2. Request is sent to the server.
3. Server performs the operation. 4. Server sends the result back. 5.
Client receives the result.

Types: - Synchronous RPC – Client waits for the result. - Asynchronous
RPC – Client can continue working without waiting immediately.

Examples: - Banking applications - Microservices - Client-server
applications

Advantages: - Easy way to communicate with remote systems. - Hides many
networking details. - Useful for distributed applications.

Important terms: - Marshalling: Converting data into a format that can
be sent. - Unmarshalling: Converting received data back into usable
form.

2. MEMORY-MAPPED FILES

Meaning: A memory-mapped file is a file that is mapped into a process’s
memory. The program can access the file almost like normal memory.

Working: 1. File is opened. 2. Operating system maps it into memory. 3.
Process reads or modifies the data through memory. 4. Changes can be
written back to the file. 5. Another process can also access the same
mapped file.

Types: - Read-only mapping - Read-write mapping - Shared mapping

Example: Two processes can use the same memory-mapped file to share
data.

Advantages: - Fast access to data. - Useful for large files. - Can be
used for sharing data between processes.

Limitation: - Synchronization may be needed when multiple processes
modify the same data.

3. COMPARISON OF IPC METHODS

IPC = Inter-Process Communication.

Pipe: - Used for simple data communication. - One process writes and
another reads. - Example: Parent and child process.

Message Queue: - Messages are stored in a queue. - Receiver reads the
messages later. - Useful for structured communication.

Shared Memory: - Multiple processes use the same memory area. - Very
fast. - Useful for large amounts of data. - Requires synchronization.

Semaphore: - Mainly used for synchronization. - Controls access to
shared resources. - Example: Controlling access to a printer.

Signal: - Used to send a small notification to another process. -
Example: Telling a process to stop.

Socket: - Used for communication between processes. - Can work on the
same computer or over a network. - Example: Browser communicating with a
web server.

RPC: - Used to call a function in another process or computer. -
Example: Banking app communicating with a server.

Memory-Mapped File: - Maps a file into memory. - Useful for fast file
access and sharing data.


