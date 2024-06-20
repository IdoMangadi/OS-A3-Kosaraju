// deque_AL.cpp
#include "deque_AL.hpp"
#include "sys/socket.h"

Graph::Graph(int V) {
    this->V = V;
    adj.resize(V);
    adjRev.resize(V);
}

void Graph::addEdge(int v, int w) {
    adj[v].push_back(w);
}

void Graph::addEdgeReverse(int v, int w) {
    adjRev[w].push_back(v);
}

void Graph::removeEdge(int v, int w) {
    adj[v].erase(remove(adj[v].begin(), adj[v].end(), w), adj[v].end());
    adjRev[w].erase(remove(adjRev[w].begin(), adjRev[w].end(), v), adjRev[w].end());
}

void Graph::fillOrder(int v, vector<bool>& visited, stack<int>& Stack) {
    visited[v] = true;
    for (int i : adj[v]) {
        if (!visited[i]) {
            fillOrder(i, visited, Stack);
        }
    }
    Stack.push(v);
}

void Graph::DFS(int v, vector<bool>& visited, vector<int>& component) {
    visited[v] = true;
    component.push_back(v);
    for (int i : adjRev[v]) {
        if (!visited[i]) {
            DFS(i, visited, component);
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


