// TOCVIEW.CPP
//
// Copyright (c) 1991-2001 by QUALCOMM, Incorporated
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

#include <afxcmn.h>
#include <afxrich.h>

#include <QCUtils.h>

#include "eudora.h"
#include "resource.h"
#include "rs.h"
#include "utils.h"
#include "guiutils.h"
#include "cursor.h"
#include "doc.h"
#include "tocdoc.h"
#include "tocview.h"
#include "font.h"
#include "msgdoc.h"
#include "mainfrm.h"
#include "compmsgd.h"
#include "SaveAsDialog.h"
#include "debug.h"
#include "fileutil.h"
#include "helpcntx.h"
#include "pop.h"
#include "MsgRecord.h"
#include "HostList.h"
#include "ClipboardMgr.h"
#include "NewMBox.h"
#include "QCProtocol.h"
#include "QCGraphics.h"
#include "PgCompMsgView.h"
#include "msgutils.h"
#include "ReadMessageDoc.h"

#include "QCCommandActions.h"
#include "QCCommandObject.h"
#include "QCCommandStack.h"
#include "QCMailboxDirector.h"
#include "QCPersonalityDirector.h"
#include "QCLabelDirector.h"

#include "QCOleDropSource.h"
#include "TocFrame.h"

#include "QCFindMgr.h"
#include "SearchEngine.h" // StringSearch
#include "SearchManager.h"

#include "Sendmail.h"

extern QCMailboxDirector		g_theMailboxDirector;

#include "DebugNewHelpers.h"

// --------------------------------------------------------------------------

void AFXAPI AfxSetWindowText(HWND hWndCtrl, LPCTSTR lpszNew); 

// --------------------------------------------------------------------------

// Characters for the message states (bullet chars have to be hand-drawn)
char StateFlag[] = 
{
	' ',	// MS_UNREAD
	' ',	// MS_READ
	'R',	// MS_REPLIED
	'F',	// MS_FORWARDED
	'D',	// MS_REDIRECT
	' ',	// MS_UNSENDABLE
	' ',	// MS_SENDABLE
	'Q',	// MS_QUEUED
	'S',	// MS_SENT
	'-',	// MS_UNSENT
	'T'		// MS_TIME_QUEUED
};

// Bitmaps preloaded for performance
CBitmap g_AttachmentBitmap;

//
// Preloaded Change Server Status menu bitmaps.
//
CBitmap g_ServerLeaveBitmap;
CBitmap g_ServerFetchBitmap;
CBitmap g_ServerDeleteBitmap;
CBitmap g_ServerFetchDeleteBitmap;

//
// Preloaded Change Priority menu bitmaps.
//
CBitmap g_PriorityLowestMenuBitmap;
CBitmap g_PriorityLowMenuBitmap;
CBitmap g_PriorityNormalMenuBitmap;
CBitmap g_PriorityHighMenuBitmap;
CBitmap g_PriorityHighestMenuBitmap;

//
// Preloaded Change Status menu bitmaps.
//
CBitmap g_StatusUnreadBitmap;
CBitmap g_StatusReadBitmap;
CBitmap g_StatusRepliedBitmap;
CBitmap g_StatusForwardedBitmap;
CBitmap g_StatusRedirectBitmap;
CBitmap g_StatusUnsendableBitmap;
CBitmap g_StatusSendableBitmap;
CBitmap g_StatusQueuedBitmap;
CBitmap g_StatusSentBitmap;
CBitmap g_StatusUnsentBitmap;
CBitmap g_StatusTimeQueuedBitmap;
CBitmap g_StatusRecoveredBitmap;

//
// Cached TOC Column Button images (static member data)
//
HBITMAP CTocHeaderButton::m_hBitmapSortByStatus = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByJunk = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByPriority = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByAttachment = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByLabel = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortBySender = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByDate = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortBySize = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByServerStatus = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByMood = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortBySubject = NULL;

UINT CTocHeaderButton::s_nSortPopupID = 0;

// Order in which to sort
static SortType g_SortMethod = NOT_SORTED;
static CTocDoc* g_SortTocDoc = NULL;

// Maximum delay when using is doing type-to-select (TTS)
static const DWORD kMaxTTSDelay = 1000;

// Space between text and column separator
static const int ColumnSpace = 4;

// Width of widest state
static int g_MaxStatusWidth = 0;


// Masks for handling information stored in m_Temp2
static const int kTemp2SelectionMask = 0x00000001;
static const int kTemp2SortOrderMask = ~kTemp2SelectionMask;


IMPLEMENT_DYNCREATE(CTocListBox, CListBox)

/////////////////////////////////////////////////////////////////////////////
// CTocListBox

CTocListBox::CTocListBox() :
	m_HorizWindowOrg(0),
	m_bSelectionResultDirectUserAction(false),
	m_SavedMouseDownPoint(-1, -1),
	m_MouseState(MOUSE_IDLE),
	m_AllowDragAndDrop(TRUE),
	m_bAllowSelChangeNotifications(true)
{
	ResetTTSPhrase();
}


CTocListBox::~CTocListBox()
{
}


void CTocListBox::ResetTTSPhrase()
{
	m_nLastKeyTime = 0;
	m_pFindPhrase[0] = '\0';
	m_nFindPhraseLen = 0;
}


void CTocListBox::OnDestroy() 
{
	CTocFrame *		pTocFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );
	ASSERT_KINDOF(CTocFrame, pTocFrame);

	//	Notify the parent CTocFrame that the sum list box is no longer available
	if (pTocFrame)
		pTocFrame->SetSumListBox(NULL);

	//	Remember that we're being destroyed so that we can ignore anything that would
	//	would normally call HandleSelectionMayHaveChanged - in particular DeleteItem.
	m_bAllowSelChangeNotifications = false;

	CListBox::OnDestroy();
}


int CTocListBox::GetIndex(const CSummary* Sum) const
{
	int i = GetCount() - 1;
	
	for (; i >= 0; i--)
	{
		if (GetItemDataPtr(i) == Sum)
			break;
	}
	
	return (i);
}


////////////////////////////////////////////////////////////////////////
// GetSelItemIndex [private]
//
// Returns index of first selected item, if any.  Otherwise, returns
// -1 if there are no selected items.
////////////////////////////////////////////////////////////////////////
int CTocListBox::GetSelItemIndex() const
{
	const int COUNT = GetCount();
	if (LB_ERR == COUNT)
		return -1;
	
	//
	// Search for first selected item.
	//
	for (int idx = 0; idx < COUNT; idx++)
	{
		if (GetSel(idx) > 0)
			return idx;
	}

	return -1;
}


////////////////////////////////////////////////////////////////////////
// SetSel [public]
//
// Overloaded version of SetSel, which allows the caller to specify
// whether or not the call is the result of direct user action.
//
// Returns LB_ERR if an error occurs.
////////////////////////////////////////////////////////////////////////
int CTocListBox::SetSel(int in_nIndex, BOOL in_bSelect, bool in_bResultDirectUserAction)
{
	// Set whether or not a given programmatic selection change is the result of
	// direct user action. By setting just before calling CListBox::SetSel we ensure
	// that OnSetSel will see the correct value (OnSetSel is called immediately).
	SetSelectionResultDirectUserAction(in_bResultDirectUserAction);

	int		nResult = CListBox::SetSel(in_nIndex, in_bSelect);

	// Revert back to false so that any other calls to SetSel default to not being
	// the result of direct user action.
	SetSelectionResultDirectUserAction(false);

	return nResult;
}

//
//	CTocListBox::GetSelIndices()
//
//	Fill pcdwaSelIndices with the indices of any selected items.  Note that we must
//	must take into account any deleted IMAP messages.
//
int CTocListBox::GetSelIndices(CDWordArray *pcdwaSelIndices, int *piTopIdx, BOOL bHideDeletedIMAPMsgs)
{
	if (!pcdwaSelIndices)
	{
		return 0;
	}

	// Remember the index of the top item so we can scroll back to the correct position when the
	// list is rebuilt.
	*piTopIdx = GetTopIndex();

	CSummary		*pSum = NULL;
	int				 i = 0;
	int				 iDeleted = 0;
	for (i = 0; i < GetCount(); ++i)
	{
		// First check if the current summary selected.  If the selected summary is marked as deleted
		// we need to remember now before we increment the deleted summary count below, otherwise this
		// index will be 1 less than it should be.
		if (GetSel(i) != 0)
		{
			pcdwaSelIndices->Add(i - iDeleted);
		}
		// Tally the number of deleted IMAP messages we encounter so we can adjust the index (if
		// necessary) to account for hidden deleted summaries.
		pSum = (CSummary*)GetItemDataPtr(i);
		if (bHideDeletedIMAPMsgs && pSum->IsIMAPDeleted())
		{
			++iDeleted;
		}
	}

	return pcdwaSelIndices->GetSize();
}

//
//	CTocListBox::SetSelFromIndices()
//
//	Restore the selection based on the indices provided in pcdwaSelIndices.  Note that we must
//	must take into account any deleted IMAP messages.
//
void CTocListBox::SetSelFromIndices(CDWordArray *pcdwaSelIndices, int iTopIdx, BOOL bHideDeletedIMAPMsgs)
{
	if (!pcdwaSelIndices)
	{
		return;
	}

	// Scroll back to the original location.
	SetTopIndex(iTopIdx);

	CSummary		*pSum = NULL;
	int				 i = 0;
	int				 j = 0;
	int				 iDeleted = 0;
	int				 iFirstSel = -1;
	for (i = 0, j = 0; (i < GetCount()) && (j < pcdwaSelIndices->GetSize()); ++i)
	{
		// Tally the number of deleted IMAP messages we encounter so we can adjust the index (if
		// necessary) to account for hidden deleted summaries.
		pSum = (CSummary*)GetItemDataPtr(i);
		if (pSum && !bHideDeletedIMAPMsgs && pSum->IsIMAPDeleted())
		{
			++iDeleted;
		}

		// If the index matches the index of a selected item (accounting for any deleted IMAP messages)
		// then select this item.
		if (i == (int)((pcdwaSelIndices->GetAt(j) + iDeleted)))
		{
			SetSel(i, TRUE);
			if (iFirstSel == -1)
			{
				iFirstSel = i;
			}
			++j;
		}
	}
	if ((iFirstSel == -1) && (i > 0))
	{
		// If we didn't find anything to select, select the last item.
		SetSel(i - 1, TRUE);
	}
	if (iFirstSel > -1)
	{
		// If the selected item is not in view scroll it into view now.
		MaybeSetSelCenter(iFirstSel, false/*bUnSelAll*/);
	}
}

	
BEGIN_MESSAGE_MAP(CTocListBox, CListBox)
	//{{AFX_MSG_MAP(CTocListBox)
	ON_WM_DESTROY()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CHAR()
	ON_WM_KEYUP()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_PAINT()
	ON_WM_GETDLGCODE()
	ON_CONTROL_REFLECT(LBN_KILLFOCUS, OnKillfocus)
	ON_CONTROL_REFLECT(LBN_SETFOCUS, OnSetfocus)
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelChange)
	ON_MESSAGE(LB_SETSEL, OnSetSel)
	ON_MESSAGE(WM_USER_QUERY_PALETTE, OnMyQueryNewPalette)
	ON_WM_PALETTECHANGED()
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTocListBox message handlers

void CTocListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = 30;
}


static int CompareSubject(CSummary* Sum1, CSummary* Sum2)
{
	return (stricmp(RemoveSubjectPrefixMT((LPCSTR)Sum1->m_Subject), RemoveSubjectPrefixMT((LPCSTR)Sum2->m_Subject)));
}

static int CompareServerStatus(CSummary* Sum1, CSummary* Sum2)
{
	CPopHost* CurrentHost = CHostList::GetHostForCurrentPersona();
	if (!CurrentHost)
		return (0);

	int Value1 = 0, Value2 = 0;

	//actually it is not necessary to call IsImapToc on Sum1 and Sum2 , since only one will suffice.
	if(Sum1 && Sum1->m_TheToc && Sum1->m_TheToc->IsImapToc()  && Sum2 && Sum2->m_TheToc && Sum2->m_TheToc->IsImapToc())
	{
			if ( Sum1->IsNotIMAPDownloadedAtAll() )
					Value1=1;
			else if (Sum1->m_nUndownloadedAttachments > 0)
					Value1=2;

			if (Sum1->m_Imflags & IMFLAGS_DELETED)
					Value1+= 4;


		   if ( Sum2->IsNotIMAPDownloadedAtAll() )
					Value2=1;
			else if (Sum2->m_nUndownloadedAttachments > 0)
					Value2=2;

			if (Sum2->m_Imflags & IMFLAGS_DELETED)
					Value2+= 4;
			return Value1 - Value2;
	}

	//CMsgRecord* LMOSRecord1 = GetMsgByHash( Sum1 );
	//CMsgRecord* LMOSRecord2 = GetMsgByHash( Sum2 );

	{
		CLMOSRecord LMOSRecord1(Sum1); 
		CMsgRecord *pMsgRecord1 = LMOSRecord1.GetMsgRecord();
		if ( pMsgRecord1 )
		{
			Value1 = 1;
			if (pMsgRecord1->GetRetrieveFlag() != LMOS_DONOT_RETRIEVE/*1*/)
				Value1 += 1;
			if (pMsgRecord1->GetDeleteFlag() != LMOS_DONOT_DELETE /*1*/)
				Value1 += 2;
		}
	}//destructor removes LMOS lock due to scope
	

	{
		CLMOSRecord LMOSRecord2(Sum2);
		CMsgRecord *pMsgRecord2 = LMOSRecord2.GetMsgRecord();
		if ( pMsgRecord2 )
		{
			Value2 = 1;
			if (pMsgRecord2->GetRetrieveFlag() != LMOS_DONOT_RETRIEVE/*1*/)
				Value2 += 1;
			if (pMsgRecord2->GetDeleteFlag() != LMOS_DONOT_DELETE/*1*/)
				Value2 += 2;
		}
	}//LMOSRecord2 destructor removes LMOS lock due to scope

	return (Value1 - Value2);
}

inline int CompareLong(long Long1, long Long2)
{
	long Result = Long1 - Long2;

	if (Result > 0L)
		return (1);
	if (Result < 0L)
		return (-1);
	return (0);
}

static int CompareSize(CSummary* Sum1, CSummary* Sum2)
{
	const BOOL bSum1Empty = Sum1->HasEmptyBody();
	const BOOL bSum2Empty = Sum2->HasEmptyBody();

	if (bSum1Empty)
	{
		return (bSum2Empty? 0: -1);
	}
	else if (bSum2Empty)
		return (1);

	if (IsImapSum(Sum1))
		return (CompareLong(Sum1->m_MsgSize, Sum2->m_MsgSize));

	return (CompareLong(Sum1->m_Length, Sum2->m_Length));
}

int CompareItem(CSummary* Sum1, CSummary* Sum2)
{
	int Result;

	switch (g_SortMethod & 0x7F)
	{
	case NOT_SORTED:	Result = 0;		break;
	
	// Do status comparison seemingly in the reverse because we want the unread status messages to
	// show up at the bottom even though the unread status has the lowest constant
	case BY_STATUS:		Result = Sum2->m_State - Sum1->m_State;						break;
	case BY_JUNK:		Result = Sum1->m_ucJunkScore - Sum2->m_ucJunkScore;			break;
	case BY_PRIORITY:	Result = Sum1->m_Priority - Sum2->m_Priority;				break;
	case BY_ATTACHMENT:	Result = Sum1->HasAttachment() - Sum2->HasAttachment();		break;
	case BY_LABEL:		Result = Sum1->m_Label - Sum2->m_Label;						break;
	case BY_SENDER:		Result = stricmp(Sum1->m_From, Sum2->m_From);				break;
	case BY_MOOD:		Result = Sum1->m_nMood - Sum2->m_nMood;						break;
	case BY_DATE:		Result = CompareLong(Sum1->m_Seconds, Sum2->m_Seconds);		break;
	case BY_SIZE:		Result = CompareSize(Sum1, Sum2);							break;
	case BY_SERVERSTATUS:Result = CompareServerStatus(Sum1, Sum2);					break;
	case BY_SUBJECT:	Result = CompareSubject(Sum1, Sum2);						break;
        
	default:
		//ASSERT(FALSE);
		return (0);
	}
	
	if (g_SortMethod & 0x80)
		 Result = -Result;
		 
	return (Result);
}

inline bool HighestIndexLessThan(CSummary* Sum1, CSummary* Sum2)
{
	int Result = (Sum1->m_Temp - Sum2->m_Temp);

	// If they are the same, use the original order to determine sort order
	if (Result == 0)
		return ( (Sum1->m_Temp2 & kTemp2SortOrderMask) < (Sum2->m_Temp2 & kTemp2SortOrderMask) );
	
	return (Result < 0);
}

inline bool SubjectLessThan(CSummary* Sum1, CSummary* Sum2)
{
	int Result = CompareSubject(Sum1, Sum2);

	// If they are the same, use the original order to determine sort order
	if (Result == 0)
		return ( (Sum1->m_Temp2 & kTemp2SortOrderMask) < (Sum2->m_Temp2 & kTemp2SortOrderMask) );
	
	return (Result < 0);
}

inline bool SumLessThan(CSummary* Sum1, CSummary* Sum2)
{
	if (!g_SortTocDoc)
	{
		ASSERT(0);
		return true;
	}

	// We shouldn't get here unless we're at least sorting by one criteria
	ASSERT(g_SortTocDoc->GetSortMethod(0) != NOT_SORTED);

	for (int i = 0; i < g_SortTocDoc->NumSortColumns; i++)
	{
		g_SortMethod = g_SortTocDoc->GetSortMethod(i);
		if (g_SortMethod == NOT_SORTED)
			break;
		int Comparison = CompareItem(Sum1, Sum2);
		if (Comparison)
			return (Comparison < 0);
	}

	// They are the same, so use the original order to determine sort order
	return ( (Sum1->m_Temp2 & kTemp2SortOrderMask) < (Sum2->m_Temp2 & kTemp2SortOrderMask) );
}

int CTocListBox::CompareItem(LPCOMPAREITEMSTRUCT /*lpCompareItemStruct*/)
{              
	// Should never get called, but needs to be here or else MFC will assert
	ASSERT(FALSE);
	return (0);
}

void CTocListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CSummary* Sum = (CSummary*)lpDIS->itemData;
	short* FieldSep = ((CTocView*)GetParent())->m_FieldSep;
	BOOL HasFocus = (GetFocus() == this);
	CRect rect(lpDIS->rcItem);
	CRect reg(rect);

	CPoint WOrg = pDC->GetWindowOrg();
	m_HorizWindowOrg = WOrg.x;

