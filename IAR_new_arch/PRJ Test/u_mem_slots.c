#include "debug.h"
#include "mem_slots.h"



//bool SL_zone_check();
//int SL_available();
//int SL_busy();
//void SL_free(char *buff);
//char* SL_alloc(void);

void check_create_delete(void){
  int busy = SL_busy();
  int available = SL_available();
  char *buff = SL_alloc();
  if (SL_busy() == 1)
    printf("Slot create ok\r\n");
  else {
    printf("Slot create fails\r\n");
    return;
  }
  SL_free(buff); 
  if (SL_busy() == 0)
    printf("Slot delete ok\r\n");
  else {
    printf("Slot delete fails\r\n");
    return;
  }
}

void check_max_alloc(void){
  int max = 0;
  char *buff[25];
  char *ret;
  while ((ret = SL_alloc())!=NULL){
    buff[max] = ret; 
    max++;
  }

  printf("Max alloc slot %d", max);
  
  for (int i = 0; i < max; i++){
    SL_free(buff[i]);
  }
   if (SL_busy() == 0)
    printf("Slot delete ok\r\n");
  else {
    printf("Slot delete fails\r\n");
    return;
  }
}

void aasert_test(void){
  char *buff = SL_alloc();
  SL_free(buff);
 // SL_free(buff); // Повторное удаление буфера
  
  char *bad_buff = NULL; // Удаление невыделенного буфера
  SL_free(bad_buff); 
}

void red_zones(void){
int res = SL_zone_check();
char *buff = SL_alloc();
buff = SL_alloc();
buff = SL_alloc();
buff = SL_alloc();
buff = SL_alloc();
buff = SL_alloc();
buff[150] = 34;
res = SL_zone_check();
}

void u_mem_slots(void){
//aasert_test();
  red_zones();
check_create_delete();
check_max_alloc();
}