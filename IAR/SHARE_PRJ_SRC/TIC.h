#pragma once
#include "stdbool.h"
#include "stdint.h"
#include "delays.h"

void TIC_Init(void);
void TIC_Reset(void);
void TIC_SetTimer(uint16_t ticks);
uint16_t TIC_GetTimer(void);
bool TIC_SetTXState(uint8_t TS, bool state);
bool TIC_SetRXState(uint8_t TS, bool state);
bool TIC_GetTXState(uint8_t TS);
bool TIC_GetRXState(uint8_t TS);
void TIC_CloseAllState();
void TIC_SetRXCallback(void (*fn)(uint8_t TS));
void TIC_SetTXCallback(void (*fn)(uint8_t TS));
void TIC_SetSECallback(void (*fn)(uint8_t TS));
void TIC_SetSyncTimeAllocCallback(void (*fn)(void));
void TIC_SetTS1Callback(void (*fn)(void));
uint32_t TIC_GetUptime(void);
uint32_t TIC_GetRTC(void);
bool TIC_SetRTC(uint32_t RTC);
void TIC_SetNonce(uint32_t nonce);
uint32_t TIC_GetNonce(void);
uint32_t TIC_TimeUsFromTS0();
TimeStamp_s* TIC_GetTimeStampTS0(void);
uint16_t TIC_SleepTime(void);
uint16_t TIC_SlotActivityTime(void);
uint16_t TIC_SlotTime(uint8_t TS);