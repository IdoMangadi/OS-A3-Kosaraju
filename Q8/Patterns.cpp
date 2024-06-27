#include "Patterns.hpp"
#include <algorithm>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

// Reactor class implementation
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
}

void Reactor::stopReactor() {
    running = false;
}

// CompletionHandler implementation
void ConcreteCompletionHandler::handleEvent(int sockfd) {
    char buf[1024];  // Buffer for client data
    int bytesReceived = recv(sockfd, buf, sizeof(buf), 0);
    if (bytesReceived <= 0) {
        if (bytesReceived == 0) printf("server: socket %d hung up\n", sockfd);
        else perror("recv");
        close(sockfd);
    } else {
        buf[bytesReceived] = '\0';  // Null-terminate the string
        std::string action = std::string(buf);
        std::cout << "Received action: " << action << std::endl;
        // Handle the action (e.g., newgraph, newedge, removeedge, kosaraju)
        // ...
    }
}

// Proactor class implementation
void *Proactor::run(void *arg) {
    Proactor *proactor = static_cast<Proactor *>(arg);
    proactor->running = true;
    
    while (proactor->running) {
        struct sockaddr_storage remoteaddr; // Client address
        socklen_t addrlen = sizeof remoteaddr;
        int newfd = accept(proactor->sockfd, (struct sockaddr *)&remoteaddr, &addrlen);
        if (newfd == -1) {
            perror("accept");
            continue;
        }
        // Start a new thread to handle the client connection
        pthread_t clientThread;
        pthread_create(&clientThread, nullptr, Proactor::threadFuncWrapper, (void *)(intptr_t)newfd);
        pthread_detach(clientThread);  // Detach the thread to clean up automatically
    }
    return nullptr;
}

void *Proactor::threadFuncWrapper(void *arg) {
    int sockfd = (int)(intptr_t)arg;
    Proactor *proactor = static_cast<Proactor *>(arg);
    return proactor->threadFunc(sockfd); // Call the original ProactorFunc
}

pthread_t Proactor::startProactor(int sockfd, ProactorFunc threadFunc) {
    this->sockfd = sockfd;
    this->threadFunc = threadFunc;
    pthread_create(&threadId, nullptr, Proactor::run, this);
    return threadId;
}

int Proactor::stopProactor(pthread_t tid) {
    running = false;
    return pthread_cancel(tid);
}
