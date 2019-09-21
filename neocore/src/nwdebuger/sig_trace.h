#include "sig_trace_defs.h"

#define SFD_DETECTED 1


#if (SFD_DETECTED==1)
	#define SIG_SET_SFD_DETECTED() SIG1_HIGH
	#define SIG_CLR_SFD_DETECTED() SIG1_LOW
	#define SIG_PULSE_SFD_DETECTED() SIG1_PULSE
#else
	#define SIG_SET_SFD_DETECTED()
	#define SIG_CLR_SFD_DETECTED()
	#define SIG_PULSE_SFD_DETECTED()
#endif