//	TRACE3("DrawItem: Sum = 0x%08X, itemAction = 0x%x, itemState = 0x%x\n", Sum, lpDIS->itemAction, lpDIS->itemState);

	// Draw focus rect
	if (lpDIS->itemAction == ODA_FOCUS)
	{
		pDC->DrawFocusRect(&rect);
		return;
	}

	// Set up some color values
	COLORREF BackColor, ForeColor;
	const BOOL bWholeSummaryLabelColor = GetIniShort(IDS_INI_WHOLE_SUMMARY_LABEL_COLOR);
	int Label = bWholeSummaryLabelColor? Sum->m_Label : 0;
	if (lpDIS->itemState & ODS_SELECTED)
	{
		if (HasFocus)
		{
			BackColor = Label? (QCLabelDirector::GetLabelDirector()->GetLabelColor(Label)) : GetSysColor(COLOR_HIGHLIGHT);
			ForeColor = Label? GetSysColor(COLOR_WINDOW) : GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else
		{
			BackColor = RGB(0xC0, 0xC0, 0xC0);
			ForeColor = Label? (QCLabelDirector::GetLabelDirector()->GetLabelColor(Label)) : RGB(0x00, 0x00, 0x00);
		}
	}
	else
	{
		BackColor = GetSysColor(COLOR_WINDOW);
		ForeColor = Label? (QCLabelDirector::GetLabelDirector()->GetLabelColor(Label)) : GetSysColor(COLOR_WINDOWTEXT);
	}
	
	COLORREF OldBackColor = pDC->SetBkColor(BackColor);
	
	// Draw background
	CBrush BackBrush(BackColor);
	pDC->FillRect(&rect, &BackBrush);
	
    // Display column lines
	CPen Pen(PS_SOLID, 1, GetIniShort(IDS_INI_BLACK_TOC_LINES)?
		0 : GetSysColor(COLOR_WINDOWTEXT) ^ 0xC0C0C0);
	CPen* OldPen = pDC->SelectObject(&Pen);

	if (GetIniShort(IDS_INI_SHOW_MAILBOX_LINES))
	{
		for (int i = FW_ATTACHMENT; i <= FW_MOOD; i++)
		{       
			pDC->MoveTo(FieldSep[i], rect.top);
			pDC->LineTo(FieldSep[i], rect.bottom - 1);
		}
	
		// Draw bottom line
		pDC->MoveTo(rect.left, rect.bottom - 1);
		pDC->LineTo(rect.right, rect.bottom - 1);
	}
	
	// Set text colors
	COLORREF OldForeColor = pDC->SetTextColor(ForeColor);
	int OldBkMode = pDC->SetBkMode(TRANSPARENT);
	CBrush* OldBrush = NULL;
	CBrush ForeBrush(ForeColor);
	CFont* OldFont = NULL;
	if (GetIniShort(IDS_INI_COMP_SUMMARY_ITALIC) && Sum->CantEdit() && Sum->m_TheToc->m_Type != MBT_OUT)
		OldFont = pDC->SelectObject(&ScreenFontItalic);
	
	// Status
	int Index = 0;
	switch (Sum->m_State)
	{
		case MS_UNREAD:			Index = QCTocImageList::IMAGE_UNREAD; break;
		case MS_READ:			Index = QCTocImageList::IMAGE_READ; break;
		case MS_REPLIED:		Index = QCTocImageList::IMAGE_REPLIED; break;
		case MS_FORWARDED:		Index = QCTocImageList::IMAGE_FORWARDED; break;
		case MS_REDIRECT:		Index = QCTocImageList::IMAGE_REDIRECTED; break;
		case MS_UNSENDABLE:		Index = QCTocImageList::IMAGE_UNSENDABLE; break;
		case MS_SENDABLE:		Index = QCTocImageList::IMAGE_SENDABLE; break;
		case MS_QUEUED:			Index = QCTocImageList::IMAGE_QUEUED; break;
		case MS_SENT:			Index = QCTocImageList::IMAGE_SENT; break;
		case MS_UNSENT:			Index = QCTocImageList::IMAGE_UNSENT; break;
		case MS_TIME_QUEUED:	Index = QCTocImageList::IMAGE_TIME_QUEUED; break;
		case MS_SPOOLED:			Index = QCTocImageList::IMAGE_SPOOLED; break;
		case MS_RECOVERED:		Index = QCTocImageList::IMAGE_RECOVERED; break;
			// Need to add an image for MS_SPOOLED;
	}

	if (GetIniShort(IDS_INI_MBOX_SHOW_STATUS))
	{
		reg.left = ColumnSpace - 1;
		reg.right = rect.left + FieldSep[FW_STATUS] - ColumnSpace;
		reg.right = min(reg.right, rect.right);
		if (reg.right > reg.left)
		{
			g_theTocImageList.Draw(Index, reg.left - 1, 
				(rect.top + rect.bottom - TocBitmapHeight) / 2, pDC, ForeColor);
		}
	}

	// Junk
	CTocView* pTocView = (CTocView *) GetParent();
	ASSERT_KINDOF(CTocView, pTocView);
	CTocDoc* pTocDoc = (CTocDoc *) pTocView->GetDocument();
	ASSERT_KINDOF(CTocDoc, pTocDoc);
	if (UsingFullFeatureSet() &&
		((GetIniShort(IDS_INI_MBOX_SHOW_JUNK) || pTocDoc->IsJunk())))
	{
		// Draw the junk score if the junk column is being shown or if
		// this is the junk mailbox (where the junk column is always shown).
		if ((Sum->m_ucJunkScore >= 0) && (Sum->m_ucJunkScore <= 100))
		{
			char		 strJunkScore[8];
			sprintf(strJunkScore, "%d", Sum->m_ucJunkScore);
			reg.left = rect.left + FieldSep[FW_JUNK - 1] + ColumnSpace;
			reg.right = rect.left + FieldSep[FW_JUNK] - ColumnSpace;
			reg.right = min(reg.right, rect.right);
			UINT OldTextAlign = pDC->SetTextAlign(TA_RIGHT);
			pDC->ExtTextOut(reg.right, reg.top, ETO_CLIPPED, &reg,
							strJunkScore, strlen(strJunkScore), NULL);
			pDC->SetTextAlign(OldTextAlign);
		}
	}

	// Priority
	if (GetIniShort(IDS_INI_MBOX_SHOW_PRIORITY))
	{
		if (Sum->m_Priority != MSP_NORMAL &&
			FieldSep[FW_PRIORITY] - FieldSep[FW_PRIORITY - 1] > ColumnSpace)
		{
			g_theTocImageList.Draw(QCTocImageList::IMAGE_LOWEST + (5 - Sum->m_Priority),
				rect.left + FieldSep[FW_PRIORITY - 1] + ColumnSpace, 
				(rect.top + rect.bottom - TocBitmapHeight) / 2, pDC, ForeColor);
		}
	}

	// Attachment
	if (GetIniShort(IDS_INI_MBOX_SHOW_ATTACHMENT))
	{
		if (Sum->HasAttachment() &&
			FieldSep[FW_ATTACHMENT] - FieldSep[FW_ATTACHMENT - 1] > ColumnSpace)
		{
			g_theTocImageList.Draw(QCTocImageList::IMAGE_ATTACH,
				rect.left + FieldSep[FW_ATTACHMENT - 1] + ColumnSpace, 
				(rect.top + rect.bottom - TocBitmapHeight) / 2, pDC, ForeColor);
		}
	}
	
	// Label
	if (GetIniShort(IDS_INI_MBOX_SHOW_LABEL))
	{
		if (Sum->m_Label)
		{
			reg.left = rect.left + FieldSep[FW_LABEL - 1] + ColumnSpace;
			reg.right = rect.left + FieldSep[FW_LABEL] - ColumnSpace;
			reg.right = min(reg.right, rect.right);

			UINT nLabelIdx = Sum->m_Label;
			COLORREF OldNonLabelColor = 0;
			if (!bWholeSummaryLabelColor)
			{
				 OldNonLabelColor =
					pDC->SetTextColor(QCLabelDirector::GetLabelDirector()->GetLabelColor(Sum->m_Label));
			}
			LPCSTR Text = (QCLabelDirector::GetLabelDirector()->GetLabelText(nLabelIdx));
			pDC->ExtTextOut(reg.left, reg.top, ETO_CLIPPED, &reg, Text, ::SafeStrlenMT(Text), NULL);
			if (!bWholeSummaryLabelColor)
				 pDC->SetTextColor(OldNonLabelColor);
		}
	}

	// Sender
	if (GetIniShort(IDS_INI_MBOX_SHOW_SENDER))
	{
		reg.left = rect.left + FieldSep[FW_SENDER - 1] + ColumnSpace;
		reg.right = rect.left + FieldSep[FW_SENDER] - ColumnSpace;
		reg.right = min(reg.right, rect.right);
		pDC->ExtTextOut(reg.left, reg.top, ETO_CLIPPED, &reg, Sum->m_From, strlen(Sum->m_From), NULL);
	}

	// Date
	if (GetIniShort(IDS_INI_MBOX_SHOW_DATE))
	{
		reg.left = rect.left + FieldSep[FW_DATE - 1] + ColumnSpace;
		reg.right = rect.left + FieldSep[FW_DATE] - ColumnSpace;
		reg.right = min(reg.right, rect.right);
		pDC->ExtTextOut(reg.left, reg.top, ETO_CLIPPED, &reg, Sum->m_Date, strlen(Sum->m_Date), NULL);
	}

	// Length
	if (GetIniShort(IDS_INI_MBOX_SHOW_SIZE))
	{
		reg.left = rect.left + FieldSep[FW_SIZE - 1] + ColumnSpace;
		reg.right = rect.left + FieldSep[FW_SIZE] - ColumnSpace;
		reg.right = min(reg.right, rect.right);
		
		char Length[16];
		if (Sum->HasEmptyBody())
			strcpy(Length, "Ø");
		else
		{
			int nLen = IsImapSum(Sum)? Sum->m_MsgSize : ((Sum->m_Length/1024)+1);
			wsprintf(Length, "%d", nLen);
		}
		UINT OldTextAlign = pDC->SetTextAlign(TA_RIGHT);
		pDC->ExtTextOut(reg.right, reg.top, ETO_CLIPPED, &reg, Length, strlen(Length), NULL);
		pDC->SetTextAlign(OldTextAlign);
	}

	// Server status
	if (GetIniShort(IDS_INI_MBOX_SHOW_SERVER_STATUS))
	{
		if (FieldSep[FW_SERVER] - FieldSep[FW_SERVER - 1] > ColumnSpace)
		{
			int BitmapIndex = -1;
#ifdef IMAP4 // IMAP4
			if (Sum->m_TheToc->IsImapToc())
			{
				BitmapIndex = 0;
				if ( Sum->IsNotIMAPDownloadedAtAll() )
					BitmapIndex = 4;
				else if (Sum->m_nUndownloadedAttachments > 0)
					BitmapIndex = 12;

				if (Sum->m_Imflags & IMFLAGS_DELETED)
					BitmapIndex = 2;
			}
			else
#endif // END IMAP4
			{
				CLMOSRecord LMOSRecord(Sum);
				CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();

				// If the command to delete the message has been sent to the server,
				// then treat it as though it's not on the server at all
				if ( pMsgRecord && pMsgRecord->GetDeleteFlag() != LMOS_DELETE_SENT)
				{
					//BOOL RetFlag = (pMsgRecord->GetRetrieveFlag() != 1);
					//BOOL DelFlag = (pMsgRecord->GetDeleteFlag() != 1);
					BitmapIndex = 0;
					
					if (pMsgRecord->GetRetrieveFlag() == LMOS_RETRIEVE_MESSAGE)	////if (RetFlag)
						BitmapIndex = 1;
					
					if (pMsgRecord->GetDeleteFlag() == LMOS_DELETE_MESSAGE)		//if (DelFlag)
						BitmapIndex += 2;

					if ((0 == BitmapIndex) && (pMsgRecord->GetSkippedFlag() == LMOS_SKIP_MESSAGE /*0*/))
					{
						//
						// Message not marked for fetch or delete, but we only 
						// have a stub, so indicate the stub to the user.
						// 
						BitmapIndex = 4;
					}
				}
			}
			if (BitmapIndex >= 0)
			{
				g_theTocImageList.Draw(QCTocImageList::IMAGE_SERVER_LEAVE + BitmapIndex,
					rect.left + FieldSep[FW_SERVER - 1] + ColumnSpace, 
					(rect.top + rect.bottom - TocBitmapHeight) / 2, pDC, ForeColor);
			}
		}
	}

	// MOOD MAIL
	if (GetIniShort(IDS_INI_MBOX_SHOW_MOOD) && UsingFullFeatureSet() && 
		GetIniShort(IDS_INI_MOOD_MAIL_CHECK) )
	{
		if ( (Sum->m_nMood>1)&&(Sum->m_nMood<5) &&
			  FieldSep[FW_MOOD] - FieldSep[FW_MOOD - 1] > ColumnSpace)
		{
			g_theMoodImageList.Draw(Sum->m_nMood,
				rect.left + FieldSep[FW_MOOD - 1] + ColumnSpace, 
				(rect.top + rect.bottom - TocBitmapHeight) / 2, pDC, ForeColor);
		}
	}

	// Subject
	if (GetIniShort(IDS_INI_MBOX_SHOW_SUBJECT))
	{
		reg.left = rect.left + FieldSep[FW_MOOD] + ColumnSpace;
		reg.right = rect.right;
		pDC->ExtTextOut(reg.left, reg.top, ETO_CLIPPED, &reg, Sum->m_Subject, strlen(Sum->m_Subject), NULL);
	}

	// We selected some objects, so we need to deselect them because GDI leaks are not fun
	if (OldFont)
		pDC->SelectObject(OldFont);
	if (OldBrush)
		pDC->SelectObject(OldBrush);
	pDC->SetBkMode(OldBkMode);
	pDC->SetTextColor(OldForeColor);
	pDC->SetBkColor(OldBackColor);
	pDC->SelectObject(OldPen);
}


void CTocListBox::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	if (m_bAllowSelChangeNotifications)
	{
		// Deleting an item is assumed to not have been the result of direct user action.
		// Those cases when it is the result of direct user action already call
		// MailboxChangedByHuman, which causes the OnSelChange code to fire.
		HandleSelectionMayHaveChanged(false);
	}
}


////////////////////////////////////////////////////////////////////////
// GetItemText [public]
//
// Returns an ASCII string for the list box item at index 'nIndex'.  This
// string consists of generic, tab-delimited fields.  This string is
// designed to be copied to the clipboard as text.
////////////////////////////////////////////////////////////////////////
BOOL CTocListBox::GetItemText(int nIndex, CString& rString) const
{
	CSummary* p_sum = GetItemDataPtr(nIndex);
	if (NULL == p_sum)
		return FALSE;

	//
	// For performance reasons, we pre-allocate the target buffer
	// to be a little bigger than the size of an average data line
	// for a summary.
	//
	rString.Empty();
	rString.GetBuffer(128);

	//
	// Status field.
	//
	if (GetIniShort(IDS_INI_MBOX_SHOW_STATUS))
	{
		rString += '[';
		if (p_sum->m_State == MS_UNREAD || p_sum->m_State == MS_SENDABLE)
			rString += '*';		// Use '*' as bullet character.
		else
			rString += StateFlag[p_sum->m_State];
		rString += ']';
	}

	//
	// Priority field.
	//
	if (GetIniShort(IDS_INI_MBOX_SHOW_PRIORITY))
	{
		if (! rString.IsEmpty())
			rString += "\t";
		rString += p_sum->GetPriority();
	}

	//
	// Attachments field
	//
	if (GetIniShort(IDS_INI_MBOX_SHOW_ATTACHMENT))
	{
		if (! rString.IsEmpty())
			rString += "\t";

		//
		// Note -- Mac Eudora leaves this field blank, even if there's 
		// an attachment.  I believe this is a bug in the Mac implementation.
		//
		if (p_sum->HasAttachment())
			rString += "[+]";
		else
			rString += "[ ]";
	}

	//
	// Label field [Commercial only]
	//
	if (GetIniShort(IDS_INI_MBOX_SHOW_LABEL))
	{
		if (! rString.IsEmpty())
			rString += "\t";
		if (p_sum->m_Label)
		{
			LPCSTR p_label = (QCLabelDirector::GetLabelDirector()->GetLabelText(p_sum->m_Label));
			ASSERT(p_label != NULL);
			rString += p_label;
		}
	}

	//
	// From field.
	//
	if (GetIniShort(IDS_INI_MBOX_SHOW_SENDER))
	{
		if (! rString.IsEmpty())
			rString += "\t";
		rString += p_sum->m_From;
	}

	//
	// Date field.
	//
	if (GetIniShort(IDS_INI_MBOX_SHOW_DATE))
	{
		if (! rString.IsEmpty())
			rString += "\t";
		rString += p_sum->m_Date;
	}

	//
	// Size field.
	//
	if (GetIniShort(IDS_INI_MBOX_SHOW_SIZE))
	{
		if (! rString.IsEmpty())
			rString += "\t";

		char length[16];
		int nLen = IsImapSum(p_sum)?p_sum->m_MsgSize: ((p_sum->m_Length/1024)+1);
		//wsprintf(length, "%2d", int(p_sum->m_Length / 1024) + 1);
		wsprintf(length, "%2d", nLen + 1);
		ASSERT(strlen(length) < sizeof(length));		// better late than never
		rString += length;
	}
	
	//
	// Server Status field [Commercial only]
	//
	if (GetIniShort(IDS_INI_MBOX_SHOW_SERVER_STATUS))
	{
		//
		// FORNOW, we don't include the server status field in the 
		// clipboard data.
		//
	}
	//
	//Mood field
	if (GetIniShort(IDS_INI_MBOX_SHOW_MOOD))
	{
		//
		// FORNOW, we don't include the Mood field in the 
		// clipboard data.
		//
	}

	//
	// Junk field
	//
	if (GetIniShort(IDS_INI_MBOX_SHOW_JUNK))
	{
		//
		// FORNOW, we don't include the Junk field in the 
		// clipboard data.
		//
	}

	//
	// Subject field.
	//
	if (GetIniShort(IDS_INI_MBOX_SHOW_SUBJECT))
	{
		if (! rString.IsEmpty())
			rString += "\t";
		rString += p_sum->m_Subject;
	}

	return TRUE;
}

void CTocListBox::OnPaint()
{
	CListBox::OnPaint();
	
	if (GetIniShort(IDS_INI_SHOW_MAILBOX_LINES))
	{
		CRect ClientRect;
		CRect ItemRect(0, 0, 0, 0);
		int Count = GetCount();
		
		GetClientRect(&ClientRect);
		if (!Count || GetItemRect(Count - 1, &ItemRect) == LB_ERR ||
			ItemRect.bottom <= ClientRect.bottom)
		{
			CDC* pDC = GetDC();
			if (pDC)
			{
				// Display column lines that don't get drawn by list items
				short* FieldSep = ((CTocView*)GetParent())->m_FieldSep;
				CPen Pen(PS_SOLID, 1, (GetIniShort(IDS_INI_BLACK_TOC_LINES)?
					0 : GetSysColor(COLOR_WINDOWTEXT) ^ 0xC0C0C0));
				pDC->SelectObject(&Pen);
				for (int i = FW_ATTACHMENT; i <= FW_MOOD; i++)
				{
					pDC->MoveTo(FieldSep[i] - m_HorizWindowOrg, ItemRect.bottom);
					pDC->LineTo(FieldSep[i] - m_HorizWindowOrg, ClientRect.bottom);
				}
				pDC->SelectStockObject(BLACK_PEN);
				ReleaseDC(pDC);
			}
		}
	}
} 

LRESULT CTocListBox::OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam)
{            
	return m_Palette.DoQueryNewPalette((CWnd*)this);
}

void CTocListBox::OnPaletteChanged(CWnd* pFocusWnd)
{
	m_Palette.DoPaletteChanged(this, pFocusWnd);
}

void CTocListBox::OnSysColorChange()
{
	m_Palette.DoSysColorChange(this);
}


// FindSelectedField
// Returns the field that the point is on
//
FieldWidthType CTocListBox::FindSelectedField(const CPoint& point)
{
   	short* FieldSep = ((CTocView*)GetParent())->m_FieldSep;
	int Index = GetTopIndex();
	int Count = GetCount();
	CRect rect, ClientRect;

	GetClientRect(&ClientRect);
	for (; Index < Count; Index++)
	{
		if (GetItemRect(Index, &rect) == LB_ERR)
			break;
			
		if (rect.PtInRect(point))
		{
			for (int i = FW_STATUS; i < FW_UNUSED; i++)
			{
				if (point.x < FieldSep[i])
					return ((FieldWidthType)i);
			}
			break;
		}
		
		if (rect.bottom >= ClientRect.bottom)
			break;
	}	
	return (FW_UNUSED);
}

BOOL CTocListBox::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	return (CListBox::OnSetCursor(pWnd, nHitTest, message));
}

int CTocListBox::SameItemSelectCompare(FieldWidthType Field, CSummary* Sum1, CSummary* Sum2)
{
	if (Field == FW_SIZE)
		return ( IsImapSum(Sum1)?Sum1->m_MsgSize:Sum1->m_Length/1024 - IsImapSum(Sum2)?Sum2->m_MsgSize:Sum2->m_Length/1024);
	else if (Field == FW_DATE)
		return (strcmp(Sum1->m_Date, Sum2->m_Date));

	return (::CompareItem(Sum1, Sum2));
}

