// trace.c

#pragma warning(disable : 4115 4201 4214 4514)
#include <windows.h>
#pragma warning(default : 4115 4201 4214)
#include <stdio.h>
#include <stdarg.h>

#include "trace.h"

#ifdef DEBUG

// in ndebug mode, this is defined as ((void)0)
void __cdecl Trace(LPSTR pszFmt, ...)
{
    va_list args;
    char buffer[512];

    va_start(args, pszFmt);
    _vsnprintf(buffer, sizeof(buffer), pszFmt, args);
    OutputDebugString(buffer);
    va_end(args);
}

#endif

