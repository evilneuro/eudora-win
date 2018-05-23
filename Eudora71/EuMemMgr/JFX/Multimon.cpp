#include "stdafx.h"


// This file includes the multimon stub functions that are implmented in multimon.h.

// This is only necessary if the app is supporting versions of Windows before
// Windows 2000 - specifically Windows 98 & ME.
#if !defined(COMPILE_MULTIMON_STUBS) && (WINVER < 0x0500)

// This needs to be here so that we get the wrapper stubs that deal with pre-Windows 2000 platforms
#define COMPILE_MULTIMON_STUBS
#include "multimon.h"

#endif // !defined(COMPILE_MULTIMON_STUBS) && (WINVER < 0x0500)

