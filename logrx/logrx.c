/* 
 * File:   main.c
 * Author: prakash
 *
 * Created on October 15, 2010, 11:55 AM
 */

#include <stdio.h>
#include <stdlib.h>

#include "af_unix_socket_wrapper.h"
/*
 * 
 */
int main(int argc, char** argv) {
    int s;

    unlink (LOG_SOCK_NAME);
    s = log_open(LOG_MODE_SERVER, 10, LOG_SOCKET_DGRAM);
    if (s <= 0) {
        perror ("Unable to open server socket\n");
        exit (-1);
    }

    const int MAX_LEN_LOG_MSG=2000;
    int i = 0;
    char buf[MAX_LEN_LOG_MSG];
    while (i++ < 10000) {
        logrx(s, buf, MAX_LEN_LOG_MSG);
        printf("Msg %d received\n", i);
    }
    log_close (s);
    return (EXIT_SUCCESS);
}
