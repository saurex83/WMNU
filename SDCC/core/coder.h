#pragma once
#include "stdbool.h"
#include "stdint.h"
  
void AES_init(void);
void AES_StreamCoder(bool enc_mode, uint8_t *src, uint8_t *dst, uint8_t *key,
                         uint8_t *iv, uint8_t len);
void AES_CCMEncrypt( uint8_t *src, uint8_t c, uint8_t f, uint8_t m,
                    uint8_t *key, uint8_t *nonce, uint8_t *MIC);

bool AES_CCMDecrypt( uint8_t *src, uint8_t c, uint8_t f, uint8_t m,
                    uint8_t *key, uint8_t *nonce, uint8_t *MIC);