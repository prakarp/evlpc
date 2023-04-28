/* 
 * File:   loglib.h
 * Author: prakash
 *
 * Created on October 15, 2010, 4:05 PM
 */

#ifndef LOGLIB_H
#define	LOGLIB_H

#ifdef	__cplusplus
extern "C" {
#endif

#define LOG_SOCK_NAME "/tmp/.p_n_c"
#define LOG_MODE_SERVER 1
#define LOG_MODE_CLIENT 2

/* socket_mode : datagram or connected */
#define LOG_SOCKET_DGRAM 1
#define LOG_SOCKET_CONNECTED 2

int  log_open(int log_mode, int max_log_connections, int socket_mode);
int logtx(int fd, char *buf, int buflen);
int logrx(int fd, char *buf, int buflen);
void log_close(int s);


#ifdef	__cplusplus
}
#endif

#endif	/* LOGLIB_H */

