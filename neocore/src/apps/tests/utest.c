#include "nwdebuger.h"
#include "stdlib.h"
#include "printf.h"

void main()
{
		//char * buffer = (char*) malloc(200); 
		//free(buffer);
		//tfp_printf("dd");
		LOG(MSG_ON | MSG_ALARM | MSG_TRACE, "defege");
		 int x=5;

		//ASSERT(x,"hello %s", "wer");
		ASSERT_HALT(x,"hello %s", "wer");
		SIG_SET_SFD_DETECTED();
}
