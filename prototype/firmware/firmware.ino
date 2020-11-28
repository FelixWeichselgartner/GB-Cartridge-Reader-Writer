/*
 GBCartRead
 Version: 1.6 Rev 1
 Author: Alex from insideGadgets (http://www.insidegadgets.com)
 Created: 18/03/2011
 Last Modified: 17/02/2014
 
 GBCartRead is an Arduino based Gameboy Cartridge Reader which uses a C program or python script to interface with 
 the Arduino. GBCartRead allows you to dump your ROM, save the RAM and write to the RAM.

 Works with Arduino Duemilanove and Uno. Will work for 5V Arduinos but requires wiring changes.

 Speed increase thanks to Frode vdM. (fvdm1992@yahoo.no) and David R

 ***********************************************

 Some changes made by Felix Weichselgartner.
 
*/

// ShiftRegisters
int latchPin = 10;
int dataPin = 11;
int clockPin = 12;
// Cartridge
int rdPin = A1;
int wrPin = 13;
int mreqPin = A4;
int dataPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};
// Shield
int ledPin = A3;

#include "Cartridge.hpp"
Cartridge cartridge(dataPin, clockPin, latchPin, rdPin, wrPin, mreqPin, dataPins);

void led_high() {
    digitalWrite(ledPin, HIGH);
}

void led_low() {
    digitalWrite(ledPin, LOW);
}

void setup() {
    Serial.begin(74880);
    pinMode(ledPin, OUTPUT);
}

void loop() {
    // Wait for serial input
    while (Serial.available() <= 0) {
        led_high();
        delay(200);
    }

    led_low();
    
    // Decode input
    char readInput[10];
    int readCount = 0;
    while (Serial.available() > 0) {
        char c = Serial.read();
        readInput[readCount] = c;
        readCount++;
    }
    readInput[readCount] = '\0';
    
    cartridge.ReadHeader();

    /*for (unsigned short i = 0; i < 4; i++) {
        //cartridge.WriteByte(0x6000, 0); // Set ROM Mode 
        //cartridge.WriteByte(0x4000, i >> 5); // Set bits 5 & 6 (01100000) of ROM bank
        //cartridge.WriteByte(0x2000, i & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
        cartridge.WriteByte(0x2100, i & 0xFF);
        //cartridge.WriteByte(0x3000, 0);
        Serial.print(i);
        Serial.print(": ");
        Byte data1 = cartridge.ReadByte(0x4000);
        Serial.println(data1);
    }*/
    
    // Cartridge Header
    if (strstr(readInput, "HEADER")) {
        cartridge.SerialPrintHeader();
    }
    // Dump ROM
    else if (strstr(readInput, "READROM")) {
        cartridge.DumpROM();
    }
    // Read RAM
    else if (strstr(readInput, "READRAM")) {
        cartridge.DumpRAM();
    }
    // Write RAM
    else if (strstr(readInput, "WRITERAM")) {
        cartridge.UploadRAM();
    }

    cartridge.rd_wr_mreq_low();
}
