/* 
 * File:   main.cpp
 * Author: pj
 *
 * Created on October 4, 2014, 1:33 PM
 */

#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>


using namespace std;


#define PORT_NO 3033
#define BUFFER_SIZE 1024

int main() {
    int sd;
    struct sockaddr_in addr;
    int addr_len = sizeof (addr);
    char buffer[BUFFER_SIZE] = "";

    // Create client socket
    if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return -1;
    }


    bzero(&addr, sizeof (addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NO);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Connect to server socket
    if (connect(sd, (struct sockaddr *) &addr, sizeof addr) < 0) {
        perror("Connect error");
        return -1;
    }

    while (strcmp(buffer, "q") != 0) {
        // Read input from user and send message to the server
        gets(buffer);
        send(sd, buffer, strlen(buffer), 0);

        // Receive message from the server
        recv(sd, buffer, BUFFER_SIZE, 0);
        printf("message: %s\n", buffer);
    }

    return 0;
}

