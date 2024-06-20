# OS-A3-Kosaraju
Course - Operation Systems - Assignment 3- Kosaraju

# Kosaraju's Algorithm Implementation and Extensions

This project involves the implementation of Kosaraju-Sharir's algorithm for finding strongly connected components (SCCs) in a directed graph. It is extended to support graph modifications, TCP communication, and multiple clients, culminating in a multi-threaded server that handles dynamic graph operations efficiently.

## Overview

Kosaraju's algorithm is a two-pass algorithm that uses depth-first search to determine the SCCs of a directed graph. The implementation progresses through several stages:

1. **Basic Implementation**: Implementing the algorithm to read graph data from standard input and output the SCCs.
2. **Profiling**: Comparing the performance of different list implementations in C++ (e.g., `std::deque` vs. `std::list`).
3. **Interactive Server**: Extending the application to read commands from standard input to manipulate the graph and compute SCCs on demand.
4. **Multi-user Support**: Integrating with a simple chat protocol to allow multiple users to interact with a shared graph simultaneously over a network.
5. **Reactor Pattern Library**: Building a library for the reactor pattern to manage file descriptors efficiently.
6. **Proactor Pattern Library**: Extending the library to include the proactor pattern, focusing on asynchronous I/O operations.
7. **Multi-threading**: Implementing the server to handle multiple clients using threads, ensuring thread safety with mutexes.
8. **Producer-Consumer Monitoring**: Adding functionality to monitor the graph state and notify when a significant portion of the graph forms a single SCC or when it does not after modifications.

## Setup and Compilation

### Dependencies

- C++ Compiler (g++ recommended)
- Make

### Building the Project

Run the following command in the terminal:

```bash
make all
```

This will compile all source files and link the necessary objects into an executable named `kosaraju`.

## Usage
To start the server, run:

```bash
./kosaraju
```
The server will then wait for input commands. Commands are structured as follows:

- `Newgraph n,m`: Initializes a new graph with `n` vertices and `m` edges.
- `Newedge i,j`: Adds a directed edge from vertex `i` to vertex `j`.
- `Removeedge i,j`: Removes the directed edge from vertex `i` to vertex `j`.
- `Kosaraju`: Runs Kosaraju's algorithm on the current graph and outputs the SCCs.

## Features

- **Dynamic Graph Manipulation**: Users can modify the graph in real-time and compute the SCCs based on the current state.
- **Network Communication**: Supports handling multiple clients over TCP, allowing concurrent graph manipulations.
- **Thread Safety**: Ensures that graph computations and modifications are thread-safe using mutexes.
- **Scalability**: Utilizes the reactor and proactor patterns to efficiently manage multiple client connections and ensure responsiveness across networked operations.
- **Interactivity**: Supports real-time interaction with the graph via standard input commands, providing a responsive and user-driven experience.

## Cleaning Up

To clean up the build files, you can use:

```bash
make clean
```
This command will remove all compiled object files and executable files, keeping your directory clean.

## Conclusion
This project not only implements a fundamental graph algorithm but also extends it to operate in a dynamic, multi-user, networked environment. By doing so, it demonstrates practical applications of system programming concepts and network programming, making it an excellent educational tool for understanding complex algorithms and their real-world implications.
