/* 
 * File:   main.cpp
 * Author: pj
 *
 * Created on October 5, 2014, 7:29 PM
 */

#include <cstdlib>
#include <iostream>
#include <list>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ini.h>
#include <iterator>
#include <logconfig.pb.h>
#include <appcommon.h>
#include <appcmn1.h>
#include <session.h>

//#include <google/protobuf/io/coded_stream.h>

using namespace std;
using namespace app_pkg;
//using namespace google::protobuf::io;

// configuration data

typedef struct {
    char app_name[8];
    char host[32];
    int port;
} app_server_port_t;

typedef list<app_server_port_t> port_list_t;

// to store run-time data

typedef struct {
    app_server_port_t port_config;
    // TODO use uv_ref and uv_unref ? 
    uv_connect_t *app_server_connection;
} app_server_connection_t;

typedef list<app_server_connection_t *> connections_list_t;
connections_list_t gconnections;

// all of the configuration

typedef struct {
    char app_name[32];
    char default_host[32];
    char app_id[8];
    char my_default_port[8];
    port_list_t app_server_ports;
} configuration;

// global variable
configuration gconfig;

#define MAX_LEN_SECTION (32)
#define MAX_LEN_ATTRIB_NAME (32)
#define MAX_LEN_ATTRIB_VALUE (128)

static int handler(void* user, const char* section, const char* name,
        const char* value) {
    configuration* pconfig = (configuration*) user;
    const char *app_name = pconfig->app_name;

#define MATCH_SECTION(s) (strncmp(section, s, MAX_LEN_SECTION) == 0)
#define MATCH_NAME(n)    (strncmp(name, n, MAX_LEN_ATTRIB_NAME) == 0)
#define MATCH(s, n)  (MATCH_SECTION(s) && MATCH_NAME(n))

    if (MATCH("default", "host")) {
        strncpy(pconfig->default_host, value, sizeof (pconfig->default_host) - 1);
    } else if (MATCH("app_id", app_name)) {
        strncpy(pconfig->app_id, value, sizeof (pconfig->app_id) - 1);
    } else if (MATCH_SECTION("default_port")) {
        // list of all the server connections to be made
        int port = atoi(value);
        if (port > 0 && port <= 65535) {
            app_server_port_t app_server_port;
            strncpy(app_server_port.app_name, name, sizeof (app_server_port) - 1);
            app_server_port.port = port;
            strncpy(app_server_port.host, pconfig->default_host, sizeof (app_server_port.host) - 1);
            pconfig->app_server_ports.push_back(app_server_port);
            printf("Adding port to the list %s %d\n", app_server_port.app_name,
                    app_server_port.port);
            if (MATCH_NAME(app_name)) {
                strncpy(pconfig->my_default_port, value, sizeof (pconfig->my_default_port) - 1);
            }
        }
    } else {
        return 0; /* unknown section/name, error */
    }
    return 1;
}

int read_ini_config(const char *inifn, const char *app_name) {
    strncpy(gconfig.app_name, app_name, sizeof (gconfig.app_name) - 1);
    if (ini_parse(inifn, handler, &gconfig) < 0) {
        printf("Can't load %s\n", inifn);
        return 1;
    }
    printf("Config loaded from %s: default_host=%s, app_id=%s, default_port=%s\n",
            inifn, gconfig.default_host, gconfig.app_id, gconfig.my_default_port);
    return 0;
}

int myRxListener(uv_stream_t *connection, ssize_t siz, uv_buf_t buf) {
    printf("Received %ld bytes\n", siz);
    return process_msg(connection, siz, buf);
}

void start_all_client_connections(uv_loop_t *loop);
int myTimer1Listener2(uv_timer_t *handle, int status);

int myTimer1Listener(uv_timer_t* handle, int status) {
    printf("Timer1 Occurred: Status: %d\n", status);
    start_all_client_connections(handle->loop);

    // launch the second timer that simply send a message to the connected servers
    // once every 10 seconds
    EventLoop *eventLoop = (EventLoop *) handle->loop->data;
    eventLoop->timer1Listener = myTimer1Listener2;
    printf("Launching the periodic ping timer\n");
    // 10 seconds
    eventLoop->reinitTimer1(3000, 0);
}

