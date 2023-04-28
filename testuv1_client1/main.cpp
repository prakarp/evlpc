#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

        
#define log(x) printf("%s\n", x);

uv_loop_t *loop;

void on_connect(uv_connect_t *req, int status);
void on_write_end(uv_write_t *req, int status);
uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size);
void echo_read(uv_stream_t *server, ssize_t nread, uv_buf_t buf);

void echo_read(uv_stream_t *server, ssize_t nread, uv_buf_t buf) {
    if (nread == -1) {
        fprintf(stderr, "error echo_read");
        return;
    }

    printf("result: %s\n", buf.base);
}

// suggeseted_size ã§æ¸¡ã•ã‚ŒãŸé ˜åŸŸã‚’ç¢ºä¿

uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size) {

    return uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void on_write_end(uv_write_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "error on_write_end");
        return;
    }

    // æ›¸ãè¾¼ã¿ãŒçµ‚ã‚ã£ãŸã‚‰ã€ã™ãã«èª­ã¿è¾¼ã¿ã‚’é–‹å§‹
    uv_read_start(req->handle, alloc_buffer, echo_read);
}

// ã‚µãƒ¼ãƒã¨ã®æŽ¥ç¶šã‚’ç¢ºç«‹å¾Œ, ã‚µãƒ¼ãƒã«æ–‡å­—åˆ—ã‚’é€ä¿¡

void on_connect(uv_connect_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "error on_write_end");
        return;
    }

    // é€ä¿¡ãƒ¡ãƒƒã‚»ãƒ¼ã‚¸ã‚’ç™»éŒ²
    char message[1000];
    strcpy(message, "hello.txt");
    int len = strlen(message);

    /** ã“ã‚Œã ã¨ã‚»ã‚°ãƒ•ã‚©
     * uv_buf_t buf[1];
     * buf[0].len = len;
     * buf[0].base = message;
     */

    char buffer[100];
    uv_buf_t buf = uv_buf_init(buffer, sizeof (buffer));
   
    buf.len = len;
    buf.base = message;

    // ãƒãƒ³ãƒ‰ãƒ«ã‚’å–å¾—
    uv_stream_t* tcp = req->handle;

    uv_write_t write_req;

    int buf_count = 1;

    uv_write(&write_req, tcp, &buf, buf_count, on_write_end);
}




int main(void) {
    // loop ç”Ÿæˆ
    loop = uv_default_loop();

    // Network I/O ã®æ§‹é€ ä½“
    uv_tcp_t client;

    // loop ã¸ã®ç™»éŒ²
    uv_tcp_init(loop, &client);

    // ã‚¢ãƒ‰ãƒ¬ã‚¹ã®å–å¾—
    struct sockaddr_in req_addr = uv_ip4_addr("127.0.0.1", 7000);

    // TCP ã‚³ãƒã‚¯ã‚·ãƒ§ãƒ³ç”¨ã®æ§‹é€ ä½“
    uv_connect_t connect_req;

    // æŽ¥ç¶š
    uv_tcp_connect(&connect_req, &client, req_addr, on_connect);

    // ãƒ«ãƒ¼ãƒ—ã‚’é–‹å§‹
    return uv_run(loop, UV_RUN_DEFAULT);
}
