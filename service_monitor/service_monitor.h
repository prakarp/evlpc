/* 
 * File:   service_monitor.h
 * Author: pj
 *
 * Created on October 8, 2014, 10:33 PM
 */

#ifndef SERVICE_MONITOR_H
#define	SERVICE_MONITOR_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <uv.h>

struct ProcessInfoNode_s;
typedef struct ProcessInfoNode_s ProcessInfoNode_t;

#define MAX_CMD_ARGUMENTS 32
struct ProcessInfoNode_s {
    char name[32];
    char instance[8];
    char defaultPort[8];
    char binary[64];
    char dir[128];
    char args[128];
    char override_args[128];
    char primary_args[128]; // these would be the first few automatic args
    ProcessInfoNode_t *next;
    pid_t pid;
    
    char *all_args[MAX_CMD_ARGUMENTS];   // maximum of 32  arguments
        // plus, args[0] should be prog name,  and last arg must be NULL
        // so the remainder of 30 is available for commandline arguments]
    // libuv-specific 
    uv_process_t child_req;
    uv_process_options_t options;
};

struct PgroupInfoNode_s;
typedef struct PgroupInfoNode_s PgroupInfoNode_t;

struct PgroupInfoNode_s {
    int group_id;
    char common_args[256];
    ProcessInfoNode_t *processInfoNode;
    PgroupInfoNode_t *next;
};


int read_element_string(xmlNode *node, const char *elementName, char *str, int n);
int read_attribute_string(xmlNode *node, const char *attributeName, char *str, int n);
int myTimer1Listener2(uv_timer_t *handle, int status);
int myTimer1Listener(uv_timer_t* handle, int status);


#endif	/* SERVICE_MONITOR_H */

