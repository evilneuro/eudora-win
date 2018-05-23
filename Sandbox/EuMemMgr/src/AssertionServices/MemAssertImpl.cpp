///////////////////////////////////////////////////////////////////////////////
//
// MemAssertImpl.cpp
//
// Memory Manager Assertion code (although this is generic enough
// to be used in any code).
//
// Created: 05/06/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#define __MEM_ASSERT_IMPL_CPP__

#include "MemAssertImpl.h"

#ifdef __DEBUG__
#ifdef __cplusplus
extern "C" {
#endif
AssertHandler
SetMAssertHandler(AssertHandler newHandler)
{
    AssertHandler oldHandler = MAssertHandler;
    MAssertHandler           = newHandler;
    return(oldHandler);
}
#ifdef __cplusplus
}
#endif
#endif // __DEBUG__

#ifdef __DEBUG__
#ifdef __cplusplus
extern "C" {
#endif
DbgOutputHandler
SetMDbgOutputHandler(DbgOutputHandler newHandler)
{
    DbgOutputHandler oldHandler = MDbgOutputHandler;
    MDbgOutputHandler           = newHandler;
    return(oldHandler);
}
#ifdef __cplusplus
}
#endif
#endif // __DEBUG__

#ifdef __DEBUG__
#ifdef __cplusplus
extern "C" {
#endif
MemLeakHandler
SetMMemLeakHandler(MemLeakHandler newHandler)
{
    MemLeakHandler oldHandler = MMemLeakHandler;
    MMemLeakHandler           = newHandler;
    return(oldHandler);
}
#ifdef __cplusplus
}
#endif
#endif // __DEBUG__

#ifdef __DEBUG__
#ifdef __cplusplus
extern "C" {
#endif
void _MAssert(c_char * const szExpr, c_char * const szMsg,
	      c_char * const szFile, int nLine)
{
    char buf[1024];
#ifdef WIN32
    char mesg[] =
	"\n\n\tPress OK to continue or Cancel to enter the debugger.";
#else
    char mesg[] = "";
#endif
	    
    if (szMsg != 0) {
	sprintf(buf, "\tAssertion Failure\n\t\t%s\n\t%s\n\t%s (%d)%s\n",
		szMsg, szExpr, szFile, nLine, mesg);
    }
    else {
	sprintf(buf, "\tAssertion Failure\n\t\t%s\n\t\t%s (%d)%s\n",
		szExpr, szFile, nLine, mesg);
    }
	    
    if (MAssertHandler)
	(*MAssertHandler)(buf);
    else {
	fputs(buf, stdout);
	abort();
    }
}
#ifdef __cplusplus
}
#endif
#endif // __DEBUG__

#ifdef __DEBUG__
#ifdef __cplusplus
extern "C" {
#endif
void _MAssertTyped(c_char * const szExpr, c_char * const szType,
		   c_char * const szMsg, c_char * const szFile,
		   int nLine)
{
    char buf[1024];
#ifdef WIN32
    char mesg[] =
	"\n\n\tPress OK to continue or Cancel to enter the debugger.";
#else
    char mesg[] = "";
#endif
	    
    if (szMsg != 0) {
	sprintf(buf,
	"\t%s: %s\n\texpr: %s\n\tin %s at line %d %s\n",
		szType, szMsg, szExpr, szFile, nLine, mesg);
    }
    else {
	sprintf(buf,
	"\t%s:\n\texpr: %s\n\tin %s at line %d %s\n",
		szType, szExpr, szFile, nLine, mesg);
    }
	    
    if (MAssertHandler)
	(*MAssertHandler)(buf);
    else {
	fputs(buf, stdout);
	abort();
    }
}
#ifdef __cplusplus
}
#endif
#endif // __DEBUG__


#ifdef __DEBUG__
#ifdef __cplusplus
extern "C" {
#endif
void _MDbgOutput(c_char * const message)
{
    if (MDbgOutputHandler) {
	(*MDbgOutputHandler)(message);
    }
}
#ifdef __cplusplus
}
#endif
#endif // __DEBUG__


#ifdef __DEBUG__
#ifdef __cplusplus
extern "C" {
#endif
void _MMemLeak(c_char * const message)
{
    if (MMemLeakHandler) {
	(*MMemLeakHandler)(message);
    }
}
#ifdef __cplusplus
}
#endif
#endif // __DEBUG__
