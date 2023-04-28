/* 
 * File:   libcommon.h
 * Author: pj
 * T
 * Created on September 27, 2014, 8:56 PM
 * 
 * This is an application object, and a polling eventloop object, manages just
 * the file descriptors.  Must be invoked by a main program.
 * Applications may override the Application class
 * Activities (logging, logging config, certain applications will need to be
 * part of the Applications so that all activities in the application may
 * share the same database connections, global constants/variables etc.
 */

#ifndef LIBCOMMON_H
#define	LIBCOMMON_H

#include <iostream>
#include <assert.h>
/*
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>   // this is not included by any files stdio/fcntl/unistd etc

class EventListener {
public:
    int (*callback)(int fd, int userData, struct epoll_event *event);
private:
    int privateDataToken;
};

class EventLoop {
public:
    static const int MAX_FD_IN_LOOP = 64;
    static const int MAX_EVENTS = 32;
    EventLoop();
    ~EventLoop();
    
    int addFd(int fd, int userData, EventListener *listener);
    int deleteFd(int fd);
    void run();
    
private:
    int initLoopFd();
    int deInitLoopFd();
    void initLocalData();
    void deInitLocalData();
    int addToEventLoop(int fd, int i);
    int deleteFromEventLoop(int fd);
    int deleteFromEventLoop2(int fd, int slot);
    void deleteAllFds();
    void freeSlot(int slot);
    void useSlot(int slot, int fd, int userData, EventListener *listener);
    
    int epfd;
    int fds[MAX_FD_IN_LOOP];
    int userData[MAX_FD_IN_LOOP];  // nanomsg socket
    EventListener *listeners[MAX_FD_IN_LOOP];
};


class Application {
    EventLoop *eventLoop;
    char progName[32];
    char appName[64];
    int appInstance;
    
public:
    Application();
    Application(const int argc, const char *argv[]);
    ~Application();
    void run();
    int initServerActivities();
    void initialize(const int argc, const char *argv[]);
    EventLoop *getEventLoop() {
        return eventLoop;
    }
};

#endif	/* LIBCOMMON_H */

