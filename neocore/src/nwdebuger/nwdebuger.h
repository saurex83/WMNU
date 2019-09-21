/*
 Модуль отладки стека
*/

#include "printf.h"
#include "sig_trace.h"

// Уровни логирования
#define MSG_INFO	0x00
#define MSG_WARNING	0x01
#define MSG_ALARM	0x02
// Флаги логирования
#define MSG_TRACE	0x04
#define MSG_STATE	0x08
#define MSG_FRESH	0x10
#define MSG_HALT	0x20
#define MSG_ON		0x40
#define MSG_OFF		0x80

//******************************************************************************
//******************************************************************************
// Активные уровни логирования
#define DEBUG_LOG_LEVEL	MSG_INFO // MSG_WARNING MSG_INFO 
#define DEBUG_TYPES_ON  MSG_TRACE|MSG_STATE|MSG_FRESH|MSG_HALT
//******************************************************************************
//******************************************************************************

// Младшие два бита уровень логирования 
#define DBG_MASK_LEVEL 0x03

__attribute__((weak)) void STACK_FAILURE(char* msg);

#ifdef LOG_ENABLE
	#define LOG(level,...) {\
		if 	(\
			 ( (level) & MSG_ON) &&\
			 ( (level) & (DEBUG_TYPES_ON) ) &&\
			 ( ((level) & DBG_MASK_LEVEL) >= DEBUG_LOG_LEVEL)\
			)\
		{\
			tfp_printf("%s:%d->",__FILE__, __LINE__);\
			tfp_printf(__VA_ARGS__);\
		};\
		if ( (level) & MSG_HALT) {\
			STACK_FAILURE("Log halt");\
		};\
	}; 
#else
	#define LOG(level,...)
#endif

//Если условие ложно то выводим сообщение
#ifdef ASSERT_ENABLE
	
	#define ASSERT_HALT(condition, ...) {\
		if (!(condition)) {\
			tfp_printf("%s:%d->",__FILE__, __LINE__);\
			tfp_printf(__VA_ARGS__);\
			STACK_FAILURE("Assert fails");\
		}\
	}

	#define ASSERT(condition, ...) {\
		if (!(condition)) {\
			tfp_printf("%s:%d->",__FILE__, __LINE__);\
			tfp_printf(__VA_ARGS__);\
		}\
	}

#else
	#define ASSERT_HALT(condition,...)
	#define ASSERT(condition,...)
#endif

void nwDebugerInit(void);