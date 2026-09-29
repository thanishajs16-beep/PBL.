TECHNIQUES OF IPC

1.Pipes

Meaning:

A pipe is one of the simplest IPC techniques used for communication between processes. It provides a temporary communication channel through which one process can send data and another process can receive it.
A pipe generally follows FIFO (First In, First Out) order.

Working:

The basic working of a pipe is:
A pipe is created by the operating system.
One process writes data into the pipe.
The data remains in the pipe until another process reads it.
The receiving process reads the data from the pipe.
After the data is read, it is removed from the pipe.

For example, one process can produce some information and write it into the pipe, while another process reads and uses that information.

Types of Pipes:

1. Anonymous Pipe:
It is usually used between related processes, such as a parent process and its child process. It generally exists only while the processes are running.

2. Named Pipe (FIFO):
It has a name in the file system and can be used by processes that are not directly related to each other.

Advantages:
* Simple and easy to use. Useful for communication        
* Data can be transferred in              
 organised manner.

Example:
A common example is the command pipeline in Linux

2.Message Queues

Meaning:

A message queue is an IPC technique in which processes communicate by sending and receiving messages. The messages are stored in a queue maintained by the operating system until the receiving process reads them.

Working:

A message queue is created by the operating system.
The sending process creates a message.
The message is placed into the queue.
The receiving process checks the queue.
It receives the required message from the queue.
The message is removed from the queue after it is received.
Messages can also contain information such as message type or priority, depending on the operating system.

Types of Message Queues:

1. System V Message Queues:
A traditional IPC mechanism provided by Unix/Linux systems.

2. POSIX Message Queues:
A standardized message queue mechanism provided by POSIX-compatible operating systems.

Advantages:
* Messages are stored until the receiver is ready to receive them.
* Processes do not always have to communicate at exactly the same time.
* It is useful when different types of information need to be exchanged.
* It provides better separation between the sender and receiver.

Example:
Consider a printer system. Different applications can send printing requests to a message queue. The printer-related process can take one request at a time from the queue and process it.

 3. Shared Memory

Meaning:

Shared memory is an IPC technique where two or more processes are given access to the same area of memory. Processes can use this common memory area to exchange data.

It is considered one of the fastest IPC methods because processes can access the shared data directly instead of repeatedly sending messages through the operating system.

Working:

The operating system creates a shared memory area.
The required processes are given access to this memory.
One process writes data into the shared memory.
Another process reads the data from the same memory area.
After communication is completed, the shared memory can be removed or detached.

Types of Shared Memory:

1. System V Shared Memory:
An IPC mechanism commonly available in Unix/Linux systems.

2. POSIX Shared Memory:
A standardized shared-memory mechanism provided by POSIX systems.

Advantages
* Very fast compared with many other IPC methods.
* Suitable for transferring large amounts of data.
* Processes can directly access the shared data.
* Reduces the need for repeated data copying.

Example:
A good example is a database or multimedia application where multiple processes need to access a large amount of common data. Instead of sending the complete data repeatedly, the processes can access it through a shared memory region.
