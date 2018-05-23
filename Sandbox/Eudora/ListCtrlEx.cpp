// ListCtrlEx.cpp : implementation file
//

#include "stdafx.h"
#include <list> // STL list

#include "ListCtrlEx.h"

// Debug Memory Manager block
// This should be after all system includes and before any user includes
#ifdef _DEBUG 
	#undef THIS_FILE 
	static char THIS_FILE[] = __FILE__; 
	#define new DEBUG_NEW 
#endif // _DEBUG

// --------------------------------------------------------------------------

// Don't define this if you want a slow and flickery list
#define USE_OFFSCREEN_BITMAP

UINT msgListCtrlEx_RBtn = ::RegisterWindowMessage("msgListCtrlEx_RBtn");
UINT msgListCtrlEx_LBtnDblClk = ::RegisterWindowMessage("msgListCtrlEx_LBtnDblClk");
UINT msgListCtrlEx_DeleteKey = ::RegisterWindowMessage("msgListCtrlEx_DeleteKey");
UINT msgListCtrlEx_ReturnKey = ::RegisterWindowMessage("msgListCtrlEx_ReturnKey");

// --------------------------------------------------------------------------
//	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_DELETEITEM, nItem, 0L); }

BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
	//{{AFX_MSG_MAP(CListCtrlEx)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHeaderClicked) 
	ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHeaderClicked)
	//}}AFX_MSG_MAP
	ON_WM_MEASUREITEM_REFLECT( )
	ON_MESSAGE(WM_SETFONT, OnSetFont)
END_MESSAGE_MAP()

//ON_WM_RBUTTONDOWN( )
//afx_msg void OnRButtonDown( UINT, CPoint );	

// --------------------------------------------------------------------------

CListCtrlEx::CListCtrlEx()
{
	m_nHighlight = LVEX_HIGHLIGHT_NORMAL;

	m_DispTypeArr = NULL;
	m_DispTypeArrLen = 0;

	m_DispImageListArr = NULL;
	m_DispImageListArrLen = 0;

	m_ImageWidthArr = NULL;
	m_ImageWidthArrLen = 0;

	m_ImageHeightArr = NULL;
	m_ImageHeightArrLen = 0;

	m_MinRowHeight = 0;
	m_nLabelOffset = 0;

	unsigned int m_GraphHeight = 0; // Init, will be set later

	m_GraphBorderColor = RGB(0,0,0); // Black
	m_GraphCompletedColor = RGB(18,106,254);
	m_GraphRemainColor = RGB(129,207,254);

	m_GraphUseBorder = true;

	m_bHorzGrid = false;

	m_nLinePerRow = 1;
	m_bEatReturnKey = false;

	ResetSortColumn();
}

// --------------------------------------------------------------------------

CListCtrlEx::~CListCtrlEx()
{
	if (m_DispTypeArr)
		delete[] m_DispTypeArr;

	if (m_ImageWidthArr)
		delete[] m_ImageWidthArr;

	if (m_ImageHeightArr)
		delete[] m_ImageHeightArr;

	if (m_DispImageListArr)
	{
		for (unsigned int i=0; i<m_DispImageListArrLen; i++)
		{
			if (m_DispImageListArr[i])
				delete m_DispImageListArr[i];
		}

		delete[] m_DispImageListArr;
	}
}

// --------------------------------------------------------------------------

int CListCtrlEx::SetHighlightType(int hilite)
{
	int oldhilite = m_nHighlight;
	if( hilite <= LVEX_HIGHLIGHT_ROW )
	{
		m_nHighlight = hilite;
		Invalidate();
	}	
	
	return oldhilite;
}

// --------------------------------------------------------------------------

