#include "utest_suite.h"
#include "string.h"
#include "Frames.h"
#include "stdio.h"

static void test_ETH_H(void)
{
  ETH_H_t ETH_H;
  
  // Заполняем нулями.
  memset( &ETH_H, 0x00, ETH_H_T_SIZE);
  umsg("Frames", "Size of ETH_H_t struct 8 bytes", ETH_H_T_SIZE == 8);
//  printf("size %d", ETH_H_T_SIZE);
}

static void test_ETH_F(void)
{
  ETH_F_t ETH_F;
  
  // Заполняем нулями.
  memset( &ETH_F, 0x00, ETH_F_T_SIZE);
  umsg("Frames", "Size of ETH_F_t struct 2 bytes", ETH_F_T_SIZE == 2);
 // printf("size %d", ETH_F_T_SIZE);
}

static void test_IP_HEADER(void)
{
  IP_HEADER_t IP_H;
  
  // Заполняем нулями.
  memset( &IP_H, 0x00, IP_HEADER_T_SIZE);
  umsg("Frames", "Size of IP_HEADER_t struct 7 bytes", IP_HEADER_T_SIZE == 7);
 // printf("size %d", ETH_F_T_SIZE);
}

static void test_SYNC(void)
{
  SYNC_t SYNC;
  
  // Заполняем нулями.
  memset( &SYNC, 0x00, SYNC_T_SIZE);
  umsg("Frames", "Size of SYNC_t struct 6 bytes", SYNC_T_SIZE == 6);
 // printf("size %d", ETH_F_T_SIZE);
}

void suite_Frames(void)
{
  umsg_line("Frames module");
  test_ETH_H();
  test_ETH_F();
  test_IP_HEADER();
  test_SYNC();
}