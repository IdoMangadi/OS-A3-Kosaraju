
#include "patterns.hpp"

using namespace std;

Reactor::Reactor() : running(false) {}
Reactor::~Reactor() {
    if (running) {  // Stop the reactor if it's running
        stopReactor();
    }    
}

void Reactor::run() {  // private function
    while (running) {  // Main loop for the reactor
        // setting timeout to 100 ms:
        int ret = poll(fds.data(), fds.size(), 100);  // Poll file descriptors

        if (ret > 0) {  // Call associated functions for file descriptors with events
            for (auto& pfd : fds) {  // Loop through all file descriptors
                if (pfd.revents != 0) {  // If the file descriptor has events
                    fdMap[pfd.fd](pfd.fd);  // Call the associated function with argument as the file descriptor of the client socket
                }
            }
        } else if (ret == -1) {
            perror("poll");
            break;
        }
    }
}

void Reactor::startReactor(){
    running = true;
    this->run();  // Start the reactor's main loop in a separate thread
}

void Reactor::addFdToReactor(int fd, ReactorFunc func) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    fds.push_back(pfd);
    fdMap[fd] = func;  // map file descriptor to callback function
}

void Reactor::removeFdFromReactor(int fd) {
    fds.erase(std::remove_if(fds.begin(), fds.end(), [fd](const struct pollfd& pfd) { return pfd.fd == fd; }), fds.end());  // Remove file descriptor from the reactor
    fdMap.erase(fd);  // Remove file descriptor from the map
    for(auto& pfd : fds) {
        if (pfd.fd == fd) {
            while(true){           std::cout << "Error: file descriptor not removed from reactor" << std::endl;
            }
        }
    }
}

void Reactor::stopReactor() {
    running = false;
}



Proactor::Proactor() : running(false) {}
Proactor::~Proactor() {
    if (running) {  // Stop the proactor if it's running
        stopProactor();
    }
}


/**
 * this function is running in a separate thread by the listener thread.
 * in a infinite loop, it accepts a new client connection and create a new thread to handle the client.
 * the arguments this function get is a pointer to the proactor instance and the sockfd of the listener.
 */
void Proactor::run() {
    // Main loop for the proactor
    while (running) {
        // accept a new client connection
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrLen);  // Accept a new client connection
        if (clientSockfd == -1) {
            perror("accept");
            continue;
        }
        cout << "Server: new connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;
        // Create a new thread to handle the client
        thread([this, clientSockfd]() {  // Lambda function to handle the client
           threadFunc(clientSockfd);
        }).detach(); // Detach the thread to handle the client independently
    }
    
}

/**
 * Start the proactor's main loop in a separate thread
 * param[in] sockfd The listener socket file descriptor, this will be the listener socket.
 * param[in] threadFunc Function to call when a client connects (this function should handle the client).
 */
void Proactor::startProactor(int sockfd, ProactorFunc clientFunc) { 

    this->sockfd = sockfd;  // Set the listener socket file descriptor
    this->threadFunc = clientFunc;  // Set the function to call when a client connects
    this->running = true;  // Set the running flag to true

    // creating a new thread to run the proactor's main loop with the function run
    proactorThread = thread([this, sockfd, clientFunc](){ // Detach the thread to run the proactor's main loop (using lambda function)
        run();// Run the proactor's main loop
    });
    if(proactorThread.joinable()){
        proactorThread.join();  // Wait for the thread to finish
    }
}

void Proactor::stopProactor() {
    running = false;  // Set the running flag to false
    if (proactorThread.joinable()) {
        proactorThread.join(); // Wait for the thread to finish
    }
}
