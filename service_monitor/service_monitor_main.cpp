/* 
 * File:   main.cpp
 * Author: pj
 *
 * Created on October 5, 2014, 10:40 PM
 */

#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <appcommon.h>
#include <logconfig.pb.h>
#include <pbcommon.h>
#include <appcmn1.h>
#include "service_monitor.h"


using namespace std;
using namespace app_pkg;

/*
 * 
 */
/*
 *To compile this file using gcc you can type
 *gcc `xml2-config --cflags --libs` -o xmlexample libxml2-example.c
 */

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
static void
print_element_names(xmlNode * a_node) {
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("node type: Element, name: %s\n", cur_node->name);
        }

        print_element_names(cur_node->children);
    }
}

char *trimwhitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace(*str)) str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

ProcessInfoNode_t *
read_processes(xmlNode *node) {
    xmlNode *cur_node;
    ProcessInfoNode_t *processInfoNodeHead = NULL;
    ProcessInfoNode_t *process_cur_node = NULL;

    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type != XML_ELEMENT_NODE) {
            continue;
        }
        // printf("Checking element %s\n", cur_node->name);
        if (0 != xmlStrcmp(cur_node->name, (const xmlChar *) "process")) {
            continue;
        }
        ProcessInfoNode_t *processInfoNode;
        processInfoNode = (ProcessInfoNode_t *) malloc(sizeof (ProcessInfoNode_t));
        if (NULL == processInfoNode) {
            return processInfoNodeHead;
        }
        memset((void *) processInfoNode, 0, sizeof (ProcessInfoNode_t));

        int rc;

        rc = read_attribute_string(cur_node, "name", processInfoNode->name,
                sizeof (processInfoNode->name));
        if (0 != rc) {
            continue;
        }

        rc = read_attribute_string(cur_node, "instance", processInfoNode->instance,
                sizeof (processInfoNode->instance));
        if (0 != rc) {
            continue;
        }

        rc = read_element_string(cur_node->children, "defaultPort", processInfoNode->defaultPort,
                sizeof (processInfoNode->defaultPort));

        rc = read_element_string(cur_node->children, "binary", processInfoNode->binary,
                sizeof (processInfoNode->binary));

        rc = read_element_string(cur_node->children, "dir", processInfoNode->dir,
                sizeof (processInfoNode->dir));

        rc = read_element_string(cur_node->children, "args", processInfoNode->args,
                sizeof (processInfoNode->args));

        rc = read_element_string(cur_node->children, "override_args",
                processInfoNode->override_args,
                sizeof (processInfoNode->override_args));

        processInfoNode->next = NULL;

        sprintf(processInfoNode->primary_args, "%s %s %s", processInfoNode->name,
                processInfoNode->instance, processInfoNode->defaultPort);
        // save the first node
        if (NULL == processInfoNodeHead) {
            processInfoNodeHead = processInfoNode;
        }
        if (NULL != process_cur_node) {
            process_cur_node->next = processInfoNode;
        }
        process_cur_node = processInfoNode;

    }

    return processInfoNodeHead;
}

int read_attribute_string(xmlNode *node, const char *attributeName, char *str, int n) {
    char attrib_value[128];
    char *attrib_value_ptr;
    xmlChar *valuePtr;
    str[0] = 0;

    valuePtr = xmlGetProp(node, (const xmlChar *) attributeName);
    if (NULL == valuePtr) {
        return -1;
    }
    strncpy(attrib_value, (const char *) valuePtr, sizeof (attrib_value) - 1);
    xmlFree(valuePtr);
    attrib_value_ptr = &attrib_value[0];
    attrib_value_ptr = trimwhitespace(attrib_value_ptr);
    strncpy(str, attrib_value_ptr, n - 1);
    // printf("Attribute: %s  Value %s\n", attributeName, str);
    return 0;
}

