///////////////////////////////////////////////////////////////////////////////
// DSRightViewImpl.h
// 
// Created: 10/09/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_RIGHT_VIEW_IMPL_H_
#define __DS_RIGHT_VIEW_IMPL_H_

// Enforce that only DSRightViewImpl.cpp includes it.
#ifndef __DS_RIGHT_VIEW_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSRightViewImpl.cpp
#endif

//#include "stdafx.h"     // No precompiled header.
#include <objbase.h>    // for CoCreateInstance.
#include <initguid.h>
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "font.h"
#include "guiutils.h"

#ifdef new
#undef new
#endif 

#include "WazooWnd.h"
#include "DSUtilsIface.h"
#include "DSMailIface.h"
#include "DSSplitterWndIface.h"
#include "DSEditIface.h"
#include "DSLeftViewIface.h"
#include "DSRightViewIface.h"
#include "DSMainSplitterWndIface.h"
#include "DSCritSectionIface.h" // DS Template Crit Section Impl
#include "RegisterComponentIface.h"

const DWORD PRE_IE_FOUR_OH_HEADER_SIZE = 40;

const char *
_ds_uds = "Directory Services unavailable during this session due to unsuccessful initialization. Please ensure that \n\tDirServ.dll\n\tISock.dll\n\tPh.dll\n\tLdap.dll\n\tEudoraBk.dll are in the Eudora working folder and restart.";

bool
ShouldUpdateIni(int code = -1);

#endif // __DS_RIGHT_VIEW_IMPL_H_