void CTocListBox::SameItemSelect(const CPoint& ClickPoint, int Index)
{
	CWaitCursor wc;

	// Figure out which column was selected, and then select all summaries
	// that match the value of the column of the item selected.
	const FieldWidthType Field = FindSelectedField(ClickPoint);
	CSummary* AnchorSum = GetItemDataPtr(Index);

	if (AnchorSum)
	{
		BOOL MovedOne = FALSE;
		BOOL DoMove = GetIniShort(IDS_INI_ALT_CLICK_MOVE_SUMMARY);
		if (GetKeyState(VK_SHIFT) < 0)
			DoMove = !DoMove;

		switch (Field)
		{
		case FW_STATUS:		g_SortMethod = BY_STATUS;		break;
		case FW_JUNK:		g_SortMethod = BY_JUNK;			break;
		case FW_PRIORITY:	g_SortMethod = BY_PRIORITY;		break;
		case FW_ATTACHMENT:	g_SortMethod = BY_ATTACHMENT;	break;
		case FW_LABEL:		g_SortMethod = BY_LABEL;		break;
		case FW_SENDER:		g_SortMethod = BY_SENDER;		break;
		case FW_DATE:		g_SortMethod = BY_DATE;			break;
		case FW_SIZE:		g_SortMethod = BY_SIZE;			break;
		case FW_SERVER:		g_SortMethod = BY_SERVERSTATUS;	break;
		case FW_MOOD:		g_SortMethod = BY_MOOD;			break;
		case FW_SUBJECT:	g_SortMethod = BY_SUBJECT;		break;
		
		default: ASSERT(FALSE); break;
		}	

		CTocView* pTocView = (CTocView *) GetParent();
		ASSERT_KINDOF(CTocView, pTocView);
		CTocDoc* pTocDoc = (CTocDoc *) pTocView->GetDocument();
		ASSERT_KINDOF(CTocDoc, pTocDoc);

		CSumList &		listSums = pTocDoc->GetSumList();

		POSITION AnchorPos = listSums.FindIndex(Index);
		ASSERT(AnchorPos);
		int Count = GetCount();
		ASSERT(Count == listSums.GetCount(pTocDoc->HideDeletedIMAPMsgs()));

		// Start off with nothing selected
		SetSel(-1, FALSE);

		BOOL		bHideDeletedIMAPMsgs = pTocDoc->HideDeletedIMAPMsgs();

		// Look above (before) the clicked-on summary for matches,
		// and gravitate the matches toward it
		int CompareIndex = Index;
		int CurrentIndex = CompareIndex - 1;
		POSITION ComparePos = AnchorPos;
		POSITION CurrentPos = ComparePos;
		listSums.GetPrev(CurrentPos);
		POSITION PrevPos = CurrentPos;
		int iDeleted = 0;
		for (; CurrentPos; CurrentPos = PrevPos, CurrentIndex--)
		{
			CSummary* Sum = listSums.GetPrev(PrevPos);
			if (Sum && bHideDeletedIMAPMsgs && Sum->IsIMAPDeleted())
			{
				++iDeleted;
			}
			if (Sum &&
				(SameItemSelectCompare(Field, AnchorSum, Sum) == 0) &&
				!(bHideDeletedIMAPMsgs && Sum->IsIMAPDeleted()))
			{
				if (!DoMove)
					SelItemRange(TRUE, CurrentIndex - 1, CurrentIndex);
				else
				{
					if (CurrentIndex != CompareIndex - 1)
					{
						pTocDoc->MoveSumBefore(CurrentPos, ComparePos);
						DeleteString(CurrentIndex);
						InsertString(CompareIndex - 1, (LPCSTR)Sum);
						MovedOne = TRUE;
					}
				}

				CompareIndex--;
				listSums.GetPrev(ComparePos);
			}
			else if (!DoMove)
			{
				SelItemRange(FALSE, CurrentIndex - 1, CurrentIndex);
			}
		}
		if (DoMove)
			SelItemRange(TRUE, CompareIndex, Index);

		// Look below (after) the clicked-on summary for matches,
		// and gravitate the matches toward it
		CompareIndex = Index;
		CurrentIndex = CompareIndex + 1;
		ComparePos = AnchorPos;
		CurrentPos = ComparePos;
		iDeleted = 0;
		listSums.GetNext(CurrentPos);
		POSITION NextPos = CurrentPos;
		for (; CurrentPos; CurrentPos = NextPos, CurrentIndex++)
		{
			CSummary* Sum = listSums.GetNext(NextPos);
			if (Sum && bHideDeletedIMAPMsgs && Sum->IsIMAPDeleted())
			{
				++iDeleted;
			}
			if (Sum &&
				(SameItemSelectCompare(Field, AnchorSum, Sum) == 0) &&
				!(bHideDeletedIMAPMsgs && Sum->IsIMAPDeleted()))
			{
				if (!DoMove)
					SelItemRange(TRUE, CurrentIndex, CurrentIndex + 1);
				else
				{
					if (CurrentIndex != CompareIndex + 1)
					{
						pTocDoc->MoveSumAfter(CurrentPos, ComparePos);
						DeleteString(CurrentIndex);
						InsertString(CompareIndex + 1, (LPCSTR)Sum);
						MovedOne = TRUE;
					}
				}

				CompareIndex++;
				listSums.GetNext(ComparePos);
			}
			else if (!DoMove)
			{
				SelItemRange(FALSE, CurrentIndex, CurrentIndex + 1);
			}
		}
		if (DoMove)
			SelItemRange(TRUE, Index, CompareIndex);

		// Same item select is direct user action
		// This will make sure that the clicked-on summary will have the focus
		SetSel(Index, TRUE, true);

		if (MovedOne)
		{
			// Only set m_bNeedsSorting and not m_bNeedsSortingNOW because we
			// only want to resort the mailbox after some major event, like
			// a new message being added to the mailbox, or the mailbox being
			// closed and reopened.
			pTocDoc->SetNeedsSorting();
			pTocDoc->SetModifiedFlag();
		}
	}
}


void CTocListBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	BOOL WeBeDragDrop = TRUE;
	BOOL AltDown = (GetKeyState(VK_MENU) < 0);

	//
	// Check whether or not drag and drop is enabled every time
	// the user clicks the mouse button.
	//
	m_AllowDragAndDrop = GetIniShort(IDS_INI_ALLOW_DRAGDROP);
	if (! m_AllowDragAndDrop)
	{
		//TRACE0("CTocListBox::OnLButtonDown() normal button down (bypass drag/drop)\n");
		m_MouseState = MOUSE_IDLE;
		m_SavedMouseDownPoint = CPoint(-1, -1);
		CListBox::OnLButtonDown(nFlags, point);

		// No need to continue if we're not doing the special selection feature
		if (AltDown == FALSE)
			return;

		WeBeDragDrop = FALSE;
	}

	//
	// Okay, since this list box is now an OLE Drop Source, we need to 
	// break the default CListBox selection behavior of changing the
	// selection on the mouse down.  Basically, when clicking on
	// on selected items, the idea is to postpone selection changes
	// until the user *releases* the mouse button.
	//
	// This selection scheme is modeled after the behavior of the File 
	// Pane of the Visual SourceSafe Explorer.  :-)
	//
	// Okay, so let's start off with a hack alert!  If the button up routine
	// detects that we've postponed the mouse down processing, then
	// it sets the mouse status to "MOUSE_DOWN_FAKE" to indicate that
	// it has issued a fake button down message to make up for the one
	// that we didn't process the first time around.
	//
	if (WeBeDragDrop && MOUSE_DOWN_FAKE == m_MouseState)
	{
		//TRACE0("CTocListBox::OnLButtonDown() fake button down\n");
		CListBox::OnLButtonDown(nFlags, point);
		m_MouseState = MOUSE_IDLE;
		m_SavedMouseDownPoint = CPoint(-1, -1);
		return;
	}
	
	BOOL is_outside = FALSE;
	UINT idx = UINT(-1);
	if (IsVersion4())
	{
		idx = ItemFromPoint(point, is_outside);
	}
	else
	{
		//
		// The ItemFromPoint() implementation doesn't work
		// under NT 3.51 (always returns zero), so fall back
		// on the old-fashioned Win16 methodology.
		//
		int item_height = GetItemHeight(0);
		ASSERT(item_height > 0);
		idx = GetTopIndex() + point.y / item_height;
		if (idx >= UINT(GetCount()))
			is_outside = TRUE;		// hit below the last item
	}

	//TRACE2("CTocListBox::OnLButtonDown(): idx=%d, is_outside=%d\n", idx, is_outside);
	if (! is_outside)
	{
		if (AltDown)
		{
			SameItemSelect(point, idx);

			// Don't do any further processing because it would affect selection
			return;
		}

		if (WeBeDragDrop)
		{
			//
			// If item is already selected, then keep it selected 
			// (i.e., ignore the button click) since we may be starting a 
			// drag operation on the current selection.
			//
			if (GetSel(idx) == 0)
			{
				//TRACE0("CTocListBox::OnLButtonDown() not on selection, handling normally\n");
				CListBox::OnLButtonDown(nFlags, point);
				m_MouseState = MOUSE_DOWN_OFF_SELECTION;
			}
			else
			{
				//TRACE0("CTocListBox::OnLButtonDown() on selection, postponing handling\n");
				m_MouseState = MOUSE_DOWN_ON_SELECTION;
			}
			m_SavedMouseDownPoint = point;
		}
	}
	else
	{
		//TRACE0("CTocListBox::OnLButtonDown() bogus selection\n");
		m_MouseState = MOUSE_IDLE;
		m_SavedMouseDownPoint = CPoint(-1, -1);
	}
}


////////////////////////////////////////////////////////////////////////
// OnMouseMove [protected]
//
////////////////////////////////////////////////////////////////////////
void CTocListBox::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_AllowDragAndDrop && (nFlags & MK_LBUTTON) && (m_MouseState != MOUSE_IDLE))
	{
		switch (m_MouseState)
		{
		case MOUSE_DOWN_OFF_SELECTION:
			// If drag-to-select messages switch is on, then it's business as usual
			if (GetIniShort(IDS_INI_DRAG_SELECT_MESSAGES))
			{
				ASSERT(m_SavedMouseDownPoint != CPoint(-1, -1));
				CListBox::OnMouseMove(nFlags, point);
				return;
			}
			// else fall through to drag-and-drop behavior
		case MOUSE_DOWN_ON_SELECTION:
			//
			// User is starting a drag operation on an previously selected item.
			// In this case, we bypass the normal selection action and just
			// start the drag operation.
			//
			ASSERT(m_SavedMouseDownPoint != CPoint(-1, -1));

			//
			// For some reason, NT is generating a WM_MOUSEMOVE right after
			// the WM_LBUTTONDOWN, even though the mouse has not actually
			// changed positions...  Therefore, avoid the internal
			// state change to MOUSE_IS_MOVING until we actually detect
			// a real mouse position change.
			//
			if (m_SavedMouseDownPoint != point)
				m_MouseState = MOUSE_IS_MOVING;			// indicate we're in the drag part of a click and drag action
			CListBox::OnMouseMove(nFlags, point);
			return;
		case MOUSE_IS_MOVING:
			//
			// User has started a click and drag action.  Check to see if
			// the user has dragged far enough to initiate a drag and
			// drop action.
			//
			ASSERT(m_SavedMouseDownPoint != CPoint(-1, -1));
			if (abs(point.x - m_SavedMouseDownPoint.x) > 6 ||
				abs(point.y - m_SavedMouseDownPoint.y) > 6)
			{
				break;		// fall through to drag and drop processing below...
			}
			return;
		case MOUSE_IS_DRAGGING:
			// silently return since OleDataSource is in control
			return;
		default:
			ASSERT(0);
			return;
		}

		CTocView* p_tocview = (CTocView *) GetParent();
		ASSERT(p_tocview != NULL);
		ASSERT_KINDOF(CTocView, p_tocview);

		//
		// If we get this far, initiate a drag and drop operation.
		// Create global memory for drag 'n drop data.  Since Eudora is
		// both the drop source and the drop target, we put Eudora's 
		// HINSTANCE and source TocDoc in the buffer to make sure
		// that we don't drag and drop between two different instances
		// of Eudora.
		//
		QCFTRANSFER qcfTransfer;
		qcfTransfer.m_pSourceTocDoc = (CTocDoc *) p_tocview->GetDocument();
		ASSERT(qcfTransfer.m_pSourceTocDoc);
		ASSERT_KINDOF(CTocDoc, qcfTransfer.m_pSourceTocDoc);
		ASSERT(NULL == qcfTransfer.m_pSingleSummary);

		// Cache the HINSTANCE data
		g_theClipboardMgr.CacheGlobalData(m_OleDataSource, qcfTransfer);

		//
		// Save the client rect of the entire *View* window (in screen
		// coordinates) so that the drag and drop code doesn't indicate 
		// that this source window is a valid drop target.
		//
		p_tocview->GetWindowRect(&m_DataSourceRect);
		m_MouseState = MOUSE_IS_DRAGGING;
                                             
		//
		// Create another global memory object for the selected text
		// itself.  This allows you to drag the text selection to
		// a text-aware application such as word processors and spreadsheets.
		//
		CString sel_text;
		p_tocview->GetSelText(sel_text);

		HGLOBAL h_gdata = GlobalAlloc(GPTR, sel_text.GetLength() + 1);   
		ASSERT(h_gdata != NULL);
		char* p_str = (char *) GlobalLock(h_gdata);
		ASSERT(p_str != NULL);
		strcpy(p_str, sel_text);

		// Cache the selected text
		m_OleDataSource.CacheGlobalData(CF_TEXT, h_gdata);

		//
		// Do the drag and drop!
		//
		QCOleDropSource oleDropSource;
		DROPEFFECT dropEffect = m_OleDataSource.DoDragDrop(DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK, NULL, &oleDropSource);
 
		if ((dropEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)     
		{
			//
			// The OLE docs say that we should process deletion of the
			// selected moved items here.  However, since Eudora
			// itself is the only thing that handles transfers from
			// TOCs, we'll just let the Transfer code called from the
			// Drop *Target* side take care of deleting the selection.
			//
		}
      
		// Clear the Data Source's cache
		m_OleDataSource.Empty();
	}

	m_MouseState = MOUSE_IDLE;
	m_SavedMouseDownPoint = CPoint(-1, -1);
	CListBox::OnMouseMove(nFlags, point);
}


////////////////////////////////////////////////////////////////////////
// OnLButtonUp [protected]
//
// If we release the mouse button we made need to change the list
// box selection since we break the default selection behavior of the
// CListBox in OnLButtonDown().
////////////////////////////////////////////////////////////////////////
void CTocListBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_AllowDragAndDrop &&
		(m_SavedMouseDownPoint != CPoint(-1, -1)) && 
		(m_SavedMouseDownPoint == point) &&
		(m_MouseState == MOUSE_DOWN_ON_SELECTION))
	{
		//
		// Okay, this is gonna sound weird, but we actually want to
		// call the default button *DOWN* procedure upon receiving a
		// button *UP* message.  This "unbreaks" the broken (postponed)
		// selection behavior we setup in the button down handler.  
		// Really.  Trust me.
		//
		//TRACE0("CTocListBox::OnLButtonUp() handling postponed button down\n");
		m_MouseState = MOUSE_DOWN_FAKE;
		m_SavedMouseDownPoint = CPoint(-1, -1);
		SendMessage(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
		ASSERT(MOUSE_IDLE == m_MouseState);
		ASSERT(CPoint(-1, -1) == m_SavedMouseDownPoint);
		SendMessage(WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));
	}
	else
	{
		//TRACE0("CTocListBox::OnLButtonUp() normal button up\n");
		CListBox::OnLButtonUp(nFlags, point);
		m_MouseState = MOUSE_IDLE;
		m_SavedMouseDownPoint = CPoint(-1, -1);
	}
}


////////////////////////////////////////////////////////////////////////
// OnRButtonDown [protected]
//
// With context menus enabled, right click on an item should map
// to an unmodified (ignore Shift, Ctrl, Alt) left click on the item.
// The context menu will appear on the right button release.
////////////////////////////////////////////////////////////////////////
void CTocListBox::OnRButtonDown(UINT nFlags, CPoint point)
{
	BOOL is_outside = FALSE;
	UINT idx = UINT(-1);
	if (IsVersion4())
	{
		idx = ItemFromPoint(point, is_outside);
	}
	else
	{
		//
		// The ItemFromPoint() implementation doesn't work
		// under NT 3.51 (always returns zero), so fall back
		// on the old-fashioned Win16 methodology.
		//
		int item_height = GetItemHeight(0);
		ASSERT(item_height > 0);
		idx = GetTopIndex() + point.y / item_height;
		if (idx >= UINT(GetCount()))
			is_outside = TRUE;		// hit below the last item
	}

	if ((! is_outside) && (GetSel(idx) == 0))
	{
		//
		// Didn't hit an existing selection, so do an implicit
		// single-select action.
		//
		if (nFlags & MK_SHIFT)
		{
			SetSel(-1, FALSE);		// deselect all (needed for modified right-click)
			SetAnchorIndex(idx);	// make sure we don't end up with an extended selection
			SetSel(idx, TRUE);
		}
		else if (nFlags & MK_CONTROL)
			SetSel(-1, FALSE);		// deselect all (needed for modified right-click)

		//
		// Now fake a left mouse click.
		//
		SendMessage(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
		SendMessage(WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));
	}
	else
	{
		//
		// Either hit an existing selection or clicked somewhere
		// outside of valid item list.  Just do the default processing
		// and wait for a button up to display the context menu.
		// 
		CListBox::OnRButtonDown(nFlags, point);
	}
}


////////////////////////////////////////////////////////////////////////
// IsDraggingOver [public]
//
// Returns TRUE if we're in drag and drop mode and the mouse is dragged
// over the source window rectangle.  This is used by the CTocView's
// drag and drop methods to determine whether the data source window
// is the same as the drop target window.
////////////////////////////////////////////////////////////////////////
BOOL CTocListBox::IsDraggingOver(const CPoint& screenPt) const
{
	if ((MOUSE_IS_DRAGGING == m_MouseState) &&
		m_DataSourceRect.PtInRect(screenPt))
	{
		return TRUE;
	}
	return FALSE;
}


void CTocListBox::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CListBox::OnLButtonDblClk(nFlags, point);
	
	((CTocView*)GetParent())->OpenMessages();
}

void CTocListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Turn message-switching hotkeys into ID_PREVIOUS_MESSAGE/ID_NEXT_MESSAGE
	if ((nChar == VK_LEFT || nChar == VK_UP || nChar == VK_RIGHT || nChar == VK_DOWN))
	{
		BOOL CtrlDown = (GetKeyState(VK_CONTROL) < 0);
		BOOL ControlArrows = GetIniShort(IDS_INI_CONTROL_ARROWS) != 0;
		
		if (ControlArrows && CtrlDown)
		{
			GetParent()->SendMessage(WM_COMMAND, (WPARAM)((nChar == VK_LEFT || nChar == VK_UP)?
				ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE));
			return;
		}
	}
	
	if ((nChar == VK_SPACE) && ShiftDown())
	{
		// Shift+Space means "toggle read status"
		//
		// Send the message to the CTocFrame (as opposed to the CTocView, where the
		// message handling where the message handling for ID_MESSAGE_STATUS_TOGGLE
		// used to reside).
		CFrameWnd *		pFrame = GetParentFrame();
		ASSERT(pFrame);
		if (pFrame)
			pFrame->SendMessage(WM_COMMAND, ID_MESSAGE_STATUS_TOGGLE);

		// Reset the TTS phrase because the user is not currently typing a phrase anymore
		ResetTTSPhrase();

		return;		// bypass normal key down handling which toggles selection
	}
	
	CListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTocListBox::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Turn message-switching hotkeys into ID_PREVIOUS_MESSAGE/ID_NEXT_MESSAGE
	if (nChar == VK_LEFT || nChar == VK_UP || nChar == VK_RIGHT || nChar == VK_DOWN)
	{
		BOOL CtrlDown = (GetKeyState(VK_CONTROL) < 0);
		BOOL AltArrows = GetIniShort(IDS_INI_ALT_ARROWS) != 0;
		
		if (AltArrows && !CtrlDown)
		{
			GetParent()->SendMessage(WM_COMMAND, (WPARAM)((nChar == VK_LEFT || nChar == VK_UP)?
				ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE));
			return;
		}
	}

	CListBox::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CTocListBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	bool	bNotDoingTTS = (m_nFindPhraseLen == 0) || ((GetTickCount() - m_nLastKeyTime) > kMaxTTSDelay);
	
	if ( ((nChar == VK_SPACE) && !ShiftDown() && bNotDoingTTS) || (nChar == VK_RETURN) )
	{
		((CTocView*)GetParent())->OpenMessages();
	}
	else if (nChar == VK_BACK && GetIniShort(IDS_INI_BACKSPACE_DELETE))
	{
		CTocFrame *		pTocFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );
		if (pTocFrame)
			pTocFrame->OnCmdDelete();
	}
	else if ( isalnum(nChar) || ispunct(nChar) || ((nChar == VK_SPACE) && !ShiftDown()) )
	{
		TTSCharacterPressed(nChar);
	}
	else
	{
		CListBox::OnChar(nChar, nRepCnt, nFlags);
	}
}

void CTocListBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Detect keys that should reset TTS
	if ( (nChar == VK_BACK) || (nChar == VK_TAB) || (nChar == VK_RETURN) ||
		 (nChar == VK_DELETE) || (nChar == VK_ESCAPE) )
	{
		ResetTTSPhrase();
	}	
	
	if (nChar == VK_DELETE && ((nFlags & 0x2000) == 0))
	{
		CTocFrame *		pTocFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );
		if (pTocFrame)
			pTocFrame->OnCmdDelete();
	}
	else
	{
		CListBox::OnKeyUp(nChar, nRepCnt, nFlags);
	}
}

LRESULT CTocListBox::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_SETFONT)
	{		
		SetItemHeight(0, ScreenFont.CellHeight() + 2);
		((CTocView *)GetParent())->SizeHeaders(); 
	}
		
	return (CListBox::WindowProc(message, wParam, lParam));
}

