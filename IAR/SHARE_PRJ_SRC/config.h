#pragma once

struct
{
  uint8_t node_type;
  uint8_t node_ver;
  uint8_t stream_key[16];
  uint8_t stream_iv[16];

} CONFIG;

void CF_init();