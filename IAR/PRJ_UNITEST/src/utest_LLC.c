#include "utest_suite.h"
#include "LLC.h"
#include "NTMR.h"
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "frame.h"
#include "RADIO.h"
#include "TIC.h"
#include "mem.h"

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
  frame_addHeader(fr, src, len);
  fr->meta.SEND_TIME = 0;
  fr->meta.TS = TS;
  fr->meta.CH = CH;
  return fr;
}

static void show_heap_ptr(void)
{ 
  uint16_t *heap_ptr;
  heap_ptr = (uint16_t*)re_malloc(1);
  LOG_ON("HEAP_PTR = %d", (uint16_t)heap_ptr ); 
  re_free(heap_ptr);
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
  uint16_t heap;
  uint8_t nbr_bufs, nbr_frames;
  
  LOG_ON("Frame_s size = %d. Fbuf data = %d. Full =%d",
      fr_size, bf_size, bf_size+fr_size );
  frame_delete(fr);
  
  while(true)
  {  
    nbr_frames = frame_getCount();
    LOG_OFF("nbr_frames = %d" ,nbr_frames );
    
    if (LLC_GetTaskLen() < 20)
    {
      fr = getFrame(DATA_SEND, sizeof(DATA_SEND), CH11, TS);
      LOG_OFF("Create frame = %d", (uint16_t)fr);    
      LLC_AddTask(fr);
      TS ++;
      if (TS == 49)
      TS = 0;
    }   
    if (*ptr_stack != 0xcd) // Контроль переполнения стека
      while(1);
   
    heap =  heap_ptr(10);
    if (heap > HEAP_PTR_MAX)
    {
      HEAP_PTR_MAX = heap;
      LOG_ON("HEAP_PTR = %d", HEAP_PTR_MAX );
    }
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
    if (frame_getCount() == 0)
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
  
  // Устанавливает и поддерживает в очереди 20 пакетов.
  // Для приема пакетов испольуеться второй модуль с прошивкой RadioRecvTest1
  test_create(); 
 // test_mem();
 // find_mem_problem();
   
}