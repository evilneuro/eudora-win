/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#define _WIN32_WINDOWS 0x0410
#define _WIN32_WINNT 0x0403 // For InitializeCriticalSectionAndSpinCount
#define WINVER 0x0410

#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <PSAPI.h>
#include <process.h>
#include <malloc.h>
#include <mapi.h>

// Declare these so they are accessable anywhere in the project.
extern "C" void * _ReturnAddress ( void ) ;
#pragma intrinsic ( _ReturnAddress )

// Utilize the sexy (and safe!) string functions.
// Use the library.
#define STRSAFE_LIB
// Only use character count functions.
#define STRSAFE_NO_CB_FUNCTIONS
// STRSAFE came out long after I wrote this code.  Since it's working
// and heavily tested, I'll move the functions over as I can.  Hence, 
// I don't want to depreciate everything just yet.
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <crtdbg.h>
#define NEW_INLINE_WORKAROUND  new ( _NORMAL_BLOCK,\
                                     __FILE__ , __LINE__ )
#define new NEW_INLINE_WORKAROUND
