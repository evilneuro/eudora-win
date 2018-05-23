// LabelComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "controls.h"
#include "resource.h"

#include "LabelComboBox.h"

#include "DebugNewHelpers.h"

// --------------------------------------------------------------------------
// CLabelComboBox

BEGIN_MESSAGE_MAP(CLabelComboBox, CComboBox)
	//{{AFX_MSG_MAP(CLabelComboBox)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropdown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// --------------------------------------------------------------------------

CLabelComboBox::CLabelComboBox()
{
	m_HighlightBkgdClr = GetSysColor(COLOR_HIGHLIGHT);
	m_HighlightTextClr = GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_WindowTextClr = GetSysColor(COLOR_WINDOWTEXT);
	m_WindowBkgdClr = GetSysColor(COLOR_WINDOW);

	m_bInited = false;
}

// --------------------------------------------------------------------------

CLabelComboBox::~CLabelComboBox()
{
}

// --------------------------------------------------------------------------

bool CLabelComboBox::InitLabels()
{
	QCLabelDirector *pLblDir = QCLabelDirector::GetLabelDirector();

	ASSERT(pLblDir);
	if (!pLblDir)
		return (false);

	CComboBox::ResetContent();

	// Fill list with labels text
	int nInsertIdx = -1;

	// First selection is 'None'
	{
		nInsertIdx = CComboBox::AddString("None");
		
		ASSERT(nInsertIdx >= 0);
		if (nInsertIdx >= 0)
			CComboBox::SetItemData(nInsertIdx, (DWORD) -1); // "None" has default color
	}

	// Add the labels, with color
	const unsigned int nLblCount = pLblDir->GetCount();
	for (unsigned int nLblIndex = 1; nLblIndex <= nLblCount; nLblIndex++)
	{
		nInsertIdx = CComboBox::AddString(pLblDir->GetLabelText(nLblIndex));
		
		ASSERT(nInsertIdx >= 0);
		if (nInsertIdx >= 0)
			CComboBox::SetItemData(nInsertIdx, (DWORD) pLblDir->GetLabelColor(nLblIndex));
	}

	CComboBox::SetCurSel(0); // Select first item by default

	m_bInited = true;

	return 0;
}

// --------------------------------------------------------------------------

unsigned int CLabelComboBox::GetCurLabel() const
{
	int nIdx = CComboBox::GetCurSel();
	ASSERT(nIdx >= 0);

	if (nIdx < 0)
		return (0);

	// The labels are listed in-order in the combo box, 0 = "None", 1 = "Label 1", etc.

	return (static_cast<unsigned int>(nIdx));
}

// --------------------------------------------------------------------------

void CLabelComboBox::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	if (!m_bInited)
		return;

	ASSERT(lpDIS);

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	ASSERT(pDC);

	pDC->SetBkMode(TRANSPARENT);

	const CRect ItemRct(lpDIS->rcItem);

	const bool bFocus = ((lpDIS->itemState & ODS_FOCUS) != 0);
	const bool bSelected = ((lpDIS->itemState & ODS_SELECTED) != 0);
	const bool bEditField = ((lpDIS->itemState & ODS_COMBOBOXEDIT) != 0);

	if (bSelected)
	{
		// Selected
		if (lpDIS->itemData == ((DWORD) -1))
			pDC->FillSolidRect(ItemRct, m_HighlightBkgdClr); // "None"
		else
			pDC->FillSolidRect(ItemRct, (COLORREF) lpDIS->itemData); // Label N

		pDC->SetTextColor(m_HighlightTextClr);
	}
	else
	{
		// Not selected
		if (lpDIS->itemData == ((DWORD) -1))
			pDC->SetTextColor(m_WindowTextClr); // "None"
		else
			pDC->SetTextColor((COLORREF) lpDIS->itemData); // Label N

		pDC->FillSolidRect(ItemRct, m_WindowBkgdClr);
	}

	if ((int)(lpDIS->itemID) < 0) // No item
		return;

	CString strText;
	GetLBText(lpDIS->itemID, strText);

	CRect TxtRct(ItemRct);

	if (bEditField)
		TxtRct.left += 1; // Edit field text is one pixel shifted
	else
		TxtRct.left += 2; // The drop-down text is shifted right two pixels

	pDC->DrawText(strText, TxtRct, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	if (bFocus)
		pDC->DrawFocusRect(ItemRct);
}

// --------------------------------------------------------------------------
// CLabelComboBox message handlers

void CLabelComboBox::OnDropdown() // CBN_DROPDOWN
{
}
