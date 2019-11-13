#include "utest_suite.h"
#include "LLC.h"
#include "NTMR.h"

static void test_create()
{
  NT_Init();
  LLC_Init();
  
  umsg("LLC", "Need program dep MAC ", true);
}

void suite_LLC_HW(void)
{
  umsg_line("LLC module");
  
  test_create();
}