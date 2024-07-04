#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <mutex>
#include <thread>
#include <vector>

#include "deque_AL.hpp"

#define BOLD "\033[1m"
#define RESETCOLOR "\033[0m"
#define PORT 9035
#define MAXBYTES 1024

std::mutex graphMutex;

Graph *g = nullptr; // Global graph object
const vector<string> graphActions = {"newgraph", "newedge", "removeedge", "kosaraju"};

using namespace std;

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
 * param clientSocket: the socket of the client (allready connected)
 * this function handles the client connecction and the communication with it.
 * for incomming requests it calls the handleInput function and sends the response back to the client.
 */
void handleClient(int clientSocket)
{
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
            return;
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
            return;
        }

    } // end of while loop

    close(clientSocket);
}

/**
 * this function is used to start the server and listen to the clients.
 * it creates a new thread for each client that connects to the server.
 */
void startServer()
{

    int serverFd, newSocket; // File descriptors for server and client sockets
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port PORT
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(serverFd, 10) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    cout << "Server listening on port " << PORT << endl;
    while (true)
    { // Keep the server running and accepting connections
        if ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Create a thread for each client
        std::thread clientThread(handleClient, newSocket);
        clientThread.detach(); // Detach the thread to run independently
        cout << "New client connected with socket: " << newSocket << endl;
    }
}

/**
 * standart main function that create threads for the server
 */
int main()
{
    std::thread serverThread(startServer);
    serverThread.join(); // Keep the main thread alive untill the server thread is finished
    return 0;
}