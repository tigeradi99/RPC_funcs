#!/usr/bin/env python3

import time
import sys
import datetime
import messagefile_pb2 as message_out

# print(sys.version_info)
class I2C_Sensor:
  
  def __init__(self, client_handle = None, topic = None):
      self.client_handle = client_handle
      self.topic= topic
      self.data_outbox = message_out.xRPCMessage()
     
  def serialize_data(self, sda_pin , scl_pin):  

    sec1 , usec1 = divmod(time.time(),1)
    time_stamp = message_out.TimeVal()
    time_stamp.tv_sec = int(sec1)
    time_stamp.tv_usec = int(usec1 * 1000000)
    self.data_outbox.time_stamp.CopyFrom(time_stamp)
    print("Current timestamp: seconds {} | microseconds {}".format(self.data_outbox.time_stamp.tv_sec, self.data_outbox.time_stamp.tv_usec))
    sda = sda_pin
    scl = scl_pin
    self.data_outbox.request.i2c_request.slave_sda_gpio = int(sda)
    self.data_outbox.request.i2c_request.slave_scl_gpio = int(scl)
    print("Outbox: ", self.data_outbox)


      
  def send(self):
    otb = self.data_outbox.SerializeToString()
    self.client_handle.publish(self.topic,otb,qos=0)
    



