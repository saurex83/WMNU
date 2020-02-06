#pragma once
#include "action_manager.h"
#include "stdlib.h"

/* Перечень груп в которые входят модули */
#define COLD_MOD_DEF {NULL}
#define HOT_MOD_DEF {NULL}
#define HW_MOD_DEF {&AT_MODULE}
#define SW_MOD_DEF {&TM_MODULE, &FR_MODULE, &MD_MODULE}
#define POW_MOD_DEF {&RTC_MODULE}

/* Обьявления всех модулей */
extern module_s TM_MODULE, AT_MODULE, FR_MODULE, RTC_MODULE,
  MD_MODULE, DBG_MODULE;
