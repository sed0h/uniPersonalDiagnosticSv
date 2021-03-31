/*
 * xif_safe.c
 *
 *  Created on: May 17, 2020
 *      Author: yusw
 */

#include "xif_safe.h"

//InterlockedCompareExchange(__inout LONG volatile * Target,__in LONG Exchange, __in LONG Comperand);

#if defined(WIN32)
bool CAS(volatile uint32_t* val, uint32_t ov, uint32_t nv){
	LONG ret = InterlockedCompareExchange(val, nv, ov);
	return ret == ov;
}
#endif
