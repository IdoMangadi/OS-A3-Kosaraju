#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <fcntl.h>
#include "deque_AL.hpp"
#include "Patterns.hpp"

#define BOLD "\033[1m"
#define RESETCOLOR "\033[0m"

#define PORT "9034" // Port we're listening on

using namespace std;

// Global variables
Graph *g = nullptr;
Proactor proactor; // Create a proactor object on the stack

/**
 * Function to convert a string to lower case
 */
string toLowerCase(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::vector<std::string> splitStringBySpaces(const std::string &input) {
    std::istringstream stream(input);
    std::vector<std::string> result;
    std::string temp;

    while (stream >> temp) result.push_back(temp);

    return result;
}

/**
 * This function will insert to G M edges based on the input from the client.
 */
void initGraph(Graph *g, int m, int clientFd) {
    // Reading m edges from the client:
    for (int i = 0; i < m; i++) {
        // Sending instructions to the client:
        string msg = "Please enter the edge number " + to_string(i + 1) + " in the format: u v ";
        send(clientFd, msg.c_str(), msg.size(), 0);

        // Reading the msg from client in format: u v (edge from u to v)
        char buf[1024];  // Buffer for client data
        int bytesReceived = recv(clientFd, buf, sizeof buf, 0);
        if (bytesReceived <= 0) {
            if (bytesReceived == 0) printf("server: socket %d hung up\n", clientFd);
            else perror("recv");
            close(clientFd); // Bye!
        } else {  // We got data from a client:
            buf[bytesReceived] = '\0';  // Null-terminate the string
            string action = string(buf);
            vector<string> tokens = splitStringBySpaces(action);  // Split the input string by spaces
            int u = stoi(tokens[0]);
            int v = stoi(tokens[1]);
            g->addEdge(u - 1, v - 1);        // Add edge from u to v
            g->addEdgeReverse(u - 1, v - 1); // Add reverse edge for the transpose graph
        }
    }
}

pair<string, Graph *> handleInput(Graph *g, string action, int clientFd) {
    string msg;
    vector<string> tokens = splitStringBySpaces(action);  // Split the input string by spaces
    string realAction = tokens[0];  // Get the first token as the action
    int n, m;

    if (realAction == "newgraph") {  // format: newgraph n m
        n = stoi(tokens[1]);
        m = stoi(tokens[2]);
        cout << "Creating a new graph with " << n << " vertices and " << m << " edges" << endl;

        if (g != nullptr) delete g;
        g = new Graph(n); // Create a new graph of n vertices
        initGraph(g, m, clientFd);  // Initialize the graph with m edges

        msg = "Action completed: Graph initialized with " + to_string(m) + " edges\n";
        send(clientFd, msg.c_str(), msg.size(), 0);  // Sending the client finishing message:
        msg = "Client " + to_string(clientFd) + " successfully created a new Graph with " + to_string(n) + " vertices and " + to_string(m) + " edges";
        return {msg, g};
    } else if (realAction == "newedge") {  // format: newedge n m (add an edge from n to m)
        if (g != nullptr) {
            n = stoi(tokens[1]);
            m = stoi(tokens[2]);
            cout << "Adding an edge from " << n << " to " << m << endl;
            cout << &(*g);
            g->addEdge(n - 1, m - 1);        // Add edge from u to v
            g->addEdgeReverse(n - 1, m - 1); // Add reverse edge for the transpose graph
            msg = "Action completed: an edge added from " + to_string(n) + " to " + to_string(m) + "\n";
            send(clientFd, msg.c_str(), msg.size(), 0);
            msg = "Client " + to_string(clientFd) + " added an edge from " + to_string(n) + " to " + to_string(m);
        } else {
            msg = "Client " + to_string(clientFd) + " tried to perform the operation but there is no graph";
        }
        return {msg, nullptr};
    } else if (realAction == "removeedge") {  // format: removeedge n m (remove an edge from n to m)
        if (g != nullptr) {
            n = stoi(tokens[1]);
            m = stoi(tokens[2]);
            cout << "Removing an edge from " << n << " to " << m << endl;
            g->removeEdge(n - 1, m - 1); // Remove edge from u to v
            msg = "Action completed: an edge removed from " + to_string(n) + " to " + to_string(m);
            send(clientFd, msg.c_str(), msg.size(), 0);
            msg = "Client " + to_string(clientFd) + " removed an edge from " + to_string(n) + " to " + to_string(m) + "\n";
        } else {
            msg = "Client " + to_string(clientFd) + " tried to perform the operation but there is no graph";
        }
        return {msg, nullptr};
    } else if (realAction == "kosaraju") {  // format: kosaraju
        if (g == nullptr) {
            msg = "Client " + to_string(clientFd) + " tried to perform the operation but there is no graph";
            return {msg, nullptr};
        } else {
            msg = "Client " + to_string(clientFd) + " requested to print all strongly connected components";
            int stdout_save = dup(STDOUT_FILENO); // Save the current state of STDOUT
            int pipefd[2];
            pipe(pipefd);                   // Create a pipe
            dup2(pipefd[1], STDOUT_FILENO); // Redirect STDOUT to the pipe
            close(pipefd[1]);               // Close the write-end of the pipe as it's now duplicated

            g->printSCCs(); // This will write to the pipe instead of STDOUT

            // Restore the original STDOUT
            dup2(stdout_save, STDOUT_FILENO);
            close(stdout_save); // Close the saved STDOUT

            // Read from the pipe
            std::string output;
            char buffer[128];
            ssize_t bytes_read;
            while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0'; // Null-terminate the string
                output += buffer;
            }
            close(pipefd[0]); // Close the read-end of the pipe
            // Use 'output' as needed
            msg += "SCCs Output: \n" + output;
            send(clientFd, msg.c_str(), msg.size(), 0);
            return {msg, nullptr};
        }
    } else {
        msg = "Client " + to_string(clientFd) + " sent a message:" + action;
        return {msg, nullptr};
    }
}

