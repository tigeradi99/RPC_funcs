/* MQTT (over TCP) Protocol Buffers Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "messagefile.pb-c.h"
#include <sys/time.h>
#include "driver/ledc.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "bme280.h"

#define DATA_LENGTH 512                  /*!< Data buffer length of test buffer */
#define RW_TEST_LENGTH 128               /*!< Data length for r/w test, [0,DATA_LENGTH] */
#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave rx buffer size */
#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */
#define I2C_MASTER_PORT I2C_NUM_0                             /*!<I2C CONTROLLER PORT SET TO 0 FOR MASTER>*/
#define I2C_SLAVE_PORT I2C_NUM_1                               /*!<I2C CONTROLLER PORT SET TO 1 FOR SLAVE>*/
#define ESP_SLAVE_ADDR 0x76

static const char *TAG = "xRPC_example";
int driver_installed_stat = 0;

struct param
{
    esp_mqtt_client_handle_t client;
    Request *request_i2c;
}para;

/**
*   Initialize the I2C controller with given sda and scl gpio numbers passed to init function as parameters.
*   set mode to I2C MASTER, enable internal pullup resistors for both SDA and SCL.
*   Finally, clock speed is set to 100000 Hz.
*/
void i2c_init_master(int sda_gpio, int scl_gpio)
{
    i2c_config_t conf_mas;
    conf_mas.mode = I2C_MODE_MASTER;
    conf_mas.sda_io_num = sda_gpio;
    conf_mas.sda_pullup_en = GPIO_PULLUP_ENABLE; //ENABLE INTERNAL PULLUP RESISTORS FOR SDA LINE
    conf_mas.scl_io_num = scl_gpio;
    conf_mas.scl_pullup_en = GPIO_PULLUP_ENABLE; //ENABLE INTERNAL PULLUP RESISTORS FOR SCL LINE
    conf_mas.master.clk_speed = 100000;
    i2c_param_config(I2C_MASTER_PORT, &conf_mas);
    i2c_driver_install(I2C_MASTER_PORT, conf_mas.mode, 0, 0, 0);
    
    esp_err_t f_retval;// check if the I2C Controller has initialized successfully or not
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();// Creating an I2C procedure handle named cmd

    /* Start I2C communication procedure after creating link. Here, we write 7 bit slave address and wait for acknowledgement bit from slave.*/
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    /* End I2C Communication procedure*/

    f_retval = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);// Execute the procedure defined above for a duration of 1 second.
    if (f_retval != ESP_OK) {
        printf("I2C slave NOT working or wrong I2C slave address - error (%i)", f_retval);
        // LABEL
    }
    i2c_cmd_link_delete(cmd);// deleting the created procedure handle cmd
}
/**
 *        test code to read esp-i2c-sensor.
 *        First, we write the register address to BME280. (first write 7 bit sensor address, wait for 1 bit acknowledgement. Them write 1 byte of data(register address))
 *        Then we start reading bytes from BME280's register. (first write 7 bit sensor address, wait for 1 bit acknowledgement. AFTER THAT, WRITE N - 1 bytes and wait for acknowledgement.
 *        After getting acknowledgement, write last byte and wait for NACK (no response condition) to stop transmission).                
 *
 * ____________________________________________________________________________________________________________________________________________
 * | start | slave_addr + wr_bit +ack | write reg_addr byte + ack | slave_addr + rd_bit + ack|read n-1 bytes + ack | read 1 byte + nack| stop |
 * --------|--------------------------|---------------------------|--------------------------|---------------------|-------------------|------|
 *
 */
int8_t i2c_master_reg_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
    int8_t iError;
    esp_err_t esp_err;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN); //write slave address + ack
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    if(length > 1)
    {
        i2c_master_read(cmd, reg_data, length-1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, reg_data + length - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000 / portTICK_PERIOD_MS);
    if(esp_err == ESP_OK)
    {
        iError = 0;
    }
    else{
        iError = -1;
    }
    i2c_cmd_link_delete(cmd);
    return iError;
}

