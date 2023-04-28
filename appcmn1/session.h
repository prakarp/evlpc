/* 
 * File:   session.h
 * Author: pj
 *
 * Created on October 20, 2014, 6:41 AM
 */

#ifndef SESSION_H
#define	SESSION_H

#include <cstdlib>
#include <stdint.h>
#include <list>
#include <vector>
#include <pbcommon.h>

using namespace std;

typedef list<uint32_t> sid_list_t;

#define MAX_SESSIONS 1000
#define MAX_CONNECTIONS 64

class SessionMapping_t {
    uint32_t ib_sid; // inbound SID
    sid_list_t ob_sid_list; // outbound SID
};
typedef list<SessionMapping_t> session_mapping_t;

typedef int (*on_reply_cb_t)(void *data, uv_stream_t *connection, PbMsg *pbmsg);

typedef int (*ib_callback_t)(uv_stream_t *connection, void *data);

class IbSessionInfo_t {
public:
    uint32_t ib_sid; // inbound SID
    uv_stream_t *connection;
    ib_callback_t cb_func;
    void *partial_results;
    void *data;
};


typedef vector<IbSessionInfo_t> ib_sessions_t;

typedef int (*ob_callback_t)(uv_stream_t *connection, void *data);

class ObSessionInfo_t {
public:
    uint32_t ob_sid;
    bool inbound_session_present; // true only if this is a session for an inbound
    uint32_t ib_sid; // related inbound sid, for this outbound sid
    uint32_t start_time;
    uint32_t magic;
    void *data;
    on_reply_cb_t  cb;
};

typedef vector<ObSessionInfo_t> ob_sessions_t;

uint32_t make_new_session(void *data, uint32_t magic, on_reply_cb_t cb);
int find_stored_session(uint32_t sid, uint32_t magic, ObSessionInfo_t *session);
void return_sid(uint32_t sid);

#endif	/* SESSION_H */

