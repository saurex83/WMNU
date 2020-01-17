/*
 Модуль отладки стека
*/

#include "stdio.h"

#define UART_DEBUG // Если определено, то вывод в uart
//#define LOG_ENABLE
#define ASSERT_ENABLE

// Пример использования
//LOG("Message trace %f.2\n",7.7);
//ASSERT(false);

/**
@brief Отказ системы
*/
__attribute__((weak)) void STACK_FAILURE(char* msg);

#define LOG_OFF(...)

#ifdef LOG_ENABLE
  #define LOG_ON(...) {\
    printf("*LOG* %s:%d:%s -> ",__FILE__, __LINE__, __FUNCTION__); \
    printf(__VA_ARGS__); \
    printf("\r\n"); \
      }
#else
  #define LOG_ON(...)
#endif

//Если условие ложно то выводим сообщение
#ifdef ASSERT_ENABLE
  #define ASSERT(condition) {\
    if (!(condition)) {\
      printf("*ASSERT* %s:%d:%s -> ",__FILE__, __LINE__, __FUNCTION__);\
      printf("\r\n");\
      while(1);\
    }\
  }

#else
  #define ASSERT(condition)
#endif

void nwDebugerInit(void);