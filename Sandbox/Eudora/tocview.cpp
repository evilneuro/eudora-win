// TOCVIEW.CPP
//

#include "stdafx.h"
#include <afxcmn.h>
#include <afxrich.h>

#include <QCUtils.h>

#include "eudora.h"
#include "resource.h"
#include "rs.h"
#include "helpxdlg.h"
#include "utils.h"
#include "guiutils.h"
#include "cursor.h"
#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "tocview.h"
#include "font.h"
#include "msgdoc.h"
#include "mainfrm.h"
#include "compmsgd.h"
#include "changeq.h"
#include "controls.h"
#include "filtersd.h"
#include "SaveAsDialog.h"
#include "debug.h"
#include "msgframe.h"
#include "namenick.h"
#include "progress.h"
#include "fileutil.h"
#include "helpcntx.h"
#include "pop.h"
#include "MsgRecord.h"
#include "HostList.h"
#include "persona.h"
#include "ClipboardMgr.h"
#include "NewMBox.h"
#include "trnslate.h"
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
#include "MakeFilter.h"
#include "TocFrame.h"

#include "QCFindMgr.h"
#include "SearchEngine.h" // StringSearch

extern QCCommandStack			g_theCommandStack;
extern QCMailboxDirector		g_theMailboxDirector;
extern QCPersonalityDirector	g_thePersonalityDirector;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

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
HBITMAP CTocHeaderButton::m_hBitmapSortByPriority = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByAttachment = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByLabel = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortBySender = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByDate = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortBySize = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortByServerStatus = NULL;
HBITMAP CTocHeaderButton::m_hBitmapSortBySubject = NULL;

// Order in which to sort
static SortType g_SortMethod = NOT_SORTED;
static SortType g_LastSortMethod = NOT_SORTED;
static CTocDoc* g_SortTocDoc = NULL;

// Space between text and column separator
static const int ColumnSpace = 4;

// Width of widest state
static int g_MaxStatusWidth = 0;

// Registered message for preview-pane feature
UINT umsgPreviewMessage = RegisterWindowMessage( "umsgPreviewMessage" );

IMPLEMENT_DYNCREATE(CTocListBox, CListBox)

/////////////////////////////////////////////////////////////////////////////
// CTocListBox

CTocListBox::CTocListBox() :
	m_GotReturnKeyDown(FALSE),
	m_HorizWindowOrg(0),
	m_SavedMouseDownPoint(-1, -1),
	m_MouseState(MOUSE_IDLE),
	m_AllowDragAndDrop(TRUE),
	m_nLastKeyTime(0),
	m_nFindPhraseLen(0),
	m_nStartIndex(0),
	m_bInCanceledMode(false)
{
	m_pFindPhrase[0] = '\0';
	m_bTTSSubject = (GetIniShort(IDS_INI_TTS_SUBJECT) != 0); // IDS_INI_TTS_SUBJECT = "TypeToSelectSubject\n0"
}

CTocListBox::~CTocListBox()
{
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


//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// PreloadStatusBitmaps [public, static]
//FORNOW//
//FORNOW// Preloads the Status bitmaps shared by all CTocListBox instances.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWBOOL CTocListBox::PreloadStatusBitmaps()
//FORNOW{
//FORNOW	if (CTocListBox::m_bmpStatusUnread.GetSafeHandle())
//FORNOW		return TRUE;	// already initialized
//FORNOW
//FORNOW	if (false == ::QCLoadBitmap(IDB_STATUS_UNREAD, CTocListBox::m_bmpStatusUnread) ||
//FORNOW		false == ::QCLoadBitmap(IDB_STATUS_READ, CTocListBox::m_bmpStatusRead) ||
//FORNOW		false == ::QCLoadBitmap(IDB_STATUS_REPLIED, CTocListBox::m_bmpStatusReplied) ||
//FORNOW		false == ::QCLoadBitmap(IDB_STATUS_FORWARDED, CTocListBox::m_bmpStatusForwarded) ||
//FORNOW		false == ::QCLoadBitmap(IDB_STATUS_REDIRECT, CTocListBox::m_bmpStatusRedirect) ||
//FORNOW		false == ::QCLoadBitmap(IDB_STATUS_UNSENDABLE, CTocListBox::m_bmpStatusUnsendable) ||
//FORNOW		false == ::QCLoadBitmap(IDB_STATUS_SENDABLE, CTocListBox::m_bmpStatusSendable) ||
//FORNOW		false == ::QCLoadBitmap(IDB_STATUS_QUEUED, CTocListBox::m_bmpStatusQueued) ||
//FORNOW		false == ::QCLoadBitmap(IDB_STATUS_SENT, CTocListBox::m_bmpStatusSent) ||
//FORNOW		false == ::QCLoadBitmap(IDB_STATUS_UNSENT, CTocListBox::m_bmpStatusUnsent) ||
//FORNOW		false == ::QCLoadBitmap(IDB_STATUS_TIME_QUEUED, CTocListBox::m_bmpStatusTimeQueued))
//FORNOW	{
//FORNOW		ASSERT(0);
//FORNOW		return FALSE;
//FORNOW	}
//FORNOW
//FORNOW	BITMAP bm;
//FORNOW	CTocListBox::m_bmpStatusUnread.GetObject(sizeof(bm), &bm);
//FORNOW	s_nStatusWidth = bm.bmWidth;
//FORNOW	s_nStatusHeight = bm.bmHeight;
//FORNOW
//FORNOW	return TRUE;
//FORNOW}


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


	
BEGIN_MESSAGE_MAP(CTocListBox, CListBox)
	//{{AFX_MSG_MAP(CTocListBox)
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

int CompareItem(CSummary* Sum1, CSummary* Sum2)
{
	int Result;
	BOOL bImap = IsImapSum(Sum1);
	ASSERT( bImap == IsImapSum(Sum2));

	switch (g_SortMethod & 0x7F)
	{
	case NOT_SORTED:	Result = 0;		break;
	
	// Do status comparison seemingly in the reverse because we want the unread status messages to
	// show up at the bottom even though the unread status has the lowest constant
	case BY_STATUS:		Result = Sum2->m_State - Sum1->m_State;						break;
	case BY_PRIORITY:	Result = Sum1->m_Priority - Sum2->m_Priority;				break;
	case BY_ATTACHMENT:	Result = Sum1->HasAttachment() - Sum2->HasAttachment();		break;
	case BY_LABEL:		Result = Sum1->m_Label - Sum2->m_Label;						break;
	case BY_SENDER:		Result = stricmp(Sum1->m_From, Sum2->m_From);				break;
	case BY_DATE:		Result = CompareLong(Sum1->m_Seconds, Sum2->m_Seconds);		break;
	
	case BY_SIZE:		Result = CompareLong(bImap?Sum1->m_MsgSize:Sum1->m_Length, 
									bImap?Sum2->m_MsgSize:Sum2->m_Length);		break;
	
	case BY_SERVERSTATUS:Result = CompareServerStatus(Sum1, Sum2);						break;
	case BY_SUBJECT:	Result = CompareSubject(Sum1, Sum2);						break;
        
	default:
		ASSERT(FALSE);
		return (0);
	}
	
	if (g_SortMethod & 0x80)
		 Result = -Result;
		 
	return (Result);
}

inline bool HighestIndexLessThan(CSummary* Sum1, CSummary* Sum2)
{
	return (Sum1->m_Temp < Sum2->m_Temp);
}

inline bool SubjectLessThan(CSummary* Sum1, CSummary* Sum2)
{
	return (CompareSubject(Sum1, Sum2) < 0);
}

inline bool SumLessThan(CSummary* Sum1, CSummary* Sum2)
{
	if (!g_SortTocDoc)
	{
		ASSERT(0);
		return true;
	}

	for (int i = 0; i < g_SortTocDoc->NumSortColumns; i++)
	{
		g_SortMethod = g_SortTocDoc->GetSortMethod(i);
		if (g_SortMethod == NOT_SORTED)
			break;
		int Comparison = CompareItem(Sum1, Sum2);
		if (Comparison)
			return (Comparison < 0);
	}

	return true;
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

	// If we're drawing the focus, then post event to see if preview pane needs to be updated
	if ( (lpDIS->itemAction & ODA_FOCUS) && (lpDIS->itemState & ODS_FOCUS) )
	{
		//
		// Don't embed a Summary pointer inside the umsgPreviewMessage
		// since it could be invalid by the time the Frame gets around
		// to processing the message.  Instead, stuff the Summary pointer
		// in the CTocDoc and let the Frame get the value from there...
		//
		CTocView* pTocView = (CTocView *) GetParent();
		ASSERT_KINDOF(CTocView, pTocView);
		CTocDoc* pTocDoc = (CTocDoc *) pTocView->GetDocument();
		ASSERT_KINDOF(CTocDoc, pTocDoc);

		// This item has the focus, but we don't want unselected messages in the TOC
		// to be previewed because it creates confusion to the user.  If the message
		// is selected, we do want to preview it, though.
		if (lpDIS->itemState & ODS_SELECTED)
		{
//			TRACE0("Turning preview ON\n");
			pTocDoc->SetPreviewableSummary(Sum);		// can be NULL
		}
		else
		{
//			TRACE0("Turning preview OFF\n");
			pTocDoc->SetPreviewableSummary(NULL);
		}

		CTocFrame* pFrm = (CTocFrame*) GetParentFrame();
		pFrm->SendMessage(umsgPreviewMessage);
	}

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
		for (int i = FW_ATTACHMENT; i <= FW_SERVER; i++)
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
			COLORREF OldNonLabelColor;
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
		char Length[8];
		reg.left = rect.left + FieldSep[FW_SIZE - 1] + ColumnSpace;
		reg.right = rect.left + FieldSep[FW_SIZE] - ColumnSpace;
		reg.right = min(reg.right, rect.right);
		
		int nLen = IsImapSum(Sum)?Sum->m_MsgSize: ((Sum->m_Length/1024)+1);
		wsprintf(Length, "%2d", nLen);
		//wsprintf(Length, "%2d", (int)(Sum->m_Length / 1024) + 1);
		pDC->ExtTextOut(reg.left, reg.top, ETO_CLIPPED, &reg, Length, strlen(Length), NULL);
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
				if (Sum->m_Imflags & IMFLAGS_NOT_DOWNLOADED)
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

	// Subject
	if (GetIniShort(IDS_INI_MBOX_SHOW_SUBJECT))
	{
		reg.left = rect.left + FieldSep[FW_SERVER] + ColumnSpace;
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
				for (int i = FW_ATTACHMENT; i <= FW_SERVER; i++)
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
		case FW_PRIORITY:	g_SortMethod = BY_PRIORITY;		break;
		case FW_ATTACHMENT:	g_SortMethod = BY_ATTACHMENT;	break;
		case FW_LABEL:		g_SortMethod = BY_LABEL;		break;
		case FW_SENDER:		g_SortMethod = BY_SENDER;		break;
		case FW_DATE:		g_SortMethod = BY_DATE;			break;
		case FW_SIZE:		g_SortMethod = BY_SIZE;			break;
		case FW_SERVER:		g_SortMethod = BY_SERVERSTATUS;	break;
		case FW_UNUSED:		g_SortMethod = BY_SUBJECT;		break;

		default: ASSERT(FALSE); break;
		}

		CTocView* pTocView = (CTocView *) GetParent();
		ASSERT_KINDOF(CTocView, pTocView);
		CTocDoc* pTocDoc = (CTocDoc *) pTocView->GetDocument();
		ASSERT_KINDOF(CTocDoc, pTocDoc);
		POSITION AnchorPos = pTocDoc->m_Sums.FindIndex(Index);
		ASSERT(AnchorPos);
		int Count = GetCount();
		ASSERT(Count == pTocDoc->m_Sums.GetCount());

		// Start off with nothing selected
		SetSel(-1, FALSE);

		// Look above (before) the clicked-on summary for matches,
		// and gravitate the matches toward it
		int CompareIndex = Index;
		int CurrentIndex = CompareIndex - 1;
		POSITION ComparePos = AnchorPos;
		POSITION CurrentPos = ComparePos;
		pTocDoc->m_Sums.GetPrev(CurrentPos);
		POSITION PrevPos = CurrentPos;
		for (; CurrentPos; CurrentPos = PrevPos, CurrentIndex--)
		{
			CSummary* Sum = pTocDoc->m_Sums.GetPrev(PrevPos);
			if (Sum && SameItemSelectCompare(Field, AnchorSum, Sum) == 0)
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
				pTocDoc->m_Sums.GetPrev(ComparePos);
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
		pTocDoc->m_Sums.GetNext(CurrentPos);
		POSITION NextPos = CurrentPos;
		for (; CurrentPos; CurrentPos = NextPos, CurrentIndex++)
		{
			CSummary* Sum = pTocDoc->m_Sums.GetNext(NextPos);
			if (Sum && SameItemSelectCompare(Field, AnchorSum, Sum) == 0)
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
				pTocDoc->m_Sums.GetNext(ComparePos);
			}
			else if (!DoMove)
			{
				SelItemRange(FALSE, CurrentIndex, CurrentIndex + 1);
			}
		}
		if (DoMove)
			SelItemRange(TRUE, Index, CompareIndex);

		// This will make sure that the clicked-on summary will have the focus
		SetSel(Index, TRUE);

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
		BOOL PlainArrows = GetIniShort(IDS_INI_PLAIN_ARROWS) != 0;
		BOOL ControlArrows = GetIniShort(IDS_INI_CONTROL_ARROWS) != 0;
		
//		Shouldn't be doing this for mailboxes!
//		if (PlainArrows && !CtrlDown)
//		{
//			GetParent()->SendMessage(WM_COMMAND, (nChar == VK_LEFT || nChar == VK_UP)?
//				ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE);
//			return;
//		}
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
		((CTocView*)GetParent())->SendMessage(WM_COMMAND, ID_MESSAGE_STATUS_TOGGLE, LPARAM(GetSafeHwnd()));
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
	if (((nChar == VK_SPACE) && !ShiftDown()) || nChar == VK_RETURN)
		((CTocView*)GetParent())->OpenMessages();
	else if (nChar == VK_BACK)
		((CTocView*)GetParent())->Delete();
	else if (((nChar >= 'a') && (nChar <= 'z')) || ((nChar >= 'A') && (nChar <= 'Z')) || ((nChar >= '0') && (nChar <= '9')))
		AlphaCharacterPressed(nChar);
	else if ((nChar == '.')) // Type-To-Select find next
	{
		if ((m_nFindPhraseLen > 0) && (!m_bInCanceledMode))
		{
			m_nLastKeyTime = 0;
			ResetTTSPos();
			FindNextTTS();
		}
		else
		{
			MessageBeep(MB_ICONEXCLAMATION);
		}
	
		return;
	}
	else
		CListBox::OnChar(nChar, nRepCnt, nFlags);
}

void CTocListBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_RETURN && m_GotReturnKeyDown)
		((CTocView*)GetParent())->OpenMessages();
	else if (nChar == VK_DELETE && ((nFlags & 0x2000) == 0))
		((CTocView*)GetParent())->Delete();
	else
		CListBox::OnKeyUp(nChar, nRepCnt, nFlags);

	m_GotReturnKeyDown = FALSE;
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
	if ( !hMenu || ! popupMenus.Attach( hMenu ) )
	{
		ASSERT(0);		// hosed resources?
		return;
	}

	// Out mailbox has a different Popup than the others 
	int nMenuPos;
	if ( ((CTocView*)GetParent())->GetDocument()->m_Type ==  MBT_OUT )
	{
		 nMenuPos = MP_POPUP_OUT_MAILBOX;
	}
	else if( ((CTocView*)GetParent())->GetDocument()->m_Type ==  MBT_IMAP_MAILBOX )
	{
		 nMenuPos = MP_POPUP_IMAP_MAILBOX;
	}
	else
	{
		 nMenuPos = MP_POPUP_MAILBOX;
	}

	CMenu* pTempPopupMenu = popupMenus.GetSubMenu(nMenuPos);	
	if (pTempPopupMenu != NULL)
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

		CMenu theTransferMenu;
		theTransferMenu.CreatePopupMenu();		

		//
		// Populate the new Transfer popup menu.
		//
		g_theMailboxDirector.NewMessageCommands( CA_TRANSFER_TO, &theTransferMenu, CA_TRANSFER_NEW );
		::WrapMenu( theTransferMenu.GetSafeHmenu() );
		theTransferMenu.InsertMenu( 3, MF_BYPOSITION, ID_TRANSFER_NEW_MBOX_IN_ROOT, CRString( IDS_MAILBOX_NEW ) );
		theTransferMenu.InsertMenu( 3, MF_BYPOSITION | MF_SEPARATOR );

		tempPopupMenu.InsertMenu(	MP_TRANSFER_TOC,
									MF_BYPOSITION | MF_POPUP,
									(UINT) theTransferMenu.GetSafeHmenu(), 
									CRString( IDS_TRANSFER_NAME ) );

		//
		// Create Change Personality menu.
		//
		// Not for IMAP or OUT mbx
		//
		CMenu theChangePersonaMenu;

