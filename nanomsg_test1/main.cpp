/* 
 * File:   main.cpp
 * Author: pj
 *
 * Created on September 20, 2014, 8:44 PM
 */
#include <iostream>
#include <assert.h>
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define NODE0 "node0"
#define NODE1 "node1"

int node0_blocking_recv(const char *url) {
    int sock = nn_socket(AF_SP, NN_PULL);
    assert(sock >= 0);
    assert(nn_bind(sock, url) >= 0);
    while (1) {
        char *buf = NULL;
        int bytes = nn_recv(sock, &buf, NN_MSG, 0);
        assert(bytes >= 0);
        printf("NODE0: RECEIVED \"%s\"\n", buf);
        nn_freemsg(buf);
    }
}

//struct nn_poller my_poller;
//struct nn_poller_hndl  my_net_poller_handle;
int epfd;
struct epoll_event my_poller_event;

int init_poller() {
    epfd = epoll_create1(EPOLL_CLOEXEC);
    assert(epfd >= 0);
    printf("Opened EPOLL with FD: %d\n", epfd);

    return 0;
}

int init_polling_fd(int fd) {
    assert(epfd >= 0);
    my_poller_event.data.fd = 100;
    my_poller_event.events = EPOLLIN;

    int rc;
    rc = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &my_poller_event);
    if (rc) {
        perror("epoll_ctl");
        return -1;
    }
    return 0;
}

int node0(const char *url) {
    int rc;

    rc = init_poller();
    assert(rc == 0);

#if 0
    rc = nn_poller_init(&my_poller);
    if (rc < 0) {
        printf("Error initializing the poller...\n");
        return 0;
    }
#endif

    int DEBUG = 1;
    int sock = nn_socket(AF_SP, NN_PULL);
    assert(sock >= 0);
    assert(nn_bind(sock, url) >= 0);

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
    rc = init_polling_fd(net_fd);
    assert(rc == 0);

    int MAX_EVENTS = 32;
    struct epoll_event *events;
    events = (struct epoll_event *)
            malloc(sizeof (struct epoll_event) * MAX_EVENTS);

    assert(events != NULL);
    int nr_events;

    while (1) {
        nr_events = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nr_events < 0) {
            if (EINTR == errno) {
                continue;
            }
            perror("epoll_wait");
            free(events);
            return 1;
        }
        int i;

        for (i = 0; i < nr_events; i++) {
            printf("event=%u on fd=%d \n",
                    events[i].events,
                    events[i].data.fd);

            /*
             * We now can, per events[i].events, operate on
             * events[i].data.fd without blocking.
             */
            if ((events[i].events & EPOLLERR) ||
              (events[i].events & EPOLLHUP) ||
              (!(events[i].events & EPOLLIN)))
	    {
              /* An error has occured on this fd, or the socket is not
                 ready for reading (why were we notified then?) */
	      perror ("epoll error ");
	      // close (events[i].data.fd);
	      exit(-1);
	    }
            // https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
            if (100 == events[i].data.fd) {
                // this is the network fd           
                char *buf = NULL;
                int bytes = nn_recv(sock, &buf, NN_MSG, 0);
                assert(bytes >= 0);
                if (DEBUG) {
                    printf("NODE0: RECEIVED \"%s\"\n", buf);
                }
                nn_freemsg(buf);
                //rc = init_polling_fd(net_fd);
                //assert(rc == 0);
            }
        }
    }

    close(epfd);

    return 0;
}

int node1(const char *url, const char *msg, const int starting_number) {
    int DEBUG = 1;
    int sz_msg; // = strlen(msg) + 1; // '\0' too
    int sock = nn_socket(AF_SP, NN_PUSH);
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


