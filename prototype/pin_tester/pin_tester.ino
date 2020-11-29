/*
This is a pin tester. It's supposed to verify your hardware is working properly.
Recommended to use with https://github.com/FelixWeichselgartner/Arduino_Logic_Analyzer.
See here on how to test https://felixweichselgartner.github.io/2020/11/28/GB_Cartridge_Reader_Writer_continued.html.

Copy GPIO.hpp, GPIO.cpp and datatypes.h from the firmware folder in this one.
*/

#include "GPIO.hpp"
GPIO gpio; 

void setup() {
  gpio.dataOutput();
  gpio.setDataPinsOutput(0x00);
  gpio.wr_high();
  gpio.rd_high();
  gpio.cs_high();
}

void loop() {
  gpio.wr_low();
  delay(40);
  gpio.wr_high();
  gpio.rd_low();
  delay(40);
  gpio.rd_high();
  gpio.cs_low();
  delay(40);
  gpio.cs_high();

  for (int i = 0; i < 16; i++) {
    gpio.setAddress(1 << i);
    delay(40);
  }
  gpio.setAddress(0x0000);

  for (int i = 0; i < 8; i++) {
    gpio.setDataPinsOutput(1 << i);
    delay(40);
  }
  gpio.setDataPinsOutput(0x00);
}
