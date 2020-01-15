/*!
@file Модуль работы с памятью
@brief Работа с памятью
@detail При работе с памятью есть неприятный момент - функции stdlib не являются
 реентерабельными. При прерывании работы malloc прерыванием с повторным
 вызовом malloc присходит разрушении кучи. Ошибка сложная для обнаружения.
 Для решения проблемы функции работы с память обертнуты в запрет прерывания 
 с последующим снятием блокировка после выделения памяти.
*/

#include "stdint.h"
#include "stdlib.h"

void re_free(void* ptr);
void* re_malloc(size_t size);
uint16_t heap_ptr(size_t size);
void re_memcpy(void *dst, const void *src, size_t n);