#ifdef COMMERCIAL
		const bool bAddPersonaMenu = ((nMenuPos != MP_POPUP_IMAP_MAILBOX) && (MP_POPUP_OUT_MAILBOX != nMenuPos));
#else
		//Single Persona only in 4.1 Light
		const bool bAddPersonaMenu = false;
#endif // COMMERCIAL

		if (bAddPersonaMenu)
		{
			theChangePersonaMenu.CreatePopupMenu();
			g_thePersonalityDirector.NewMessageCommands (CA_CHANGE_PERSONA, &theChangePersonaMenu );
			::WrapMenu( theChangePersonaMenu.GetSafeHmenu() );

			tempPopupMenu.InsertMenu(	MP_TRANSFER_TOC + 5,
									MF_BYPOSITION | MF_POPUP,
									(UINT) theChangePersonaMenu.GetSafeHmenu(), 
									CRString( IDS_CHANGE_PERSONA ) );
		}

		CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
		CContextMenu(&tempPopupMenu, ptScreen.x, ptScreen.y);
		
		tempPopupMenu.RemoveMenu(MP_TRANSFER_TOC + 5, MF_BYPOSITION);
		tempPopupMenu.RemoveMenu(MP_TRANSFER_TOC, MF_BYPOSITION);

		if (bAddPersonaMenu)
		{
			g_thePersonalityDirector.RemoveMessageCommands( CA_CHANGE_PERSONA, &theChangePersonaMenu );
		}

		g_theMailboxDirector.RemoveMessageCommands( CA_TRANSFER_TO, &theTransferMenu );
		g_theMailboxDirector.RemoveMessageCommands( CA_TRANSFER_NEW, &theTransferMenu );
		
		VERIFY(tempPopupMenu.Detach());
	}
	else
	{
		ASSERT(0);		// uh, oh ... resources hosed?
	}

	popupMenus.DestroyMenu();
} 

///////AlphaCharacterPressed///////
// This is for when an alpha character is pressed while the focus is in the toclistbox. We want to, instead of just sitting there
// Like a log, figure out which message they want to select. It will be based on which header is selected as the sort column.
/////////////////

void CTocListBox::AlphaCharacterPressed(UINT nChar)
{
	const DWORD dwTime = GetTickCount();
	const DWORD dwDeltaTime = (dwTime - m_nLastKeyTime);
	const bool bDelayExpired = (dwDeltaTime > 1000);

	m_nLastKeyTime = dwTime;

	if (m_bInCanceledMode)
	{
		if (bDelayExpired)
			m_bInCanceledMode = false;
		else
			return; // During cancel mode we do nothing (until the user waits long enough to reset)
	}

	if ((m_nFindPhraseLen >= 32) || (bDelayExpired))
	{
		// Reset everything
		m_pFindPhrase[0] = '\0';
		m_nFindPhraseLen = 0;

		if (bDelayExpired)
		{
			ResetTTSPos();
		}
		else // str too long
		{
			// They have exceeded max str len, so enter canceled mode (until it is reset)
			m_bInCanceledMode = true;
			return;
		}
	}

	// Append the typed character
	m_pFindPhrase[m_nFindPhraseLen] = ((char) nChar);
	m_pFindPhrase[++m_nFindPhraseLen] = '\0';

	FindNextTTS();
}

void CTocListBox::FindNextTTS()
{
	ASSERT(!m_bInCanceledMode);

	const int nCount = GetCount();

	LPSTR pTheFieldStr = NULL;
	CSummary *pSum = NULL;
	bool bFoundMatch = false;
	int nIndex = m_nLastMatchIndex;

	// We are searching from the selected (prior to TTS) down, then wrapping to top
	// This follows how FIND TEXT works.

	do {
		pSum = GetItemDataPtr(nIndex);
		ASSERT(pSum);

		if ((pSum) && (pSum != ((CSummary *)(-1))))
		{
			if (MatchTTS((LPSTR) pSum->GetFrom(), m_pFindPhrase))
			{
				bFoundMatch = true;
			}
			else if ((m_bTTSSubject) && (MatchTTS(SkipSubjectPrefix((LPSTR) pSum->GetSubject()), m_pFindPhrase)))
			{
				bFoundMatch = true;
			}
		}
	} while ((!bFoundMatch) && ((nIndex = GetNextWrapIndex(nIndex, nCount, m_nStartIndex)) >= 0));

	if (bFoundMatch)
	{
		// Only change the selection if we really need to
		if ((GetSel(nIndex) <= 0) || (GetSelCount() != 1))
		{
			SetSelCenter(nIndex); // Select the item and scroll it to center
			UpdateWindow(); // Force the selection to be shown
		}

		m_nLastMatchIndex = nIndex;
	}
	else // Not found
	{
		::MessageBeep(MB_ICONEXCLAMATION);
		m_bInCanceledMode = true;
	}
}

