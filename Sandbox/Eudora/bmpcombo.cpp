// BMPCOMBO.CPP
//

#include "stdafx.h"

#include "QCUtils.h"

#include "font.h"
#include "bmpcombo.h"
#include "resource.h"
#include "rs.h"
#include "QCGraphics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


CBitmapComboItem::CBitmapComboItem(UINT BitmapID, UINT StringID, int ID /*= 0*/)
{
	m_Text.LoadString(StringID);
	
	BITMAP bm;
	
	if ( BitmapID )
	{
		QCLoadBitmap( BitmapID, m_Bitmap );
	
		VERIFY(m_Bitmap.GetObject(sizeof(bm), &bm));
		m_BitmapWidth = bm.bmWidth;
		m_BitmapHeight = bm.bmHeight;
	}
	else
	{
		m_BitmapWidth = 0;
		m_BitmapHeight = 0;
	}

	m_ID = ID;
}

CBitmapComboItem::CBitmapComboItem(UINT BitmapID, CString String, int ID /*= 0*/)
{
	m_Text = String;
	
	BITMAP bm;

	if( BitmapID )
	{
		QCLoadBitmap( BitmapID, m_Bitmap );

		m_Bitmap.GetObject(sizeof(bm), &bm);
		m_BitmapWidth = bm.bmWidth;
		m_BitmapHeight = bm.bmHeight;
	}
	else
	{
		m_BitmapWidth = 0;
		m_BitmapHeight = 0;
	}
	
	m_ID = ID;
}


/////////////////////////////////////////////////////////////////////////////
// CBitmapCombo

// Static members
CFontInfo CBitmapCombo::m_Font;
int CBitmapCombo::m_Count = 0;
	
IMPLEMENT_DYNCREATE(CBitmapCombo, CComboBox)

CBitmapCombo::CBitmapCombo()
{
	m_ItemHeight = HIWORD(GetDialogBaseUnits()) + 2;
	m_Width = 2 * SPACE;
	m_Height = 2 * SPACE;
	m_pLastFocus = NULL;
	m_EditWidth = 0;
}

CBitmapCombo::~CBitmapCombo()
{
}

void CBitmapCombo::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = m_ItemHeight;
}

int CBitmapCombo::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	(lpCompareItemStruct);
	// Has to be here or else MFC debug library will ASSERT
	return (0);
}

void CBitmapCombo::DeleteItem(LPDELETEITEMSTRUCT lpDIS)
{
	CBitmapComboItem* item = (CBitmapComboItem*)lpDIS->itemData;
	
	delete item;
}

void CBitmapCombo::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC*				pDC = CDC::FromHandle(lpDIS->hDC);
	CBitmapComboItem*	item = (CBitmapComboItem*)lpDIS->itemData;
	CRect				rect(lpDIS->rcItem);
	CDC					MemDC;
	CSize				theSize;
	
	// Draw focus rect
	if ((lpDIS->itemAction & ODA_FOCUS) == ODA_FOCUS)
	{
		pDC->DrawFocusRect(&rect);
		return;
	}
	
	if (lpDIS->itemID & 0x8000)
		return;
	
	// Set up some color values
	COLORREF BackColor, ForeColor;
	if ((lpDIS->itemState & ODS_SELECTED) && GetDroppedState())
	{
		BackColor = ::GetSysColor(COLOR_HIGHLIGHT);
		ForeColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	}
	else
	{
		BackColor = ::GetSysColor(COLOR_WINDOW);
		ForeColor = ::GetSysColor(COLOR_WINDOWTEXT);
	}
	
	// Draw background
	CBrush BackBrush(BackColor);
	pDC->FillRect(&rect, &BackBrush);

	// Draw bitmap                                    
	if (MemDC.CreateCompatibleDC(pDC) && ( item->m_Bitmap.m_hObject != NULL ) )
	{
		int 	left = rect.left + SPACE;
		
		CBitmap* OldBitmap = MemDC.SelectObject(&item->m_Bitmap);
		
		pDC->BitBlt(left, (rect.top + rect.bottom - item->m_BitmapHeight) / 2,
			item->m_BitmapWidth, item->m_BitmapHeight, &MemDC, 0, 0,
			SRCAND);

		if(OldBitmap != NULL)
			MemDC.SelectObject(OldBitmap);

		rect.left += item->m_BitmapWidth;
	}
	
	rect.left += SPACE * 2;

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(ForeColor);
	pDC->SetTextAlign( TA_LEFT | TA_TOP );
	
	// center the item vertically
	theSize = pDC->GetTextExtent( item->m_Text, item->m_Text.GetLength() );
	if ( theSize.cy < ( rect.bottom - rect.top + 1 ) )
	{
		rect.top += ( ( rect.bottom - rect.top + 1 ) - theSize.cy ) >> 1;
	}

	// draw the text
	pDC->ExtTextOut(rect.left, rect.top, ETO_CLIPPED, &rect, item->m_Text, item->m_Text.GetLength(), 0);
}

