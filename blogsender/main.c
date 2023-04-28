/* 
 * File:   main.c
 * Author: prakash
 *
 * Created on September 30, 2010, 4:27 PM
 */


/*
 *       Sample Program : AF_UNIX stream sockets, client process
 *
 *       PITCH - SEND DATA TO THE CATCHER
 *
 *       Pitch and catch set up a simple unix domain stream socket
 *       client-server connection. The client (pitch) then sends
 *       data to the server (catch), throughput is calculated, and
 *       the result is printed to the client's stdout.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKNAME    "/tmp/p_n_c"
#define BUFSIZE          32*1024-1
char buffer[BUFSIZE];

struct bullet {
    int bytes;
    int throughput;
    int magic;
} bullet = {0, 0, 12345};

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
    int bufsize, bytes, cc, i, total, pid;
    float msec;
    struct timeval tp1, tp2;
    int s, sendsize, secs, usec;
    struct timezone tzp;
    struct sockaddr_un sa;
    /*
     * The SIGPIPE signal will be received if the peer has gone away
     * and an attempt is made to write data to the peer.  Ignoring
     * the signal causes the write operation to receive an EPIPE error.
     * Thus, the user is informed about what happened.
     */
    signal(SIGPIPE, SIG_IGN);
    setbuf(stdout, 0);
    setbuf(stderr, 0);
    if (argc < 2) {
        printf("usage: pitch Kbytes [pid]\n");
        exit(0);
    }
    argv++;
    /*
     * Set up socket variables (address family; name of server socket)
     * (they'll be used later for the connect() call)
     */
    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, SOCKNAME,
            (sizeof (struct sockaddr_un) - sizeof (short)));
    bullet.bytes = bytes = 1024 * atoi(*argv++);
    if (argc > 2)
        pid = atoi(*argv++);
    else
        pid = 0;
    sendsize = (bytes < BUFSIZE) ? bytes : BUFSIZE;
    /*
     * Create the client socket
     */
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("pitch - socket failed");
        exit(0);
    }
    bufsize = BUFSIZE;
    /*
     * Change the default buffer size to improve throughput for
     * large data transfers
     */
    if ((setsockopt(s, SOL_SOCKET, SO_SNDBUF, &bufsize,
            sizeof (bufsize)))
            == -1) {
        perror("pitch - setsockopt failed");
        exit(0);
    }
    /*
     * Connect to the server
     */
    if ((connect(s, &sa, sizeof (struct sockaddr_un))) == -1) {
        perror("pitch - connect failed");
        exit(0);
    }
    /*
     * send and receive the bullet to synchronize both sides
     */
    send_data(s, &bullet, sizeof (struct bullet));
    recv_data(s, &bullet, sizeof (struct bullet));
    cc = 0;
    if (pid)
        kill(pid, SIGUSR1);
    if (gettimeofday(&tp1, &tzp) == -1) {
        perror("pitch time of day failed");
        exit(0);
    }
    i = bytes;
    total = 0;
    /*
     * Send the data
     */
    while (i > 0) {
        cc = sendsize < i ? sendsize : i;
        send_data(s, buffer, cc);
        i -= cc;
        total += cc;
        sleep (1);
    }
    /*
     * Receive the bullet to calculate throughput
     */
    recv_data(s, &bullet, sizeof (struct bullet));
    if (gettimeofday(&tp2, &tzp) == -1) {
        perror("pitch time of day failed");
        exit(0);
    }
    if (pid)
        kill(pid, SIGUSR2);
    /*
     * Close the socket
     */
    close(s);
    secs = tp2.tv_sec - tp1.tv_sec;
    usec = tp2.tv_usec - tp1.tv_usec;
    if (usec < 0) {
        secs;
        usec += 1000000;
    }
    msec = 1000 * (float) secs;
    msec += (float) usec / 1000;
    printf("PITCH: %d Kbytes/sec\n", (int) (bytes / msec));
    printf("CATCH: %d Kbytes/sec\n", bullet.throughput);
    printf("AVG:   %d Kbytes/sec\n",
            ((int) (bytes / msec) + bullet.throughput) / 2);
}