void CTocListBox::ResetTTSPos()
{
	m_nStartIndex = (GetSelItemIndex() + 1);

	if (m_nStartIndex >= GetCount())
		m_nStartIndex = 0;

	m_nLastMatchIndex = m_nStartIndex;
}

int CTocListBox::GetNextWrapIndex(int nCurIdx, int nItemCount, int nStartIdx)
{
	nCurIdx++;

	// Wrap to top when get to bottom
	if (nCurIdx >= nItemCount)
		nCurIdx = 0;

	// Stop when we reach start
	if (nCurIdx == nStartIdx)
		return (-1);

	return (nCurIdx);
}

// Find pSubStr in pStr (with TTS restrictions)
bool CTocListBox::MatchTTS(LPCTSTR pStr, LPCTSTR pSubStr)
{
	LPSTR pFound = QCStrstr(pStr, pSubStr, false, QCSTR_BEGWORD_SEARCH);

	// We only want to match beginning of words
	
	// Either it wasn't found,
	//  or it's not at the beginning AND the char before the match is an alpha num
	if ((!pFound) || ((pFound != pStr) && (isalnum(*(pFound - 1)))))
		return (false);

	return (true);
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
// LBN_SELCHANGE should be renamed to LBN_SELCHANGED.  Also, the docs
// on it are wrong.  You get LBN_SELCHANGE *after* the selection has
// been drawn, not "when the selection in a list box is about to change"
//
// The DrawItem() code will have already told the frame to preview the
// selected message that is getting the focus.  Now we just need to tell
// it do do the auto-mark-as-read behavior by passing a non-zero value
// in the wParam of the registered preview message.
////////////////////////////////////////////////////////////////////////
void CTocListBox::OnSelChange() 
{
//	TRACE0("CTocListBox::OnSelChange()\n");

	if (GetSelCount() == 1)
	{
		CTocFrame* pFrm = (CTocFrame*) GetParentFrame();
		pFrm->SendMessage(umsgPreviewMessage, TRUE);
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

	// Select the correct item
	SetSel(idx, TRUE);

	if (bChangeTopIdx)
		SetTopIndex(nTopIdx);

	SetRedraw(TRUE);
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
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYPRIORITY, CTocHeaderButton::m_hBitmapSortByPriority) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYATTACHMENT, CTocHeaderButton::m_hBitmapSortByAttachment) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYLABEL, CTocHeaderButton::m_hBitmapSortByLabel) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYSENDER, CTocHeaderButton::m_hBitmapSortBySender) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYDATE, CTocHeaderButton::m_hBitmapSortByDate) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYSIZE, CTocHeaderButton::m_hBitmapSortBySize) &&
		CTocHeaderButton::LoadOneButtonImage(IDB_SORTBYSERVERSTATUS, CTocHeaderButton::m_hBitmapSortByServerStatus) &&
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
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByPriority)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByAttachment)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByLabel)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortBySender)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByDate)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortBySize)
	PURGE_BITMAP(CTocHeaderButton::m_hBitmapSortByServerStatus)
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


/////////////////////////////////////////////////////////////////////////////
// CTocView

IMPLEMENT_DYNCREATE(CTocView, C3DFormView)


CTocView::CTocView() : C3DFormView(CTocView::IDD)
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
}

CTocView::~CTocView()
{
	g_theMailboxDirector.UnRegister( this );
}

void CTocView::SelectAll(BOOL bSelect /*= TRUE*/, BOOL bRedisplay /*= TRUE*/)
{
	int SelCount = m_SumListBox.GetSelCount();
	
	m_SumListBox.SetRedraw(FALSE);
	if (SelCount || bSelect)
		m_SumListBox.SetSel(-1, bSelect);
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
	DDX_Control(pDX, IDC_HSCROLL_BAR, m_HScrollBar);
	DDX_Control(pDX, IDC_SUMLIST, m_SumListBox);
	DDX_Control(pDX, IDC_PEANUT_MENU, m_PeanutButton);
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
	
	CTocDoc* TocDoc = GetDocument();

	// Size parent with saved position
	if (!TocDoc->m_SavedPos.IsRectEmpty())
		GetParentFrame()->MoveWindow(&TocDoc->m_SavedPos, FALSE);

	// Size parent preview pane
	{
		CTocFrame* pTocFrame = (CTocFrame *) GetParentFrame();
		ASSERT_KINDOF(CTocFrame, pTocFrame);
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

	// Add the Peanut Button if this has been Peanut-ized
	m_PeanutButton.LoadBitmaps(IDB_PEANUT_PLAIN, IDB_PEANUT_FLASHY);
	if (TocDoc->IsPeanutTagged())
		m_PeanutButton.ShowWindow(SW_SHOW);
	else
		m_PeanutButton.ShowWindow(SW_HIDE);
	
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
	if (TocDoc->m_bNeedsSorting || TocDoc->m_bNeedsSortingNOW)
	{
		// Sorting may add the items to the listbox
		bListBoxFilled = SortNow();
	}
	if (!bListBoxFilled)
	{
		// Add the summaries to the list box
		POSITION pos = TocDoc->m_Sums.GetHeadPosition();
		while (pos)
			m_SumListBox.AddString((LPCSTR)TocDoc->m_Sums.GetNext(pos));
	}

	// Select the last summary
	int Index = TocDoc->m_Sums.GetCount() - 1;
	m_SumListBox.SetSel(Index);

	m_SumListBox.SetTopIndex(Index);

	// keyboard messages would sometimes go bye-bye...
	GetDlgItem((UINT)ID_EDIT_SORT_SORTBYSENDER)->SetFocus();
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
	ON_BN_CLICKED(IDC_PEANUT_MENU, OnPeanutContextMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	//}}AFX_MSG_MAP

	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)

	// Commands not handled by ClassWizard
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateSelectAll)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_DELETE, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_SPECIAL_MAKENICKNAME, OnUpdateNeedSel)
	
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REPLY, OnUpdateResponse)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REPLY_ALL, OnUpdateResponse)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_FORWARD, OnUpdateResponse)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REDIRECT, OnUpdateResponse)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SENDAGAIN, OnUpdateResponse)
	
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SENDIMMEDIATELY, OnUpdateSendImmediately)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_CHANGEQUEUEING, OnUpdateChangeQueueing)
	
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_UNREAD, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_READ, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_REPLIED, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_FORWARDED, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_REDIRECTED, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_RECOVERED, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_TOGGLE, OnUpdateToggleStatus)
	
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_SENDABLE, OnUpdateSendable)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_QUEUED, OnUpdateSendable)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_TIME_QUEUED, OnUpdateSendable)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_SENT, OnUpdateSentUnsent)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_UNSENT, OnUpdateSentUnsent)

	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PRIOR_HIGHEST, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PRIOR_HIGH, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PRIOR_NORMAL, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PRIOR_LOW, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PRIOR_LOWEST, OnUpdateNeedSel)
	
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_NONE, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_1, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_2, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_3, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_4, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_5, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_6, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_7, OnUpdateNeedSel)

	ON_UPDATE_COMMAND_UI(ID_SPECIAL_MAKE_FILTER, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_SPECIAL_FILTERMESSAGES, OnUpdateNeedSel)

	// Added by JOK - 4/20/98
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_LEAVE, OnUpdateServerStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_FETCH, OnUpdateServerStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_DELETE, OnUpdateServerStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_FETCH_DELETE, OnUpdateServerStatus)
	
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, Delete)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnSelectAll)
	ON_COMMAND(ID_MESSAGE_DELETE, Delete)
	ON_COMMAND(ID_SPECIAL_MAKENICKNAME, OnMakeNickname)
	
	ON_COMMAND_EX_RANGE(ID_EDIT_SORT_SORTBYSTATUS, ID_EDIT_SORT_SORTBYSUBJECT, Sort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_SORT_SORTBYSTATUS, ID_EDIT_SORT_SORTBYSUBJECT, OnUpdateSort)

	ON_COMMAND(ID_EDIT_SORT_GROUPBYSUBJECT, OnGroupBySubject)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SORT_GROUPBYSUBJECT, OnUpdateGroupBySubject)
	
	ON_COMMAND_EX(ID_MESSAGE_REPLY, OnComposeMessage)
	ON_COMMAND_EX(ID_MESSAGE_REPLY_ALL, OnComposeMessage)
	ON_COMMAND_EX(ID_MESSAGE_FORWARD, OnComposeMessage)
	ON_COMMAND_EX(ID_MESSAGE_REDIRECT, OnComposeMessage)
	ON_COMMAND(ID_MESSAGE_SENDAGAIN, OnSendAgain)
	
	ON_COMMAND(ID_MESSAGE_SENDIMMEDIATELY, OnSendImmediately)
	ON_COMMAND(ID_MESSAGE_CHANGEQUEUEING, OnChangeQueueing)

	ON_COMMAND_EX(ID_MESSAGE_STATUS_UNREAD, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_READ, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_REPLIED, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_FORWARDED, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_REDIRECTED, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_SENDABLE, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_QUEUED, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_SENT, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_UNSENT, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_RECOVERED, SetStatus)

	ON_COMMAND(ID_MESSAGE_STATUS_TOGGLE, OnToggleStatus)

	ON_COMMAND_EX(ID_MESSAGE_PRIOR_HIGHEST, SetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_HIGH, SetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_NORMAL, SetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_LOW, SetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_LOWEST, SetPriority)
	
	ON_COMMAND_EX(ID_MESSAGE_LABEL_NONE, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_1, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_2, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_3, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_4, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_5, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_6, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_7, SetLabel)

	ON_COMMAND_EX(ID_MESSAGE_SERVER_LEAVE, SetServerStatus)
	ON_COMMAND_EX(ID_MESSAGE_SERVER_FETCH, SetServerStatus)
	ON_COMMAND_EX(ID_MESSAGE_SERVER_DELETE, SetServerStatus)
	ON_COMMAND_EX(ID_MESSAGE_SERVER_FETCH_DELETE, SetServerStatus)

	ON_COMMAND_EX(IDOK, OnEnterKey)

	ON_COMMAND(ID_SPECIAL_FILTERMESSAGES, FilterMessages)
	ON_COMMAND(ID_SPECIAL_MAKE_FILTER, OnMakeFilter)

	ON_COMMAND( ID_TRANSFER_NEW_MBOX_IN_ROOT, OnTransferNewInRoot )
	ON_UPDATE_COMMAND_UI( ID_TRANSFER_NEW_MBOX_IN_ROOT, OnUpdateTransferNewInRoot )
	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand )

#ifdef IMAP4
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_UNDELETE, OnUpdateUndelete)
	ON_COMMAND(ID_MESSAGE_UNDELETE, UnDelete)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PURGE, OnUpdateImapExpunge)
	ON_COMMAND(ID_MESSAGE_PURGE, ImapExpunge)
	ON_COMMAND(ID_MESSAGE_IMAP_CLEAR_CACHED, OnImapClearCached)
	ON_COMMAND(ID_MBOX_IMAP_RESYNC, OnImapResync)

	ON_UPDATE_COMMAND_UI(ID_MESSAGE_IMAP_REDOWNLOAD_FULL, OnUpdateImapRedownload)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_IMAP_REDOWNLOAD_DEFAULT, OnUpdateImapRedownload)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_IMAP_FETCH_FULL, OnUpdateImapFetch)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_IMAP_FETCH_DEFAULT, OnUpdateImapFetch)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_IMAP_CLEAR_CACHED, OnUpdateImapClearCached)

	ON_COMMAND_EX(ID_MESSAGE_IMAP_REDOWNLOAD_FULL, OnImapRedownload)
	ON_COMMAND_EX(ID_MESSAGE_IMAP_REDOWNLOAD_DEFAULT, OnImapRedownload)
	ON_COMMAND_EX(ID_MESSAGE_IMAP_FETCH_FULL, OnImapFetch)
	ON_COMMAND_EX(ID_MESSAGE_IMAP_FETCH_DEFAULT, OnImapFetch)
#endif

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

		BOOL bDocAlreadyLoaded = (pSummary->FindMessageDoc() != NULL);	// TRUE for view-less preview documents
		CMessageDoc* pDoc = pSummary->GetMessageDoc();
		if (pDoc)
		{
			char* pszFullMessage = pDoc->GetFullMessage();
			if (pszFullMessage)
			{
				const char* pszMessage = NULL;
				if (bIncludeHeaders)
				{
					//
					// Strip the taboo headers from received messages.
					//
					CReadMessageDoc* pReadMessageDoc = DYNAMIC_DOWNCAST(CReadMessageDoc, pDoc);
					if (pReadMessageDoc)
						 pReadMessageDoc->StripTabooHeaders(pszFullMessage);
					pszMessage = pszFullMessage;
				}
				else
					pszMessage = FindBody(pszFullMessage);

				if (bGuessParagraphs)
				{
					if (bIncludeHeaders)
					{
						//
						// Don't unwrap the headers!
						//
						char* pszBody = (char *) FindBody(pszMessage);
						if (pszBody)
							::UnwrapText(pszBody);
					}
					else
						::UnwrapText((char *) pszMessage);
				}

				theFile.PutLine(pszMessage);

				delete [] pszFullMessage;

				// If we saved this msg, then mark the message as read if it was unread
				if (pSummary->m_State == MS_UNREAD)
					pSummary->SetState(MS_READ);
			}

			if (! bDocAlreadyLoaded)
				pSummary->NukeMessageDocIfUnused();
		}
		else
		{
			ASSERT(0);		// hmm.  couldn't load document?
		}
	}
}

void CTocView::OnFilePrint()
{
	INT				nToPrint;
	INT				i;
	BOOL			bWasOpen;
	INT				nTotalCount;
	CSummary*		pSummary;
	QCProtocol*		pProtocol;
	BOOL			bError;
	INT				iUseBidentAlways;
	INT				iMajorVersion;
	INT				iMinorVersion;
	BOOL			bResetIniValues;
	CPrintDialog	theDialog( TRUE );
	
	EnableWindow( FALSE );

	nTotalCount = m_SumListBox.GetCount();
	nToPrint = m_SumListBox.GetSelCount();

	if( !nToPrint || ( AfxGetApp()->DoPrintDialog( &theDialog ) != IDOK ) )
	{
		EnableWindow( TRUE );
		return;
	}

	CountdownProgress( CRString( IDS_PRINTING_MESSAGES_LEFT ), nToPrint );
	bError = FALSE;

	bResetIniValues = FALSE;

	GetMSHTMLVersion( &iMajorVersion, &iMinorVersion);
	
	iUseBidentAlways = GetIniShort( IDS_INI_USE_BIDENT_ALWAYS );

	if ( iUseBidentAlways &&
		( iMajorVersion == 4 ) &&    
		( iMinorVersion == 70 ) )
	{
		SetIniShort( IDS_INI_USE_BIDENT_ALWAYS, 0 );
		bResetIniValues = TRUE;
	}

	CMDIChild::m_nCmdShow = SW_HIDE;

	// Start of the main loop going through all of the list items
	for( i = 0; !bError && nToPrint &&  ( i < nTotalCount ); i++ )
	{
		if ( m_SumListBox.GetSel(i) <= 0 )
		{
			continue;
		}

		nToPrint --;

		pSummary = m_SumListBox.GetItemDataPtr(i);
		
		if ( !pSummary )
		{
			continue;
		}

		// If the message window was already open, then we don't want to Display() it because
		// that will bring it to the front of the window list.  However, if the message window
		// isn't open, then Display() will create one for us, and the assignment above to set
		// CMDIChild::m_nCmdShow to SW_HIDE will make sure that the window doesn't get shown.
		
		bWasOpen =( pSummary->m_FrameWnd != NULL );
		
		if( !bWasOpen )
		{
			pSummary->Display();
		}

		pProtocol = QCProtocol::QueryProtocol( QCP_QUIET_PRINT, pSummary->m_FrameWnd );
		
		// Ok now that we have a valid view...let get on with the printing...
		if( pProtocol )
		{
			Progress( -1, pSummary->GetTitle(), -1 );
			bError = !pProtocol->QuietPrint();			
		}
				
		// If we were forced to create the message window becuase it wasn't open before we
		// started printing, then shut it down now
		if ( !bWasOpen )
		{
			pSummary->GetMessageDoc()->OnCloseDocument();
		}

		DecrementCountdownProgress();
	}
	
	CMDIChild::m_nCmdShow = -1;

	if( bResetIniValues )
	{
		// reset stuff
		SetIniShort( IDS_INI_USE_BIDENT_ALWAYS, short(iUseBidentAlways) );
		bResetIniValues = FALSE;
	}

	EnableWindow(TRUE);
	CloseProgress();
	m_SumListBox.SetFocus();
}


////////////////////////////////////////////////////////////////////////
// CopyTextToClipboard [protected]
//
// Generic routine for copying the text in a CString object to the
// clipboard in CF_TEXT format.
////////////////////////////////////////////////////////////////////////
BOOL CTocView::CopyTextToClipboard(const CString& rString) const
{
	// Allocate memory block
	HGLOBAL h_mem = ::GlobalAlloc(GMEM_DDESHARE, rString.GetLength() + 1);
	if (NULL == h_mem)
		return (FALSE);

	// Copy the string to the memory block
	char* p_str = (char*)::GlobalLock(h_mem);
	if (NULL == p_str)
	{
		::GlobalFree(h_mem);
		return (FALSE);
	}
	
	strcpy(p_str, rString);
	::GlobalUnlock(h_mem);
	p_str = NULL;

	// Copy the data to the clipboard
	BOOL status = FALSE;
	if (::OpenClipboard(GetSafeHwnd()))
	{
		::EmptyClipboard();
		status = ::SetClipboardData(CF_TEXT, h_mem) ? TRUE : FALSE;
		::CloseClipboard();
	}

	if (!status)
		::GlobalFree(h_mem);
	
	return (status);
}

void CTocView::OnEditCopy()
{
	CString SelText;

	if (GetSelText(SelText))
		CopyTextToClipboard(SelText);
}

void CTocView::OnUpdateSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CTocView::OnSelectAll()
{
	SelectAll(!ShiftDown(), TRUE);
}

void CTocView::OnUpdateResponse(CCmdUI* pCmdUI)
{

	BOOL EnableIt = FALSE;
	
	if (m_SumListBox.GetSelCount() > 0)
	{
		// Enable all menu items except Reply and Redirect for the Out mailbox
		EnableIt = TRUE;
		if (GetDocument()->m_Type == MBT_OUT && 
				(pCmdUI->m_nID == ID_MESSAGE_REPLY		|| 
				 pCmdUI->m_nID == ID_MESSAGE_REPLY_ALL	||
				 pCmdUI->m_nID == ID_MESSAGE_REDIRECT)	)
		{
			EnableIt = FALSE;
		}
	}
	
	pCmdUI->Enable(EnableIt);
	
	// If this is the Reply menu item, then continue the updating so that the text
	// can be set if necessary
	if (pCmdUI->m_nID == ID_MESSAGE_REPLY)
		pCmdUI->ContinueRouting();
	if (pCmdUI->m_nID == ID_MESSAGE_REPLY_ALL)
		pCmdUI->ContinueRouting();
}

void CTocView::OnUpdateNeedSel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_SumListBox.GetSelCount() > 0);
	
	if (pCmdUI->m_nID >= ID_MESSAGE_LABEL_1 && pCmdUI->m_nID <= ID_MESSAGE_LABEL_7)
		pCmdUI->ContinueRouting();
}


//
// Added by JOK - 4/20/98
// Disable these for IMAP!!
//
void CTocView::OnUpdateServerStatus(CCmdUI* pCmdUI)
{
	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc ())
	{
		pCmdUI->Enable(FALSE);
	}
	else
	{
		// For POP, shouldn't we test for selected messages??
		pCmdUI->Enable(TRUE);
	}
}


#ifdef IMAP4 // IMAP4
//
void CTocView::OnUpdateUndelete(CCmdUI* pCmdUI)
{
	//
	// Enable undelete if this is an IMAP mailbox and there are messages 
	// marked with the IMAP \delete flag.
	//

	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc ())
	{
		for (int i = 0; i < m_SumListBox.GetCount(); i++)
		{
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);

				if (Sum && (Sum->m_Imflags & IMFLAGS_DELETED))
				{
					// If at least 1 message has this flag.
					pCmdUI->Enable(TRUE);

					return;
				}
			}
		}
	}

	// If we get here.
	pCmdUI->Enable(FALSE);
}


void CTocView::UnDelete()
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc())
	{
		Doc->ImapUnDeleteMessages(NULL, FALSE);
	}
}


void CTocView::OnUpdateImapExpunge (CCmdUI* pCmdUI)
{
	//
	// Enable purge if this is an IMAP mailbox and there are messages 
	// marked with the IMAP \delete flag.
	//

	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc ())
	{
		for (int i = 0; i < m_SumListBox.GetCount(); i++)
		{
			CSummary* Sum = m_SumListBox.GetItemDataPtr(i);

			if (Sum && (Sum->m_Imflags & IMFLAGS_DELETED))
			{
				// If at least 1 message has this flag.
				pCmdUI->Enable(TRUE);

				return;
			}
		}
	}

	// If we get here.
	pCmdUI->Enable(FALSE);
}


void CTocView::ImapExpunge()
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc())
	{
		Doc->ImapExpunge (FALSE);
	}
}


