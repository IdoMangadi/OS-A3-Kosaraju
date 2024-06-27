#ifndef PATTERNS_HPP
#define PATTERNS_HPP

#include <unordered_map>
#include <vector>
#include <thread>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <functional>
#include <pthread.h>

// Function type for reactor callbacks
typedef void (*ReactorFunc)(int fd);

// Function type for proactor callbacks
typedef void *(*ProactorFunc)(int sockfd);

/**
 * @class CompletionHandler
 * @brief Interface for completion handlers in the Proactor pattern
 */
class CompletionHandler {
public:
    virtual void handleEvent(int sockfd) = 0;
    virtual ~CompletionHandler() = default;
};

/**
 * @class ConcreteCompletionHandler
 * @brief Concrete implementation of the CompletionHandler interface
 */
class ConcreteCompletionHandler : public CompletionHandler {
public:
    void handleEvent(int sockfd) override;
};

/**
 * @class Reactor
 * @brief A class to represent a reactor pattern implementation
 */
class Reactor {
    std::unordered_map<int, ReactorFunc> fdMap;  // Map of (file descriptor : callback functions)
    std::vector<struct pollfd> fds;  // Vector of pollfd structures for poll
    bool running;  // Flag to control the reactor's main loop

    /**
     * Main loop for the reactor, polls file descriptors and calls associated functions
     */
    void run();

    public:
        /**
         * Constructor to initialize a reactor
         */
        Reactor();

        /**
         * Destructor to stop the reactor if it's running
         */
        ~Reactor();

        /**
         * Start the reactor's main loop in a separate thread
         */
        void startReactor();

        /**
         * Add a file descriptor and associated function to the reactor
         * @param fd File descriptor
         * @param func Function to call when the file descriptor is ready
         */
        void addFdToReactor(int fd, ReactorFunc func);

        /**
         * Remove a file descriptor from the reactor
         * @param fd File descriptor
         */
        void removeFdFromReactor(int fd);

        /**
         * Stop the reactor's main loop
         */
        void stopReactor();
};

/**
 * @class Proactor
 * @brief A class to represent a proactor pattern implementation
 */
class Proactor {
    pthread_t threadId;  // Thread ID for the proactor
    int sockfd;  // Socket file descriptor
    ProactorFunc threadFunc;  // Function to call when a client connects
    bool running;  // Flag to control the proactor's main loop

    /**
     * Main loop for the proactor, accepts connections and starts new threads
     */
    static void *run(void *arg);

    /**
     * Wrapper function to call the ProactorFunc with the correct signature
     */
    static void *threadFuncWrapper(void *arg);

    public:
        /**
         * Start the proactor's main loop in a separate thread
         * @param sockfd Socket file descriptor to listen for connections
         * @param threadFunc Function to call when a client connects
         * @return Thread ID for the proactor
         */
        pthread_t startProactor(int sockfd, ProactorFunc threadFunc);

        /**
         * Stop the proactor's main loop
         * @param tid Thread ID of the proactor
         * @return 0 on success
         */
        int stopProactor(pthread_t tid);
};

#endif // PATTERNS_HPP
