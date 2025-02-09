#ifndef AZUREIOT_HPP
#define AZUREIOT_HPP

#include "mbed.h"
//#include "rtos/ThisThread.h"
//#include "NTPClient.h"

#include "certs.h"
#include "iothub.h"
#include "iothub_client_options.h"
#include "iothub_device_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/tickcounter.h"
//#include "azure_c_shared_utility/xlogging.h"

#include "iothubtransportmqtt.h"
//#include "azure_cloud_credentials.h"
#include <cstring>
#include <string.h>


static void on_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context);
static IOTHUBMESSAGE_DISPOSITION_RESULT on_message_received(IOTHUB_MESSAGE_HANDLE message, void* user_context);
static void on_message_sent(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback);
void RedLEDToggle(const char* payload, size_t size);
void GreenLEDToggle(const char* payload, size_t size);
int SetThreshold(const char* payload, int size, int* temp, int* light, int* pressure);
void Plot(const char* payload, int size, bool* success);
static int on_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback);
void cleanup();
void IoTInitial();
void SendMessage();
int StartConnection();

#endif