// 
void CTocView::OnUpdateImapRedownload (CCmdUI* pCmdUI)
{
	//
	// Enable if this is an IMAP mailbox.
	//

	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc ())
	{
		for (int i = 0; i < m_SumListBox.GetCount(); i++)
		{
			// Is selected:
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);

				if (Sum)
				{
					// If at least 1 message has this flag, enable "Use defaults".
					if (pCmdUI->m_nID == ID_MESSAGE_IMAP_REDOWNLOAD_DEFAULT)
					{
						pCmdUI->Enable(TRUE);
						return;
					}

					// Enable "Include Attachments" only if a message has attachments.
					//
					if (pCmdUI->m_nID == ID_MESSAGE_IMAP_REDOWNLOAD_FULL)
					{
						if (Sum->HasAttachment())
						{
							pCmdUI->Enable(TRUE);
							return;
						}
					}
				}
			}
		}
	}

	// If we get here.
	pCmdUI->Enable(FALSE);
}


BOOL CTocView::OnImapRedownload(UINT nID)
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc())
	{
		//
		// Fetch the selected messages from the server.
		// Fetch attachments also.
		//
		BOOL bIncludeAttachments	= TRUE;
		BOOL bOnlyIfNotDownloaded	= FALSE;
		BOOL bClearCacheOnly		= FALSE;

		if (nID == ID_MESSAGE_IMAP_REDOWNLOAD_DEFAULT)
			bIncludeAttachments = FALSE;

		Doc->ImapFetchMessages (NULL, bIncludeAttachments, bOnlyIfNotDownloaded, bClearCacheOnly);
	}

	return TRUE;
}



// This only fetches if it's not yet downloaded
// So if none of the selected messages is un-downloaded, disable.
//
void CTocView::OnUpdateImapFetch (CCmdUI* pCmdUI)
{
	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc ())
	{
		const BOOL bShiftDown = ShiftDown();

		for (int i = 0; i < m_SumListBox.GetCount(); i++)
		{
			// Is selected:
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);

				// Shift + Fetch does redownload, which can always be done
				if (bShiftDown || (Sum && (Sum->m_Imflags & IMFLAGS_NOT_DOWNLOADED)))
				{
					// If at least 1 message has this flag, enable "Use defaults".
					if (pCmdUI->m_nID == ID_MESSAGE_IMAP_FETCH_DEFAULT)
					{
						pCmdUI->Enable(TRUE);
						return;
					}
				}

				if (bShiftDown || (Sum && (Sum->m_Imflags & (IMFLAGS_NOT_DOWNLOADED|IMFLAGS_UNDOWNLOADED_ATTACHMENTS))))
				{
					// Enable "Include Attachments" only if a message has attachments.
					//
					if (pCmdUI->m_nID == ID_MESSAGE_IMAP_FETCH_FULL)
					{
						if (Sum->HasAttachment())
						{
							pCmdUI->Enable(TRUE);
							return;
						}
					}
				}
			}
		}
	}

	// If we get here.
	pCmdUI->Enable(FALSE);
}



BOOL CTocView::OnImapFetch(UINT nID)
{
	// Shift + Fetch performs a redownload
	if (ShiftDown())
		return OnImapRedownload(nID == ID_MESSAGE_IMAP_FETCH_FULL? ID_MESSAGE_IMAP_REDOWNLOAD_FULL : ID_MESSAGE_IMAP_REDOWNLOAD_DEFAULT);


	// If this is an IMAP mailbox, do special things.
	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc())
	{
		//
		// Fetch the selected messages from the server.
		// Don't fetch attachments necessarily (unless
		// his options to omit only large attachments is set).
		//
		BOOL bIncludeAttachments = FALSE;
		BOOL bOnlyIfNotDownloaded = TRUE;
		BOOL bClearCacheOnly		= FALSE;

		if (nID == ID_MESSAGE_IMAP_FETCH_FULL)
			bIncludeAttachments = TRUE;

		Doc->ImapFetchMessages (NULL, bIncludeAttachments, bOnlyIfNotDownloaded, bClearCacheOnly);
	}

	return TRUE;
}



// Clear cached contents of selected messages and reset the summaries to
// Un-Downloaded.
//
void CTocView::OnUpdateImapClearCached(CCmdUI* pCmdUI)
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc ())
	{
		for (int i = 0; i < m_SumListBox.GetCount(); i++)
		{
			// Is selected:
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);

				// Enable if it's downloaded.
				if (Sum && !(Sum->m_Imflags & IMFLAGS_NOT_DOWNLOADED))
				{
					pCmdUI->Enable(TRUE);
					return;
				}
			}
		}
	}

	pCmdUI->Enable(FALSE);
}



void CTocView::OnImapClearCached()
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc())
	{
		//
		// Fetch the selected messages from the server.
		// Don't fetch attachments necessarily (unless
		// his options to omit only large attachments is set).
		//
		BOOL bIncludeAttachments	= FALSE;
		BOOL bOnlyIfNotDownloaded	= TRUE;
		BOOL bClearCacheOnly		= TRUE;	// This overrides the others.

		Doc->ImapFetchMessages (NULL, bIncludeAttachments, bOnlyIfNotDownloaded, bClearCacheOnly);
	}
}




// HISTORY:
// (4/20/98 - JOK) These menu items are now disabled, so this
// method would probably go away...
// END HISTORY
//
void CTocView::ImapHandleServerStatus (UINT nID)
{
	// Must have a doc.
	CTocDoc* doc = GetDocument();

	if (! ( doc && doc->IsImapToc() ) )
		return;

	//
	// We handle only these cases:
	//
	switch (nID)
	{
		case ID_MESSAGE_SERVER_FETCH:
			//
			// Fetch the selected messages from the server.
			//
			doc->ImapFetchMessages ();

			break;

		case ID_MESSAGE_SERVER_DELETE:
			//
			// Delete the selected messages.
			//
			doc->ImapDeleteMessages (NULL, FALSE, FALSE);

			break;

		default:
			break;
	}
}


// OnImapResync [PUBLIC]
//
// Resynchronize a mailbox.
// This is called after eudora has started so IMAP mboxes that open by default
// would get resync'd, but can also be called manually.
//
void CTocView::OnImapResync ()
{
	CTocDoc* doc = GetDocument();

	if ( !doc->IsImapToc() )
	{
		ASSERT (0);
		return;
	}

	// Pass this to the IMAP command object.
	//
	QCMailboxCommand* pImapFolder = g_theMailboxDirector.FindByPathname( (const char *) doc->MBFilename() );
	if( pImapFolder != NULL )
		pImapFolder->Execute( CA_IMAP_RESYNC, NULL);
	else
	{
		ASSERT(0);
	}
}


//
#endif // IMAP4


void CTocView::OnUpdateNeedSelNoOut(CCmdUI* pCmdUI)
{
	CTocDoc* Doc = GetDocument();
	int SelCount = m_SumListBox.GetSelCount();
	BOOL Enable = FALSE;
	int ThisState = -1;

	switch (pCmdUI->m_nID)
	{
	case ID_MESSAGE_STATUS_UNREAD:		ThisState = MS_UNREAD;		break;
	case ID_MESSAGE_STATUS_READ:		ThisState = MS_READ;		break;
	case ID_MESSAGE_STATUS_REPLIED:		ThisState = MS_REPLIED;		break;
	case ID_MESSAGE_STATUS_FORWARDED:	ThisState = MS_FORWARDED;	break;
	case ID_MESSAGE_STATUS_REDIRECTED:	ThisState = MS_REDIRECT;	break;
	case ID_MESSAGE_STATUS_RECOVERED:	ThisState = MS_RECOVERED;	break;
	default: ASSERT(FALSE);
	}

	if (ThisState != -1 && Doc && Doc->m_Type != MBT_OUT && SelCount > 0)
	{
		Enable = TRUE;
		for (int i = m_SumListBox.GetCount() - 1; SelCount && i >= 0; i--)
		{
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
				if (Sum && Sum->m_State != ThisState)
				{
					if (Sum->CantEdit())
					{
						Enable = FALSE;
						break;
					}
				}
				SelCount--;
			}
		}
	}

	pCmdUI->Enable(Enable);
}


void CTocView::OnUpdateToggleStatus(CCmdUI* pCmdUI)
{
	CTocDoc* pDocument = GetDocument();
	int nSelCount = m_SumListBox.GetSelCount();
	BOOL bEnable = FALSE;

	if (pDocument && pDocument->m_Type != MBT_OUT && nSelCount > 0)
	{
		bEnable = TRUE;
		for (int i = m_SumListBox.GetCount() - 1; nSelCount && i >= 0; i--)
		{
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
				if (Sum && Sum->m_State != MS_UNREAD && Sum->m_State != MS_READ)
				{
					bEnable = FALSE;
					break;
				}
				nSelCount--;
			}
		}
	}

	pCmdUI->Enable(bEnable);
}


void CTocView::OnUpdateSendable(CCmdUI* pCmdUI)
{
	CTocDoc* Doc = GetDocument();
	int SelCount = m_SumListBox.GetSelCount();
	BOOL Enable = FALSE;
	int ThisState = -1;

	switch (pCmdUI->m_nID)
	{
	case ID_MESSAGE_STATUS_SENDABLE:	ThisState = MS_SENDABLE;	break;
	case ID_MESSAGE_STATUS_QUEUED:		ThisState = MS_QUEUED;		break;
	case ID_MESSAGE_STATUS_TIME_QUEUED:	ThisState = MS_TIME_QUEUED;	break;
	default: ASSERT(FALSE);
	}

	if (ThisState != -1 && Doc && Doc->m_Type == MBT_OUT && SelCount > 0)
	{
		Enable = TRUE;
		for (int i = m_SumListBox.GetCount() - 1; SelCount && i >= 0; i--)
		{
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
				if (Sum && Sum->m_State != ThisState)
				{
					if (Sum->IsSendable() == FALSE)
					{
						Enable = FALSE;
						break;
					}
				}
				SelCount--;
			}
		}
	}

	pCmdUI->Enable(ThisState == MS_TIME_QUEUED? FALSE : Enable);
}

void CTocView::OnUpdateSentUnsent(CCmdUI* pCmdUI)
{
	int SelCount = m_SumListBox.GetSelCount();
	BOOL Enable = FALSE;
	int ThisState = -1;

	switch (pCmdUI->m_nID)
	{
	case ID_MESSAGE_STATUS_SENT:		ThisState = MS_SENT;		break;
	case ID_MESSAGE_STATUS_UNSENT:		ThisState = MS_UNSENT;		break;
	default: ASSERT(FALSE);
	}

	if (ThisState != -1 && SelCount > 0)
	{
		Enable = TRUE;
		for (int i = m_SumListBox.GetCount() - 1; SelCount && i >= 0; i--)
		{
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
				if (Sum && Sum->m_State != ThisState)
				{
					if (Sum->CantEdit() == FALSE)
					{
						Enable = FALSE;
						break;
					}
				}
				SelCount--;
			}
		}
	}

	pCmdUI->Enable(Enable);
}

void CTocView::OnUpdateSendImmediately(CCmdUI* pCmdUI)
{
	BOOL EnableIt = FALSE;
	int SelCount;
	
	// Send Immediate/Queue for Delivery can only be done on composition messages that are sendable
	if (GetDocument()->m_Type == MBT_OUT && (SelCount = m_SumListBox.GetSelCount()) > 0)
	{
		for (int i = m_SumListBox.GetCount() - 1; SelCount && i >= 0; i--)
		{
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
				if (Sum && Sum->IsSendable())
				{
					EnableIt = TRUE;
					break;
				}
				SelCount--;
			}
		}
	}
	
	pCmdUI->Enable(EnableIt);

	// Need to continue routing so that the main window can set the text of the menu item
	// depending on the Send Immediate switch
	pCmdUI->ContinueRouting();
}

