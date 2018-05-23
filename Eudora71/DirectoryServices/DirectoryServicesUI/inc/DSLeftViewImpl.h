///////////////////////////////////////////////////////////////////////////////
// DSLeftViewImpl.h
// 
// Created: 10/09/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_LEFT_VIEW_IMPL_H_
#define __DS_LEFT_VIEW_IMPL_H_

// Enforce that only DSLeftViewImpl.cpp includes it.
#ifndef __DS_LEFT_VIEW_INTERFACE_IMPLEMENTATION_
#error Error: This file is to be included only in
#error Error: DSLeftViewImpl.cpp
#endif

//#include "stdafx.h"     // No precompiled header.
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "font.h"
#include "guiutils.h"
#include "urledit.h"
#include "headertype.h"
#include "WazooWnd.h"
#include "DSUtilsIface.h"
#include "DSListCtrlIface.h"
#include "DSRECtrlIface.h"
#include "DSSplitterWndIface.h"
#include "DSUtilsIface.h"
#include "DSMailIface.h"

#include <xstddef>
#pragma warning (disable: 4663 4244)
#include "AutoCompleteSearcher.h"
#pragma warning (default: 4663 4244)
#include "DSEditIface.h"

#include "DSRightViewIface.h"
#include "DSLeftViewIface.h"
#include "DSMainSplitterWndIface.h"
#include "DSCritSectionIface.h" // DS Template Crit Section Impl

void DSResetLongResults(void *);
void DSAddStringLongResults(void *, void *);
void DSAddStringLongResultsFormatted(void *, void *);
void DSAppriseItemChanged(int, void *);
void StartQueryCB(LPVOID pvContext);
void WaitRecordCB(LPVOID pvContext);
void ExportToBufferCB(char *s, void *pvContext);
void DSKeyCB(void *pvContext, WPARAM wParam);
void AddThisHitCB(void *pvContext);
void DSHandleTabFromResultsToc(BOOL, void *);
void DSHandleTabFromResultsLong(BOOL, void *);
void DSActivateQueryInput(void *);
bool DSQueryParentQueryInProgress(void *);
void DSAddStringNoNewLine(char *s, void *);
void DSGetResultsLong(CString&, void *);

bool ShouldUpdateIni(int code = -1);

#endif // __DS_LEFT_VIEW_IMPL_H_
