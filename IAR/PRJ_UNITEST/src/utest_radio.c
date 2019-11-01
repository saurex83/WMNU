#include "utest_suite.h"
#include "RADIO.h"
#include "ioCC2530.h"


RI_s* ri;

static void RadioOn_test(void)
{
  ri = RI_create();
}



void suite_RADIO_HW(void)
{
  //umsg_line("NTMR module");
  RadioOn_test();
}