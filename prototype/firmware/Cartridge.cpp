#include "Cartridge.hpp"

Cartridge::Cartridge(
    int dataPin, 
    int clockPin,
    int latchPin,  
    int rdPin, 
    int wrPin, 
    int mreqPin, 
    int* dataPins) { 
    
    this->dataPin = dataPin;
    this->clockPin = clockPin;
    this->latchPin = latchPin;
    this->rdPin = rdPin;
    this->wrPin = wrPin;
    this->mreqPin = mreqPin;

    for (int i = 0; i < 8; i++) {
        this->dataPins[i] = dataPins[i];
    }

    this->shiftregister = new ShiftRegister74HC595(2, dataPin, clockPin, latchPin)
}

//----------------------------------------------------------------------------------------------

void Cartridge::wrPin_high() {
    digitalWrite(wrPin, HIGH);
}

void Cartridge::wrPin_low() {
    digitalWrite(wrPin, LOW);
}

void Cartridge::mreqPin_high() {
    digitalWrite(mreqPin, HIGH);
}

void Cartridge::mreqPin_low() {
    digitalWrite(mreqPin, LOW);
}

void Cartridge::rdPin_high() {
    digitalWrite(rdPin, HIGH);
}

void Cartridge::rdPin_low() {
    digitalWrite(rdPin, LOW);
}

void Cartridge::shortDelay(int amount) {
    for (int i = 0; i < amount; i++) {
        asm volatile("nop");
    }
}

void Cartridge::digitalPinsINPUT() {
  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], INPUT);
  }
}

void Cartridge::digitalPinsOUTPUT() {
  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], OUTPUT);
  }
}

//----------------------------------------------------------------------------------------------

Byte Cartridge::ReadByte(Word address) { 
    byte val = 0x00;
    shiftoutAddress(address); // Shift out address

    mreqPin_low();
    rdPin_low();

    // Delay a little (minimum is 2 (not for me) nops, using 10 to be sure)
    shortDelay(10);
    
    for (int i = 0; i < 8; i++) {
        if (digitalRead(dataPins[i])) {
            val |= (0x01 << i);
        }
    }
    
    rdPin_high();
    mreqPin_high();
    shortDelay(10);
    
    return val;
}

void Cartridge::WriteByte(Word address, Byte data) {
    digitalPinsOUTPUT();

    ShiftOutAddress(address);

    for (int i = 0; i < 8; i++) {
        digitalWrite(dataPins[i], data & (0x01 << i));
    }

    wrPin_low();
    shortDelay(10);
    wrPin_high();
    shortDelay(10);

    digitalPinsINPUT();
}

void Cartridge::ShiftOutAddress(Word address) {
    for (int i = 0; i < 16; i++) {
        shiftregister.setNoUpdate(i, ((address & 0xFFFF) & (1 << i)) == (1 << i));
    }
    shiftregister.updateRegisters();
    shortDelay(10);
}

void Cartridge::ReadHeader() {
    // Read Cartridge Header
    for (int addr = 0x0134; addr <= 0x143; addr++) {
        this->gameTitle[(addr-0x0134)] = (char) readByte(addr);
    }
    this->gameTitle[16] = '\0';

    this->cartridgeType = readByte(0x0147);
    this->romSize = readByte(0x0148);
    this->ramSize = readByte(0x0149);
    this->romBanks = 2; // Default 32K
    this->ramBanks = 1; // Default 8K RAM

    switch(this->romSize) {
        case 1: this->romBanks = 4; break;
        case 2: this->romBanks = 8; break;
        case 3: this->romBanks = 16; break;
        case 4: this->romBanks = 32; break;
        case 5: 
            if (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3) {
                this->romBanks = 63;
            } else {
                this->romBanks = 64;
            }
            break;
        case 6: 
            if (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3) {
                this->romBanks = 125;
            } else {
                this->romBanks = 128;
            }
            break;
        
        case 7: this->romBanks = 256; break;
        case 82: this->romBanks = 72; break;
        case 83: this->romBanks = 80; break;
        case 84: this->romBanks = 96; break;
    }

    switch(this->ramSize) {
        case 3: this->ramBanks = 4; break;
        case 4: this->ramBanks = 16; break;  // GB Camera
    }
}

void Cartridge::DumpROM() {
    unsigned int addr = 0;
    
    // Read x number of banks
    for (int y = 1; y < romBanks; y++) {
        writeByte(0x2100, y); // Set ROM bank
        if (y > 1) {addr = 0x4000;}
        for (; addr <= 0x7FFF; addr = addr+64) {
            uint8_t readData[64];
            for(int i = 0; i < 64; i++){
                readData[i] = readByte(addr+i);
            }
            Serial.write(readData, 64);
        }
    }
}

void Cartridge::DumpRAM() {
    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    readByte(0x0134);

    unsigned int addr = 0;
    unsigned int endaddr = 0;
    if (cartridgeType == 6 && ramSize == 0) { endaddr = 0xA1FF; } // MBC2 512bytes (nibbles)
    if (ramSize == 1) { endaddr = 0xA7FF; } // 2K RAM
    if (ramSize > 1) { endaddr = 0xBFFF; } // 8K RAM
    
    // Does cartridge have RAM
    if (endaddr > 0) {
        // Initialise MBC
        writeByte(0x0000, 0x0A);

        // Switch RAM banks
        for (int bank = 0; bank < ramBanks; bank++) {
            writeByte(0x4000, bank);

            // Read RAM
            for (addr = 0xA000; addr <= endaddr; addr = addr+64) {  
                uint8_t readData[64];
                for(int i = 0; i < 64; i++){
                    readData[i] = readByte(addr+i);
                }
                Serial.write(readData, 64);
            }
        }
        
        // Disable RAM
        writeByte(0x0000, 0x00);
    }
}

void Cartridge::UploadRAM() {
    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    readByte(0x0134);
    unsigned int addr = 0;
    unsigned int endaddr = 0;
    if (cartridgeType == 6 && ramSize == 0) { endaddr = 0xA1FF; } // MBC2 512bytes (nibbles)
    if (ramSize == 1) { endaddr = 0xA7FF; } // 2K RAM
    if (ramSize > 1) { endaddr = 0xBFFF; } // 8K RAM
    
    // Does cartridge have RAM
    if (endaddr > 0) {
        // Initialise MBC
        writeByte(0x0000, 0x0A);
        
        // Switch RAM banks
        for (int bank = 0; bank < ramBanks; bank++) {
            writeByte(0x4000, bank);
            
            // Write RAM
            for (addr = 0xA000; addr <= endaddr; addr=addr+64) {  
            
            // Wait for serial input
            for (uint8_t i = 0; i < 64; i++) {
                // Wait for serial input
                while (Serial.available() <= 0);
                
                // Read input
                uint8_t bval = (uint8_t) Serial.read();
                
                // Write to RAM
                mreqPin_low();
                writeByte(addr+i, bval);
                asm volatile("nop");
                asm volatile("nop");
                asm volatile("nop");
                shortDelay(10);
                mreqPin_high(); 
            }
            }
        }
        
        // Disable RAM
        writeByte(0x0000, 0x00);
        Serial.flush(); // Flush any serial data that wasn't processed
    }
}