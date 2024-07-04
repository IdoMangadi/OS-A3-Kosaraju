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
#include <mutex>

#include "deque_AL.hpp"
#include "patterns.hpp"
#include <condition_variable>

#define BOLD "\033[1m"
#define RESETCOLOR "\033[0m"

#define PORT "9034" // Port we're listening on
#define MAXBYTES 1024

using namespace std;

// Global variables
std::mutex graphMutex;
Graph *g = nullptr; // Global graph object
const vector<string> graphActions = {"newgraph", "newedge", "removeedge", "kosaraju"};
Proactor proactor;

// Global variables for majority connected components:
mutex condMutex;
condition_variable cond; // condition variable
bool sccBool = false; // flag to indicate if the majority of the graph is connected by the SCC latest call
bool graphState = false;  // boolean to determine if the graph is above 50% connectivity

// Worker thread function:
void printMessages() {
    unique_lock<std::mutex> lock(condMutex);  // Lock the mutex
    while(true) {
        cond.wait(lock, []{return ((sccBool&&!graphState)||(!sccBool&&graphState));});  // Wait until sccBool is true (using lambda function)
        if (sccBool && graphState == false) {  // Print the message if the graph is connected
            cout << "At least 50% of the graph belongs to the same SCC" << endl;
            graphState = true;
        }
        else if(sccBool == false && graphState == true) {  // Print the message if the graph is not connected
            cout << "Less than 50% of the graph belongs to the same SCC" << endl;
            graphState = false;
        }
    }
}

/**
 * Function to convert a string to lower case
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

// Get sockaddr, IPv4 or IPv6:
void *getInAddr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in *)sa)->sin_addr);
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void initGraph(Graph *g, int m, int clientFd)
{
    string msg = "To create an edge u->v please enter the edge number in the format: u v\n";
    if (send(clientFd, msg.c_str(), msg.size(), 0) < 0)
    {
        perror("send");
    }
    int stdin_save = dup(STDIN_FILENO); // Save the current state of STDIN
    dup2(clientFd, STDIN_FILENO);       // Redirect STDIN to the socket
    for (int i = 0; i < m; i++)
    { // Read the edges
        int u, v;
        cin >> u >> v;
        g->addEdge(u - 1, v - 1);        // Add edge from u to v
        g->addEdgeReverse(u - 1, v - 1); // Also add reverse edge for the transpose graph
    }
    dup2(stdin_save, STDIN_FILENO); // Restore the original STDIN
}

pair<string, Graph *> newGraph(int n, int m, int clientFd, Graph *g)
{
      if(graphMutex.try_lock() == false){
        string msg = "Graph is currently being used by another client, please try again later\n";
        return {msg, nullptr};
    }
    
    cout<<"Performing action on graph - Mutex locked\n";
    cout << "Creating a new graph with " << n << " vertices and " << m << " edges" << endl;

    if (g != nullptr)
        delete g;
    g = new Graph(n);          // Create a new graph of n vertices
    initGraph(g, m, clientFd); // Initialize the graph with m edges
    sccBool = false;           // Reset the sccBool flag
    graphState = false;        // Reset the graphState flag
    string msg = "successfully created a new Graph with " + to_string(n) + " vertices and " + to_string(m) + " edges" + "\n";
    cout << "Graph created successfully - unlocking Mutex\n";
    graphMutex.unlock();
    return {msg, g};
}

pair<string, Graph *> newEdge(int n, int m, int clientFd, Graph *g)
{
     if(graphMutex.try_lock() == false){
        string msg = "Graph is currently being used by another client, please try again later\n";
        return {msg, nullptr};
    }
    cout<<"Performing action on graph - Mutex locked\n";
    cout << "Adding an edge from " << n << " to " << m << endl;
    g->addEdge(n - 1, m - 1);        // Add edge from u to v
    g->addEdgeReverse(n - 1, m - 1); // Add reverse edge for the transpose graph
    string msg = "added an edge from " + to_string(n) + " to " + to_string(m) + "\n";
    cout << "Edge added successfully - unlocking Mutex\n";
    graphMutex.unlock();
    return {msg, g};
}

pair<string, Graph *> removeedge(int n, int m, int clientFd, Graph *g)
{
      if(graphMutex.try_lock() == false){
        string msg = "Graph is currently being used by another client, please try again later\n";
        return {msg, nullptr};
    }
    cout<<"Performing action on graph - Mutex locked\n";
    cout << "Removing an edge from " << n << " to " << m << endl;
    g->removeEdge(n - 1, m - 1); // Remove edge from u to v
    string msg = "removed an edge from " + to_string(n) + " to " + to_string(m) + "\n";
    cout << "Edge removed successfully - unlocking Mutex\n";
    graphMutex.unlock();
    return {msg, g};
}

pair<string, Graph *> kosaraju(Graph *g, int clientFd)
{
    if(graphMutex.try_lock() == false){
        string msg = "Graph is currently being used by another client, please try again later\n";
        return {msg, nullptr};
    }
    cout<<"Performing action on graph - Mutex locked\n";
    // graphMutex.lock();
    string msg = "requested to print all strongly connected components\n";
    int stdout_save = dup(STDOUT_FILENO); // Save the current state of STDOUT
    int pipefd[2];
    pipe(pipefd);                   // Create a pipe
    dup2(pipefd[1], STDOUT_FILENO); // Redirect STDOUT to the pipe
    close(pipefd[1]);               // Close the write-end of the pipe as it's now duplicated

    // if(g->printSCCs()) sccBool = true; // Set the sccBool flag to true if the majority of the graph is connected
    sccBool = g->printSCCs(); // Print the strongly connected components
    // Restore the original STDOUT
    dup2(stdout_save, STDOUT_FILENO);
    close(stdout_save); // Close the saved STDOUT
    cond.notify_one(); // Notify the worker thread to print the message

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
    cout << "Strongly connected components printed successfully - unlocking Mutex\n";

    graphMutex.unlock(); // Unlock the mutex
    
    return {msg, nullptr};
}

 /**
  * param g: the graph object
  * param action: the action to be performed
  * param clientFd: the client socket
  * this function handles the input from the client and performs the action on the graph.
  * it returns a pair of strings, the first string is the message to be printed to the server console
  * and the second string is the message to be sent back to the client.
  */