void CListCtrlEx::DrawItemLabel(CDC *pDC,
								CRect &ColumnRct,
								CString &LabelStr,
								UINT nColAlignment,
								unsigned int nColumn)
{
	bool bDoNormalDisplay = true;
	unsigned int nDispType = GetColDisplayType(nColumn);

	switch (nDispType)
	{
		case LVEX_DISPLAY_BARGRAPH:
		{
			char cFirstChar = '\0';
			
			if (LabelStr.GetLength() > 0)
				cFirstChar = LabelStr.GetAt(0);

			if ((cFirstChar >= '0') && (cFirstChar <= '9'))
			{
				char *pColon = strchr(LPCSTR(LabelStr), ':');

				COLORREF cCompleted = m_GraphCompletedColor, cRemain = m_GraphRemainColor;

				if (pColon)
				{
					int nRed = 0, nGreen = 0, nBlue = 0;
					char *pRGB = (pColon + 1);

					if (sscanf(pRGB, "%d,%d,%d", &nRed, &nGreen, &nBlue) == 3)
					{
						cCompleted = RGB(nRed, nGreen, nBlue);
						pColon = strchr(pRGB, ':');

						if (pColon)
						{
							pRGB = (pColon + 1);

							if (sscanf(pRGB, "%d,%d,%d", &nRed, &nGreen, &nBlue) == 3)
							{
								cRemain = RGB(nRed, nGreen, nBlue);
							}
						}
					}
				}

				double val = ((double) atoi(LPCSTR(LabelStr))) / 100;

				if ((val >= 0) && (val <= 100))
				{
					CRect FullBarRct, CompletedRct, RemainingRct;

					FullBarRct = ColumnRct;
					
					FullBarRct.InflateRect(-2,-2);
					FullBarRct.top = (ColumnRct.CenterPoint().y - (m_GraphHeight / 2));
					FullBarRct.bottom = FullBarRct.top + m_GraphHeight;


					if ((FullBarRct.left + 2) < FullBarRct.right) // Min graph width is three
					{
						int nSavedDC = pDC->SaveDC();

						CompletedRct = FullBarRct;
	
						if (m_GraphUseBorder)
							CompletedRct.InflateRect(-1,-1);
						
						RemainingRct = CompletedRct;

						CompletedRct.right = CompletedRct.left + (int(((CompletedRct.right - CompletedRct.left)+1) * (val)));
						RemainingRct.left = CompletedRct.right;

						if (CompletedRct.right > RemainingRct.right)
							CompletedRct.right = RemainingRct.right;

						if (m_GraphUseBorder)
						{
							CBrush borderBrush(m_GraphBorderColor);
							CBrush* pBorderBrush = &borderBrush;
							pDC->FrameRect(FullBarRct, pBorderBrush);
						}

						if (CompletedRct.left <= CompletedRct.right)
							pDC->FillSolidRect(CompletedRct, cCompleted);

						if (RemainingRct.left <= RemainingRct.right)
							pDC->FillSolidRect(RemainingRct, cRemain);

						pDC->RestoreDC(nSavedDC);
					}

					bDoNormalDisplay = false;
				}
			}
			else if (cFirstChar == '\'')
				LabelStr = LabelStr.Right(LabelStr.GetLength() - 1);
		}
		break;

		case LVEX_DISPLAY_IMAGELIST:
		{
			CImageList *pImageList = GetColImageList(nColumn);
			if (pImageList)
			{
				char cFirstChar = LabelStr.GetAt(0);

				if ((cFirstChar >= '0') && (cFirstChar <= '9'))
				{
					const int nImageIdx = atoi(LPCSTR(LabelStr));
					pImageList->GetImageCount();

					if ((nImageIdx >= 0) && (nImageIdx < (pImageList->GetImageCount())))
					{
						const unsigned int nIconWidth = GetColImageWidth(nColumn);

						CString TxtStr;
						unsigned int nTxtWidth = 0;
						
						int nIdx = LabelStr.Find(',');
						if (nIdx != (-1))
						{
							TxtStr = LabelStr.Right(LabelStr.GetLength() - (nIdx + 1)); // nIdx is zero-based
							nTxtWidth = (unsigned int) ((pDC->GetTextExtent(LPCSTR(TxtStr))).cx);
						}

						if (nTxtWidth > 0) // ICON with text
						{
							const unsigned int nSpacing = m_nLabelOffset;
							const unsigned int nBothWidth = nIconWidth + nSpacing + nTxtWidth;

							CPoint ImagePt;
							ImagePt.y = ColumnRct.top;

							UINT nEllipsis = 0;

							switch (nColAlignment)
							{
								case LVCFMT_RIGHT:
								{
									ImagePt.x = ColumnRct.right - nBothWidth - m_nLabelOffset;

									if (ImagePt.x < ColumnRct.left)
										ImagePt.x = ColumnRct.left;

									nEllipsis = DT_PATH_ELLIPSIS;
								}
								break;

								case LVCFMT_CENTER:
								{
									ImagePt.x = ColumnRct.left + ((ColumnRct.Width() / 2) - (nBothWidth / 2));

									if (ImagePt.x < ColumnRct.left)
										ImagePt.x = ColumnRct.left;

									nEllipsis = DT_END_ELLIPSIS;
								}
								break;

								default:
								case LVCFMT_LEFT:
								{
									if ((ColumnRct.Width() - m_nLabelOffset) < (int) nBothWidth)
										ImagePt.x = ColumnRct.left;
									else
										ImagePt.x = ColumnRct.left + m_nLabelOffset;

									nEllipsis = DT_END_ELLIPSIS;
								}
								break;
							}

							// TBD: Center vert

							pImageList->Draw(pDC, nImageIdx, ImagePt, ILD_TRANSPARENT);

							CRect TxtRct = ColumnRct;
							TxtRct.left = ImagePt.x + nIconWidth + nSpacing;

							if (m_nLinePerRow == 1)
							{
								pDC->DrawText(TxtStr, -1, TxtRct, DT_LEFT
																			| DT_SINGLELINE
																			| DT_NOPREFIX
																			| DT_VCENTER
																			| DT_NOPREFIX
																		//	| DT_WORDBREAK
																			| nEllipsis);
							}
							else // multi-line = do wrapping
							{
								pDC->DrawText(TxtStr, -1, TxtRct, DT_LEFT
																		//	| DT_SINGLELINE
																		//	| DT_NOPREFIX
																		//	| DT_VCENTER
																			| DT_NOPREFIX
																			| DT_WORDBREAK
																			| nEllipsis);
							}
						}
						else // ICON only
						{
							CPoint ImagePt;
							ImagePt.y = ColumnRct.top;

							switch (nColAlignment)
							{
								case LVCFMT_RIGHT:
								{
									ImagePt.x = ColumnRct.right - nIconWidth - m_nLabelOffset;

									if (ImagePt.x < ColumnRct.left)
										ImagePt.x = ColumnRct.left;
								}
								break;

								case LVCFMT_CENTER:
								{
//									const unsigned int nImageWidth = nIconWidth;
									ImagePt.x = ColumnRct.left + ((ColumnRct.Width() / 2) - (nIconWidth / 2));

									if (ImagePt.x < ColumnRct.left)
										ImagePt.x = ColumnRct.left;
								}
								break;

								default:
								case LVCFMT_LEFT:
								{
									ImagePt.x = ColumnRct.left + m_nLabelOffset;
								}
								break;
							}

							// TBD: Center vert

							pImageList->Draw(pDC, nImageIdx, ImagePt, ILD_TRANSPARENT);
						}

						bDoNormalDisplay = false;
					}
				}
				else if (cFirstChar == '\'')
					LabelStr = LabelStr.Right(LabelStr.GetLength() - 1);
			}
		}
		break;
	}

	if (bDoNormalDisplay)
	{
		UINT nTextAlignment = 0;
		UINT nEllipsis = 0;

		switch (nColAlignment)
		{
			case LVCFMT_RIGHT:
			{
				nTextAlignment = DT_RIGHT;
				nEllipsis = DT_PATH_ELLIPSIS;
			}
			break;

			case LVCFMT_CENTER:
			{
				nTextAlignment = DT_CENTER;
				nEllipsis = DT_END_ELLIPSIS;
			}
			break;

			default:
			case LVCFMT_LEFT:
			{
				nTextAlignment = DT_LEFT;
				nEllipsis = DT_END_ELLIPSIS;
			}
			break;
		}

		CRect LabelRct = ColumnRct;		
		LabelRct.left += m_nLabelOffset;
		LabelRct.right -= m_nLabelOffset;
		
		if (m_nLinePerRow == 1)
		{
			pDC->DrawText(LabelStr, -1, LabelRct, nTextAlignment
														| DT_SINGLELINE
														| DT_NOPREFIX
														| DT_VCENTER
														| DT_NOPREFIX
													//	| DT_WORDBREAK
														| nEllipsis);
		}
		else // multi-line = do wrapping
		{
			pDC->DrawText(LabelStr, -1, LabelRct, nTextAlignment
													//	| DT_SINGLELINE
													//	| DT_NOPREFIX
													//	| DT_VCENTER
														| DT_NOPREFIX
														| DT_WORDBREAK
														| nEllipsis);
		}
	}
}

