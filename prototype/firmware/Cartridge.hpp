#ifndef CARTRIDGE_HPP_
#define CARTRIDGE_HPP_

#include "datatypes.h"
#include <ShiftRegister74HC595.h>

class Cartridge {

private:

    void wrPin_high();
    void wrPin_low();
    void mreqPin_high();
    void mreqPin_low();
    void rdPin_high();
    void rdPin_low();
    void shortDelay();
    void digitalPinsINPUT();
    void digitalPinsOUTPUT();

public:

    Cartridge(int, int, int, int, int, int, int*);
    ~Cartridge();

    inline Byte getCartridgetype() { return this->cartridgeType; }
    inline Byte getRomsize() { return this->romSize; }
    inline Byte getRamsize() { return this->ramSize; }
    inline Byte getRombanks() { return this->romBanks; }
    inline char* getGametitle() { return this->gameTitle; }

    Byte ReadByte(Word);
    void WriteByte(Word, Byte);
    void ShiftOutAddress(Word);

    void ReadROM();
    void ReadRAM();
    void WriteRAM();

    void ReadHeader();
    void DumpROM();
    void DumpRAM();
    void UploadRAM();

private:

    Byte cartridgeType;
    Byte romSize, romBanks;
    Byte ramSize, ramBanks;
    char gameTitle[17];

    int latchPin, dataPin, clockPin, rdPin, wrPin, mreqPin;
    int dataPins[8];

    ShiftRegister74HC595 *shiftregister; 

};

#endif