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
    print("Data inbox: ", data_inbox)
    now = datetime.datetime.now()
    time_stamp = now.strftime("%m/%d %H:%M:%S")
    print(time_stamp, "receiving <"+ message.topic +">")
    if data_inbox.response.HasField("gettimeofday_response"):
        print("Message type: gettimeofday_response")
        print("Timestamp: seconds: {} microseconds: {}".format(data_inbox.time_stamp.tv_sec , data_inbox.time_stamp.tv_usec))
        print("System time at ESP32 in seconds and microseconds: {} {}".format(data_inbox.response.gettimeofday_response.timeval.tv_sec, data_inbox.response.gettimeofday_response.timeval.tv_usec))
        print("getTime response return and errno: {} {}".format(data_inbox.response.gettimeofday_response.return_value, data_inbox.response.gettimeofday_response.errno_alt))
        print("Press Ctrl + C to stop current loop and select y/n to send another request.")
    elif data_inbox.response.HasField("settimeofday_response"):
        print("Procedure called: settimeofday_response")
        print("Set time return value and errno: {} {}".format(data_inbox.response.settimeofday_response.return_value, data_inbox.response.settimeofday_response.errno_alt))
        print("Timestamp: seconds: {} microseconds: {}".format(data_inbox.time_stamp.tv_sec , data_inbox.time_stamp.tv_usec))
        print("Press Ctrl + C to stop current loop and select y/n to send another request.")
    elif data_inbox.response.HasField("ledc_channel_config_response"):
        print("Procedure called: ledcontroller")
        print("Timestamp: seconds: {} microseconds: {}".format(data_inbox.time_stamp.tv_sec , data_inbox.time_stamp.tv_usec))
        if(data_inbox.response.ledc_channel_config_response.config_status == 1):
            print("Led control achieved. Press Ctrl + C to stop current loop and select y/n to send another request.")
        else:
            print("Failed")
    else:
        print("Requests are not supported by the subscriber.")

def rpc_sen(client, topic , procedure):
    data_outbox = message_in.xRPCMessage()
    
    sec1 , usec1 = divmod(time.time(),1)
    time_stamp = message_in.TimeVal()
    time_stamp.tv_sec = int(sec1)
    time_stamp.tv_usec = int(usec1 * 1000000)
    data_outbox.time_stamp.CopyFrom(time_stamp)
    print("Current timestamp: seconds {} | microseconds {}".format(data_outbox.time_stamp.tv_sec, data_outbox.time_stamp.tv_usec))
    if procedure == "settimeofday":
        now = datetime.datetime.utcnow()
        time_stamp = now.strftime("%m/%d %H:%M:%S")
        print(time_stamp)
        sec , usec = divmod(time.time(),1)
        time_stamp_setRequest = message_in.TimeVal()
        time_stamp_setRequest.tv_sec = int(sec)
        time_stamp_setRequest.tv_usec = int(usec * 1000000)
        data_outbox.request.settimeofday_request.timeval.CopyFrom(time_stamp_setRequest)
        print("Request to set time sent. Timeval: {} {}".format(data_outbox.request.settimeofday_request.timeval.tv_sec, 
        data_outbox.request.settimeofday_request.timeval.tv_usec ))
        print("Message type:", data_outbox.request)
        print("Outbox: ", data_outbox)
    if procedure == "gettimeofday":
        data_outbox.request.gettimeofday_request.stub = 0
        print("Message type:", data_outbox.request)
        print("Outbox: ", data_outbox)
        print("Request to get time set.")
    if procedure == "ledcontrol":
        data_outbox.request.ledc_channel_config_request.ledc_conf.gpio_num = 2
        data_outbox.request.ledc_channel_config_request.ledc_conf.hpoint = 0
        data_outbox.request.ledc_channel_config_request.ledc_conf.duty = int(input("Enter duty cycle: range(0-8191) "))
        speed_mode = input("Enter speed mode: high/low/max ")
        correct = True
        while correct:
            if speed_mode == "high":
                data_outbox.request.ledc_channel_config_request.ledc_conf.speed_mode.ledc_speed = message_in.ledc_mode_t.ledc_mode_e.LEDC_HIGH_SPEED_MODE
                correct = False
            elif speed_mode == "low":
                data_outbox.request.ledc_channel_config_request.ledc_conf.speed_mode.ledc_speed = message_in.ledc_mode_t.ledc_mode_e.LEDC_LOW_SPEED_MODE
                correct = False
            elif speed_mode =="max":
                data_outbox.request.ledc_channel_config_request.ledc_conf.speed_mode.ledc_speed = message_in.ledc_mode_t.ledc_mode_e.LEDC_SPEED_MODE_MAX
                correct = False
            else:
                print("Enter valid input: high/low/max")
                correct = True
        #data_outbox.request.ledc_channel_config_request.ledc_conf.speed_mode.ledc_speed = message_in.ledc_mode_t.ledc_mode_e.LEDC_HIGH_SPEED_MODE
        print("Using channel 0")
        data_outbox.request.ledc_channel_config_request.ledc_conf.channel.channel = 0
        data_outbox.request.ledc_channel_config_request.ledc_conf.intr_type.intr = message_in.ledc_intr_type_t.interrupt_type.LEDC_INTR_FADE_END
        print("Message type:", data_outbox.request)
        print("Outbox: ", data_outbox)
    otb = data_outbox.SerializeToString()
    client.publish(topic,otb,qos=0)





def main(client, topic):
    print("Enter procedure: gettimeofday/settimeofday. ")
    procedure = input()
    rpc_sen(client, topic , procedure)
    
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
    broker="spr.io"
    port=60083
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