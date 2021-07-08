from interface import I2C_Sensor
import datetime
import paho.mqtt.client as paho

if __name__=="__main__":
    broker = "test.mosquitto.org"
    port = 1883
    ts = datetime.datetime.now().isoformat()
    c = 'client-' + ts[-6:]
    client= paho.Client(c)
    client.connect(broker,port)
    sensor = I2C_Sensor(client, "101/xRPC_Request") # Initialize the sensor class
    sensor.serialize_data(21,22) #serialize read request
    sensor.send() #send request via MQTT, wait for a response and store it in a queue
    
    #Extract message from queue and return parameters
    temp, hum, pres = sensor.deserialize_message() 
    
    print(f'Temperature: {temp}, Pressure: {pres}, Humidity: {hum}') 
    client.disconnect()
