#include "libcommon.h"
#include "utils1.h"

EventLoop::EventLoop() {
    initLoopFd();
    initLocalData();
}

EventLoop::~EventLoop() {
    deInitLocalData();
    deInitLoopFd();
}

void EventLoop::run() {
    
    struct epoll_event *events;
    events = (struct epoll_event *)
            malloc(sizeof (struct epoll_event) * MAX_EVENTS);

    assert(events != NULL);
    int polling_timeout = -1;
    while (1) {
        // fully blocking.
        // TODO - there is no signal or timeout configured here.
        int nr_events = epoll_wait(epfd, events, MAX_EVENTS, polling_timeout);
        
        if (nr_events < 0) {
            if (EINTR == errno) {
                continue;
            }
            
            
            perror("epoll_wait error exiting");
            free(events);
            exit(-1);
        }
       
        for (int i = 0; i < nr_events; i++) {
            struct epoll_event *event;
            event = &events[i];
            
            printf("event=%u on fd slot=%d \n",
                    event->events,
                    event->data.fd);

            /*
             * We now can, per events[i].events, operate on
             * events[i].data.fd without blocking.
             */
            if ((event->events & EPOLLERR) ||
              (event->events & EPOLLHUP) ||
              (!(event->events & EPOLLIN)))
	    {
              /* An error has occured on this fd, or the socket is not
                 ready for reading (why were we notified then?) */
	      perror ("epoll error - exiting ");
	      exit(-1);
	    }
            
            // obtained the user-token passed to epoll()
            int slot = event->data.fd;
            EventListener *listener = listeners[slot];
            // TODO filter out the events
            if (NULL != listener->callback) {
                listener->callback(fds[slot], userData[slot], event); // pass the event to the listener
            }
            // continue after calling the event listeners 
        }
    }
}

void EventLoop::freeSlot(int slot) {
    fds[slot] = 0;
    userData[slot] = 0;
    listeners[slot] = NULL;
    return;
}

void EventLoop::useSlot(int slot, int fd, int sock, EventListener *listener) {
    fds[slot] = fd;
    userData[slot] = sock;
    listeners[slot] = listener;
}

int EventLoop::addFd(int fd, int sock, EventListener *listener) {
    int slot;
    for (slot = 0; slot < MAX_FD_IN_LOOP; slot++) {
        if (0 == fds[slot]) { 
            useSlot(slot, fd, sock, listener);
            int rc = addToEventLoop(fd, slot);
            if (0 == rc) { 
                // this FD has been added to the slot
                return 0;
            } else {
                freeSlot(slot);
                printf("Failed to add FD to event loop\n");
                return -1;
            }
        }
    }
    printf("Failed to add FD to event loop. Reached max slots\n");
    return -1;
}


int EventLoop::addToEventLoop(int fd, int i) {
    struct epoll_event my_poller_event;
    assert(epfd >= 0);
    my_poller_event.data.fd = i;   // slot number! 
    my_poller_event.events = EPOLLIN;

    int rc;
    rc = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &my_poller_event);
    if (rc) {
        perror("epoll_ctl - could not add");
        return -1;
    }
    return 0;
}

void EventLoop::deleteAllFds() {
    for (int slot = 0; slot < MAX_FD_IN_LOOP; slot++) {
        int fd = fds[slot]; // temporarily store the fd, because 
                         // freeSlot will remove that FD from the array
        if (fd > 0) {
            freeSlot(slot); // listener is not called anymore
            deleteFromEventLoop2(fd, slot);
        }
    }
}

int EventLoop::deleteFromEventLoop2(int fd, int slot) {
    assert(epfd > 0);
    assert(fd > 0);
    assert (slot >= 0 && slot < MAX_FD_IN_LOOP);
  
    struct epoll_event my_poller_event;

    my_poller_event.data.fd = slot;
    my_poller_event.events = EPOLLIN;

    int rc;
    rc = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &my_poller_event);
    if (rc) {
        perror("epoll_ctl - could not delete fd");
        return -1;
    }
    return 0;
}

