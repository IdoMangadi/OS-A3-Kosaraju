
#include <algorithm>
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
    void stopReactor() ;

    /**
     * Constructor for the Reactor class
     */
    Reactor() ;

    /**
     * Destructor for the Reactor class
     */
    ~Reactor() ;
};



