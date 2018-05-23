// MoreHelpWnd.cpp: implementation for the More Help window.
//
// Copyright (c) 2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include "MoreHelpWnd.h"

#include "resource.h"
#include "QCSharewareManager.h"
#include "fileutil.h"
#include "guiutils.h"
#include "utils.h"

#include "DebugNewHelpers.h"


void SubstituteString(CString& strSrc, CString& strTarget, CString& strNewStr)
{
	int			iMatchLoc = -1;		// Location where strTarget occurs
	CString		strTemp = _T("");	// Temporary string
	do
	{
		iMatchLoc = strSrc.Find(strTarget);
		if (iMatchLoc != -1)
		{
			// Take the part after the replace position...
			strTemp = strSrc.Right(strSrc.GetLength() - (iMatchLoc + strTarget.GetLength()));
			// ...and the part before the replace position...
			strSrc = strSrc.Left(iMatchLoc);
			// ...add the replace string...
			strSrc += strNewStr;
			// ...and concatenate the last part of the string.
			strSrc += strTemp;
		}
	} while (iMatchLoc != -1);
}


//////////////////////////////////////////////////////////////////////
// CMoreHelpPaigeView: Paige view for More Help window
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMoreHelpPaigeView, CPaigeEdtView)

CMoreHelpPaigeView::CMoreHelpPaigeView()
{
}

CMoreHelpPaigeView::~CMoreHelpPaigeView()
{
}

void CMoreHelpPaigeView::Clear()
{
	pgSetSelection(m_paigeRef, 0, pgTextSize(m_paigeRef), 0, TRUE);
	pgDelete(m_paigeRef, NULL, best_way);
}

void CMoreHelpPaigeView::ResetCaret()
{
	pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);
}

void CMoreHelpPaigeView::HideCaret()
{
	pgSetHiliteStates(m_paigeRef, deactivate_verb, no_change_verb, TRUE);
}

void CMoreHelpPaigeView::OnInitialUpdate()
{
	CPaigeEdtView::OnInitialUpdate();

	CString		strMoreHelpHTML;

	if (!QCLoadTextData(IDT_MOREHELP_HTML, strMoreHelpHTML))
		return;

	// Substitute real content for parts the HTML file doesn't
	// really know about (actual URLs, etc.)
	CString		strTempBuf;							// Temporary buffer for doing substitutions
	CString		strSearchPattern;					// Pattern to search for
	CString		strTextToSub;						// Text to substitute in
	CString		strJumpURL;							// Base jump URL
	GetJumpURL(&strJumpURL);

	// Replace all instances of %updateurl% with actual update URL.
	strTextToSub = strJumpURL;
	ConstructURLWithQuery(strTextToSub, ACTION_UPDATE);
	strSearchPattern = "%updateurl%";
	SubstituteString(strMoreHelpHTML, strSearchPattern, strTextToSub);

	// Replace all instances of %supporturl% with actual support URL.
	strTextToSub = strJumpURL;
	ConstructURLWithQuery(strTextToSub, ACTION_SUPPORT);
	strSearchPattern = "%supporturl%";
	SubstituteString(strMoreHelpHTML, strSearchPattern, strTextToSub);

	// Replace all instances of %supporturlnotopic% with actual support URL w/out topic.
	strTextToSub = strJumpURL;
	ConstructURLWithQuery(strTextToSub, ACTION_SUPPORT_NO_TOPIC);
	strSearchPattern = "%supporturlnotopic%";
	SubstituteString(strMoreHelpHTML, strSearchPattern, strTextToSub);

	// Replace all instances of "%regcode%" with the regcode.
	strTextToSub = QCSharewareManager::GetRegCodeForCurrentMode();
	strSearchPattern = "%regcode%";
	SubstituteString(strMoreHelpHTML, strSearchPattern, strTextToSub);

	// Replace all instances of "%regfirstname%" with the registration first name.
	strTextToSub = QCSharewareManager::GetFirstNameForCurrentMode();
	strSearchPattern = "%regfirstname%";
	SubstituteString(strMoreHelpHTML, strSearchPattern, strTextToSub);

	// Replace all instances of "%regname%" with the registration name.
	strTextToSub = QCSharewareManager::GetLastNameForCurrentMode();
	strSearchPattern = "%reglastname%";
	SubstituteString(strMoreHelpHTML, strSearchPattern, strTextToSub);

	// The HTML file may contain sections only appropriate for light
	// mode or pro/adware mode.  Remove any sections that shouldn't be there.
	int			iStart = -1;
	int			iStop = -1;
	CString		strStart = "";
	CString		strStop = "";
	if (GetSharewareMode() == SWM_MODE_LIGHT)
	{
		strStart = "<!-- START REMOVE FOR LIGHT -->";
		strStop = "<!-- STOP REMOVE FOR LIGHT -->";
	}
	else
	{
		strStart = "<!-- START REMOVE FOR PROAD -->";
		strStop = "<!-- STOP REMOVE FOR PROAD -->";
	}
	do
	{
		iStart = strMoreHelpHTML.Find(strStart);
		iStop = strMoreHelpHTML.Find(strStop);
		if ((iStart != -1) && (iStop != -1))
		{
			// Take the part after the stop point...
			strTempBuf = strMoreHelpHTML.Right(strMoreHelpHTML.GetLength() - (iStop + strStop.GetLength()));
			// ...and the part before the start point...
			strMoreHelpHTML = strMoreHelpHTML.Left(iStart);
			// ...and concatenate them.
			strMoreHelpHTML += strTempBuf;
		}
	} while ((iStart != -1) && (iStop != -1));

	// Set the Paige view to readonly.
	SetReadOnly();

	// Clear the Paige view.
	Clear();

	// Import HTML from the file.
	SetAllHTML(strMoreHelpHTML, FALSE);

	// SetAllHTML() modifies the document, but it isn't really modified.
	if (m_pDocument)	m_pDocument->SetModifiedFlag(FALSE);

	// Set caret to the beginning.
	ResetCaret();
	HideCaret();

	// Update the scrollbars and scroll to the beginning.
	UpdateScrollBars(true);
	ScrollToCursor();
}


//////////////////////////////////////////////////////////////////////
// CMoreHelpDoc: Document for More Help window
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMoreHelpDoc, CDoc)

CMoreHelpDoc::CMoreHelpDoc()
{
}

CMoreHelpDoc::~CMoreHelpDoc()
{
}

BOOL CMoreHelpDoc::OnNewDocument()
{		
	if (!CDoc::OnNewDocument())
		return FALSE;

	SetTitle(CRString(IDR_MOREHELP));
	return TRUE;
}