void on_write_end(uv_write_t *req, int status) {
    app_server_connection_t *c = (app_server_connection_t *) req->data;
    if (status == -1) {
        fprintf(stderr, "error on_write_end");
    } else {
        // printf("Writing completed with status %d\n", status);
        printf("Writing completed to port %d\n Freeing memory %p\n",
                c->port_config.port, req);
    }
    free(req);
}

// as a client, send a number of log configuration messages

int sample_global_log_config() {
    A1_GlobalLogConfig global_log_config;
    global_log_config.set_log_level(LOG_DEBUG);
    char *msg_buf = global_log_config.make_msg();
    uint32_t msg_buf_size = global_log_config.get_msg_size();
    uv_buf_t buf = uv_buf_init(msg_buf, msg_buf_size);

    printf("Message total size: %u\n", msg_buf_size);
    // for every client connection, send this message
    connections_list_t::iterator list_iter;
    for (list_iter = gconnections.begin();
            list_iter != gconnections.end();
            list_iter++) {
        // TODO we have serious issues if we take pointers, 
        // storing a pointer in the callback, but this pointer is in a list
        app_server_connection_t *c;

        c = *list_iter;
        // c = &connection;
        LLG(LOG_DEBUG, "Sending data to port %d\n", c->port_config.port);
        printf("Sending data to port %d\n", c->port_config.port);
        printf("Send data Connection %p   Req: %p  Port: %d\n",
                c, c->app_server_connection, c->port_config.port);
        uv_stream_t *tcp = c->app_server_connection->handle;
        uv_write_t *write_req = (uv_write_t *) malloc(sizeof (uv_write_t));
        printf("Sending data using request %p\n", write_req);
        write_req->data = c; // store the connection as callback data
        int buf_count = 1;
        uv_write(write_req, tcp, &buf, buf_count, on_write_end);
    }
    return 0;
}

app_server_connection_t *get_service_monitor_connection() {
    app_server_connection_t *c;
    connections_list_t::iterator list_iter;
    for (list_iter = gconnections.begin();
            list_iter != gconnections.end();
            list_iter++) {
        // TODO we have serious issues if we take pointers, 
        // storing a pointer in the callback, but this pointer is in a list

        c = *list_iter;
        printf("Checking element port %d\n", c->port_config.port);
        //  if (strcmp(c->port_config.app_name, "service_monitor") == 0) 
        if (c->port_config.port == 8124) {
            printf("Found the service monitor port %d\n", c->port_config.port);
            break;
        }
    }
    return c;
};

int on_reply_to_idling_request(void *data, uv_stream_t *connection, PbMsg *pbmsg) {
    app_server_connection_t *c = (app_server_connection_t *)data;
    // now we have the data, and the connection where the data came from
    printf("Idling request has been completed..\n");
    return 0;
}

int delayed_response_test() {
    A1_DelayedResponse req;

    app_server_connection_t *c;
    c = get_service_monitor_connection();
    if (c == NULL) {
        printf("Connection has been closed. Not testing.\n");
        return -1;
    }

    printf("\n\nConnection found %p connect_t %p data: %p handle: %p\n",
            c, c->app_server_connection, c->app_server_connection->data,
            c->app_server_connection->handle);
    if (uv_is_active((uv_handle_t *)(c->app_server_connection->handle))) {
        printf("Connection is active... proceeding..\n");
    } else {
        printf("Connection is not active. It has been closed. Not proceeding with the test\n");
        printf("OK to reconnect here at this point...\n");
        return -1;
    }
    for (int i = 0; i < 3; i++) {
        printf("\nSending delayed response test request %d\n", i);
        uint32_t r = rand();
        uint32_t sid = make_new_session((void *) c, r, on_reply_to_idling_request);
        req.pbmsg.hdr.session_id = sid;
        req.pbmsg.hdr.random_id = r;

        req.set_delay((rand() % 5) + 1);

        char *msg_buf = req.make_msg();
        uint32_t msg_buf_size = req.get_msg_size();
        uv_buf_t buf = uv_buf_init(msg_buf, msg_buf_size);

        printf("---- Delayed Response Req sid: %u, random: %u total size: %u\n",
                sid, r, msg_buf_size);

        LLG(LOG_DEBUG, "Sending delayed req data to port %d\n", c->port_config.port);
        // printf("Sending delay data to port %d\n", c->port_config.port);
        // printf("Send delay data Connection %p   Req: %p  Port: %d\n",
        //       c, c->app_server_connection, c->port_config.port);
        uv_stream_t *tcp = c->app_server_connection->handle;
        uv_write_t *write_req = (uv_write_t *) malloc(sizeof (uv_write_t));
        // printf("Sending delay data using request %p\n", write_req);
        write_req->data = c; // store the connection as callback data
        int buf_count = 1;

        // TODO is the memory to be allocated or is it overwritten?
        int rc = uv_write(write_req, tcp, &buf, buf_count, on_write_end);
        printf("Write request result: %d\n", rc);
        if (rc < 0) {
            // error in writing, this connection is not valid, close it?
            write_req->data = NULL;
            free(write_req);
            printf("\n\nALREADY CLOSED connection %p %p data: %p handle: %p\n",
                    c, c->app_server_connection, c->app_server_connection->data,
                    c->app_server_connection->handle);
            // uv_close((uv_handle_t*)(c->app_server_connection->handle), NULL);
        }
    }
    return 0;
}

