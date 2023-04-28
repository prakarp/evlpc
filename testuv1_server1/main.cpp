/* 
 * File:   main.cpp
 * Author: pj
 *
 * Created on October 4, 2014, 5:16 PM
 */

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <uv.h>

using namespace std;

uv_loop_t *loop;
void on_new_connection(uv_stream_t *server, int status);
uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size);
void echo_read(uv_stream_t *server, ssize_t nread, uv_buf_t buf);

/*
 * 
 */
int main(int argc, char** argv) {

    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    struct sockaddr_in bind_addr = uv_ip4_addr("0.0.0.0", 7000);
    uv_tcp_bind(&server, bind_addr);
    int r = uv_listen((uv_stream_t*) & server, 128, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(uv_last_error(loop)));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status == -1) {
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof (uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        printf("Accepted connection...\n");
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    } else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

// suggeseted_size

uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size) {
    printf("Allocating %d bytes of buffer", (int) suggested_size);
    return uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void echo_read(uv_stream_t *server, ssize_t nread, uv_buf_t buf) {
    if (nread == -1) {
        fprintf(stderr, "error echo_read");
        return;
    }
    printf("server side result: %s\n", buf.base);
}




