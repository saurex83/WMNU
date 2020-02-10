#include "debug.h"
#include "frame.h"
#include "cpu.h"


void create(void){
struct frame *fr = FR_create();

printf("frame busy %d",FR_busy());
printf("frame available %d",FR_available());

FR_delete(fr);

printf("frame busy %d",FR_busy());
printf("frame available %d",FR_available());

fr = FR_create();

char tst[] = {1,2,3,4,5,6,7,8,9,10};
FR_add_header(fr, tst, sizeof(tst));
FR_del_header(fr, 5);

ATOMIC_BLOCK_RESTORE{
printf("frame busy %d",FR_busy());
printf("frame available %d",FR_available());
}
ATOMIC_BLOCK_FORCEON{
FR_delete(fr);
}

}

void u_frame(void){
    create();
}