#pragma once

/**
@brief Главное хранилище данных и настроек узла.
@detail Не придумал ничего лучше как использовать глобальную структуру.
 Другие решения более сложные и обьемные. 
*/
struct
{
  uint8_t node_type;
  uint8_t node_ver;
  uint8_t stream_key[16];
  uint8_t stream_iv[16];
  uint8_t sync_channel;
  uint8_t panid;
} CONFIG;

void CF_init();