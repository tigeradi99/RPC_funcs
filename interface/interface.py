import messagefile_pb2 as message_out
import time 
import queue
class I2C_Sensor:

    def __init__(self, client_handle, topic):
        #self.foo = "bar"
        #Initialize the following: They should be passed as a parameter when an object of this class is created
        # the MQTT client handle
        #Also initialize a data buffer tx_data to store data which will be sent over MQTT
        self.client_handle = client_handle
        self.topic = topic
        self.client_handle.on_message = self.on_message
        self.client_handle.subscribe("+/xRPC_Response")
        self.data_outbox = message_out.xRPCMessage()    #Store incoming message
        self.data_inbox = message_out.xRPCMessage()     #Store message to be serialized
        self.tx_data = None                             #Store serialized message to be published
        self.message_q = queue.Queue()
        self.temp = float(0)
        self.humidity = float(0)
        self.pressure = float(0)

        
    def serialize_data(self, sda_pin = 21 , scl_pin = 22):
        #First fill in the fields for timestamp
        #COnsult the Google documentation for Protocol Buffers in Python

        sec1 , usec1 = divmod(time.time(),1)
        time_stamp = message_out.TimeVal()
        time_stamp.tv_sec = int(sec1)
        time_stamp.tv_usec = int(usec1 * 1000000)
        self.data_outbox.time_stamp.CopyFrom(time_stamp)

        #Serialize the data with given parameters (Only serialize data for I2C sensor read request, ignore the rest)
        #foo.request.i2c_request.slave_sda_gpio = int(sda)
        #foo_outbox.request.i2c_request.slave_scl_gpio = int(scl)

        self.data_outbox.request.i2c_request.slave_scl_gpio = int(scl_pin)
        self.data_outbox.request.i2c_request.slave_sda_gpio = int(sda_pin)
        #Consult the messagefile.proto definition for field names

        self.tx_data = self.data_outbox.SerializeToString() #serializing buffer to string, which will be sent via MQTT
        print("Done serializing.")
    
    def on_message(self, client, userdata, message):
        self.message_q.put(message)
        client.loop_stop()

    def deserialize_message(self):
        if(not self.message_q.empty()):
            print("Empty Queue: No message recieved yet")

        m = self.message_q.get() #Extract recieved message from queue
            
        inb = m.payload

        self.data_inbox.ParseFromString(inb)

        if self.data_inbox.response.HasField("i2c_response"):
            if self.data_inbox.response.i2c_response.success == 0:
                self.temp = self.data_inbox.response.i2c_response.temp
                self.humidity = self.data_inbox.response.i2c_response.rel_hum
                self.pressure = self.data_inbox.response.i2c_response.pres
        
        return self.temp, self.humidity, self.pressure
    
    def send(self):
        #Send the serialized data
        print("Subscribed")
        self.client_handle.loop_start()
        self.client_handle.publish(self.topic, self.tx_data, qos = 0)
        print("Data sent")
        