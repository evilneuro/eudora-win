///////////////////////////////////////////////////////////////////////////////
// DSResultsTocImpl.h
// 
// Created: 10/25/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_RESULTS_TOC_IMPL_H_
#define __DS_RESULTS_TOC_IMPL_H_

// Enforce that only DSResultsTocImpl.cpp includes it.
#ifndef __DS_RESULTS_TOC_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSResultsTocImpl.cpp
#endif

//#include "stdafx.h"     // No precompiled header.
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "font.h"

#include "DSUtilsIface.h"
#include "DSResultsTocIface.h"
#include "DSCritSectionIface.h" // DS Template Crit Section Impl

typedef struct _rec_node {
    IDSPRecord *data;
    _rec_node  *next;
} RecNode, *PRecNode;

PRecNode pRecHead = 0, pRecCur = 0;

#endif // __DS_RESULTS_TOC_IMPL_H_



