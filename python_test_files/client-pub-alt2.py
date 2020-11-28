#!/usr/bin/env python3

import sys
import paho.mqtt.client as paho
import datetime
import time
import messagefile_pb2 as message

# print(sys.version_info)
def rpc_sen():
    data_outbox = message.xRPCMessage()
    
    sec1 , usec1 = divmod(time.time(),1)
    time_stamp = message.TimeVal()
    time_stamp.tv_sec = int(sec1)
    time_stamp.tv_usec = int(usec1 * 1000000)
    data_outbox.time_stamp.CopyFrom(time_stamp)
    print("Current timestamp: seconds {} | microseconds {}".format(data_outbox.time_stamp.tv_sec, data_outbox.time_stamp.tv_usec))
    if sys.argv[2] == "settimeofday":
        now = datetime.datetime.utcnow()
        time_stamp = now.strftime("%m/%d %H:%M:%S")
        print(time_stamp)
        sec , usec = divmod(time.time(),1)
        time_stamp_setRequest = message.TimeVal()
        time_stamp_setRequest.tv_sec = int(sec)
        time_stamp_setRequest.tv_usec = int(usec * 1000000)
        data_outbox.request.settimeofday_request.timeval.CopyFrom(time_stamp_setRequest)
        print("Request to set time sent. Timeval: {} {}".format(data_outbox.request.settimeofday_request.timeval.tv_sec, 
        data_outbox.request.settimeofday_request.timeval.tv_usec ))
        print("Message type:", data_outbox.request)
        print("Outbox: ", data_outbox)
    if sys.argv[2] == "gettimeofday":
        data_outbox.request.gettimeofday_request.stub = 0
        print("Message type:", data_outbox.request)
        print("Outbox: ", data_outbox)
        print("Request to get time set.")
    otb = data_outbox.SerializeToString()
    client.publish(sys.argv[1],otb,qos=0)

broker="spr.io"
port=60083
ts = datetime.datetime.now().isoformat()
c = 'client-' + ts[-6:] # use the timestamp to create unique MQTT client
print(c, broker, port)

client= paho.Client(c)
client.connect(broker, port)

# publish an empty string
if len(sys.argv) == 2:
    client.publish(sys.argv[1],'',retain = False, qos=0)
# publish <topic> <message>
elif len(sys.argv) == 3:
    rpc_sen()
# publish <topic> <message> <retain>
elif len(sys.argv) == 4:
    data_outbox = message.xRPCMessage()
    
    print("Message type:")
    sec1 , usec1 = divmod(time.time(),1)
    time_stamp = message.TimeVal()
    time_stamp.tv_sec = int(sec1)
    time_stamp.tv_usec = int(usec1 * 1000000)
    data_outbox.time_stamp.CopyFrom(time_stamp)
    print("Current timestamp: seconds {} | microseconds {}".format(data_outbox.time_stamp.tv_sec, data_outbox.time_stamp.tv_usec))
    if sys.argv[2] == "settimeofday":
        data_outbox.request = message.SettimeofdayRequest
        now = datetime.datetime.utcnow()
        time_stamp = now.strftime("%m/%d %H:%M:%S")
        print(time_stamp)
        sec , usec = divmod(time.time(),1)
        time_stamp_setRequest = message.TimeVal()
        time_stamp_setRequest.tv_sec = int(sec)
        time_stamp_setRequest.tv_usec = int(usec * 1000000)
        data_outbox.request.timeval.CopyFrom(time_stamp_setRequest)
        print("Request to set time sent. Timeval: {} {}".format(data_outbox.settimeofday_request.timeval.tv_sec, 
        data_outbox.settimeofday_request.timeval.tv_usec ))
    if sys.argv[2] == "gettimeofday":
        data_outbox.request = message.GettimeofdayRequest()
        data.outbox.stub = 0
        data_outbox.request.CopyFrom(stub)
        print("Request to get time set.")
    otb = data_outbox.SerializeToString()
    client.publish(sys.argv[1],otb, retain = True, qos=0)
else:
    print(sys.argv[0], '<topic> <message> [True (retain)]')

client.disconnect()
client.loop_stop()