pair<string, Graph *> handleInput(Graph *g, string action, int clientFd, string actualAction, int n, int m)
{
  
    string msg;
    if (actualAction == "emptyMessage")
    {
        msg = "sent an empty message\n";
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

/**
 * create a listener socket and bind it to the port.
 */
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
void *handleClient(int clientSocket) {
    string actualAction;
    char buffer[MAXBYTES];
    memset(buffer, 0, MAXBYTES);
    int n, m;
    // sending welcome message to the client:
    string welcomeMsg = "Welcome to the server, please enter your action in the format: action n m\n";
    send(clientSocket, welcomeMsg.c_str(), welcomeMsg.size(), 0);

    while (true)
    {
        // receive client request
        int bytesReceived = recv(clientSocket, buffer, MAXBYTES, 0);
        if (bytesReceived <= 0)
        {
            if (bytesReceived == 0)
                printf("server: socket %d hung up\n", clientSocket);
            else
                perror("recv");
            close(clientSocket);
            return nullptr;
        }

        // Process the client request
        buffer[bytesReceived] = '\0'; // Null-terminate the string
        string action = string(buffer);
        action = toLowerCase(string(buffer));
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

        cout << "Action: " << actualAction << endl;
        string msg;
        Graph *g2;

        tie(msg, g2) = handleInput(g, action, clientSocket, actualAction, n, m);
        if (g2 != nullptr)
            g = g2; // Update the global graph if needed (if the function returned a new graph object)
        if (actualAction != "emptyMessage" && actualAction != "message" && actualAction != "kosaraju" && g != nullptr)
            msg = "Graph updated successfully: " + msg;
        // Send response to client
        if (send(clientSocket, msg.c_str(), msg.size(), 0) == -1)
        {
            perror("send");
            return nullptr;
        }

    } // end of while loop

    close(clientSocket);

    return nullptr;
}

/**
 * Main function to test the proactor pattern.
 * The idea is to create a server that listens to incoming connections.
 * When a connection is accepted, the server will add the new fd to the proactor as a client socket.
 */
int main() {

    // creating the worker thread:
    thread workerThread(printMessages); // Start the worker thread
    workerThread.detach(); // Detach the worker thread to run independently

    // creating the listener socket
    int listener = getListenerSocket();
    cout << "Server is running on port " << PORT << endl;
    if (listener == -1) {
        fprintf(stderr, "error getting listener socket\n");
        exit(1);
    }

    // Start the proactor with the listener socket and handleIncomingConnection function as the callback
    proactor.startProactor(listener, handleClient);
    close(listener);
    workerThread.~thread();
    return 0;
}
