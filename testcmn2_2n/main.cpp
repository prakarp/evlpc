/* 
 * File:   main.cpp
 * Author: pj
 *
 * Created on October 5, 2014, 7:29 PM
 */

#include <cstdlib>

#include <appcommon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

int myRxListener(uv_stream_t *connection, ssize_t siz, uv_buf_t buf) {
    printf("Received %ld bytes, data: %s\n", siz, buf.base);
    return 0;
}

int myTimer1Listener(uv_timer_t* handle, int status) {
    printf("Timer1 Occurred: Status: %d\n", status);
}

/*
 * 
 */
int main(int argc, char** argv) {
    uint16_t defaultPort = 7000;
    char progname[32];
    char app_id[32];
    char instance_id[8];
    
    printf ("ARGC %d\n", argc);

    strncpy(progname, argv[0], sizeof(progname)-1);
    if (argc > 1) {
        strncpy(app_id, argv[1], sizeof(app_id)-1);
    } else {
        strncpy(app_id, "app", sizeof(app_id)-1);
    }
    if (argc > 2) {
        strncpy(instance_id, argv[2], sizeof(instance_id)-1);
    } else {
        strncpy(instance_id, "1", sizeof(instance_id)-1);
    }
    
    if (argc > 3) {
        defaultPort = atoi(argv[3]);
        if (defaultPort < 1024 || defaultPort > 65535) {
            printf("Invalid Port number supplied: %d (allowed: 1024-65535)\n", defaultPort);
            exit (-2);
        }
    }
         
    printf("Starting application %s %s %s %u\n", progname, app_id, instance_id, defaultPort);
    Application *app = new Application(progname, app_id, instance_id);
    EventLoopDefaultListener myListener;
    myListener.listenerMethod = myRxListener;
    app->initServerActivities(defaultPort);
    app->getEventLoop()->defaultListener = myRxListener;
    app->getEventLoop()->timer1Listener = myTimer1Listener;
    app->run();
    return 0;
}

