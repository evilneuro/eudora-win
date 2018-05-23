///////////////////////////////////////////////////////////////////////////////
// DSGenericListCtrlImpl.h
// 
// Created: 10/25/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_GENERIC_LIST_CTRL_IMPL_H_
#define __DS_GENERIC_LIST_CTRL_IMPL_H_

// Enforce that only DSGenericListCtrlImpl.cpp includes it.
#ifndef __DS_GENERIC_LIST_CTRL_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSGenericListCtrlImpl.cpp
#endif

//#include "stdafx.h"     // No precompiled header.
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "font.h"

#include "DSUtilsIface.h"
#include "DSGenericListCtrlIface.h"
#include "DSCritSectionIface.h" // DS Template Crit Section Impl

#endif // __DS_GENERIC_LIST_CTRL_IMPL_H_



