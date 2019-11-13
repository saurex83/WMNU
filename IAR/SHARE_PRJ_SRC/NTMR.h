#pragma once
#include "stdbool.h"
#include "stdint.h"

void NT_Init(void);
bool NT_SetTime(uint16_t ticks);
void NT_SetCompare(uint16_t ticks);
void NT_SetEventCallback(void (*fn)(uint16_t ticks));
uint16_t NT_GetTime(void);