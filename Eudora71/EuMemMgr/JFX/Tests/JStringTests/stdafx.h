/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#pragma once

#define _CRTDBG_MAP_ALLOC
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <crtdbg.h>
#define NEW_INLINE_WORKAROUND  new ( _NORMAL_BLOCK,\
                                     __FILE__ , __LINE__ )
#define new NEW_INLINE_WORKAROUND

// TODO: reference additional headers your program requires here
