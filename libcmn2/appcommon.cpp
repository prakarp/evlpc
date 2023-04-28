#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <assert.h>
#include <syslog.h>
#include "appcommon.h"
#include "globals.h"

const int MAX_ALLOCATION_SIZE = 65536;

EventLoop::EventLoop(uint16_t defaultPort) {
    loop = uv_default_loop();
    loop->data = this;
    addDefaultServerPort(defaultPort);
    defaultListener = NULL;
    timer1Listener = NULL;
}

EventLoop::~EventLoop() {
    // TODO eventloop must be closed or removed?
    // orderly deletion/closure of all TCP connections
    // maximum time provided.. 

}

int EventLoop::run() {
    if (NULL == loop) {
        return -1;
    } else {
        printf("EventLoop: Starting the main eventloop\n");
        return uv_run(loop, UV_RUN_DEFAULT);
    }
}
static int once_close_cb_called = 0;

static void once_close_cb(uv_handle_t* handle) {
    printf("ONCE_CLOSE_CB\n");
    ASSERT(handle != NULL);
    ASSERT(0 == uv_is_active(handle));
    once_close_cb_called++;
}

static int once_cb_called = 0;

static void once_cb(uv_timer_t* handle, int status) {

    printf("ONCE_CB %d\n", once_cb_called);
    ASSERT(handle != NULL);
    ASSERT(0 == uv_is_active((uv_handle_t*) handle));
    once_cb_called++;

    uv_loop_t *loop = handle->loop;
    EventLoop *eventLoop = (EventLoop *) loop->data;
    Timer1ListenerMethod timer1Listener = eventLoop->timer1Listener;
    if (NULL != timer1Listener) {
        timer1Listener(handle, status);
    }

#if 0
    // do not close it, if there is a plan to restart the timer.
    uv_close((uv_handle_t*) handle, once_close_cb);
#endif
    /* Just call this randomly for the code coverage. */
    uv_update_time(handle->loop);
}

int EventLoop::initTimer1(int delay, int period) {
    uv_timer_t *once = &this->onceTimer1;
    int r;
    r = uv_timer_init(loop, once);
    assert(r == 0);
    r = uv_timer_start(once, once_cb, delay, period);
    assert(r == 0);
}

int EventLoop::reinitTimer1(int delay, int period) {
    uv_timer_t *once = &this->onceTimer1;
    int r;
    r = uv_timer_start(once, once_cb, delay, period);
    assert(r == 0);
}

void onNewConnection(uv_stream_t *server, int status);
void readData(uv_stream_t *server, ssize_t nread, uv_buf_t buf);
uv_buf_t allocBuffer(uv_handle_t *handle, size_t suggested_size);

int EventLoop::addDefaultServerPort(uint16_t defaultPort) {

    uv_tcp_init(loop, &defaultServer);

    struct sockaddr_in bind_addr = uv_ip4_addr("0.0.0.0", defaultPort);
    uv_tcp_bind(&defaultServer, bind_addr);
    defaultServer.data = NULL;

    int r = uv_listen((uv_stream_t*) & defaultServer, 128, onNewConnection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(uv_last_error(loop)));
        return 1;
    }
    return 0;
};

uv_buf_t allocBuffer(uv_handle_t *handle, size_t suggested_size) {
    // handle is basically the client data structure in a TCP connection
    printf("Allocating Memory: Handle Event Loop %p (client) Handle %p, Data: %p\n",
            handle->loop, handle, handle->data);
    // printf("Allocating %d bytes of buffer", (int) suggested_size);
    RxBufAddr_t *rx_buf_info = (RxBufAddr_t *) handle->data;

    if (rx_buf_info->rx_buf_addr == NULL) {

        int allocating_size;
        if (suggested_size > MAX_ALLOCATION_SIZE) {
            allocating_size = MAX_ALLOCATION_SIZE;
        } else {
            allocating_size = suggested_size;
        }
        // no buffer has been allocated, allocate one now
        rx_buf_info->rx_buf_addr = (char *) malloc(allocating_size);
        assert(rx_buf_info->rx_buf_addr != NULL);
        printf("Allocated memory %p\n", rx_buf_info->rx_buf_addr);

        rx_buf_info->leftover_data_size = 0;
        rx_buf_info->rx_buf_size = allocating_size; // never ever change this!

        return uv_buf_init(rx_buf_info->rx_buf_addr,
                rx_buf_info->rx_buf_size);
    } else {
        // memory has been allocated already, but we may have unprocessed 
        // data at the end, so return a uv_buf data structure from the 
        // offset
        if (rx_buf_info->leftover_data_size == 0) {
            return uv_buf_init(rx_buf_info->rx_buf_addr,
                    rx_buf_info->rx_buf_size);
        } else {
            return uv_buf_init(&rx_buf_info->rx_buf_addr[rx_buf_info->leftover_data_size],
                    rx_buf_info->rx_buf_size - rx_buf_info->leftover_data_size);
        }
    }
#if 0    
    char *buf = (char *) malloc(allocating_size);
    printf("Allocated memory %p\n", buf);
    return uv_buf_init(buf, allocating_size);
#endif
}

