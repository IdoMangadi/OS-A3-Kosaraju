// deque_AL.hpp
#ifndef DEQUE_AL_HPP
#define DEQUE_AL_HPP

#include <iostream>
#include <deque>
#include <stack>
#include <algorithm>
#include <vector>

using namespace std;

// Class definition for a directed graph
class Graph {
    int V; // Number of vertices in the graph
    deque<deque<int>> adj; // Adjacency list for storing edges
    deque<deque<int>> adjRev; // Adjacency list for storing reverse edges

    void fillOrder(int v, vector<bool>& visited, stack<int>& Stack); // Helper function for DFS to store vertices by finish time
    void DFS(int v, vector<bool>& visited, vector<int>& component); // Recursive function for standard DFS

public:
    Graph(int V); // Constructor to initialize a graph with V vertices
    void addEdge(int v, int w); // Function to add an edge from v to w
    void addEdgeReverse(int v, int w); // Function to add a reverse edge from w to v for the transpose graph
    void printSCCs(int client_fd); // Function to print all strongly connected components
    void removeEdge(int v, int w); // Function to remove an edge from v to w
};

extern "C" {
    Graph* create_graph(int V);
    void delete_graph(Graph* g);
    void add_edge(Graph* g, int v, int w);
    void remove_edge(Graph* g, int v, int w);
    void print_sccs(Graph* g, int client_fd);
}

#endif // DEQUE_AL.HPP