// as a client, send a number of log configuration messages

int periodic_publish() {
    A1_PublishedItems items(MT_PUBLISH_REQ);
    PublishedItem *item;

    // add an item to the list of items
    item = items.add_items();
    item->set_item_id(OT_VLAN);
    item->set_item_instance_start(0);
    item->set_item_instance_end(0);

    item = items.add_items();
    item->set_item_id(OT_SYSTEM);
    item->set_item_instance_start(0);
    item->set_item_instance_end(0);

    char *msg_buf = items.make_msg();
    uint32_t msg_buf_size = items.get_msg_size();
    uv_buf_t buf = uv_buf_init(msg_buf, msg_buf_size);

    printf("Message total size: %u\n", msg_buf_size);
    // for every client connection, send this message
    connections_list_t::iterator list_iter;
    for (list_iter = gconnections.begin();
            list_iter != gconnections.end();
            list_iter++) {
        // TODO we have serious issues if we take pointers, 
        // storing a pointer in the callback, but this pointer is in a list
        app_server_connection_t *c;

        c = *list_iter;
        // c = &connection;
        LLG(LOG_DEBUG, "Sending publish data to port %d\n", c->port_config.port);
        printf("Sending publish data to port %d\n", c->port_config.port);
        printf("Send publish data Connection %p   Req: %p  Port: %d\n",
                c, c->app_server_connection, c->port_config.port);
        uv_stream_t *tcp = c->app_server_connection->handle;
        uv_write_t *write_req = (uv_write_t *) malloc(sizeof (uv_write_t));
        printf("Sending publish data using request %p\n", write_req);
        write_req->data = c; // store the connection as callback data
        int buf_count = 1;
        uv_write(write_req, tcp, &buf, buf_count, on_write_end);
    }
    return 0;
}

int myTimer1Listener2(uv_timer_t *handle, int status) {
    printf("Periodic Timer - Listener 2\n");
    LLG(LOG_DEBUG, "Periodic Timer, Listener 2\n");

    // publish a number of objects periodically
    // periodic_publish();
    delayed_response_test();
    // launch the second timer that simply send a message to the connected servers
    // once every 10 seconds
    EventLoop *eventLoop = (EventLoop *) handle->loop->data;
    // eventLoop->timer1Listener = myTimer1Listener2;
    printf("Launching the periodic ping timer again\n");
    // 10 seconds
    eventLoop->reinitTimer1(30000, 0);
    return 0;
}

/*
 * 
 */
int main(int argc, char** argv) {
    int defaultPort = 7000;
    char progname[32];
    char app_id[32];
    char instance_id[8];

    printf("ARGC %d\n", argc);

    strncpy(progname, argv[0], sizeof (progname) - 1);
    if (argc > 1) {
        strncpy(app_id, argv[1], sizeof (app_id) - 1);
    } else {
        strncpy(app_id, "app", sizeof (app_id) - 1);
    }
    if (argc > 2) {
        strncpy(instance_id, argv[2], sizeof (instance_id) - 1);
    } else {
        strncpy(instance_id, "1", sizeof (instance_id) - 1);
    }

    if (argc > 3) {
        defaultPort = atoi(argv[3]);
        if (defaultPort < 1024 || defaultPort > 65535) {
            printf("Invalid Port number supplied: %d (allowed: 1024-65535)\n", defaultPort);
            exit(-2);
        }
    }

    char ini_file_path[128];
    strncpy(ini_file_path, APP_RESOURCES_DIR, sizeof (ini_file_path) - 1);
    strncat(ini_file_path, "/", sizeof (ini_file_path) - 1);
    strncat(ini_file_path, APP_INI_FILE, sizeof (ini_file_path) - 1);

    read_ini_config(ini_file_path, "cfgagent");
    defaultPort = atoi(gconfig.my_default_port);
    printf("Starting application %s %s %s %d\n", progname, app_id, instance_id,
            defaultPort);
    Application *app = new Application(progname, app_id, instance_id);
    EventLoopDefaultListener myListener;
    myListener.listenerMethod = myRxListener;
    app->initServerActivities(defaultPort);
    app->getEventLoop()->defaultListener = myRxListener;
    app->getEventLoop()->timer1Listener = myTimer1Listener;
    register_all_cbs();

    app->run();
    return 0;
}