BOOL CBitmapCombo::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (!CComboBox::Create(dwStyle | WS_CHILD | WS_VISIBLE |
		CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED, rect, pParentWnd, nID))
	{
		return (FALSE);
	}

	if (!m_Count)
		m_Font.CreateFont(CRString(IDS_BMPCOMBO_FONT_NAME), atoi(CRString(IDS_BMPCOMBO_FONT_SIZE)));
		
	SetFont(&m_Font, FALSE);
	m_Count++;
	m_EditWidth = rect.right - rect.left + 1;
	m_Width = m_EditWidth;

	return (TRUE);
}

int CBitmapCombo::Add(CBitmapComboItem* Item)
{
	int BitmapWidth;
	int Width;

	if (!Item)
	{
		return (CB_ERR);
	}	

	Width = 2 * SPACE + TextWidth(&m_Font, Item->m_Text);

	if ( Item->m_BitmapWidth )
	{
		BitmapWidth = 2 * SPACE + Item->m_BitmapWidth;
		Width += BitmapWidth;
	}
	
	if (Width > m_Width)
	{
		m_Width = Width;
	}

	m_Height += m_ItemHeight + 1;
	
	if ( Item->m_BitmapHeight )
	{
		SetWindowPos(NULL, 0, 0, BitmapWidth + GetSystemMetrics(SM_CXSIZE), Item->m_BitmapHeight + m_Height + SPACE,//m_Height,
					 SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);
	}
	else
	{
		SetWindowPos(	NULL, 0, 0, 
						m_EditWidth, 
						m_ItemHeight + m_Height,
						SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER );
	}
	
	SetDroppedWidth( m_Width + SPACE);
	
	return (AddString((LPCSTR)Item));
}


void CBitmapCombo::PostNcDestroy()
{
	CComboBox::PostNcDestroy();

	m_Count--;
	if (!m_Count)
		m_Font.DeleteObject();
}

BEGIN_MESSAGE_MAP(CBitmapCombo, CComboBox)
	//{{AFX_MSG_MAP(CBitmapCombo)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CBitmapCombo::RestoreFocus()
{
	if( m_pLastFocus )
	{
		m_pLastFocus->SetFocus();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBitmapCombo message handlers


void CBitmapCombo::OnSetFocus(CWnd* pOldWnd)
{
	m_pLastFocus = pOldWnd;
	CComboBox::OnSetFocus(pOldWnd);
}


/////////////////////////////////////////////////////////////////////////////
// CPriorityCombo

CPriorityCombo::CPriorityCombo()
{
}

CPriorityCombo::~CPriorityCombo()
{
}

BOOL CPriorityCombo::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (!CBitmapCombo::Create(dwStyle, rect, pParentWnd, nID))
		return (FALSE);

	for (UINT i = IDB_PRIOR_HIGHEST; i <= IDB_PRIOR_LOWEST; i++)
		Add(new CBitmapComboItem(i, i - IDB_PRIOR_HIGHEST + IDS_PRIORITY_HIGHEST));
		
	return (TRUE);
}
	

BOOL CPriorityCombo::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (GET_WM_COMMAND_HWND(wParam, lParam) && GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELENDOK )
	{
		// Priority has been selected, so post an ID_MESSAGE_PRIOR_XXX message
		// to the view to kick things off
		UINT Message = (UINT)(ID_MESSAGE_PRIOR_HIGHEST + GetCurSel());
#ifdef WIN32
		GetParentFrame()->GetActiveView()->PostMessage(WM_COMMAND, MAKEWPARAM(Message,1), 0);
#else
		GetParentFrame()->GetActiveView()->PostMessage(WM_COMMAND, Message, MAKELPARAM(0, 1));
#endif
		return (TRUE);
	}
	
	return (CBitmapCombo::OnCommand(wParam, lParam));
}

BEGIN_MESSAGE_MAP(CPriorityCombo, CBitmapCombo)
	//{{AFX_MSG_MAP(CPriorityCombo)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPriorityCombo message handlers