void CTocListBox::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{				  
  	// Get the menu that contains all the context popups
	CMenu popupMenus;
	HMENU hMenu = QCLoadMenu(IDR_CONTEXT_POPUPS);
	if (!hMenu || !popupMenus.Attach(hMenu))
	{
		ASSERT(0);		// hosed resources?
		return;
	}

	// Out mailbox has a different Popup than the others 
	int nMenuPos;
	if (((CTocView*)GetParent())->GetDocument()->m_Type == MBT_OUT)
		 nMenuPos = MP_POPUP_OUT_MAILBOX;
	else if (((CTocView*)GetParent())->GetDocument()->m_Type == MBT_IMAP_MAILBOX)
		 nMenuPos = MP_POPUP_IMAP_MAILBOX;
	else
		 nMenuPos = MP_POPUP_MAILBOX;

	CMenu* pTempPopupMenu = popupMenus.GetSubMenu(nMenuPos);	
	if (!pTempPopupMenu)
		ASSERT(0);		// uh, oh ... resources hosed?
	else
	{
		//
		// Since the popup menu we get from GetSubMenu() is a pointer
		// to a temporary object, let's make a local copy of the
		// object so that we have explicit control over its lifetime.
		//
		// Note that we edit the context menu on-the-fly in order to
		// stick in the latest/greatest Transfer menu, display the
		// edited context menu, then remove the Transfer menu.
		//
		CMenu tempPopupMenu;
		tempPopupMenu.Attach(pTempPopupMenu->GetSafeHmenu());
	
		//
		// Add bitmaps to certain sub-menus.
		//
		{
			//
			// There are two different mailbox context menus -- one for
			// the Out mailbox and one for all other mailboxes.
			//
			CMenu* pSubMenu = NULL;
			CMenu* pPrioritySubMenu = NULL;
			switch (nMenuPos)
			{
			case MP_POPUP_OUT_MAILBOX:
				VERIFY(pSubMenu = tempPopupMenu.GetSubMenu(6));			// Out Mbx:Change Status
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_UNSENDABLE,  MF_BYCOMMAND, &g_StatusUnsendableBitmap, NULL);
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_SENDABLE,    MF_BYCOMMAND, &g_StatusSendableBitmap, NULL);
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_QUEUED,      MF_BYCOMMAND, &g_StatusQueuedBitmap, NULL);
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_SENT,        MF_BYCOMMAND, &g_StatusSentBitmap, NULL);
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_UNSENT,      MF_BYCOMMAND, &g_StatusUnsentBitmap, NULL);
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_TIME_QUEUED, MF_BYCOMMAND, &g_StatusTimeQueuedBitmap, NULL);

				VERIFY(pPrioritySubMenu = tempPopupMenu.GetSubMenu(7));		// Out Mbx:Change Priority
				break;
			case MP_POPUP_MAILBOX:
			case MP_POPUP_IMAP_MAILBOX:	// IMAP mboxes have slightly modified menus.
				VERIFY(pSubMenu = tempPopupMenu.GetSubMenu(5));				// Mailbox:Change Status
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_UNREAD,     MF_BYCOMMAND, &g_StatusUnreadBitmap, NULL);
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_READ,       MF_BYCOMMAND, &g_StatusReadBitmap, NULL);
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_REPLIED,    MF_BYCOMMAND, &g_StatusRepliedBitmap, NULL);
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_FORWARDED,  MF_BYCOMMAND, &g_StatusForwardedBitmap, NULL);
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_REDIRECTED, MF_BYCOMMAND, &g_StatusRedirectBitmap, NULL);
				pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_RECOVERED, MF_BYCOMMAND, &g_StatusRecoveredBitmap, NULL);

				if (nMenuPos == MP_POPUP_MAILBOX)  // No server status for IMAP.
				{
					VERIFY(pSubMenu = tempPopupMenu.GetSubMenu(8));				// Mailbox:Change Server Status
					pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_LEAVE,        MF_BYCOMMAND, &g_ServerLeaveBitmap, NULL);
					pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_FETCH,        MF_BYCOMMAND, &g_ServerFetchBitmap, NULL);
					pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_DELETE,       MF_BYCOMMAND, &g_ServerDeleteBitmap, NULL);
					pSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_FETCH_DELETE, MF_BYCOMMAND, &g_ServerFetchDeleteBitmap, NULL);
				}

				VERIFY(pPrioritySubMenu = tempPopupMenu.GetSubMenu(6));		// Mailbox:Change Priority
				break;
			default:
				ASSERT(0);
				break;
			}

			if (pPrioritySubMenu)
			{
				pPrioritySubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_HIGHEST, MF_BYCOMMAND, &g_PriorityHighestMenuBitmap, NULL);
				pPrioritySubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_HIGH,    MF_BYCOMMAND, &g_PriorityHighMenuBitmap, NULL);
				pPrioritySubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_NORMAL,  MF_BYCOMMAND, &g_PriorityNormalMenuBitmap, NULL);
				pPrioritySubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_LOW,     MF_BYCOMMAND, &g_PriorityLowMenuBitmap, NULL);
				pPrioritySubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_LOWEST,  MF_BYCOMMAND, &g_PriorityLowestMenuBitmap, NULL);
			}
		}

		tempPopupMenu.InsertMenu(MP_TRANSFER_TOC, MF_BYPOSITION | MF_POPUP,
									(UINT) CMainFrame::QCGetMainFrame()->GetTransferMenu()->GetSafeHmenu(),
									CRString(IDS_TRANSFER_NAME));

		//
		// Add Change Personality menu.
		//
		// Not for IMAP or OUT mbx
		//
		const bool bAddPersonaMenu = (nMenuPos != MP_POPUP_IMAP_MAILBOX && nMenuPos != MP_POPUP_OUT_MAILBOX);

		if (bAddPersonaMenu)
		{
			tempPopupMenu.InsertMenu(MP_TRANSFER_TOC + 5,
										MF_BYPOSITION | MF_POPUP,
										(UINT) CMainFrame::QCGetMainFrame()->GetMessageChangePersonalityMenu()->GetSafeHmenu(),
										CRString(IDS_CHANGE_PERSONA));
		}

		CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
		CContextMenu(&tempPopupMenu, ptScreen.x, ptScreen.y);
		
		if (bAddPersonaMenu)
			tempPopupMenu.RemoveMenu(MP_TRANSFER_TOC + 5, MF_BYPOSITION);
		tempPopupMenu.RemoveMenu(MP_TRANSFER_TOC, MF_BYPOSITION);
		
		VERIFY(tempPopupMenu.Detach());
	}

	popupMenus.DestroyMenu();
} 

// TTSCharacterPressed
// Accumulates characters pressed while the focus is in the TOC list.
// Then triggers type-to-select search based on the current accumulated string.

void CTocListBox::TTSCharacterPressed(UINT nChar)
{
	const DWORD dwTime = GetTickCount();
	const DWORD dwDeltaTime = (dwTime - m_nLastKeyTime);
	const bool bDelayExpired = (dwDeltaTime > kMaxTTSDelay);

	if (bDelayExpired)
	{
		// Reset everything
		ResetTTSPhrase();
	}

	if (m_nFindPhraseLen < 32)
	{
		// Append the typed character
		m_pFindPhrase[m_nFindPhraseLen] = ((char) nChar);
		m_pFindPhrase[++m_nFindPhraseLen] = '\0';

		FindTTS();

		m_nLastKeyTime = dwTime;
	}
	// else we've already found the longest phrase
}

void CTocListBox::FindTTS()
{
	const int		nCount = GetCount();
	CSummary *		pSum = NULL;
	int				nFoundIndex = -1;
	CTocView *		pTocView = reinterpret_cast<CTocView *>( GetParent() );
	bool			bTTSFromTop = (GetIniShort(IDS_INI_TTS_FROM_TOP) != 0);

	if (pTocView)
	{
		// Start with a string distance that we'll beat if we find anything
		int		nMinStringDistance = INT_MAX;
		int		nCurrentStringDistance;

		// Determine where to start - at the end (default) or beginning (if specified)
		int		nIndex = bTTSFromTop ? 0 : nCount - 1;
		
		do
		{
			pSum = GetItemDataPtr(nIndex);
			ASSERT( pSum && (pSum != reinterpret_cast<CSummary *>(-1)) );

			if ( pSum && (pSum != reinterpret_cast<CSummary *>(-1)) )
			{
				// If the last primary sort type was by subject, get the subject
				// skipping the prefix. Otherwise get the "From".
				LPCSTR		pTargetString = pTocView->IsLastSortBySubject() ?
											SkipSubjectPrefix( pSum->GetSubject() ) :
											pSum->GetFrom();

				// Match case insensitively, and allow substring matches
				LPSTR		pFound = QCStrstr( pTargetString, m_pFindPhrase,
											   QCSTR_CASE_INSENSITIVE, QCSTR_SUBSTR_SEARCH );
				if (pFound)
				{
					// Calculate the distance - how "good" the match is.
					// This gives preference to matches found at the start
					// of the subject.
					nCurrentStringDistance = pFound - pTargetString;
					
					// If we have a substring match - penalize that heavily.
					// Constant taken from the Mac code.
					if ( (pFound > pTargetString) && isalnum((unsigned char)pFound[-1]) )
						nCurrentStringDistance += 100;

					if (nCurrentStringDistance < nMinStringDistance)
					{
						// Best match so far. Remember it.
						nFoundIndex = nIndex;
						nMinStringDistance = nCurrentStringDistance;
					}
				}
			}

			if (bTTSFromTop)
			{
				// Searching from the top - advance to the next item down
				// and stop when we go past the last item.
				nIndex++;
				if (nIndex == nCount)
					break;
			}
			else
			{
				// Searching from the bottom - back up to the next item up
				// and stop when we go before the first item.
				nIndex--;
				if (nIndex < 0)
					break;
			}
			
			// Stop if we've already gotten the best possible match
		} while (nMinStringDistance != 0);
	}

	if (nFoundIndex != -1)
	{
		// Only change the selection if we really need to
		if ( (GetSel(nFoundIndex) <= 0) || (GetSelCount() != 1) )
		{
			SetSelCenter(nFoundIndex); // Select the item and scroll it to center
			UpdateWindow(); // Force the selection to be shown
		}
	}
	else // Not found
	{
		::MessageBeep(MB_ICONEXCLAMATION);
	}
}

LPSTR CTocListBox::SkipSubjectPrefix(LPCSTR pSubject)
{
	LPSTR pStr = (LPSTR) pSubject;
	bool bMatchFWD = false;

	// Check for a prefix
	while ( (bMatchFWD = (_strnicmp(pStr, "FWD:", 4) == 0)) || (_strnicmp(pStr, "RE:", 3) == 0) )
	{
		// Skip over the prefix
		if (bMatchFWD)
			pStr += 4; // "FWD:"
		else
			pStr += 3; // "RE:"

		// Skip any following spaces
		while (' ' == (*pStr))
			pStr++;
	}

	return pStr;
}

void CTocListBox::RedisplaySelection()
{
	int Index = GetTopIndex();

	if (Index != LB_ERR)
	{
		int Count = GetCount();
		CRect rect, ClientRect;

		GetClientRect(&ClientRect);
		for (; Index < Count; Index++)
		{
			if (GetItemRect(Index, &rect) == LB_ERR)
				break;
				
			if (GetSel(Index) > 0)
			{
				CSummary* Sum = GetItemDataPtr(Index);
				if (Sum)
				{
					InvalidateRect(&rect);
//					UpdateWindow();
				}
				
				if (rect.bottom >= ClientRect.bottom)
					break;
			}
		}
	}
}


void CTocListBox::OnKillfocus() 
{
	RedisplaySelection();
}

void CTocListBox::OnSetfocus() 
{
	RedisplaySelection();
}


////////////////////////////////////////////////////////////////////////
// OnSelChange [protected]
//
// The LBN_SELCHANGE happens after the selection has been drawn, but
// that's ok. We no longer check selection in the DrawItem code
// (for numerous reasons multiple selection is happier this way -
// DrawItem required use of timers, whereas they're optional this
// way; DrawItem generated numerous superfluous notficaitions; etc.).
////////////////////////////////////////////////////////////////////////
void CTocListBox::OnSelChange() 
{
	if (m_bAllowSelChangeNotifications)
	{
		HandleSelectionMayHaveChanged();
	}
}


////////////////////////////////////////////////////////////////////////
// OnSetSel [protected]
//
// We go ahead and call the same HandleSelectionMayHaveChanged helper
// method that OnSelChange calls. See comments in that method for
// further discussion regarding OnSetSel and auto-mark-as-read.
////////////////////////////////////////////////////////////////////////
LRESULT CTocListBox::OnSetSel(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = DefWindowProc(LB_SETSEL, wParam, lParam);

	if (m_bAllowSelChangeNotifications)
	{
		//	OnSetSel gets called immediately when SetSel is called.
		//	If appropriate m_bSelectionResultDirectUserAction is set to true
		//	just before CListBox::SetSel is called.
		HandleSelectionMayHaveChanged(m_bSelectionResultDirectUserAction);
	}

	return lResult;
}


//
//	CTocListBox::MaybeSetSelCenter()
//
//	If the specified item is not currently visible scroll it into view.
//
void CTocListBox::MaybeSetSelCenter(int idx, bool bUnSelAll /* = true */)
{
	const int nItemHeight = GetItemHeight(0);
	ASSERT(nItemHeight > 0);

	CRect rct;
	GetClientRect(rct);
	const int nListHeight = rct.Height();
	ASSERT(nListHeight > 0);

	if ((nListHeight > 0) && (nItemHeight > 0))
	{
		int		 nVisibleItemCount = nListHeight / nItemHeight;
		int		 nTopIdx = GetTopIndex();
		if ((idx < nTopIdx) || (idx > (nTopIdx + nVisibleItemCount - 1)))
		{
			SetSelCenter(idx, bUnSelAll);
		}
	}
}


void CTocListBox::SetSelCenter(int idx, bool bUnSelAll /* = true */)
{
	bool bChangeTopIdx = false;
	int nTopIdx = 0;

	const int nItemHeight = GetItemHeight(0);
	ASSERT(nItemHeight > 0);

	CRect rct;
	GetClientRect(rct);
	const int nListHeight = rct.Height();
	ASSERT(nListHeight > 0);

	if ((nListHeight > 0) && (nItemHeight > 0))
	{
		const int nVisibleItemCount = nListHeight / nItemHeight;
		const int nMaxTopIdx = GetCount() - nVisibleItemCount;
		const int nScrollTopIdx = idx - (nVisibleItemCount / 2);

		nTopIdx = __min(nScrollTopIdx, nMaxTopIdx);
		nTopIdx = __max(0, nTopIdx);

		bChangeTopIdx = (GetTopIndex() != nTopIdx);
	}

	SetRedraw(FALSE);

	if (bUnSelAll)
		SetSel(-1, FALSE);

	// Everyplace that SetSelCenter is called is the result of direct user action
	// Select the correct item
	SetSel(idx, TRUE, true);

	if (bChangeTopIdx)
		SetTopIndex(nTopIdx);

	SetRedraw(TRUE);
}


void CTocListBox::ResetContent()
{
	bool	bSaveAllowSelChangeNotifications = m_bAllowSelChangeNotifications;
	
	//	Make sure that HandleSelectionMayHaveChanged is not called while
	//	ResetContent is operating. Otherwise HandleSelectionMayHaveChanged
	//	is called once for each element in the list, which results in *really*
	//	slow performance. Sorting is currently the only code that calls
	//	CTocListBox::ResetContent, and it was severely affected by this.
	//
	//	Note that it is the responsibility of the caller to call
	//	HandleSelectionMayHaveChanged as appropriate after calling ResetContent.
	m_bAllowSelChangeNotifications = false;
	CListBox::ResetContent();
	m_bAllowSelChangeNotifications = bSaveAllowSelChangeNotifications;
}


void CTocListBox::HandleSelectionMayHaveChanged(bool bSelectionResultDirectUserAction)
{
	if ( GetIniShort(IDS_INI_SHOW_SEL_SUMMARY_COUNT) )
	{
		CTocView *		pTocView = reinterpret_cast<CTocView *>( GetParent() );
		if (pTocView)
			pTocView->SetCompact();
	}

	// When the selection is changed directly by the user (i.e. by clicking or keyboard
	// navigation) OnSelChange is called. As such OnSelChange will always pass true
	// for bSelectionResultDirectUserAction, which means that it will always invoke
	// the auto-mark-as-read behavior.
	//
	// When the selection is changed programmatically OnSetSel is called. In some cases
	// the selection is changed programmatically as the result of direct user action
	// (e.g. Select All) in others it is not the result of direct user action (FUMLUB).
	// Cases that *are* the result of direct user action either:
	// * Call the overloaded version of SetSel in CTocListBox with true as the last
	//	 parameter (or equivalently CTocView::Select)
	// * Call MailboxChangedByHuman immediately after performing their action
	//
	// Even if bSelectionResultDirectUserAction is true, the auto-mark-as-read
	// behavior will only mark messages as read if they're displayed to the user
	// for a long enough time period (multiple message selections that don't get
	// displayed won't trigger auto-mark-as-read).
	CTocFrame *		pTocFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );
	if (pTocFrame)
		pTocFrame->NotifyPreviewSelectionMayHaveChanged(false, bSelectionResultDirectUserAction);
}


/////////////////////////////////////////////////////////////////////////////
// CTocHeaderButton

CImageList CTocHeaderButton::s_ilDigits4x5;

CTocHeaderButton::CTocHeaderButton()
{
	if (s_ilDigits4x5.m_hImageList == NULL)
		VERIFY(s_ilDigits4x5.Create(IDB_IL_DIGITS_4X5, 4, 0, RGB(192, 192, 192)));
}

CTocHeaderButton::~CTocHeaderButton()
{
}


BEGIN_MESSAGE_MAP(CTocHeaderButton, CButton)
	//{{AFX_MSG_MAP(CTocHeaderButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////
// LoadButtonImageCache [public, static]
//
// Sets up an image cache for TOC "column button" images.
////////////////////////////////////////////////////////////////////////
BOOL CTocHeaderButton::LoadButtonImageCache()
{
	if (CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYSTATUS, CTocHeaderButton::m_hBitmapSortByStatus) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYJUNK, CTocHeaderButton::m_hBitmapSortByJunk) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYPRIORITY, CTocHeaderButton::m_hBitmapSortByPriority) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYATTACHMENT, CTocHeaderButton::m_hBitmapSortByAttachment) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYLABEL, CTocHeaderButton::m_hBitmapSortByLabel) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYSENDER, CTocHeaderButton::m_hBitmapSortBySender) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYDATE, CTocHeaderButton::m_hBitmapSortByDate) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYSIZE, CTocHeaderButton::m_hBitmapSortBySize) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYSERVERSTATUS, CTocHeaderButton::m_hBitmapSortByServerStatus) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYMOOD, CTocHeaderButton::m_hBitmapSortByMood) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYSUBJECT, CTocHeaderButton::m_hBitmapSortBySubject))
	{
		return TRUE;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// LoadOneButtonImage [private, static]
//
// Sets up a single TOC "column button" image.
////////////////////////////////////////////////////////////////////////
BOOL CTocHeaderButton::LoadOneButtonImage(int nResourceID, HBITMAP& hBitmap)
{
	if (NULL == hBitmap)
	{
		HINSTANCE hResInst = ::QCFindResourceHandle(MAKEINTRESOURCE(nResourceID), RT_BITMAP);
		hBitmap = HBITMAP(::LoadImage(hResInst, MAKEINTRESOURCE(nResourceID), IMAGE_BITMAP, 0, 0, 
										LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS | LR_SHARED));

		if (NULL == hBitmap)
		{
			ASSERT(0);
			return FALSE;
		}
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// PurgeButtonImageCache [public, static]
//
// Cleans up the image cache for TOC "column button" images.
////////////////////////////////////////////////////////////////////////
void CTocHeaderButton::PurgeButtonImageCache()
{
#define PURGE_BITMAP(_hBitmap_) if (_hBitmap_) { ::DeleteObject(_hBitmap_); _hBitmap_ = NULL; }

	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByStatus)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByJunk)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByPriority)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByAttachment)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByLabel)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortBySender)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByDate)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortBySize)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByServerStatus)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByMood)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortBySubject)
}


/////////////////////////////////////////////////////////////////////////////
// CTocHeaderButton message handlers

void CTocHeaderButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	const int Action = lpDrawItemStruct->itemAction;
	const UINT nID = lpDrawItemStruct->CtlID;
	SortType ColumnSortType = NOT_SORTED;
	int ColumnSortIndex = -1;
	BOOL bIsGroupBySubject = FALSE;
	BOOL bIsReverseSort = FALSE;

	CTocView* Parent = STATIC_DOWNCAST(CTocView, GetParent());
	if (Parent)
	{
		for (int i = 0; i <= ID_EDIT_SORT_SORTBYSUBJECT - ID_EDIT_SORT_SORTBYSTATUS; i++)
		{
			SortType ActualSort = Parent->GetSortMethod(i);
			if (ActualSort == NOT_SORTED)
				break;
			bIsReverseSort = (ActualSort >= BY_REVERSE_STATUS);
			if (nID == (UINT)(ID_EDIT_SORT_SORTBYSTATUS + ActualSort - (bIsReverseSort? BY_REVERSE_STATUS : 1)))
			{
				ColumnSortType = ActualSort;
				ColumnSortIndex = i;
				break;
			}
		}
		bIsGroupBySubject = (nID == ID_EDIT_SORT_SORTBYSUBJECT && Parent->GetGroupBySubject());
	}

	if (Action == ODA_SELECT || Action == ODA_DRAWENTIRE)
	{
		CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
		VERIFY(pDC);
		CRect rectButton(lpDrawItemStruct->rcItem);
		BOOL bSelected = (lpDrawItemStruct->itemState & ODS_SELECTED);
		CString Text;

		// Easy way to draw buttons: Win32-style
		pDC->DrawFrameControl(&rectButton, DFC_BUTTON, DFCS_BUTTONPUSH | (bSelected? DFCS_FLAT : 0));

		switch (nID)
		{
		case ID_EDIT_SORT_SORTBYSTATUS:
		case ID_EDIT_SORT_SORTBYPRIORITY:
		case ID_EDIT_SORT_SORTBYATTACHMENT:
		case ID_EDIT_SORT_SORTBYSIZE:
		case ID_EDIT_SORT_SORTBYMOOD:
		case ID_EDIT_SORT_SORTBYJUNK:
		case ID_EDIT_SORT_SORTBYSERVERSTATUS:
			if (ColumnSortType == NOT_SORTED)
			{
				// Draw centered bitmap, in lieu of button text.
				// But only if there isn't a sort triangle being displayed.
				CBitmap* pBitmap = NULL;
				int xOffset = 0;
				int yOffset = 0;

				{
					switch (nID)
					{
					case ID_EDIT_SORT_SORTBYSTATUS:			pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortByStatus); break;
					case ID_EDIT_SORT_SORTBYPRIORITY:		pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortByPriority); break;
					case ID_EDIT_SORT_SORTBYATTACHMENT:		pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortByAttachment); break;
					case ID_EDIT_SORT_SORTBYSIZE:			pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortBySize); break;
					case ID_EDIT_SORT_SORTBYMOOD:			pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortByMood); break;
					case ID_EDIT_SORT_SORTBYJUNK:			pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortByJunk); break;
					case ID_EDIT_SORT_SORTBYSERVERSTATUS:	pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortByServerStatus); break;
					}

					//
					// Get bitmap dimensions so we can draw the image centered
					// on the button.
					//
					if (NULL == pBitmap)
					{
						ASSERT(0);
						return;
					}
					else
					{
						BITMAP bm;
						pBitmap->GetObject(sizeof(BITMAP), &bm);

						xOffset = (rectButton.Width() - bm.bmWidth) / 2;
						yOffset = (rectButton.Height() - bm.bmHeight) / 2;

						if (bSelected)
						{
							xOffset += 1;	// cheap animation effect...
							yOffset += 1;
						}
					}
				}

				CPaletteDC memDC;
				memDC.CreateCDC();
				CBitmap* pOldBitmap = memDC.SelectObject(pBitmap);
				if (pOldBitmap == NULL)
					return;
				pDC->BitBlt(rectButton.left+xOffset, rectButton.top+yOffset, 
							rectButton.Width(), rectButton.Height(),
							&memDC, 0, 0, SRCCOPY);
				memDC.SelectObject(pOldBitmap);
			}
			break;

		case ID_EDIT_SORT_SORTBYLABEL:
		case ID_EDIT_SORT_SORTBYSENDER:
		case ID_EDIT_SORT_SORTBYDATE:
		case ID_EDIT_SORT_SORTBYSUBJECT:
			{
				//
				// Draw left-justified bitmap.
				//
				CBitmap* pBitmap = NULL;
				int xOffset = 2;
				int yOffset = 0;
				int xTextOffset = 0;

				{
					switch (nID)
					{
					case ID_EDIT_SORT_SORTBYLABEL:		pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortByLabel); break;
					case ID_EDIT_SORT_SORTBYSENDER:		pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortBySender); break;
					case ID_EDIT_SORT_SORTBYDATE:		pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortByDate); break;
					case ID_EDIT_SORT_SORTBYSUBJECT:	pBitmap = CBitmap::FromHandle(CTocHeaderButton::m_hBitmapSortBySubject); break;
					}

					//
					// Get bitmap dimensions so we can draw the image 
					// vertically centered on the button.
					//
					if (NULL == pBitmap)
					{
						ASSERT(0);
						return;
					}
					else
					{
						BITMAP bm;
						pBitmap->GetObject(sizeof(BITMAP), &bm);

						yOffset = (rectButton.Height() - bm.bmHeight) / 2;

						if (bSelected)
						{
							xOffset += 1;	// cheap animation effect...
							yOffset += 1;
						}

						xTextOffset = bm.bmWidth + 3;
					}
				}

				CPaletteDC memDC;
				memDC.CreateCDC();
				CBitmap* pOldBitmap = memDC.SelectObject(pBitmap);
				if (pOldBitmap == NULL)
					return;
				pDC->BitBlt(rectButton.left+xOffset, rectButton.top+yOffset, 
							rectButton.Width(), rectButton.Height(),
							&memDC, 0, 0, SRCCOPY);
				memDC.SelectObject(pOldBitmap);

				//
				// Draw the button label text to the right of the bitmap.
				//
				GetWindowText(Text);

				// Add bullet char to indicate Group by Subject
				if (bIsGroupBySubject)
					Text += '\225';

				rectButton.OffsetRect(xTextOffset, 0);		// offset text to account for bitmap
				rectButton.InflateRect(-(ColumnSpace - 1), -2);
				if (bSelected)
				{
					rectButton.top += 1;	// cheap animation effect...
					rectButton.left += 1;
				}

				//
				// Shorten the available text area by the bitmap width.
				// This could result in a rectangle with negative width.
				//
				rectButton.SetRect(rectButton.left, rectButton.top, 
									rectButton.right - xTextOffset,
									rectButton.bottom);

				if (rectButton.Width() > 0)
				{
					int oldmode = pDC->SetBkMode(TRANSPARENT);
					pDC->DrawText(Text, -1, rectButton, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
					pDC->SetBkMode(oldmode);
				}
			}
			break;

		default:
			ASSERT(0);
			return;
		}

		if (ColumnSortType != NOT_SORTED)
		{
			TEXTMETRIC tm;
			VERIFY(pDC->GetTextMetrics(&tm));
			int Height = tm.tmHeight - tm.tmInternalLeading - tm.tmExternalLeading;
			if ((Height / 2) * 2 == Height)
				Height--;

			pDC->DrawText(Text, -1, rectButton, DT_CALCRECT | DT_LEFT | DT_SINGLELINE | DT_VCENTER);
			rectButton.left = rectButton.right + 4;
			rectButton.right = rectButton.left + Height;

			CPen DarkPen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
			CPen LightPen(PS_SOLID, 1, GetSysColor(COLOR_3DHIGHLIGHT));
			CPen* OldPen;
			if (bIsReverseSort)
			{
				CPoint Corner(rectButton.left + Height / 2,
							rectButton.CenterPoint().y + Height / 2 + 1);

				VERIFY(OldPen = pDC->SelectObject(&DarkPen));
				pDC->MoveTo(Corner);
				Corner.x -= Height / 2;
				Corner.y -= Height - 1;
				VERIFY(pDC->LineTo(Corner));
				Corner.x += Height;
				VERIFY(pDC->LineTo(Corner));

				VERIFY(pDC->SelectObject(&LightPen));
				Corner.x -= 1;
				Corner.y += 1;
				pDC->MoveTo(Corner);
				Corner.x -= Height / 2;
				Corner.y += Height - 2;
				VERIFY(pDC->LineTo(Corner));
			}
			else
			{
				CPoint Corner(rectButton.left + Height / 2 + 1,
							rectButton.CenterPoint().y - Height / 2);
				pDC->MoveTo(Corner);

				VERIFY(OldPen = pDC->SelectObject(&LightPen));
				Corner.x += Height / 2 - 1;
				Corner.y += Height - 2;
				VERIFY(pDC->LineTo(Corner));
				Corner.x -= Height;
				VERIFY(pDC->LineTo(Corner));

				VERIFY(pDC->SelectObject(&DarkPen));
				Corner.x += Height / 2 + 1;
				Corner.y -= Height - 1;
				pDC->MoveTo(Corner);
				Corner.x -= Height / 2;
				Corner.y += Height - 1;
				VERIFY(pDC->LineTo(Corner));
			}
			VERIFY(pDC->SelectObject(OldPen));

			if (ColumnSortIndex > 0 || Parent->GetSortMethod(1) != NOT_SORTED)
			{
				CPoint TopLeft(rectButton.left + Height / 2 - 2, rectButton.CenterPoint().y - 1);
				if (bIsReverseSort)
					TopLeft.y -= 2;
				s_ilDigits4x5.Draw(pDC, ColumnSortIndex + 1, TopLeft, ILD_NORMAL);
			}
		}
	}
}

void CTocHeaderButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect;

	GetClientRect(&rect);

	// Don't allow 0,0 to be a valid place to start sizing.  If 0, 0 were allowed
	// the accelerator keys will put the user in sizing mode.

	if( ( ( point.x != 0 ) || ( point.y != 0 ) ) && 
		( ( point.x < 3 ) || ( point.x > rect.right - 6 ) ) )
	{
		MapWindowPoints(GetParent(), &point, 1);
		GetParent()->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
		return;
	}

	CButton::OnLButtonDown(nFlags, point);
}

void CTocHeaderButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rect;

	GetClientRect(&rect);
	if (point.x < 3 || point.x > rect.right - 6)
	{
		MapWindowPoints(GetParent(), &point, 1);
		GetParent()->SendMessage(WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y));
		return;
	}

	CButton::OnMouseMove(nFlags, point);
}

void CTocHeaderButton::OnContextMenu(CWnd*, CPoint point) 
{
	s_nSortPopupID = GetDlgCtrlID();
	CRString StatusText(s_nSortPopupID);
	LPCTSTR MenuText = strchr(StatusText, '\n');
	CMenu SortMenu;
	CMenu* SortPopupMenu;

	if (!MenuText || !SortMenu.Attach(QCLoadMenu(IDR_MAILBOX_SORT_POPUP)) ||
		!(SortPopupMenu = SortMenu.GetSubMenu(0)))
	{
		ASSERT(0);
		return;
	}

	MenuText++;

	for (int i = SortPopupMenu->GetMenuItemCount() - 1; i >= 0; i--)
	{
		UINT nMenuID = SortPopupMenu->GetMenuItemID(i);
		if (nMenuID == ID_MAILBOX_SORT_NONE ||
			nMenuID == ID_MAILBOX_SORT_ASCEND ||
			nMenuID == ID_MAILBOX_SORT_DESCEND)
		{
			CString TextFormat;
			CString FinalText;
			SortPopupMenu->GetMenuString(i, TextFormat, MF_BYPOSITION);
			FinalText.Format(TextFormat, MenuText);
			SortPopupMenu->ModifyMenu(i, MF_BYPOSITION | MF_STRING, nMenuID, FinalText);
		}
	}

	SortPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
}


/////////////////////////////////////////////////////////////////////////////
// CTocView

IMPLEMENT_DYNCREATE(CTocView, C3DFormView)


CTocView::CTocView()
	:	C3DFormView(CTocView::IDD), m_bProfileComboBoxIsForMultipleSel(false)
{
	//{{AFX_DATA_INIT(CTocView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_OldWidth = m_OldHeight = 0;

	// initialize the StatusFlag
	CRString tmpFlags( IDS_STATUS_FLAGS );
	if ( ! tmpFlags.IsEmpty() )
	{
		int bytes = sizeof( StateFlag );
		memcpy( (char *)&StateFlag[0], tmpFlags, bytes );
	}
	
	g_theMailboxDirector.Register( this );
	m_pSWM = GetSharewareManager();
	if (m_pSWM)
	{
		m_pSWM->QCCommandDirector::Register((QICommandClient*)this);
	}
}

CTocView::~CTocView()
{
	g_theMailboxDirector.UnRegister( this );
	if (m_pSWM)
	{
		m_pSWM->QCCommandDirector::UnRegister((QICommandClient*)this);
	}
}

void CTocView::SelectAll(BOOL bSelect /*= TRUE*/, BOOL bRedisplay /*= TRUE*/, bool in_bResultDirectUserAction /*=false*/)
{
	int SelCount = m_SumListBox.GetSelCount();
	
	m_SumListBox.SetRedraw(FALSE);
	if (SelCount || bSelect)
		m_SumListBox.SetSel(-1, bSelect, in_bResultDirectUserAction);
	if (bRedisplay)
	{
		m_SumListBox.SetRedraw(TRUE);
	}
}

////////////////////////////////////////////////////////////////////////
// GetSelText [public]
//
// Copies the tab-separated fields of the current selection to the 
// given string object.
////////////////////////////////////////////////////////////////////////
BOOL CTocView::GetSelText(CString& rString)
{
	int num_selected = m_SumListBox.GetSelCount();

	//
	// Build clipboard buffer string, one item at a time.  For best
	// CString performance, we pre-allocate an estimated 100 bytes per 
	// summary to hold the text for the summary.
	//
	rString.Empty();
	rString.GetBuffer(num_selected * 100);
	::AsyncEscapePressed(TRUE);			// reset Escape key logic
	for (int i = 0; i < m_SumListBox.GetCount() && num_selected > 0; i++)
	{
		if (::AsyncEscapePressed())
			break;
		if (m_SumListBox.GetSel(i) > 0)
		{
			CString item;
			if (m_SumListBox.GetItemText(i, item))
			{
				if (! rString.IsEmpty())
					rString += "\r\n";		// for compatibility with Notepad
				rString += item;
			}

			num_selected--;
		}
	}
	rString.FreeExtra();

	return TRUE;
}
	
void CTocView::DoDataExchange(CDataExchange* pDX)
{
	C3DFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTocView)
	DDX_Control(pDX, IDC_COMPACT, m_Compact);
	DDX_Control(pDX, IDC_PROFILE_COMBO_BOX, m_ProfileCombo);
	DDX_Control(pDX, IDC_HSCROLL_BAR, m_HScrollBar);
	DDX_Control(pDX, IDC_SUMLIST, m_SumListBox);
	//}}AFX_DATA_MAP
	for (UINT i = ID_EDIT_SORT_SORTBYSTATUS; i <= ID_EDIT_SORT_SORTBYSUBJECT; i++)
		DDX_Control(pDX, i, m_HeaderButton[i - ID_EDIT_SORT_SORTBYSTATUS]);
}



void CTocView::OnInitialUpdate()
{
	//
	// Make sure the TOC column buttons have their image cache loaded up.
	//
	CTocHeaderButton::LoadButtonImageCache();

//FORNOW	//
//FORNOW	// Make sure the TOC list box has its Status images loaded up.
//FORNOW	//
//FORNOW	CTocListBox::PreloadStatusBitmaps();

	CFormView::OnInitialUpdate();

	SortType	primarySortMethod = GetSortMethod(0);
	
	m_bLastSortBySubject = (primarySortMethod == BY_SUBJECT) || (primarySortMethod == BY_REVERSE_SUBJECT);
	
	CTocDoc *		TocDoc = GetDocument();

	CSumList &		listSums = TocDoc->GetSumList();

	CTocFrame *		pTocFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );
	ASSERT_KINDOF(CTocFrame, pTocFrame);

	// Size parent with saved position
	if (!TocDoc->m_SavedPos.IsRectEmpty())
		GetParentFrame()->MoveWindow(&TocDoc->m_SavedPos, FALSE);

	// Size parent preview pane
	{
		pTocFrame->ShowHideSplitter();
	}

    // Set the font and item height of some of the summary list box
	m_SumListBox.SetFont(&ScreenFont, FALSE);
	m_SumListBox.SetItemHeight(0, ScreenFont.CellHeight() + 2);

	// Set the font of the header controls and the compact button
	for (UINT i = ID_EDIT_SORT_SORTBYSTATUS; i <= ID_EDIT_SORT_SORTBYSUBJECT; i++)
	{
		CWnd* Button = GetDlgItem(i);
		
		if (Button)
		{
			Button->SetFont(&ScreenFont, FALSE); 
		}
	}
   	
	// Set the text and the enable/disable status of the compact button
	SetCompact();

	// Size the controls
	// This is a kludge so it doesn't assert every time this view is resized
	SetScaleToFitSize(CSize(1, 1));
	SetFieldSeparators();
	m_SumListBox.SetHorizontalExtent(m_FieldSep[FW_UNUSED - 1] +
		ScreenFont.CharWidth() * sizeof(((CSummary*)i)->m_Subject));
	m_SumListBox.ShowScrollBar(SB_HORZ, FALSE);
	m_OldWidth = m_OldHeight = 0;
	OnSize(SIZE_RESTORED, -1, -1);
	m_HScrollBar.SetScrollPos(0);
	
	BOOL bListBoxFilled = FALSE;

	// Attempt to avoid flickers
	m_SumListBox.SetRedraw(FALSE);
	if (TocDoc->m_bNeedsSorting || TocDoc->m_bNeedsSortingNOW)
	{
		// Sorting may add the items to the listbox
		bListBoxFilled = SortNow();
	}
	if (!bListBoxFilled)
	{
		FillInContents();
	}

	// Notify the parent CTocFrame that the sum list box is available
	if (pTocFrame)
		pTocFrame->SetSumListBox(&m_SumListBox);

	BOOL bDateDescending = TocDoc->SortedDateDescending();

	if(bDateDescending)
	{
		int iCount = listSums.GetCount(TocDoc->HideDeletedIMAPMsgs());

		if(iCount > 0)
		{
			m_SumListBox.SetSel(0);
			m_SumListBox.SetTopIndex(0);
		}
	}
	else
	{
		// Select the last summary
		int Index = listSums.GetCount(TocDoc->HideDeletedIMAPMsgs()) - 1;
		m_SumListBox.SetSel(Index);

		m_SumListBox.SetTopIndex(Index);
	}

	// Ok to draw again
	m_SumListBox.SetRedraw(TRUE);

	// keyboard messages would sometimes go bye-bye...
	GetDlgItem((UINT)ID_EDIT_SORT_SORTBYSENDER)->SetFocus();

	int			nHorizScrollbarHeight = GetSystemMetrics(SM_CYHSCROLL);

	// Guess work attempt to make the combo box play nicely in same
	// height as scrollbar. Draws somewhat clipped at the moment.
	m_ProfileCombo.SetExtendedStyle(CBES_EX_NOSIZELIMIT, CBES_EX_NOSIZELIMIT);
	m_ProfileCombo.SetItemHeight(-1, nHorizScrollbarHeight-6);

	// Tell the Recent Mailboxes code that this mailbox should be added to the list,
	// but only if we're not in the middle of opening up windows from a previous session
	if (GetIniShort(IDS_INI_OPEN_WINDOWS_FAILED) == FALSE)
		QCMailboxDirector::UpdateRecentMailboxList(TocDoc->GetMBFileName(), TRUE);
}

    
BEGIN_MESSAGE_MAP(CTocView, C3DFormView)
	//{{AFX_MSG_MAP(CTocView)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_BN_CLICKED(IDC_COMPACT, OnCompact)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP

	// Commands not handled by ClassWizard
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateSelectAll)

	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnSelectAll)
	
	ON_COMMAND_EX_RANGE(ID_EDIT_SORT_SORTBYSTATUS, ID_EDIT_SORT_SORTBYSUBJECT, Sort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_SORT_SORTBYSTATUS, ID_EDIT_SORT_SORTBYSUBJECT, OnUpdateSort)

	ON_COMMAND(ID_EDIT_SORT_GROUPBYSUBJECT, OnGroupBySubject)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SORT_GROUPBYSUBJECT, OnUpdateGroupBySubject)

	ON_COMMAND_EX_RANGE(ID_MAILBOX_SORT_NONE, ID_MAILBOX_SORT_DESCEND, OnSortPopup)
	ON_UPDATE_COMMAND_UI_RANGE(ID_MAILBOX_SORT_NONE, ID_MAILBOX_SORT_DESCEND, OnUpdateSortPopup)

	ON_CBN_SELCHANGE(IDC_PROFILE_COMBO_BOX, OnProfileComboBoxItemSelected)

	ON_COMMAND_EX(IDOK, OnEnterKey)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTocView message handlers


