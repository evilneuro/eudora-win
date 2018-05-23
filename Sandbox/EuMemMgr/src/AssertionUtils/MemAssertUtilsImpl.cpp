///////////////////////////////////////////////////////////////////////////////
//
// MemAssertUtilsImpl.cpp
//
// Memory Manager Assertion System Support.
//
// Created:  05/06/97 smohanty
// Modified: 01/15/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#define __MEM_ASSERT_UTILS_IMPL_CPP__

#include "MemAssertUtilsImpl.h"

#ifdef __DEBUG__
void
MemAssertFail(c_char * const msg)
{
#if defined(WIN32)
    int nRet = MessageBox(NULL, msg, "Assertion Failure",
			  MB_OKCANCEL | MB_ICONSTOP);
    if (nRet == IDCANCEL) {
	DebugBreak();
    }
#elif defined(__unix__)
    fprintf(stdout, "%s\n", _strSeparator);
    fprintf(stdout, "Assertion failure\n");
    fprintf(stdout, "%s", msg);
    fprintf(stdout, "%s\n", _strSeparator);
#else
#error Developer, please provide implementation for reporting
#error assertion failure for your platform ...
#endif // WIN32
}
#endif // __DEBUG__

#ifdef __DEBUG__
void
MemOutputMessage(c_char * const msg)
{
#if defined(WIN32)
    OutputDebugString(_strSeparator);
    OutputDebugString("\n");
    OutputDebugString(msg);
    OutputDebugString(_strSeparator);
    OutputDebugString("\n");
#elif defined(__unix__)
    fprintf(stdout, "%s\n", _strSeparator);
    fprintf(stdout, "%s", msg);
    fprintf(stdout, "%s\n", _strSeparator);
#else
#error Developer, please provide implementation for reporting
#error Debug Memory Manager messages for your platform ...
#endif // WIN32
}
#endif // __DEBUG__


#ifdef __DEBUG__
void
MemLeakOutputMessage(c_char * const msg)
{
#if defined(WIN32)
//    OutputDebugString(_strSeparator);
//    OutputDebugString("\n");
    OutputDebugString(msg);
//    OutputDebugString(_strSeparator);
//    OutputDebugString("\n");
#elif defined(__unix__)
    fprintf(stdout, "%s", msg);
#else
#error Developer, please provide implementation for reporting
#error memory leaks messages for your platform ...
#endif // WIN32
}
#endif // __DEBUG__