// --------------------------------------------------------------------------

void CListCtrlEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pOriginalDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	int nSavedDC = pOriginalDC->SaveDC();

	CRect OrgItemRct(lpDrawItemStruct->rcItem);
	int nItemIdx = lpDrawItemStruct->itemID;

	// Get rects
	CRect BoundsRct, LabelRct, IconRct, ColumnRct, ClientRct;
	GetItemRect(nItemIdx, BoundsRct, LVIR_BOUNDS);
	GetItemRect(nItemIdx, LabelRct, LVIR_LABEL);	
	GetItemRect(nItemIdx, IconRct, LVIR_ICON);
	ColumnRct = BoundsRct;

	GetClientRect(&ClientRct);
	ClientRct.top = BoundsRct.top;
	ClientRct.bottom = BoundsRct.bottom;

	CRect ItemRct = OrgItemRct;
	CDC* pDC = pOriginalDC;

	CRect SrcItemRct(0,0,OrgItemRct.Width(), OrgItemRct.Height());
	CRect DestItemRct = ClientRct;

#ifdef USE_OFFSCREEN_BITMAP
	{
		int nX = -(BoundsRct.left);
		int nY = -(BoundsRct.top);
		BoundsRct.OffsetRect(nX, nY);
		LabelRct.OffsetRect(nX, nY);
		IconRct.OffsetRect(nX, nY);
		ColumnRct.OffsetRect(nX, nY);
		ClientRct.OffsetRect(nX, nY);
	}

	BITMAP BitMapInfo = { 0,0,0,0,0,0,0 };
	pOriginalDC->GetCurrentBitmap()->GetBitmap( &BitMapInfo );

	CDC dcAlt;
	dcAlt.CreateCompatibleDC(pOriginalDC);
	CBitmap bm;
	
	int nBitmapHeitgh = BoundsRct.Height();
	int nBitmapWidth = BoundsRct.Width();
	if (ClientRct.Width() > nBitmapWidth)
		nBitmapWidth = ClientRct.Width(); // this extends the bitmap past the righ-most column when needed
	
	bm.CreateCompatibleBitmap(pOriginalDC, nBitmapWidth, nBitmapHeitgh);
	HBITMAP hAltBitmap = HBITMAP(bm);
	HBITMAP hOldBitmap = HBITMAP(dcAlt.SelectObject(hAltBitmap));
	ASSERT(hOldBitmap);

	CDC* pAltDC = &dcAlt;

	ItemRct = SrcItemRct;
	pDC = pAltDC;
#endif

	CImageList *pImageList;

	pDC->FillSolidRect(ClientRct, ::GetSysColor(COLOR_WINDOW));
	pDC->SelectStockObject(BLACK_PEN); // Set default pen
	pDC->SelectStockObject(ANSI_VAR_FONT); // Choose the default 'skinny' font

	// Get item image and state info	
	LV_ITEM LVItem;
	LVItem.mask = LVIF_IMAGE | LVIF_STATE;	
	LVItem.iItem = nItemIdx;	
	LVItem.iSubItem = 0;
	LVItem.stateMask = 0xFFFF;		// get all state flags	
	GetItem(&LVItem);

	// Should the item be highlighted
	bool bHighlight =((LVItem.state & LVIS_DROPHILITED) || ((LVItem.state & LVIS_SELECTED)
		&& ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS))));

	// Get the main (column zero) text
	CString LabelStr = GetItemText(nItemIdx, 0);

	// Labels are nLabelOffset by a certain amount  
	// This nLabelOffset is related to the width of a space character
	m_nLabelOffset = (pDC->GetTextExtent(_T(" "), 1).cx) * 2;	

	CRect HighlightRct;

	switch(m_nHighlight)
	{
		case LVEX_HIGHLIGHT_ALLCOLUMNS:
		{
			HighlightRct = BoundsRct;		
			HighlightRct.left = LabelRct.left;		
		}
		break;	

		case LVEX_HIGHLIGHT_ROW:
		{
			HighlightRct = BoundsRct;
			HighlightRct.left = LabelRct.left;
			if (ClientRct.right > HighlightRct.right)
				HighlightRct.right = ClientRct.right;		
		}
		break;

		case LVEX_HIGHLIGHT_NORMAL:
		default:
		{
			int nTextWidth = pDC->GetOutputTextExtent(LabelStr).cx + m_nLabelOffset;
			HighlightRct = LabelRct;
			if (LabelRct.left + nTextWidth < LabelRct.right)
				HighlightRct.right = LabelRct.left + nTextWidth;
		}
		break;
	}	
	
	if (bHighlight)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));

		pDC->FillSolidRect(HighlightRct, ::GetSysColor(COLOR_HIGHLIGHT));
	}
	else
	{
		pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));	

		pDC->FillSolidRect(HighlightRct, ::GetSysColor(COLOR_WINDOW));

		if (m_bHorzGrid)
		{
			int nSavedDC = pDC->SaveDC();
			CPen gridpen(PS_SOLID, 1, RGB(192,192,192));
			pDC->SelectObject(&gridpen);
			pDC->MoveTo(0, ClientRct.bottom-1);
			pDC->LineTo(ClientRct.right, ClientRct.bottom-1);
			pDC->RestoreDC(nSavedDC);
		}
	}


	ColumnRct.right = ColumnRct.left + GetColumnWidth(0);	
	
	// Set clip region to the column
	CRgn ClipRgn;
	ClipRgn.CreateRectRgnIndirect(&ColumnRct);	
	pDC->SelectClipRgn(&ClipRgn);
	ClipRgn.DeleteObject();	
	
	// Draw state icon	
	if (LVItem.state & LVIS_STATEIMAGEMASK)
	{
		int nImageIdx = ((LVItem.state & LVIS_STATEIMAGEMASK)>>12) - 1;
		pImageList = GetImageList(LVSIL_STATE);		

		if (pImageList)
			pImageList->Draw(pDC, nImageIdx, CPoint(ColumnRct.left, ColumnRct.top), ILD_TRANSPARENT);
	}	

	// Draw normal and overlay icon	
	pImageList = GetImageList(LVSIL_SMALL);
	if (pImageList)
	{
		UINT nOvlImageMask = (LVItem.state & LVIS_OVERLAYMASK);
		UINT nStyle = ILD_TRANSPARENT | nOvlImageMask;

		if (bHighlight)
			nStyle |= ILD_BLEND50;
		else
			nStyle |= ILD_NORMAL;

		pImageList->Draw(pDC, LVItem.iImage, IconRct.TopLeft(), nStyle);
	}

	/*void CListCtrlEx::DrawItemLabel(CDC *pDC,
								CRect &LabelRct,
								CString &LabelStr,
								UINT nColAlignment,
								unsigned int nDispType)*/

	LV_COLUMN LVColumn;
	LVColumn.mask = LVCF_FMT | LVCF_WIDTH;
	GetColumn(0, &LVColumn);

	UINT nColAlign = (LVColumn.fmt & LVCFMT_JUSTIFYMASK);
	DrawItemLabel(pDC, LabelRct, LabelStr, nColAlign, 0);

	// Draw item label - Column 0	
