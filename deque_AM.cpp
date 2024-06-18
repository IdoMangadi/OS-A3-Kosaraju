#include <iostream>
#include <deque>
#include <stack>
#include <algorithm>

using namespace std;

/**
 * @class Graph
 * @brief A class to represent a directed graph using adjacency list representation
 */
class Graph {
    int V; // Number of vertices in the graph
    deque<deque<bool>> adj; // Adjacency matrix for storing edges
    deque<deque<bool>> adjRev; // Adjacency matrix for storing reverse edges

    /**
     * @brief Helper function for DFS to store vertices by finish time
     * @param v Current vertex
     * @param visited Visited vertices tracker
     * @param Stack Stack to store vertices by finish time
     */
    void fillOrder(int v, deque<bool>& visited, stack<int>& Stack);

    /**
     * @brief Recursive function for standard DFS
     * @param v Current vertex
     * @param visited Visited vertices tracker
     * @param component deque to store the current component
     */
    void DFS(int v, deque<bool>& visited, deque<int>& component);

public:
    /**
     * @brief Constructor to initialize a graph with V vertices
     * @param V Number of vertices
     */
    Graph(int V);

    /**
     * @brief Function to add an edge from v to w
     * @param v Source vertex
     * @param w Destination vertex
     */
    void addEdge(int v, int w);

    /**
     * @brief Function to add a reverse edge from w to v for the transpose graph
     * @param v Source vertex
     * @param w Destination vertex
     */
    void addEdgeReverse(int v, int w);

    /**
     * @brief Function to print all strongly connected components
     */
    void printSCCs();
};

Graph::Graph(int V) {
    this->V = V;
    adj.resize(V, deque<bool>(V, false)); // Resize the adjacency matrix to hold V vertices
    adjRev.resize(V, deque<bool>(V, false)); // Resize the reverse adjacency matrix to hold V vertices
}

void Graph::addEdge(int v, int w) {
    adj[v][w] = true; // Add an edge from v to w
}

void Graph::addEdgeReverse(int v, int w) {
    adjRev[w][v] = true; // Add a reverse edge from w to v
}

void Graph::fillOrder(int v, deque<bool>& visited, stack<int>& Stack) {
    visited[v] = true; // Mark the current node as visited
    for (int i = 0; i < V; i++) {
        if (adj[v][i] && !visited[i]) { // For all vertices adjacent to v
            fillOrder(i, visited, Stack); // Recur
        }
    }
    Stack.push(v); // Push current vertex to stack which stores result
}

void Graph::DFS(int v, deque<bool>& visited, deque<int>& component) {
    visited[v] = true; // Mark the current node as visited
    component.push_back(v); // Add it to the component
    for (int i = 0; i < V; i++) {
        if (adjRev[v][i] && !visited[i]) { // For unvisited vertices
            DFS(i, visited, component); // Perform DFS
        }
    }
}

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