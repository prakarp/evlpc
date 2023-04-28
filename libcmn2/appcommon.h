/* 
 * File:   appcommon.h
 * Author: pj
 *
 * Created on October 4, 2014, 5:09 PM
 */
#ifndef APPCOMMON_H
#define	APPCOMMON_H

#include <uv.h>
#include <syslog.h>

#define ASSERT assert

#define APP_RESOURCES_DIR  "/home/pj/local/interop"
#define APP_INI_FILE  "app.ini"

typedef int (*ListenerMethod)(uv_stream_t *, ssize_t, uv_buf_t);
typedef int (*Timer1ListenerMethod)(uv_timer_t* handle, int status);

class EventLoopDefaultListener {
public:

    EventLoopDefaultListener() {
        initialize();
    };

    ~EventLoopDefaultListener() {
        initialize();
    };

    // callback is a pointer to a function, this is not a function declaration
    // int (*listenerMethod)(uv_stream_t *server, ssize_t nread, uv_buf_t buf);
    ListenerMethod listenerMethod;

    // setters

    void setUserTokens(int64_t token, void *ptrToken) {
        privateToken = token;
        privatePtrToken = ptrToken;
    };

    // getters

    int64_t getToken() {
        return privateToken;
    };

    void *getPtrToken() {
        return privatePtrToken;
    };

    void initialize() {
        listenerMethod = NULL;
        setUserTokens(0, NULL);
    };

private:
    int64_t privateToken;
    void *privatePtrToken;
};

#if 0

class EventLoop {
public:
    static const int MAX_FD_IN_LOOP = 64;
    static const int MAX_EVENTS = 32;
    EventLoop();
    ~EventLoop();

    int addFd(int fd, int userData, EventListener *listener);
    int deleteFd(int fd);
    void run();

private:
    int initLoopFd();
    int deInitLoopFd();
    void initLocalData();
    void deInitLocalData();
    int addToEventLoop(int fd, int i);
    int deleteFromEventLoop(int fd);
    int deleteFromEventLoop2(int fd, int slot);
    void deleteAllFds();
    void freeSlot(int slot);
    void useSlot(int slot, int fd, int userData, EventListener *listener);

    int epfd;
    int fds[MAX_FD_IN_LOOP];
    int userData[MAX_FD_IN_LOOP]; // nanomsg socket
    EventListener *listeners[MAX_FD_IN_LOOP];
};

#endif

extern const int MAX_ALLOCATION_SIZE;

class EventLoop {
    uv_tcp_t defaultServer;
    uv_loop_t *loop;
    uv_timer_t onceTimer1;

public:
    EventLoop(uint16_t defaultPort = 7000);
    ~EventLoop();
    int run();
    int initTimer1(int delay = 1000, int period = 0);
    int reinitTimer1(int delay, int period);
    int addDefaultServerPort(uint16_t defaultPort = 7000);

    uv_loop_t *getEventLoop() {
        return loop;
    }
    ListenerMethod defaultListener;
    Timer1ListenerMethod timer1Listener;
};

class Application {
    char progName[32];
    char appName[64];
    int appInstance;
    EventLoop *eventLoop;

public:
    Application();
    Application(const int argc, const char *argv[]);
    Application(const char *progname, const char *app_id, const char *instance_id);
    ~Application();
    int run();
    int initServerActivities(uint16_t defaultPort = 7000);
    void initialize(const int argc, const char *argv[]);

    EventLoop *getEventLoop() {
        return eventLoop;
    }
};

#if 0

class ServerApplication : public Application {
    EventLoop *eventLoop;
public:
    ServerApplication();
    ServerApplication(const int argc, const char *argv[]);
    ~ServerApplication();
    int initServerActivities();

    EventLoop *getEventLoop() {
        return eventLoop;
    }
};
#endif

typedef struct {
    char *rx_buf_addr; // receive buffer address
    uint32_t rx_buf_size; // rx buffer size
    uint32_t leftover_data_size;
    // how much data is still unprocessed at the end of the buffer.
    // if leftover_data_size is zero, allocated memory may be returned from
    // the start, else the leftover_data has to be copied over to the beginning
    // after the callbacks are completed.

} RxBufAddr_t;


void readClientConnections(uv_stream_t *client);



// current log level
extern uint32_t LL_C;

#define errno_str() (errno == 0 ? "None" : strerror(errno))

// macros for logging
#define LLG(LL,fmt,...) if (LL <= LL_C) syslog(LL, "(%s:%d) " fmt "", __FILE__, __LINE__, ##__VA_ARGS__)

// void logit(int severity, char *format, const char *file, int line, ...);
void initlog(const char *module_name);
void set_global_log_severity(uint32_t log_severity_level);


// global constants

// all protocols

#define PROTOBUF_V1  0x00000101

// all message types

enum MsgTypes {
    MT_NONE = 0,
    MT_STD_REPLY,
    MT_DELAYED_RESPONSE_REQ,
    MT_GLOBAL_LOG_CONFIG_REQ,
    MT_GLOBAL_LOG_CONFIG_REPLY,
    MT_PUBLISH_REQ,
    MT_PUBLISH_REPLY,
    MT_MAX
};

// all objects (items) published by a program
#define OT_NONE 0
#define OT_GLOBAL_LOG_CONFIG 1
#define OT_PUBLISHED_ITEM 2
#define OT_SYSTEM 4
#define OT_PORT 5
#define OT_BRIDGE 6
#define OT_VLAN 7
#define OT_MSTP 8
#define OT_INTERFACE 9
#define OT_PORT_GROUP 10
#define OT_ACL 11

// PROGRAMMER ALERT keep the following number same as above
#define OT_MAX 11

#endif	/* APPCOMMON_H */

