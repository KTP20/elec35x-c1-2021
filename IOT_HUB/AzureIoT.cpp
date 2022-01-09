/*
 * Copyright (c) 2020 Arm Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "rtos/ThisThread.h"
#include "NTPClient.h"

#include "certs.h"
#include "iothub.h"
#include "iothub_client_options.h"
#include "iothub_device_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/xlogging.h"

#include "iothubtransportmqtt.h"
// #include "azure_cloud_credentials.h"
#include <cstring>
#include <string.h>

#include "Alarm.h"
/**
 * This example sends and receives messages to and from Azure IoT Hub.
 * The API usages are based on Azure SDK's official iothub_convenience_sample.
 */

// Global symbol referenced by the Azure SDK's port for Mbed OS, via "extern"
EventQueue PrintQueueIoT;

static bool message_received = false;
int temp[] ={0,0};
int light[] = {0,0};
int pressure[] = {0,0};
char RESPONSE_STRING[] = "Failue to send command";
char iothub_connection_string[] = "HostName=iotc-fbab197f-edaf-440b-a90a-fff2e43d3ec1.azure-devices.net;DeviceId=fvgk75mov7;SharedAccessKey=mcmDDGOzTkINwERiEUOgWeMQjD7gT1rSVu+WeGTYK6o=";
DigitalOut ledRedTraf(TRAF_RED1_PIN);
char Letter = 'A';

//outputs the connection status
static void on_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED) {
        LogInfo("Connected to IoT Hub");
    } else {
        LogError("Connection failed, reason: %s", MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, reason));
    }
}

// **************************************
// * MESSAGE HANDLER (no response sent) *
// **************************************

static IOTHUBMESSAGE_DISPOSITION_RESULT on_message_received(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
    LogInfo("Message received from IoT Hub");

    const unsigned char *data_ptr;
    size_t len;
    if (IoTHubMessage_GetByteArray(message, &data_ptr, &len) != IOTHUB_MESSAGE_OK) {
        LogError("Failed to extract message data, please try again on IoT Hub");
        return IOTHUBMESSAGE_ABANDONED;
    }

    message_received = true;
    LogInfo("Message body: %.*s", len, data_ptr);

    if (strncmp("true", (const char*)data_ptr, len) == 0) {
        ledRedTraf = 1;
    } else {
        ledRedTraf = 0;
    }

    return IOTHUBMESSAGE_ACCEPTED;
}

