#include "action_manager.h"
#include "time_manager.h"
#include "debug.h"
#include "model.h"
#include "sync.h"
#include "action_manager.h"

static void callback(void){
}
              
static void pre_init(void){
  AM_HW_Init();
  AM_SW_Init();
  LOG_ON("Hardware inited");
  MODEL.SYNC.mode = 1;
  MODEL.TM.MODE = 1;
  AM_set_callback(callback);
}

void main(void){
  pre_init();
  LOG_ON("Node started");
  
  while (1){
    MODEL.SYNC.mode = 1;
    MODEL.TM.MODE = 1;
    while (!network_sync(1000000U));
    LOG_ON("Synced");
    Neocore_start();
    AM_SW_Init();
    LOG_ON("START RESYNC");
  }
};