void readClientConnections(uv_stream_t *client) {
    printf("Accepted connection...client %p\n", client);
    syslog(LOG_INFO, "Accepted connection... client %p", client);

    RxBufAddr_t *rx_buf_info;
    rx_buf_info = (RxBufAddr_t *) malloc(sizeof (RxBufAddr_t));
    rx_buf_info->leftover_data_size = 0;
    rx_buf_info->rx_buf_addr = NULL;
    client->data = rx_buf_info;

    printf("Client buffer data : %p\n", client->data);

    uv_read_start((uv_stream_t*) client,
            allocBuffer,
            readData);
    return;
}

void onNewConnection(uv_stream_t *server, int status) {
    if (status == -1) {
        // error!
        return;
    }
    printf("New connection at the server side\n");
    syslog(LOG_INFO, "New connection");

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof (uv_tcp_t));
    client->data = NULL;

    //printf("New connection: Server EventLoop: %p\n", server->loop);
    uv_tcp_init(server->loop, client);
    //printf("New connection Client EventLoop: %p\n", client->loop);

    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        printf("Accepted connection...server %p server data: %p client %p\n",
                server, server->data, client);
        syslog(LOG_INFO, "Accepted connection... client %p", client);

        RxBufAddr_t *rx_buf_info;
        rx_buf_info = (RxBufAddr_t *) malloc(sizeof (RxBufAddr_t));
        rx_buf_info->leftover_data_size = 0;
        rx_buf_info->rx_buf_addr = NULL;
        client->data = rx_buf_info;

        printf("Client buffer data : %p, but actual buffer space not allocated yet\n", 
                client->data);

        uv_read_start((uv_stream_t*) client,
                allocBuffer,
                readData);
        // TODO store all clients in a list and close those, when terminating
    } else {
        // Free the memory after closing the client's socket.
        printf("Closing the connection to the client %p\n", client);
        uv_close((uv_handle_t*) client, NULL);
        free(client); // it is ok to pass this as a callback function too, 
        // e.g. (uv_close_cb)free) as the second argument above
        // but there may be a bug in libuv
        client = NULL; // nullify so that there are no more errors
    }
}

static void client_close_cb(uv_handle_t* handle) {
    // ASSERT(handle != NULL);  
    uv_stream_t *stream = (uv_stream_t *) handle;
    printf("Close callback. stream/handle %p  buf_info %p, connect_t %p\n", 
            stream,
            stream->data, stream->connect_req);

    RxBufAddr_t *rx_buf_info = (RxBufAddr_t *) stream->data;
    // discard all data that has been read so far, whether processed or not.
    if (rx_buf_info) {
        rx_buf_info->leftover_data_size = 0;
    }
    
#if 0
    // KEEP THIS
    // This code is to free up the memory allocated for the buffer
    // space allocated for the client. Works, but no need to free up the 
    // space. 
    // When walking through the list of connections, it is easy to look
    // at whether the client is active or not. 
    // Call uv_tcp_connect on the same connect_req structure if necessary.
    RxBufAddr_t *rx_buf_info = (RxBufAddr_t *) stream->data;
    if (rx_buf_info) {

        if (rx_buf_info->rx_buf_addr) {
            printf("Freeing rx buffer: %p, size: %u\n", rx_buf_info->rx_buf_addr,
                    rx_buf_info->rx_buf_size);
            free(rx_buf_info->rx_buf_addr);
            rx_buf_info->rx_buf_addr = NULL;
            rx_buf_info->rx_buf_size = 0;
        }

        rx_buf_info->leftover_data_size = 0;
        free(rx_buf_info);
        stream->data = NULL;
    }
#endif
    
}

