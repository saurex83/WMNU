#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"

// Размер памяти использованной на куче.
size_t heap_size(void);
enum utest_leds {D1, D2, D3};
bool memory_compare(char* area1, char* area2, uint16_t size);
void umsg_summary();
void umsg_line(char* name);
void umsg(char* module, char* name, bool status);
void utestSigTraceInit(void);
void LED(uint8_t led, bool state);