void CTocView::OnUpdateChangeQueueing(CCmdUI* pCmdUI)
{
	BOOL EnableIt = FALSE;
	int SelCount;
	
	// Change Queueing can only be done on composition messages that are sendable
	if (GetDocument()->m_Type == MBT_OUT && (SelCount = m_SumListBox.GetSelCount()) > 0)
	{
		for (int i = m_SumListBox.GetCount() - 1; SelCount && i >= 0; i--)
		{
			if (m_SumListBox.GetSel(i) > 0)
			{
				if (m_SumListBox.GetItemDataPtr(i)->IsSendable())
				{
					EnableIt = TRUE;
					break;
				}
				SelCount--;
			}
		}
	}
	
	pCmdUI->Enable(EnableIt);
}

void CTocView::OnSendImmediately()
{
	BOOL ReverseSendQueue = ShiftDown();
	int SelCount;
	
	// Send Immediate/Queue for Delivery can only be done on messages that are sendable
	if (GetDocument()->m_Type == MBT_OUT && (SelCount = m_SumListBox.GetSelCount()) > 0)
	{
		for (int i = m_SumListBox.GetCount() - 1; SelCount && i >= 0; i--)
		{
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
				int State = Sum->m_State;
				if (State == MS_SENDABLE)
					Sum->SetState(MS_QUEUED);
				SelCount--;
			}
		}
		
		FlushQueue = GetIniShort(IDS_INI_IMMEDIATE_SEND);
		if (ReverseSendQueue)
			FlushQueue = !FlushQueue;
		
		SetQueueStatus();

		if (FlushQueue)
			SendQueuedMessages();
	}
}

void CTocView::OnChangeQueueing()
{
	long Seconds = 0;
	int SelCount;
	
	// Change Queueing can only be done on composition messages that are sendable or queued
	if (GetDocument()->m_Type == MBT_OUT && (SelCount = m_SumListBox.GetSelCount()) > 0)
	{
		int i;
		for (i = m_SumListBox.GetCount() - 1; SelCount && i >= 0; i--)
		{
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
				if (Sum->IsSendable())
				{
					if (Sum->m_State == MS_TIME_QUEUED && Sum->m_Seconds)
					{
						Seconds = Sum->m_Seconds;
						break;
					}
				}
				SelCount--;
			}
		}
		
		CChangeQueueing dlg(Seconds);
		if (dlg.DoModal() == IDOK)
		{
			SelCount = m_SumListBox.GetSelCount();
			for (i = m_SumListBox.GetCount() - 1; SelCount && i >= 0; i--)
			{
				if (m_SumListBox.GetSel(i) > 0)
				{
					CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
					if (Sum->IsSendable())
						dlg.ChangeSummary(Sum);
					SelCount--;
				}
			}
			if (FlushQueue)
				SendQueuedMessages();
		}
	}
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
	CRect rect, Crect;

	GetClientRect(&rect);
	GetDlgItem((UINT)ID_EDIT_SORT_SORTBYSTATUS)->GetClientRect(&Crect);

	Crect.left = rect.left;
	Crect.right = rect.right;
	Crect.top = Crect.bottom;
	Crect.bottom = rect.bottom - GetSystemMetrics(SM_CYHSCROLL);
	m_SumListBox.MoveWindow(&Crect, TRUE);

	// Win95 makes the scrollbar visible after sziing the window, so hide it
	m_SumListBox.ShowScrollBar(SB_HORZ, FALSE);

	m_Compact.GetClientRect(&Crect);
	Crect.top = rect.bottom - GetSystemMetrics(SM_CYHSCROLL);
	Crect.bottom = rect.bottom;
	m_Compact.MoveWindow(&Crect, TRUE);

	if (GetDocument()->IsPeanutTagged())
	{
		CRect pbRect;
		m_PeanutButton.GetClientRect(&pbRect);
		int width = 20;
		Crect.left = Crect.right + 2;
		Crect.right += width;
		m_PeanutButton.MoveWindow(&Crect, TRUE);
		m_PeanutButton.ShowWindow(SW_SHOW);
	}
	
	Crect.left = Crect.right;
	Crect.right = rect.right;
	m_HScrollBar.MoveWindow(&Crect, TRUE);

	m_OldWidth = cx;
	m_OldHeight = cy;


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
	
	GetClientRect(&rect);
	GetDlgItem((UINT)ID_EDIT_SORT_SORTBYSTATUS)->GetClientRect(&Crect);

	Crect.right = rect.right - GetSystemMetrics(SM_CXVSCROLL);
	SetFieldSeparators();
	for (int i = FW_UNUSED; i >= FW_STATUS; i--)
	{
		CWnd* control = GetDlgItem((UINT)(i + ID_EDIT_SORT_SORTBYSTATUS));
		if (!GetIniShort(IDS_INI_MBOX_SHOW_STATUS + i))
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

	UINT message;
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
			int OldFieldSep = m_FieldSep[FieldNum];
		
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
			m_SumListBox.GetItemRect(GetDocument()->m_Sums.GetIndex(Sum), &rect);
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
		g_theMailboxDirector.CompactAMailbox( toc );
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


void CTocView::Delete()
{
	BOOL WarnDeleteUnread = GetIniShort(IDS_INI_WARN_DELETE_UNREAD);
	BOOL WarnDeleteQueued = GetIniShort(IDS_INI_WARN_DELETE_QUEUED);
	BOOL WarnDeleteUnsent = GetIniShort(IDS_INI_WARN_DELETE_UNSENT);
	BOOL EasyDelete = !WarnDeleteUnread && !WarnDeleteQueued && !WarnDeleteUnsent;
	int SelCount = m_SumListBox.GetSelCount();
	int FoundSelected = FALSE;
	CTocDoc* doc = GetDocument();
	int MBType = doc->m_Type;
	CCursor cursor;
	
	for (int i = m_SumListBox.GetCount() - 1; i >= 0 && SelCount > 0; i--)
	{
		if (m_SumListBox.GetSel(i) > 0)
		{
			CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
			int State = Sum->m_State;
			FoundSelected = TRUE;
			if (EasyDelete)// || MBType == MBT_TRASH)
				break;
			if (State == MS_UNREAD && WarnDeleteUnread)
			{
				if (WarnDialog(IDS_INI_WARN_DELETE_UNREAD, IDS_WARN_DELETE_UNREAD) == IDOK)
					break;
				return;
			}
			else if (Sum->IsQueued() && WarnDeleteQueued)
			{
				if (WarnDialog(IDS_INI_WARN_DELETE_QUEUED, IDS_WARN_DELETE_QUEUED) == IDOK)
					break;
				return;
			}
			else if (Sum->IsSendable() && WarnDeleteUnsent)
			{
				if (WarnDialog(IDS_INI_WARN_DELETE_UNSENT, IDS_WARN_DELETE_UNSENT) == IDOK)
					break;
				return;
			}
			SelCount--;
		}
	}

	if (!FoundSelected)
		return;

#ifdef IMAP4
	// If this is an IMAP mailbox, just flag messages and get out.
	if (doc->IsImapToc())
	{
		// Pass this to the IMAP command object.
		//
		QCMailboxCommand* pImapFolder = g_theMailboxDirector.FindByPathname( (const char *) doc->MBFilename() );
		if( pImapFolder != NULL )
			pImapFolder->Execute( CA_DELETE_MESSAGE, NULL);
		else
		{
			ASSERT(0);
		}

		return;
	}
#endif // IMAP4

	// Clean up any unsaved open composition windows
	if (MBType == MBT_OUT)
	{
		SelCount = m_SumListBox.GetSelCount();
		for (int i = m_SumListBox.GetCount() - 1; i >= 0 && SelCount; i--)
		{
			if (m_SumListBox.GetSel(i) > 0)
			{
				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
				CCompMessageDoc* doc = (CCompMessageDoc*)Sum->FindMessageDoc();
				if (doc && doc->m_HasBeenSaved == FALSE)
					doc->OnSaveDocument(NULL);
				SelCount--;
			}
		}
	}

	CTocDoc* TrashToc = GetTrashToc();
	if (TrashToc)
	{
		//
		// We no longer call Xfer() here since we want to go through
		// the command director logic for CA_TRANSFER_TO.  The command
		// director notifies the TOC window containing this message
		// that it is okay to invoke the "auto-mark-as-read" logic
		// since the mailbox was changed due to an explicit user
		// action.  The Xfer() function does not notify the TOC
		// window.
		//
		//doc->Xfer(TrashToc);
		QCMailboxCommand* pFolder = g_theMailboxDirector.FindByPathname( (const char *) TrashToc->MBFilename() );
		if( pFolder != NULL )
			pFolder->Execute( CA_TRANSFER_TO, doc);
		else
		{
			ASSERT(0);
		}
	}
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
	CSumList& SumList = TocDoc->m_Sums;
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

		// Remember which items were selected so that they remain selected after resorting
		if (Sum->m_Temp2 = (m_SumListBox.GetSel(i) > 0))
		{
			if (!FirstSel)
				FirstSel = m_SumListBox.GetItemDataPtr(i);
		}
	}
	if (!FirstSel)
		FirstSel = m_SumListBox.GetItemDataPtr(m_SumListBox.GetCaretIndex());
	
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
		stable_sort(vSums.begin(), vSums.end(), SumLessThan);
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
		}

		// Finally, sort by the highest subject index so that subjects get grouped together
		stable_sort(vSums.begin(), vSums.end(), HighestIndexLessThan);
	}
	
	// Turn off redraw before doing major listbox item manipulation
	m_SumListBox.SetRedraw(FALSE);
	
	// Add the items back to the listbox in their newly sorted order.
	// Select an item if it was previously selected.
	m_SumListBox.ResetContent();
	SumList.RemoveAll();
	vector<CSummary*>::const_iterator SumIter = vSums.begin();
	for (; SumIter != vSums.end(); SumIter++)
	{
		Sum = *SumIter;
		SumList.AddTail(Sum);
		int Index = m_SumListBox.AddString((LPCSTR)Sum);
		if (Sum->m_Temp2)
			m_SumListBox.SetSel(Index);
	}
		
	// Restore the redraw
	m_SumListBox.SetRedraw(TRUE);
		
	// Ok, I suppose we'll be a nice little program and let them have control again
	AfxGetMainWnd()->EnableWindow(TRUE);

	return TRUE;
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

