#include "deque_AL.hpp"


string toLowerCase(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

void initGraph(Graph* g, int m) {
    for (int i = 0; i < m; i++) { // Read the edges
        int u, v;
        cin >> u >> v;
        g->addEdge(u - 1, v - 1); // Add edge from u to v
        g->addEdgeReverse(u - 1, v - 1); // Also add reverse edge for the transpose graph
    }
}

// Constructor initializes the graph with given number of vertices
Graph::Graph(int V) {
    this->V = V;
    adj.resize(V); // Resize the adjacency list to hold V vertices
    adjRev.resize(V); // Resize the reverse adjacency list to hold V vertices
}

// Function to add a directed edge from vertex v to vertex w
void Graph::addEdge(int v, int w) {
    adj[v].push_back(w); //add w to v's list
}

// Function to add a reverse edge from vertex w to vertex v
void Graph::addEdgeReverse(int v, int w) {
    adjRev[w].push_back(v);
}

// Function to remove a directed edge from vertex v to vertex w
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
    stack<int> Stack;
    vector<bool> visited(V, false);

    // Fill vertices in stack according to their finishing times
    for (int i = 0; i < V; i++) {
        if (!visited[i]) {
            fillOrder(i, visited, Stack);
        }
    }

    // Mark all the vertices as not visited (For the second DFS)
    fill(visited.begin(), visited.end(), false);

    // Now process all vertices in order defined by Stack
    while (!Stack.empty()) {
        int v = Stack.top();
        Stack.pop();

        // Print Strongly connected component of the popped vertex
        if (!visited[v]) {
            vector<int> component;
            DFS(v, visited, component);

            // Print the component
            sort(component.begin(), component.end());
            for (int i : component) {
                cout << i + 1 << " ";
            }
            cout << endl;
        }
    }
}

int main() {
    Graph* g = nullptr;

    string action;
    while (true) {
        cin >> action;
        action = toLowerCase(action); // Convert action to lower case
        if (action == "newgraph") {
            int n, m;
            cin >> n >> m; // Read number of vertices and edges
            if (g != nullptr) {
                delete g;
            }
            g = new Graph(n); // Create a new graph of n vertices
            initGraph(g, m);
        }
        else if (action == "kosaraju") {
            if (g == nullptr) {
                cout << "No graph to perform the operation\n";
                continue;
            }
            g->printSCCs(); // Print all strongly connected components
        }
        else if (action == "newedge") {
            int u, v;
            cin >> u >> v; // Read the edge vertices
            if (g != nullptr) {
                g->addEdge(u - 1, v - 1); // Add edge from u to v
                g->addEdgeReverse(u - 1, v - 1); // Add reverse edge for the transpose graph
            }
        }
        else if (action == "removeedge") {
            int u, v;
            cin >> u >> v; // Read the edge vertices
            if (g != nullptr) {
                g->removeEdge(u - 1, v - 1); // Remove edge from u to v
            }
        }
        else if (action == "exit") {
            if (g != nullptr) {
                delete g;
            }
            break;
        }
    }
    return 0;
}

