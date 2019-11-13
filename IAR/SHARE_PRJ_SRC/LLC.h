#pragma once
#include "stdbool.h"
#include "stdint.h"
#include "MAC.h"
#include "TIC.h"
#include "frame.h"

typedef struct
{
  void (*LLC_SetRXCallback)(void (*fn)(frame_s *fr));
  void (*LLC_TimeAlloc)(void (*fn)(void));
  void (*LLC_AddTask)(frame_s* fr); 
} LLC_s;

#define LLC_S_SIZE sizeof(LLC_s)

LLC_s* LLC_Create(MAC_s *mac, TIC_s *tic);
bool LLC_Delete(LLC_s *llc);