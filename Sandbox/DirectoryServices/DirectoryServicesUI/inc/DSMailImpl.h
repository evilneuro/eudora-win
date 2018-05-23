///////////////////////////////////////////////////////////////////////////////
// DSMailImpl.h
// 
// Created: 09/24/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_MAIL_IMPL_H_
#define __DS_MAIL_IMPL_H_

// Enforce that only DSMailImpl.cpp includes it.
#ifndef __DS_MAIL_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSMailImpl.cpp
#endif

//#include "stdafx.h"     // No precompiled header.
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "font.h"
#include "headertype.h"
#include "QCUtils.h"
#include "compmsgd.h"
#include "mainfrm.h"
#include "address.h"
#include "WazooWnd.h"
#include "summary.h"

#include "DSMailIface.h"
#include "DirectoryServicesCritSectionIface.h" // DS Template Crit Section Impl

#endif // __DS_MAIL_IMPL_H_
