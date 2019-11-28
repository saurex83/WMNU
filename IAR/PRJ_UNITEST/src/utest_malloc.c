#include "utest_suite.h"
#include "stdlib.h"

static  uint16_t getMallocSize(void *ptr)
{
  uint16_t *size = (uint16_t*)ptr - 1;
  return *size-2;
}


static void test_1(void)
{
  // Проверка как хранит размер памяти фун-я маллок
  // Перед указатель выделяется 2 байта и длинна на 2 байта увеличиается
  void *ptr_1 = malloc(0x32);
  void *ptr_2 = malloc(0x78);
  uint16_t size_1 = getMallocSize(ptr_1);
  uint16_t size_2 = getMallocSize(ptr_2);
}

void suite_malloc(void)
{
 // umsg_line("LLC module");
  
  // Устанавливает и поддерживает в очереде 10 пакетов.
  // Мигает зеленым светодиодом. В TS0 включается, в TS5 выключаеться
  test_1();  
}