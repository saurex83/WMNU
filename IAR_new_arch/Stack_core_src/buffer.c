#include "debug.h"
#include "macros.h"
#include "frame.h"
#include "stdbool.h"

#include "action_manager.h"
static void SW_Init(void);
module_s BF_MODULE = {ALIAS(SW_Init)};

struct property{
  union{
    struct {
    char tx_ready: 1;
    char rx_ready: 1;
   };
   char val;
  };
};

static struct property BUFFER_PROP[MAX_TXRX_BUFFER];
static struct frame* BUFFER[MAX_TXRX_BUFFER];

static void SW_Init(void){
  for_each_type(struct property, BUFFER_PROP, prop){
    prop->val = 0;
  }
};

static inline int free_idx(void){
  int idx = 0;
  for_each_type(struct property, BUFFER_PROP, prop)
    if (!prop->val)
      return idx;
    else
      idx++;
  HALT("No free idx");
  return -1;
};

int BF_add_rx(struct frame* frame){
  int idx = free_idx();
  BUFFER_PROP[idx].rx_ready = true;
  BUFFER[idx] = frame;
  return idx;
};

int  BF_add_tx(struct frame* frame){
  int idx = free_idx();
  BUFFER_PROP[idx].tx_ready = true;
  BUFFER[idx] = frame;
  return idx;
};

void BF_del(int idx){
  ASSERT (idx >= 0 && idx < MAX_TXRX_BUFFER);
  ASSERT(BUFFER_PROP[idx].val != 0);
  BUFFER_PROP[idx].val = 0;
};

struct frame* BF_get_tx(int *idx){
  struct frame* ret = NULL;
  while (*idx < MAX_TXRX_BUFFER){
    if (BUFFER_PROP[*idx].tx_ready){
        ret = BUFFER[*idx];
        idx++;
        break;
    }
  }
  return ret;
};

struct frame* BF_get_rx(int *idx){
  struct frame* ret = NULL;
  while (*idx < MAX_TXRX_BUFFER){
    if (BUFFER_PROP[*idx].rx_ready){
        ret = BUFFER[*idx];
        idx++;
        break;
    }
  }
  return ret;
};
