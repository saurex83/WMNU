#include "nwdebuger.h"
#include "manager.h"
#include "auth.h"
#include "balancer.h"
#include "ethernet.h"
#include "ip.h"
#include "neighnbor.h"
#include "route.h"
#include "tcp.h"
#include "udp.h"

/**
@file
@detail
*/

// Определения

// Локальные функции

// Глобальные функции
void NEOCORE_Init(void);
bool NEOCORE_Reset(void);

// Локальные переменные

void NEOCORE_Init(void){
  neocore_hw_init();
  AUTH_Init();
  TB_Init();
  ETH_Init();
  IP_Init();
  NP_Init();
  RP_Init();
  TCP_Init();
  UDP_Init();
};

bool NEOCORE_Reset(void){
  neocore_stack_reset();
  AUTH_Reset();
  TB_Reset();
  ETH_Reset();
  IP_Reset();
  NP_Reset();
  RP_Reset();
  TCP_Reset();
  UDP_Reset();
  
  // Проверим все ли пакеты уничтожены
  uint8_t nbrF = frame_getCount(); 
  ASSERT(nbrF == 0);
  if (!nbrF)
    return true;
  return false;
}