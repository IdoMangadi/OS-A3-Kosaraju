/**
 * By the profiling results, the best implementation is the one that uses the deque of deques to store the adjacency list.
 */
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
    void printSCCs(); // Function to print all strongly connected components
    Graph getTranspose(); // Function to create a transpose of the graph
    void removeEdge(int v, int w);
};

// Constructor initializes the graph with given number of vertices
Graph::Graph(int V) {
    this->V = V;
    adj.resize(V); // Resize the adjacency list to hold V vertices
    adjRev.resize(V); // Resize the reverse adjacency list to hold V vertices
}

// Function to add a directed edge from vertex v to vertex w
void Graph::addEdge(int v, int w) {
    adj[v].push_back(w); //add w to v's list
    adjRev[w].push_back(v); //add v to w's list

}

// Function to add a reverse edge from vertex w to vertex v

void Graph::removeEdge(int v, int w){
    adj[v].erase(remove(adj[v].begin(), adj[v].end(), w), adj[v].end());
    adjRev[w].erase(remove(adjRev[w].begin(), adjRev[w].end(), v), adjRev[w].end());
}

// Fill order for vertices in DFS based on their finish times
void Graph::fillOrder(int v, vector<bool>& visited, stack<int>& Stack) {
    visited[v] = true; // Mark the current node as visited
    for (int i : adj[v]) { // For all vertices adjacent to v
        if (!visited[i]) { // If they haven't been visited
            fillOrder(i, visited, Stack); // Recur
        }
    }
    Stack.push(v); // Push current vertex to stack which stores result
}

// A function to perform a depth-first search on the reversed graph
void Graph::DFS(int v, vector<bool>& visited, vector<int>& component) {
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
    vector<bool> visited(V, false); // Track visited vertices

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
            vector<int> component;
            DFS(v, visited, component); // Find all reachable vertices
            sort(component.begin(), component.end()); // Optional: sort the component
            for (int i : component) {
                cout << i + 1 << " "; // Print the component
            }
            cout << endl;
        }
    }
}
void initGraph(Graph* g, int n, int m){
    for (int i = 0; i < m; i++) { // Read the edges
        int u, v;
        cin >> u >> v;
        g->addEdge(u - 1, v - 1); // Add edge from u to v
    }
}

int main() {
    Graph* g = nullptr;
    
    string action;
    string indexes;
    while(true){
        cin >> action;
        if(action == "NewGraph"){
            cin >> indexes; // Read the edge vertices
            int n = indexes[0] - '0';
            int m = indexes[2] - '0';
            Graph g1(n); // Create a graph of n vertices
            // g = new Graph(n); // Create a graph of n vertices
            g = &g1;
            initGraph(g, n, m);
        }
        else if(action == "Kosaraju"){
            if (g==nullptr)
            {
                cout << "No graph to perform the operation\n";
                continue;
            }
            
            g->printSCCs(); // Print all strongly connected components
        }
        else if(action == "Newedge"){
             cin >> indexes; // Read the edge vertices
            int n = indexes[0] - '0';
            int m = indexes[2] - '0';
            g->addEdge(n-1, m-1); // Add edge from u to v
        }
        else if(action == "Removeedge"){
             cin >> indexes; // Read the edge vertices
            int n = indexes[0] - '0';
            int m = indexes[2] - '0';
            g->removeEdge(n-1, m-1); // Add edge from u to v
        }
        else if(action == "Exit"){
            break;
        }
}
 return 0;
}