////////////////////////////////////////////////////////////////////////
// OnFileSaveAs [protected]
//
// In previous versions of Eudora, the File Save As command for a TOC 
// entry used to be identical to the File Save As command for a message
// window.  It also formerly wrote out the contents of the message
// views in order to capture any unsaved changes (even going as far as
// creating temporary hidden message windows if the message windows
// didn't already exist).
//
// However, now that we have different message view window types (Paige
// vs. Bident/Trident), this gets all messed up.  Creating temporary
// Bident/Trident views is particularly nasty since you have to wait for
// the view to completely render the message (and any remote server-based
// graphics) before you could save out the view's contents.  Blech.
// We also found that creating temporary Paige views would confuse the
// hell out of MFC and it would cause mysterious crashes.
//
// So....   the solution is to fall back to the older 3.x style of 
// using the *documents* instead of views for saving out the message
// data.  Yes, this means that you can get some ugly HTML-ish source in 
// your output file, but too bad.  Creating temporary views just to
// save out the message data is just too ugly.  :-)
//
////////////////////////////////////////////////////////////////////////
void CTocView::OnFileSaveAs()
{
	int				SelCount = m_SumListBox.GetSelCount();
	int				TotalCount = m_SumListBox.GetCount();
	int				FoundOne = FALSE;
	char			szName[_MAX_PATH + 1];
	CString			szPathName;
	JJFile			theFile;
	CSummary		*pSummary;

	//Get the Filename to save the multiple messages

	for (int i = 0, j = SelCount; j && i < TotalCount && !FoundOne; i++)
	{
		if (m_SumListBox.GetSel(i) <= 0 )
		{
			continue;			
		}
		
		--j;

		pSummary = m_SumListBox.GetItemDataPtr(i);
		
		if ( !pSummary )
			continue;

		strcpy( szName, pSummary->m_Subject );

		if (! *szName )
			continue;

		::StripIllegalMT( szName, EudoraDir );

		if( !::LongFileSupportMT( EudoraDir ) )
		{
			szName[8] = 0;
		}

		FoundOne = TRUE;
	}

	//
	// Unlike File Saves from message windows, you cannot save the
	// message data as an HTML file.  When saving a TOC selection, it
	// doesn't really make sense to save it as an HTML file anyway for
	// multiple-selections or when the user wants to include headers.
	//
	CSaveAsDialog theDlg(	szName, 
							TRUE, 
							FALSE, 
							CRString( IDS_TEXT_EXTENSION ), 
							CRString( IDS_TEXT_FILE_FILTER ), 
							NULL );
					
				
	if ( theDlg.DoModal() != IDOK )
	{
		return;
	}
				
	//
	// Hack alert!  Under the 32-bit Version 4 shell, the OnOK()
	// method of dialog doesn't get called!  Therefore, this is a
	// hack workaround to manually update these INI settings
	// outside of the dialog class.  Whatta hack.
	//
	if (IsVersion4())
	{
		SetIniShort(IDS_INI_INCLUDE_HEADERS, ( short ) theDlg.m_Inc );
		SetIniShort(IDS_INI_GUESS_PARAGRAPHS, ( short ) theDlg.m_Guess);
	}

	szPathName = theDlg.GetPathName();

	CCursor waitCursor;
	if (FAILED(theFile.Open( szPathName, O_CREAT | O_TRUNC | O_WRONLY)))
		return;


	BOOL bIncludeHeaders = BOOL(::GetIniShort(IDS_INI_INCLUDE_HEADERS));	// cached for speed
	BOOL bGuessParagraphs = BOOL(::GetIniShort(IDS_INI_GUESS_PARAGRAPHS));	// cached for speed

	for (i = 0, j = SelCount; j && i < TotalCount; i++)
	{

		// Start of the main loop going through all of the list items
		if ( m_SumListBox.GetSel(i) <= 0 )
		{
			continue;
		}

		--j;

		pSummary = m_SumListBox.GetItemDataPtr(i);
		
		if ( !pSummary )
			continue;

		// Will be true for view-less preview documents
		bool bCreatedDoc = false;
		CMessageDoc* pDoc = pSummary->GetMessageDoc(&bCreatedDoc);
		if (pDoc)
		{
			char* pszFullMessage = pDoc->GetFullMessage();
			if (pszFullMessage)
			{
				char szDateMaybe[64];
				*szDateMaybe = 0;
				
				const char* pszMessage = pszFullMessage;
				if (!bIncludeHeaders)
					pszMessage = FindBody(pszFullMessage);
				else
				{
					//
					// Strip the taboo headers from received messages.
					//
					CReadMessageDoc* pReadMessageDoc = DYNAMIC_DOWNCAST(CReadMessageDoc, pDoc);
					if (pReadMessageDoc)
						 pReadMessageDoc->StripTabooHeaders(pszFullMessage);
					else
					{
						// build the Date: header
						ComposeDate( szDateMaybe, pSummary );
						if (*szDateMaybe)
							strcat( szDateMaybe, "\r\n" );
					}
				}

				if (bGuessParagraphs)
				{
					if (!bIncludeHeaders)
						::UnwrapText((char *) pszMessage);
					else
					{
						//
						// Don't unwrap the headers!
						//
						char* pszBody = (char *) FindBody(pszMessage);
						if (pszBody)
							::UnwrapText(pszBody);
					}
				}

				theFile.Put(szDateMaybe);
				theFile.PutLine(pszMessage);

				delete [] pszFullMessage;

				// If we saved this msg, then mark the message as read if it was unread
				if (pSummary->m_State == MS_UNREAD)
					pSummary->SetState(MS_READ);
			}

			if (bCreatedDoc)
				pSummary->NukeMessageDocIfUnused();
		}
		else
		{
			ASSERT(0);		// hmm.  couldn't load document?
		}
	}
}


void CTocView::OnEditCopy()
{
	CString SelText;

	if (GetSelText(SelText))
		CopyTextToClipboard( SelText, GetSafeHwnd() );
}


void CTocView::OnUpdateSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


void CTocView::OnSelectAll()
{
	//	User doing Edit->Select All is a direct user action so pass in true
	//	for the last parameter
	SelectAll(!ShiftDown(), TRUE, true);
}


void CTocView::OnUpdateNeedSel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_SumListBox.GetSelCount() > 0);
}


void CTocView::OnSize(UINT nType, int cx, int cy)
{
	C3DFormView::OnSize(nType, cx, cy);

	// Don't do any of this if the controls aren't created yet, or the window is being minimized
	if (m_SumListBox.m_hWnd == NULL || nType == SIZE_MINIMIZED)
		return;

	// Only change some items (header buttons and scroll range) if the width has changed
	if (cx != m_OldWidth)
	{
		SizeHeaders();
		
		int Min, Max;
		m_SumListBox.GetScrollRange(SB_HORZ, &Min, &Max);
		m_HScrollBar.SetScrollRange(Min, Max);
	}
		
	// Always resize/move the summary list box, compact button, and horizontal scroll bar
	CRect		rectWindow;
	CRect		rectControl;

	GetClientRect(&rectWindow);
	SizeBottomOfWindowControls(&rectControl);

	const int		nListBottom = rectControl.top;
	GetDlgItem((UINT)ID_EDIT_SORT_SORTBYSTATUS)->GetClientRect(&rectControl);
	rectControl.left = rectWindow.left;
	rectControl.right = rectWindow.right;
	rectControl.top = rectControl.bottom;
	rectControl.bottom = nListBottom;
	m_SumListBox.MoveWindow(&rectControl, TRUE);

	// Win95 makes the scrollbar visible after sizing the window, so hide it
	m_SumListBox.ShowScrollBar(SB_HORZ, FALSE);

	m_OldWidth = cx;
	m_OldHeight = cy;

	CTocFrame *		pTocFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );
	ASSERT_KINDOF(CTocFrame, pTocFrame);

	CMDIFrameWnd*	pMainFrame = (CMDIFrameWnd *) ::AfxGetMainWnd();
	ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);

	if (pTocFrame && pMainFrame)
	{
		// If this TOC is the active frame, then remember the splitter position
		// because it may have just changed.
		if ( pTocFrame == pMainFrame->MDIGetActive() )
			pTocFrame->RememberSplitterPos();
	}
}

static BOOL SizingColumn = FALSE;

void CTocView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!SizingColumn)
	{
		int i = HeaderSplitColumn(point);
		if (i >= 0)
		{
			QCSetCursor(AFX_IDC_HSPLITBAR);
            SizingColumn = TRUE;
            SizeColumn(i);
            SizingColumn = FALSE;
		}
	}
	
	Default();
}

BOOL CTocView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	
	GetCursorPos(&point);
	ScreenToClient(&point);

	if (HeaderSplitColumn(point) >= 0)
	{
		QCSetCursor(AFX_IDC_HSPLITBAR);
		return TRUE;
	}

	return C3DFormView::OnSetCursor(pWnd, nHitTest, message);
}

void CTocView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!SizingColumn && !(nFlags & MK_LBUTTON))
	{
		if (HeaderSplitColumn(point) >= 0)
		{
			QCSetCursor(AFX_IDC_HSPLITBAR);
			return;
		}
	}
	
	Default();
}

void CTocView::RedisplayHeaderSortChange(SortType Header, BOOL /*bAdding*/)
{
	if (Header != NOT_SORTED)
	{
		UINT HeaderID = ID_EDIT_SORT_SORTBYSTATUS +
			(Header >= BY_REVERSE_STATUS? Header - BY_REVERSE_STATUS : Header - 1);
		CWnd* HeaderCtrl = GetDlgItem(HeaderID);

		if (HeaderCtrl)
			HeaderCtrl->InvalidateRect(NULL);
	}
}

int CTocView::HeaderSplitColumn(const CPoint& point)
{
	CRect Crect;
	GetDlgItem((UINT)ID_EDIT_SORT_SORTBYSTATUS)->GetClientRect(&Crect);

	if (point.y <= Crect.bottom)
	{
		CTocDoc* TocDoc = GetDocument();

		if (TocDoc)
		{
			for (int i = 0; i < TocDoc->NumFields && m_FieldSep[i] != -1; i++)
			{       
				if (point.x > m_FieldSep[i] - 6 && point.x < m_FieldSep[i] + 3)
					return i;
			}
		}
	}

	return -1;
}

void CTocView::SizeHeaders()
{
	CRect rect, Crect;
	bool bSkipCol = false;
	
	GetClientRect(&rect);
	GetDlgItem((UINT)ID_EDIT_SORT_SORTBYSTATUS)->GetClientRect(&Crect);

	Crect.right = rect.right - GetSystemMetrics(SM_CXVSCROLL);
	SetFieldSeparators();
	for (int i = FW_UNUSED; i >= FW_STATUS; i--)
	{
		CWnd* control = GetDlgItem((UINT)(i + ID_EDIT_SORT_SORTBYSTATUS));
		if (!control) continue;

		bSkipCol = false;
		if (i == FW_MOOD)
		{
			// Skip the mood column if in light mode or if mood feature not used.
			if (!UsingFullFeatureSet() || !GetIniShort(IDS_INI_MOOD_MAIL_CHECK))
			{
				bSkipCol = true;
			}
		}
		else if (i == FW_JUNK)
		{
			// Skip the junk column if in light mode or if junk column is not being shown,
			// BUT always show the junk column in the junk mailbox.
			CTocDoc* pTocDoc = (CTocDoc*)GetDocument();
			ASSERT_KINDOF(CTocDoc, pTocDoc);
			if (!pTocDoc->IsJunk() &&
				(!UsingFullFeatureSet() ||
				 !GetIniShort(IDS_INI_MBOX_SHOW_JUNK)))
			{
				bSkipCol = true;
			}
		}
		else if (!GetIniShort(IDS_INI_MBOX_SHOW_STATUS + i))
		{
			bSkipCol = true;
		}
		if (bSkipCol)
			control->ShowWindow(SW_HIDE);
		else
		{
			Crect.bottom = Crect.top + ScreenFont.CellHeight() + 5;
			Crect.left = (i == FW_STATUS? 0 : m_FieldSep[i - 1] + 1);
			if (i < FW_UNUSED)
				Crect.right = min((int)m_FieldSep[i]+1, (int)Crect.right);
			int ButtonWidth = Crect.Width();
			const int Offset = m_SumListBox.GetHorizWindowOrg();
			if (i == FW_UNUSED)
				ButtonWidth += Offset;
			control->SetWindowPos(NULL, Crect.left - Offset, Crect.top, ButtonWidth, Crect.Height(),
									SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_SHOWWINDOW);
			control->Invalidate();
		}
	}
}


void CTocView::SizeBottomOfWindowControls(CRect * out_pRectControl)
{
	// Always resize/move the summary list box, compact button, and horizontal scroll bar
	int			nHorizScrollbarHeight = GetSystemMetrics(SM_CYHSCROLL);
	int			nVertScrollbarWidth = GetSystemMetrics(SM_CYVSCROLL);
	CRect		rectWindow;

	GetClientRect(&rectWindow);

	CRect		rectControl;
	m_Compact.GetWindowRect(&rectControl);
	ScreenToClient(&rectControl);
	rectControl.bottom = rectWindow.bottom;
	rectControl.top = rectControl.bottom - nHorizScrollbarHeight;
	m_Compact.MoveWindow(&rectControl, TRUE);

	CTocFrame *		pTocFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );
	ASSERT_KINDOF(CTocFrame, pTocFrame);

	if ( UsingFullFeatureSet() && pTocFrame && pTocFrame->ShouldPreview() )
	{
		m_ProfileCombo.ShowWindow(SW_SHOW);
		
		m_ProfileCombo.GetWindowRect(&rectControl);
		ScreenToClient(&rectControl);
		rectControl.bottom = rectWindow.bottom;
		rectControl.top = rectControl.bottom - nHorizScrollbarHeight;
		m_ProfileCombo.MoveWindow(&rectControl, TRUE);
	}
	else
	{
		m_ProfileCombo.ShowWindow(SW_HIDE);
	}

	rectControl.left = rectControl.right;
	rectControl.right = rectWindow.right - nVertScrollbarWidth;
	rectControl.bottom = rectWindow.bottom;
	rectControl.top = rectControl.bottom - nHorizScrollbarHeight;
	m_HScrollBar.MoveWindow(&rectControl, TRUE);

	if (out_pRectControl)
		*out_pRectControl = rectControl;
}


void CTocView::SizeColumn(int FieldNum)
{
	int SizingColumn = m_FieldSep[FieldNum];
	const int MinValue = FieldNum? m_FieldSep[FieldNum - 1] + ColumnSpace : ColumnSpace;
	CTocDoc* TocDoc = GetDocument();
	CDC* pDC = GetDC();
	CRect rect;

	GetClientRect(&rect);
	const int MaxValue = rect.right;

	SetCapture();

	int OldDrawMode = pDC->SetROP2(R2_NOTXORPEN);
	pDC->SelectStockObject(BLACK_PEN);
	pDC->MoveTo(SizingColumn, 0);
	pDC->LineTo(SizingColumn, 32767);

	UINT message = 0;
	while (1)
	{
		MSG msg;
		GetMessage(&msg, m_hWnd, 0, 0);
		message = msg.message;

		if (message == WM_LBUTTONUP || (message >= WM_KEYFIRST && message <= WM_KEYLAST &&
			msg.wParam == VK_ESCAPE))
		{
			break;
		}
		else if (message == WM_MOUSEMOVE)
		{
			int col = LOWORD(msg.lParam);
			col = max(col, MinValue);
			if (col != SizingColumn)
			{
				if (SizingColumn < MaxValue)
				{
				    pDC->MoveTo(SizingColumn, 0);
				    pDC->LineTo(SizingColumn, 32767);
				}
				if (col < MaxValue)
				{
				    pDC->MoveTo(col, 0);
				    pDC->LineTo(col, 32767);
				}
				SizingColumn = col;
			}
		}
	}

	if (SizingColumn < MaxValue)
	{
		pDC->MoveTo(SizingColumn, 0);
		pDC->LineTo(SizingColumn, 32767);
	}

	ReleaseCapture();

	if (message == WM_LBUTTONUP && SizingColumn < MaxValue)
	{
		short NewWidth = (short)((SizingColumn - (MinValue - ColumnSpace)) / ScreenFont.CharWidth());
			
		if (NewWidth != TocDoc->m_FieldWidth[FieldNum])
		{
			// If we're changing the width of the size or junk column, then invalidate that whole
			// column since the text is right-jusitifed
			int OldFieldSep = m_FieldSep[((FieldNum == FW_SIZE) || (FieldNum == FW_JUNK)) ? FieldNum - 1 : FieldNum];
		
			TocDoc->m_FieldWidth[FieldNum] = NewWidth;
			SizeHeaders();
			rect.left = min(OldFieldSep, (int)m_FieldSep[FieldNum]) - ColumnSpace;
			InvalidateRect(&rect, TRUE);
//			UpdateWindow();
			TocDoc->SetModifiedFlag();
		}
	}
    
	pDC->SetROP2(OldDrawMode);
	ReleaseDC(pDC);
}

void CTocView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!pHint && !lHint)
		CView::OnUpdate(pSender, lHint, pHint);
	else
	{
		// If this assert fails we're probably getting unnecessary repaints
		ASSERT(pSender != this);
		
		// lHint contains the field to update
		FieldWidthType field = (FieldWidthType)LOWORD(lHint);

		// If modifying a particular summary, pHint is a pointer to the summary.
		// Otherwise, redraw the specified field of all the summaries.
		RECT rect;
		if (pHint)
		{
			CSummary* Sum = (CSummary*)pHint;
			if (Sum->IsKindOf(RUNTIME_CLASS(CSummary)) == FALSE)
			{
				ASSERT(FALSE);
				return;
			}
			BOOL		 bHideDeleted = FALSE;
			CTocDoc		*pTocDoc = GetDocument();
			if (pTocDoc)
			{
				bHideDeleted = pTocDoc->HideDeletedIMAPMsgs();

				CSumList &		listSums = pTocDoc->GetSumList();

				m_SumListBox.GetItemRect(listSums.GetIndex(Sum, bHideDeleted), &rect);
			}
		}
		else
		{
			ASSERT(field != FW_LABEL);	// Probably not what was intended
			m_SumListBox.GetClientRect(&rect);
		}

		// If the label has changed, then the whole summary needs to be redrawn
		if (field != FW_LABEL)
		{
			if (field > FW_STATUS)
				rect.left = m_FieldSep[field - 1];
			if (field < FW_UNUSED)
				rect.right = m_FieldSep[field];
		}
		m_SumListBox.InvalidateRect(&rect, FALSE);
	}
}

void CTocView::OnSetFocus(CWnd* pOldWnd)
{
	CView::OnSetFocus(pOldWnd);
	
	m_SumListBox.SetFocus();
}

void CTocView::OnCompact()
{
	CTocDoc* toc = GetDocument();
	
	if (toc)
	{
		g_theMailboxDirector.CompactAMailbox( toc, true/*bUserRequested*/ );
	}
	m_SumListBox.SetFocus();
}

void CTocView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// Make sure everything in the window is repainted correctly
	GetParentFrame()->UpdateWindow();
	
	m_SumListBox.SendMessage(WM_HSCROLL, MAKEWPARAM(nSBCode, nPos));
		
	// This is kind of kludgy:
	//   GetScrollPos() will always return 0 if the scroll bar is not shown, so we need to show the
	//   listbox scroll bar, call GetScrollPos(), and then hide the scrollbar.  But this causes a
	//   flicker so we need to wrap it in some SetRedraw() calls.  But even that's not good enough
	//   because the window will still get invalidated when redraw is off, so validate the listbox
	//   client rect.  Whew!
	SetRedraw(FALSE);
	m_SumListBox.ShowScrollBar(SB_HORZ, TRUE);
	nPos = m_SumListBox.GetScrollPos(SB_HORZ);
	m_SumListBox.ShowScrollBar(SB_HORZ, FALSE);
	SetRedraw(TRUE);
	ValidateRect(NULL);
	if (nPos != (UINT)m_HScrollBar.GetScrollPos())
		m_HScrollBar.SetScrollPos(nPos);

	UpdateWindow();
	SizeHeaders();
}


////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
// Intercept create message in order to register this view as an OLE
// drop target.
////////////////////////////////////////////////////////////////////////
int CTocView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (C3DFormView::OnCreate(lpCreateStruct) == -1)           
		return -1;                                      

	// Register the COleDropTarget object with the OLE dll's
	if (! m_dropTarget.Register(this))
	{
		ASSERT(0);
		return -1;
	}

	return 0;
}

void CTocView::OnDestroy()
{
	// Hide window so we don't get repaint slowness while closing large mailboxes
	if (GetParentFrame())
	{
		GetParentFrame()->ShowWindow(SW_HIDE);
		AfxGetMainWnd()->UpdateWindow();
	}

	// write it to disk if need be
	CTocDoc* doc = GetDocument();
	if ( doc && doc->IsModified() )
	{
		doc->Write();
		doc->SetModifiedFlag( FALSE );
	}

	C3DFormView::OnDestroy();
}


