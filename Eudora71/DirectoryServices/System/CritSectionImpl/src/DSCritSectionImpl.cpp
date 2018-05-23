///////////////////////////////////////////////////////////////////////////////
// DSCritSectionImpl.cpp
//
// Created: 07/08/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DIRECTORYSERVICES_INTERFACE_IMPLEMENTATION_
#include "DSCritSectionImpl.h"

UINT DirectoryServicesCritSectionMT::nRefCnt = 0;

DirectoryServicesCritSectionMT::DirectoryServicesCritSectionMT()
{
    if (nRefCnt == 0)
	InitializeCriticalSection(&csDirectoryServicesCriticalSection);
    nRefCnt++;
}

DirectoryServicesCritSectionMT::~DirectoryServicesCritSectionMT()
{
    nRefCnt--;
    if (nRefCnt == 0)
	DeleteCriticalSection(&csDirectoryServicesCriticalSection);
}

void
DirectoryServicesCritSectionMT::EnterMT()
{
    EnterCriticalSection(&csDirectoryServicesCriticalSection);
}

void
DirectoryServicesCritSectionMT::LeaveMT()
{
    LeaveCriticalSection(&csDirectoryServicesCriticalSection);
}
