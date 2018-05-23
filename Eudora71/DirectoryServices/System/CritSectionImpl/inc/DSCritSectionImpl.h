///////////////////////////////////////////////////////////////////////////////
// DSCritSectionImpl.h
// 
// Created: 08/13/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DIRECTORYSERVICES_CRIT_SECTION_IMPL_H_
#define __DIRECTORYSERVICES_CRIT_SECTION_IMPL_H_

// Enforce that a file other than NewImpl.c does not include it.
#ifndef __DIRECTORYSERVICES_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSCritSectionImpl.cpp.
#endif

#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)

#include "DSCritSectionIface.h"
CRITICAL_SECTION csDirectoryServicesCriticalSection;

#endif // __DIRECTORYSERVICES_CRIT_SECTION_IMPL_H_


