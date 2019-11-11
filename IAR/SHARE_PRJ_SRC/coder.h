#pragma once
#include "stdbool.h"
#include "stdint.h"
  
void AES_StreamCoder(bool enc_mode, uint8_t *src, uint8_t *dst, uint8_t *key,
                         uint8_t *iv, uint8_t len);
void AES_init(void);