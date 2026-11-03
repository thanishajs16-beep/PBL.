# Multi-Process Simulator and IPC

This project implements a simple CPU simulator with memory, stack, and queue operations. It uses **socket-based IPC** instead of message queues to communicate between the core process and the component processes.

## Files

- `core.c` – handles commands and coordinates the component processes through sockets.
- `cpu.c` – implements CPU instructions and arithmetic operations.
- `Memory.c` – implements memory read and write operations.
- `Stack.c` – implements stack operations.
- `Queue.c` – implements queue operations.
- `test_core.c` – sends commands to the core process through a socket.

## Compile

Keep all six `.c` files in the same folder. Open two terminals in that folder.

In the first terminal, compile the core program:

```bash
gcc -Wall -Wextra -std=c11 core.c cpu.c Memory.c Stack.c Queue.c -o core
```

In the second terminal, compile the test program:

```bash
gcc -Wall -Wextra -std=c11 test_core.c -o test_core
```

The two programs are compiled separately because both `core.c` and `test_core.c` contain a `main()` function.

## Run

In the first terminal:

```bash
./core
```

In the second terminal:

```bash
./test_core
```

Enter commands at the `>` prompt.

## Available Commands

**CPU**

- `LOAD 10` – loads a value into the accumulator.
- `ADD 10` – adds a value to the accumulator.
- `ADD 10 20` – adds two values and stores the result in the accumulator.
- `SUB`, `MUL`, `DIV` – perform subtraction, multiplication, and division. These accept one or two operands like `ADD`.
- `HALT` – stops CPU instruction execution.

**Memory**

- `STORE 5 99` – stores `99` at memory address `5`.
- `READ 5` – reads the value at memory address `5`.

**Stack**

- `PUSH 50` – pushes a value onto the stack.
- `POP` – removes the top value.
- `PEEK` – displays the top value without removing it.

**Queue**

- `ENQUEUE 50` – adds a value to the queue.
- `DEQUEUE` – removes the front value.
- `QPEEK` – displays the front value without removing it.

**Other**

- `HELP` – displays the command list.
- `EXIT` – closes the test program.
- `SHUTDOWN` – closes the core process.

The program uses local Unix domain sockets and requires a Linux environment such as Ubuntu or WSL.
