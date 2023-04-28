
/* 
 * File:   main.cpp
 * Author: pj
 *
 * Created on September 20, 2014, 8:44 PM
 * http://tim.dysinger.net/posts/2013-09-16-getting-started-with-nanomsg.html
 */
#include <libcommon.h>
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/tcp.h>

#define NODE0 "node0"
#define NODE1 "node1"

int process_event2(int fd, int sock, struct epoll_event *event) {
    const int DEBUG = 1;
    printf("Processing event from FD %d, NNsock %d\n", fd, sock);
    char *buf = NULL;
    int bytes = nn_recv(sock, &buf, NN_MSG, 0);
    assert(bytes >= 0);
    if (DEBUG) {
        printf("NODE0: RECEIVED \"%s\"\n", buf);
    }
    nn_freemsg(buf);
    return 0;
}

int process_event(int fd, int sock, struct epoll_event *event) {
    const int DEBUG = 1;
    printf("Processing event from FD %d, NNsock %d Events 0x%x\n", fd, sock,
            event->events);
    
    if (event->events & EPOLLIN) {
        char *buf = NULL;
        int bytes = nn_recv(sock, &buf, NN_MSG, 0);
        assert(bytes >= 0);
        if (DEBUG) {
            printf("NODE0: RECEIVED \"%s\"\n", buf);
        }
        nn_freemsg(buf);
    }
    return 0;
}


int node0(const char *url) {
    int rc;
    int DEBUG = 1;

    int sock = nn_socket(AF_SP, NN_REP);
    printf ("Opened NN server side socket %d\n", sock);
    
    assert(sock >= 0);   // WARNING: Odd but it is ok for this socket to be zero, 
                        // and it is within this application namespace!
    printf ("Binding to URL: %s\n", url);
    rc = nn_bind(sock, url);
    if (rc < 0) {
        perror ("What happened to binding to the URL");
        printf("RC : %d\n", rc);
    }
    assert(rc >= 0);

    Application *app = new Application();
    int x = app->initServerActivities();
    EventLoop *eventLoop = app->getEventLoop();
    EventListener *listener = new EventListener();
    listener->callback = process_event;
    
    int net_fd = 0;
    size_t sz;
    sz = sizeof(net_fd);
    rc = nn_getsockopt(sock, NN_SOL_SOCKET, NN_RCVFD,
            &net_fd, &sz);

    if (rc) {
        printf("Unable to get the FD of the nn socket\n");
        return -1;
    }
    assert(net_fd > 0);

    printf("Obtained FD: %d\n", net_fd);
    
    eventLoop->addFd(net_fd, sock, listener);
    app->run();

    printf("Application terminated.\n");
    return 0;
}

int node1(const char *url, const char *msg, const int starting_number) {
    int DEBUG = 1;
    int sz_msg; // = strlen(msg) + 1; // '\0' too
    int sock = nn_socket(AF_SP, NN_REQ);
    perror("Socket error ");
    assert(sock >= 0);
    assert(nn_connect(sock, url) >= 0);
    printf("NODE1: SENDING \"%s\"\n", msg);
    int n = 10;
    for (int i = 0; i < n; i++) {
        if (DEBUG) {
            printf("NODE1/%d: Sending msg with id: %d\n", starting_number,
                    i + starting_number);
        }
        char fmsg[128];
        snprintf(fmsg, sizeof (fmsg), "%s:%d", msg, i + starting_number);
        sz_msg = strlen(fmsg) + 1;
        int bytes = nn_send(sock, fmsg, sz_msg, 0);

        if (DEBUG) {
            printf("NODE1/%d: Sent %d bytes\n", starting_number, bytes);
            sleep(1); // sleep for a second
        }
        assert(bytes == sz_msg);
    }
    return nn_shutdown(sock, 0);
}

int main(const int argc, const char **argv) {
    char node_name[10];
    node_name[0] = 0;

    if (argc > 1) {
        strncpy(node_name, argv[1], sizeof (node_name) - 1);
        node_name[sizeof (node_name) - 1] = 0;
    }

    if (!((strcmp(node_name, NODE0) == 0) || (strcmp(node_name, NODE1) == 0))) {
        printf("First argument should be either %s or %s\n", NODE0, NODE1);
        exit(-1);
    }

    char url[128];
    url[0] = 0;
    if (argc > 2) {
        strncpy(url, argv[2], sizeof (url) - 1);
        url[sizeof (url) - 1] = 0;
    }

    if (strlen(url) == 0) {
        printf("Second argument should be the URL e.g. ipc:///tmp/reqrep.ipc\n");
        exit(-1);
    }

    char msg[128];
    msg[0] = 0;
    if (argc > 3) {
        strncpy(msg, argv[3], sizeof (msg) - 1);
        msg[sizeof (msg) - 1] = 0;
    }

    if ((strcmp(node_name, NODE1) == 0) && (strlen(msg) == 0)) {
        printf("Third argument for %s must be a simple message text\n", NODE1);
        exit(-1);
    }

    int starting_number = 1;
    if (argc > 4) {
        starting_number = atoi(argv[4]);
    }

    if (strncmp(NODE0, node_name, strlen(NODE0)) == 0 && argc > 1)
        return node0(url);
    else if (strncmp(NODE1, node_name, strlen(NODE1)) == 0 && argc > 2)
        return node1(url, msg, starting_number);
    else {
        fprintf(stderr, "Usage: pipeline %s|%s <URL> <ARG> ...'\n",
                NODE0, NODE1);
        return 1;
    }
}

