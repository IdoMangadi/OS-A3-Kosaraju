
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <fcntl.h>

#include "deque_AL.hpp"
#include "Patterns.hpp"

#define BOLD "\033[1m"
#define RESETCOLOR "\033[0m"

#define PORT "9034" // Port we're listening on

using namespace std;

// creating a global variable to store the graph:
Graph *g = nullptr;
Reactor reactor; // Create a reactor object on the stack
const vector<string> graphActions = {"newgraph", "newedge", "removeedge", "kosaraju"};

/**
 * function to convert a string to lower case
 */
string toLowerCase(string s)
{
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}
std::vector<std::string> splitStringBySpaces(const std::string &input)
{
    std::istringstream stream(input);
    std::vector<std::string> result;
    std::string temp;

    while (stream >> temp)
        result.push_back(temp);

    return result;
}

/**
 * this function will insert to G M edges based on the input from the client.
 */
void initGraph(Graph *g, int m, int clientFd)

{
    // sending instructions to the client:
    string msg = "Please enter the edges in the format: u v\n";
    send(clientFd, msg.c_str(), msg.size(), 0);
    // reading m edges from the client:
    for (int i = 0; i < m; i++)
    {
        
       
        // raading the msg from client in format: u v (edge from u to v)
        char buf[1024]; // Buffer for client data
        int bytesReceived = recv(clientFd, buf, sizeof buf, 0);
        if (bytesReceived <= 0)
        {
            if (bytesReceived == 0)
                printf("server: socket %d hung up\n", clientFd);
            else
                perror("recv");
            close(clientFd); // Bye!
            reactor.removeFdFromReactor(clientFd);
        }
        else
        {                              // We got data from a client:
            buf[bytesReceived] = '\0'; // Null-terminate the string
            string action = string(buf);
            vector<string> tokens = splitStringBySpaces(action); // Split the input string by spaces
            int u = stoi(tokens[0]);
            int v = stoi(tokens[1]);
            g->addEdge(u - 1, v - 1);        // Add edge from u to v
            g->addEdgeReverse(u - 1, v - 1); // Add reverse edge for the transpose graph
        }
    }
}

pair<string, Graph *> newGraph(int n, int m, int clientFd, Graph *g)
{
    cout << "Creating a new graph with " << n << " vertices and " << m << " edges" << endl;

    if (g != nullptr)
        delete g;
    g = new Graph(n);          // Create a new graph of n vertices
    initGraph(g, m, clientFd); // Initialize the graph with m edges

    string msg = "successfully created a new Graph with " + to_string(n) + " vertices and " + to_string(m) + " edges" + "\n";
    cout << "Graph created successfully\n";
    return {msg, g};
}

pair<string, Graph *> newEdge(int n, int m, int clientFd, Graph *g)
{
    cout << "Adding an edge from " << n << " to " << m << endl;
    g->addEdge(n - 1, m - 1);        // Add edge from u to v
    g->addEdgeReverse(n - 1, m - 1); // Add reverse edge for the transpose graph
    string msg = "added an edge from " + to_string(n) + " to " + to_string(m) + "\n";
    return {msg, g};
}

pair<string, Graph *> removeedge(int n, int m, int clientFd, Graph *g)
{
    cout << "Removing an edge from " << n << " to " << m << endl;
    g->removeEdge(n - 1, m - 1); // Remove edge from u to v
    string msg = "remove an edge from " + to_string(n) + " to " + to_string(m) + "\n";
    return {msg, g};
}

pair<string, Graph *> kosaraju(Graph *g, int clientFd)
{

    string msg = "requested to print all strongly connected components\n";
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
    while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0'; // Null-terminate the string
        output += buffer;
    }
    close(pipefd[0]); // Close the read-end of the pipe
    // Use 'output' as needed
    msg += "SCCs Output: \n" + output;
    return {msg, nullptr};
}

pair<string, Graph *> handleInput(Graph *g, string action, int clientFd, string actualAction, int n, int m)
{
    string msg;
    if (actualAction== "emptyMessage")
    {
        msg = "sent an empty message!\n";
        return {msg, nullptr};
    }

    if (actualAction == "newgraph")
    { // format: newgraph n m
        return newGraph(n, m, clientFd, g);
    }
    else if (actualAction == "newedge")
    { // format: newedge n m (add an edge from n to m)
        if (g != nullptr)
        {
            return newEdge(n, m, clientFd, g);
        }
        else
        {
            msg = "tried to perform the operation but there is no graph\n";
            return {msg, nullptr};
        }
    }
    else if (actualAction == "removeedge")
    { // format: removeedge n m (remove an edge from n to m)
        if (g != nullptr)
        {
            return removeedge(n, m, clientFd, g);
        }
        else
        {
            msg = "tried to perform the operation but there is no graph\n";
            return {msg, nullptr};
        }
    }
    else if (actualAction == "kosaraju")
    { // format: kosaraju
        if (g == nullptr)
        {
            msg = "tried to perform the operation but there is no graph\n";
            return {msg, nullptr};
        }
        else
        {

            return kosaraju(g, clientFd);
        }
    }
    else
    {
        msg = "sent a message:" + action;
        return {msg, nullptr};
    }
}

