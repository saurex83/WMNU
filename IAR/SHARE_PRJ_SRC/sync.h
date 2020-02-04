#pragma once
#include "stdbool.h"
#include "stdint.h"

void SY_Init(void);
void SY_Reset(void);
void SY_setIV(void* ptr_IV);
void SY_setKEY(void* ptr_KEY);
bool SY_SYNC_NETWORK(uint16_t *panid,uint16_t timeout);
void SY_Enable(bool en);
uint32_t SY_sync_sended(void);
bool SY_is_synced(void);