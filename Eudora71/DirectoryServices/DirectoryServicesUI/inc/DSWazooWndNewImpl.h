///////////////////////////////////////////////////////////////////////////////
// DSWazooWndNewImpl.h
// 
// Created: 08/12/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DIRECTORYSERVICES_WAZOOWNDNEW_IMPL_H_
#define __DIRECTORYSERVICES_WAZOOWNDNEW_IMPL_H_

// Enforce that only DSWazooWndNewImpl.cpp includes it.
#ifndef __DIRECTORYSERVICES_WAZOOWNDNEW_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSWazooWndNewImpl.cpp
#endif

//#include "stdafx.h"     // No precompiled header.
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "mainfrm.h"

#include "DSMainSplitterWndIface.h"

#ifdef new
#undef new
#endif 

#include "DSViewNewIface.h"
#include "DSWazooWndNewIface.h" // DSWazooWndNew Interface.
#include "DSCritSectionIface.h" // DS Template Crit Section Impl

#endif // __DIRECTORYSERVICES_WAZOOWNDNEW_IMPL_H_




