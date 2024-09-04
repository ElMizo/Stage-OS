# Stage-OS

---

# Process Management and Scheduling

## Overview

This document provides a detailed overview of the process management and scheduling system implemented in the `process.c` file. The file includes functions for creating, managing, and scheduling processes in a multi-tasking environment. This README explains the key functions and the scheduling algorithm used, supported by diagrams to illustrate the process flow.

## Scheduling Algorithm

### Round Robin Scheduling

The scheduling algorithm implemented in this system is **Round Robin (RR)**. Round Robin is one of the simplest and most widely used CPU scheduling algorithms, especially in time-sharing systems. In this algorithm, each process is assigned a fixed time slot (quantum) during which it can execute. Once the time slot expires, the process is preempted, and the next process in the queue is given the CPU.

### How It Works

1. **Process Queue**: All ready processes are maintained in a circular queue called `ready_list`. The head of this queue is the process currently being executed.

2. **Time Quantum**: Each process is given a fixed time quantum. If the process does not complete within this time, it is preempted.

3. **Context Switch**: When a process's time quantum expires or it voluntarily yields the CPU (via `process_yield()`), a context switch occurs. The current process's state is saved, and the next process in the `ready_list` is selected for execution.

4. **Preemption**: The `process_preempt()` function is called to handle preemption. If preemption is allowed and there is a ready process in the queue, the current process is moved to the `PROCESS_STATE_READY` state, and the next process is selected.

5. **Grave List**: Completed or terminated processes are moved to the `grave_list` where they are eventually cleaned up by the `process_reap_all()` function.

### Diagram: Process Flow in Round Robin Scheduling

```
         +---------------------------+
         |      Start / Idle State    |
         +---------------------------+
                       |
                       V
         +---------------------------+
         |  Select Process from Queue |
         +---------------------------+
                       |
                       V
         +---------------------------+
         |     Execute Process        |
         +---------------------------+
                       |
                       V
         +---------------------------+
         |  Time Quantum Expired?     |
         +---------------------------+
                       |
           +-----------+------------+
           |                        |
           V                        V
+----------------------+  +----------------------+
| Preempt & Save State  |  |   Process Completed   |
|   (process_yield)     |  |  Move to Grave List   |
+----------------------+  +----------------------+
           |                        |
           V                        |
   +-------------------------+      |
   |   Add Process to End of  |      |
   |   Ready List (process_   |      |
   |   launch)                |      |
   +-------------------------+      |
           |                        |
           V                        |
  +---------------------------------+
  |     Select Next Process         |
  +---------------------------------+
```

### Function Descriptions

#### `process_create()`

- **Purpose**: Allocates and initializes a new process. This function sets up the process's page table, stack, and process state. It is the entry point for creating any new process in the system.
- **Key Operations**:
  - Allocates a unique Process ID (PID) using `process_allocate_pid()`.
  - Sets up the initial kernel stack and page table for the process.
  - Initializes the process state to `PROCESS_STATE_READY`.

#### `process_yield()`

- **Purpose**: Yields the CPU from the current process, allowing the scheduler to switch to the next process in the queue.
- **Key Operations**:
  - Saves the current process's state.
  - Switches the process state to `PROCESS_STATE_READY`.
  - Selects the next process in the `ready_list` for execution.

#### `process_preempt()`

- **Purpose**: Handles preemption in the Round Robin scheduler. If a process's time quantum expires, it is preempted, and the next process in the queue is selected.
- **Key Operations**:
  - Checks if preemption is allowed.
  - If allowed, switches the current process to `PROCESS_STATE_READY` and selects the next process.

#### `process_delete()`

- **Purpose**: Deletes a process, freeing its resources. This function is used to clean up processes that have terminated.
- **Key Operations**:
  - Closes all open kernel objects associated with the process.
  - Frees the

process's page table, stack, and any other allocated resources.
  - Moves the process to the `grave_list` for final cleanup by `process_reap_all()`.

#### `process_reap_all()`

- **Purpose**: Cleans up all processes in the `grave_list`. This function ensures that all terminated processes have their resources properly freed and removed from the system.
- **Key Operations**:
  - Iterates through the `grave_list`, deallocating each process's resources.
  - Removes each process from the system.

#### 'New command line '
-The clear command in this code is handled within the 'kshell_execute()' function and is used to clear the terminal screen. This is achieved by resetting the console to its initial state. 
-" command Implementation: " :
   - The 'strcmp(cmd, "clear")' checks if the command is "clear".
   - When the clear command matches,the terminal screen is cleared by calling the 'console_reset()'function.
   - 
![clre](https://github.com/user-attachments/assets/604aae47-6879-4596-9a64-03087a5d6957)
![clear](https://github.com/user-attachments/assets/d18b34ec-353f-4ac0-a1d6-48c533440eb1)
     
## Conclusion

The `process.c` file implements a basic yet efficient process management system with Round Robin scheduling. The algorithm ensures that all processes receive equal CPU time, making it ideal for time-sharing systems. This README provides a comprehensive overview of how the scheduling works and the key functions involved in managing processes.

---