/**
 *        Test code to write esp-i2c-sensor
 *        Master device write data to slave(BME280),
 *        the data will be stored in slave buffer.
 *
 * _______________________________________________________________________________________
 * | start | slave_addr + wr_bit + ack |write reg_addr + ack|write n bytes + ack  | stop |
 * --------|---------------------------|--------------------|---------------------|------|
 *
 */
int8_t i2c_master_reg_write(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
    int8_t iError;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | I2C_MASTER_WRITE, true); //write slave address + check ack 
    i2c_master_write_byte(cmd, reg_addr, true); //write byte (register_address)
    i2c_master_write(cmd, reg_data, length, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000 / portTICK_RATE_MS);
    if(ret == ESP_OK){
        iError = 0;
    }
    else{
        iError = -1;
    }
    i2c_cmd_link_delete(cmd);
    return iError;
}

/**
 *      custom delay function for i2c opn
 */
void delay_ms(uint32_t period_ms, void *intf_ptr) {
    /* Implement the delay routine according to the target machine */
         ets_delay_us(period_ms * 1000);
}

void *buffer; //buffer to store incoming.

/* Function used to get time and store it into response which us a struct of type GettimeofdayResponse*/
int gettimeofday_func(void *clnt , void *request)
{
    printf("Gettime function running. \n");
    struct timeval tv;
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)clnt;
    //Request *request_gettime = (Request*)request;
    XRPCMessage toSend = X_RPCMESSAGE__INIT;
    toSend.message_type_case = X_RPCMESSAGE__MESSAGE_TYPE_RESPONSE;

    Response resp;
    toSend.response = &resp;
    response__init(toSend.response);

    toSend.response->response_func_case = RESPONSE__RESPONSE_FUNC_GETTIMEOFDAY_RESPONSE;
    GettimeofdayResponse getTimeResponse = GETTIMEOFDAY_RESPONSE__INIT;
    /* Pass values obtained from gettimeofday() to response submessage field gettimeofdayresponse
    */
    int status = gettimeofday(&tv , NULL);
    getTimeResponse.return_value = status;
    getTimeResponse.errno_alt = errno;

    /* INITIALIZING timeval field in GettimeofdayResponse, a submessage of toSend 
    */
    TimeVal tv_get = TIME_VAL__INIT;
    tv_get.tv_sec = tv.tv_sec;
    tv_get.tv_usec = tv.tv_usec;
    getTimeResponse.timeval = &tv_get;
    toSend.response->gettimeofday_response = &getTimeResponse;
    printf("Get time parameters: return_value = %d | errno_alt = %d \n", toSend.response->gettimeofday_response->return_value, toSend.response->gettimeofday_response->errno_alt);
    printf("Get time parameters: tv_sec = %d | tv_usec = %d \n", toSend.response->gettimeofday_response->timeval->tv_sec, 
    toSend.response->gettimeofday_response->timeval->tv_usec);
    
    /* INITIALIZING time_stamp field ,a submessage of toSend 
    */
    TimeVal time_st;
    toSend.time_stamp = &time_st;
    time_val__init(toSend.time_stamp);
    toSend.time_stamp->tv_sec = tv.tv_sec;
    toSend.time_stamp->tv_usec = tv.tv_usec;

    int len = x_rpcmessage__get_packed_size(&toSend);
    //Avoiding use of malloc()
    uint8_t buffer[len + 1];
    x_rpcmessage__pack(&toSend , (void*)buffer);
    int msg_id_1;
    msg_id_1 = esp_mqtt_client_publish(client,"101/xRPC_Response", (void*)buffer , len , 0, 0);
    ESP_LOGI(TAG, "Successfully published to 101/xRPC_Response, msg_id=%d", msg_id_1);


    return 0;
}
/*Function used to set time. It gets the input from request, struct of type SysRpc__SettimeofdayRequest and store response in struct of type
SysRpc__SettimeofdayResponse */
int settimeofday_func(void *clnt , void *request)
{
    printf("Settime function running. \n");
    struct timeval tv;
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)clnt;
    Request *request_settime = (Request*)request;
    TimeVal *time = request_settime->settimeofday_request->timeval;
    printf("Set time parameters: tv_sec = %d | tv_usec = %d \n", time->tv_sec, time->tv_usec);
    XRPCMessage toSend = X_RPCMESSAGE__INIT;
    toSend.message_type_case = X_RPCMESSAGE__MESSAGE_TYPE_RESPONSE;

    Response resp;
    toSend.response = &resp;
    response__init(toSend.response);

    toSend.response->response_func_case = RESPONSE__RESPONSE_FUNC_SETTIMEOFDAY_RESPONSE;
    SettimeofdayResponse setTimeResponse;
    toSend.response->settimeofday_response = &setTimeResponse;
    settimeofday_response__init(toSend.response->settimeofday_response);
    
    tv.tv_sec = time->tv_sec;
    tv.tv_usec = time->tv_usec;
    int status = settimeofday(&tv, NULL);
    toSend.response->settimeofday_response->return_value = status;
    toSend.response->settimeofday_response->errno_alt = errno;
    printf("Return Value: %d | errno_alt: %d \n", toSend.response->settimeofday_response->return_value,toSend.response->settimeofday_response->errno_alt);
    
    /* INITIALIZING time_stamp field ,a submessage of toSend 
    */
    TimeVal time_st;
    toSend.time_stamp = &time_st;
    time_val__init(toSend.time_stamp);
    toSend.time_stamp->tv_sec = time->tv_sec;
    toSend.time_stamp->tv_usec = time->tv_usec;
    
    int len = x_rpcmessage__get_packed_size(&toSend);
    //Avoiding use of malloc()
    uint8_t buffer[len + 1];
    x_rpcmessage__pack(&toSend , (void*)buffer);
    int msg_id_1;
    msg_id_1 = esp_mqtt_client_publish(client,"101/xRPC_Response", (void*)buffer , len , 0, 0);
    ESP_LOGI(TAG, "Successfully published to 101/xRPC_Response, msg_id=%d", msg_id_1);
    return status;
}
int ledcontroller_func(void *clnt, void *request)
{
    /* To configure the LED PWM Controller, we need to do the following.
        1. Configure settings in the structure led_timer_config_t. Pass a reference of this structure to ledc_timer_config().
        2. Configure settings in the structure led_channel_config_t. Pass a reference of this structure to ledc_channel_config().

        The LED PWM controller uses PWM to control the brightness of an LED. The frequency of the square wave used needs to be defined and also the resolution of 
        its duty cycle. Here, we have used a frequency 5Khz and duty resolution of 13 bits, which means that the duty cycle can be assigned wit a value in the range
        0 to 2^n-1, where n= duty resolution. Here, the resolution used is 13 bits, so the range from 0 to 8191.

        Higher the duty of PWM wave, more the brightness of the LED.
    */
    printf("Function ledcontroller_func started. \n");
    /* The structure timeval tv below is used internally by the OS to store time in terms of seconds and microseconds. THis structure has two variables in it:
    tv.sec and tv.usec, which stores time in seconds and microseconds respectively.*/
    struct timeval tv;

    //The below structure stores information about the client (ESP32)
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)clnt;

    /*XRPCMessage is a structure which represents the base message field. ALl the other message fields in the proto definition are stored as submessage types
    inside of the XRPCMessage structure in the form of pointers. */
    XRPCMessage toSend = X_RPCMESSAGE__INIT;

    /*XRPCMessage structure looks like this internally:
        struct XRPCMessage{
            TimeVal *time_stamp;
            XRPCMessage__MessageTypeCase message_type_case;
            union {
                Request *request; // This represents Request message type
                Response *response; // This represents Response message type.
            };
        };
        The union structure is used to represent the type of message: response/request.
        Since, in the proto definition; oneof message type is used to store Response and Request submessages;
        only one out of Response and Request submessage type can be declared inside of the XRPCMesaage structure.
        Hence, the message type is stored in variable message_type_case; which is an enum of type XRPCMessage__MessageTypeCase.
        typedef enum {
            X_RPCMESSAGE__MESSAGE_TYPE__NOT_SET = 0,
            X_RPCMESSAGE__MESSAGE_TYPE_REQUEST = 1,
            X_RPCMESSAGE__MESSAGE_TYPE_RESPONSE = 2
            PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(X_RPCMESSAGE__MESSAGE_TYPE)
        } XRPCMessage__MessageTypeCase;

        one of these enum values has to be set to declare whether the message is response or request.
    */
    toSend.message_type_case = X_RPCMESSAGE__MESSAGE_TYPE_RESPONSE;

    /*Now, we set the timestamp of the message.
    THis is done by initializing a message of type TimeVal, whose reference is the passed to the TimeVal time_stamp pointer inside of the XRPCMessage structure by:
    toSend.time_stamp = &time_stamp;

    The TimeVal message is represented by a structure which looks like:
    *struct  _TimeVal
    *{
    *    ProtobufCMessage base;
    *    
    *    //store time elapse since Jan 1, 1970 00:00 in seconds (tv_sec)
    *    
    *    uint32_t tv_sec;
    *    
    *    //store time elapse since Jan 1, 1970 00:00 in microseconds (tv_usec)
    *    
    *    uint32_t tv_usec;
    *}; 
    * After that, the gettimeofday function is called; and we pass the reference of the structure defined earlier (timeval tv) to get the time.
    */
    TimeVal time_st;
    toSend.time_stamp = &time_st;
    time_val__init(toSend.time_stamp);
    gettimeofday(&tv, NULL);
    toSend.time_stamp->tv_sec = tv.tv_sec;
    toSend.time_stamp->tv_usec = tv.tv_usec;
     
    //Similarly, as shown above; we will define a message of type Response and pass its reference; call the init function to initialize the message field.
    Response resp;
    toSend.response = &resp;
    response__init(toSend.response);

    toSend.response->response_func_case = RESPONSE__RESPONSE_FUNC_LEDC_CHANNEL_CONFIG_RESPONSE;// defined as ledc_channel config message
    LedcChannelConfigResponse led_resp;
    toSend.response->ledc_channel_config_response = &led_resp;
    ledc_channel_config__response__init(toSend.response->ledc_channel_config_response);

    //The request parameter in themessage definition is of void type; so it has to be casted to type Request.
    //To know more about the message structure; you can oen the messagefile.pb-c.h header file.
    LedcChannelT *ledc_channel_cfg = ((Request*)request)->ledc_channel_config_request->ledc_conf->channel;
    LedcModeT *ledc_mode_cfg = ((Request*)request)->ledc_channel_config_request->ledc_conf->speed_mode;
    LedcIntrTypeT *ledc_intr = ((Request*)request)->ledc_channel_config_request->ledc_conf->intr_type;
    
    ledc_timer_config_t ledc_timer;//Configure LED timer settings. 
    ledc_channel_config_t ledc_channel;// Configure LED Channel ->8 channels can be used.
    //For more info, check the documentation on the Espressif website mentioned in the email.
    ledc_timer.freq_hz = 5000;// Frequency set to 5000 hz
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;// Clock configuration is set to AUTO; i.e the clock to be used will be selected automatically according our requirement.
    ledc_timer.duty_resolution = LEDC_TIMER_13_BIT;//Duty Resolution is set to 13 bits.
    if(ledc_mode_cfg->ledc_speed == LEDC_MODE_T__LEDC_MODE_E__LEDC_HIGH_SPEED_MODE)//Now, set rest of the parameters according to mode: high speed or low speed
    {
        ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;// set timer mode to high speed
        ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;// set channel mode to high speed
        ledc_timer.timer_num = LEDC_TIMER_0;// use TIMER 0 for high speed purposes
        ledc_channel.timer_sel = LEDC_TIMER_0;// The channel will use TIMER 0
    }
    else if(ledc_mode_cfg->ledc_speed == LEDC_MODE_T__LEDC_MODE_E__LEDC_LOW_SPEED_MODE)
    {
        ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;// set timer mode to low speed
        ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;// set channel mode to low speed
        ledc_timer.timer_num = LEDC_TIMER_1;// Timer 1 will be configured for low speed mode
        ledc_channel.timer_sel = LEDC_TIMER_1;// The channel will use Timer 1 for low speed mode.
    }
    else
    {
        ledc_timer.speed_mode = LEDC_SPEED_MODE_MAX;// Use the max speed mode.
        ledc_channel.speed_mode = LEDC_SPEED_MODE_MAX;// Use max speed mode.
        ledc_timer.timer_num = LEDC_TIMER_0;// Timer 0 will be configured for max speed mode.
        ledc_channel.timer_sel = LEDC_TIMER_0;// The channel will use Timer 0 for max speed mode.
    }
    ledc_timer_config(&ledc_timer);// All timer settings have been configured, pass the structure to function to set the timer.

    ledc_channel.duty = ((Request*)request)->ledc_channel_config_request->ledc_conf->duty;// set the duty (time for which PWM signal is high)
    ledc_channel.gpio_num = ((Request*)request)->ledc_channel_config_request->ledc_conf->gpio_num;// set gpio output pin(2 for internal LED)
    ledc_channel.hpoint = ((Request*)request)->ledc_channel_config_request->ledc_conf->hpoint;// Read documentation for more
    switch(ledc_channel_cfg->channel)//Now, we set the channel according to user request
    {
        case 0:
            ledc_channel.channel = LEDC_CHANNEL_0;
            break;
        case 1:
            ledc_channel.channel = LEDC_CHANNEL_1;
            break;
        case 2:
            ledc_channel.channel = LEDC_CHANNEL_2;
            break; 
        case 3:
            ledc_channel.channel = LEDC_CHANNEL_3;
            break;
        case 4:
            ledc_channel.channel = LEDC_CHANNEL_4;
            break; 
        case 5:
            ledc_channel.channel = LEDC_CHANNEL_5;
            break;  
        case 6:
            ledc_channel.channel = LEDC_CHANNEL_6;
            break;
        case 7:
            ledc_channel.channel = LEDC_CHANNEL_7;
            break;
        case 8:
            ledc_channel.channel = LEDC_CHANNEL_MAX;
            break;
    }
    if(ledc_intr->intr == LEDC_INTR_TYPE_T__INTERRUPT_TYPE__LEDC_INTR_DISABLE)// Also, set interrupt type according to request
    {
        ledc_channel.intr_type = LEDC_INTR_DISABLE;
    }
    if(ledc_intr->intr == LEDC_INTR_TYPE_T__INTERRUPT_TYPE__LEDC_INTR_FADE_END)
    {
        ledc_channel.intr_type = LEDC_INTR_FADE_END;
    }
    printf("Finished setting values. \n");
    ledc_channel_config(&ledc_channel); // PAss structure to function to initialize the channel

    // ledc_fade_func_install(0);// Activate the fade function.
    // ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 8192, 3000);
    // ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_NO_WAIT);
    // vTaskDelay(3000 / portTICK_PERIOD_MS);


    toSend.response->ledc_channel_config_response->config_status = 1;

    int len = x_rpcmessage__get_packed_size(&toSend);
    uint8_t buffer[len + 1];
    x_rpcmessage__pack(&toSend , (void*)buffer);
    int msg_id_1;
    msg_id_1 = esp_mqtt_client_publish(client,"101/xRPC_Response", (void*)buffer , len , 0, 0);
    ESP_LOGI(TAG, "Successfully published to 101/xRPC_Response, msg_id=%d", msg_id_1);

    printf("Finished. \n");
    printf("Exiting... \n");

    return 0;
}