// Get sockaddr, IPv4 or IPv6:
void *getInAddr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in *)sa)->sin_addr);
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Return a listening socket
int getListenerSocket(void)
{

    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
            continue;

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL)
        return -1;

    // Listen
    if (listen(listener, 10) == -1)
        return -1;

    return listener;
}

/**
 * function to handle the message received from the client.
 * the availible actions are: newgraph, newedge, removeedge, kosaraju.
 * this function will be called by the reactor for each ready client socket.
 * this function will read the message from the client and operate the action on the graph.
 * parameter: fd - the file descriptor of the client socket.
 */
void handleClientMessage(int clientFd)
{
    string actualAction;
    int n, m;
    char buf[1024]; // Buffer for client data
    int bytesReceived = recv(clientFd, buf, sizeof buf, 0);
    if (bytesReceived <= 0)
    {
        if (bytesReceived == 0)
            printf("server: socket %d hung up\n", clientFd);
        else
            perror("recv");
        close(clientFd);
        reactor.removeFdFromReactor(clientFd);
    }
    else
    {                              // We got data from a client:
        buf[bytesReceived] = '\0'; // Null-terminate the string
        string action = string(buf);
        action = toLowerCase(string(buf));
        vector<string> tokens = splitStringBySpaces(action);
        if (tokens.size() > 0)
        {
            actualAction = tokens[0];
        }
        else
        {
            actualAction = "emptyMessage";
        }
        if (find(graphActions.begin(), graphActions.end(), actualAction) == graphActions.end())
        {
            actualAction = "message";
        }
        else if (actualAction != "kosaraju")
        {
            n = stoi(tokens[1]);
            m = stoi(tokens[2]);
        }

        cout << "User " << clientFd << " requested action: " << actualAction << endl;
        string msg;
        Graph *g2;
        tie(msg, g2) = handleInput(g, action, clientFd, actualAction, n, m);
        if (g2 != nullptr)
            g = g2; // Update the global graph if needed (if the function returned a new graph object)
        
         if(actualAction != "emptyMessage" && actualAction != "message" && actualAction != "kosaraju")
           msg = "Graph updated successfully: " + msg;
        if (send(clientFd, msg.c_str(), msg.size(), 0) == -1)
        {
            perror("send");
            reactor.stopReactor();
        }
    }
}

/**
 * function to handle the incoming connection.
 * will accept the connection and add the new fd to the reactor.
 * parameters: fd - the file descriptor of the listening socket, pointer to the reactor object so that we can add the new fd (client socket) to the reactor.
 */
void handleIncomingConnection(int fd, Reactor *reactor)
{

    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    addrlen = sizeof remoteaddr;

    int newfd = accept(fd, (struct sockaddr *)&remoteaddr, &addrlen); // Accept the incoming connection
    if (newfd == -1)
    {
        perror("accept");
        reactor->stopReactor();
    }
    else
    {
        // print the client IP address:
        char remoteIP[INET6_ADDRSTRLEN];
        inet_ntop(remoteaddr.ss_family,
                  getInAddr((struct sockaddr *)&remoteaddr),
                  remoteIP, INET6_ADDRSTRLEN);
        printf("server: new connection from %s on socket %d\n", remoteIP, newfd);
        // add the new fd to the reactor with the handleClientMessage function as the callback function:
        reactor->addFdToReactor(newfd, handleClientMessage);
        string message = "Welcome to the server. Please enter your action\n";
        if (send(newfd, message.c_str(), message.size(), 0) == -1)
        {
            perror("send");
            reactor->stopReactor();
        }
    }
}

/**
 * this main function is used to test the reactor pattern.
 * the idea is to create a server that listens to incoming connections.
 * when a connection accepted, the server will add the new fd to the reactor as a client socket.
 */
int main()
{
    int listener = getListenerSocket(); // Listening socket descriptor
    cout << "Server is running on port " << PORT << endl;
    if (listener == -1)
    {
        fprintf(stderr, "error getting listener socket\n");
        exit(1);
    }

    // Create a thread with the start function of the reactor:
    thread reactorThread(&Reactor::startReactor, &reactor);

    while (true)
    {
        handleIncomingConnection(listener, &reactor);
    }

    return 0;
}