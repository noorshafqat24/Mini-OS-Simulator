# Mini OS Simulator

## Project Overview
This project is a simulation of core operating system concepts including:
- Process synchronization
- CPU scheduling
- Deadlock prevention
- Resource management

The system integrates multiple OS algorithms into a single working simulator.

---

## Features

### 1. Producer–Consumer (Bounded Buffer)
- Implemented using semaphores and mutex
- No busy waiting
- At least 2 producer threads and 1 consumer thread
- Ready queue implemented as bounded buffer

### 2. CPU Scheduling
Scheduling rule:
- If number of ready processes ≤ 5 → Priority Scheduling
- If number of ready processes > 5 → Round Robin Scheduling (quantum = 4)

Outputs:
- Gantt chart
- Waiting Time (WT)
- Turnaround Time (TAT)
- Average WT and TAT

### 3. Deadlock Prevention (Banker’s Algorithm)
- Checks system safety before granting resources
- Displays safe sequence
- Unsafe processes moved to blocked queue
- Blocked processes retried after resource release

### 4. Menu Driven Interface
User can:
1. Start Simulation  
2. Add Process  
3. Display State  
4. Exit  

---

## Technologies Used
- C++
- POSIX Threads
- Semaphores
- Mutex
- GNU Compiler (g++)

---

## Build and Run (Ubuntu)

```bash
make clean
make
./sim