int read_element_string(xmlNode *node, const char *elementName, char *str, int n) {
    xmlNode *cur_node;
    str[0] = 0;

    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type != XML_ELEMENT_NODE) {
            continue;
        }
        // printf("Checking element %s\n", cur_node->name);
        if (0 != xmlStrcmp(cur_node->name, (const xmlChar *) elementName)) {
            continue;
        }

        xmlChar *valuePtr = xmlNodeGetContent(cur_node);
        // printf("Content: %s\n", valuePtr);
        char content[256];
        char *contentPtr = &content[0];
        strncpy(contentPtr, (const char *) valuePtr, n - 1);
        contentPtr = trimwhitespace(contentPtr);
        strncpy(str, contentPtr, n - 1);

        xmlFree(valuePtr);
        //  printf("Element: %s  value: %s\n", elementName, str);
        return 0;
    }
    //  printf("ERROR: returning error Element not found\n");
    return -1;
}

PgroupInfoNode_t *
read_pgroups(xmlNode * node) {
    xmlNode *cur_node;

    PgroupInfoNode_t *pgroup_cur_node = NULL;
    PgroupInfoNode_t *pgroup_node_head = NULL;

    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type != XML_ELEMENT_NODE) {
            continue;
        }
        //  printf("Checking element %s\n", cur_node->name);
        if (0 != xmlStrcmp(cur_node->name, (const xmlChar *) "process_group")) {
            continue;
        }

        PgroupInfoNode_t *pgroup_info_node;

        pgroup_info_node = (PgroupInfoNode_t *) malloc
                (sizeof (PgroupInfoNode_t));

        if (NULL == pgroup_info_node) {
            return pgroup_node_head;
        }
        memset((void *) pgroup_info_node, 0, sizeof (PgroupInfoNode_t));
        int rc;
        char group_id_str[32];
        rc = read_attribute_string(cur_node, "group_id",
                group_id_str,
                sizeof (group_id_str));
        if (0 != rc) {
            continue;
        }

        int group_id;
        group_id = atoi(group_id_str);

        if (group_id < 1 || group_id > 16) {
            continue;
        }

        pgroup_info_node->group_id = group_id;
        // found the group_id, now move on to starting all processes

        rc = read_element_string(cur_node->children, "common_args",
                pgroup_info_node->common_args,
                sizeof (pgroup_info_node->common_args));

        pgroup_info_node->processInfoNode = read_processes(cur_node->children);
        pgroup_info_node->next = NULL;

        if (NULL == pgroup_node_head) {
            pgroup_node_head = pgroup_info_node;
        }

        if (NULL != pgroup_cur_node) {
            pgroup_cur_node->next = pgroup_info_node;
        }
        pgroup_cur_node = pgroup_info_node;
    }

    return pgroup_node_head;
}

/**
 * Simple example to parse a file called "file.xml", 
 * walk down the DOM, and print the name of the 
 * xml elements nodes.
 */


