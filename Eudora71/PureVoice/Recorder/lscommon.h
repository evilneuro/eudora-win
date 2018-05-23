#ifndef _LSCOMMON_H_
#define _LSCOMMON_H_

#include "LsDefines.h"

// GLOBAL TYPEDEF
typedef char			Int8;
typedef unsigned char	uInt8;
typedef short			Int16;
typedef unsigned short	uInt16;
typedef long			Int32;
typedef unsigned long	uInt32;

typedef int LsStateType;

// GLOBAL ENUM
enum {
	UNIT_TIME_MS,
	UNIT_SAMPLES,
	UNIT_BYTES,
	UNIT_PACKETS		// valid only for CODEC
};

//	GLOABL DEFINES
#define LS_RANGE( x, min, max )	if ((x)<(min)) x=min;\
								else if ((x)>(max)) x=max;

#endif