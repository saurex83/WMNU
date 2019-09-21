
#include "stdlib.h"
#include "malloc.h"
#include "utest_suite.h"

struct utest_summary_s utest_summary = {.passed =0, .failed = 0};

size_t heap_size(void)
{
	struct mallinfo info = mallinfo();
	
//https://github.com/eblot/newlib/blob/master/newlib/libc/include/malloc.h
	return info.hblkhd;  
}


bool memory_compare(char* area1, char* area2, uint16_t size)
{
	while (size > 0)
	{
		if (*area1 != *area2)
			return false;
		area1++;
		area2++;
	} 
	return true;
}

void umsg(char* module, char* name, bool status)
{
	if (status){
		utest_summary.passed++;
		tfp_printf("Module: %s, Name: %s, Status: %s", module, name, "PASSED");
		}
	else{
		utest_summary.failed++;
		tfp_printf("Module: %s, Name: %s, Status: %s", module, name, "FAILED");
		}
}

void umsg_line(char* name)
{
	tfp_printf("**************************************\n");
	tfp_printf("%s\n", name);
	tfp_printf("**************************************\n");
}

void umsg_summary()
{
	umsg_line("Unit test symmary");
	tfp_printf("PASSED: %d\n", utest_summary.passed);
	tfp_printf("FAILD : %d\n", utest_summary.failed);
	if (utest_summary.failed == 0)
		tfp_printf("Tests PASSED\n");
	else
		tfp_printf("Tests FAILED");
}
