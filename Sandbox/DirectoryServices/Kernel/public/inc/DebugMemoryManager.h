///////////////////////////////////////////////////////////////////////////////
// DebugMemoryManager.h
//
// Created: 09/10/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DEBUG_MEMORY_MANAGER_H_
#define __DEBUG_MEMORY_MANAGER_H_

#include "NewIfaceMT.h"

#ifdef _DEBUG
#define DEBUG_NEW_DIRSERV new(THIS_FILE, __LINE__)
#endif

#define BASED_CODE

#endif // __DEBUG_MEMORY_MANAGER_H_
