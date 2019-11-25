#include "utest_suite.h"
#include "LLC.h"
#include "NTMR.h"
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "frame.h"
#include "RADIO.h"
#include "TIC.h"

static void timealloc(void)
{
  static bool led_stat = false;
    LED(D1, led_stat);
    led_stat = !led_stat;
}

static void test_create()
{
  LLC_Init();
  LLC_TimeAlloc(timealloc);
  EA = 1;
  uint8_t DATA_SEND[10] = {0,1,2,3,4,5,6,7,8,9};  
  uint8_t TS = 47;
  
  frame_s *fr;
  
  uint16_t added = 0;
  
  while(true)
  {  
    if (LLC_GetTaskLen() < 10)
    {
      fr = frame_create();
      fbuf_s *fb = fbuf_create(FB_RAW_LAY, DATA_SEND, sizeof(DATA_SEND));
      fr->meta.SEND_TIME = 0;
      fr->meta.TS = TS;
      fr->meta.CH = CH11;
      frame_insert_head(fr, fb);
      if (fr == NULL)
        while(1);
      LLC_AddTask(fr);
      added ++;
    }
  }
  
}

void suite_LLC_HW(void)
{
 // umsg_line("LLC module");
  
  // Устанавливает и поддерживает в очереде 10 пакетов.
  // Мигает зеленым светодиодом. В TS0 включается, в TS5 выключаеться
  test_create();  
}