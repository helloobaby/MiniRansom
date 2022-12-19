#include"encryp.h"

void encrypt_file(uint8_t* buffer, uint32_t length) {
  static uint8_t xor_key = 0x7A;

  for (uint32_t i = 0; i < length; i++) {
    buffer[i] ^= xor_key;
  }
}