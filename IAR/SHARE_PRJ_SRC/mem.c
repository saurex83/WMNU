#include "ioCC2530.h"
#include "stdlib.h"
#include "stdint.h"

void re_free(void* ptr);
void* re_malloc(size_t size);
uint16_t heap_ptr(size_t size);

/**
@brief Возвращает адрес следующего свободного участка памяти
@detail Функция нужна для оценки использования стека
@param[in] size размер запрашиваемой области
@return если памяти нету возвращает 0, иначе адрес свободного участка.
*/
uint16_t heap_ptr(size_t size)
{
  uint16_t *heap_ptr;
  heap_ptr = (uint16_t*)re_malloc(size);
  re_free(heap_ptr);
  return (uint16_t)heap_ptr;
}

/**
@brief Реентерабельное выделение памяти
*/
void* re_malloc(size_t size)
{
  void* ptr;
  unsigned short EA_save = EA;
  
  EA = 0;
  ptr = malloc(size);
  EA = EA_save;
  
  return ptr;
}

/**
@brief Реентерабельное освобождение памяти
*/
void re_free(void* ptr)
{
  unsigned short EA_save = EA;
  
  EA = 0;
  free(ptr);
  EA = EA_save;
}
