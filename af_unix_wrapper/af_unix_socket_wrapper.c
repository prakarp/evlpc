#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// #include <time.h>
// #include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "af_unix_socket_wrapper.h"

int logtx(int fd, char *buf, int buflen) {
    struct sockaddr_un da;
    int cc;

    da.sun_family = AF_UNIX;
    strncpy(da.sun_path, LOG_SOCK_NAME,
            (sizeof (struct sockaddr_un) - sizeof (short)));

    while (buflen > 0) {

        // cc = send(fd, buf, buflen, 0);
        cc = sendto (fd, buf, buflen, 0, (struct sockaddr*)&da,
                sizeof(struct sockaddr_un));
        if (cc == -1) {
            perror("send");
            return -1; // exit(0);
        }
        buf += cc;
        buflen -= cc;
    }
    return 0;
}

int logrx(int fd, char *buf, int buflen) {
    int cc;
    while (buflen > 0) {
        // cc = recv(fd, buf, buflen, 0);
        cc = recvfrom(fd, buf, buflen, 0, NULL, NULL);
        /* recv is for connected sockets only.
         recvfrom works for both.. but use NULL for unconnected sockets.
         try recvmsg() for strucutred messages. */
        if (cc == -1) {
            perror("recvfrom ");
            return -1;
        }
        buf += cc;
        buflen -= cc;
    }
    return 0;
}

// for a logging message server, pass a large number
// for a client, do not need this number. however, to receive logging-config
// from some config-server, max_log_connections should be 1
// socket_name should be "SOCK_NAME"
int log_open(int log_mode, int max_log_connections, int socket_mode) {
    int i = sizeof (struct sockaddr_un);
    int ns;
    int s;
    int pid;
    int s_mode;
    struct sockaddr_un sa;

    if ((log_mode != LOG_MODE_SERVER) &&
            (log_mode != LOG_MODE_CLIENT))
        return -2;

    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, LOG_SOCK_NAME,
            (sizeof (struct sockaddr_un) - sizeof (short)));

    if (LOG_SOCKET_DGRAM == socket_mode) {
        s_mode = SOCK_DGRAM;
    } else {
        s_mode = SOCK_SEQPACKET;
    }

    if ((s = socket(AF_UNIX, s_mode, 0)) == -1) {
        perror("game - socket failed");
        exit(0);
    }


    if (log_mode == LOG_MODE_CLIENT) {
        // clients, simply return if the server is datagram mode
        if (LOG_SOCKET_DGRAM == socket_mode) {
            return s;
        }
        // if the server is in the connected mode, attempt to connect to the server
        if ((connect(s, (struct sockaddr *)&sa, sizeof (struct sockaddr_un))) == -1) {
            perror("connect failed");
            exit(0);
        }
        return s;
    }

    // server mode, bind and start listening
    if ((bind(s, (struct sockaddr *)&sa, sizeof (struct sockaddr_un))) == -1) {
        perror("game - bind failed");
        exit(0);
    }
   
    // do not listen for connections, return immediately if in the datagram mode
    if (LOG_SOCKET_DGRAM == socket_mode) {
        return s;
    }

    if ((listen(s, 0)) == -1) {
        perror("catch - listen failed");
        exit(0);
    }

    while (max_log_connections--) {

        i = sizeof (struct sockaddr_un);
        if ((ns = accept(s, (struct sockaddr *)&sa, &i)) == -1) {
            if (errno == EINTR)
                continue;
            perror("catch - accept failed");
            exit(0);
        }
        if ((pid = fork()) != 0) {
            // parent process, only listen to connections and spawn
            // new threads
            close(ns);
            continue; // proceed to accepting new connections
        } else {
            // child process, start receiving message
            return ns;  // return the new socket opened for the child
                        // this is the server socket
        }
    }
    // used up all log connections
    return -1;
}

void log_close(int s) {
    close (s);
}