// Get sockaddr, IPv4 or IPv6:
void *getInAddr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) return &(((struct sockaddr_in *)sa)->sin_addr);
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Return a listening socket
int getListenerSocket(void) {
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) continue;

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL) return -1;

    // Listen
    if (listen(listener, 10) == -1) return -1;

    return listener;
}

/**
 * Function to handle the message received from the client.
 * The available actions are: newgraph, newedge, removeedge, kosaraju.
 * This function will be called by the proactor for each ready client socket.
 * This function will read the message from the client and operate the action on the graph.
 * Parameter: fd - the file descriptor of the client socket.
 */
void *handleClientMessage(int clientFd) {
    ConcreteCompletionHandler handler;
    handler.handleEvent(clientFd);
    return nullptr;
}

/**
 * Function to handle the incoming connection.
 * Will accept the connection and add the new fd to the proactor.
 * Parameters: fd - the file descriptor of the listening socket, pointer to the proactor object so that we can add the new fd (client socket) to the proactor.
 */
void *handleIncomingConnection(void *arg) {
    int listener = *((int *)arg);

    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen = sizeof remoteaddr;
    
    int newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);  // Accept the incoming connection
    if (newfd == -1) {
        perror("accept");
        return nullptr;
    } else {
        // Print the client IP address:
        char remoteIP[INET6_ADDRSTRLEN];
        inet_ntop(remoteaddr.ss_family,
                  getInAddr((struct sockaddr *)&remoteaddr),
                  remoteIP, INET6_ADDRSTRLEN);
        printf("server: new connection from %s on socket %d\n", remoteIP, newfd);
        // Add the new fd to the proactor with the handleClientMessage function as the callback function:
        proactor.startProactor(newfd, handleClientMessage);
        string message = "Welcome to the server. Please enter your action\n";
        if (send(newfd, message.c_str(), message.size(), 0) == -1) {
            perror("send");
        }
    }
    return nullptr;
}

/**
 * Main function to test the proactor pattern.
 * The idea is to create a server that listens to incoming connections.
 * When a connection is accepted, the server will add the new fd to the proactor as a client socket.
 */
int main() {
    int listener = getListenerSocket();  // Listening socket descriptor
    cout << "Server is running on port " << PORT << endl;
    if (listener == -1) {
        fprintf(stderr, "error getting listener socket\n");
        exit(1);
    }

    // Start the proactor with the listener socket and handleIncomingConnection function as the callback
    pthread_t proactorThread = proactor.startProactor(listener, handleIncomingConnection);

    // Wait for the proactor thread to finish
    pthread_join(proactorThread, nullptr);

    // Clean up and exit
    close(listener);
    return 0;
}
