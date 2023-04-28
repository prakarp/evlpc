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
#include <ini.h>

using namespace std;
    
typedef struct
{
    char app_name[32];
    char default_host[32];
    char app_id[8];
    char default_port[8];
} configuration;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;
    const char *app_name = pconfig->app_name;
    
    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("default", "host")) {
        strncpy(pconfig->default_host, value, sizeof(pconfig->default_host)-1);
    } else if (MATCH("app_id", app_name)) {
        strncpy(pconfig->app_id, value, sizeof(pconfig->app_id)-1);
    } else if (MATCH("default_port", app_name)) {
        strncpy(pconfig->default_port, value, sizeof(pconfig->default_port)-1);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

configuration gconfig;

int read_ini_config(const char *inifn, const char *app_name) {
    strncpy(gconfig.app_name, app_name, sizeof(gconfig.app_name)-1);
    if (ini_parse(inifn, handler, &gconfig) < 0) {
        printf("Can't load %s\n", inifn);
        return 1;
    }
    printf("Config loaded from %s: default_host=%s, app_id=%s, default_port=%s\n",
            inifn, gconfig.default_host, gconfig.app_id, gconfig.default_port);
    return 0;
}


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
       
    read_ini_config("logging_tokens.ini", "ca_log");
    
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