PgroupInfoNode_t *
read_config_xml(const char *configFn) {
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    PgroupInfoNode_t *pgroup_info_node;

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(configFn, NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", configFn);
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    // print_element_names(root_element);
    pgroup_info_node = read_pgroups(root_element);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    // all the info has been read, now launch the processes

    return pgroup_info_node;
}

// http://nikhilm.github.io/uvbook/processes.html 

void on_normal_exit(uv_process_t *req, int exit_status, int term_signal) {
    ProcessInfoNode_t *cur_node;
    printf("NORMAL EXIT: Process exited with status %d, signal %d\n", exit_status, term_signal);
    cur_node = (ProcessInfoNode_t *) req->data;
    printf("Program that exited: %s  PID: %d\n", cur_node->binary, cur_node->child_req.pid);
    uv_close((uv_handle_t*) req, NULL);
}

void on_exit(uv_process_t *req, int exit_status, int term_signal) {
    ProcessInfoNode_t *cur_node;
    fprintf(stderr, "Process exited with status %d, signal %d\n", exit_status, term_signal);
    cur_node = (ProcessInfoNode_t *) req->data;
    printf("Program that exited: %s  PID: %d\n", cur_node->binary, cur_node->child_req.pid);
    uv_close((uv_handle_t*) req, NULL);
}

int launch_processes(uv_loop_t *loop, ProcessInfoNode_t *process_node,
        const char *common_args) {

    ProcessInfoNode_t *cur_node;
    for (cur_node = process_node; cur_node; cur_node = cur_node->next) {
        printf("Launching process: %s\n", cur_node->binary);

        char pargs[256];
        strncpy(pargs, cur_node->primary_args, sizeof (pargs) - 1);

        char args[256];
        strncpy(args, cur_node->override_args, sizeof (args) - 1);
        if (strlen(args) == 0) {
            strncpy(args, common_args, sizeof (args) - 1);
        }
        strncat(args, " ", sizeof (args) - 1);
        strncat(args, cur_node->args, sizeof (args) - 1);

        strncat(pargs, " ", sizeof (pargs) - 1);
        strncat(pargs, args, sizeof (pargs) - 1);

        char fullpath[256];
        strncpy(fullpath, cur_node->dir, sizeof (fullpath) - 1);
        int n = strlen(fullpath);
        if (n > 0) {
            if (fullpath[n - 1] != '/') {
                strncat(fullpath, "/", sizeof (fullpath) - 1);
                n++;
            }
        }
        strncat(fullpath, cur_node->binary, sizeof (fullpath) - 1);
        printf("Binary: %s  Args: %s\n", fullpath, pargs);

        // libuv-specific 
        memset((void *) &cur_node->child_req, 0, sizeof (cur_node->child_req));
        memset((void *) &cur_node->options, 0, sizeof (cur_node->options));

        // callback identification data
        cur_node->child_req.data = cur_node;

        int i;
        for (i = 0; i < MAX_CMD_ARGUMENTS; i++) {
            cur_node->all_args[i] = NULL;
        }
        cur_node->all_args[0] = cur_node->binary; // convention
        cur_node->all_args[1] = cur_node->name;
        cur_node->all_args[2] = cur_node->instance;
        cur_node->all_args[3] = cur_node->defaultPort;
        cur_node->all_args[4] = NULL; // must-have
        // the last argument was initialized to NULL already.

        cur_node->options.exit_cb = on_exit;
        cur_node->options.file = fullpath; // local variable
        cur_node->options.args = cur_node->all_args;
        // to make the process a daemon
        //cur_node->options.flags = UV_PROCESS_DETACHED;

        if (uv_spawn(loop, &cur_node->child_req, cur_node->options)) {
            fprintf(stderr, "%s\n", uv_strerror(uv_last_error(loop)));
            return 1;
        }
        fprintf(stderr, "Launched sleep with PID %d\n", cur_node->child_req.pid);
        // This statement is only if the program needs to be launched and
        // then unmonitored. 
        // uv_unref((uv_handle_t*) & child_req);

    }

    return 0;
}

int launch_processes_custom_fork(ProcessInfoNode_t *process_node, const char *common_args) {
    ProcessInfoNode_t *cur_node;
    for (cur_node = process_node; cur_node; cur_node = cur_node->next) {
        printf("Launching process: %s\n", cur_node->binary);

        char pargs[256];
        strncpy(pargs, cur_node->primary_args, sizeof (pargs) - 1);

        char args[256];
        strncpy(args, cur_node->override_args, sizeof (args) - 1);
        if (strlen(args) == 0) {
            strncpy(args, common_args, sizeof (args) - 1);
        }
        strncat(args, " ", sizeof (args) - 1);
        strncat(args, cur_node->args, sizeof (args) - 1);

        strncat(pargs, " ", sizeof (pargs) - 1);
        strncat(pargs, args, sizeof (pargs) - 1);

        char fullpath[256];
        strncpy(fullpath, cur_node->dir, sizeof (fullpath) - 1);
        int n = strlen(fullpath);
        if (n > 0) {
            if (fullpath[n - 1] != '/') {
                strncat(fullpath, "/", sizeof (fullpath) - 1);
                n++;
            }
        }
        strncat(fullpath, cur_node->binary, sizeof (fullpath) - 1);
        printf("Binary: %s  Args: %s\n", fullpath, pargs);

        pid_t pid;
        pid = fork();
        if (pid == 0) {
            int rc;
            rc = execlp(fullpath, fullpath, cur_node->name, cur_node->instance,
                    cur_node->defaultPort, NULL);
            // child process has been launched
        } else {
            // parent process
            printf("Parent: Launched process with PID: %d\n", (int) pid);
            cur_node->pid = pid;
        }

    }

    return 0;
}

int
launch_all_processes(uv_loop_t *loop, PgroupInfoNode_t *pgroup_info_node) {
    PgroupInfoNode_t *pgroup_cur_node;
    for (pgroup_cur_node = pgroup_info_node; pgroup_cur_node;
            pgroup_cur_node = pgroup_cur_node->next) {
        printf("Launching process group: %d\n", pgroup_cur_node->group_id);
        int rc;
        rc = launch_processes(loop, pgroup_cur_node->processInfoNode,
                pgroup_cur_node->common_args);
        if (0 == rc) {
            printf("Successfully launched all processes\n");
        }
    }
    return 0;
}

int
normal_terminate_processes(uv_loop_t *loop, ProcessInfoNode_t *process_node) {
    ProcessInfoNode_t *cur_node;
    for (cur_node = process_node; cur_node; cur_node = cur_node->next) {
        printf("Terminating process: %s %d\n", cur_node->binary,
                cur_node->child_req.pid);
        cur_node->options.exit_cb = on_normal_exit;
        int rc;
        rc = uv_process_kill(&cur_node->child_req, SIGTERM);
        printf("Initiated process termination... rc: %d\n", rc);
    }
    return 0;
}


// TODO global variable for now.
PgroupInfoNode_t *pgroup_info_node;

int
normal_terminate_pgroups(uv_loop_t * loop, PgroupInfoNode_t *pgroup_info_node) {
    PgroupInfoNode_t *pgroup_cur_node;
    for (pgroup_cur_node = pgroup_info_node; pgroup_cur_node;
            pgroup_cur_node = pgroup_cur_node->next) {
        printf("Terminating process group: %d\n", pgroup_cur_node->group_id);
        int rc;
        rc = normal_terminate_processes(loop, pgroup_cur_node->processInfoNode);
        if (0 == rc) {
            printf("Successfully terminated all processes\n");
        }
    }
    return 0;
}

void on_response_end(uv_write_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "error on_write_end");
    } else {
        // printf("Writing completed with status %d\n", status);
        printf("Writing completed. data: %p, Freeing memory %p\n",
                req->data, req);
    }
    free(req);
}