int EventLoop::deleteFromEventLoop(int fd) {
    assert (fd > 0);
    int slot;
    for (slot = 0; slot < MAX_FD_IN_LOOP; slot++) {
        if (fd == fds[slot]) {
            // found this FD at this slot
            // free up the slot first, so that the listener is not working 
            // WARNING:  fds array element is clear after this point
            freeSlot(slot); // listener is not called anymore
            // WARNING:  fds/listeners array elements is clear after this point
            deleteFromEventLoop2(fd, slot);  // does not use fds/listeners
            return 0;
        }
    }

    printf("Could not find the FD in the monitor list");
    return -1;
}

void EventLoop::initLocalData() {
    for (int slot = 0; slot < MAX_FD_IN_LOOP; slot++) {
        fds[slot] = 0;
        userData[slot] = 0;
        listeners[slot] = NULL;
    }
}

int EventLoop::initLoopFd() {
    epfd = epoll_create1(EPOLL_CLOEXEC);
    assert(epfd > 0);
    printf("Opened EPOLL with FD: %d\n", epfd);
    return 0;
}

int EventLoop::deInitLoopFd() {
    close (epfd);
    epfd = 0; 
}

void EventLoop::deInitLocalData() {
    for (int slot = 0; slot < MAX_FD_IN_LOOP; slot++) {
        if (fds[slot] != 0) {
            printf("PROG_WARNING:  File descriptor/socket descriptor NOT closed.");
            // they may be closed elsewhere in the application
            // programmer warning, not a customer warning
        }
        fds[slot] = 0;
        userData[slot] = 0;
        listeners[slot] = NULL;
    }
}

void Application::initialize(const int argc, const char *argv[]) {
    eventLoop = NULL;
    strlcpy(progName, argv[0], sizeof(progName));
   
    const char *optAppName;
    if (argc > 1) {
        optAppName = argv[1];
    } else {
        optAppName = argv[0];
    }
    strlcpy(appName, optAppName, sizeof(appName));
    appInstance = 0;
    if (argc > 2) {
        int optint = atoi(argv[2]);
        if (optint >= 0  &&  optint < 16) {
            appInstance = optint;
        }
    }
}

Application::Application(const int argc, const char *argv[])
{
    initialize(argc, argv);
//    eventLoop = NULL;
//    strlcpy(progName, argv[0], sizeof(progName));
//   
//    const char *optAppName;
//    if (argc > 1) {
//        optAppName = argv[1];
//    } else {
//        optAppName = argv[0];
//    }
//    strlcpy(appName, optAppName, sizeof(appName));
//    appInstance = 0;
//    if (argc > 2) {
//        int optint = atoi(argv[2]);
//        if (optint >= 0  &&  optint < 16) {
//            appInstance = optint;
//        }
//    }
}

Application::Application()
{
    const char *argv[] = {"PROGRAM", "Application", "1"};
    const int argc = 3;
    initialize(argc, argv);
    
//    eventLoop = NULL;
//    strlcpy(progName, argv[0], sizeof(progName));
//   
//    const char *optAppName;
//    if (argc > 1) {
//        optAppName = argv[1];
//    } else {
//        optAppName = argv[0];
//    }
//    strlcpy(appName, optAppName, sizeof(appName));
//    appInstance = 0;
//    if (argc > 2) {
//        int optint = atoi(argv[2]);
//        if (optint >= 0  &&  optint < 16) {
//            appInstance = optint;
//        }
//    }
}

int Application::initServerActivities() {
    eventLoop = new EventLoop();  // any activity needs an eventloop
    if (eventLoop == NULL)
        return -1;
    return 0;
}


Application::~Application()
{
    if (eventLoop) {
        delete eventLoop;
        eventLoop = NULL;
    }
}

void Application::run() {
    if (eventLoop == NULL) {
        // there is no event loop, there is nothing to do
        printf("There is nothing to do in this application. No eventloop configured.\n");
        return;
    }
    
    // there is an eventloop, but if there is no FD configured, there is 
    // nothing to do
    eventLoop->run(); 
}