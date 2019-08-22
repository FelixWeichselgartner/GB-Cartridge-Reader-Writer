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

#include <ShiftRegister74HC595.h>

int latchPin = 10;
int dataPin = 11;
int clockPin = 12;
int rdPin = A1;
int wrPin = 13;
int mreqPin = A4;
int ledPin = A3;
int readPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};

// parameters: (number of shift registers, data pin, clock pin, latch pin)
ShiftRegister74HC595 sr (2, dataPin, clockPin, latchPin); 

#define wrPin_high    digitalWrite(wrPin, HIGH);
#define wrPin_low     digitalWrite(wrPin, LOW);
#define mreqPin_high  digitalWrite(mreqPin, HIGH);
#define mreqPin_low   digitalWrite(mreqPin, LOW);
#define rdPin_high    digitalWrite(rdPin, HIGH);
#define rdPin_low     digitalWrite(rdPin, LOW);
#define led_high      digitalWrite(ledPin, HIGH);
#define led_low       digitalWrite(ledPin, LOW);

void setup() {
  Serial.begin(400000);
  pinMode(rdPin, OUTPUT);
  pinMode(wrPin, OUTPUT);
  pinMode(mreqPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  for (int i = 0; i < 8; i++) {
    pinMode(readPins[i], INPUT);
  }
}

void loop() {
  // Wait for serial input
  while (Serial.available() <= 0) {
    digitalWrite(ledPin, HIGH);
    delay(200);
  }

  digitalWrite(ledPin, LOW);
  // Decode input
  char readInput[10];
  int readCount = 0;
  while (Serial.available() > 0) {
    char c = Serial.read();
    readInput[readCount] = c;
    readCount++;
  }
  readInput[readCount] = '\0';
  
  // Turn everything off
  rdPin_high; // RD off
  wrPin_high; // WR off
  mreqPin_high; // MREQ off

  // Read Cartridge Header
  char gameTitle[17];
  for (int addr = 0x0134; addr <= 0x143; addr++) {
    gameTitle[(addr-0x0134)] = (char) readByte(addr);
  }
  gameTitle[16] = '\0';
  int cartridgeType = readByte(0x0147);
  int romSize = readByte(0x0148);
  int ramSize = readByte(0x0149);
  int romBanks = 2; // Default 32K
  if (romSize == 1) { romBanks = 4; } 
  if (romSize == 2) { romBanks = 8; } 
  if (romSize == 3) { romBanks = 16; } 
  if (romSize == 4) { romBanks = 32; } 
  if (romSize == 5 && (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3)) { romBanks = 63; } 
  else if (romSize == 5) { romBanks = 64; } 
  if (romSize == 6 && (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3)) { romBanks = 125; } 
  else if (romSize == 6) { romBanks = 128; }
  if (romSize == 7) { romBanks = 256; }
  if (romSize == 82) { romBanks = 72; }
  if (romSize == 83) { romBanks = 80; }
  if (romSize == 84) { romBanks = 96; }
  int ramBanks = 1; // Default 8K RAM
  if (ramSize == 3) { ramBanks = 4; }
  if (ramSize == 4){ ramBanks = 16; } // GB Camera
   
  // Cartridge Header
  else if (strstr(readInput, "HEADER")) {
    Serial.println(gameTitle);
    Serial.println(cartridgeType);
    Serial.println(romSize);
    Serial.println(ramSize);
  }

  // Dump ROM
  else if (strstr(readInput, "READROM")) {
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
  
  // Read RAM
  else if (strstr(readInput, "READRAM")) {
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
  
  // Write RAM
  else if (strstr(readInput, "WRITERAM")) {
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
            mreqPin_low;
            writeByte(addr+i, bval);
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            mreqPin_high; 
          }
        }
      }
      
      // Disable RAM
      writeByte(0x0000, 0x00);
      Serial.flush(); // Flush any serial data that wasn't processed
    }
  }
}

uint8_t readByte(int address) {
  byte val = 0x00;
  shiftoutAddress(address); // Shift out address

  mreqPin_low;
  rdPin_low;
  asm volatile("nop"); // Delay a little (minimum is 2 nops, using 3 to be sure)
  asm volatile("nop");
  asm volatile("nop");
  delay(1);
  
  //uint8_t bval = ((PINB << 6) | (PIND >> 2)); // Read data
  for (int i = 0; i < 8; i++) {
    if (digitalRead(readPins[i])) {
      val |= (0x01 << i);
    }
  }
  
  rdPin_high;
  mreqPin_high;
  delay(1);
  
  return val;
}

void writeByte(int address, uint8_t data) {
  // Set pins as outputs
  DDRB |= ((1<<PB0) | (1<<PB1)); // D8 & D9
  DDRD |= ((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7)); // D2 to D7
  
  shiftoutAddress(address); // Shift out address
  
  // Clear outputs and set them to the data variable
  PORTB &= ~((1<<PB0) | (1<<PB1)); // D8 & D9
  PORTD &= ~((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7)); // D2 to D7
  PORTD |= (data << 2);
  PORTB |= (data >> 6);
  
  // Pulse WR
  wrPin_low;
  asm volatile("nop");
  wrPin_high;
  
  // Set pins as inputs
  DDRB &= ~((1<<PB0) | (1<<PB1)); // D8 & D9
  DDRD &= ~((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7)); // D2 to D7
}

// Use the shift registers to shift out the address
void shiftoutAddress(unsigned int shiftAddress) {
  for (int i = 0; i < 16; i++) {
    sr.setNoUpdate(i, ((shiftAddress & 0xFFFF) & (1 << i)) == (1 << i));
    //digitalWrite(ledPin, ((shiftAddress & 0xFFFF) & (1 << i)) == (1 << i));
    //delay(1);
  }
  sr.updateRegisters();
  delay(1);
}
