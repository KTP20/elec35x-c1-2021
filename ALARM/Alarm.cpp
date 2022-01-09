//get sample for the 


//class for the LED Matrix bar graph
//cpp file

#include "mbed.h"
#include "Alarm.h"
#include <iostream>
#include "azure_c_shared_utility/xlogging.h"

using namespace std;
//timer and LEd for Critical erorr
Timeout critTimer;
DigitalOut ledRed(LED3);
//interrupt and led for threshold error
DigitalOut ledGreen(LED1);
DigitalOut ledBlue(LED2);
InterruptIn blueButton(USER_BUTTON);
Timeout ThresholdTimer;

EventQueue PrintQueueAlarm;
        void criticalErrorResetISR()
        {
            CriticalSectionLock::enable();
            //system reset
            PrintQueueAlarm.call(printf,"System Reset");
            //send to the event queue
            ledRed = 0;
            CriticalSectionLock::disable();
        }

        void ThresholdTimeoutISR()
        {
            CriticalSectionLock::enable();
            ledBlue =0;
            PrintQueueAlarm.call(printf,"timeout finished for threshold alarm");
            CriticalSectionLock::disable();
        }
        
        void ThresholdErrorISR()
        {
            CriticalSectionLock::enable();
            ledGreen = 0;
            ThresholdTimer.attach(&ThresholdTimeoutISR,60s);
            ledBlue = 1;
            CriticalSectionLock::disable();
        }
        void Alarm::thresholdErrorOutput()
        {
            LogError("Measurements outside threshold");
            ledGreen = 1;
            blueButton.rise(&ThresholdErrorISR);

        }

        void Alarm::criticalError(char* errorMessage)
        {
            critTimer.attach(&criticalErrorResetISR, 30s);
            LogError("Critical Error, %s",errorMessage);
            ledRed = 1;
        }

        //input,T,L,P for temp, light and pressure. and add in the low and high threshould that you want to set.
        void Alarm::setThreshold(char measurement, int LowThreshold, int HighThreshold)
        {
            switch(measurement)
            {
                case 'T':
                    TempLowThreshold = LowThreshold;
                    TempHighThreshold = HighThreshold;
                    break;
                case 'L':
                    LightLowThreshold = LowThreshold;
                    LightHighThreshold = HighThreshold;
                    break;
                case 'P':
                    PresLowThreshold = LowThreshold;
                    PresHighThreshold = HighThreshold;
                    break;  
                default:
                    LowThreshold = 0;
                    HighThreshold = 16;
                    break;
            }
            // printf("Temp LOW:%d, HIGH:%d\t",TempLowThreshold ,TempHighThreshold);
            // printf("Light LOW:%d, HIGH:%d\t",LightLowThreshold, LightHighThreshold);
            // printf("Pres LOW:%d, HIGH:%d\t\n", PresLowThreshold, PresHighThreshold);
        }

        void Alarm::getThreshold(char measurement, int* Low, int* High)
        {
            int LowThreshold;
            int HighThreshold;
            switch(measurement)
            {
                case 'T':
                    LowThreshold = TempLowThreshold;
                    HighThreshold = TempHighThreshold;
                    break;
                case 'L':
                    LowThreshold = LightLowThreshold;
                    HighThreshold = LightHighThreshold;
                    break;
                case 'P':
                    LowThreshold = PresLowThreshold;
                    HighThreshold = PresHighThreshold;
                    break;  
                default:
                    LowThreshold = 0;
                    HighThreshold = 16;
                    break;
            }
            *Low = LowThreshold;
            *High = HighThreshold;
        } 

        int Alarm::ThresholdCheck(int value,char measurement)
        {
            int LowThreshold;
            int HighThreshold;
            switch(measurement)
            {
                case 'T':
                    LowThreshold = TempLowThreshold;
                    HighThreshold = TempHighThreshold;
                    break;
                case 'L':
                    LowThreshold = LightLowThreshold;
                    HighThreshold = LightHighThreshold;
                    break;
                case 'P':
                    LowThreshold = PresLowThreshold;
                    HighThreshold = PresHighThreshold;
                    break;  
                default:
                    LowThreshold = 0;
                    HighThreshold = 16;
                    break;
            }
            if ((value > HighThreshold) || (value < LowThreshold))
            {
                Alarm::thresholdErrorOutput();
                return 0;
            } else {
                return 1;
            }
        }

        int Alarm::GetSample()
        {
            // when there are actual values to get, add those in.
            int sample = rand() %16;
            printf("Sample = %u\n",sample);

            // test sample against thresholds
            return sample;
        }
