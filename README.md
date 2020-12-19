# ESP-MQTT sample application
(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example connects to the broker URI selected using `idf.py menuconfig` (using mqtt tcp transport) and  subscribes/unsubscribes and sends a message on certain topic.
(Please note that the public broker is maintained by the community so may not be always available, for details please see this [disclaimer](https://iot.eclipse.org/getting-started/#sandboxes))

Note: If the URI equals `FROM_STDIN` then the broker address is read from stdin upon application startup (used for testing)

It uses ESP-MQTT library which implements mqtt client to connect to mqtt broker, and then uses protocol buffers for communication.

To interface with the BME280 I2C sensor, we have used the BME280 library from Bosch Sensortec. (https://github.com/BoschSensortec/BME280_driver/tree/master)
The library is stored in the components subdirectory.
## How to use example

### Hardware Required

This example can be executed on any ESP32 board, the only required interface is WiFi and connection to internet, and a BME280 sensor.
Use client.py to execute RPCs on the board.
The currently set device ID is 101, that means publisher has to publish to 101/xRPC_Request to execute RPCs.
When client.py is executed, it will prompt you to enter a topic to publish to. Here you may enter 101/xRPC_Request (this is hardcoded to esp32)
Then you may pick any one of the operations, and then enter the necessary inputs to execute the operation.
After that if you want to continue, press CTRL+C(WIN) / CMD + C(MAC) and then enter whether you want to continue or not (y/n). 
# PIN ASSIGNMENT FOR I2C OPERATION:
* Enter pin nos. for master sda and scl (We can use master_sda_gpio = 18, master_scl_gpio = 19) 

Preferably , use this settings while testing:
|                  | SDA    | SCL    |
| ---------------- | ------ | ------ |
| BME280           | GPIO18 | GPIO19 |

 Connecting BME280 to ESP32
  - GPIO18 is assigned as the data signal of BME280 (SDA)
  - GPIO19 is assigned as the clock signal of BME280 (SCL)
If not connected properly, then an I2C timeout error will occur. 

**Note: ** There’s no need to add an external pull-up resistors for SDA/SCL pin, because the driver will enable the internal pull-up resistors by default in our program.

### Configure the project

* Open the project configuration menu (`idf.py menuconfig`)
* Configure Wi-Fi or Ethernet under "Example Connection Configuration" menu. See "Establishing Wi-Fi or Ethernet Connection" section in [examples/protocols/README.md](../../README.md) for more details.
* When using Make build system, set `Default serial port` under `Serial flasher config`.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output (NEEDS TO BE UPDATED)
Output for gettimeofday()

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
Output at client-pub-alt2.py:
```
client-015347 spr.io 60083
11/19 16:24:58
Sent request to get time
```
## Example Output settimeofday
Output for settimeofday:
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
Output of client-pub-alt2.py:
```
client-819918 spr.io 60083
11/19 16:27:16
Request to set time sent. Timeval: 1605803236 136679
```

