/* 
 * File:   logtx.c
 * Author: prakash
 *
 * Created on October 15, 2010, 11:58 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "af_unix_socket_wrapper.h"

/*
 * 
 */
int main(int argc, char** argv) {
    int s;
    int n_msgs;
    s = log_open(LOG_MODE_CLIENT, 10, LOG_SOCKET_DGRAM);
    if (s <= 0) {
        perror ("Unable to open server socket\n");
        exit (-1);
    }
    const int MAX_LEN_LOG_MSG=2000;
    int i = 0;
    char buf[MAX_LEN_LOG_MSG];

    if (argc > 1) {
        n_msgs = atoi(argv[1]);
    } else {
        n_msgs = 10;
    }

    if (n_msgs <= 0 || n_msgs > 1000) {
        n_msgs = 10;
    }
    
    memset(buf, 0, MAX_LEN_LOG_MSG);
    while (i++ < 10) {
        logtx(s, buf, MAX_LEN_LOG_MSG);
        printf("Msg %d transmitted.\n", i);
        sleep (1);
    }
    log_close (s);
    return (EXIT_SUCCESS);

}

