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
  
  frame_s *fr;
  
  uint16_t TS = 0;
  
  fr = getFrame(DATA_SEND, sizeof(DATA_SEND), CH11, TS);
  uint16_t fr_size = sizeof(frame_s);
  uint16_t bf_size = frame_len(fr);
  
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, 
      "Frame_s size = %d. Fbuf data = %d. Full =%d\r\n",
      fr_size, bf_size, bf_size+fr_size );
  frame_delete(fr);
  
  while(true)
  {  
    if (LLC_GetTaskLen() < 5)
    {
          fr = getFrame(DATA_SEND, sizeof(DATA_SEND), CH11, TS);
          LLC_AddTask(fr);
          TS ++;
          if (TS == 49)
            TS = 0;
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