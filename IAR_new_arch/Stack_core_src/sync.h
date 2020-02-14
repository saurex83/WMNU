#pragma once
#include "stdbool.h"
#include "radio.h"

#define SYNC_MODEL SYNC
struct SYNC{
  char mode;
  bool synced;
  channel_t sys_channel;
  channel_t sync_channel;
  unsigned long next_sync_send;
  unsigned long next_time_recv;
  char panid;
};