void on_client_connect(uv_connect_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "error on_write_end..");
        return;
    }
    printf("TCP connection completed\n");
    app_server_port_t *app_server_port;
    app_server_port = (app_server_port_t *) req->data;
    printf("TCP connection completed for for %s:%d\n", app_server_port->host,
            app_server_port->port);

    // TODO dangerous to store the pointer information from a list
    // as a callback pointer, fix it by pointing to a stationary item
    // TODO NEXT - the data is not stored, req structure is not stored anywhere
    app_server_connection_t *connection;
    connection = (app_server_connection_t *) malloc(sizeof (app_server_connection_t));
    assert(connection != NULL);
    connection->port_config = *app_server_port;
    connection->app_server_connection = req;

    // store the connection into the global connections list
    gconnections.push_back(connection);
    printf("Add Connection %p   Req: %p  Port: %d\n",
            connection, req, connection->port_config.port);

    // DO NOT STORE THE REQ UNLESS connection is to be attempted again.
    // req->handle->connect_req = req;
    readClientConnections(req->handle);

#if 0
    port_list_t::iterator list_iter;
    for (list_iter = gconfig.app_server_ports.begin();
            list_iter != gconfig.app_server_ports.end();
            list_iter++) {
        app_server_port_t this_port;
        // app_server_port *this_port = &this_port_s;
        this_port = *list_iter;
        if ((this_port.port == app_server_port->port) &&
                strncmp(this_port.host, app_server_port->host,
                sizeof (app_server_port->host) == 0)) {
            // this is the port
            // store the connection data
            printf("TCP connection stored for for %s:%d\n", app_server_port->host,
                    app_server_port->port);
            app_server_connection_t connection;
            connection.port_config = this_port;
            connection.app_server_connection = req;

            // store the connection into the global connections list
            gconnections.push_back(connection);
            break;
        }
    }
#endif

    printf("Connection storage completed\n");
    return;
}

void start_all_client_connections(uv_loop_t *loop) {
    port_list_t::iterator list_iter;
    for (list_iter = gconfig.app_server_ports.begin();
            list_iter != gconfig.app_server_ports.end();
            list_iter++) {
        // std::cout << *list_iter << endl;
        // for every port, create a connection request
        app_server_port_t *app_server_port = (app_server_port_t *) malloc(sizeof (app_server_port_t));
        assert(app_server_port != NULL);
        *app_server_port = *list_iter;
#if 0
        // it is ok to connect to a server on the same program
        if (app_server_port->port == atoi(gconfig.my_default_port)) {
            free(app_server_port);
            continue;
        }
#endif
        printf("Starting TCP connection request for %s:%d\n", app_server_port->host,
                app_server_port->port);
        uv_tcp_t *client = (uv_tcp_t *) malloc(sizeof (uv_tcp_t));
        assert(client != NULL);
        uv_tcp_init(loop, client);
        struct sockaddr_in req_addr = uv_ip4_addr(app_server_port->host,
                app_server_port->port);

        // TODO see if this is supposed to be in heap and if memory leak occurs.
        // examples do not use heap
        uv_connect_t *connect_req = (uv_connect_t *) malloc(sizeof (uv_connect_t));
        assert(connect_req != NULL);
        connect_req->data = (void *) app_server_port;
        // DO NOT DO THIS HERE, it causes connections to fail, eventhough its value
        // is NULL after connection is completed.
       // client->connect_req = connect_req;
        uv_tcp_connect(connect_req, client, req_addr, on_client_connect);
    }
    // all client connections have been started up
}
