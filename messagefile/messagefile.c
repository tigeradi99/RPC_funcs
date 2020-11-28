#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "messagefile.pb-c.h"
#include <stdbool.h>
#include <time.h>

int main(int argc, const char * argv[])
{
    XRPCMessage *out_msg, msg = X_RPCMESSAGE__INIT;

    void *buf;
    unsigned int len;

	msg.has_message_type = 1;
	msg.message_type = X_RPCMESSAGE__MESSAGE_TYPE__REQUEST;

	TimeVal time_stamp;
	msg.time_stamp = &time_stamp;
	time_val__init(msg.time_stamp);
	msg.time_stamp->has_tv_sec = 1;
	msg.time_stamp->tv_sec = time(NULL);

	// select which procedure
	msg.request_type_case = X_RPCMESSAGE__REQUEST_TYPE_SETTIMEOFDAY_REQUEST;

	// define and init variable to the nested structure
    SettimeofdayRequest settimeofday_request;
	msg.settimeofday_request = &settimeofday_request;
	settimeofday_request__init(msg.settimeofday_request);
	
	// second level of nesting, define, init, assign
	TimeVal timeval;
	msg.settimeofday_request->timeval = &timeval;
	time_val__init(msg.settimeofday_request->timeval);
	msg.settimeofday_request->timeval->has_tv_sec = 1;;
	msg.settimeofday_request->timeval->tv_sec = time(NULL);
	msg.settimeofday_request->timeval->has_tv_usec = 1;;
	msg.settimeofday_request->timeval->tv_usec = 888;

    // find the length of the packed structure
    len = x_rpcmessage__get_packed_size(&msg);
 
    //lets arrange for as much size as len
    buf = malloc(len);

    //lets get the serialized structure in buf
    x_rpcmessage__pack(&msg, buf);


	// for testing, use gdb to inspect the values and pointers
	out_msg = x_rpcmessage__unpack(NULL, len, buf);
	
	printf("message_type: %d\n", out_msg->message_type);	
	printf("time_stamp: %d\n", out_msg->time_stamp->tv_sec);	

	// switch is an example of unpack
	// use a array of pointer to funcions later
	switch(out_msg->request_type_case){
		case X_RPCMESSAGE__REQUEST_TYPE_SETTIMEOFDAY_REQUEST:
			printf("X_RPCMESSAGE__REQUEST_TYPE_SETTIMEOFDAY_REQUEST\n");
			printf("tv_sec: %d\n", out_msg->settimeofday_request->timeval->tv_sec);
			break;
		
		// TODO other cases

		case X_RPCMESSAGE__REQUEST_TYPE__NOT_SET:
			// handle error
			printf("X_RPCMESSAGE__REQUEST_TYPE__NOT_SET\n");
			break;

		default:
			// handle error
			break;
	}
    //free buffer
    free(buf);
     
     return 0;
}
