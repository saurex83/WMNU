#include "utest_suite.h"
#include "LLC.h"

static void test_create()
{
  //MAC_s mac;
  //TIC_s tic;
  
  //LLC_s* LLC_Create(MAC_s *mac, TIC_s *tic);
  umsg("LLC", "Need program dep MAC ", true);
}

void suite_LLC(void)
{
  umsg_line("LLC module");
  
  test_create();
}