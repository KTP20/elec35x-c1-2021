/*
 * Copyright (c) 2020 Arm Limited
 * SPDX-License-Identifier: Apache-2.0
 */

// #include "mbed.h"
//  #include "uop_msb.h"
// #include "rtos/ThisThread.h"
// #include "azure_c_shared_utility/xlogging.h"
// #include <cstring>
// #include <string.h>
// #include "LEDMatrix.h"
#include "iotcenter.cpp"

// using namespace uop_msb;

// extern void azureDemo();
// extern NetworkInterface *_defaultSystemNetwork;

// bool connect()
// {
//     LogInfo("Connecting to the network");

//     _defaultSystemNetwork = NetworkInterface::get_default_instance();
//     if (_defaultSystemNetwork == nullptr) {
//         LogError("No network interface found");
//         return false;
//     }

//     int ret = _defaultSystemNetwork->connect();
//     if (ret != 0) {
//         LogError("Connection error: %d", ret);
//         return false;
//     }
//     LogInfo("Connection success, MAC: %s", _defaultSystemNetwork->get_mac_address());
//     return true;
// }

// bool setTime()
// {
//     LogInfo("Getting time from the NTP server");

//     NTPClient ntp(_defaultSystemNetwork);
//     ntp.set_server("time.google.com", 123);
//     time_t timestamp = ntp.get_timestamp();
//     if (timestamp < 0) {
//         LogError("Failed to get the current time, error: %ud", timestamp);
//         return false;
//     }
//     LogInfo("Time: %s", ctime(&timestamp));
//     set_time(timestamp);
//     return true;
// }



int main() {

    //EDMatrix Graph;
    
    //InitAzure();
    while(1)
    {
        // char type = 'T';
        // Graph.sensorType(type);
        // thread_sleep_for(500000);
        
    }
    return 0;
}
