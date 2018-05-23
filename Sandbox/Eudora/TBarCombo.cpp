#include "stdafx.h"
#include "eudora.h"
#include "TBarCombo.h"

IMPLEMENT_BUTTON(CTBarComboBtn);

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// WINDOWSX.H has a SubclassWindow macro which collides
#undef SubclassWindow


BOOL CTBarComboBtn::CreateWnd(CWnd* pParentWnd, DWORD dwStyle, CRect& rect, int nID)
{
	if(!(dwStyle & (CBS_DROPDOWNLIST | CBS_DROPDOWN)))
		dwStyle |= CBS_DROPDOWNLIST;

	// Create the combo box.
	BOOL bRes = CComboBox::Create(dwStyle, rect, pParentWnd, nID);
	ASSERT(bRes);
	if(bRes)
	{
		if(dwStyle & CBS_DROPDOWN)
		{
			// Subclass the child edit.
			CWnd* pWnd = GetWindow(GW_CHILD);
			if(pWnd != NULL)
			{
				VERIFY(m_edit.SubclassWindow(pWnd->m_hWnd));
				m_edit.m_pCombo = (SECWndBtn*) this;

			}
		}

		OnFontCreateAndSet();
		AdjustSize();		// save latest and greatest size info
	}

	return bRes;
}


void CTBarComboBtn::OnFontCreateAndSet() {
	// set default font for this control for proper sizing
	m_font.CreateFont (8, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        	   DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, 
			   CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, 
               DEFAULT_PITCH | FF_DONTCARE, _T("MS Sans Serif"));
   
	SetFont (&m_font,TRUE);
}

