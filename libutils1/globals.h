/* 
 * File:   globals.h
 * Author: pj
 *
 * Created on October 17, 2014, 11:38 PM
 */

#ifndef GLOBALS_H
#define	GLOBALS_H


/*
 Numerical         Severity
             Code

              0       Emergency: system is unusable
              1       Alert: action must be taken immediately
              2       Critical: critical conditions
              3       Error: error conditions
              4       Warning: warning conditions
              5       Notice: normal but significant condition
              6       Informational: informational messages
              7       Debug: debug-level messages

              Table 2. Syslog Message Severities
 * */

// Logging severity levels
#define LS_EMERG 0
#define LS_ALERT 1
#define LS_CRITICAL  2
#define LS_ERROR   3
#define LS_WARNING  4
#define LS_NOTICE  5
#define LS_INFO 6
#define LS_DEBUG 7

// Logging audience, high level filter for log messages
// operator level info
// TAC level info
// maintainer level - 
// original coder level - everything
#define LA_ALL 0
#define LA_PROG 1
#define LA_TEST 2
#define LA_CUSTOMER 3


// Logging filters
// a control flow, 
// a specific tag
// a specific subsystem
// what we need here is a boolean expression to identify every single log message

#define LS_ALL 0
#define LS_FLOW 1  
#define LS_SUBSYS 2
#define LS_TAG 3

#endif	/* GLOBALS_H */