//	LabelRct.left += (nLabelOffset / 2);
//	LabelRct.right -= nLabelOffset;
//	pDC->DrawText(LabelStr,-1, LabelRct, (DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS));	

	// Draw labels for remaining columns

	if ((bHighlight) && (m_nHighlight == LVEX_HIGHLIGHT_NORMAL))
	{
		// Change colors back to normal for remaining columns
		pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));	
	}	

	if (HighlightRct.right > BoundsRct.right)
		BoundsRct.right = HighlightRct.right;

	for (int nColumn = 1; GetColumn(nColumn, &LVColumn); nColumn++)
	{
		ColumnRct.left = ColumnRct.right;		
		ColumnRct.right += LVColumn.cx;

		ClipRgn.CreateRectRgnIndirect(&ColumnRct);
		pDC->SelectClipRgn(&ClipRgn);				   
		ClipRgn.DeleteObject();	

		// Draw the background if needed		
		if (m_nHighlight == LVEX_HIGHLIGHT_NORMAL)
			pDC->FillSolidRect(ColumnRct, ::GetSysColor(COLOR_HIGHLIGHT));

		LabelStr = GetItemText(nItemIdx, nColumn);

		if (LabelStr.GetLength() == 0)
			continue;		
	
		nColAlign = (LVColumn.fmt & LVCFMT_JUSTIFYMASK);
		DrawItemLabel(pDC, ColumnRct, LabelStr, nColAlign, nColumn);
	}

	pDC->SelectClipRgn(NULL);				   

	// Draw focus rectangle if item has focus
	if ((LVItem.state & LVIS_FOCUSED) && (GetFocus() == this))
		pDC->DrawFocusRect(HighlightRct);		

#ifdef USE_OFFSCREEN_BITMAP
	pOriginalDC->BitBlt(DestItemRct.left, DestItemRct.top,
			ClientRct.Width(), ClientRct.Height(),
			pAltDC,
			ClientRct.left, ClientRct.top,
			SRCCOPY);


	ASSERT(hOldBitmap);
	::SelectObject(dcAlt.GetSafeHdc(), hOldBitmap);
	hOldBitmap = NULL;
#endif

	// Restore dc	
	pOriginalDC->RestoreDC(nSavedDC);
}

// --------------------------------------------------------------------------

BOOL CListCtrlEx::OnEraseBkgnd(CDC *pDC) // WM_ERASEBKGND
{
	CRect EraseRct;
	GetClientRect(&EraseRct);
	int count = GetItemCount();

	if (count > 0)
	{
		CRect LastItemRct;
		GetItemRect((count-1), LastItemRct, LVIR_BOUNDS);

		if (LastItemRct.bottom >= EraseRct.bottom)
			return (true); // Nothing to erase, entire view has items

		EraseRct.top = LastItemRct.bottom;
	}
	else // count == 0
	{
		CHeaderCtrl *pHeader = (CHeaderCtrl*) this->GetDlgItem(0);
		ASSERT(pHeader);

		if (pHeader)
		{
			CRect HdrRct;
			pHeader->GetWindowRect(HdrRct);
			ScreenToClient(HdrRct);

			EraseRct.top = HdrRct.bottom;
		}
	}

	int nSavedDC = pDC->SaveDC();

	pDC->FillSolidRect(EraseRct, ::GetSysColor(COLOR_WINDOW));

	pDC->RestoreDC(nSavedDC);

	return true;
}

// --------------------------------------------------------------------------

void CListCtrlEx::OnPaint() // WM_PAINT 
{	
	// in full row select mode, we need to extend the clipping region
	// so we can paint a selection all the way to the right
	if (m_nHighlight == LVEX_HIGHLIGHT_ROW &&
		(GetStyle() & LVS_TYPEMASK) == LVS_REPORT )	{
		CRect rcBounds;
		GetItemRect(0, rcBounds, LVIR_BOUNDS);
		CRect rcClient;
		GetClientRect(&rcClient);
		if(rcBounds.right < rcClient.right)		{
			CPaintDC dc(this);
			CRect rcClip;
			dc.GetClipBox(rcClip);
			rcClip.left = __min(rcBounds.right-1, rcClip.left);
			rcClip.right = rcClient.right;
			InvalidateRect(rcClip, FALSE);
		}
	}

	CListCtrl::OnPaint();
}

// --------------------------------------------------------------------------

void CListCtrlEx::OnSetFocus(CWnd* pOldWnd) 
{
	CListCtrl::OnSetFocus(pOldWnd);
	
	// check if we are getting focus from label edit box
	if(pOldWnd!=NULL && pOldWnd->GetParent()==this)
		return;
	// repaint items that should change appearance
	if((GetStyle() & LVS_TYPEMASK)==LVS_REPORT)
		RepaintSelectedItems();
}

// --------------------------------------------------------------------------

void CListCtrlEx::OnKillFocus(CWnd* pNewWnd) 
{
	CListCtrl::OnKillFocus(pNewWnd);
	
	// check if we are losing focus to label edit box
	if(pNewWnd != NULL && pNewWnd->GetParent() == this)
		return;
	// repaint items that should change appearance
	if((GetStyle() & LVS_TYPEMASK) == LVS_REPORT)
		RepaintSelectedItems();
}

