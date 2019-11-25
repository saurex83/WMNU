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

static frame_s* getFrame(uint8_t *src, uint8_t len, uint8_t CH, uint8_t TS)
{
  frame_s *fr;
  fr = frame_create();
  fbuf_s *fb = fbuf_create(FB_RAW_LAY, src, len);
  fr->meta.SEND_TIME = 0;
  fr->meta.TS = TS;
  fr->meta.CH = CH;
  frame_insert_head(fr, fb);
  return fr;
}

static void test_create()
{
  LLC_Init();
  LLC_TimeAlloc(timealloc);
  EA = 1;
  uint8_t DATA_SEND[10] = {0,1,2,3,4,5,6,7,8,9};  
  
  frame_s *fr1, *fr2, *fr3;
  
  uint16_t added = 0;
  
  while(true)
  {  
    if (LLC_GetTaskLen() < 2)
    {
      fr1 = getFrame(DATA_SEND, sizeof(DATA_SEND), CH11, 1);
     // fr2 = getFrame(DATA_SEND, sizeof(DATA_SEND), CH11, 3);
      fr3 = getFrame(DATA_SEND, sizeof(DATA_SEND), CH11, 20);
     
      LLC_AddTask(fr1);
     // LLC_AddTask(fr2);
      LLC_AddTask(fr3);
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