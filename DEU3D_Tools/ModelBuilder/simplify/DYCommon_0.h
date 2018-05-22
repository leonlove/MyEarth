//////////////////////////////////////////////////////////////////////
//
// DYCommon_0.h: global definition for the tidu classes.
//
//////////////////////////////////////////////////////////////////////

#ifndef DY_COMMON_0_H
#define DY_COMMON_0_H

#ifdef _MSC_VER
#if (_MSC_VER>1200)

#pragma warning(disable:4819)
#pragma warning(disable:4995)
#pragma warning(disable:4996)

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#define POINTER_64 __ptr64

typedef void * PVOID;
typedef void * POINTER_64 PVOID64;

#endif
#endif

#endif  // #ifndef DY_COMMON_0_H
