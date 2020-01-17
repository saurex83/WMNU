#include "dma.h"
#include "ioCC2530.h"

DMA_s DMA_CH[5];

/**
@brief Настройка DMA каналов.
@detail Согласно документации каналы 1-4 идут последовательно
*/
void DMA_init()
{
    // Настроим канал 0 DMA для загрузки данных в AES
  uint16_t CONF_ADDR = (uint16_t)&DMA_CH[0];
  DMA0CFGH = CONF_ADDR >> 8;
  DMA0CFGL = CONF_ADDR & 0xFF;
  
  // Настроим канал 1 DMA для выгрузки данных из AES
  CONF_ADDR = (uint16_t)&DMA_CH[1];
  DMA1CFGH = CONF_ADDR >> 8;
  DMA1CFGL = CONF_ADDR & 0xFF;
}