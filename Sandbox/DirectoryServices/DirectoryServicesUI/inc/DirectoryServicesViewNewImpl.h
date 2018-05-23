///////////////////////////////////////////////////////////////////////////////
// DirectoryServicesViewNewImpl.h
// 
// Created: 10/09/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DIRECTORYSERVICES_VIEWNEW_IMPL_H_
#define __DIRECTORYSERVICES_VIEWNEW_IMPL_H_

// Enforce that only DirectoryServicesViewNewImpl.cpp includes it.
#ifndef __DIRECTORYSERVICES_VIEWNEW_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DirectoryServicesViewNewImpl.cpp
#endif

//#include "stdafx.h"        // No precompiled header.
#include "rs.h"            // Resource strings methods.
#include "resource.h"      // Resource definitions.
#include "utils.h"
#include "font.h"
//#include "bigedit.h"
#include "DSPrintEdit.h"
#include "WazooWnd.h"
#include "DSUtilsIface.h"
#include "DSMailIface.h"
#include "DSEditIface.h"
#include "DSSplitterWndIface.h"
#include "DSMainSplitterWndIface.h"
#include "DSLeftViewIface.h"
#include "DSRightViewIface.h"

#ifdef new
#undef new
#endif 

#include "DirectoryServicesViewNewIface.h"     // DSWazooWnd Interface.
#include "DirectoryServicesCritSectionIface.h" // DS Template Crit Section Impl

extern
BOOL MakeNickname();

void
DSSetPrintingPreviewingFlag(void *data, bool state);

#endif // __DIRECTORYSERVICES_VIEWNEW_IMPL_H_
