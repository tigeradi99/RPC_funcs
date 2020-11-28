#!/usr/bin/env python3

import queue
import time
import sys
import paho.mqtt.client as paho
import datetime
import random
import messagefile_pb2 as message_in

# print(sys.version_info)

message_q=queue.Queue()

def empty_queue(delay=0):
    while not message_q.empty():
      m=message_q.get()
      print("Received message  ",m)
    if delay!=0:
      time.sleep(delay)
#define callback
def on_message(client, userdata, message):
  #time.sleep(1)
  data_inbox = message_in.xRPCMessage()
  inb = bytes(message.payload)
  print("Raw Data: " ,inb)
  print("Parsing message.")
  data_inbox.ParseFromString(inb)
  print("Data inbox: ", data_inbox)
  now = datetime.datetime.now()
  time_stamp = now.strftime("%m/%d %H:%M:%S")
  print(time_stamp, "receiving <"+ message.topic +">")
  if data_inbox.response.HasField("gettimeofday_response"):
      print("Message type: gettimeofday_response")
      print("System time at ESP32 in seconds and microseconds: {} {}".format(data_inbox.response.gettimeofday_response.timeval.tv_sec, data_inbox.response.gettimeofday_response.timeval.tv_usec))
      print("getTime response return and errno: {} {}".format(data_inbox.response.gettimeofday_response.return_value, data_inbox.response.gettimeofday_response.errno))
  elif data_inbox.response.HasField("settimeofday_response"):
      print("Procedure called: settimeofday_response")
      print("Set time return value and errno: {} {}".format(data_inbox.response.settimeofday_response.return_value, data_inbox.response.settimeofday_response.errno))
      print("Timestamp: seconds: {} microseconds: {}".format(data_inbox.time_stamp.tv_sec , data_inbox.time_stamp.tv_usec))
  else:
      print("Requests are not supported by the subscriber.")
broker="spr.io"
port=60083
ts = datetime.datetime.now().isoformat()
c = 'client-' + ts[-6:]
print(c, broker, port)
client= paho.Client(c)
client.on_message=on_message

client.connect(broker, port)
if len(sys.argv) == 1:
    client.subscribe("+/xRPC_Response") # default: subscribe to all topics in /xRPC_Response
else:
    for arg in sys.argv[1:]:
        print("subscribing to ", arg)
        client.subscribe(arg)

client.loop_start() #start loop to process received messages

time.sleep(1)
try:
  while True:
    time.sleep(1)
    pass
except KeyboardInterrupt:
    print ("You hit control-c")

time.sleep(1)

client.disconnect()
client.loop_stop()
