# Project README

## Overview

This project simulates an operating system with an "oss" (Operating System Simulator) process and multiple "worker" processes. The oss manages a system clock, maintains a process table, and sends messages to workers. Workers simulate user processes, perform tasks, and terminate after a specified runtime.

## Usage

### Compilation

```bash
make
```

### Running the Simulation

1. **Run the `oss` process:**

   ```bash
   ./oss -n 3 -s 5 -t 7 -i 100 -f log.txt
   ```

2. **Open a terminal for each worker:**

   ```bash
   ./worker 5 500000
   ```

### Cleanup

```bash
make clean
```

## Example

- Simulate an environment with 3 workers:

   ```bash
   ./oss -n 3 -s 5 -t 7 -i 100 -f log.txt
   ```

- Launch a worker with a runtime of 5 seconds and 500,000 nanoseconds:

   ```bash
   ./worker 5 500000
   ```

## Output

- `oss` prints system information, process table, and messages.
- `worker` processes print runtime information, iteration progress, and termination status.


## Features

- **Timeout:** Simulation terminates after 60 seconds.
- **Cleanup:** Press `Ctrl-C` to gracefully terminate the simulation.

