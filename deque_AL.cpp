#include <iostream>
#include <deque>
#include <stack>
#include <algorithm>

using namespace std;

// Class definition for a directed graph
class Graph {
    
    int V; // Number of vertices in the graph
    deque<deque<int>> adj; // Adjacency list for storing edges
    deque<deque<int>> adjRev; // Adjacency list for storing reverse edges

    void fillOrder(int v, deque<bool>& visited, stack<int>& Stack); // Helper function for DFS to store vertices by finish time
    void DFS(int v, deque<bool>& visited, deque<int>& component); // Recursive function for standard DFS

public:
    Graph(int V); // Constructor to initialize a graph with V vertices
    void addEdge(int v, int w); // Function to add an edge from v to w
    void addEdgeReverse(int v, int w); // Function to add a reverse edge from w to v for the transpose graph
    void printSCCs(); // Function to print all strongly connected components
    Graph getTranspose(); // Function to create a transpose of the graph
};

// Constructor initializes the graph with given number of vertices
Graph::Graph(int V) {
    this->V = V;
    adj.resize(V); // Resize the adjacency list to hold V vertices
    adjRev.resize(V); // Resize the reverse adjacency list to hold V vertices
}

// Function to add a directed edge from vertex v to vertex w
void Graph::addEdge(int v, int w) {
    adj[v].push_back(w);
}

// Function to add a reverse edge from vertex w to vertex v
void Graph::addEdgeReverse(int v, int w) {
    adjRev[w].push_back(v);
}

// Fill order for vertices in DFS based on their finish times
void Graph::fillOrder(int v, deque<bool>& visited, stack<int>& Stack) {
    visited[v] = true; // Mark the current node as visited
    for (int i : adj[v]) { // For all vertices adjacent to v
        if (!visited[i]) { // If they haven't been visited
            fillOrder(i, visited, Stack); // Recur
        }
    }
    Stack.push(v); // Push current vertex to stack which stores result
}

// A function to perform a depth-first search on the reversed graph
void Graph::DFS(int v, deque<bool>& visited, deque<int>& component) {
    visited[v] = true; // Mark the current node as visited
    component.push_back(v); // Add it to the component
    for (int i : adjRev[v]) { // Traverse its adjacency list
        if (!visited[i]) { // For unvisited vertices
            DFS(i, visited, component); // Perform DFS
        }
    }
}

// Print all strongly connected components
void Graph::printSCCs() {
    stack<int> Stack; // Stack to store the vertices based on finish times
    deque<bool> visited(V, false); // Track visited vertices

    for (int i = 0; i < V; i++) { // Order the vertices as per their finish times
        if (!visited[i]) {
            fillOrder(i, visited, Stack);
        }
    }

    fill(visited.begin(), visited.end(), false); // Reset visited for the second pass

    while (!Stack.empty()) { // Process all vertices in order defined by Stack
        int v = Stack.top(); // Pop a vertex from stack
        Stack.pop();
        if (!visited[v]) { // If not yet visited
            deque<int> component;
            DFS(v, visited, component); // Find all reachable vertices
            sort(component.begin(), component.end()); // Optional: sort the component
            for (int i : component) {
                cout << i + 1 << " "; // Print the component
            }
            cout << endl;
        }
    }
}

int main() {
    int n, m; // Number of vertices and edges
    cin >> n >> m; // Read number of vertices and edges
    Graph g(n); // Create a graph of n vertices
    for (int i = 0; i < m; i++) { // Read the edges
        int u, v;
        cin >> u >> v;
        g.addEdge(u - 1, v - 1); // Add edge from u to v
        g.addEdgeReverse(u - 1, v - 1); // Also add reverse edge for the transpose graph
    }
    cout << "\n";
    g.printSCCs(); // Print all strongly connected components

    return 0;
}