//message sent status
static void on_message_sent(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    if (result == IOTHUB_CLIENT_CONFIRMATION_OK) {
        LogInfo("Message sent successfully");
    } else {
        LogInfo("Failed to send message, error: %s",
            MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    }
}

// ****************************************************
// * COMMAND HANDLER (sends a response back to Azure) *
// ****************************************************

//DigitalOut led1(LED1);
//DigitalOut led3(LED3);
// DigitalIn blueButton(USER_BUTTON);


// void RedLEDToggle(const char* payload, size_t size)
// {
//     if (strncmp("true", (const char*)payload, size) == 0 ) {
//         printf("LED3 ON\n");
//         led3 = 1;
//     } else {
//         printf("LED3 OFF\n");
//         led3 = 0;
//     }
// }

// void GreenLEDToggle(const char* payload, size_t size)
// {
//     if ( strncmp("true", (const char*)payload, size) == 0 ) {
//         printf("LED1 ON\n");
//         led1 = 1;
//     } else {
//         printf("LED1 OFF\n");
//         led1 = 0;
//     }
// }

//set the htreshold for the temp, light, pressure
int SetThreshold(const char* payload, int size, int* temp, int* light, int* pressure)
{
    //temporary counters and placeholder arrays 
    int j = 0;
    int l = 0;
    char xtemp[10];
    char ytemp[10];
    char ztemp[10];

    //goes through the payload characters to get out the numbers
    // all the x y and z
    for (int i=0;i<2;i++)
    {
        int startOfPhrase = j;
        //checks through the characters, and for the colon, start of the numbers
        while(payload[l] != ':')
        {
            l++;
        }
        //looks for thecomma, end of the number 
        while(payload[j] != ','){
            j++;
            if(j>size){
                break;
            } 
        }
        int counter = 0;
        //adds all the numbers into the temp array
        for(int k=l+1;k<j;k++)
        {   
            //looks through temp, light, pressure.
            switch(i){
            case 0: //temp, x
                xtemp[counter] = payload[k];
            break;
            case 1: //light, y
                ytemp[counter] = payload[k];
            break;
            case 2: //pressure, z
                ztemp[counter] = payload[k];
            break;
            default:
                PrintQueueIoT.call(printf,"Error");
            break; 
            }
            counter++;
        }
        j++;
        l++;
    }
    //does the same for the last  number.
    while(payload[l] != ':')
    {
        l++;
    }
    //looks for the end of the string
    while(payload[j] != '}'){
        j++;
        if(j>size){
            break;
        } 
    }
    int counter = 0;
    //adds the number to the pressure temp
    for(int k=l+1;k<j;k++)
        {
            ztemp[counter] = payload[k];
            counter++;
        }
    //turns the arrzay into a number
    int x = std::atoi(xtemp);
    int y = std::atoi(ytemp);
    int z = std::atoi(ztemp);
    
    //prints out the x,y,z
    PrintQueueIoT.call(printf,"x = %d, y = %d, z = %d\n",x,y,z);
    //adds them the poiinters to get them out of the function.
    *temp = x;
    *light = y;
    *pressure = z;

    return 1;
} 

// outputs the different sensor data, for the LED matrix
void Plot(const char* payload, int size, bool* success,char* Letter)
{
   char letter;
    bool Recieved =false;
    //checks which letter is inputted
        if(strncmp("\"T\"", (const char*)payload, size) == 0 ){
            PrintQueueIoT.call(printf,"Temperature output");
            //assign Temperature to the LED Matrix
            //maybe a message or a function call
            Recieved = true;
            letter = 'T';
        }
        else if(strncmp("\"P\"", (const char*)payload, size) == 0 ){
            PrintQueueIoT.call(printf,"Pressure Output");
            //same applies
            //TBC
            Recieved = true;
            letter = 'P';
        }
        else if(strncmp("\"L\"", (const char*)payload, size) == 0 ){
            PrintQueueIoT.call(printf,"Light output");
            Recieved = true;
            letter = 'L';
        } 
        //error if a T,L or P not put in.
        else{
            PrintQueueIoT.call(printf,"Error in the Plot function, invalid Character");
            Recieved = false;
            letter = 'A';
        }
    *success = Recieved;
    *Letter = letter;
}

void latest(const char* payload, int size)
{
    //Fetch Light level, temp and pressure. 
    //add in the date and time of the fetch and send off to the IoT as a response
    if ( strncmp("true", (const char*)payload, size) == 0 ) {
        PrintQueueIoT.call(printf,"Latest = TRUE\n");
    } else {
        PrintQueueIoT.call(printf,"Latest = FALSE\n");
    }
}

void buffered(const char* payload, int size)
{
    //fetch size of buffer
    // send off to the IoT as a response
    if ( strncmp("true", (const char*)payload, size) == 0 ) {
        PrintQueueIoT.call(printf,"Buffered = TRUE\n");
    } else {
        PrintQueueIoT.call(printf,"Buffered = FALSE\n");
    }
}

void flush(const char* payload, int size)
{
    //send command to flush buffer
    //send off to the IoT acknoledgement
    if ( strncmp("true", (const char*)payload, size) == 0 ) {
        PrintQueueIoT.call(printf,"Flush = TRUE\n");
    } else {
        PrintQueueIoT.call(printf,"Flush = FALSE\n");
    }
}

// message and response
static int on_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback)
{
    Alarm ThresholdAlarm;
    // checks the method name and the call the right function 
    const char* device_id = (const char*)userContextCallback;
    const char * input_string = (const char *) payload;
    char RESPONSE_STRING[] = "\"Failue to send command\"";
    bool messageSent = 1;
    PrintQueueIoT.call(printf,"\r\nDevice Method called for device %s\r\n", device_id);
    PrintQueueIoT.call(printf,"Device Method name:    %s\r\n", method_name);
    PrintQueueIoT.call(printf,"Device Method payload: %.*s\r\n", (int)size, input_string);

    // if (strncmp("ToggleLED",(const char*)method_name, 11) == 0) {
    //     GreenLEDToggle((const char*)payload, (size_t)size);
    //     strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");
    // }
    // else if (strncmp("ToggleLED3",(const char*)method_name, 11) == 0) {
    //     RedLEDToggle(input_string,size);
    //     strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");
    // } 
    // else if(strncmp("TestValue",(const char*)method_name, 11) == 0) {
    //     LcdValue(input_string);
    //     strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");
    // }
    
    if(strncmp("SetLowVector",(const char*)method_name, 12) == 0)
    {
        SetThreshold(input_string,size,&(temp[0]),&(light[0]),&(pressure[0]));
        PrintQueueIoT.call(printf,"low threshold: temp is %d, light is %d, pressure is %d",temp[0], light[0],pressure[0]);
        strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");

    } 
    else if(strncmp("SetHighVector",(const char*)method_name, 13) == 0)
    {
        SetThreshold(input_string,size,&(temp[1]),&(light[1]),&(pressure[1]));
        PrintQueueIoT.call(printf,"highthreshold: temp is %d, light is %d, pressure is %d\n",temp[1], light[1],pressure[1]);
        strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");

    } 
    else if (strncmp("Plot",(const char*)method_name, 13) == 0)
    {
        bool success = false;
        PrintQueueIoT.call(printf,"PLOT payload is %s\t size is %u\n",payload,size);
        Plot(input_string,size,&success,&Letter);
        if (success == true){
            strcpy(RESPONSE_STRING, "{ \"Response\" : \"Commmand Recieved\" }");
        } else 
        {
            strcpy(RESPONSE_STRING, "{ \"Response\" : \"Invalid Character\" }");
        }
    }else if(strncmp("Latest",(const char*)method_name, 13) == 0)
    {
        latest(input_string,size);
        PrintQueueIoT.call(printf,"Latest done");
        strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved. \t Latest data is ...\" }");
    } else if(strncmp("Buffered",(const char*)method_name, 13) == 0)
    {
        buffered(input_string,size);
        PrintQueueIoT.call(printf,"Buffered done");
        strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved. \t Buffer Size is ...\" }");
    } else if(strncmp("Flush",(const char*)method_name, 13) == 0)
    {
        flush(input_string,size);
        PrintQueueIoT.call(printf,"Flush done");
        strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");
    } 
    else 
    {
        PrintQueueIoT.call(printf,"error somewhere lol");
        strcpy(RESPONSE_STRING, "{ \"Responce\" : \"Failue to send command\" }");
    }

    int status = 200;
   
    PrintQueueIoT.call(printf,"\r\nResponse status: %d\r\n", status);
    PrintQueueIoT.call(printf,"Response payload: %s\r\n\r\n", RESPONSE_STRING);

    int rlen = strlen(RESPONSE_STRING);
    *response_size = rlen;
    if ((*response = (unsigned char*)malloc(rlen)) == NULL) {
        status = -1;
    }
    else {
        memcpy(*response, RESPONSE_STRING, *response_size);
    }
    PrintQueueIoT.call(printf,"\r\nResponse status: %d\r\n", status);
    return status;

    ThresholdAlarm.setThreshold('T', temp[0], temp[1]);
    ThresholdAlarm.setThreshold('L', light[0], light[1]);
    ThresholdAlarm.setThreshold('P', pressure[0], pressure[1]);
}
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// static int on_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback)
// {
//     const char* device_id = (const char*)userContextCallback;

