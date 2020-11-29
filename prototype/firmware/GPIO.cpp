#include "GPIO.hpp"

GPIO::GPIO() { 
    this->shiftregister = new ShiftRegister74HC595(2, this->dataPin, this->clockPin, this->latchPin);

    // data pins.
    this->dataInput();

    pinMode(wrPin, OUTPUT);
    pinMode(rdPin, OUTPUT);
    pinMode(csPin, OUTPUT);

    this->wr_high();
    this->rd_high();
    this->cs_high();
}

void GPIO::setAddress(Word address) { 
    for (int i = 0; i < 16; i++) {
        shiftregister->setNoUpdate(i, ((address & 0xFFFF) & (1 << i)) == (1 << i));
    }
    shiftregister->updateRegisters();
}

Byte GPIO::getByte(Word address) {
    this->setAddress(address);
    this->rd_low();
    this->cs_low();
    delayMicroseconds(10);

    Byte val = 0x00;
    for (int i = 0; i < 8; i++) {
        if (digitalRead(dataPins[i])) {
            val |= (0x01 << i);
        }
    }

    this->rd_high();
    this->cs_high();
    delayMicroseconds(10);
    
    return val;
}

void GPIO::setByte(Word address, Byte data) {
    this->dataOutput();

    setAddress(address);
    for (int i = 0; i < 8; i++) {
        digitalWrite(dataPins[i], data & (0x01 << i));
    }

    this->wr_low();
    delayMicroseconds(10);
    this->wr_high();
    delayMicroseconds(10);

    this->dataInput();
}

void GPIO::wr_low() {
    digitalWrite(wrPin, LOW);
}

void GPIO::wr_high() {
    digitalWrite(wrPin, HIGH);
}

void GPIO::rd_low() {
    digitalWrite(rdPin, LOW);
}

void GPIO::rd_high() {
    digitalWrite(rdPin, HIGH);
}

void GPIO::dataInput() {
    for (int i = 0; i < 8; i++) {
        pinMode(dataPins[i], INPUT);
    }
}

void GPIO::cs_low() {
    digitalWrite(csPin, LOW);
}

void GPIO::cs_high() {
    digitalWrite(csPin, HIGH);
}

void GPIO::dataOutput() {
    for (int i = 0; i < 8; i++) {
        pinMode(dataPins[i], OUTPUT);
    }
}