BOOL CTocView::Sort(UINT nID)
{
	CTocDoc* TocDoc = GetDocument();
	const BOOL bReverseCycle = ShiftDown();
	const BOOL bAddSort = CtrlDown();
	SortType OldSort = NOT_SORTED;
	SortType NormalSort, ReverseSort;
	BOOL bRedoSort = FALSE;
	CCursor cursor;
	int SortChangeIndex;
	int i;

	NormalSort = ((SortType)(BY_STATUS + (nID - ID_EDIT_SORT_SORTBYSTATUS)));
	ReverseSort = ((SortType)(NormalSort + (BY_REVERSE_STATUS - BY_STATUS)));

	if (!bAddSort)
	{
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

	if ((bReverseCycle && OldSort == NormalSort) || (!bReverseCycle && OldSort == ReverseSort))
	{
		// Undo the sort.
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
		if (OldSort == NOT_SORTED)
		{
			if (bReverseCycle)
				g_SortMethod = ReverseSort;
			else
				g_SortMethod = NormalSort;
		}
		else if (OldSort == NormalSort)
		{
			// bReverseCycle should be FALSE, otherwise it should have been caught above
			ASSERT(!bReverseCycle);
			g_SortMethod = ReverseSort;
		}
		else
		{
			ASSERT(OldSort == ReverseSort);
			// bReverseCycle should be TRUE, otherwise it should have been caught above
			ASSERT(bReverseCycle);
			g_SortMethod = NormalSort;
		}
	
		SetSortMethod(SortChangeIndex, g_SortMethod);
		RedisplayHeaderSortChange(g_SortMethod, TRUE);
		bRedoSort = TRUE;
	}
	
	if (bRedoSort)
		SortNow();
		
	g_LastSortMethod = g_SortMethod;	// Save the last sort method so we can do a type-to-select search on it.

	// Turn sorting off so that later additions go to the end
	g_SortMethod = NOT_SORTED;
    
	m_SumListBox.SetFocus();
	TocDoc->SetModifiedFlag();	
	
	return (TRUE);
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

	wsprintf(buf, CRString(IDS_TOC_K_FORMAT), toc->NumSums(), toc->m_MesSpace / 1024, toc->m_DelSpace / 1024);
	AfxSetWindowText(m_Compact.GetSafeHwnd(), buf);
	m_Compact.EnableWindow(toc->m_DelSpace != 0);
}


// SetStatus
// Set all of the selected summaries to the specified state
//
BOOL CTocView::SetStatus(UINT nID)
{
	int SelCount = m_SumListBox.GetSelCount();
	char ThisState;
	
	switch (nID)
	{
	case ID_MESSAGE_STATUS_UNREAD:		ThisState = MS_UNREAD;		break;
	case ID_MESSAGE_STATUS_READ:		ThisState = MS_READ;		break;
	case ID_MESSAGE_STATUS_REPLIED:		ThisState = MS_REPLIED;		break;
	case ID_MESSAGE_STATUS_FORWARDED:	ThisState = MS_FORWARDED;	break;
	case ID_MESSAGE_STATUS_REDIRECTED:	ThisState = MS_REDIRECT;	break;
	case ID_MESSAGE_STATUS_SENDABLE:	ThisState = MS_SENDABLE;	break;
	case ID_MESSAGE_STATUS_QUEUED:		ThisState = MS_QUEUED;		break;
	case ID_MESSAGE_STATUS_SENT:		ThisState = MS_SENT;		break;
	case ID_MESSAGE_STATUS_UNSENT:		ThisState = MS_UNSENT;		break;
	case ID_MESSAGE_STATUS_RECOVERED:	ThisState = MS_RECOVERED;	break;
	default: ASSERT(FALSE); return (FALSE);
	}

#ifdef IMAP4
	// If this is an IMAP toc, get the document to handle this because we've
	// got lots to do.
	//
	CTocDoc* toc = GetDocument();

	if ( toc && toc->IsImapToc() )
	{
		toc->ImapSetStatusOfSelectedMessages (ThisState);

		if (ThisState == MS_SENDABLE || ThisState == MS_QUEUED)
			SetQueueStatus();

		// We handled everything.
		//
		return TRUE;
	}
#endif // IMAP4
	
	for (int i = m_SumListBox.GetCount() - 1; i >= 0 && SelCount; i--)
	{
		if (m_SumListBox.GetSel(i) > 0)
		{
			CSummary* pSummary = m_SumListBox.GetItemDataPtr(i);
			ASSERT_KINDOF(CSummary, pSummary);
			pSummary->SetState(ThisState);
			SelCount--;
		}
	}

	if (ThisState == MS_SENDABLE || ThisState == MS_QUEUED)
		SetQueueStatus();
	
	return (TRUE);
}


////////////////////////////////////////////////////////////////////////
// OnToggleStatus [protected]
//
////////////////////////////////////////////////////////////////////////
void CTocView::OnToggleStatus()
{
	int nSelCount = m_SumListBox.GetSelCount();

#ifdef IMAP4
	// If an IMAP mailbox, pass the buck.
	CTocDoc* toc = GetDocument();

	if ( toc && toc->IsImapToc() )
	{
		toc->ImapToggleStatusOfSelectedMessages ();

		// We handled everything.
		//
		return;
	}
#endif // IMAP4
	
	for (int i = m_SumListBox.GetCount() - 1; i >= 0 && nSelCount; i--)
	{
		if (m_SumListBox.GetSel(i) > 0)
		{
			CSummary* pSummary = m_SumListBox.GetItemDataPtr(i);
			ASSERT_KINDOF(CSummary, pSummary);
			if (pSummary->IsComp())
			{
				//
				// Special case.  The Shift+Space shortcut bypasses
				// the normal CmdUI handlers and directly generates
				// an ID_MESSAGE_STATUS_TOGGLE command, so we have to
				// make sure we don't mark comp messages with a bogus
				// "unread" status.
				//
			}
			else if (MS_UNREAD == pSummary->m_State)
				pSummary->SetState(MS_READ);
			else if (MS_READ == pSummary->m_State)
				pSummary->SetState(MS_UNREAD);

			nSelCount--;
		}
	}
}


// SetPrioriy
// Set all of the selected summaries to the specified priority
//
BOOL CTocView::SetPriority(UINT nID)
{
	int Priority = (int)(nID - ID_MESSAGE_PRIOR_HIGHEST) + MSP_HIGHEST;
	int SelCount = m_SumListBox.GetSelCount();
	
	for (int i = m_SumListBox.GetCount() - 1; i >= 0 && SelCount; i--)
	{
		if (m_SumListBox.GetSel(i) > 0)
		{
			m_SumListBox.GetItemDataPtr(i)->SetPriority(Priority);
			SelCount--;
		}
	}
	
	return (TRUE);
}

// SetLabel
// Set all of the selected summaries to the specified label
//
BOOL CTocView::SetLabel(UINT nID)
{
	int Label = (int)(nID - ID_MESSAGE_LABEL_NONE);
	int SelCount = m_SumListBox.GetSelCount();
	
	for (int i = m_SumListBox.GetCount() - 1; i >= 0 && SelCount; i--)
	{
		if (m_SumListBox.GetSel(i) > 0)
		{
			m_SumListBox.GetItemDataPtr(i)->SetLabel(Label);
			SelCount--;
		}
	}
	
	return (TRUE);
}

// SetServerStatus
// Set all of the selected summaries to the specified server status
//
BOOL CTocView::SetServerStatus(UINT nID)
{

#ifdef IMAP4 // IMAP4
	//
	// If this is an IMAP mailbox, do special things for Fetch and Delete.
	// Otherwise, fall through.
	CTocDoc* doc = GetDocument();

	if ( doc && doc->IsImapToc () )
	{
		ImapHandleServerStatus (nID);
	
		return TRUE;
	}

#endif // END IMAP4

	int SelCount = m_SumListBox.GetSelCount();
	
	for (int i = m_SumListBox.GetCount() - 1; i >= 0 && SelCount; i--)
	{
		if (m_SumListBox.GetSel(i) > 0)
		{
			m_SumListBox.GetItemDataPtr(i)->SetServerStatus(nID);
			SelCount--;
		}
	}
	
	return (TRUE);
}

void CTocView::OnMakeNickname()
{
	MakeNickname();
}

void CTocView::FilterMessages()
{
	int TotalCount = m_SumListBox.GetCount();
	int SelCount = m_SumListBox.GetSelCount();
	int DidTransfer = FALSE;

#ifdef IMAP4
	// Do IMAP differently.
	CTocDoc* Doc = GetDocument();

	if (Doc && Doc->IsImapToc ())
	{
		Doc->ImapFilterMessages();

		return;
	}
#endif // IMAP4	


	CFilterActions filt;

	if (!filt.StartFiltering())
		return;
		
	CountdownProgress(CRString(IDS_FILTER_MESSAGES_LEFT), SelCount);

	// process things from highest index to lowest
	// if messages are transfered out the lower selection indexes remain valid
	// if messages are transfered back in (circular filter actions) they are added at the end
	// so lower selection indexes remain valid
	for (int i = TotalCount - 1; i >= 0 && SelCount; i--)
	{
		if (EscapePressed())
			break;
			
		if (m_SumListBox.GetSel(i) > 0)
		{
			if (filt.FilterOne(m_SumListBox.GetItemDataPtr(i), WTA_MANUAL) & FA_TRANSFER)
				DidTransfer = TRUE;

			SelCount--;			// we can probably get out early...

			DecrementCountdownProgress();
		}
	}
	
	// The listbox redraw got turned off if a transfer was done, so turn it on
	if (DidTransfer)
	{
		m_SumListBox.SetRedraw(TRUE);
	}
	
	filt.EndFiltering();
}

extern int MessageCascadeSpot = -1;

////////////////////////////////////////////////////////////////////////
// ComposeMessage [public]
//
// Public method for doing a Reply, Forward, or Redirect.
////////////////////////////////////////////////////////////////////////
void CTocView::ComposeMessage
(
	UINT nID, 
	const char* pszDefaultTo, 
	const char* pszStationery /*= NULL*/,
	const char* pszPersona /*= NULL*/
)
{
	CCursor cursor;
	int Selected = m_SumListBox.GetSelCount();
	
	StartGroup();

	MessageCascadeSpot = 0;
	::AsyncEscapePressed(TRUE);				// reset Escape key logic
	for (int i = m_SumListBox.GetCount() - 1; Selected && i >= 0; i--)
	{
		if (::AsyncEscapePressed())
			break;
		if (m_SumListBox.GetSel(i) > 0)
		{
			m_SumListBox.GetItemDataPtr(i)->ComposeMessage(nID, pszDefaultTo, FALSE, pszStationery, pszPersona);
			Selected--;
		}
	}
	MessageCascadeSpot = -1;

	EndGroup();
}


////////////////////////////////////////////////////////////////////////
// OnComposeMessage [protected]
//
// Internal command handler for message map.
////////////////////////////////////////////////////////////////////////
BOOL CTocView::OnComposeMessage(UINT nID)
{
	ComposeMessage(nID, NULL);
	
	return (TRUE);
}

void CTocView::Fumlub()
{
	if (!GetIniShort(IDS_INI_FUMLUB))
		return;

	CTocDoc* doc = GetDocument();
	POSITION PrevPos, pos;
	
	if (!doc || !(pos = doc->m_Sums.GetTailPosition()))
		return;

	CSummary* Sum = NULL;
	int i = doc->m_Sums.GetCount() - 1;
	for (PrevPos = pos; doc->m_Sums.GetPrev(PrevPos) && PrevPos; pos = PrevPos, i--)
	{
		Sum = doc->m_Sums.GetAt(PrevPos);
		if (Sum && Sum->m_State != MS_UNREAD)
			break;
	}
	
	if (!PrevPos)
	{
		pos = doc->m_Sums.GetHeadPosition();
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


////////////////////////////////////////////////////////////////////////
// SendAgain [public]
//
// Public method for doing the Send Again command.
////////////////////////////////////////////////////////////////////////
void CTocView::SendAgain(const char* pszPersona /*= NULL*/)
{
	int Selected = m_SumListBox.GetSelCount();

	StartGroup();
	
	MessageCascadeSpot = 0;
	::AsyncEscapePressed(TRUE);
	for (int i = m_SumListBox.GetCount() - 1; Selected && i >= 0; i--)
	{
		if (::AsyncEscapePressed())
			break;
		if (m_SumListBox.GetSel(i) > 0)
		{
			m_SumListBox.GetItemDataPtr(i)->SendAgain(TRUE, pszPersona);
			Selected--;
		}
	}
	MessageCascadeSpot = -1;

	EndGroup();
}


////////////////////////////////////////////////////////////////////////
// OnSendAgain [public]
//
// Internal command handler for message map.
////////////////////////////////////////////////////////////////////////
void CTocView::OnSendAgain()
{
	SendAgain();
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
		if (!GetIniShort(IDS_INI_MBOX_SHOW_STATUS + i))
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
	QCMailboxCommand* pFolder = g_theMailboxDirector.FindByPathname( (const char *) p_tocdoc->MBFilename() );

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


void CTocView::OnTransferNewInRoot()
{
	QCMailboxCommand*	pCommand;
	pCommand = g_theMailboxDirector.CreateTargetMailbox( NULL, TRUE );
	if ( pCommand )
	{
		ASSERT_KINDOF( QCMailboxCommand, pCommand );
		ASSERT( pCommand->GetType() == MBT_REGULAR );

		if ( ShiftDown() )
			pCommand->Execute( CA_COPY, GetDocument() );
		else
			pCommand->Execute( CA_TRANSFER_TO, GetDocument() );
	}
}


void CTocView::OnUpdateTransferNewInRoot(
CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_SumListBox.GetSelCount() > 0 );
}


BOOL CTocView::OnDynamicCommand(
UINT uID )
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	
	
	if( !g_theCommandStack.GetCommand( ( WORD ) uID, &pCommand, &theAction ) )
	{
		return FALSE;
	}

	if( ( pCommand == NULL ) || !theAction )
	{
		return FALSE;
	}

	if( theAction == CA_TRANSFER_NEW )
	{
		ASSERT_KINDOF( QCMailboxCommand, pCommand );
		pCommand = g_theMailboxDirector.CreateTargetMailbox( ( QCMailboxCommand* ) pCommand, TRUE );
		if ( NULL == pCommand )
			return TRUE;		// user didn't want to transfer after all

		ASSERT_KINDOF( QCMailboxCommand, pCommand );

#ifdef IMAP4 // Must also work if this is an IMAP mailbox. Sorry.

		ASSERT( ( ( QCMailboxCommand* ) pCommand)->GetType() == MBT_REGULAR ||
				( ( QCMailboxCommand* ) pCommand)->GetType() == MBT_IMAP_MAILBOX );

#else // Original->

		ASSERT( ( ( QCMailboxCommand* ) pCommand)->GetType() == MBT_REGULAR );

#endif // END IMAP4

		theAction = CA_TRANSFER_TO;
	}

	if ( pCommand->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) ) )
	{
		if ( theAction == CA_TRANSFER_TO )
		{
			if ( ShiftDown() )
				pCommand->Execute( CA_COPY, GetDocument() );
			else
				pCommand->Execute( CA_TRANSFER_TO, GetDocument() );
			return TRUE;
		}
	}

	if( ( theAction == CA_REPLY_WITH ) ||
		( theAction == CA_REPLY_TO_ALL_WITH ) ||
		( theAction == CA_FORWARD_TO ) ||
		( theAction == CA_REDIRECT_TO ) ||
		( theAction == CA_CHANGE_PERSONA ) )
	{
		CCursor		theCursor;
		int			nSelected;
		
		nSelected = m_SumListBox.GetSelCount();
		
		StartGroup();

		MessageCascadeSpot = 0;
	
		::AsyncEscapePressed(TRUE);				// reset Escape key logic
		
		for (int i = m_SumListBox.GetCount() - 1; nSelected && i >= 0; i-- )
		{
			if (::AsyncEscapePressed())
			{
				break;
			}

			if (m_SumListBox.GetSel(i) > 0)
			{
				pCommand->Execute( theAction, m_SumListBox.GetItemDataPtr(i) );
				nSelected--;
			}
		}
		
		MessageCascadeSpot = -1;

		EndGroup();
		return TRUE;
	}
	return FALSE;
}


