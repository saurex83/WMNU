/**************************************************
 *
 * This file contains human readable definitions
 * for icc8051 runtime model definitions.
 *
 * The 8051 C/EC++ compiler will define these
 * symbols by default. These symbols can be used
 * to check against when writing code that needs
 * to work on several runtime models.
 * 
 * NOTE: The check for existence of the symbols is
 *       useful when you include this file from
 *       the 8051 assembler file that was generated
 *       by the 8051 compiler. These symbols must
 *       then be given as '-D' defines on the
 *       assembler command line.
 * 
 * Copyright 2004-2005 IAR Systems. All rights reserved.
 *
 * $Revision: 6265 $
 *
 **************************************************/

#ifdef __IAR_SYSTEMS_ASM__

#ifndef __CORE__
#error "You must define __CORE__"
#endif
/* To be tested with __CORE__ */
#define __CORE_TINY__      0
#define __CORE_PLAIN__     1
#define __CORE_EXTENDED1__ 2
#define __CORE_EXTENDED2__ 3

#ifndef __CODE_MODEL__
#error "You must define 'Code model' with: __CODE_MODEL__"
#endif
/* To be tested with __CODE_MODEL__ */
#define __CM_TINY__   0
#define __CM_NEAR__   1
#define __CM_BANKED__ 2
#define __CM_FAR__    3

#ifndef __DATA_MODEL__
#error "You must define __DATA_MODEL__"
#endif
/* To be tested with __DATA_MODEL__ */
#define __DM_TINY__        0
#define __DM_SMALL__       1
#define __DM_LARGE__       2
#define __DM_GENERIC__     3
#define __DM_FAR__         4
#define __DM_FAR_GENERIC__ 5

#ifndef __CALLING_CONVENTION__
#error "You must define __CALLING_CONVENTION__"
#endif
/* To be tested with __CALLING_CONVENTION__ */
#define __CC_DO__   0
#define __CC_IO__   1
#define __CC_IR__   2
#define __CC_PR__   3
#define __CC_XR__   4
#define __CC_ER__   5
/* Obsoletified!
define __DATA_OVERLAY__         0
#define __IDATA_OVERLAY__        1
#define __IDATA_REENTRANT__      2
#define __PDATA_REENTRANT__      3
#define __XDATA_REENTRANT__      4
#define __EXT_STACK_REENTRANT__  5
*/
#ifndef __NUMBER_OF_DPTRS__
#error "You must define 'Nr of data pointers' (X) with: __NUMBER_OF_DPTRS__"
#endif

#endif /*__IAR_SYSTEMS_ASM__*/
