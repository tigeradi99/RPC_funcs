import messagefile_pb2 as message_out
import time 
class I2C_Sensor:

    def __init__(self, client_handle = None, topic = None):
        self.foo = "bar"
        #Initialize the following: They should be passed as a parameter when an object of this class is created
        # the MQTT client handle
        #Also initialize a data buffer tx_data to store data which will be sent over MQTT
        
    def serialize_data(self, sda_pin , scl_pin):
        #First fill in the fields for timestamp
        #data_outbox = message_in.xRPCMessage() -> This is the buffer created
        
        #COnsult the Google documentation for Protocol Buffers in Python

        #sec1 , usec1 = divmod(time.time(),1)
        #time_stamp = foo.TimeVal()
        #time_stamp.tv_sec = int(sec1)
        #time_stamp.tv_usec = int(usec1 * 1000000)

        #Serialize the data with given parameters (Only serialize data for I2C sensor read request, ignore the rest)
        #foo.request.i2c_request.slave_sda_gpio = int(sda)
        #foo_outbox.request.i2c_request.slave_scl_gpio = int(scl)

        #Consult the messagefile.proto definition for field names
        
        foo = "bar"

        #self.tx_data = foo_outbox.SerializeToString() -> serializing buffer to string, which will be sent via MQTT
        
    
    def send(self):
        #Send the serialized data
        foo = "bar"