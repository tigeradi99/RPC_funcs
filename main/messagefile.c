#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "messagefile.pb-c.h"
#include <stdbool.h>
#include <time.h>

#define MAX_BUF 1640 // MTU of Ethernet/Internet

int func0 (void *v) {
	printf("func0\n");
	return 0;
}

int settimeofday_func(void *val) {
	printf("func0\n");
    Request *request = (Request *) (val);
	TimeVal *timeval = request->settimeofday_request->timeval;
	printf("tv_sec: %d tv_usec: %d\n", timeval->tv_sec, timeval->tv_usec);

	// TODO call the settimeofday() and save return_value and errno

    XRPCMessage *out_msg, msg = X_RPCMESSAGE__INIT;

	msg.message_type_case = X_RPCMESSAGE__MESSAGE_TYPE_RESPONSE;
	Response response;
	msg.response = &response;
	response__init(msg.response);

	msg.response->response_func_case = REQUEST__REQUEST_FUNC_SETTIMEOFDAY_REQUEST;
    SettimeofdayResponse settimeofday_response;
	msg.response->settimeofday_response = &settimeofday_response;
	settimeofday_response__init(msg.response->settimeofday_response);
	// TODO set return_value and errno here
	msg.response->settimeofday_response->has_return_value = 1;
	msg.response->settimeofday_response->return_value = -1;
	msg.response->settimeofday_response->has_errno = 1;
	msg.response->settimeofday_response->errno = 7;

    size_t len = x_rpcmessage__get_packed_size(&msg);
	if (len >= MAX_BUF) {
		printf("message is too long\n");
		exit(2);
	}
	// avoiding malloc()
	uint8_t buf[len + 1];

    //lets get the serialized structure in buf
    x_rpcmessage__pack(&msg, (void *)&buf);

	// TODO publish the buf

	// verify out_msg in gdb, remove later
	out_msg = x_rpcmessage__unpack(NULL, len, (void *)&buf);

	return 0;
}

int gettimeofday_func(void *val) {
	printf("gettimeofday\n");
	return 0;
}
typedef int (* func_ptr) (void *);
static func_ptr request_fp[] =  {
	func0,
	settimeofday_func,
	gettimeofday_func,
};


int main(int argc, const char * argv[])
{
    XRPCMessage *out_msg, msg = X_RPCMESSAGE__INIT;

    unsigned int len;

	msg.message_type_case = X_RPCMESSAGE__MESSAGE_TYPE_REQUEST;
	Request request;
	msg.request = &request;
	request__init(msg.request);

	msg.request->request_func_case = REQUEST__REQUEST_FUNC_SETTIMEOFDAY_REQUEST;
    SettimeofdayRequest settimeofday_request;
	msg.request->settimeofday_request = &settimeofday_request;
	settimeofday_request__init(msg.request->settimeofday_request);

	// third level of nesting, define, init, assign
	TimeVal timeval;
	msg.request->settimeofday_request->timeval = &timeval;
	time_val__init(msg.request->settimeofday_request->timeval);
	msg.request->settimeofday_request->timeval->has_tv_sec = 1;;
	msg.request->settimeofday_request->timeval->tv_sec = time(NULL);
	msg.request->settimeofday_request->timeval->has_tv_usec = 1;;
	msg.request->settimeofday_request->timeval->tv_usec = 888;

    len = x_rpcmessage__get_packed_size(&msg);
	if (len >= MAX_BUF) {
		printf("message is too long\n");
		exit(2);
	}
	// avoiding malloc()
	uint8_t buf[len + 1];

    //lets get the serialized structure in buf
    x_rpcmessage__pack(&msg, (void *)&buf);

	// for testing, use gdb to inspect the values and pointers
	out_msg = x_rpcmessage__unpack(NULL, len, (void *)&buf);
	
	printf("message_type: %d\n", out_msg->message_type_case);	
	if (out_msg->message_type_case == X_RPCMESSAGE__MESSAGE_TYPE_REQUEST) {
		printf("time_stamp: %d\n", out_msg->request->settimeofday_request->timeval->tv_sec);
		request_fp[out_msg->request->request_func_case]((void *) (out_msg->request));
	}
	return 0;
}
