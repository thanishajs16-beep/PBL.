1. Semaphores
Meaning
A semaphore is a synchronization mechanism used to control access to a shared resource by multiple processes or threads.
It uses an integer value and mainly prevents race conditions.

Working
1.	A semaphore maintains a counter value. 
2.	Before accessing a shared resource, a process performs wait (P) operation. 
3.	If the resource is available, the process enters the critical section. 
4.	After completing the work, it performs signal (V) operation. 
5.	The resource becomes available to another process. 

Simple flow:
Process → wait() → Critical Section → signal() → Next Process

Types
1.	Binary Semaphore – Value is 0 or 1; used like a lock. 
2.	Counting Semaphore – Value can be any non-negative integer; manages multiple identical resources. 

Example
Suppose there is one printer shared by many processes.
•	Semaphore = 1 
•	Process A performs wait() → semaphore becomes 0 → uses printer. 
•	Process B must wait. 
•	Process A finishes → performs signal() → semaphore becomes 1. 
•	Process B can now use the printer. 

Advantages
•	Prevents race conditions. 
•	Provides process synchronization. 
•	Controls access to shared resources. 
•	Helps avoid simultaneous access to critical sections. 
________________________________________
2. Signals
Meaning
A signal is a software notification sent to a process to inform it that a particular event has occurred.
It is mainly used for asynchronous communication between processes.

Working
1.	An event occurs. 
2.	The operating system generates a signal. 
3.	The signal is delivered to the target process. 
4.	The process performs a predefined action or signal handler. 
5.	The process continues or terminates depending on the signal. 

Simple flow:
Event → Signal Generated → Process Receives Signal → Signal Handler → Action

Types / Common Signals
Signal	Meaning
SIGINT	Interrupt from keyboard, usually Ctrl+C
SIGTERM	Requests process termination
SIGKILL	Forces process termination
SIGSTOP	Stops/suspends a process
SIGCONT	Continues a stopped process
SIGCHLD	Indicates that a child process has changed state

Example
When a program is running in the terminal and the user presses Ctrl+C, the operating system sends SIGINT to the process.
The process can catch the signal and perform some cleanup before stopping.

Advantages
•	Simple way to notify processes. 
•	Useful for handling asynchronous events. 
•	Requires little communication overhead. 
•	Useful for process control and termination. 
________________________________________
3. Sockets
Meaning
A socket is an endpoint used for communication between processes.
Sockets can allow communication:
•	Between processes on the same computer 
•	Between processes on different computers over a network 

Working
Socket communication generally follows:
Server:
Create Socket → Bind → Listen → Accept → Send/Receive → Close
Client:
Create Socket → Connect → Send/Receive → Close

Types
1.	Stream Socket (TCP) 
o	Connection-oriented. 
o	Reliable and ordered communication. 
o	Uses TCP. 
2.	Datagram Socket (UDP) 
o	Connectionless. 
o	Faster but does not guarantee delivery. 
o	Uses UDP. 

Example
A web browser and web server communicate using sockets.
For example:
•	Browser acts as the client. 
•	Web server acts as the server. 
•	Client connects to the server using a socket. 
•	They exchange data. 
•	Connection is closed after communication. 

Advantages
•	Supports communication between different computers. 
•	Can be used for network-based IPC. 
•	Supports both TCP and UDP communication. 
•	Suitable for client-server applications. 
•	Flexible and widely supported.

