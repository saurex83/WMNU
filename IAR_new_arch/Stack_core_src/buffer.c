#include "debug.h"
#include "macros.h"
#include "frame.h"
#include "stdbool.h"
#include "cpu.h"
#include "list.h"

#include "action_manager.h"
static void SW_Init(void);
module_s BF_MODULE = {ALIAS(SW_Init)};

struct node {
  struct frame *frame;
  struct list_head list ;
};

static struct node NODE_LIST[MAX_TXRX_BUFF_SIZE];
LIST_HEAD(TX_LIST_HEAD);
LIST_HEAD(RX_LIST_HEAD);
static int TX_BUFF_FRAME_COUNT;
static int RX_BUFF_FRAME_COUNT;

static void SW_Init(void){
  TX_BUFF_FRAME_COUNT = 0;
  RX_BUFF_FRAME_COUNT = 0;
  INIT_LIST_HEAD(&TX_LIST_HEAD);
  INIT_LIST_HEAD(&RX_LIST_HEAD);
  
  for_each_type(struct node, NODE_LIST, node){
    node->frame = NULL;
    INIT_LIST_HEAD(&node->list);
  }
};

int BF_rx_busy(){
  return RX_BUFF_FRAME_COUNT;
};

int BF_tx_busy(){
  return TX_BUFF_FRAME_COUNT;
}

int BF_available(){
  int summ = RX_BUFF_FRAME_COUNT + TX_BUFF_FRAME_COUNT;
  ASSERT(summ <= MAX_TXRX_BUFF_SIZE);
  return MAX_TXRX_BUFF_SIZE - summ;
}

static inline struct node* find_free_node(void){
  for_each_type(struct node, NODE_LIST, node)
    if (node->frame == NULL){
      INIT_LIST_HEAD(&node->list);
      return node;
    }
  return NULL;
};

static inline bool _push(struct list_head *list, struct frame *frame){
  ASSERT(frame != NULL);
  struct node *node = find_free_node();
  if (!node) // Нет свободных мест
    return false;
  node->frame = frame;
  list_add(&node->list , list);
  return true;
};

bool BF_push_tx(struct frame *frame){
  bool res;
  ATOMIC_BLOCK_RESTORE{
    res = _push(&TX_LIST_HEAD, frame);
    if (res)
      TX_BUFF_FRAME_COUNT++;
  };
  return res;
};

bool BF_push_rx(struct frame *frame){
  bool res;
  ATOMIC_BLOCK_RESTORE{
    res = _push(&RX_LIST_HEAD, frame);
    if (res)
      RX_BUFF_FRAME_COUNT++;
  };
  return res;
};

void* BF_cursor_rx(void){
  if (list_empty(&RX_LIST_HEAD))
    return NULL;
  struct node *node = list_first_entry(&RX_LIST_HEAD, struct node, list);
  return node;
}

void* BF_cursor_tx(void){
  if (list_empty(&TX_LIST_HEAD)) 
    return NULL;
  struct node *node = list_first_entry(&TX_LIST_HEAD, struct node, list);
  return node;
}  
void* BF_cursor_next(void* cursor){
  if (!cursor)
    return NULL;
  struct node *node = (struct node*)(cursor);
  struct node *next = list_next_entry(node, struct node, list);
  if (node == next)
    return NULL;
  return next;
}

bool BF_remove(void *cursor){
  if (!cursor)
    return false;
  ATOMIC_BLOCK_RESTORE{
    struct node *node = (struct node*)(cursor);    
    node->frame = NULL;
    list_del(&node->list);    
    TX_BUFF_FRAME_COUNT--; 
  };  
  return true;
}

struct frame* BF_content(void* cursor){
  if (!cursor)
    return NULL;
  struct node *node = (struct node*)(cursor);
  return node->frame;
}