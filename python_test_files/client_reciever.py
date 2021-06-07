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
    inb = message.payload
    print("Raw Data: " ,inb)
    print("Parsing message.")
    data_inbox.ParseFromString(inb)
    #print("Data inbox: ", data_inbox)
    now = datetime.datetime.now()
    time_stamp = now.strftime("%m/%d %H:%M:%S")
    print(time_stamp, "receiving <"+ message.topic +">")
    if data_inbox.response.HasField("i2c_response"):
        print("Procedure called: i2c operation")
        print("Timestamp: seconds: {} microseconds: {}".format(data_inbox.time_stamp.tv_sec , data_inbox.time_stamp.tv_usec))
        if data_inbox.response.i2c_response.success == 0:
            print("i2c master read/write to slave buffer successful.")
            print("Temperature in C: ", data_inbox.response.i2c_response.temp)
            print("Relative Humidity in %: ", data_inbox.response.i2c_response.rel_hum)
            print("Pressure in Pa: ", data_inbox.response.i2c_response.pres)
            print("Press Ctrl + C to stop current loop and select y/n (yes/no) to send another request.")
        else:
            print("Error occured!")
    else:
        print("Requests are not supported by the subscriber.")


def main(client, topic):
            
    client.loop_start() #start loop to process received messages
    
    
    try:
        while True:
           time.sleep(1)
           pass
        time.sleep(1)
        pass
    except KeyboardInterrupt:
        print ("You hit control-c")
    
    time.sleep(1)
    
    client.loop_stop()
    return 0

if __name__ == "__main__":
    broker="test.mosquitto.org"
    port=1883
    ts = datetime.datetime.now().isoformat()
    c = 'client-' + ts[-6:]
    print(c, broker, port)
    client= paho.Client(c)
    client.on_message=on_message
    print("Enter subscriber topic:")
    topic = input()
    client.connect(broker, port)
    client.subscribe("+/xRPC_Response") # default: subscribe to all topics in /xRPC_Response
    while True:
        main(client,topic)
        inp = input("Do you want to continue? y/n: ")
        if inp == "n":
            client.disconnect()
            break