// --------------------------------------------------------------------------

void CListCtrlEx::RepaintSelectedItems()
{
	CRect rcBounds, rcLabel;

	// Invalidate focused item so it can repaint 
	int nItem = GetNextItem(-1, LVNI_FOCUSED);

	if(nItem != -1)
	{
		GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
		GetItemRect(nItem, rcLabel, LVIR_LABEL);
		rcBounds.left = rcLabel.left;

		InvalidateRect(rcBounds, FALSE);
	}

	// Invalidate selected items depending on LVS_SHOWSELALWAYS
	if(!(GetStyle() & LVS_SHOWSELALWAYS))
	{
		for(nItem = GetNextItem(-1, LVNI_SELECTED);
			nItem != -1; nItem = GetNextItem(nItem, LVNI_SELECTED))
		{
			GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
			GetItemRect(nItem, rcLabel, LVIR_LABEL);
			rcBounds.left = rcLabel.left;

			InvalidateRect(rcBounds, FALSE);
		}
	}

	UpdateWindow();
}

// --------------------------------------------------------------------------

unsigned int CListCtrlEx::GetColDisplayType(unsigned int nCol)
{
	if (m_DispTypeArrLen <= nCol)
		return (LVEX_DISPLAY_NORMAL);

	return (m_DispTypeArr[nCol]);
}

// --------------------------------------------------------------------------

unsigned int CListCtrlEx::SetColDisplayType(unsigned int nCol, unsigned int nType)
{
	if (m_DispTypeArrLen <= nCol)
	{
		unsigned int *pArr = new unsigned int [nCol + 1];

		for (unsigned int i=0; i<m_DispTypeArrLen; i++)
			pArr[i] = m_DispTypeArr[i];

		for (i=m_DispTypeArrLen; i <= nCol; i++)
			pArr[i] = LVEX_DISPLAY_NORMAL;

		delete[] m_DispTypeArr;
		m_DispTypeArr = pArr;
		m_DispTypeArrLen = (nCol + 1);
	}

	unsigned int nOldVal = m_DispTypeArr[nCol];
	m_DispTypeArr[nCol] = nType;
	return (nOldVal);
}

// --------------------------------------------------------------------------

bool CListCtrlEx::SetColImageList(unsigned int nCol, CImageList *pImageList)
{
	typedef CImageList *CImageListPtr;

	if (m_DispImageListArrLen <= nCol)
	{
		CImageListPtr *pArr = new CImageListPtr[nCol + 1];

		for (unsigned int i=0; i<m_DispImageListArrLen; i++)
			pArr[i] = m_DispImageListArr[i];

		for (i=m_DispImageListArrLen; i <= nCol; i++)
			pArr[i] = NULL;

		delete[] m_DispImageListArr;
		m_DispImageListArr = pArr;
		m_DispImageListArrLen = (nCol + 1);
	}

	if (m_DispImageListArr[nCol])
	{
		delete m_DispImageListArr[nCol];
		m_DispImageListArr[nCol] = NULL;
	}

	m_DispImageListArr[nCol] = pImageList;

	GetColImageWidth(nCol); // Force the width into our cache array, also updates the min row height

	SetColDisplayType(nCol, LVEX_DISPLAY_IMAGELIST);

	return (true);
}

// --------------------------------------------------------------------------

CImageList *CListCtrlEx::GetColImageList(unsigned int nCol)
{
	if (m_DispImageListArrLen <= nCol)
		return (NULL);

	return (m_DispImageListArr[nCol]);
}

// --------------------------------------------------------------------------

unsigned int CListCtrlEx::GetColImageWidth(unsigned int nCol)
{
	if (m_ImageWidthArrLen <= nCol)
	{
		CImageList *pImageList = GetColImageList(nCol);
		if (!pImageList)
			return (0);

		IMAGEINFO ImageInfo = { 0,0,0,0,0 };
		pImageList->GetImageInfo(0, &ImageInfo);

		SetColImageWidth(nCol, (ImageInfo.rcImage.right - ImageInfo.rcImage.left));
		SetColImageHeight(nCol, (ImageInfo.rcImage.bottom - ImageInfo.rcImage.top));
	}

	return (m_ImageWidthArr[nCol]);
}

// --------------------------------------------------------------------------

void CListCtrlEx::SetColImageWidth(unsigned int nCol, unsigned int nWidth)
{
	if (m_ImageWidthArrLen <= nCol)
	{
		unsigned int *pArr = new unsigned int[nCol + 1];

		for (unsigned int i=0; i<m_ImageWidthArrLen; i++)
			pArr[i] = m_ImageWidthArr[i];

		for (i=m_ImageWidthArrLen; i <= nCol; i++)
			pArr[i] = 0;

		delete[] m_ImageWidthArr;
		m_ImageWidthArr = pArr;
		m_ImageWidthArrLen = (nCol + 1);
	}

	m_ImageWidthArr[nCol] = nWidth;
}

// --------------------------------------------------------------------------

unsigned int CListCtrlEx::GetColImageHeight(unsigned int nCol)
{
	if (m_ImageHeightArrLen <= nCol)
	{
		CImageList *pImageList = GetColImageList(nCol);
		if (!pImageList)
			return (0);

		IMAGEINFO ImageInfo = { 0,0,0,0,0 };
		pImageList->GetImageInfo(0, &ImageInfo);

		SetColImageWidth(nCol, (ImageInfo.rcImage.right - ImageInfo.rcImage.left));
		SetColImageHeight(nCol, (ImageInfo.rcImage.bottom - ImageInfo.rcImage.top));
	}

	return (m_ImageHeightArr[nCol]);
}

// --------------------------------------------------------------------------

void CListCtrlEx::SetColImageHeight(unsigned int nCol, unsigned int nHeight)
{
	if (m_ImageHeightArrLen <= nCol)
	{
		unsigned int *pArr = new unsigned int[nCol + 1];

		for (unsigned int i=0; i<m_ImageHeightArrLen; i++)
			pArr[i] = m_ImageHeightArr[i];

		for (i=m_ImageHeightArrLen; i <= nCol; i++)
			pArr[i] = 0;

		delete[] m_ImageHeightArr;
		m_ImageHeightArr = pArr;
		m_ImageHeightArrLen = (nCol + 1);
	}

	if (nHeight > m_MinRowHeight)
	{
		m_MinRowHeight = nHeight;
		OnSetFont(0,0);
	}

	m_ImageHeightArr[nCol] = nHeight;
}

