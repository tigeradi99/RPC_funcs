#!/usr/bin/env python3

import sys
import paho.mqtt.client as paho
import datetime
import time
import messagefile_pb2

msg = messagefile_pb2.xRPCMessage
msg.time_stamp = 111

msg.request = messagefile_pb2.SettimeofdayRequest
msg.request.timeval = messagefile_pb2.TimeVal
msg.request.timeval.tv_sec = 333

print(msg.request.timeval.tv_sec)

# TODO fix error
msg.SerializeToString()

settimeofday_request = messagefile_pb2.SettimeofdayRequest
settimeofday_request.timeval = messagefile_pb2.TimeVal

settimeofday_request.timeval.tv_sec = 222

msg.request = settimeofday_request



print(msg.request.timeval.tv_sec)

