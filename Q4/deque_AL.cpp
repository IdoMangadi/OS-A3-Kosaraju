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

void Graph::printSCCs(int client_fd) {
    stack<int> Stack;
    vector<bool> visited(V, false);

    for (int i = 0; i < V; i++) {
        if (!visited[i]) {
            fillOrder(i, visited, Stack);
        }
    }

    fill(visited.begin(), visited.end(), false);

    string result;
    while (!Stack.empty()) {
        int v = Stack.top();
        Stack.pop();

        if (!visited[v]) {
            vector<int> component;
            DFS(v, visited, component);

            sort(component.begin(), component.end());
            for (int i : component) {
                result += to_string(i + 1) + " ";
            }
            result += "\n";
        }
    }
    send(client_fd, result.c_str(), result.size(), 0);
}

extern "C" {
    Graph* create_graph(int V) {
        return new Graph(V);
    }

    void delete_graph(Graph* g) {
        delete g;
    }

    void add_edge(Graph* g, int v, int w) {
        g->addEdge(v, w);
        g->addEdgeReverse(v, w);
    }

    void remove_edge(Graph* g, int v, int w) {
        g->removeEdge(v, w);
    }

    void print_sccs(Graph* g, int client_fd) {
        g->printSCCs(client_fd);
    }
}
