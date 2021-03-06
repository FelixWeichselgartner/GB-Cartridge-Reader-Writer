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

// Shield
int ledPin = A3;

#include "Cartridge.hpp"
Cartridge cartridge;

void led_high() {
    digitalWrite(ledPin, HIGH);
}

void led_low() {
    digitalWrite(ledPin, LOW);
}

void setup() {
    Serial.begin(115200);
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
}
