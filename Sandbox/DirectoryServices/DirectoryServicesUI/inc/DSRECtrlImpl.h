///////////////////////////////////////////////////////////////////////////////
// DSRECtrlImpl.h
// 
// Created: 09/12/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_RE_CTRL_IMPL_H_
#define __DS_RE_CTRL_IMPL_H_

// Enforce that only DSRECtrlImpl.cpp includes it.
#ifndef __DS_RE_CTRL_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSRECtrlImpl.cpp
#endif

//#include "stdafx.h"     // No precompiled header.
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "guiutils.h"
#include "font.h"
#include "urledit.h"

#include "dsapi.h"
#include "DSUtilsIface.h"
#include "DSRECtrlIface.h"
#include "DirectoryServicesCritSectionIface.h" // DS Template Crit Section Impl

bool DSQueryParentQueryInProgress(void *);
bool ShouldUpdateIni(int code = -1);

#endif // __DS_RE_CTRL_IMPL_H_



