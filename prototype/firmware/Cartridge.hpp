#ifndef CARTRIDGE_HPP_
#define CARTRIDGE_HPP_

#include "Arduino.h"
#include "GPIO.hpp"

class Cartridge {

private:

public:

    void reset();
    Cartridge();
    ~Cartridge();

    void ReadHeader();
    void DumpROM();
    void DumpRAM();
    void UploadRAM();

    void SerialPrintHeader();

private:

    GPIO gpio;

    Byte cartridgeType;
    Byte romSize;
    Word romBanks;
    Byte ramSize, ramBanks;
    Word ramEndAddress;
    char gameTitle[17];
    bool logoCheck;

};

#endif
