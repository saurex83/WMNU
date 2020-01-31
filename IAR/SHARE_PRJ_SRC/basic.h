#pragma once
#include "stdint.h"

//#define GATEWAY
#define NODE_VER 0
#define NODE_TYPE 1

#define HEADER_ETH_VER 1 // Версия протокола

#define RARIO_STREAM_ENCRYPT  // Шифрование данных включенно 

#define DEFAULT_KEY  {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}
#define DEFAULT_IV   {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}
#define DEFAULT_SYNC_CHANNEL 28
#define DEFAULT_PANID 0x12 