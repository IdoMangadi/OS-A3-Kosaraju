
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