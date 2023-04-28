#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <appcommon.h>
#include <cstdlib>
#include <list>
#include <vector>
#include "session.h"

using namespace std;

session_mapping_t session_map;

ib_sessions_t ib_sessions;
ob_sessions_t ob_sessions;

// in order to respond, what is needed is a 
sid_list_t used_list, free_list;
uint32_t current_max_sid = 0;

void create_free_sids() {
    uint32_t new_sids = 10;
    uint32_t new_max_sid = current_max_sid + 10;
    for (uint32_t next_sid = current_max_sid;
            next_sid < new_max_sid;
            next_sid++) {
        free_list.push_back(next_sid);
        printf("Added SID to the list %u, size: %lu\n", next_sid, free_list.size());
    }
    current_max_sid = new_max_sid;
    printf("Added SID to the list max: %u, size: %lu", current_max_sid, free_list.size());
}

uint64_t current_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return ( (uint64_t) t.tv_sec + (uint64_t) t.tv_usec);
}

uint32_t current_time_sec() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return ((uint32_t) t.tv_sec);
}

static int inited = 0;

uint32_t find_free_sid() {

    long unsigned int n;
    n = free_list.size();
    printf("Free List size: %lu\n", n);
    if (n <= 0) {
        // no free entries, add more
        printf("Adding more session IDs to the free list, current %u",
                current_max_sid);
        create_free_sids();
    }

    uint32_t sid = free_list.front();
    free_list.pop_front();
    used_list.push_back(sid);
    printf("Added %u to used-list (%lu used, %lu free)\n", sid,
            used_list.size(), free_list.size());
    return sid;
}

void return_sid(uint32_t sid) {
    used_list.remove(sid);
    free_list.push_back(sid);
    printf("Added %u to used-list (%lu used, %lu free)\n", sid,
            used_list.size(), free_list.size());
}

uint32_t make_new_session(void *data, uint32_t magic, on_reply_cb_t cb) {
    if (inited == 0) {
        free_list.clear();
        used_list.clear();
        ob_sessions.clear();
        ib_sessions.clear();
        ob_sessions.resize(1000);
        ib_sessions.resize(1000);
        inited = 1;
    }

    uint32_t sid = find_free_sid();
    printf("New OB session ID: %u\n", sid);
    printf("To store session in the vector. #elements = %lu\n", ob_sessions.size());
    ObSessionInfo_t session;
    session.ob_sid = sid;
    session.data = data;
    session.magic = magic;
    session.inbound_session_present = false;
    session.ib_sid = 0;
    session.start_time = current_time_sec();
    session.cb = cb;
    ob_sessions[sid] = session;
    printf("Stored session in the vector. #elements = %lu\n", ob_sessions.size());
    return sid;
}

int find_stored_session(uint32_t sid, uint32_t magic, ObSessionInfo_t *session) {
    uint32_t stored_magic;
    stored_magic = ob_sessions[sid].magic;
    if (stored_magic != magic) {
        return -2;
    }
    *session = ob_sessions.at(sid);
   // DON'T ERASE ob_sessions.erase(ob_sessions.begin() + sid);
    ob_sessions[sid].magic = 0;
    printf("Invalidated session in the vector. CHECK MAGIC. #elements = %lu\n", ob_sessions.size());
    return 0;
}
