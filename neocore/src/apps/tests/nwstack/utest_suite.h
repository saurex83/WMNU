
#undef ASSERT_ENABLE
#define ASSERT_ENABLE

#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "nwdebuger.h"

// Размер памяти использованной на куче.
size_t heap_size(void);

struct utest_summary_s
{
	uint16_t passed;
	uint16_t failed;
};

struct utest_summary_s utest_summary;


bool memory_compare(char* area1, char* area2, uint16_t size);
void umsg_summary();
void umsg_line(char* name);
void umsg(char* module, char* name, bool status);