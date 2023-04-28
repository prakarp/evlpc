/* 
 * File:   main.c
 * Author: prakash
 *
 * Created on September 30, 2010, 3:45 PM
 */

/*
 *      Sample Program: AF_UNIX stream sockets, server process
 *
 *      CATCH - RECEIVE DATA FROM THE PITCHER
 *
 *      Pitch and catch set up a simple unix domain stream socket
 *      client-server connection. The client (pitch) then sends
 *      data to server (catch), throughput is calculated, and the
 *      result is printed to the client's stdout.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/time.h>

#define SOCKNAME        "/tmp/p_n_c"
#define BUFSIZE         32*1024-1
int timeout();
int s; /* server socket */
char buffer[BUFSIZE];

struct bullet {
    int bytes;
    int throughput;
    int magic;
} bullet = {0, 0, 0};

send_data(int fd, char *buf, int buflen)
{
    int cc;
    while (buflen > 0) {
        cc = send(fd, buf, buflen, 0);
        if (cc == -1) {
            perror("send");
            exit(0);
        }
        buf += cc;
        buflen -= cc;
    }
}

recv_data(int fd, char *buf, int buflen)
{
    int cc;
    while (buflen > 0) {
        cc = recv(fd, buf, buflen, 0);
        if (cc == -1) {
            perror("recv");
            exit(0);
        }
        buf += cc;
        buflen -= cc;
    }
}

main(int argc, char *argv[])
{
    int bufsize, bytes, cc, i, total, pid, counter_pid;
    float msec;
    struct timeval tp1, tp2;
    int ns, recvsize, secs, usec;
    struct timezone tzp;
    struct sockaddr_un sa;
    /*
     * The SIGPIPE signal will be received if the peer has gone away
     * and an attempt is made to write data to the peer. Ignoring this
     * signal causes the write operation to receive an EPIPE error.
     * Thus, the user is informed about what happened.
     */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCLD, SIG_IGN);
    signal(SIGINT, timeout);
    setbuf(stdout, 0);
    setbuf(stderr, 0);
    if (argc > 1) {
        argv++;
        counter_pid = atoi(*argv++);
    } else
        counter_pid = 0;
    /*
     * Set up the socket variables - address family, socket name.
     * They'll be used later to bind() the name to the server socket.
     */
    sa.sun_family = AF_UNIX;
    unlink (SOCKNAME);
    strncpy(sa.sun_path, SOCKNAME,
            (sizeof (struct sockaddr_un) - sizeof (short)));
    /*
     * Create the server socket
     */
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("catch - socket failed");
        exit(0);
    }
    bufsize = BUFSIZE;
    /*
     * Use setsockopt() to change the socket buffer size to improve
     * throughput for large data transfers
     */
    if ((setsockopt(s, SOL_SOCKET, SO_RCVBUF, &bufsize,
            sizeof (bufsize)))
            == -1) {
        perror("catch - setsockopt failed");
        exit(0);
    }
    /*
     * Bind the server socket to its name
     */
    if ((bind(s, (struct sockaddr *) &sa, sizeof (struct sockaddr_un))) == -1) {
        perror("catch - bind failed");
        exit(0);
    }
    /*
     * Call listen() to enable reception of connection requests
     * (listen() will silently change given backlog 0, to be 1 instead)
     */
    if ((listen(s, 0)) == -1) {
        perror("catch - listen failed");
        exit(0);
    }
next_conn:
    i = sizeof (struct sockaddr_un);
    /*
     * Call accept() to accept connection request. This call will block
     * until a connection request arrives.
     */
    if ((ns = accept(s, (struct sockaddr *) &sa, &i)) == -1) {
        if (errno == EINTR)
            goto next_conn;
        perror("catch - accept failed");
        exit(0);
    }
    if ((pid = fork()) != 0) {
        close(ns);
        goto next_conn;
    }
    /*
         close(s);
     */
    /*
     * Receive the bullet to synchronize with the other side
     */
    recv_data(ns, &bullet, sizeof (struct bullet));
    if (bullet.magic != 12345) {
        printf("catch: bad magic %d\n", bullet.magic);
        exit(0);
    }
    bytes = bullet.bytes;
    recvsize = (bytes > BUFSIZE) ? BUFSIZE : bytes;
    /*
     * Send the bullet back to complete synchronization
     */
    send_data(ns, &bullet, sizeof (struct bullet));
    cc = 0;
    if (counter_pid)
        kill(counter_pid, SIGUSR1);
    if (gettimeofday(&tp1, &tzp) == -1) {
        perror("catch time of day failed");
        exit(0);
    }
    /*
     * Receive data from the client
     */
    total = 0;
    i = bytes;
    while (i > 0) {
        cc = recvsize < i ? recvsize : i;
        recv_data(ns, buffer, cc);
        total += cc;
        i -= cc;
    }
    /*
     * Calculate throughput
     */
    if (gettimeofday(&tp2, &tzp) == -1) {
        perror("catch time of day failed");
        exit(0);
    }
    if (counter_pid)
        kill(counter_pid, SIGUSR2);
    secs = tp2.tv_sec - tp1.tv_sec;
    usec = tp2.tv_usec - tp1.tv_usec;
    if (usec < 0) {
        secs;
        usec += 1000000;
    }
    msec = 1000 * (float) secs;
    msec += (float) usec / 1000;
    bullet.throughput = bytes / msec;
    /*
     * Send back the bullet with throughput info, then close the
     * server socket
     */
    if ((cc = send(ns, &bullet, sizeof (struct bullet), 0)) == -1) {
        perror("catch - send end bullet failed");
        exit(0);
    }
    close(ns);
}

timeout() {
    printf("alarm went off -- stopping the catch process\n");
    fprintf(stderr, "stopping the catch process\n");
    unlink(SOCKNAME);
    close(s);
    exit(6);
}
