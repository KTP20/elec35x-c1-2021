//class for the LED Matrix bar graph
//cpp file

#include "mbed.h"
#include "LEDMatrix.h"
#include <iostream>
using namespace std;


// MATRIX DISPLAY SPI WITH OUTPUT ENABLE
#define SCAN_RATE_MS   50
// THE MATRIX
#define MATRIX_LATCH_PIN PB_6
#define MATRIX_OE_PIN PB_12
#define MATRIX_SPI_PIN PC_12



    
        // set chip select
        void LEDMatrix::setChipSelect(int chip)
        {
            matrix_spi_cs = chip;
        }
        
        //clears one line of the matrix
        void LEDMatrix::clearMatrix()
        {
            // sets all the LEDs to off
            setChipSelect(0);
            matrix_spi_oe = 0;
            matrix_spi.write(0x00);//COL RHS
            matrix_spi.write(0x00);//COL LHS
            matrix_spi.write(0x00);//ROW RHS
            setChipSelect(1);
        }
        //sets one column to a certain height
        void LEDMatrix::setColumn(unsigned long long i, unsigned long long j, long k)
        {
            // sets the column top hal and bottom hafl anf the row and turs that many lights on at once.
            //printf("LEDS on = %llu, %llu, %ld\n",i,j,k);

            setChipSelect(0);
            matrix_spi.write(i);//COL RHS
            matrix_spi.write(j);//COL LHS
            matrix_spi.write(k);//ROW RHS
            setChipSelect(1);
            thread_sleep_for(5);
            clearMatrix();
            
        }
        // converts the value from an in into 2 hex values , quantized to 16 values based on the high and low threshhold
        void LEDMatrix::convertValue(unsigned int sample, unsigned int low_threshold, unsigned int high_threshold, long column)
        {
            float value; 
            //cchanges the sample to be quantised to 16 levels, using the tresholds for the sample type
            int fullHexValue = 0xFFFF;
            float thresholdDifference = high_threshold-low_threshold;
            
            // if the threshold difference is less or equal to 0, then i cant do the division so i need to error out
            if(thresholdDifference <= 0)
            {
                printf("Numerical Error, cannot divide by 0 or negative\n");
                float value = 0;
            } else 
            {
                value = ((sample-low_threshold)/thresholdDifference)*16;
               
            }
            //changes the value from an int between 0 and 16 to a hex value
            value = round(value);
            int shiftAmount =  16 - value;
            // shifts 0xFFFF down to make the number of 1s in the value
            fullHexValue = fullHexValue >> shiftAmount;
            //printf("height = %u\n",fullHexValue);
            // chaninging the full hex value, to the lhs and rhs of the hex vakue for the ematrix
            int RHSHexValue = 0x00;
            int LHSHexValue = 0x00;
            if (fullHexValue < 0xFF)
            {
                LHSHexValue = fullHexValue;
            } else
            {
                RHSHexValue = fullHexValue >> 8;
                LHSHexValue = fullHexValue - (RHSHexValue << 8);
            }     
            //uses the values made and sets the column
            setColumn(RHSHexValue, LHSHexValue, column);
        } 
        //gets the sample and put it into the convert values and the set column
        void LEDMatrix::sensorType (char sensor,int Low, int High)
        {

            int sample[8];
            for(int i=0;i<7;i++)
            {
                sample[i] = GetSample();
            }
            
            for(int i=7;i>=0;i--)
            {
                LEDMatrix::convertValue(sample[i], Low, High, i);
            }
        }
        //get sample //need the get from the FIFO
        int LEDMatrix::GetSample()
        {
            // when there are actual values to get, add those in.
            int sample = rand() %16;
            //printf("Sample = %u\n",sample);
            return sample;
        }
/*
int main
{
    LEDMatrix LEDMatrix; // contrustor // instansiatate
    LEDMatrix.clearMatrix();
    LEDMatrix.setColumn(column,value);
    LEDMatrix.setChipSelect(High_or_Low);
    LEDMatrix.setOutputEnable(High_or_Low);


};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         // LED Matrix 
        SPI matrix_spi;   // MOSI, MISO, SCLK
        DigitalOut matrix_spi_cs;            //Chip Select ACTIVE LOW
        DigitalOut matrix_spi_oe;           //Output Enable ACTIVE LOW        

// MATRIX DISPLAY SPI WITH OUTPUT ENABLE
#define SCAN_RATE_MS   50

// Clear the LED matrix (code by Martin Simpson)
void clearMatrix(void)
{
    matrix_spi_cs=0;           //CLEAR Matrix
    matrix_spi.write(0x00);//COL RHS
    matrix_spi.write(0x00);//COL LHS
    matrix_spi.write(0x00);//ROX RHS
    matrix_spi_cs=1;
}

// Test the LED matrix (code by Martin Simpson)
void matrix_scan(void)
{
    for (int j=0;j<=7;j++)  //NB ROW has 3 to 8 decoder so 0 to 7 to Row Select
                            //COLUMNS(COL) are 0 to 255 to select each Byte
    {
        for (int i=1;i<255;i*=2) //Increment LEFT Hand Block of 8
        {
            matrix_spi_cs=0;           //Send Data to Matrix
            matrix_spi.write(0x00);//COL RHS
            matrix_spi.write(i);   //COL LHS
            matrix_spi.write(j);   //ROW RHS
            matrix_spi_cs=1;           //low to high will effectivelly LATCH the Shift register to output
            thread_sleep_for(SCAN_RATE_MS);
        }
        for (int i=1;i<255;i*=2) //Increment RIGHT Hand Block of 8
        {
            matrix_spi_cs=0;           //Send Data to Matrix
            matrix_spi.write(i);   //COL RHS
            matrix_spi.write(0x00);//COL LHS
            matrix_spi.write(j);   //ROW RHS
            matrix_spi_cs=1;           //low to high will effectivelly LATCH the Shift register to output
            thread_sleep_for(SCAN_RATE_MS);
        }
    }
for (int j=0;j<=7;j++)  //NB ROW has 3 to 8 decoder so 0 to 7 to Row Select
                            //COLUMNS(COL) are 0 to 255 to select each Byte
    {
        {
            matrix_spi_cs=0;           //Send Data to Matrix
            matrix_spi.write(0xFF);//COL RHS
            matrix_spi.write(0xFF);   //COL LHS
            matrix_spi.write(j);   //ROW RHS
            matrix_spi_cs=1;           //low to high will effectivelly LATCH the Shift register to output
            thread_sleep_for(SCAN_RATE_MS);
        }
    }
    for (int j=7;j>=0;j--)  //NB ROW has 3 to 8 decoder so 0 to 7 to Row Select
                            //COLUMNS(COL) are 0 to 255 to select each Byte
    {
        {
            matrix_spi_cs=0;           //Send Data to Matrix
            matrix_spi.write(0xFF);//COL RHS
            matrix_spi.write(0xFF);   //COL LHS
            matrix_spi.write(j);   //ROW RHS
            matrix_spi_cs=1;           //low to high will effectivelly LATCH the Shift register to output
            thread_sleep_for(SCAN_RATE_MS);
        }
    }
}
*/