#ifndef PATTERNS_HPP
#define PATTERNS_HPP

#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utility>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <poll.h>
#include <unistd.h>
#include <mutex>
#include <memory>

// Function type for reactor callbacks
typedef void (*ReactorFunc)(int fd);

/**
 * @class Reactor
 * @brief A class to represent a reactor pattern implementation
 */
class Reactor {
private:
    std::unordered_map<int, ReactorFunc> fdMap;  // Map of (file descriptor : callback functions)
    std::vector<struct pollfd> fds;  // Vector of pollfd structures for poll
    bool running;  // Flag to control the reactor's main loop
    std::thread reactorThread;  // Thread to run the reactor

    /**
     * Main loop for the reactor, polls file descriptors and calls associated functions
     */
    void run(); 

public:

    /**
     * Constructor for the Reactor class
     */
    Reactor() ;
    /**
     * Start the reactor's main loop in a separate thread
     */
    void startReactor() ;

    /**
     * Add a file descriptor and associated function to the reactor
     * @param fd File descriptor
     * @param func Function to call when the file descriptor is ready
     */
    void addFdToReactor(int fd, ReactorFunc func) ;
    /**
     * Remove a file descriptor from the reactor
     * @param fd File descriptor
     */
    void removeFdFromReactor(int fd);
    /**
     * Stop the reactor's main loop
     */
    void stopReactor();
    /**
     * Destructor for the Reactor class
     */
    ~Reactor() ;
};


// Function type for proactor callbacks
typedef void* (*ProactorFunc)(int sockfd);

/**
 * @class Proactor
 * @brief A class to represent a proactor pattern implementation.
 * in this implementation we create an proactor object that will run in a separate thread.
 * it will accept incomming connections and create a new thread to handle each client.
 * the function that the client thread will run is given by the user (it is the handleClientFd)
 */
class Proactor{

    private:
        std::thread proactorThread;  // Thread  for the proactor
        int sockfd;  // Socket file descriptor, this will be the listener socket.
        bool running;  // Flag to control the proactor's main loop
        /**
         * Main loop for the proactor, accepts connections and starts new threads
         */
        void run();


    public:
        ProactorFunc threadFunc;  // Function to call when a client connects
        /**
         * Constructor to initialize a proactor
         */
        Proactor();
        /**
         * Start the proactor's main loop in a separate thread
         * @param sockfd Socket file descriptor to listen for connections
         * @param threadFunc Function to call when a client connects
         * @return Thread ID for the proactor
         */
        void startProactor(int sockfd, ProactorFunc threadFunc);
        /**
         * Stop the proactor's main loop
         * @param tid Thread ID of the proactor
         * @return 0 on success
         */
        void stopProactor();
        
        ~Proactor(); // Destructor to stop the proactor if it's running
        
        static Proactor& getInstance();  // Get the singleton instance of the Proactor
        
};

#endif // PATTERNS_HPP
