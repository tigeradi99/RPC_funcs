# BME280 I2C Sensor Interface v1.0

This is an I2C sensor interface which can be used to read BME280 sensor data(temperature, humidity and pressure) over MQTT. Functions have been implemented for serializing read request and deserializing responses from the sensor, and the sensor data is returned as a tuple in the following format:
    (temperature, humidity, pressure)

## Usage Instructions:
1. Move the files interface.py, messagefile_pb2.py to the directory where your python script  is located.

2. Import the I2C_Sensor class by using:
```from interface import I2C_Sensor```

3. Create an object of this class with the MQTT client handle and topic(101/xRPC_Request) as parameters.

4. Call the function **serialize_data()**, which will take in two parameters: pin number of SDA( serial data line, set to 21 as default) and pin number of SCL(serial clock, set to 22 as default). Parameters need not be passed if sensor's SDA and SCL are connected to these default pins during usage.
```I2C_Sensor.serialize_data(<sda-pin> , <scl-pin>)```

5. Call the **send()** function, which will send a request to the sensor via MQTT. The response recieved will stored in a message queue.

6. Finally, call the **deserialize_message()** function, which will extract the temperature, humidity and pressure values from the sensor response message. The function will a tuple in the following format: (temperature, humidity, pressure)

You can refer to the **test.py** script to see an example on how to use this interface.

## Dependencies:
To run this interface, you need:
1. Protocol Buffers package for Python, v3.14.0
```pip install protobuf==3.14.0```

2. Paho MQTT library
```pip install paho-mqtt```