// SortNow
//
// Does the actual sorting of the mailbox
//
// Returns TRUE if actual sorting was done, and FALSE if nothing
// was done.  We need this to decide whether or not to populate
// the listbox on creation.
//
BOOL CTocView::SortNow()
{
	CTocDoc* TocDoc = GetDocument();
	CSumList& SumList = TocDoc->GetSumList();
	const int Count = SumList.GetCount();
	CSummary* FirstSel;
	CCursor cursor;
	int i;

	TocDoc->m_bNeedsSorting = FALSE;
	TocDoc->m_bNeedsSortingNOW = FALSE;

	// Let's not waste time, shall we
	if (Count < 2)
		return FALSE;
	if (!TocDoc->m_bGroupBySubject && TocDoc->GetSortMethod(0) == NOT_SORTED)
		return FALSE;

	BOOL		 bHideDeletedMsgs = TocDoc->HideDeletedIMAPMsgs();

	// Don't let user do anything while sorting is going on
	AfxGetMainWnd()->EnableWindow(FALSE);

	// Set up STL vector for later sorting
	POSITION pos = SumList.GetHeadPosition();
	vector<CSummary*> vSums(Count);
	FirstSel = NULL;
	CSummary* Sum;
	for (i = 0; pos; i++)
	{
		Sum = SumList.GetNext(pos);
		vSums[i] = Sum;

		// This is not real obvious.
		// m_Temp2 is going to store two things:
		//   1) Whether the item was selected before the sort
		//      Stored in the low bit
		//   2) The starting position in order to do stable sorting
		//      Stored in the rest of the bits
		//
		// This allows us to save some space, which is crucial for
		// summaries because we make lots of 'em.
		Sum->m_Temp2 = i << 1;

		// Remember which items were selected so that they remain selected after resorting
		if (m_SumListBox.GetSel(i) > 0)
		{
			Sum->m_Temp2 |= 1;
			if (!FirstSel)
				FirstSel = m_SumListBox.GetItemDataPtr(i);
		}
	}
	if (!FirstSel)
		FirstSel = m_SumListBox.GetItemDataPtr(m_SumListBox.GetCaretIndex());
	
	// Sanity check: Make sure the actual number of summaries matches the official count.
	if (i != Count)
	{
		ASSERT(0);
		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			char buf[128];
			sprintf(buf, "Pre sort err: Count=%d, sums found=%d", Count, i);
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, buf);
		}
	}

	// Prepare for Group by Subject by getting unique subject IDs
	int HighestSubjectID = 0;
	if (TocDoc->m_bGroupBySubject)
	{
		// Sort by subject so that equivalent subjects are next to each other
		sort(vSums.begin(), vSums.end(), SubjectLessThan);

		vector<CSummary*>::const_iterator SumIter = vSums.begin() + 1;
		CSummary* LastSum = vSums[0];
		LastSum->m_Temp = 0;
		for (; SumIter != vSums.end(); SumIter++)
		{
			CSummary* CurSum = *SumIter;
			// If the next subject is different, bump up the unique subject ID
			if (CompareSubject(CurSum, LastSum) != 0)
				HighestSubjectID++;
			CurSum->m_Temp = HighestSubjectID;
			LastSum = CurSum;
		}
	}

	if (TocDoc->GetSortMethod(0) == NOT_SORTED)
	{
		// This means we're only doing a Group by Subject, so undo the sort by subject from above
		pos = SumList.GetHeadPosition();
		for (i = 0; pos; i++)
		{
			Sum = SumList.GetNext(pos);
			vSums[i] = Sum;
		}
	}
	else
	{
		// Do the sort
		g_SortTocDoc = TocDoc;
		sort(vSums.begin(), vSums.end(), SumLessThan);
		g_SortTocDoc = NULL;
	}

	// After the sort, group the messages with the same subject together
	if (TocDoc->m_bGroupBySubject)
	{
		vector<int> HighestIndex(HighestSubjectID + 1);
		fill(HighestIndex.begin(), HighestIndex.end(), -1);

		for (i = Count - 1; i >= 0; i--)
		{
			// This gets a little tricky.
			//
			// What this code does is keep track of the highest index of each of the unique subjects in
			// the vector (array) HighestIndex.  It starts at the end of the list of summaries, and works
			// its way to the beginning.  If it finds a particular Subject ID has not been seen yet (has a
			// value of -1), that means that this item has the highest index of that Subject ID, so that is
			// what gets put in the HighestIndex vector for that Subject ID.  Then, m_Temp (what holds the
			// Subject ID for that summary) can be overwritten with the highest index for that Subject ID,
			// since the Subject ID for that summary will never be needed again.
			const int SubjectID = vSums[i]->m_Temp;
			const int HighestIndexOfSubjectID = HighestIndex[SubjectID];
			if (HighestIndexOfSubjectID < 0)
				vSums[i]->m_Temp = HighestIndex[SubjectID] = i;
			else
				vSums[i]->m_Temp = HighestIndexOfSubjectID;

			// We've already used m_Temp2 for our first pass of sorting above. Now we're grouping
			// subjects and in cases of ties we want the new sort order to rule. Mask out
			// the old sort order and replace it with the new sort order.
			vSums[i]->m_Temp2 &= kTemp2SelectionMask;
			vSums[i]->m_Temp2 |= i << 1;
		}

		// Finally, sort by the highest subject index so that subjects get grouped together
		sort(vSums.begin(), vSums.end(), HighestIndexLessThan);
	}
	
	vector<CSummary*>::const_iterator SumIter;

	// Compare the sorted order against the current order.
	// If they are the same, then there's nothing to do.
	BOOL bFoundDifference = FALSE;
	pos = SumList.GetHeadPosition();
	for (SumIter = vSums.begin(); SumIter != vSums.end(); SumIter++)
	{
		Sum = *SumIter;
		CSummary* ExistingSum = SumList.GetNext(pos);

		if (!Sum || !ExistingSum || Sum != ExistingSum)
		{
			bFoundDifference = TRUE;
			break;
		}
	}

	if (bFoundDifference)
	{
		// Turn off redraw before doing major listbox item manipulation
		m_SumListBox.SetRedraw(FALSE);
		
		// Add the items back to the listbox in their newly sorted order.
		// Select an item if it was previously selected.
		BOOL			 bBadPos = FALSE;

		//	Make sure that HandleSelectionMayHaveChanged is not called while
		//	we're rebuilding the list (avoids calls during ResetContent
		//	and SetSel). Otherwise HandleSelectionMayHaveChanged is called
		//	a lot, which results in *really* slow performance.
		m_SumListBox.SetAllowSelChangeNotifications(false);

		m_SumListBox.ResetContent();

		// Changed the sorting logic here from RemoveAll()/AddTail() to
		// SetAt() to help deal with any potential summary loss problems.
		// This technique circumvents memory problems related to deleting
		// and recreating the summary list.  Also, if the summary count
		// is off it will remain off so Eudora can catch and correct it later.
		pos = SumList.GetHeadPosition();
		for (SumIter = vSums.begin(); SumIter != vSums.end(); SumIter++)
		{
			Sum = *SumIter;
			if (pos)
			{
				SumList.SetAt(pos, Sum);
				SumList.GetNext(pos);
			}
			else
			{
				// Definitely should NOT happen.  If it does, log the error
				// and add the summary to the end so we don't lose it altogether.
				if (!bBadPos)
				{
					PutDebugLog(DEBUG_MASK_TOC_CORRUPT, "Mid sort err: NULL list pos");
				}
				SumList.AddTail(Sum);
				bBadPos = TRUE;
			}
			if (Sum && !(bHideDeletedMsgs && Sum->IsIMAPDeleted()))
			{
				int Index = m_SumListBox.AddString((LPCSTR)Sum);
				if ((Index >= 0) && (Sum->m_Temp2 & kTemp2SelectionMask))
					m_SumListBox.SetSel(Index);
			}
		}
			
		// Sanity check: Make sure we ended up with the same number of summaries we started with.
		if (Count != SumList.GetCount())
		{
			ASSERT(0);
			if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
			{
				char buf[128];
				sprintf(buf, "Post sort err: orig GetCount=%d, new GetCount=%d", Count, SumList.GetCount());
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, buf);
			}
		}

		//	Allow HandleSelectionMayHaveChanged to be called in response to list
		//	changes again.
		m_SumListBox.SetAllowSelChangeNotifications(true);

		//	Above list rebuilding should have perfectly restored the previous
		//	selection, but let's check just to be sure.
		m_SumListBox.HandleSelectionMayHaveChanged(false);

		// Restore the redraw
		m_SumListBox.SetRedraw(TRUE);

		// Notify the Search Manager that the summaries have been re-ordered so it may
		// have to restart iterating through them.
		CTocDoc *	pTocDoc = GetDocument();
		if (pTocDoc)
			SearchManager::Instance()->NotifyRestartProcessingMailbox( pTocDoc->GetMBFileName() );
	}
			
	// Ok, I suppose we'll be a nice little program and let them have control again
	AfxGetMainWnd()->EnableWindow(TRUE);

	// We need to return whether or not we filled the listbox.
	// Some callers, like OnInitialUpdate(), need to know.
	return bFoundDifference;
}

void CTocView::OnUpdateSort(CCmdUI* pCmdUI)
{
	CTocDoc* TocDoc = GetDocument();
	BOOL bEnable = FALSE;
	SortType NormalSort = ((SortType)(BY_STATUS + (pCmdUI->m_nID - ID_EDIT_SORT_SORTBYSTATUS)));
	SortType ReverseSort = ((SortType)(NormalSort + (BY_REVERSE_STATUS - BY_STATUS)));

	if (TocDoc)
	{
		for (int i = 0; i < TocDoc->NumSortColumns; i++)
		{
			const SortType st = GetSortMethod(i);
			if (st == NOT_SORTED)
				break;
			if (st == NormalSort || st == ReverseSort)
			{
				bEnable = TRUE;
				break;
			}
		}
	}

	pCmdUI->SetCheck(bEnable);
}

BOOL CTocView::Sort(UINT nButtonID, UINT nSortDirectionID)
{
	CTocDoc* TocDoc = GetDocument();
	const BOOL bDoReverseSort = (!nSortDirectionID && ShiftDown());
	const BOOL bAddSort = CtrlDown();
	SortType OldSort = NOT_SORTED;
	SortType NormalSort, ReverseSort;
	BOOL bRedoSort = FALSE;
	CCursor cursor;
	int SortChangeIndex;
	int i;

	NormalSort = ((SortType)(BY_STATUS + (nButtonID - ID_EDIT_SORT_SORTBYSTATUS)));
	ReverseSort = ((SortType)(NormalSort + (BY_REVERSE_STATUS - BY_STATUS)));

	if (!bAddSort)
	{
		const SortType FirstSort = GetSortMethod(0);
		if (GetSortMethod(1) == NOT_SORTED)
		{
			if ((FirstSort == NOT_SORTED && nSortDirectionID == ID_MAILBOX_SORT_NONE) ||
				(FirstSort == NormalSort && nSortDirectionID == ID_MAILBOX_SORT_ASCEND) ||
				(FirstSort == ReverseSort && nSortDirectionID == ID_MAILBOX_SORT_DESCEND))
			{
				// No change in sort, just bail
				return TRUE;
			}
		}

		// Need to remove existing sorts
		for (i = 0; i < TocDoc->NumSortColumns; i++)
		{
			const SortType st = GetSortMethod(i);
			if (st == NOT_SORTED)
				break;
			if (st == NormalSort || st == ReverseSort)
				OldSort = st;
			else if (OldSort != NOT_SORTED)
			{
				// If there were secondary sort methods to the one we're changing
				// then we'll need to resort, and the case below which checks for
				// secondary sorts when undoing a sort won't catch this because
				// there won't be any secondary sorts once we get out of this loop.
				bRedoSort = TRUE;
			}
			SetSortMethod(i, NOT_SORTED);
			RedisplayHeaderSortChange(st, FALSE);
		}
		SortChangeIndex = 0;
	}
	else
	{
		// Adding this sort to existing sorts
		for (i = 0; i < TocDoc->NumSortColumns; i++)
		{
			const SortType st = GetSortMethod(i);
			if (st == NOT_SORTED || st == NormalSort || st == ReverseSort)
			{
				if ((st == NOT_SORTED && nSortDirectionID == ID_MAILBOX_SORT_NONE) ||
					(st == NormalSort && nSortDirectionID == ID_MAILBOX_SORT_ASCEND) ||
					(st == ReverseSort && nSortDirectionID == ID_MAILBOX_SORT_DESCEND))
				{
					// No change in sort, just bail
					return TRUE;
				}
				OldSort = st;
				break;
			}
		}
		SortChangeIndex = i;

		// If going from one sort method to two, then we need to redisplay the
		// first method because it won't have a number representing the sort level
		if (SortChangeIndex == 1)
			RedisplayHeaderSortChange(GetSortMethod(0), FALSE);
	}

	// Clicking (without Shift held down) on a column that's already sorted will undo the sort
	// Clicking (with Shift held down) on a column that's reverse sorted will also undo the sort
	BOOL bUndoSort = FALSE;
	if (nSortDirectionID)
	{
		if (nSortDirectionID == ID_MAILBOX_SORT_NONE)
			bUndoSort = TRUE;
	}
	else if ((!bDoReverseSort && OldSort != NOT_SORTED) || OldSort == ReverseSort)
		bUndoSort = TRUE;

	if (bUndoSort)
	{
		SetSortMethod(SortChangeIndex, NOT_SORTED);
		RedisplayHeaderSortChange(OldSort, FALSE);
		g_SortMethod = NOT_SORTED;

		// Columns sorted secondarily to the one we're undoing need to be moved and redrawn
		for (i = SortChangeIndex + 1; i < TocDoc->NumSortColumns; i++)
		{
			const SortType st = GetSortMethod(i);
			if (st == NOT_SORTED)
				break;

			// If we're turning off a sort, and there were sort methods
			// secondary to the sort, then we need to resort.
			bRedoSort = TRUE;

			SetSortMethod(i - 1, st);
			SetSortMethod(i, NOT_SORTED);
			RedisplayHeaderSortChange(st, TRUE);
		}
	}
	else
	{
		if (nSortDirectionID)
			g_SortMethod = (nSortDirectionID == ID_MAILBOX_SORT_ASCEND)? NormalSort : ReverseSort;
		else
		{
			// Shouldn't currently be a reverse sort, otherwise it should have been caught above
			ASSERT(OldSort != ReverseSort);
			if (OldSort == NOT_SORTED)
				g_SortMethod = bDoReverseSort? ReverseSort : NormalSort;
			else
				g_SortMethod = ReverseSort;

			ASSERT(g_SortMethod != NOT_SORTED && g_SortMethod != OldSort);
		}

		SetSortMethod(SortChangeIndex, g_SortMethod);
		RedisplayHeaderSortChange(g_SortMethod, TRUE);
		bRedoSort = TRUE;
	}
	
	if (bRedoSort)
		SortNow();

	// Remember whether the last search method was by subject so that we can
	// do type-to-select by subject when appropriate.
	if (SortChangeIndex == 0)
		m_bLastSortBySubject = (NormalSort == BY_SUBJECT);
	
	// Turn sorting off so that later additions go to the end
	g_SortMethod = NOT_SORTED;
    
	m_SumListBox.SetFocus();
	TocDoc->SetModifiedFlag();	
	
	return (TRUE);
}

BOOL CTocView::Sort(UINT nID)
{
	return Sort(nID, 0);
}

void CTocView::OnGroupBySubject()
{
	CTocDoc* TocDoc = GetDocument();

	TocDoc->m_bGroupBySubject = !TocDoc->m_bGroupBySubject;
	TocDoc->SetModifiedFlag();

	if (TocDoc->m_bGroupBySubject || GetSortMethod(0) != NOT_SORTED)
		SortNow();

	RedisplayHeaderSortChange(BY_SUBJECT, FALSE);
}

void CTocView::OnUpdateGroupBySubject(CCmdUI* pCmdUI)
{
	CTocDoc* toc = GetDocument();

	pCmdUI->SetCheck(toc->m_bGroupBySubject);
}

BOOL CTocView::OnSortPopup(UINT nID)
{
	return Sort(CTocHeaderButton::GetSortPopupID(), nID);
}

void CTocView::OnUpdateSortPopup(CCmdUI* pCmdUI)
{
	UINT nHeaderID = CTocHeaderButton::GetSortPopupID();

	SortType NormalSort = ((SortType)(BY_STATUS + (nHeaderID - ID_EDIT_SORT_SORTBYSTATUS)));
	SortType ReverseSort = ((SortType)(NormalSort + (BY_REVERSE_STATUS - BY_STATUS)));

	SortType st = NOT_SORTED;
	int i;
	for (i = 0; i < CTocDoc::NumSortColumns; i++)
	{
		st = GetSortMethod(i);
		if (st == NOT_SORTED || st == NormalSort || st == ReverseSort)
			break;
	}

	BOOL bMatchesCurrentSort = FALSE;
	if (st == NOT_SORTED || i == CTocDoc::NumSortColumns)
		bMatchesCurrentSort = (pCmdUI->m_nID == ID_MAILBOX_SORT_NONE);
	else if (st == NormalSort)
		bMatchesCurrentSort = (pCmdUI->m_nID == ID_MAILBOX_SORT_ASCEND);
	else
	{
		ASSERT(st == ReverseSort);
		bMatchesCurrentSort = (pCmdUI->m_nID == ID_MAILBOX_SORT_DESCEND);
	}
	
	pCmdUI->SetRadio(bMatchesCurrentSort);

	// Would be proper to disable a menu item that does nothing, but it doesn't
	// look that great to have a menu item with a radio button that's disabled
	pCmdUI->Enable(TRUE /*!bMatchesCurrentSort*/);
}

//
//	CTocView::MaybeResetContents()
//
//	Reset the contents only if deleted messages are being hidden.  This allows us to do nothing in the
//	case where the caller deleted a message but deleted messages are not being hidden.
//
void CTocView::MaybeResetContents()
{
	CTocDoc		*pTocDoc = GetDocument();
	if (!pTocDoc)
	{
		ASSERT(0);
		return;
	}

	if (pTocDoc->HideDeletedIMAPMsgs())
	{
		ResetContents();
	}
}

//
//	CTocView::ResetContents()
//
//	Clear out the contents of the view and fill them in again.  This function is currently only used
//	when switching between showing and hiding of deleted IMAP messages.
//
void CTocView::ResetContents()
{
	CTocDoc		*pTocDoc = GetDocument();
	if (!pTocDoc)
	{
		ASSERT(0);
		return;
	}

	BOOL		 bHideDeletedIMAPMsgs = pTocDoc->HideDeletedIMAPMsgs();

	// Get the currently selected items.
	CDWordArray		cdwaSel;
	int				iTopIdx;
	m_SumListBox.GetSelIndices(&cdwaSel, &iTopIdx, bHideDeletedIMAPMsgs);

	// Clear and re-fill the contents.
	SetRedraw(FALSE);

	m_SumListBox.SetSel(-1, FALSE/*bSelect*/);

	//	Make sure that HandleSelectionMayHaveChanged is not called while
	//	we're rebuilding the list (avoids calls during ResetContent
	//	and SetSel). Otherwise HandleSelectionMayHaveChanged is called
	//	a lot, which results in *really* slow performance.
	m_SumListBox.SetAllowSelChangeNotifications(false);

	m_SumListBox.ResetContent();
	FillInContents();

	// Restore the selection.
	m_SumListBox.SetSelFromIndices(&cdwaSel, iTopIdx, bHideDeletedIMAPMsgs);

	//	Allow HandleSelectionMayHaveChanged to be called in response to list
	//	changes again.
	m_SumListBox.SetAllowSelChangeNotifications(true);

	//	Above list rebuilding should have perfectly restored the previous
	//	selection, but let's check just to be sure.
	m_SumListBox.HandleSelectionMayHaveChanged(false);

	// Redraw and update.
	SetRedraw(TRUE);
	m_SumListBox.Invalidate();
	m_SumListBox.UpdateWindow();

	SetCompact();
}

//
//	CTocView::FillInContents()
//
//	Fill in the contents of the view.  Assumes the contents have never been filled in or have been cleared.
//  This function takes into account whether or not deleted IMAP messages are being hidden.
//
void CTocView::FillInContents()
{
	CTocDoc		*pTocDoc = (CTocDoc*)GetDocument();
	if (pTocDoc)
	{
		CSumList &	 listSums = pTocDoc->GetSumList();
		
		BOOL		 bHideDeletedIMAPMsgs = pTocDoc->HideDeletedIMAPMsgs();

		// Preallocate storage for faster operation of AddString (which
		// isn't really such an accurate name, since we're really adding
		// pointers to our summaries)
		m_SumListBox.InitStorage(listSums.GetCount(bHideDeletedIMAPMsgs), sizeof(LPCSTR));
		
		// Add the summaries to the list box
		POSITION	 pos = listSums.GetHeadPosition();
		CSummary	*pSum = NULL;
		while (pos)
		{
			pSum = listSums.GetAt(pos);
			if (!bHideDeletedIMAPMsgs || !(pSum->IsIMAPDeleted()))
			{
				m_SumListBox.AddString( reinterpret_cast<LPCSTR>(listSums.GetNext(pos)) );
			}
			else
			{
				listSums.GetNext(pos);
			}
		}
	}
}

void CTocView::OnProfileComboBoxItemSelected()
{
	CTocDoc *				pTocDoc = GetDocument();
	ASSERT(pTocDoc);

	int						nSel = m_ProfileCombo.GetCurSel();

	if ( pTocDoc && (nSel > 0) )
	{
		//	Get the profile that the user selected
		BOOL		bProfileGood = TRUE;
		char		szProfile[128];

		//	Init the profile to be the default - nothing
		szProfile[0] = '\0';
		
		if (nSel > 1)
		{
			//	If the selection is greater than 1, then the user selected something
			//	other than the default, so get the text of the selected item.
			COMBOBOXEXITEM		itemInfo;

			memset( &itemInfo, 0, sizeof(itemInfo) );
			itemInfo.mask = CBEIF_TEXT;

			itemInfo.pszText = szProfile;
			itemInfo.cchTextMax = sizeof(szProfile);
			itemInfo.iItem = nSel;
			
			bProfileGood = m_ProfileCombo.GetItem(&itemInfo);
		}

		if (bProfileGood)
		{
			//	Get the previously selected item for the type of selection
			const CString &		szOldProfile = m_bProfileComboBoxIsForMultipleSel ?
											   pTocDoc->GetMultipleSelectionProfileName() :
											   pTocDoc->GetSingleSelectionProfileName();
			
			//	See if the selected profile is actually different
			if (szOldProfile.CompareNoCase(szProfile) != 0)
			{
				//	Remember the newly selected profile
				if (m_bProfileComboBoxIsForMultipleSel)
					pTocDoc->SetMultipleSelectionProfileName(szProfile);
				else
					pTocDoc->SetSingleSelectionProfileName(szProfile);

				//	Update the display using the newly selected profile
				CTocFrame *		pTocFrame = reinterpret_cast<CTocFrame *>( GetParentFrame() );
				ASSERT_KINDOF(CTocFrame, pTocFrame);

				if (pTocFrame)
					pTocFrame->InvalidateCachedPreviewSummary(NULL, true);
			}
		}
	}
}


void CCompactButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	int Action = lpDrawItemStruct->itemAction;

	if (Action == ODA_SELECT || Action == ODA_DRAWENTIRE)
	{
		const BOOL bDisabled = lpDrawItemStruct->itemState & ODS_DISABLED;
		const BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
		CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
		CRect rect(lpDrawItemStruct->rcItem);
		CString ButtonText;

		pDC->DrawFrameControl(&rect, DFC_BUTTON, DFCS_BUTTONPUSH | (bSelected? DFCS_FLAT : 0));

		GetWindowText(ButtonText);
		COLORREF OldColor = pDC->SetTextColor(GetSysColor(bDisabled? COLOR_GRAYTEXT : COLOR_BTNTEXT));
		int OldBkMode = pDC->SetBkMode(TRANSPARENT);

		if (bSelected)
			rect.OffsetRect(1, 1);
		pDC->DrawText(ButtonText, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

		pDC->SetBkMode(OldBkMode);
		pDC->SetTextColor(OldColor);
	}
}