// --------------------------------------------------------------------------

BOOL CListCtrlEx::SetColumnAlignment(unsigned int nCol, int fmt)
{
	LV_COLUMN LVColumn;
	LVColumn.mask = LVCF_FMT;
	LVColumn.fmt = fmt;

	return SetColumn(nCol, &LVColumn);
}

// --------------------------------------------------------------------------

LRESULT CListCtrlEx::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	// When the font changes we want to update the display with the new font
	// We also need to make sure the row height is recalculated
	// This is best done by simualting a position change message.

//	HFONT hFont = (HFONT) wParam;
//	BOOL fRedraw = (BOOL) LOWORD(lParam);

	LRESULT res = Default();

	CRect rc;
	GetWindowRect( &rc );

	WINDOWPOS wp;
	wp.hwnd = m_hWnd;
	wp.cx = rc.Width();
	wp.cy = rc.Height();
	wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;

	// This causes MeasureItem to be called
	SendMessage( WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp );

	return res;
}

// --------------------------------------------------------------------------

void CListCtrlEx::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CDC* pDC = GetDC(); // Temporary DC
	TEXTMETRIC tm;

	VERIFY(pDC->GetTextMetrics((LPTEXTMETRIC )&tm));
	ASSERT(tm.tmHeight > 0);

	UINT nHeight = (UINT) tm.tmAscent;
	m_GraphHeight = nHeight; // Graph height is same as text height
	nHeight *= m_nLinePerRow;
	nHeight++; // Add one so the text doesn't go all the way to the bottom

	if (nHeight < m_MinRowHeight)
		nHeight = m_MinRowHeight;

	lpMeasureItemStruct->itemHeight = nHeight;
}

// --------------------------------------------------------------------------

BOOL CListCtrlEx::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	HD_NOTIFY *pHDN = (HD_NOTIFY *)lParam;
	const unsigned int nMinWidth = GetColImageWidth(pHDN->iItem) + 6;

	// Force a minimum column width. Never less than the width of the images
	if( ((pHDN->hdr.code == HDN_ITEMCHANGINGW) || (pHDN->hdr.code == HDN_ITEMCHANGINGA))
		&& (((unsigned int)pHDN->pitem->cxy) < (nMinWidth)))
	{
		pHDN->pitem->cxy = nMinWidth;
	}

	return CListCtrl::OnNotify(wParam, lParam, pResult);
}

// --------------------------------------------------------------------------

void CListCtrlEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	// ***** THIS IS BROKEN! CAN'T FIND FIX! *****

	// When you have full row selection on (LVEX_HIGHLIGHT_ROW), you want to have left clicks
	// which are to the right of last column to be handled as normal. Unfortunately I
	// can't manipulate the msg to get it to work.
	//
	// We don't want to handle selecting the item ourselves because that would lose the
	// expected modifier behavior (ctrl, shift, etc).
	//
	// So, we punt, which means with row-selection on you cannot left click to the right
	// of last column. Oh well.

//	int nRetVal = 0;
//	// When we LVEX_HIGHLIGHT_ROW, we want to capture clicks to the right of the last
//	// column and fake them out to be within the column.
//	if ((m_nHighlight == LVEX_HIGHLIGHT_ROW) && ((nRetVal = HitTest(point)) != (-1)))
//	{
//		// Cast away the const, so we can modify the message before it leaves here
//		MSG *pMSG = (MSG *) GetCurrentMessage();
//
//		// pMSG->lParam (CLIENT coords)
//		// pMSG->pt     (SCREEN coords)
//
//		// The lParam is the point wheer you click. The high word (0xFFFF0000) is the y, the low word (0x0000FFFF) is the x
//		// Force the x value to two (less than two doesn't work)
//		pMSG->lParam &= 0xFFFF0002;
//
//		CPoint tmpPt(((pMSG->lParam) & 0x0000FFFF), (((pMSG->lParam) & 0xFFFF0000) >> 16));
//		ClientToScreen(&tmpPt);
//		pMSG->pt.x = tmpPt.x;
//	}

	Default(); // Conintue processing the msg

	// Make sure we have the focus
	if (GetFocus() != this)
		SetFocus();
}

// --------------------------------------------------------------------------

void CListCtrlEx::OnLButtonDblClk(UINT nFlags, CPoint point) // ON_WM_LBUTTONDBLCLK()
{
	const int nIdx = HitTestEx(point);

	if (nIdx != (-1))
	{
		SetCurSel(nIdx, true); // Double-click removes other selection

		// Make sure we have the focus. To reduce flicker this should be done after we change
		// any focus/selection, but before we notify the owner.
		if (GetFocus() != this)
			SetFocus();
		
		// Because the owner could possibly steal the focus, we want to make sure
		// we indicate to the user any selection change.
		UpdateWindow();

		CWnd *pOwner = GetOwner();

		if (pOwner)
		{
			ClientToScreen(&point);
			pOwner->SendMessage(msgListCtrlEx_LBtnDblClk, (WPARAM) nFlags, MAKELPARAM(point.x, point.y));
		}
	}
}

// --------------------------------------------------------------------------

void CListCtrlEx::OnRButtonDown(UINT nFlags, CPoint point)
{
	const int nIdx = HitTestEx(point);

	if (nIdx != (-1))
	{
		// We want to be able to right-click on a block of already selected
		// items (multiple).

		// Check if this item is selected
		if (GetItemState(nIdx, LVIS_SELECTED) != LVIS_SELECTED)
			SetCurSel(nIdx, true); // Not selected, so deselect everything else and select only this item

		// Change focus no matter what
		SetItemState(nIdx, LVIS_FOCUSED, LVIS_FOCUSED);

		// Make sure we have the focus. To reduce flicker this should be done after we change
		// any focus/selection, but before we notify the owner.
		if (GetFocus() != this)
			SetFocus();

		// Because the owner could possibly steal the focus, we want to make sure
		// we indicate to the user any selection change.
		UpdateWindow();

		// Let the owner do its thang
		CWnd *pOwner = GetOwner();
		if (pOwner)
		{
			ClientToScreen(&point);
			pOwner->SendMessage(msgListCtrlEx_RBtn, (WPARAM) nFlags, MAKELPARAM(point.x, point.y));
		}
	}
	else
		Default();
}

