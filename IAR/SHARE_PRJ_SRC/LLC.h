#pragma once
#include "stdbool.h"
#include "stdint.h"
#include "frame.h"


void LLC_Init(void);
void LLC_SetRXCallback(void (*fn)(frame_s *fr));
void LLC_TimeAlloc(void (*fn)(void)); 
void LLC_AddTask(frame_s* fr);