//     printf("\r\nDevice Method called for device %s\r\n", device_id);
//     printf("Device Method name:    %s\r\n", method_name);
//     printf("Device Method payload: %.*s\r\n", (int)size, (const char*)payload);

//     if ( strncmp("true", (const char*)payload, size) == 0 ) {
//         printf("LED ON\n");
//         led1 = 1;
//     } else {
//         printf("LED OFF\n");
//         led1 = 0;
//     }

//     int status = 200;
//     //char RESPONSE_STRING[] = "{ \"Response\": \"This is the response from the device\" }";
//     char RESPONSE_STRING[64];
//     sprintf(RESPONSE_STRING, "{ \"Response\" : %d }", blueButton.read());

//     printf("\r\nResponse status: %d\r\n", status);
//     printf("Response payload: %s\r\n\r\n", RESPONSE_STRING);

//     int rlen = strlen(RESPONSE_STRING);
//     *response_size = rlen;
//     if ((*response = (unsigned char*)malloc(rlen)) == NULL) {
//         status = -1;
//     }
//     else {
//         memcpy(*response, RESPONSE_STRING, *response_size);
//     }
//     return status;
// }
// going through the functions, might need to change the ending for the new things
void IoTInitial() {
    bool trace_on = MBED_CONF_APP_IOTHUB_CLIENT_TRACE;
    tickcounter_ms_t interval = 100;
    IOTHUB_CLIENT_RESULT res;

    LogInfo("Initializing IoT Hub client");
    IoTHub_Init();

    IOTHUB_DEVICE_CLIENT_HANDLE client_handle = IoTHubDeviceClient_CreateFromConnectionString(
        iothub_connection_string,
        MQTT_Protocol
    );
    if (client_handle == nullptr) {
        LogError("Failed to create IoT Hub client handle");
        goto cleanup;
    }

    // Enable SDK tracing
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_LOG_TRACE, &trace_on);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to enable IoT Hub client tracing, error: %d", res);
        goto cleanup;
    }

    // Enable static CA Certificates defined in the SDK
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_TRUSTED_CERT, certificates);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set trusted certificates, error: %d", res);
        goto cleanup;
    }

    // Process communication every 100ms
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_DO_WORK_FREQUENCY_IN_MS, &interval);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set communication process frequency, error: %d", res);
        goto cleanup;
    }

    // set incoming message callback
    res = IoTHubDeviceClient_SetMessageCallback(client_handle, on_message_received, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set message callback, error: %d", res);
        goto cleanup;
    }

    // Set incoming command callback
    res = IoTHubDeviceClient_SetDeviceMethodCallback(client_handle, on_method_callback, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set method callback, error: %d", res);
        goto cleanup;
    }

    // Set connection/disconnection callback
    res = IoTHubDeviceClient_SetConnectionStatusCallback(client_handle, on_connection_status, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set connection status callback, error: %d", res);
        goto cleanup;
    }

    // Send ten message to the cloud (one per second)
    // or until we receive a message from the cloud
    IOTHUB_MESSAGE_HANDLE message_handle;
    char message[80];
    for (int i = 0; i < 10; ++i) {
        if (message_received) {
            // If we have received a message from the cloud, don't send more messeges
            break;
        }
        //Send data in this format:
        /*
            {
                "LightLevel" : 0.12,
                "Temperature" : 36.0
            }
        */
        double light = (float) i;
        double temp  = (float)36.0f-0.1*(float)i;
        double pressure = (float) i+2;
        sprintf(message, "{ \"LightLevel\" : %5.2f, \"Temperature\" : %5.2f, \"Pressure\" : %5.2f }", light, temp,pressure);
        LogInfo("Sending: \"%s\"", message);

        message_handle = IoTHubMessage_CreateFromString(message);
        if (message_handle == nullptr) {
            LogError("Failed to create message");
            goto cleanup;
        }

        res = IoTHubDeviceClient_SendEventAsync(client_handle, message_handle, on_message_sent, nullptr);
        IoTHubMessage_Destroy(message_handle); // message already copied into the SDK

        if (res != IOTHUB_CLIENT_OK) {
            LogError("Failed to send message event, error: %d", res);
            goto cleanup;
        }

        ThisThread::sleep_for(60s);
    }

    // If the user didn't manage to send a cloud-to-device message earlier,
    // let's wait until we receive one
    while (!message_received) {
        // Continue to receive messages in the communication thread
        // which is internally created and maintained by the Azure SDK.
        sleep();
    }

cleanup:
    IoTHubDeviceClient_Destroy(client_handle);
    IoTHub_Deinit();
}

NetworkInterface *_defaultSystemNetwork;
// connecting to the IoT and get the date and time
int StartConnection() 
{
    PrintQueueIoT.call(printf,"IoT, Everthing is fine \n");
    LogInfo("Connecting to the network\n");

    _defaultSystemNetwork = NetworkInterface::get_default_instance();
    if (_defaultSystemNetwork == nullptr) {
        LogError("No network interface found");
        return -1;
    }

    int ret = _defaultSystemNetwork->connect();
    if (ret != 0) {
        LogError("Connection error: %d", ret);
        return -1;
    }
    LogInfo("Connection success, MAC: %s", _defaultSystemNetwork->get_mac_address());

    LogInfo("Getting time from the NTP server");

    NTPClient ntp(_defaultSystemNetwork);
    ntp.set_server("time.google.com", 123);
    time_t timestamp = ntp.get_timestamp();
    if (timestamp < 0) {
        LogError("Failed to get the current time, error: %u", timestamp);
        return -1;
    }
    LogInfo("Time: %s", ctime(&timestamp));
    set_time(timestamp);

    LogInfo("Starting");
    IoTInitial();
    

    return 0;
}