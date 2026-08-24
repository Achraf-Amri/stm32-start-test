#ifndef STM32_USART_BOOTLOADER_H
#define STM32_USART_BOOTLOADER_H

#include <Arduino.h>

#define STM32_ACK  0x79
#define STM32_NACK 0x1F

class Stm32_usart_bootloader {
private:
  HardwareSerial* _serial;

  uint8_t readByte(unsigned long timeout = 1000) {
    unsigned long start = millis();
    while (!_serial->available()) {
      if (millis() - start > timeout) return 0x00;
    }
    return _serial->read();
  }

public:
  Stm32_usart_bootloader(HardwareSerial* serial) : _serial(serial) {}

  uint8_t enter() {
    while (_serial->available()) _serial->read();
    _serial->write(0x7F);
    return readByte(2000);
  }

  uint8_t program(const uint8_t* data, size_t length) {
    size_t offset = 0;
    uint32_t baseAddr = 0x08000000;

    while (offset < length) {
      uint16_t chunkSize = (length - offset > 256) ? 256 : (length - offset);
      uint32_t currentAddr = baseAddr + offset;

      while (_serial->available()) _serial->read();
      _serial->write(0x31);
      _serial->write(0xCE);
      if (readByte(1000) != STM32_ACK) return STM32_NACK;

      uint8_t addrBuf[4] = {
        (uint8_t)(currentAddr >> 24),
        (uint8_t)(currentAddr >> 16),
        (uint8_t)(currentAddr >> 8),
        (uint8_t)(currentAddr)
      };
      uint8_t addrChecksum = addrBuf[0] ^ addrBuf[1] ^ addrBuf[2] ^ addrBuf[3];

      _serial->write(addrBuf, 4);
      _serial->write(addrChecksum);
      if (readByte(1000) != STM32_ACK) return STM32_NACK;

      uint8_t lenByte = chunkSize - 1;
      uint8_t dataChecksum = lenByte;

      _serial->write(lenByte);
      for (size_t i = 0; i < chunkSize; i++) {
        uint8_t b = data[offset + i];
        _serial->write(b);
        dataChecksum ^= b;
      }
      _serial->write(dataChecksum);

      if (readByte(3000) != STM32_ACK) return STM32_NACK;

      offset += chunkSize;
    }
    return STM32_ACK;
  }

  uint8_t go(uint32_t address) {
    while (_serial->available()) _serial->read();
    _serial->write(0x21);
    _serial->write(0xDE);
    if (readByte(1000) != STM32_ACK) return STM32_NACK;

    uint8_t addrBuf[4] = {
      (uint8_t)(address >> 24),
      (uint8_t)(address >> 16),
      (uint8_t)(address >> 8),
      (uint8_t)(address)
    };
    uint8_t addrChecksum = addrBuf[0] ^ addrBuf[1] ^ addrBuf[2] ^ addrBuf[3];

    _serial->write(addrBuf, 4);
    _serial->write(addrChecksum);
    return readByte(1000);
  }
};

#endif
