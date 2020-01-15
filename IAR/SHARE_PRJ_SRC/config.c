#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "string.h"
#include "basic.h"
#include "config.h"

static uint8_t default_key[16] = DEFAULT_KEY;
static uint8_t default_iv[16] = DEFAULT_IV;

void CF_init()
{
  // Загрузка данных из различных источников
  memcpy(CONFIG.stream_key, default_key, 16); 
  memcpy(CONFIG.stream_iv,  default_iv, 16);
  CONFIG.node_ver = NODE_VER;
  CONFIG.node_type = NODE_TYPE;
  CONFIG.sync_channel = DEFAULT_SYNC_CHANNEL;
  CONFIG.panid = DEFAULT_PANID;
}