void test_i2c_rw_func(void *params)
{
    esp_mqtt_client_handle_t client = ((struct param*)params)->client;
    Request *request_i2c = ((struct param*)params)->request_i2c;
    XRPCMessage toSend = X_RPCMESSAGE__INIT;
    struct timeval tv;

    toSend.message_type_case = X_RPCMESSAGE__MESSAGE_TYPE_RESPONSE;
    Response resp;
    toSend.response = &resp;
    response__init(toSend.response);
    
    /*Setting the response message timestamp*/
    TimeVal time_st;
    toSend.time_stamp = &time_st;
    time_val__init(toSend.time_stamp);
    gettimeofday(&tv, NULL);
    toSend.time_stamp->tv_sec = tv.tv_sec;
    toSend.time_stamp->tv_usec = tv.tv_usec;    

    /*Set response message type to type I2C_RESPONSE*/
    toSend.response->response_func_case = RESPONSE__RESPONSE_FUNC_I2C_RESPONSE;
    I2cOperationsResponse i2c_resp;
    toSend.response->i2c_response = &i2c_resp;
    i2c_operations_response__init(toSend.response->i2c_response);

    /*SETTING UP MASTER AT PORT 0*/
    if(driver_installed_stat == 0)
    {
        i2c_init_master(request_i2c->i2c_request->slave_sda_gpio, request_i2c->i2c_request->slave_scl_gpio);
        driver_installed_stat = 1;
    }
 
    /*Initializing BME280 with parameters*/
    struct bme280_dev device;
    int8_t rslt = BME280_OK,rslt2;
    uint8_t dev_addr = BME280_I2C_ADDR_PRIM;
    
    device.intf_ptr = &dev_addr;
    device.intf = BME280_I2C_INTF;
    device.read = i2c_master_reg_read;//read function pointer
    device.write = i2c_master_reg_write;//write function pointer
    device.delay_us = delay_ms;//delay function pointer

    rslt = bme280_init(&device);

    uint8_t settings_sel;
	struct bme280_data comp_data;
    
	/* Recommended mode of operation: Indoor navigation. Refer to https://github.com/BoschSensortec/BME280_driver/tree/master */
	device.settings.osr_h = BME280_OVERSAMPLING_1X;
	device.settings.osr_p = BME280_OVERSAMPLING_16X;
	device.settings.osr_t = BME280_OVERSAMPLING_2X;
	device.settings.filter = BME280_FILTER_COEFF_16;
	device.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	settings_sel = BME280_OSR_PRESS_SEL;
	settings_sel |= BME280_OSR_TEMP_SEL;
	settings_sel |= BME280_OSR_HUM_SEL;
	settings_sel |= BME280_STANDBY_SEL;
	settings_sel |= BME280_FILTER_SEL;

    rslt = bme280_set_sensor_settings(settings_sel, &device);//Apply the above settings to sensor
    printf("BME280 set status: %d \n", rslt);
	rslt2 = bme280_set_sensor_mode(BME280_NORMAL_MODE, &device);//Set sensor mode to Normal operation mode
    printf("BME280 set sensor mode status: %d \n", rslt2);
    
    device.delay_us(70, device.intf_ptr);//Wait for 70ms
	rslt2 = bme280_get_sensor_data(BME280_ALL, &comp_data, &device);// Obtain readings from sensor
    printf("BME280 get_data status: %d \n", rslt);

    printf("Temp: %0.2f, Pressure:%0.2f, Humidity:%0.2f\r\n",comp_data.temperature, comp_data.pressure, comp_data.humidity);

    toSend.response->i2c_response->success = rslt2; //Stores status of operation
    toSend.response->i2c_response->temp = comp_data.temperature; //Store temperature
    toSend.response->i2c_response->rel_hum = comp_data.humidity; //Store humidity 
    toSend.response->i2c_response->pres = comp_data.pressure; //Store pressure 

    int len = x_rpcmessage__get_packed_size(&toSend);
    uint8_t buffer[len + 1];
    x_rpcmessage__pack(&toSend , (void*)buffer);
    int msg_id_1;
    msg_id_1 = esp_mqtt_client_publish(client,"101/xRPC_Response", (void*)buffer , len , 0, 0);
    ESP_LOGI(TAG, "Successfully published to 101/xRPC_Response, msg_id=%d", msg_id_1);
    
    printf("Finished. \n");
    printf("Exiting... \n");
    vTaskDelay(2000 / portTICK_PERIOD_MS);//Wait for 2 seconds
    vTaskDelete(NULL);//Delete the created task
}

