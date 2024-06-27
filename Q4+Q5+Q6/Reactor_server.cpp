
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

#define PORT "9034" // Port we're listening on

using namespace std;

// creating a global variable to store the graph:
Graph *g = nullptr;

/**
 * function to convert a string to lower case
*/
string toLowerCase(string s){
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

void initGraph(Graph *g, int m, int sender_fd){
    int stdin_save = dup(STDIN_FILENO); // Save the current state of STDIN
    dup2(sender_fd, STDIN_FILENO);      // Redirect STDIN to the socket
    for (int i = 0; i < m; i++){ 
        int u, v;
        cin >> u >> v;  // Read the edges
        g->addEdge(u - 1, v - 1);        // Add edge from u to v
        g->addEdgeReverse(u - 1, v - 1); // Also add reverse edge for the transpose graph
    }
    dup2(stdin_save, STDIN_FILENO); // Restore the original STDIN
}
std::vector<std::string> splitStringBySpaces(const std::string &input)
{
    std::istringstream stream(input);
    std::vector<std::string> result;
    std::string temp;

    while (stream >> temp) result.push_back(temp);

    return result;
}

pair<string, Graph *> handleInput(Graph *g, string action, int sender_fd){

    string msg;
    vector<string> tokens = splitStringBySpaces(action);  // Split the input string by spaces
    string realAction = tokens[0];  // Get the first token as the action
    int n, m;

    if (realAction == "newgraph"){
        n = stoi(tokens[1]);
        m = stoi(tokens[2]);

        if (g != nullptr) delete g;
        
        g = new Graph(n); // Create a new graph of n vertices
        initGraph(g, m, sender_fd);
        msg = "User" + to_string(sender_fd) + " successfully created a new Graph with " + to_string(n) + " vertices and " + to_string(m) + " edges\n";
        return {msg, g};
    }
    else if (realAction == "newedge"){
        if (g != nullptr){
            n = stoi(tokens[1]);
            m = stoi(tokens[2]);
            cout << &(*g);
            g->addEdge(n - 1, m - 1);        // Add edge from u to v
            g->addEdgeReverse(n - 1, m - 1); // Add reverse edge for the transpose graph
        }
        msg = "User" + to_string(sender_fd) + " added an edge from " + to_string(n) + " to " + to_string(m) + "\n";
        return {msg, nullptr};
    }
    else if (realAction == "removeedge"){
        if (g != nullptr){
            n = stoi(tokens[1]);
            m = stoi(tokens[2]);
            g->removeEdge(n - 1, m - 1); // Remove edge from u to v
        }
        msg = "User" + to_string(sender_fd) + " removed an edge from " + to_string(n) + " to " + to_string(m) + "\n";
        return {msg, nullptr};
    }
    else if (realAction == "kosaraju"){
        if (g == nullptr){
            msg = "User " + to_string(sender_fd) + " tried to perform the operation but there is no graph\n";
            return {msg, nullptr};
        }
        else{
            msg = "User" + to_string(sender_fd) + " requested to print all strongly connected components\n";
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
            msg += "SCCs Output: \n" + output + "\n";
            return {msg, nullptr};
        }
    }
    else{
        msg = "User " + to_string(sender_fd) + " sent a message:\n" + action + "\n";
        return {msg, nullptr};
    }
}
// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
// Return a listening socket
int get_listener_socket(void)
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
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0){
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next){
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) continue;

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0){
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
 * function to handle the message received from the client.
 * the availible actions are: newgraph, newedge, removeedge, kosaraju.
 * this function will be called by the reactor for each ready client socket.
 * this function will read the message from the client and operate the action on the graph.
 * parameter: fd - the file descriptor of the client socket.
*/
void handleClientMessage(int fd){

    char buf[1024];  // Buffer for client data
    int nbytes = recv(fd, buf, sizeof buf, 0);
    if (nbytes <= 0){
        if (nbytes == 0) printf("selectserver: socket %d hung up\n", fd);
        else perror("recv");
        close(fd); // Bye!
    }
    else{  // We got data from a client:
        buf[nbytes] = '\0';  // Null-terminate the string
        printf("Received: %s\n", buf);
        string action = string(buf);
        string msg;
        Graph* g2;
        tie(msg, g2) = handleInput(g, action, fd);
        if (g2 != nullptr) g = g2;  // Update the global graph if needed (if the function returned a new graph object)
        // TODO: add a response to the client
        cout << msg << endl;
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
    
    int newfd = accept(fd, (struct sockaddr *)&remoteaddr, &addrlen);  // Accept the incoming connection
    if (newfd == -1){
        perror("accept");
    }
    else{
        // print the client IP address:
        char remoteIP[INET6_ADDRSTRLEN];
        inet_ntop(remoteaddr.ss_family,
                  get_in_addr((struct sockaddr *)&remoteaddr),
                  remoteIP, INET6_ADDRSTRLEN);
        printf("selectserver: new connection from %s on "
               "socket %d\n",
               remoteIP, newfd);
        // add the new fd to the reactor with the handleClientMessage function as the callback function:
        reactor->addFdToReactor(newfd, handleClientMessage);
    }
}

/**
 * this main function is used to test the reactor pattern.
 * the idea is to create a server that listens to incoming connections.
 * when a connection accepted, the server will add the new fd to the reactor as a client socket.
*/
int main(){
    int listener = get_listener_socket();  // Listening socket descriptor
    if (listener == -1){
        fprintf(stderr, "error getting listener socket\n");
        exit(1);
    }

    // Create a reactor object
    Reactor reactor;
    reactor.startReactor();  // the reactor will start in a separate thread
    
    while(true){
        handleIncomingConnection(listener, &reactor);
    }

    // TODO: improve this part...

    return 0;
}