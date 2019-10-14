/*******************************************************************************
 *
 * Frequently used macros and symbols for runtime library.
 *
 * Copyright 2004-2005 IAR Systems. All rights reserved.
 *
 * $Revision: 10083 $
 *
 ******************************************************************************/

#include <iar_check_symbols.h>
#include <iar_cfi.h>

;=================;
; Common defines  ; 
;=================;

#define XSP_L ?XSP
#define XSP_H ?XSP+1

DPTR_MASK_SYMBOLS MACRO
         EXTERN ?DPMASK
?DPMASK0 EQU ((?DPMASK ^ (?DPMASK - 1)) & ?DPMASK)
?DPTMP1  EQU (?DPMASK ^ ?DPMASK0)
?DPMASK1 EQU ((?DPTMP1 ^ (?DPTMP1 - 1)) & ?DPTMP1)
?DPTMP2  EQU (?DPMASK ^ ?DPMASK0 ^ ?DPMASK1)
?DPMASK2 EQU ((?DPTMP2 ^ (?DPTMP2 - 1)) & ?DPTMP2)
        ENDM

;=================;
; Common Macros   ; 
;=================;
EXTERNS_FOR_ALL_DPTR_SYMBOLS MACRO
#ifdef __EXTENDED_DPTR__
	EXTERN	?DPX
#endif ; __EXTENDED_DPTR__
#if (__NUMBER_OF_DPTRS__ > 1)
        EXTERN  ?DPS
        DPTR_MASK_SYMBOLS()
#ifdef __DPTR_SEPARATE__
	EXTERN	?DPL1
	EXTERN	?DPH1
#ifdef __EXTENDED_DPTR__
	EXTERN	?DPX1
#endif ; __EXTENDED_DPTR__
#if (__NUMBER_OF_DPTRS__ > 2)
	EXTERN	?DPL2
	EXTERN	?DPH2
#ifdef __EXTENDED_DPTR__
	EXTERN	?DPX2
#endif ; __EXTENDED_DPTR__
#endif ; __NUMBER_OF_DPTRS__ > 2
#if (__NUMBER_OF_DPTRS__ > 3)
	EXTERN	?DPL3
	EXTERN	?DPH3
#ifdef __EXTENDED_DPTR__
	EXTERN	?DPX3
#endif ; __EXTENDED_DPTR__
#endif ; __NUMBER_OF_DPTRS__ > 3
#if (__NUMBER_OF_DPTRS__ > 4)
	EXTERN	?DPL4
	EXTERN	?DPH4
#ifdef __EXTENDED_DPTR__
	EXTERN	?DPX4
#endif ; __EXTENDED_DPTR__
#endif ; __NUMBER_OF_DPTRS__ > 4
#if (__NUMBER_OF_DPTRS__ > 5)
	EXTERN	?DPL5
	EXTERN	?DPH5
#ifdef __EXTENDED_DPTR__
	EXTERN	?DPX5
#endif ; __EXTENDED_DPTR__
#endif ; __NUMBER_OF_DPTRS__ > 5
#if (__NUMBER_OF_DPTRS__ > 6)
	EXTERN	?DPL6
	EXTERN	?DPH6
#ifdef __EXTENDED_DPTR__
	EXTERN	?DPX6
#endif ; __EXTENDED_DPTR__
#endif ; __NUMBER_OF_DPTRS__ > 6
#if (__NUMBER_OF_DPTRS__ > 7)
	EXTERN	?DPL7
	EXTERN	?DPH7
#ifdef __EXTENDED_DPTR__
	EXTERN	?DPX7
#endif ; __EXTENDED_DPTR__
#endif ; __NUMBER_OF_DPTRS__ > 7
#endif ; __DPTR_SEPARATE__
#endif ; __NUMBER_OF_DPTRS__ > 1
        ENDM

#if (__NUMBER_OF_DPTRS__ > 1)

TOGGLE_DPTR MACRO
        EXTERN  ?DPS
#if ((defined(__INC_DPSEL_SELECT__)) && (__NUMBER_OF_DPTRS__ == 2))
	INC	?DPS
#else
        XRL     ?DPS,#(?DPMASK0)
#endif
        ENDM

SELECT_DPTR0 MACRO
        EXTERN  ?DPS
        ANL	?DPS,#LOW(~(?DPMASK2 | ?DPMASK1 | ?DPMASK0))
        ENDM

; Initial
;  value 
; for DPS
;   000
;          XOR 001   (0->1)
;   001
;          XOR 011   (1->2)
;   010
;          XOR 001   (2->3)
;   011
;          XOR 111   (3->4)
;   100
;          XOR 001   (4->5)
;   101
;          XOR 011   (5->6)
;   110
;          XOR 001   (6->7)
;   111

DPTR0_TO_DPTR1 MACRO
        EXTERN  ?DPS
#if (defined(__INC_DPSEL_SELECT__))
	INC	?DPS
#else
        XRL	?DPS,#(?DPMASK0)
#endif
        ENDM

DPTR0_TO_DPTR2 MACRO
        EXTERN  ?DPS
        XRL	?DPS,#(?DPMASK1)
        ENDM

DPTR0_TO_DPTR3 MACRO
        EXTERN  ?DPS
        XRL	?DPS,#(?DPMASK1 | ?DPMASK0)
        ENDM

DPTR0_TO_DPTR4 MACRO
        EXTERN  ?DPS
        XRL	?DPS,#(?DPMASK2)
        ENDM

DPTR0_TO_DPTR5 MACRO
        EXTERN  ?DPS
        XRL	?DPS,#(?DPMASK2 | ?DPMASK0)
        ENDM

DPTR0_TO_DPTR6 MACRO
        EXTERN  ?DPS
        XRL	?DPS,#(?DPMASK2 | ?DPMASK1)
        ENDM

DPTR0_TO_DPTR7 MACRO
        EXTERN  ?DPS
        XRL	?DPS,#(?DPMASK2 | ?DPMASK1 | ?DPMASK0)
        ENDM

DPTR1_TO_DPTR2 MACRO
        EXTERN  ?DPS
#if (defined(__INC_DPSEL_SELECT__))
	INC	?DPS
#else
        XRL	?DPS,#(?DPMASK1 | ?DPMASK0)
#endif
        ENDM

DPTR2_TO_DPTR3 MACRO
        EXTERN  ?DPS
#if (defined(__INC_DPSEL_SELECT__))
	INC	?DPS
#else
        XRL	?DPS,#(?DPMASK0)
#endif
        ENDM

DPTR3_TO_DPTR4 MACRO
        EXTERN  ?DPS
#if (defined(__INC_DPSEL_SELECT__))
	INC	?DPS
#else
        XRL	?DPS,#(?DPMASK2 | ?DPMASK1 | ?DPMASK0)
#endif
        ENDM

DPTR4_TO_DPTR5 MACRO
        EXTERN  ?DPS
#if (defined(__INC_DPSEL_SELECT__))
	INC	?DPS
#else
        XRL	?DPS,#(?DPMASK0)
#endif
        ENDM

DPTR5_TO_DPTR6 MACRO
        EXTERN  ?DPS
#if (defined(__INC_DPSEL_SELECT__))
	INC	?DPS
#else
        XRL	?DPS,#(?DPMASK1 | ?DPMASK0)
#endif
        ENDM

DPTR6_TO_DPTR7 MACRO
        EXTERN  ?DPS
#if (defined(__INC_DPSEL_SELECT__))
	INC	?DPS
#else
        XRL	?DPS,#(?DPMASK0)
#endif
        ENDM

#endif ; __NUMBER_OF_DPTRS__ > 1