//void PASCAL _AfxSmartSetWindowText(HWND hWndCtrl, LPCSTR lpszNew);

// SetCompact
// Update the values in the compact button, and enable/disable
//
void CTocView::SetCompact()
{
	CTocDoc* toc = GetDocument();
	char buf[32];

	CSumList &	listSums = toc->GetSumList();

	wsprintf(buf,
			 CRString(IDS_TOC_K_FORMAT),
			 listSums.GetCount( toc->HideDeletedIMAPMsgs() ),
			 toc->m_MesSpace / 1024,
			 toc->m_DelSpace / 1024);

	if (GetIniShort(IDS_INI_SHOW_SEL_SUMMARY_COUNT))
	{
		char NewText[64];

		sprintf(NewText, "%d/%s", GetSelCount(), buf);
		strcpy(buf, NewText);
	}

	AfxSetWindowText(m_Compact.GetSafeHwnd(), buf);

	if (toc->IsJunk())
	{
		// In the Junk TOC enable the compact button if the TOC is not empty.
		// This allows the compact button to delete old junk even if there
		// is no waste.
		m_Compact.EnableWindow(toc->m_TotalSpace > 0);
	}
	else
	{
		// Enable compact button if there is any waste space.
		m_Compact.EnableWindow(toc->m_DelSpace != 0);
	}
}


extern int MessageCascadeSpot = -1;


//	We used to do Fumlub immediately when receiving new mail (either
//	after filtering into the appropriate mailbox or for the In mailbox
//	if filters didn't moved the message anywhere else). The problem was
//	that we would later sort on idle in CTocDoc::SortNowIfNeeded.
//	That caused two problems:
//	* The first unread message of the last unread block could change
//    (thus making our message choice incorrect)
//	* Since the messages moved around when sorting - the selected message
//	  could end up no longer scrolled correctly into view. In testing this
//	  sometimes manifested as the message listing appearing at the bottom
//	  of the scroll view only partly scrolled into view (with just a few
//	  pixels of it scrolled into view as reported by Rob Chandhok)
//
//	Our new tactic is that places that used to call us (Fumlub) directly
//	now instead call CTocDoc::SetNeedsFumlub. CTocDoc::SortNowIfNeeded
//	calls us if appropriate after sorting the mailbox if necessary.
//	Thus the correct message is selected and it's correctly scrolled
//	into view.
void CTocView::Fumlub()
{
	if (!GetIniShort(IDS_INI_FUMLUB))
		return;

	CTocDoc* doc = GetDocument();
	POSITION PrevPos, pos;
	
	if (!doc) return;

	CSumList &		listSums = doc->GetSumList();
	
	int i;
	BOOL	 bHideDeletedIMAP = doc->HideDeletedIMAPMsgs();
	
	if ( BOOL bDateDescending = doc->SortedDateDescending() )
	{
		// When sorted ascending by date, do lumfub
		if ( !(pos = listSums.GetHeadPosition()) )
			return;
			
		// Iterate through the summary list starting at the top until we find one that has a status
		// of anything other than unread.  If we are hiding deleted IMAP messages then we must skip
		// over any deleted IMAP message regardless of its status.
		CSummary* Sum = NULL;
		i = -1;
		PrevPos = NULL;
		do
		{
			Sum = listSums.GetAt(pos);
			if (bHideDeletedIMAP && Sum && Sum->IsIMAPDeleted())
			{
				// We are hiding deleted IMAP messages and this one is deleted.  Decrement i to cancel
				// out the i++ that is about to happen.
				--i;
			}
			else if (Sum && Sum->m_State != MS_UNREAD)
				break;
			PrevPos = pos;
			i++;
		}
		while (listSums.GetNext(pos) && pos);

		// For sanity's sake, make sure we have a valid index to select.
		if (i < 0)
		{
			i = 0;
		}
	}
	else
	{
		// Normal users get fumlub
		if ( !(pos = listSums.GetTailPosition()) )
			return;

		CSummary	*Sum = NULL;
		int			 iCount = listSums.GetCount(bHideDeletedIMAP);
		i = iCount;

		// Iterate through the summary list starting at the bottom until we find one that has a status
		// of anything other than unread.  If we are hiding deleted IMAP messages then we must skip
		// over any deleted IMAP message regardless of its status.
		for (PrevPos = pos; PrevPos; pos = PrevPos, i--)
		{
			Sum = listSums.GetAt(PrevPos);
			if (bHideDeletedIMAP && Sum && Sum->IsIMAPDeleted())
			{
				// We are hiding deleted IMAP messages and this one is deleted.  Increment i to cancel
				// out the i-- that is about to happen.
				++i;
			}
			else if (Sum && Sum->m_State != MS_UNREAD)
				break;
			listSums.GetPrev(PrevPos);
		}
		// If the first summary encountered was a deleted IMAP message which is to be hidden,
		// i will greater than the actual number of summaries so decrement it.
		if ((i > 0) && (i == iCount))
		{
			--i;
		}
	}
	
	if (!PrevPos)
	{
		pos = listSums.GetHeadPosition();
		i = 0;
	}
	
	if (pos)
	{
		SelectAll(FALSE, FALSE);		// turns redraw off 
		m_SumListBox.SetSel(i, TRUE);

		//
		// As you can see, we used to scroll the TOC list until the
		// very last message was visible, while the selection
		// highlight could be farther up the list.  But since we
		// introduced the auto-mark-as-read behavior in 4.0, it
		// actually makes more sense to scroll the display to the
		// selection, rather than the last message.  That way, you can
		// see which message is about to be auto-marked-as-read.
		//
//FORNOW		int Index = m_SumListBox.GetCount() - 1;
//FORNOW		m_SumListBox.SetTopIndex(Index);
		/////
		// If SetTopIndex ever starts doing what the dox claim
		// it does, uncomment the following lines and write
		// SetBottomIndex so that we put the lumfub at the bottom
		// of the list.  SetTopIndex seems usually to do so now,
		// so I'm leaving it.
		//if ( bDateAscending )
		//	m_SumListBox.SetBottomIndex(i);
		//else
		/////
			m_SumListBox.SetTopIndex(i);
		m_SumListBox.SetRedraw(TRUE);
	}
}

int CTocView::TopVisibleItem()
{
	CRect rect;
	int height = 0, vis = 0, top = 0;
	
	height = m_SumListBox.GetItemHeight(1);
	m_SumListBox.GetClientRect(&rect);
	vis = (rect.bottom - rect.top)/height - 1;
	top = m_SumListBox.GetCount() - vis - 1;

	return ( top >= 0 ? top : 0);
}


void CTocView::UpdateProfileComboBox(int in_nNumItemsSelected)
{
	//	Remove any previously added items
	m_ProfileCombo.ResetContent();

	if (in_nNumItemsSelected == 0)
	{
		//	No items selected - no need for profile combo box
		m_ProfileCombo.EnableWindow(FALSE);
	}
	else
	{
		//	One or more items selected - enable profile combo box
		m_ProfileCombo.EnableWindow();

		//	Update whether or not we're handling multiple or single selection
		//	(Keeping this as a separate variable may not be absolutely necessary,
		//	but I wanted to make sure that there couldn't be any conditions where
		//	the user thought he or she was changing one value, but accidently
		//	changed the other).
		m_bProfileComboBoxIsForMultipleSel = (in_nNumItemsSelected > 1);

		CString							szCurrentProfileName;
		ContentConcentrator::ContextT	ccContext = m_bProfileComboBoxIsForMultipleSel ?
													ContentConcentrator::kCCMultipleContext :
													ContentConcentrator::kCCPreviewContext;

		ContentConcentrator::Instance()->GetProfileIniString(ccContext, szCurrentProfileName);

		CTocDoc *				pTocDoc = GetDocument();
		ASSERT(pTocDoc);

		CString					szOverrideProfile;
		int						nSelectedItemID = 1;

		if (pTocDoc)
		{
			if (m_bProfileComboBoxIsForMultipleSel)
				szOverrideProfile = pTocDoc->GetMultipleSelectionProfileName();
			else
				szOverrideProfile = pTocDoc->GetSingleSelectionProfileName();
		}

		int				nSelectionTypeResID = m_bProfileComboBoxIsForMultipleSel ?
											  IDS_CON_CON_MULTIPLE_MESSAGES_PROF :
											  IDS_CON_CON_SINGLE_MESSAGE_PROF;
		CRString		szSelectionType(nSelectionTypeResID);

		m_ProfileCombo.InsertItem(szSelectionType, 0, szSelectionType.GetLength(), true);
		
		// Add the special "Default" entry second
		CString			szDefaultProfileEntry;

		szDefaultProfileEntry.Format(IDS_CON_CON_DEF_PROF_FORMAT, szCurrentProfileName);
		m_ProfileCombo.InsertItem( szDefaultProfileEntry, 1, szDefaultProfileEntry.GetLength() );

		// Add the special "None" entry third
		CRString		szNoneProfileEntry(IDS_CON_CON_PROFILE_NONE);
		m_ProfileCombo.InsertItem( szNoneProfileEntry, 2, szNoneProfileEntry.GetLength() );
		if (szOverrideProfile.CompareNoCase(szNoneProfileEntry) == 0)
			nSelectedItemID = 2;

		// Get the profile names from the Content Concentrator
		CStringList		stringList;
		ContentConcentrator::Instance()->GetProfileNames(stringList);

		if (stringList.GetCount() > 0)
		{
			// Add the profile names
			int				nItemID = 3;
			POSITION		pos = stringList.GetHeadPosition();
			while (pos)
			{
				CString &		szProfileName = stringList.GetNext(pos);
				
				m_ProfileCombo.InsertItem(szProfileName, nItemID);

				if ( (nSelectedItemID == 1) && (szOverrideProfile.CompareNoCase(szProfileName) == 0) )
					nSelectedItemID = nItemID;

				nItemID++;
			}
		}

		m_ProfileCombo.SetCurSel(nSelectedItemID);
	}
}


// OpenMessages
// Opens the messages that are currently selected
//
void CTocView::OpenMessages()
{
	CCursor cur;
	
	// If shift is held down, reverse the Show All Headers switch
	BOOL ReverseSAH = FALSE;
	if (ShiftDown())
	{   
		ReverseSAH = TRUE;
	}
	
	int Selected = m_SumListBox.GetSelCount();
	
	MessageCascadeSpot = 0;
	::AsyncEscapePressed(TRUE);
	for (int i = m_SumListBox.GetCount() - 1; Selected && i >= 0; i--)
	{
		if (::AsyncEscapePressed())
			break;
		if (m_SumListBox.GetSel(i) > 0)
		{
			m_SumListBox.GetItemDataPtr(i)->Display();
			Selected--;
		}
	}
	MessageCascadeSpot = -1;
}

// SetFieldSeparators
// Set the positions of the field separators
//
void CTocView::SetFieldSeparators()
{
	CTocDoc* TocDoc = GetDocument();
	int CharWidth = ScreenFont.CharWidth();
	char StatusChar[2] = {0, 0};
	short LastSep = 0;
	bool bSkipCol = false;

	g_MaxStatusWidth = 0;
	for (int j = MS_UNREAD; j <= MS_UNSENT; j++)
	{
		StatusChar[0] = StateFlag[j];
		int width = ScreenFont.TextWidth(StatusChar);
		if (width > g_MaxStatusWidth)
			g_MaxStatusWidth = width; 
	}

	for (int i = 0; i < CTocDoc::NumFields; i++)
	{
/*
		if ( i == FW_LABEL || i == FW_SERVER)
		{
			m_FieldSep[i] = LastSep;
			continue;
		}
*/
		bSkipCol = false;
		if (i == FW_MOOD)
		{
			// Skip the mood column if in light mode or if mood feature not used.
			if (!UsingFullFeatureSet() ||
				!GetIniShort(IDS_INI_MOOD_MAIL_CHECK) ||
				!GetIniShort(IDS_INI_MBOX_SHOW_MOOD))
			{
				bSkipCol = true;
			}
		}
		else if (i == FW_JUNK)
		{
			// Skip the junk column if in light mode or if junk column is not being shown,
			// BUT always show the junk column in the junk mailbox.
			CTocDoc* pTocDoc = (CTocDoc*)GetDocument();
			ASSERT_KINDOF(CTocDoc, pTocDoc);
			if (!pTocDoc->IsJunk() &&
				(!UsingFullFeatureSet() ||
				 !GetIniShort(IDS_INI_MBOX_SHOW_JUNK)))
			{
				bSkipCol = true;
			}
		}
		else if (!GetIniShort(IDS_INI_MBOX_SHOW_STATUS + i))
		{
			bSkipCol = true;
		}
		if (bSkipCol)
		{
			m_FieldSep[i] = LastSep;
		}
		else
		{
			if (TocDoc->m_FieldWidth[i] == 0)
				m_FieldSep[i] = (short)(LastSep + ColumnSpace);
			else
			{
				m_FieldSep[i] = (short)((TocDoc->m_FieldWidth[i] < 0?
					CTocDoc::DefaultFieldWidth[i] : TocDoc->m_FieldWidth[i]) * CharWidth);
				if (TocDoc->m_FieldWidth[i] != 0)
				{
					switch (i)
					{
					case FW_STATUS:		m_FieldSep[i] = (short)max(m_FieldSep[i], (short)(g_MaxStatusWidth + 2)); break;
	        		case FW_PRIORITY:	m_FieldSep[i] = (short)max(m_FieldSep[i], (short)TocBitmapWidth);	break;
	        		case FW_ATTACHMENT:	m_FieldSep[i] = (short)max(m_FieldSep[i], (short)TocBitmapWidth);	break;
	        		case FW_SERVER:     m_FieldSep[i] = (short)max(m_FieldSep[i], (short)TocBitmapWidth);	break;
	        		case FW_MOOD:		m_FieldSep[i] = (short)max(m_FieldSep[i], (short)TocBitmapWidth);	break;
	        		}
				}
				m_FieldSep[i] = (short)(m_FieldSep[i] + LastSep + 2 * ColumnSpace - 1);
			}
		}
		LastSep = m_FieldSep[i];
	}
}


////////////////////////////////////////////////////////////////////////
// OnDrop [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CTocView::OnDrop(COleDataObject* pDataObject,
					  DROPEFFECT dropEffect, 
					  CPoint point)
{
	//TRACE0("CTocView::OnDrop()\n");
	ASSERT_VALID(this);

	//
	// Get pointer to global data (which stores the instance handle
	// for the drop source) and make sure that we're dragging and
	// dropping in the same copy of Eudora.
	//
	QCFTRANSFER qcfTransfer;
	if (! g_theClipboardMgr.GetGlobalData(pDataObject, qcfTransfer))
	{
		ASSERT(0);
		return FALSE;
	}
	else if (qcfTransfer.m_hInstance != ::AfxGetInstanceHandle())
	{
		ASSERT(0);
		return FALSE;
	}

	ASSERT(qcfTransfer.m_pSourceTocDoc);
	ASSERT_KINDOF(CTocDoc, qcfTransfer.m_pSourceTocDoc);

	// Get main frame window 
	CMainFrame* p_mainframe = (CMainFrame *) AfxGetMainWnd();
	ASSERT(p_mainframe && p_mainframe->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	
	// get the document
	CTocDoc* p_tocdoc = GetDocument();
	
	ASSERT(p_tocdoc != NULL);

	// get the mailbox command object
	QCMailboxCommand* pFolder = g_theMailboxDirector.FindByPathname( (const char *) p_tocdoc->GetMBFileName() );

	if( pFolder != NULL )
	{
		//
		// Determine what type of command to perform.
		//
		COMMAND_ACTION_TYPE cmdType = CA_NONE;
		if (DROPEFFECT_COPY == (dropEffect & DROPEFFECT_COPY))
			cmdType = CA_COPY;
		else if ( DROPEFFECT_MOVE == ( dropEffect & DROPEFFECT_MOVE ) )
			cmdType = CA_TRANSFER_TO;
		else
		{
			ASSERT(0);	// unknown drop effect
		}

		// Do the transfer!
		if (cmdType != CA_NONE)
		{
			if (qcfTransfer.m_pSingleSummary)
			{
				// Transfer/Copy a single message (typically tow trucked)
				ASSERT_KINDOF(CSummary, qcfTransfer.m_pSingleSummary);
				ASSERT(qcfTransfer.m_pSourceTocDoc == qcfTransfer.m_pSingleSummary->m_TheToc);
				pFolder->Execute( cmdType, qcfTransfer.m_pSingleSummary );
			}
			else
			{
				// Transfer/Copy one or more selected messages from a TOC
				pFolder->Execute( cmdType,  qcfTransfer.m_pSourceTocDoc);
			}
		}
	}
	else
	{
		ASSERT(0);
	}


	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnDragEnter [public, virtual]
//
////////////////////////////////////////////////////////////////////////
DROPEFFECT CTocView::OnDragEnter(COleDataObject* pDataObject,
								 DWORD dwKeyState, 
								 CPoint point)
{
	//TRACE0("CTocView::OnDragEnter()\n");
	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDragOver [public, virtual]
//
////////////////////////////////////////////////////////////////////////
DROPEFFECT CTocView::OnDragOver(COleDataObject* pDataObject,
								DWORD dwKeyState, 
								CPoint point)
{
	if (g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_TRANSFER))
	{
		//
		// Don't allow drag and drops if the source window is the
		// same as the target window.
		//
		CPoint screen_pt(point);
		ClientToScreen(&screen_pt);
		if (m_SumListBox.IsDraggingOver(screen_pt))
			return DROPEFFECT_NONE;

		if ((dwKeyState & MK_CONTROL) || ShiftDown())
			return DROPEFFECT_COPY;
		else
			return DROPEFFECT_MOVE;
	}

	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDragLeave [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CTocView::OnDragLeave()
{
	//TRACE0("CTocView::OnDragLeave()\n");
}



BOOL CTocView::OnEnterKey(UINT nID) 
{
	OpenMessages();
	return (TRUE);
}


void CTocView::Notify( 
QCCommandObject*	pObject,
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{
	switch(theAction )
	{
		case CA_COMPACT:
			if( pObject->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) ) )
			{
				SetCompact();
			}
			break;
		case CA_SWM_CHANGE_FEATURE:
			SizeHeaders();
			SizeBottomOfWindowControls();
			Invalidate();
			break;

	}
}


// this defeats the default formview behavior of calling IsDialogMessage
// which screws up the handling of TAB keys.
BOOL CTocView::PreTranslateMessage( MSG* msg )
{
	if (msg->message == WM_KEYDOWN && msg->wParam == VK_TAB)
		return FALSE;

	return (C3DFormView::PreTranslateMessage(msg));
}


// --------------------------------------------------------------------------

//
// FIND TEXT
//
BOOL CTocView::DoFindFirst(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect)
{
	const int nCount = m_SumListBox.GetCount();
	const int nStartIdx = m_SumListBox.GetSelItemIndex();
	int idx;

	for (idx = nStartIdx + 1; idx < nCount; idx++)
	{
		if (FindTextInSum(m_SumListBox.GetItemDataPtr(idx), szSearch, bMatchCase, bWholeWord) == TRUE)
		{
			m_SumListBox.SetSelCenter(idx);


			return (TRUE);
		}


	}

	for (idx = 0; idx <= nStartIdx; idx++)
	{
		if (FindTextInSum(m_SumListBox.GetItemDataPtr(idx), szSearch, bMatchCase, bWholeWord) == TRUE)
		{
			m_SumListBox.SetSelCenter(idx);

			return (TRUE);
		}
	}

	return (FALSE);



}

BOOL CTocView::DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect)
{
	const int nCount = m_SumListBox.GetCount();
	const int nStartIdx = m_SumListBox.GetSelItemIndex();


	int idx;

	for (idx = nStartIdx + 1; idx < nCount; idx++)
	{
		if (FindTextInSum(m_SumListBox.GetItemDataPtr(idx), szSearch, bMatchCase, bWholeWord) == TRUE)
		{
			m_SumListBox.SetSelCenter(idx);

			return (TRUE);
		}
	}

	for (idx = 0; idx <= nStartIdx; idx++)
	{
		if (FindTextInSum(m_SumListBox.GetItemDataPtr(idx), szSearch, bMatchCase, bWholeWord) == TRUE)
		{
			m_SumListBox.SetSelCenter(idx);

			return (TRUE);
		}
	}

	return (FALSE);

}

BOOL CTocView::FindTextInSum(CSummary *pSum, LPCTSTR pStr, BOOL bMatchCase, BOOL bWholeWord)
{
	ASSERT(pSum);
	ASSERT(pStr);
	if ((!pSum) || (!pStr))
		return (FALSE);

	// Search FROM
	if (QCStrstr(pSum->GetFrom(), pStr, bMatchCase != FALSE, bWholeWord? QCSTR_WORD_SEARCH:QCSTR_SUBSTR_SEARCH))
		return (TRUE);

	// Search SUBJECT
	if (QCStrstr(pSum->GetSubject(), pStr, bMatchCase != FALSE, bWholeWord? QCSTR_WORD_SEARCH:QCSTR_SUBSTR_SEARCH))
		return (TRUE);

	return (FALSE);
}
