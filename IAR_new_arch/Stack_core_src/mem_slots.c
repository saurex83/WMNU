#include "debug.h"
#include "global.h"
#include "macros.h"
#include "stdbool.h"
#include "action_manager.h"
#include "mem_utils.h"  
 

//!< Максимальный размер данных в одном слоте
#define SLOT_BUFFER_SIZE 150
#define RED_ZONE_CODE 0x73

typedef char red_zone_t;

static void SW_Init(void);
module_s MS_MODULE = {ALIAS(SW_Init)};

struct property{
  char taken: 1;
};

struct slot{
  struct property property;
  red_zone_t red_zone_1;
  char buffer[SLOT_BUFFER_SIZE];
  red_zone_t red_zone_2;
};

//!< Хранилище слотов
static struct slot SLOT_POOL[SLOT_POOL_ITEMS];
//!< Указатель на текщий элемент поиска
static struct slot *slot_ptr;
//!< Количество занятых слотов
static int slot_busy;

void SW_Init(void){
  slot_busy = 0;
  slot_ptr = NULL;
  for_each_type(struct slot, SLOT_POOL, slot){
    slot->property.taken = false;
    slot->red_zone_1 = RED_ZONE_CODE;
    slot->red_zone_2 = RED_ZONE_CODE;
  }
};

/**
@brief Выделяем свободный слот
*/
static inline struct slot* alloc(void){
  for (slot_ptr = SLOT_POOL; 
       slot_ptr < &SLOT_POOL[SLOT_POOL_ITEMS]; 
       slot_ptr++){  
    if (!slot_ptr->property.taken){
      slot_ptr->property.taken = true;
      return slot_ptr;
    }
  };
  return NULL;
}

static inline void fill_zero_slot(struct slot *slot){
  struct memset memset = {
    .dst = slot->buffer,
    .len = SLOT_BUFFER_SIZE,
    .val = 0};
  MEM_memset(&memset);
}

/**
@brief Выделяем свободный слот, но в поиске не участвует slot_ptr.
*/
static inline struct slot* protect_alloc(void){
  for_each_type(struct slot, SLOT_POOL, slot){
    if (slot == slot_ptr)
      continue;
    if (!slot->property.taken){
      slot->property.taken = true;
      return slot;
    }
  };
  return NULL;  
}

/**
@brief Возвращает указатель на буфер или NULL. Буфер заполнен 0
@detail Алгоритм имеет защиту от повторного входа. Защита организована
через использование глобального указателя slot_ptr. Если slot_ptr NULL,
для выделения слота вызывается фун-я alloc, которая в качестве итератора
использует slot_ptr. При повторном вхождении если slot_ptr не равен NULL,
используется функ-я protect_alloc, которая пропускает при поиске элемент
slot_ptr. Буфер заполнен нулями.
*/
char* SL_alloc(void){  
  struct slot *ret;
  
  if (slot_ptr == NULL){
    ret = alloc();
    slot_ptr = NULL;
  }
  else 
    ret = protect_alloc();
  
  if (ret != NULL){
    fill_zero_slot(ret);
    slot_busy++;
    return ret->buffer;
  }
  return NULL;
};

void SL_free(char *buff){
  struct slot *slot = container_of(buff, struct slot, buffer);
  // Найдем индекс в массиве по указателю
  size_t index = ptr_distance(slot, SLOT_POOL) / sizeof(struct slot);
  size_t offset = ptr_distance(slot, SLOT_POOL) % sizeof(struct slot);
  
  ASSERT(index < SLOT_BUFFER_SIZE);
  ASSERT(offset == 0);
  ASSERT(slot->property.taken == true);
  
  slot->property.taken = false;
  slot_busy--;
};


int SL_busy(){
  ASSERT(slot_busy < SLOT_POOL_ITEMS);
  return slot_busy;
};

int SL_available(){
  ASSERT(slot_busy < SLOT_POOL_ITEMS);  
  return SLOT_POOL_ITEMS - slot_busy;
};

int SL_zone_check(){
  int index = 0;
  for_each_type(struct slot, SLOT_POOL, slot){
    if (!(slot->red_zone_1 == RED_ZONE_CODE &&
          slot->red_zone_2 == RED_ZONE_CODE))
      return index;
    index++;
  }
  return -1;
};