void readData(uv_stream_t *connection, ssize_t nread, uv_buf_t buf) {
    if (nread == -1) { // may compare with EOF also which is -1
        // when the client abnormally terminates
        // uv_stream_t -> uv_connect_t is NULL even in the normal case
        // i.e. connection->connect_req is null
        printf("Reading data failed. Closing stream %p\n", connection);
        // printf("GOT EOF Error echo_read - closing connection, freeing memory for buffers\n");
        // syslog(LOG_INFO, "EOF error received.. closing connection, freeing memory for buffers");

        uv_close((uv_handle_t*) connection, client_close_cb);
        return;
    }

    // connection here is the client data structure in a TCP connection
    // For a client side connection, connection->connect_req is null
    // i.e. uv_stream_t -> uv_connect_t is NULL
    printf("Reading from stream (client) %p , (buf_info)  Data: %p\n",
            connection, connection->data);
    
    // printf("server side result: %s\n", buf.base);
    uint32_t leftover_data_size;

    leftover_data_size = 0;

    uv_loop_t *loop = connection->loop;
    RxBufAddr_t *rx_buf_info = (RxBufAddr_t *) connection->data;

    uint32_t previous_unread_data = rx_buf_info->leftover_data_size;
    uint32_t total_data_read = previous_unread_data + nread;

    EventLoop *eventLoop = (EventLoop *) loop->data;
    ListenerMethod listenerMethod = eventLoop->defaultListener;
    if (NULL != listenerMethod) {
        leftover_data_size = listenerMethod(connection, total_data_read, buf);
        if (leftover_data_size == 0) {
            // all data has been processed
            rx_buf_info->leftover_data_size = 0;
        } else {
            // number of bytes processed = 
            //     total_data_read - leftover_data_size;
            // example 1:  received: 100 bytes,  left: 6 bytes
            //     so copy the last 6 bytes from offset 100-6 to the beginning
            // example 2:  had: 6, rx: 100,  total: 106,  left: 6 bytes
            //     so copy the last 6 bytes from 106-6 to the beginning
            // make sure the left over data is stored in the buffer so that it is 
            // not overwritten. Next time a read is done, it should be stored after
            // the last few bytes
            uint32_t unprocessed_data_offset = total_data_read - leftover_data_size;
            memcpy((char *) &rx_buf_info->rx_buf_addr[0],
                    (char *) &rx_buf_info->rx_buf_addr[unprocessed_data_offset],
                    leftover_data_size);
            rx_buf_info->leftover_data_size = leftover_data_size;
        }
    }
    // TODO next
    // free the memory allocated earlier
    //   printf("Freeing Allocated Memory %p\n", buf.base);
    //  free(buf.base);
}

void Application::initialize(const int argc, const char *argv[]) {
    eventLoop = NULL;
    strncpy(progName, argv[0], sizeof (progName) - 1);

    const char *optAppName;
    if (argc > 1) {
        optAppName = argv[1];
    } else {
        optAppName = argv[0];
    }
    strncpy(appName, optAppName, sizeof (appName) - 1);
    appInstance = 0;
    if (argc > 2) {
        int optint = atoi(argv[2]);
        if (optint >= 0 && optint < 16) {
            appInstance = optint;
        }
    }

    initlog(optAppName);
#if 0
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog(optAppName, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
#endif
}

Application::Application(const char *progname, const char *app_id,
        const char *instance_id) {
    const char *argv[] = {progname, app_id, instance_id};
    const int argc = 3;
    initialize(argc, argv);
}

Application::Application(const int argc, const char *argv[]) {
    initialize(argc, argv);
}

Application::Application() {
    const char *argv[] = {"PROGRAM", "Application", "1"};
    const int argc = 3;
    initialize(argc, argv);
}

int Application::initServerActivities(uint16_t defaultPort) {
    printf("Initializing eventloop with default port %u\n", defaultPort);
    eventLoop = new EventLoop(defaultPort); // any activity needs an eventloop
    if (eventLoop == NULL)
        return -1;
    eventLoop->initTimer1();
    return 0;
}

Application::~Application() {
    if (eventLoop) {
        delete eventLoop;
        eventLoop = NULL;
    }
}

int Application::run() {
    if (eventLoop == NULL) {
        // there is no event loop, there is nothing to do
        printf("There is nothing to do in this application. No eventloop configured.\n");
        return -1;
    }

    return eventLoop->run();
}

// global integer
uint32_t LL_C = LS_DEBUG;

void initlog(const char *module_name) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog(module_name, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
}

void set_global_log_severity(uint32_t log_severity_level) {
    LL_C = log_severity_level;
}

