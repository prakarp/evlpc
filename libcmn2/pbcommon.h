/* 
 * File:   pbcommon.h
 * Author: pj
 *
 * Created on October 17, 2014, 11:08 PM
 */

#ifndef PBCOMMON_H
#define	PBCOMMON_H

#include <stdint.h>

#define MAX_PB_MSG_SIZE 2500

typedef struct {
    uint32_t length;
    uint32_t protocol;
    uint32_t payload_type; // TODO to decide how to decode the data type
    uint32_t session_id;   // from a total number of outstanding session id pool
    uint32_t random_id;    // so that 
    uint64_t timestamp;    // timestamp to delete messages after 10 seconds
} PbHeader;

typedef struct {
    PbHeader hdr;    // message length (everything in this structure)
    char message[MAX_PB_MSG_SIZE]; // maximum size of PB message defined above
} PbMsg;

#endif	/* PBCOMMON_H */

