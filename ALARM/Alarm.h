//class for the LED Matrix bar graph
// h file
#ifndef ALARM_H
#define ALARM_H

#include "mbed.h"
#include "DigitalOut.h"
#include "uop_msb.h"

#define SCAN_RATE_MS   50


using namespace uop_msb;
 void criticalErrorResetISR();

 
class Alarm
{
    private:
        int LightLowThreshold;
        int LightHighThreshold;
        
        int TempLowThreshold;
        int TempHighThreshold;

        int PresLowThreshold;
        int PresHighThreshold;

        char sensor;
        
    public:
        Alarm() {};
        void criticalError(char* errorMessage);

        //void criticalErrorResetISR();

        void thresholdErrorOutput();

        void setThreshold(char measurement, int LowThreshold, int HighThreshold);
 
        void getThreshold(char measurement, int* Low, int* High);

        int GetSample();

        int ThresholdCheck(int value,char measurement);

        void sensorType (char sensor);

};
#endif