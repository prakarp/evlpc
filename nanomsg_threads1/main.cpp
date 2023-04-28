#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "nanomsg/nn.h"
#include "nanomsg/reqrep.h"

#define NO_THREADS 8
pthread_t tid[NO_THREADS];

void* oauth2_device(void *arg);
void* send_authentication_request(void *arg);

int main(int argc, const char * argv[])
{
    int no_threads = 1;
    
    assert(pthread_create(&(tid[0]), NULL, &oauth2_device, NULL) == 0);
    while(no_threads < NO_THREADS)
    {
        assert(pthread_create(&(tid[no_threads]), NULL, &send_authentication_request, NULL) == 0);
        no_threads++;
    }
    
    while(1)
    {
        sleep(1000);  
    }
    
    return 0;
}

void* oauth2_device(void *arg)
{
    
    int rc;
    
    int device_rep;
    
    int device_req;
    
    pthread_t id = pthread_self();
    
    
    /*  Intialise the device sockets. */
    
    device_rep = nn_socket (AF_SP_RAW, NN_REP);
    
    nn_bind (device_rep, "ipc://oauth2");
    
    device_req = nn_socket (AF_SP_RAW, NN_REQ);
    
    nn_bind (device_req, "ipc://oauth2handler");

    // Even I tried with 
	//	nn_connect(device_req, "ipc://oauth2handler"); with corresponding 
	//	int sock = nn_socket(AF_SP , NN_REP); nn_bind(sock, "ipc://oauth2handler");  
    
    printf("\n oauth2_device thread id: %lu\n", (unsigned long) id);
    
    
    /*  Run the device. */
    
    rc = nn_device (device_rep, device_req);
    
    /* this means nn_device loop exited either by an error or by calling nn_term () */
    
    assert (rc < 0 && nn_errno () == ETERM);
    
    
    /*  Clean up. */
    
    nn_close (device_rep);
    
    nn_close (device_req);
    
    return NULL;
    
}

void* send_authentication_request(void *arg)
{
    pthread_t id = pthread_self();
    
    int sock = nn_socket(AF_SP , NN_REP);
    
    assert(sock >= 0);
    
    //	Even I tried, with nn_bind(sock, "ipc://oauth2handler") with corresponding
	//	nn_connect(device_req, "ipc://oauth2handler"); of nn_device.
    assert( nn_connect(sock, "ipc://oauth2handler") >= 0 );
    
    printf("\n send_authentication_request thread id: %lu\n", (unsigned long) id);
    
    while(1)
    {
        char *buf = NULL;
        int bytes = nn_recv (sock, &buf, NN_MSG, 0);
        
	/*****  CONTROL IS JUST WAITING IN nn_recv */
        printf("\n thread id: %lu  bytes received: %d\n", (unsigned long) id, bytes);
        
        if(bytes > 0)
        {
            //struct string s;
            //init_string(&s);
            
            printf("recieved url: %s",buf);
            
            //  parse received data, data should be like sid=n&param1=value&param=value2...etc
            //parse_and_authorize(buf, &s);
            
            //free_string(&s);
            nn_freemsg (buf);
        }
    }
    
    return NULL;

}