// --------------------------------------------------------------------------

void CListCtrlEx::SetCurSel(int nIdx, bool bFocus /* = true */)
{
	// Clear all selected items
	for (int i = GetNextItem( -1, LVNI_ALL | LVNI_SELECTED); i != -1; i = GetNextItem( i, LVNI_ALL | LVNI_SELECTED))
	{
		SetItemState(i, ~((UINT)LVIS_SELECTED), (UINT)LVIS_SELECTED);
	}

	// Select the item in question
	SetItemState(nIdx, LVIS_SELECTED | (bFocus?LVIS_FOCUSED:0), LVIS_SELECTED | (bFocus?LVIS_FOCUSED:0));
}

// --------------------------------------------------------------------------

void CListCtrlEx::SetGraphBorderColor(COLORREF crColor)
{
	m_GraphBorderColor = crColor;
}

// --------------------------------------------------------------------------

void CListCtrlEx::SetGraphCompletedColor(COLORREF crColor)
{
	m_GraphCompletedColor = crColor;
}

// --------------------------------------------------------------------------

void CListCtrlEx::SetGraphRemainColor(COLORREF crColor)
{
	m_GraphRemainColor = crColor;
}

// --------------------------------------------------------------------------

void CListCtrlEx::SetGraphUseBorder(bool bBorder)
{
	m_GraphUseBorder = bBorder;
}

// --------------------------------------------------------------------------

void CListCtrlEx::SetLinePerRow(int nLines)
{
	if (nLines > 0)
		m_nLinePerRow = nLines;
}

// --------------------------------------------------------------------------

void CListCtrlEx::SetHorizGrid(bool bGrid)
{
	m_bHorzGrid = bGrid;
}

// --------------------------------------------------------------------------

void CListCtrlEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_DELETE)
	{
		CWnd *pOwner = GetOwner();

		if (pOwner)
		{
			pOwner->SendMessage(msgListCtrlEx_DeleteKey, (WPARAM) nRepCnt, (LPARAM) this); // DELETE
		}
	}

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

// --------------------------------------------------------------------------

bool CListCtrlEx::SetEatReturnKey(bool bEat)
{
	const bool bOldVal = m_bEatReturnKey;
	m_bEatReturnKey = bEat;
	
	return (bOldVal);
}

// --------------------------------------------------------------------------

BOOL CListCtrlEx::PreTranslateMessage(MSG* pMsg) 
{
	if ((WM_KEYDOWN == pMsg->message) && (VK_RETURN == pMsg->wParam))
	{
		CWnd *pOwner = GetOwner();

		if (pOwner)
		{
			UINT nRepCnt = ((pMsg->lParam) & 0xFF);
			pOwner->SendMessage(msgListCtrlEx_ReturnKey, (WPARAM) nRepCnt, (LPARAM) this); // RETURN

			if (m_bEatReturnKey)
				return (TRUE); //
		}
	}

	return CListCtrl::PreTranslateMessage(pMsg);
}


bool CListCtrlEx::GetCurSel(list<int> *pSelList)
{
	ASSERT(pSelList);
	if (!pSelList)
		return (false); // false indicates error

	pSelList->erase(pSelList->begin(), pSelList->end()); // Erase the list

	int nItem = (-1);

	while ( (nItem = GetNextItem(nItem, LVNI_SELECTED)) != (-1) )
	{
		pSelList->push_back(nItem);
	}

	return (true); // true means the list contains the selected items (may be empty)
}


bool CListCtrlEx::DeleteItems(const list<int>& InList)
{
	list<int> DelList = InList;
	DelList.sort();

	int nOffset = 0;
	int nItem;
	list<int>::iterator iList;

	for (iList = DelList.begin(); iList != DelList.end(); iList++)
	{
		nItem = (*iList);
		if (CListCtrl::DeleteItem(nItem - nOffset) == FALSE)
			return (false);
		nOffset++;
	}

	return (true);
}

void CListCtrlEx::InsertArr(unsigned int *& pArr, unsigned int &nLen, unsigned int nDefault, unsigned int nIdx)
{
	if (pArr)
	{
		unsigned int *pTemp = new unsigned int [nLen + 1];
		ASSERT(pTemp);

		for (unsigned int i=0; i<nIdx; i++)
			pTemp[i] = pArr[i];

		pTemp[nIdx] = nDefault;

		for (i=nIdx; i < nLen; i++)
			pTemp[i+1] = pArr[i];

		delete[] pArr;
		pArr = pTemp;
		nLen++;
	}
}

void CListCtrlEx::DeleteArr(unsigned int *& pArr, unsigned int &nLen, unsigned int nIdx)
{
	if (pArr)
	{
		unsigned int *pTemp = new unsigned int [nLen - 1];
		ASSERT(pTemp);

		for (unsigned int i=0; i<nIdx; i++)
			pTemp[i] = pArr[i];

		for (i=nIdx + 1; i < nLen; i++)
			pTemp[i-1] = pArr[i];

		delete[] pArr;
		pArr = pTemp;
		nLen--;
	}
}

void CListCtrlEx::NotifyInsertedCol(unsigned int nCol)
{
	InsertArr(m_DispTypeArr, m_DispTypeArrLen, LVEX_DISPLAY_NORMAL, nCol);
	InsertArr(m_ImageHeightArr, m_ImageHeightArrLen, 0, nCol);
	InsertArr(m_ImageWidthArr, m_ImageWidthArrLen, 0, nCol);

	if (m_DispImageListArr)
	{
		CImageList **pTemp = new CImageList *[m_DispImageListArrLen + 1];
		ASSERT(pTemp);

		for (unsigned int i=0; i<nCol; i++)
			pTemp[i] = m_DispImageListArr[i];

		pTemp[nCol] = NULL;

		for (i=nCol; i < m_DispImageListArrLen; i++)
			pTemp[i+1] = m_DispImageListArr[i];

		delete[] m_DispImageListArr;
		m_DispImageListArr = pTemp;
		m_DispImageListArrLen++;
	}
}

