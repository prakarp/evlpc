
/* 
 * File:   main.cpp
 * Author: pj
 *
 * Created on September 20, 2014, 8:44 PM
 * http://tim.dysinger.net/posts/2013-09-16-getting-started-with-nanomsg.html
 */
#include <iostream>
#include <assert.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define SERVER "server"
#define CLIENT "client"

char *date ()
{
  time_t raw = time (&raw);
  struct tm *info = localtime (&raw);
  char *text = asctime (info);
  text[strlen(text)-1] = '\0'; // remove '\n'
  return text;
}

int server (const char *url)
{
  int sock = nn_socket (AF_SP, NN_PUB);
  assert (sock >= 0);
  assert (nn_bind (sock, url) >= 0);
  while (1)
    {
      //char *d = date();
      char d[100];
      int sz_d, bytes;
      strcpy (d, "date seven");
      sz_d = strlen(d) + 1; // '\0' too
      printf ("SERVER: PUBLISHING DATE %s\n", d);
      bytes = nn_send (sock, d, sz_d, 0);
      assert (bytes == sz_d);
      
      strcpy (d, "time");
      sz_d = strlen(d) + 1; // '\0' too
      printf ("SERVER: PUBLISHING TIME %s\n", d);
      bytes = nn_send (sock, d, sz_d, 0);
      assert (bytes == sz_d);
      
      sleep(1);
    }
  return nn_shutdown (sock, 0);
}

int client (const char *url, const char *name)
{
  int sock = nn_socket (AF_SP, NN_SUB);
  assert (sock >= 0);
  
  assert (nn_connect (sock, url) >= 0);
  assert (nn_setsockopt (sock, NN_SUB, NN_SUB_SUBSCRIBE, "date", 4) >= 0);
  // assert (nn_setsockopt (sock, NN_SUB, NN_SUB_SUBSCRIBE, "time", 4) >= 0);
  while (1)
    {
      char *buf = NULL;
      int bytes = nn_recv (sock, &buf, NN_MSG, 0);
      assert (bytes >= 0);
      printf ("CLIENT (%s): RECEIVED %s\n", name, buf);
      nn_freemsg (buf);
    }
  return nn_shutdown (sock, 0);
}

#if 0
int node0(const char *url) {
    int rc;
    int DEBUG = 1;

    int sock = nn_socket(AF_SP, NN_PULL);
    assert(sock >= 0);
    assert(nn_bind(sock, url) >= 0);

    struct nn_pollfd pfd [1];
    pfd[0].fd = sock;
    pfd [0].events = NN_POLLIN;

    while (1) {
        // https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
        rc = nn_poll(pfd, 1, 2000);
        if (rc <= 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            perror("Poll failed");
            continue;
        }

        if (pfd [0].revents & NN_POLLIN) {
            printf("Message can be received from s1!");
            // this is the network fd           
            char *buf = NULL;
            int bytes = nn_recv(sock, &buf, NN_MSG, 0);
            assert(bytes >= 0);
            if (DEBUG) {
                printf("NODE0: RECEIVED \"%s\"\n", buf);
            }
            nn_freemsg(buf);
        }
    }
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
#endif

int main(const int argc, const char **argv) {
    char node_name[10];
    node_name[0] = 0;

    if (argc > 1) {
        strncpy(node_name, argv[1], sizeof (node_name) - 1);
        node_name[sizeof (node_name) - 1] = 0;
    }

    if (!((strcmp(node_name, SERVER) == 0) || (strcmp(node_name, CLIENT) == 0))) {
        printf("First argument should be either %s or %s\n", SERVER, CLIENT);
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

    if ((strcmp(node_name, CLIENT) == 0) && (strlen(msg) == 0)) {
        printf("Third argument for %s must be a simple message text\n", CLIENT);
        exit(-1);
    }

    int starting_number = 1;
    if (argc > 4) {
        starting_number = atoi(argv[4]);
    }

    if (strncmp(SERVER, node_name, strlen(SERVER)) == 0 && argc > 1)
        return server(url);
    else if (strncmp(CLIENT, node_name, strlen(CLIENT)) == 0 && argc > 2)
        return client(url, msg);
    else {
        fprintf(stderr, "Usage: %s %s|%s <URL> <ARG> ...'\n",
                argv[0], SERVER, CLIENT);
        return 1;
    }
}

