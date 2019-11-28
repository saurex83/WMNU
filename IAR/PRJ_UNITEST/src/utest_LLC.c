#include "utest_suite.h"
#include "LLC.h"
#include "NTMR.h"
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "frame.h"
#include "RADIO.h"
#include "TIC.h"
#include "stdlib.h"

static void timealloc(void)
{
  static bool led_stat = false;
    LED(D1, led_stat);
    led_stat = !led_stat;
}

static frame_s* getFrame(uint8_t *src, uint8_t len, uint8_t CH, uint8_t TS)
{
  frame_s *fr;
  EA=0;
  fr = frame_create();
  
  fbuf_s *fb = fbuf_create(FB_RAW_LAY, src, len);
  EA=1;
  fr->meta.SEND_TIME = 0;
  fr->meta.TS = TS;
  fr->meta.CH = CH;
  frame_insert_head(fr, fb);
  return fr;
}

static void show_heap_ptr(void)
{ 
  uint16_t *heap_ptr;
  heap_ptr = (uint16_t*)malloc(1);
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "HEAP_PTR = %d\r\n", (uint16_t)heap_ptr ); 
  free(heap_ptr);
}

static void test_create()
{
  LLC_Init();
  LLC_TimeAlloc(timealloc);
  EA = 1;
  uint8_t DATA_SEND[10] = {0,1,2,3,4,5,6,7,8,9};  
  
  frame_s *fr;
  
  uint16_t TS = 0;
  uint8_t *ptr_stack = (uint8_t*)0x100;
  
  fr = getFrame(DATA_SEND, sizeof(DATA_SEND), CH11, TS);
  uint16_t fr_size = sizeof(frame_s);
  uint16_t bf_size = frame_len(fr);
  uint16_t HEAP_PTR_MAX = 0;
  uint16_t *heap_ptr;
  uint8_t nbr_bufs, nbr_frames;
  
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, 
      "Frame_s size = %d. Fbuf data = %d. Full =%d\r\n",
      fr_size, bf_size, bf_size+fr_size );
  frame_delete(fr);
  
  while(true)
  {  
    nbr_bufs = fbuf_getCount();
    nbr_frames = frame_getCount();
    if (LLC_GetTaskLen() < 20)
    {
          fr = getFrame(DATA_SEND, sizeof(DATA_SEND), CH11, TS);
        LOG(MSG_OFF | MSG_INFO | MSG_TRACE, "Create frame = %d.\r\n", (uint16_t)fr);    
          LLC_AddTask(fr);
          TS ++;
          if (TS == 49)
            TS = 0;
    }   
    if (*ptr_stack != 0xcd) // Контроль переполнения стека
      while(1);
    EA=0;
    heap_ptr = (uint16_t*)malloc(1);
    EA=1;
    if ((uint16_t)heap_ptr > HEAP_PTR_MAX)
    {
      HEAP_PTR_MAX = (uint16_t)heap_ptr;
      LOG(MSG_ON | MSG_INFO | MSG_TRACE, 
      "HEAP_PTR = %d\r\n", HEAP_PTR_MAX );
    }
    EA=0;
    free(heap_ptr);
    EA=1;
  }
  
}

static void find_mem_problem(void)
{
  LLC_Init();
  LLC_TimeAlloc(timealloc);
  EA = 1;
  uint8_t DATA_SEND[10] = {0,1,2,3,4,5,6,7,8,9};  
  
  frame_s *fr;
  
  uint16_t TS = 0;
  uint8_t *ptr_stack = (uint8_t*)0x100;
  
  uint16_t HEAP_PTR_MAX = 0;
  uint16_t *heap_ptr;
  
  while(true)
  {  
    if (fbuf_getCount() == 0)
    {
      show_heap_ptr();
      for (uint8_t i = 0; i < 20 ; i++)
      {
          fr = getFrame(DATA_SEND, sizeof(DATA_SEND), CH11, TS);
          TS++;
          if (TS == 49)
            TS = 0;
          LLC_AddTask(fr);
      }
    }   
    
    if (*ptr_stack != 0xcd) // Контроль переполнения стека
      while(1);
  }
  
  
}

// Тест на максимальное количество фреймов 
static void test_mem(void)
{
  frame_s *FR[150];
  uint8_t DATA_SEND[10] = {0,1,2,3,4,5,6,7,8,9}; 
  
  for (uint8_t i = 0; i < 105; i++) 
    FR[i] = getFrame(DATA_SEND, sizeof(DATA_SEND), CH11, 5);
}


void suite_LLC_HW(void)
{
 // umsg_line("LLC module");
  
  // Устанавливает и поддерживает в очереде 10 пакетов.
  // Мигает зеленым светодиодом. В TS0 включается, в TS5 выключаеться
 // test_mem();
 // find_mem_problem();
  test_create();  
}