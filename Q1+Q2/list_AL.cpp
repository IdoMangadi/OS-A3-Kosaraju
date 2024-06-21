#include <iostream>
#include <list>
#include <vector>
#include <stack>
#include <algorithm>
#include <iterator>

using namespace std;

class Graph {
    int V; // Number of vertices
    vector<list<int>> adj; // Adjacency list using std::list
    vector<list<int>> adjRev; // Reverse adjacency list

    void fillOrder(int v, vector<bool>& visited, stack<int>& Stack);
    void DFS(int v, vector<bool>& visited, vector<int>& component);

public:
    Graph(int V);
    void addEdge(int v, int w);
    void addEdgeReverse(int v, int w);
    void printSCCs();
};

Graph::Graph(int V) : V(V), adj(V), adjRev(V) {}

void Graph::addEdge(int v, int w) {
    adj[v].push_back(w); // Add w to v’s list
}

void Graph::addEdgeReverse(int v, int w) {
    adjRev[w].push_back(v); // Add v to w’s reverse list
}

void Graph::fillOrder(int v, vector<bool>& visited, stack<int>& Stack) {
    visited[v] = true;
    for (int neighbor : adj[v]) {
        if (!visited[neighbor]) {
            fillOrder(neighbor, visited, Stack);
        }
    }
    Stack.push(v);
}

void Graph::DFS(int v, vector<bool>& visited, vector<int>& component) {
    visited[v] = true;
    component.push_back(v);
    for (int neighbor : adjRev[v]) {
        if (!visited[neighbor]) {
            DFS(neighbor, visited, component);
        }
    }
}

void Graph::printSCCs() {
    stack<int> Stack;
    vector<bool> visited(V, false);

    // Fill vertices in stack according to their finishing times
    for (int i = 0; i < V; ++i) {
        if (!visited[i]) {
            fillOrder(i, visited, Stack);
        }
    }

    // Create a reversed graph
    fill(visited.begin(), visited.end(), false);

    // Now process all vertices in order defined by Stack
    while (!Stack.empty()) {
        int v = Stack.top();
        Stack.pop();
        if (!visited[v]) {
            vector<int> component;
            DFS(v, visited, component);
            sort(component.begin(), component.end());
            for (int vert : component) {
                cout << vert + 1 << " ";
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