int read_sensor(void *clnt, void *request)
{
    /* Passing values to structure param, which contains the values to be passed to i2c read write task*/
    para.client = (esp_mqtt_client_handle_t)clnt;
    para.request_i2c = (Request*)request;
    printf("Creating Task: \n");
    /* Creating Task to execute sesor read*/
    xTaskCreate(&test_i2c_rw_func, "R/W SENSOR", 4096, &para, 0, NULL);
    return 0;
}
int func0(void *clnt , void *request)
{
    printf("Invalid type");
    return 0;
}

typedef int (*pf)(void *clnt , void *request);
static pf xRPC_func[] = {
    func0,
    settimeofday_func, 
    gettimeofday_func,
    ledcontroller_func,
    read_sensor};




static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
        /* This event handler instance handles cases where MQTT connection is established. In our case, we would like to subscribe to the topic device_id/xRPC_Request, where device_id = 101
        *  for this program. This is used to handle incoming RPCs gettimeofday() and settimeofday() via the topic 101/xRPC_Request.
        */
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "101/xRPC_Request", 0);
            ESP_LOGI(TAG, "Successfully subscribed to msg_id=%d", msg_id);// Use this? The same will also be printed via MQTT_EVENT_SUBSCRIBED.
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        /* The below case MQTT_EVENT_DATA handles incoming MQTT EVENTS from a publisher.
        *   We can use this case to handle the incoming MQTT event, so the code to do so should be written down here. 
        */    
        case MQTT_EVENT_DATA: //PROCESSES INCOMING PUBLISH REQUEST TO TOPIC SUBSCRIBED CURRENTLY. RN subscribed to 101/xRPC_Request, process accordingly.
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);

            //Start of RPC HANDLER service below.
            size_t len = event->data_len;
            printf("Size of event: %d \n", len);
            buffer = malloc(len);
            buffer = event->data;
            //unserialize data
            XRPCMessage *recvd = x_rpcmessage__unpack(NULL, len, buffer);
            printf("Message unpacked, length of buffer: %d  \n", len);
            printf("Reached check condition \n");
            if(recvd->message_type_case == X_RPCMESSAGE__MESSAGE_TYPE_REQUEST)
            {         
                printf("TImestamp: tv_sec : %d | tv_usec: %d \n", recvd->time_stamp->tv_sec, recvd->time_stamp->tv_usec);
                printf("message index: %d \n",recvd->request->request_func_case);
                xRPC_func[recvd->request->request_func_case]((void*)client , (void*)recvd->request);
            }
            else
            {
                break;
            }
            //The below block will show the currently set time.
            time_t now;
            char strftime_buf[64];
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            ESP_LOGI(TAG, "The current date/time is: %s \n", strftime_buf);
            x_rpcmessage__free_unpacked(recvd, NULL);// Destroy the *recvd pointer after usage and free up memory
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
}
