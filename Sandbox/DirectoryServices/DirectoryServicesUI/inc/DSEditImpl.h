///////////////////////////////////////////////////////////////////////////////
// DSEditImpl.h
// 
// Created: 09/12/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_EDIT_IMPL_H_
#define __DS_EDIT_IMPL_H_

// Enforce that only DSEditImpl.cpp includes it.
#ifndef __DS_EDIT_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSEditImpl.cpp
#endif

//#include "stdafx.h"     // No precompiled header.
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "font.h"

#include <xstddef>
#pragma warning (disable: 4663 4244)
#include "AutoCompleteSearcher.h"
#pragma warning (default: 4663 4244)
#include "DSUtilsIface.h"
#include "DSEditIface.h"
#include "DirectoryServicesCritSectionIface.h" // DS Template Crit Section Impl

bool DSQueryParentQueryInProgress(void *);
#endif // __DS_EDIT_IMPL_H_



