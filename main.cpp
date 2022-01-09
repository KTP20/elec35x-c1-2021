
#include "mbed.h"
#include "uop_msb.h"
#include "rtos/ThisThread.h"
#include "azure_c_shared_utility/xlogging.h"
#include <cstring>
#include <string.h>
#include <chrono>
#include <iterator>
#include "LEDMatrix.h"
#include "Alarm.h"
#include "iotcenter.cpp"
#include "AzureIoT.hpp"
#include "mbed_events.h"
// DigitalOut ledGreen(LED1);
// DigitalOut ledRed(LED3);

int flagRedLED = 0;
Thread t1,t2,t3,t4;
Alarm TestAlarm;
LEDMatrix Graph;

char errorMessage[]= "critical fail";
void workThread();
void MatrixThread();
void AlarmThread();
void IoTThread();
void PrintThread();

//queue
EventQueue PrintQueue;
extern EventQueue PrintQueueIoT;

int main() {

    TestAlarm.setThreshold('T',20,50);
    TestAlarm.setThreshold('L', 3, 6);
    TestAlarm.setThreshold('P', 1, 36);
   
    //InitAzure();
    t1.start(AlarmThread);
    t2.start(MatrixThread);
    t3.start(IoTThread);
    t4.start(PrintThread);
    t2.set_priority(osPriorityRealtime);
    t1.join();
    t2.join();  
    t3.join();

}
void AlarmThread()
{
    int LowThresh;
    int HighThresh;
    int tempArray[10] = {3,6,9,1,12,4,13,14,10,24};
    char Types[3] = {'T','L','P'};
    for(int j=0;j<3;j++)
    {
        for(int i = 0;i<10;i++)
        {
            //PrintQueue.call(printf,"TestPrint \n");
            int tempValue = tempArray[i]; 
            TestAlarm.ThresholdCheck(tempValue,Types[j]);
            //line that generates random data
            //int sample = rand() %(20-1 + 1) + 1;
            //
            TestAlarm.getThreshold(Types[j], &LowThresh, &HighThresh);
            if(tempValue == 1)
            {
                TestAlarm.criticalError(errorMessage);
            }  
            else
            {
                PrintQueue.call(printf,"Everthing is fine \n");
            }
            
            //PrintQueue.call(printf,"sample = %d\n",sample);
            thread_sleep_for(5000);
        }
    }
}
void MatrixThread()
{
    char Types[3] = {'T','L','P'};
    while(1){
        for(int i = 0;i<3;i++)
        {   
            int LowThresh;
            int HighThresh;
            TestAlarm.getThreshold(Types[i], &LowThresh, &HighThresh);
            Graph.sensorType(Types[i],LowThresh,HighThresh);
            //PrintQueue.call(printf,"Matrix Test Print \n");
        }
    }
}
void IoTThread()
{
    StartConnection();
}

void PrintThread()
{
    PrintQueueIoT.chain(&PrintQueue);
    PrintQueue.dispatch_forever();
}