void CTocView::OnUpdateDynamicCommand(
CCmdUI* pCmdUI)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	

	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) ( pCmdUI->m_nID ), &pCommand, &theAction ) )
		{
			if( ( theAction == CA_REPLY_WITH ) ||
				( theAction == CA_REPLY_TO_ALL_WITH ) || 
				( theAction == CA_REDIRECT_TO ) )
			{				
				// Enable all menu items except Reply and Redirect for the Out mailbox
				pCmdUI->Enable( IsReplyable() );
				return;
			}	
			
			if ( ( theAction == CA_TRANSFER_TO ) || 
				( theAction == CA_TRANSFER_NEW ) ||
				( theAction == CA_FORWARD_TO ) )

			{
				pCmdUI->Enable( m_SumListBox.GetSelCount() > 0 );
				return;
			}

			if ( theAction == CA_CHANGE_PERSONA )
			{
				int nNumSelected = m_SumListBox.GetSelCount();
				if (nNumSelected > 0)
				{
					for (int i = m_SumListBox.GetCount() - 1; i >= 0 && nNumSelected; i--)
					{
						if (m_SumListBox.GetSel(i) > 0)
						{
							CSummary* pSummary = m_SumListBox.GetItemDataPtr(i);
							if ( pSummary->IsComp() )
							{
								pCmdUI->Enable( FALSE );
								return;
							}
							nNumSelected--;
						}
					}

					//
					// If we get this far, then no composition messages
					// are selected, so allow the Change Persona command.
					//
					pCmdUI->Enable( TRUE );
					return;
				}
				else
				{
					pCmdUI->Enable( FALSE );		// nothing selected
					return;
				}
			}
		}
	}

	pCmdUI->ContinueRouting();
}


BOOL CTocView::IsReplyable()
{	
	if ( (m_SumListBox.GetSelCount() > 0) && ( GetDocument()->m_Type != MBT_OUT ) )
	{
		return TRUE;
	}
	
	return FALSE;
}


void CTocView::Notify( 
QCCommandObject*	pObject,
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{
	if( ( theAction == CA_COMPACT ) && pObject->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) ) )
	{
		SetCompact();
	}
}

void CTocView::OnMakeFilter()
{
	CMakeFilter	mkFiltDlg;
	CSumList	summaryList;

	int SelCount = m_SumListBox.GetSelCount();

	for (int i = m_SumListBox.GetCount() - 1; i >= 0 && SelCount; i--)
	{
		if (m_SumListBox.GetSel(i) > 0)
		{
			CSummary *selSum = m_SumListBox.GetItemDataPtr(i);
			summaryList.Add(selSum);
			SelCount--;
		}
	}

	if (mkFiltDlg.GoFigure(&summaryList))
		mkFiltDlg.DoModal();

	summaryList.RemoveAll();
}

void CTocView::OnPeanutContextMenu()
{	
	CRect rect;
	m_PeanutButton.GetWindowRect(&rect);
	((CEudoraApp *)AfxGetApp())->GetTranslators()->MboxContextMenu(GetDocument(), rect);	
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

void CTocView::OnUpdateEditFindFindText(CCmdUI* pCmdUI) // Find (Ctrl-F)
{
	pCmdUI->Enable(TRUE);
}

void CTocView::OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI) // Find Again (F3)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if ((pFindMgr) && (pFindMgr->CanFindAgain()))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

LONG CTocView::OnFindReplace(WPARAM wParam, LPARAM lParam) // WM_FINDREPLACE
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if (!pFindMgr)
		return (EuFIND_ERROR);

	// This is our internal message to ask if we support find.
	// Return non-zero (TRUE).
	if (pFindMgr->IsAck(wParam, lParam))
		return (EuFIND_ACK_YES);

	LPFINDREPLACE lpFR = (LPFINDREPLACE) lParam;
	ASSERT(lpFR);

	if (!lpFR)
		return (EuFIND_ERROR);

	if (lpFR->Flags & FR_DIALOGTERM)
	{
		ASSERT(0); // Should never fwd a terminating msg
		return (EuFIND_ERROR);
	}

	const int nCount = m_SumListBox.GetCount();
	const int nStartIdx = m_SumListBox.GetSelItemIndex();
	// nStartIdx == (-1) when no selection -- the following loops work fine in that case

	int idx;

	for (idx = nStartIdx + 1; idx < nCount; idx++)
	{
		if (FindTextInSum(m_SumListBox.GetItemDataPtr(idx), lpFR->lpstrFindWhat, ((lpFR->Flags & FR_MATCHCASE) != 0), ((lpFR->Flags & FR_WHOLEWORD)) != 0))
		{
			m_SumListBox.SetSelCenter(idx);

			return (EuFIND_OK);
		}
	}

	for (idx = 0; idx <= nStartIdx; idx++)
	{
		if (FindTextInSum(m_SumListBox.GetItemDataPtr(idx), lpFR->lpstrFindWhat, ((lpFR->Flags & FR_MATCHCASE) != 0), ((lpFR->Flags & FR_WHOLEWORD)) != 0))
		{
			m_SumListBox.SetSelCenter(idx);

			return (EuFIND_OK);
		}
	}

	return (EuFIND_NOTFOUND);
}

bool CTocView::FindTextInSum(CSummary *pSum, LPCTSTR pStr, bool bMatchCase, bool bWholeWord)
{
	ASSERT(pSum);
	ASSERT(pStr);
	if ((!pSum) || (!pStr))
		return (false);

	// Search FROM
	if (QCStrstr(pSum->GetFrom(), pStr, bMatchCase, bWholeWord?QCSTR_WORD_SEARCH:QCSTR_SUBSTR_SEARCH))
		return (true);

	// Search SUBJECT
	if (QCStrstr(pSum->GetSubject(), pStr, bMatchCase, bWholeWord?QCSTR_WORD_SEARCH:QCSTR_SUBSTR_SEARCH))
		return (true);

	return (false);
}
