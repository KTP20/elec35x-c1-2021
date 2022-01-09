//class for the LED Matrix bar graph
// h file
#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include "mbed.h"
#define SCAN_RATE_MS   50

class LEDMatrix
{
    private:
        // LED Matrix 
        SPI matrix_spi;                     // MOSI, MISO, SCLK
        DigitalOut matrix_spi_cs;           //Chip Select ACTIVE LOW
        DigitalOut matrix_spi_oe;           //Output Enable ACTIVE LOW  
        void setChipSelect(int chip);
        void clearMatrix();
        void setColumn(unsigned long long i, unsigned long long j, long k);
        void convertValue(unsigned int sample, unsigned int low_threshold, unsigned int high_threshold, long column);
        int GetSample();
        
    public:
        //constructor does a sweep of the LEDs for no reason, i wanted to
        LEDMatrix(): matrix_spi(PC_12,PC_11,PC_10), matrix_spi_cs(PB_6), matrix_spi_oe(PB_12)
        {
       
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
            matrix_spi_cs=0;
            matrix_spi.write(0x00);//COL RHS
            matrix_spi.write(0x00);//COL LHS
            matrix_spi.write(0x00);//ROW RHS
            matrix_spi_cs=1;  
    }

    void sensorType (char sensor,int Low, int High);
        
};
#endif