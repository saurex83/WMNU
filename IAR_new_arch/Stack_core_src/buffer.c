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

static bool _remove(struct list_head *list, struct frame *frame){
  struct node *node = container_of(frame, struct node, frame);
  size_t index = ptr_distance(node, NODE_LIST) / sizeof(struct node);
  size_t offset = ptr_distance(node, NODE_LIST) % sizeof(struct node);
    
  if (!(index < MAX_TXRX_BUFF_SIZE && offset == 0)) 
    return false;
  
  struct list_head *ptr;
  struct node *s_node;
  list_for_each(ptr, list){
    s_node = list_entry(ptr, struct node, list);
    if (s_node->frame == frame){
      s_node->frame = NULL;
      list_del(ptr);
      return true;
    }
  }
  return true;
}

bool BF_remove_rx(struct frame *frame){
  bool res;
  ATOMIC_BLOCK_RESTORE{
    res = _remove(&RX_LIST_HEAD, frame);
    if (res)
      RX_BUFF_FRAME_COUNT--; // ОШБКА TRUE верентся если и не удалил!!
  };
  return res;
};

bool BF_remove_tx(struct frame *frame){
  bool res;
  ATOMIC_BLOCK_RESTORE{
    res = _remove(&TX_LIST_HEAD, frame);
    if (res)
      TX_BUFF_FRAME_COUNT--;
  };
  return res;
};