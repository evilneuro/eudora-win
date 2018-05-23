///////////////////////////////////////////////////////////////////////////////
// DSSplitterWndImpl.h
// 
// Created: 09/12/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_SPLITTER_WND_IMPL_H_
#define __DS_SPLITTER_WND_IMPL_H_

// Enforce that only DSListCtrlImpl.cpp includes it.
#ifndef __DS_SPLITTER_WND_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSSplitterWndImpl.cpp
#endif

//#include "stdafx.h"     // No precompiled header.
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "font.h"

#include "DSUtilsIface.h"
#include "DSSplitterWndIface.h"
#include "DirectoryServicesCritSectionIface.h" // DS Template Crit Section Impl

bool ShouldUpdateIni(int code = -1);

#endif // __DS_SPLITTER_WND_IMPL_H_
