
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <vector>
#include <bits/socket.h>
#include <netdb.h>
#include <string.h>

#define PORT "9034"
#define MAXDATASIZE 1024
#define TIMEOUT 100 // 100 ms

using namespace std;

/**
 * Function to get a client socket
 * @param host Hostname
 * @param port Port number
 * @return Connected socket file descriptor
*/
int getClientSocket(const string &host, const std::string &port) {
    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    // Get the address info
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;  // TCP
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0) {
        perror("getaddrinfo");
        return -1;
    }

    // Connect to the server
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        return -1;
    }

    freeaddrinfo(res);
    return sockfd;
}

/**
 * this "client.cpp" file is used to implement the client side of the reactor pattern.
 * it is simply create a client socket and connect to the server on: "localhost:9034".
 * then it get an input from the user's keyboard and send it to the server in a loop untill the user type "exit".
*/
int main(){
    int sockfd = getClientSocket("localhost", PORT);
    if (sockfd == -1) {
        return 1;
    }

    // Get input from the user and send it to the server:
    string input;
    while (true) {
        cout << "Enter a message to send to the server: ";
        getline(cin, input);
        if (input == "exit") {
            break;
        }

        // Send the message to the server
        if (send(sockfd, input.c_str(), input.size(), 0) == -1) {
            perror("send");
            break;
        }

        // wait for the server to respond a TIMEOUT time (using setsockopt):
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = TIMEOUT * 1000;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        size_t bytesReceived = recv(sockfd, input.data(), input.size(), 0);
        if (bytesReceived == -1) {
            perror("recv");
            break;
        }
        if(bytesReceived > 0){
            cout << "Server response: " << input << endl;
        }
    }

    // closing the socket
    close(sockfd);

    return 0;
}