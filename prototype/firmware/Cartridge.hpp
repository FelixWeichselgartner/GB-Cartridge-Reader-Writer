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

    void shortDelay(int);
    void digitalPinsINPUT();
    void digitalPinsOUTPUT();

public:

    void rd_wr_mreq_high();
    void rd_wr_mreq_low();

    void reset();
    Cartridge(int, int, int, int, int, int, int*);
    ~Cartridge();

    inline Byte getCartridgetype() { return this->cartridgeType; }
    inline Byte getRomsize() { return this->romSize; }
    inline Byte getRamsize() { return this->ramSize; }
    inline Word getRomBanks() { return this->romBanks; }
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

    void SerialPrintHeader();

private:

    Byte cartridgeType;
    Byte romSize;
    Word romBanks;
    Byte ramSize, ramBanks;
    Word ramEndAddress;
    char gameTitle[17];
    bool logoCheck;

    int latchPin, dataPin, clockPin, rdPin, wrPin, mreqPin;
    int dataPins[8];

    ShiftRegister74HC595 *shiftregister; 

};

#endif
