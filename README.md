
# SimOS — Operating Systems Course Project

This C++ project was developed for an **Operating Systems course**.

The `SimOS` interface models processes, CPU scheduling, disk I/O, and paged memory in a simulated computer.

## Features

- **Process management:** Create processes, fork children, exit, and wait for child termination. The interface specifies zombie handling and cascading termination of descendants.
- **CPU scheduling:** Track the running process and ready queue, with timer interrupts ending the current process's time slice.
- **Disk I/O:** Submit file-read requests, track each disk's active request and waiting queue, and handle job completion.
- **Memory management:** Access logical addresses through pages and frames. The header includes tracking for least recently used (LRU) page replacement.
- **State inspection:** Retrieve the current CPU process, ready queue, memory usage, disk activity, and disk queues.

## SimOS Interface

Initialize the simulator with a disk count, RAM size, and page size:

```cpp
SimOS os(2, 4096, 1024);
```

| Methods | Purpose |
| --- | --- |
| `NewProcess()`, `SimFork()` | Create a process or fork a child. |
| `SimExit()`, `SimWait()` | Terminate the running process or wait for a child. |
| `TimerInterrupt()` | End the running process's time slice. |
| `DiskReadRequest(diskNumber, fileName)` | Request a simulated file read. |
| `DiskJobCompleted(diskNumber)` | Report completion of a disk job. |
| `AccessMemoryAddress(address)` | Access a logical memory address. |
| `GetCPU()`, `GetReadyQueue()`, `GetMemory()` | Inspect CPU, ready queue, and memory state. |
| `GetDisk(diskNumber)`, `GetDiskQueue(diskNumber)` | Inspect disk activity and queued requests. |

Process IDs begin at 1. `NO_PROCESS` is 0, indicating an idle CPU. Disk, page, and frame numbering begins at 0.

## Project Files

| File | Purpose |
| --- | --- |
| `SimOS.cpp` | SimOS implementation. |
| `SimOS.h` | Header containing SimOS declarations. |
| `main.cpp` | Program entry point. |
| `runme` | Compiled executable, assuming it was built from the source files. |

## Requirements

- A C++ compiler such as `g++` or `clang++` to build from source.
- A terminal.
- Keep `SimOS.cpp`, `SimOS.h`, and `main.cpp` in the same directory.

## Build and Run

Open a terminal in the project directory and compile the source files:

```bash
g++ -std=c++17 -Wall -Wextra SimOS.cpp main.cpp -o runme
```

Then start the program:

```bash
./runme
```

The program's behavior and any interactive commands are defined by `main.cpp`.

`SimOS.h` is included by the source files and does not need to be compiled separately. Recompile after changing the source files.

### Using the Existing Executable (macOS/Linux)

If the included `runme` is an executable built for your operating system and processor, run:

```bash
./runme
```

If you receive a permission error:

```bash
chmod +x runme
./runme
```

If the executable is incompatible with your computer, rebuild it using the command above.

### Windows (MinGW g++)

From the project directory in PowerShell:

```powershell
g++ -std=c++17 -Wall -Wextra SimOS.cpp main.cpp -o runme.exe
.\runme.exe
```

## Troubleshooting

- **Compiler not found:** Install a C++ compiler and make sure it is available in your terminal. On macOS, Apple's command-line developer tools provide `clang++`; you can substitute `clang++` for `g++` in the build command.
- **Source or header file not found:** Open the terminal in the folder containing all three source files.
- **Changes do not appear:** Recompile before running the program again.

> These build instructions assume the project requires no additional libraries and is compatible with C++17. Check the course requirements for any additional compiler flags or dependencies.
