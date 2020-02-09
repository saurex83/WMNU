#include "debug.h"
#include "stddef.h"
#include "stdint.h"
#include "macros.h"

#define TIMEOUT_EXCP 1
#define UNHANDLED_EXCP 2

void u_macros(void){
  int test[] = {1};
  
  struct abc{
  int a;
  int b;
  int c;
  };
  
  struct abc oleg;
  struct abc *y;
  
  int *c = &oleg.c;
  y = container_of(c, struct abc, c);
  
  for_each_type(int, test, item){ 
    printf("%d ",*item);
  };
  printf("\r\n");
  
  printf("Try/CATCH macro \r\n");
  TRY{
    y = &oleg;
    THROW(UNHANDLED_EXCP);
    printf("Code run!\r\n");
  }
  CATCH(TIMEOUT_EXCP){
    printf("Exception catch!\r\n");
  }
  FINALLY{
    printf("Finnaly!\r\n");
  }
  ETRY;
}