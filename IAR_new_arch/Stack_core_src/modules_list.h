#pragma once
#include "action_manager.h"
#include "stdlib.h"

/* Перечень груп в которые входят модули */
#define COLD_MOD_DEF {NULL}
#define HOT_MOD_DEF {NULL}
#define HW_MOD_DEF {&CPU_MODULE, &DBG_MODULE, &AT_MODULE, &UST_MODULE,  \
  &DMA_MODULE, &CODER_MODULE, &RADIO_MODULE}
#define SW_MOD_DEF {&FR_MODULE, &MD_MODULE, &MS_MODULE, \
  &BF_MODULE, &RADIO_MODULE}
#define POW_MOD_DEF {&RTC_MODULE}

/* Обьявления всех модулей */
extern module_s  AT_MODULE, FR_MODULE, RTC_MODULE, UST_MODULE,
  MD_MODULE, DBG_MODULE, CPU_MODULE, MS_MODULE, BF_MODULE,
  DMA_MODULE, CODER_MODULE, RADIO_MODULE;


//#include "action_manager.h"
//static void SW_Init(void);
//module_s BF_MODULE = {ALIAS(SW_Init)};
