#ifndef GPIO_HPP_
#define GPIO_HPP_

#include "Arduino.h"
#include <ShiftRegister74HC595.h>
#include "datatypes.h"

class GPIO {

public:

    GPIO();

    void setAddress(Word);
    Byte getByte(Word);
    Byte getRAMByte(Word);
    void setByte(Word, Byte);

    void wr_low();
    void wr_high();
    void rd_low();
    void rd_high();
    void cs_low();
    void cs_high();

    void dataInput();
    void dataOutput();

private:

    // ShiftRegisters
    int latchPin = 10;
    int dataPin = 11;
    int clockPin = 12;
    // Cartridge
    int rdPin = A1;
    int wrPin = 13;
    int csPin = A4;
    int dataPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};

    ShiftRegister74HC595 *shiftregister;

};

#endif
