#include <iostream>
#include <list>
#include <vector>
#include <stack>
#include <iterator>
#include <algorithm>

using namespace std;

class Graph {
    int V; // Number of vertices
    list<list<bool>> adj; // Adjacency matrix
    list<list<bool>> adjRev; // Reverse adjacency matrix

    void fillOrder(int v, vector<bool>& visited, stack<int>& Stack);
    void DFS(int v, vector<bool>& visited, vector<int>& component);

public:
    Graph(int V);
    void addEdge(int v, int w);
    void addEdgeReverse(int v, int w);
    void printSCCs();
};

Graph::Graph(int V) : V(V) {
    for (int i = 0; i < V; i++) {
        adj.push_back(list<bool>(V, false));
        adjRev.push_back(list<bool>(V, false));
    }
}

void Graph::addEdge(int v, int w) {
    auto it = next(adj.begin(), v);
    auto listIt = next(it->begin(), w);
    *listIt = true;
}

void Graph::addEdgeReverse(int v, int w) {
    auto it = next(adjRev.begin(), w);
    auto listIt = next(it->begin(), v);
    *listIt = true;
}

void Graph::fillOrder(int v, vector<bool>& visited, stack<int>& Stack) {
    visited[v] = true;
    int i = 0;
    for (auto &row : adj) {
        if (i++ == v) {
            int j = 0;
            for (auto val : row) {
                if (val && !visited[j]) {
                    fillOrder(j, visited, Stack);
                }
                j++;
            }
            break;
        }
    }
    Stack.push(v);
}

void Graph::DFS(int v, vector<bool>& visited, vector<int>& component) {
    visited[v] = true;
    component.push_back(v);
    int i = 0;
    for (auto &row : adjRev) {
        if (i++ == v) {
            int j = 0;
            for (auto val : row) {
                if (val && !visited[j]) {
                    DFS(j, visited, component);
                }
                j++;
            }
            break;
        }
    }
}

void Graph::printSCCs() {
    stack<int> Stack;
    vector<bool> visited(V, false);

    for (int i = 0; i < V; i++) {
        if (!visited[i]) {
            fillOrder(i, visited, Stack);
        }
    }

    fill(visited.begin(), visited.end(), false);

    while (!Stack.empty()) {
        int v = Stack.top();
        Stack.pop();
        if (!visited[v]) {
            vector<int> component;
            DFS(v, visited, component);
            sort(component.begin(), component.end());
            for (int i : component) {
                cout << i + 1 << " ";
            }
            cout << endl;
        }
    }
}

int main() {
    int n, m;
    cin >> n >> m;
    Graph g(n);
    for (int i = 0; i < m; i++) {
        int u, v;
        cin >> u >> v;
        g.addEdge(u - 1, v - 1);
        g.addEdgeReverse(v - 1, u - 1);
    }

    g.printSCCs();

    return 0;
}
