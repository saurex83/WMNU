#pragma once
#include "stdint.h"

#define GATEWAY
#define NODE_VER 0
#define NODE_TYPE 1

#define RARIO_STREAM_ENCRYPT  // Шифрование данных включенно 
//                   ОШИБКА НЕТ ЗАПЯТОЙ МЕЖДУ 6 и 7
#define DEFAULT_KEY  {1,2,3,4,5,67,8,9,10,11,12,13,14,15,16}
#define DEFAULT_IV   {1,2,3,4,5,67,8,9,10,11,12,13,14,15,16}
#define DEFAULT_SYNC_CHANNEL 28
#define DEFAULT_PANID 0x123 