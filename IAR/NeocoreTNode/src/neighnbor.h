#pragma once

void NP_Reset(void);
void NP_Init(void);
int NP_GetETX(void);
bool NP_Get_COMM_node_info(uint16_t *addr, uint8_t *ts, uint8_t *ch);
bool NP_is_pair_exist(uint8_t ts, uint8_t ch);
bool NP_Get_info_by_addr(uint16_t addr, uint8_t *ts, uint8_t *ch);