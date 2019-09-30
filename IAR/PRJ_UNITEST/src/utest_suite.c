#include "stdio.h"
#include "utest_suite.h"

static struct utest_summary_s
{
	uint16_t passed;
	uint16_t failed;
} utest_summary = {.passed =0, .failed = 0};;

//utest_summary_s utest_summary = {.passed =0, .failed = 0};

// Требует много програмной памяти
size_t heap_size(void)
{ 
  return 0;
}

bool memory_compare(char* area1, char* area2, uint16_t size)
{
	while (size > 0)
	{
		if (*area1 != *area2)
			return false;
		area1++;
		area2++;
        size--;
	} 
	return true;
}

void umsg(char* module, char* name, bool status)
{
	if (status){
		utest_summary.passed++;
		printf("Module: %s, Name: %s, Status: %s\n", module, name, "PASSED");
		}
	else{
		utest_summary.failed++;
		printf("Module: %s, Name: %s, Status: %s\n", module, name, "FAILED");
		}
}

void umsg_line(char* name)
{
	printf("**************************************\n");
	printf("%s\n", name);
	printf("**************************************\n");
}

void umsg_summary()
{
	umsg_line("Unit test symmary");
	printf("PASSED: %d\n", utest_summary.passed);
	printf("FAILD : %d\n", utest_summary.failed);
	if (utest_summary.failed == 0)
      printf("Tests PASSED\n");
	else
		printf("Tests FAILED\n");
}
