///////////////////////////////////////////////////////////////////////////////
// DMMDumpHandler.h
//
// This header file provides the interface (and implementation) of a generic
// output handler for the DMM. The idea is that the address of an object of
// this class can be passed as the first argument to
// 
//     DumpBlocksLogByBlockAndFileMT() // see MemIfaceMT.h
//
// after having been cast to a void pointer. When the above function calls
// the callback function, the same void pointer will be passed back as the
// first argument of the callback function. Then the void pointer could be
// cast back to a DMMDumpHandler * and ProcessOutput() called with the
// dump message.
//
// Created:  05/25/97 smohanty
// Modified: 01/11/98 smohanty 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DMM_DUMP_HANDLER_IFACE_H_
#define __DMM_DUMP_HANDLER_IFACE_H_

#ifdef WIN32
#pragma warning(disable : 4115 4201 4214 4514)
#include <windows.h>
#pragma warning(default : 4115 4201 4214)
#elif defined(__unix__)
#include <iostream.h>
#endif

#include "EudoraBaseTypes.h"
#include "MemDbgDefIface.h"

class DMMDumpHandler {

public:
    // Ctor/dtor.
     DMMDumpHandler() {}
    ~DMMDumpHandler() {}
    
    // Output.
#ifdef __DEBUG__
    void ProcessOutput(c_char * const buf)
#else
    void ProcessOutput(c_char * const)
#endif
    {
#if defined(__DEBUG__)
#if defined(WIN32)
	OutputDebugString(buf);
#elif defined(__unix__)
	cout << buf << flush;
#else
#if defined(WIN32)
#error Developer, please provide implementation of output stream \
for DMM logs for your platform...
#else
#error Developer, please provide implementation of output stream
#error for DMM logs for your platform...
#endif
#endif
#endif
    }
};

typedef DMMDumpHandler DMM_DH;

#endif // __DMM_DUMP_HANDLER_IFACE_H_
