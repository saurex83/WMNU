#include "ioCC2530.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "nwdebuger.h"

void re_free(void* ptr);
void* re_malloc(size_t size);
uint16_t heap_ptr(size_t size);
void re_memcpy(void *dst, const void *src, size_t n);

/**
@brief Возвращает адрес следующего свободного участка памяти
@detail Функция нужна для оценки использования стека
@param[in] size размер запрашиваемой области
@return если памяти нету возвращает 0, иначе адрес свободного участка.
*/
uint16_t heap_ptr(size_t size)
{
  unsigned short EA_save = EA;
  EA = 0; 
  uint16_t *heap_ptr;
  heap_ptr = (uint16_t*)re_malloc(size);
  re_free(heap_ptr);
  EA = EA_save;
  return (uint16_t)heap_ptr;
}

/**
@brief Реентерабельное копирование памяти
*/
void re_memcpy(void *dst, const void *src, size_t n)
{
  unsigned short EA_save = EA;
  EA = 0;
  memcpy(dst, src, n);
  EA = EA_save;  
}

/**
@brief Реентерабельное выделение памяти
*/
void* re_malloc(size_t size)
{
  unsigned short EA_save = EA;
  void* ptr; 
  EA = 0;
  ptr = malloc(size);
  EA = EA_save;
  //LOG(MSG_OFF | MSG_INFO | MSG_TRACE, "malloc = %d, L= %d \r\n", (uint16_t)ptr, size);
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
  //LOG(MSG_OFF | MSG_INFO | MSG_TRACE, "free = %d\r\n", (uint16_t)ptr);  
}
