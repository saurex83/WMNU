/*******************************************************************************
 *
 * Consistency check for runtime library building symbols.
 *
 * Copyright 2003,2005 IAR Systems. All rights reserved.
 *
 * $Revision: 6732 $
 *
 ******************************************************************************/

#ifndef __ICC8051__
#ifndef __CORE__
#error "You must define __CORE__"
#endif
#define __CORE_TINY__      0
#define __CORE_PLAIN__     1
#define __CORE_EXTENDED1__ 2
#define __CORE_EXTENDED2__ 3

#ifndef __CODE_MODEL__
#error "You must define 'Code model' with: -D__CODE_MODEL__"
#endif
#define __CM_TINY__        0
#define __CM_NEAR__        1
#define __CM_BANKED__      2
#define __CM_FAR__         3
#define __CM_BANKED_EXT2__ 4

#ifndef __DATA_MODEL__
#error "You must define 'data model' with: -D__DATA_MODEL__"
#endif
#define __DM_TINY__        0
#define __DM_SMALL__       1
#define __DM_LARGE__       2
#define __DM_GENERIC__     3
#define __DM_FAR__         4
#define __DM_FAR_GENERIC__ 5

#ifndef __CALLING_CONVENTION__
#error "You must define 'calling convention' with: -D__CALLING_CONVENTION__"
#endif
#define __CC_DO__   0
#define __CC_IO__   1
#define __CC_IR__   2
#define __CC_PR__   3
#define __CC_XR__   4
#define __CC_ER__   5

#ifndef __NUMBER_OF_DPTRS__
#error "You must define 'Nr of data pointers' (X) with: -D__NUMBER_OF_DPTRS__"
#endif

#if (__NUMBER_OF_DPTRS__ > 1)
  #if !( defined(__XOR_DPSEL_SELECT__) || defined(__INC_DPSEL_SELECT__) )
    #error "You must define 'DPTR Select method' with: __XOR_DPSEL_SELECT__ or __INC_DPSEL_SELECT__"
  #endif

  #if !( defined(__DPTR_SHADOWED__) || defined(__DPTR_SEPARATE__) )
    #error "You must define 'Visibility for DPTR SFRs' with: __DPTR_SHADOWED__ or __DPTR_SEPARATE__"
  #endif
#endif ; __NUMBER_OF_DPTRS__ > 1


#if (__CORE__ == __CORE_EXTENDED1__)
#undef __CODE_MODEL__
#undef __EXTENDED_DPTR__
#define __CODE_MODEL__ 3
#define __EXTENDED_DPTR__ 1
#endif
#endif