void CListCtrlEx::NotifyDeletedCol(unsigned int nCol)
{
	DeleteArr(m_DispTypeArr, m_DispTypeArrLen, nCol);
	DeleteArr(m_ImageHeightArr, m_ImageHeightArrLen, nCol);
	DeleteArr(m_ImageWidthArr, m_ImageWidthArrLen, nCol);

	if (m_DispImageListArr)
	{
		CImageList **pTemp = new CImageList *[m_DispImageListArrLen - 1];
		ASSERT(pTemp);

		for (unsigned int i=0; i<nCol; i++)
			pTemp[i] = m_DispImageListArr[i];

		for (i=nCol + 1; i < m_DispImageListArrLen; i++)
			pTemp[i-1] = m_DispImageListArr[i];

		delete[] m_DispImageListArr;
		m_DispImageListArr = pTemp;
		m_DispImageListArrLen--;
	}
}

BOOL CListCtrlEx::DeleteItem(int nItem)
{
	if ((m_nHighlight == LVEX_HIGHLIGHT_ROW) && (nItem < GetItemCount()))
	{
		// When the selection is full row, we have to make sure
		// we invalidate the full row when deleting

		CRect BoundsRct, ClientRct;
		GetItemRect(nItem, BoundsRct, LVIR_BOUNDS);
		GetClientRect(&ClientRct);
		ClientRct.top = BoundsRct.top;
		ClientRct.bottom = BoundsRct.bottom;

		InvalidateRect(ClientRct);
	}

	return (CListCtrl::DeleteItem(nItem));
}

BOOL CListCtrlEx::DeleteAllItems()
{
	if ((m_nHighlight == LVEX_HIGHLIGHT_ROW) && (GetItemCount() > 0))
	{
		// When the selection is full row, we have to make sure
		// we invalidate the full row when deleting

		CRect ClientRct;
		GetClientRect(&ClientRct);

		CRect FirstItemRct;
		GetItemRect(0, FirstItemRct, LVIR_BOUNDS);

		if (FirstItemRct.top > ClientRct.top)
			ClientRct.top = FirstItemRct.top;

		CRect LastItemRct;
		GetItemRect((GetItemCount()-1), LastItemRct, LVIR_BOUNDS);

		if (LastItemRct.bottom < ClientRct.bottom)
			ClientRct.bottom = LastItemRct.bottom;

		// Invalidate from the top of the first visible item to the
		// bottom of the last visible item.
		InvalidateRect(ClientRct);
	}

	return CListCtrl::DeleteAllItems();
}

void CListCtrlEx::OnHeaderClicked(NMHDR *pNMHDR, LRESULT *pResult)
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;

	if (phdn->iButton == 0)
	{
		// User clicked on header using left mouse button
		if( phdn->iItem == nSortedCol )
			bSortAscending = !bSortAscending;
		else
			bSortAscending = TRUE;

		nSortedCol = phdn->iItem;

		m_SortInfo.nCol = nSortedCol;
		m_SortInfo.bAscending = bSortAscending;

		CListCtrl::SortItems(CListCtrlEx::CompareCB, (DWORD)this);
	}

	*pResult = 0;
}

void CListCtrlEx::ResetSortColumn()
{
	nSortedCol = (-1);
	bSortAscending = true;
}

// --------------------------------------------------------------------------

// -1 : nItem1 < nItem2
//  0 : nItem1 == nItem2
// +1 : nItem1 >  nItem2
int CListCtrlEx::CompareItems(LPARAM lpOne, LPARAM lpTwo, int nCol) // virtual
{
	// Default to case-insensitive text compare (I know this is slow, but, hey, it's FREE!)

	LV_FINDINFO FindInfo = { LVFI_PARAM, NULL, NULL };

	// We're passed the data of the item, not the index, so go searching...
	FindInfo.lParam = lpOne;
	const int nIdx1 = CListCtrl::FindItem(&FindInfo);

	FindInfo.lParam = lpTwo;
	const int nIdx2 = CListCtrl::FindItem(&FindInfo);

	// An optimization! I'm da man...
	if (nIdx1 == nIdx2)
		return (0);

	// Grab the string of the appropriate column for each item
	CString sItem1 = GetItemText(nIdx1, nCol);
	CString sItem2 = GetItemText(nIdx2, nCol);

	// Compare
	return sItem1.CompareNoCase(sItem2);
}

// --------------------------------------------------------------------------

/* static */ int CALLBACK CListCtrlEx::CompareCB(LPARAM lpOne, LPARAM lpTwo, LPARAM lpArg)
{
	CListCtrlEx *pList = (CListCtrlEx *) lpArg;

	ASSERT(pList);
	if (!pList)
		return (0);
	
	if (false == pList->m_SortInfo.bAscending)
		return (-(pList->CompareItems(lpOne, lpTwo, pList->m_SortInfo.nCol)));
	
	return (pList->CompareItems(lpOne, lpTwo, pList->m_SortInfo.nCol));
}

// --------------------------------------------------------------------------

//
// HitTestEx [PROTECTED]
//
// Returns the index of the item at a given point (client coords). This fn
// takes into account the selection style (normal, all column, row).
//
// Return (-1) indicates the click was not at any item.
//
int CListCtrlEx::HitTestEx(CPoint point)
{
	UINT uFlags = 0;

	// HitTest seems to always work, regardless of horz position
	int nIdx = HitTest(point, &uFlags);

	switch (m_nHighlight)
	{
		case LVEX_HIGHLIGHT_NORMAL:
		{
			// For normal you have to click on the actual item
			if (!(uFlags & LVHT_ONITEM))
				nIdx = (-1);
		}
		break;

		case LVEX_HIGHLIGHT_ALLCOLUMNS:
		{
			// We want to make sure you have clicked within the columns (not to the right)
			// so we have to find the right-most column's right-side coord. There is
			// no easy way to get this info. We must loop through all cols... yuck.
			LV_COLUMN LVColumn;
			LVColumn.mask = LVCF_FMT | LVCF_WIDTH;
			int nMaxX = 0;
			for (int nColumn = 0; (point.x > nMaxX) && (GetColumn(nColumn, &LVColumn)); nColumn++)
				nMaxX += LVColumn.cx;

			// If click is beyond right column, then return invalid index
			if (point.x > nMaxX)
				nIdx = (-1);
		}
		break;

		case LVEX_HIGHLIGHT_ROW:
		{
			// Nothing to do
		}
		break;

		default:
			ASSERT(0);
	}

	return (nIdx);
}
