# ESP-MQTT application

This example connects to the broker URI selected using `idf.py menuconfig` (using mqtt tcp transport) and  subscribes/unsubscribes and sends a message on certain topic.
(Please note that the public broker is maintained by the community so may not be always available, for details please see this [disclaimer](https://iot.eclipse.org/getting-started/#sandboxes))

Note: If the URI equals `FROM_STDIN` then the broker address is read from stdin upon application startup (used for testing)

It uses ESP-MQTT library which implements mqtt client to connect to mqtt broker, and then uses protocol buffers for communication.

To interface with the BME280 I2C sensor, we have used the BME280 library from Bosch Sensortec. (https://github.com/BoschSensortec/BME280_driver/tree/master)
The library is stored in the components subdirectory.

## How to set up development environment:

A detailed explanation on how to set up ESP-IDF can be found here:

[GET-STARTED](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)

It is recommended to use Python 3.8 or onwards. Remember to set an alternative in case you are using python 2.7 (UBUNTU 18.04).

* sudo apt-get install python3 python3-pip python3-setuptools
* Finally, set alternative by running : sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10 && alias pip=pip3

In case you are using UBUNTU 20.04 and above, Python 3.8 will be the default interpreter.

More detailed docs for protocol-buffers, LED Control are provided in the DOCS directory.

### External Libraries needed:
If you intend to develop/add more functionality; you will require:
* Protocol Buffer compiler:
  * sudo apt-get install protobuf-compiler (To install Google Protocol Buffer compiler packages)
  * sudo apt-get install protobuf-c-compiler (Unofficial implementation of Protocol Buffers compiler for C)
  * pip install protobuf (Install protocol buffers package for python)
  * To compile .proto files to C format: navigate to file directory; then run protoc-c --c_out=. filename.proto
  * To compile .proto files to Python format: navigate to file directory; then run protoc --python_out=. filename.proto

* MQTT package for python: pip install paho-mqtt


## How to use example

The source code for MQTT operation was sourced from the mqtt examples on the ESP-IDF examples directory. It was modified to work with Protocol Buffers and a rudimentary form of RPC was implemented.

Read below on how to use this example.
### Hardware Required

This example can be executed on any ESP32 board, the only required interface is WiFi and connection to internet, and a BME280 sensor.

Use client.py to execute RPCs on the board. (gettimeofday/settimeofday/ledcontrol/i2c_op

The currently set device ID is 101, that means publisher has to publish to 101/xRPC_Request to execute RPCs.

When client.py is executed, it will prompt you to enter a topic to publish to. Here you may enter 101/xRPC_Request (this is hardcoded to esp32)

Then you may pick any one of the operations, and then enter the necessary inputs to execute the operation.

After that if you want to continue, press CTRL+C(WIN) / CMD + C(MAC) and then enter whether you want to continue or not (y/n). 
# PIN ASSIGNMENT FOR I2C OPERATION:
* Enter pin nos. for master sda and scl (We can use master_sda_gpio = 21, master_scl_gpio = 22) 

Preferably , use this setting while testing:
|                  | SDA    | SCL    |
| ---------------- | ------ | ------ |
| BME280           | GPIO21 | GPIO22 |

 Connecting BME280 to ESP32

  - GPIO21 is assigned as the data signal of BME280 (SDA)
  - GPIO22 is assigned as the clock signal of BME280 (SCL)
  - Use 3V3 from ESP32 to power the BME280, then connect the GND pin next to 3V3  pin on the ESP32 to the GND pin on the BME280.

If not connected properly, then an I2C timeout error will occur. 

**Note: ** There’s no need to add an external pull-up resistors for SDA/SCL pin, because the driver will enable the internal pull-up resistors by default in our program.

### Configure the project

* Open the project configuration menu (`idf.py menuconfig`)
* Configure Wi-Fi or Ethernet under "Example Connection Configuration" menu. Enter the Wifi SSID and password.
* Configure MQTT connection (if needed) under "Example Configuration" menu. NOte that MQTT broker links should start with mqtt://(URL)
* Remember to change the mqtt broker url in the client.py program, if you change the broker in the ESP32 configs
* When using Make build system, set `Default serial port` under `Serial flasher config`.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output for gettimeofday/settimeofday
### Output for gettimeofday()

```
I (4667) MQTT_EXAMPLE: Other event id:7
W (4677) wifi:<ba-add>idx:1 (ifx:0, cc:2d:21:72:42:41), tid:0, ssn:0, winSize:64
W (5007) wifi:<ba-add>idx:2 (ifx:0, cc:2d:21:72:42:41), tid:1, ssn:0, winSize:64
I (5317) MQTT_EXAMPLE: MQTT_EVENT_CONNECTED
I (5317) MQTT_EXAMPLE: Successfully subscribed to msg_id=64959
I (5617) MQTT_EXAMPLE: MQTT_EVENT_SUBSCRIBED, msg_id=64959
I (10127) MQTT_EXAMPLE: MQTT_EVENT_DATA
TOPIC=101/xRPC_Request
Size of event: 16 
Message unpacked, length of buffer: 16  
Reached check condition 
Reached value storing. 
toSend.mes_type : 0x0 
messageType : 0x3ffb0ebc 
toSend.mes_type : 0x3ffb0ebc 
Values succesfully set by message type. 
Starting function call: 
Passing Parameters to gettimeofday. 
Parameters passed, values obtained from gettimeofday. 
Storing values to response 
seconds : 9
micro seconds : 745973 
Returning value and exiting function gettimeofday. 
Return value generated from x_gettimeofday. 
Time obtained from gettimeofday: 
seconds : 9
micro seconds : 745973 
gettime response: seconds: 9 | microseconds: 745973| return_value: 0 | errno: 119 
settimerequest: seconds: 0 | microseconds: 0 
set time response: return_value: 0 | errno: 0 
All variables set, going to pack. 
Length of packed buffer: 27 
Packing complete 
I (10197) MQTT_EXAMPLE: sent publish response successful, msg_id=0
I (10207) MQTT_EXAMPLE: The current date/time is: Thu Jan  1 00:00:09 1970 
```
### Output at client-pub-alt2.py:
```
client-015347 spr.io 60083
11/19 16:24:58
Sent request to get time
```
## Example Output settimeofday
### Output for settimeofday:
```
I (147957) MQTT_EXAMPLE: MQTT_EVENT_DATA
TOPIC=101/xRPC_Request
Size of event: 24 
Message unpacked, length of buffer: 24  
Reached check condition 
Reached value storing. 
toSend.mes_type : 0x3ffb0ebc 
messageType : 0x3ffb0ebc 
toSend.mes_type : 0x3ffb0ebc 
Values succesfully set by message type. 
Starting function call: 
Passing Parameters to timeval struct. 
tv_sec: 1605803236 
tv_usec: 136679 
Passing Parameters to settimeofday. 
Parameters passed, values obtained from settimeofday. 
Storing values to response 
Returning value and exiting function settimeofday. 
Return value generated from x_settimeofday. 
gettime response: seconds: 1605803236 | microseconds: 152666| return_value: 0 | errno: 119 
settimerequest: seconds: 1605803236 | microseconds: 136679 
set time response: return_value: 0 | errno: 119 
All variables set, going to pack. 
Length of packed buffer: 42 
Packing complete 
I (148027) MQTT_EXAMPLE: sent publish response successful, msg_id=0
I (148027) MQTT_EXAMPLE: The current date/time is: Thu Nov 19 16:27:16 2020 
```
### Output of client-pub-alt2.py:
```
client-819918 spr.io 60083
11/19 16:27:16
Request to set time sent. Timeval: 1605803236 136679
```
## Example : ledcontrol
### client.py
```
Enter procedure: gettimeofday / settimeofday / ledcontrol / i2c_op .
ledcontrol
Current timestamp: seconds 1610995734 | microseconds 711292
Enter duty cycle: range(0-8191) 4192
Enter speed mode: high/low/max high
Using channel 0
Raw Data:  b'\x12\x04\x1a\x02\x08\x01\x1a\x07\x08\xc6\x01\x10\xfe\x9c\x02'
Parsing message.
01/19 00:19:02 receiving <101/xRPC_Response>
Procedure called: ledcontroller
Timestamp: seconds: 198 microseconds: 36478
Led control achieved. Press Ctrl + C to stop current loop and select y/n to send another request.

```
### ESP32 Monitor:
```
I (198434) xRPC_example: MQTT_EVENT_DATA
TOPIC=101/xRPC_Request
Size of event: 31 
Message unpacked, length of buffer: 31  
Reached check condition 
Timestamp: tv_sec : 1610995734 | tv_usec: 711292 
message index: 3 
Function ledcontroller_func started. 
Finished setting values. 
I (198454) xRPC_example: Successfully published to 101/xRPC_Response, msg_id=0
Finished. 
Exiting... 
I (198464) xRPC_example: The current date/time is: Thu Jan  1 00:03:18 1970
```
## Example Output: i2c_op (read temperature, pressure and relative humidity)
### client.py:
```
client-963300 test.mosquitto.org 1883
Enter subscriber topic:
101/xRPC_Request
Enter procedure: gettimeofday / settimeofday / ledcontrol / i2c_op .
i2c_op
Current timestamp: seconds 1610995579 | microseconds 330712
Enter sensor sda pin: 21
Enter sensor scl pin: 22
Raw Data:  b'\x12\x11"\x0f\x15|\x1a\xe6A\x1d\xc6\x1f\x8aB%\x89\x0b\xc5G\x1a\x06\x08&\x10\xc3\xe7$'
Parsing message.
01/19 00:16:24 receiving <101/xRPC_Response>
Procedure called: i2c operation
Timestamp: seconds: 38 microseconds: 603075
i2c master read/write to slave buffer successful.
Temperature in C:  28.76293182373047
Relative Humidity in %:  69.06205749511719
Pressure in Pa:  100887.0703125
Press Ctrl + C to stop current loop and select y/n (yes/no) to send another request.
```
### ESP32 Monitor:
```
I (7154) xRPC_example: MQTT_EVENT_CONNECTED
I (7164) xRPC_example: Successfully subscribed to msg_id=33091
I (7364) xRPC_example: MQTT_EVENT_SUBSCRIBED, msg_id=33091
I (39004) xRPC_example: MQTT_EVENT_DATA
TOPIC=101/xRPC_Request
Size of event: 20 
Message unpacked, length of buffer: 20  
Reached check condition 
Timestamp: tv_sec : 1610995579 | tv_usec: 330712 
message index: 4 
Creating Task: 
I (39014) xRPC_example: The current date/time is: Thu Jan  1 00:00:38 1970 
                                                                                                          
BME280 set status: 0 
BME280 set sensor mode status: 0 
BME280 get_data status: 0 
Temp: 28.76, Pressure:100887.07, Humidity:69.06
I (40614) xRPC_example: Successfully published to 101/xRPC_Response, msg_id=0
Finished. 
Exiting...
```