#include "action_manager.h"
#include "frame.h"
#include "coder.h"

static void SW_Init(void);


module_s MAC_MODULE = {ALIAS(SW_Init)};

struct ack{ // Формат структуры пакета ACK
  char CRC8;
} __attribute__((packed));

static void SW_Init(void){};

void MAC_Send(){
}

void MAC_Receive(){
}

static char xor_calc(struct frame *fr){
  char crc = 0x34; // Начальное значение
  char *val = fr->payload;
  for (char i = 0; i < fr->len; i++)
    crc ^= val[i];
  return crc;
}
// AES_StreamCoder(false, src, src, KEY, IV, size); BitRawDecrypt
// AES_StreamCoder(true, src, src, KEY, IV, size);