// connection is a client-side connection when TCP is used
// and FD is always -1

int myRxListener(uv_stream_t *connection, ssize_t siz, uv_buf_t buf) {
    return process_msg(connection, siz, buf);
}

int myTimer1Listener(uv_timer_t* handle, int status) {
    printf("Timer1 Occurred: Status: %d\n", status);
    printf("Launching all processes...\n");
    int rc;
    // pass the eventloop as it is necessary to keep the process tied to the
    // eventloop
    rc = launch_all_processes(handle->loop, pgroup_info_node);
    printf("Timer callback completed. Returning %d\n", rc);


#if 0
    // launch the second timer that will terminate all launched processes
    EventLoop *eventLoop = (EventLoop *) handle->loop->data;
    eventLoop->timer1Listener = myTimer1Listener2;
    printf("Launching the process exit timer\n");
    // 10 seconds
    eventLoop->reinitTimer1(10000, 0);
#endif

    return 0;
}

int myTimer1Listener2(uv_timer_t *handle, int status) {
#if 0
    int rc;
    rc = normal_terminate_pgroups(handle->loop, pgroup_info_node);
    printf("Exit timer callback completed %d\n", rc);
#endif
    return 0;
}

int
main(int argc, char **argv) {
    char configFn[200];
    if (argc > 1) {
        strncpy(configFn, argv[1], sizeof (configFn) - 1);
    } else {
        strcpy(configFn, "/home/pj/local/interop/service_monitor.xml");
    }

    pgroup_info_node = read_config_xml(configFn);

    char progname[32] = "service_monitor";
    char app_id[32] = "service_monitor";
    char instance_id[16] = "1";
    uint16_t defaultPort = 7000;

    printf("Starting application %s %s %s %u\n", progname, app_id, instance_id, defaultPort);
    Application *app = new Application(progname, app_id, instance_id);
    EventLoopDefaultListener myListener;
    myListener.listenerMethod = myRxListener;
    app->initServerActivities(defaultPort);
    app->getEventLoop()->defaultListener = myRxListener;
    app->getEventLoop()->timer1Listener = myTimer1Listener;
    register_all_cbs();
    app->run();

    printf("Service Monitor Eventloop returned\n");

    int sl = 1000;
    while (--sl > 0) {
        sleep(1000);
    }
    return 0;
}
