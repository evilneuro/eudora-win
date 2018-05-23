////////////////////////////////////////////////////////////////////////
// Filename:  TBarSendButton.cpp
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "rs.h"

#include "TBarSendButton.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_BUTTON(TBarSendButton)


////////////////////////////////////////////////////////////////////////
// AdjustSize [public, virtual]
//
// Override of SECStdBtn::AdjustSize() implemenation.
// Setting the toolbar's 'm_nMaxBtnWidth' here is critical for assuring
// that the button DC's are setup with the proper dimensions.
////////////////////////////////////////////////////////////////////////
void TBarSendButton::AdjustSize()
{
	SECStdBtn::AdjustSize();

	int nSendQueueBtnWidth = m_pToolBar->GetStdBtnWidth() + 10;
	{
		// 
		// Using a temp DC for the host toolbar, select the font to
		// be used, then measure the size of the Send and Queue
		// keywords (locale dependent).  Auto-size this button to account
		// for the worst case width of the Send/Queue keywords.
		//
		CFont* pTempFont = CFont::FromHandle(HFONT(::GetStockObject(ANSI_VAR_FONT)));
		ASSERT(pTempFont);
		CDC* pTempDC = m_pToolBar->GetDC();
		CFont* pOldFont = pTempDC->SelectObject(pTempFont);

		if (pOldFont)
		{
			CRString strSend(IDS_SEND);
			CRString strQueue(IDS_QUEUE);

			CSize sizeSend(pTempDC->GetTextExtent(strSend, strSend.GetLength()));
			CSize sizeQueue(pTempDC->GetTextExtent(strQueue, strQueue.GetLength()));

			nSendQueueBtnWidth += max(sizeSend.cx, sizeQueue.cx);

			pTempDC->SelectObject(pOldFont);
		}
		m_pToolBar->ReleaseDC( pTempDC );
	}

	if (m_pToolBar)
		m_pToolBar->m_nMaxBtnWidth = max(m_pToolBar->m_nMaxBtnWidth, nSendQueueBtnWidth);

	m_cx = nSendQueueBtnWidth;
}


////////////////////////////////////////////////////////////////////////
// DrawFace [protected, virtual]
//
// Override of SECStdBtn::DrawFace() implemenation.  Handles the details
// of drawing the face of the wider-than-average "Send/Queue" button.
////////////////////////////////////////////////////////////////////////
void TBarSendButton::DrawFace(SECBtnDrawData& data, BOOL bForce, int& x, int& y, 
						 int& nWidth, int& nHeight, int nImgWidth)
{
	extern SEC_AUX_DATA secData;

	// Draw the face of the button to m_drawDC (bForce causes the button to
	// be drawn up and enabled).

	if(nImgWidth == -1)
		nImgWidth = m_pToolBar->GetImgWidth();

	// Calc. the position of the bitmap
	int xImg = (m_pToolBar->GetStdBtnWidth() - nImgWidth - 1)/2 + x;
	int yImg = (nHeight - m_pToolBar->GetImgHeight())/2 + y;

	BOOL bCool = m_pToolBar->CoolLookEnabled();

	// Lets start with filling the background
	data.m_drawDC.FillSolidRect(x, y, nWidth, nHeight, secData.clrBtnFace);

	// Draw the borders around the edge of the button
	if(m_nStyle & (TBBS_PRESSED | TBBS_CHECKED) && !bForce)
	{
		// Button is down
		data.m_drawDC.Draw3dRect(x, y, nWidth, nHeight, 
								 bCool ? secData.clrBtnShadow : 
								 		 secData.clrWindowFrame,
								 secData.clrBtnHilite);
		x++; y++; nWidth -= 2; nHeight -= 2;
		if(!bCool)
		{
			data.m_drawDC.Draw3dRect(x, y, nWidth, nHeight, 
									 secData.clrBtnShadow,
									 secData.clrBtnFace);
		}

		x++; y++; nWidth -= 2; nHeight -= 2;

		// Offset image to give impression of movement.
		xImg++; yImg++;
	}
	else if(bCool && (m_nStyle & SEC_TBBS_RAISED) && !bForce)
	{
		data.m_drawDC.Draw3dRect(x, y, nWidth, nHeight, secData.clrBtnHilite,
					 			 secData.clrBtnShadow);
		x += 2; y += 2; nWidth -= 4; nHeight -= 4;
	}
	else if(bCool)
	{
		// Button is up
		x += 2; y += 2; nWidth -= 4; nHeight -= 4;
	}
	else
	{
		// Button is up
		data.m_drawDC.Draw3dRect(x, y, nWidth, nHeight, secData.clrBtnHilite, 
								 secData.clrWindowFrame);
		x++; y++; nWidth -= 2; nHeight -= 2;
		data.m_drawDC.Draw3dRect(x, y, nWidth, nHeight, secData.clrBtnFace, 
								 secData.clrBtnShadow);
		x++; y++; nWidth -= 2; nHeight -= 2;
	}

	//
	// Set Send or Queue bitmap per "Immediate Send" setting.  Christie
	// will hate me, but it is assumed that the toolbar bitmaps for
	// the Send and Queue images are adjacent -- Send first, Queue
	// second.
	//
	int nRealImage = m_nImage;
	if (! GetIniShort(IDS_INI_IMMEDIATE_SEND))
		nRealImage++;
	
	// Now draw the button image
	data.m_drawDC.BitBlt(xImg, yImg, nImgWidth, m_pToolBar->GetImgHeight(), 
						 &data.m_bmpDC, nRealImage * m_pToolBar->GetImgWidth(), 
						 0, SRCCOPY);

	//
	// Finally, draw the Send or Queue keyword on the button face.
	//
	{
		CFont* pTempFont = CFont::FromHandle(HFONT(::GetStockObject(ANSI_VAR_FONT)));
		ASSERT(pTempFont);
		COLORREF rgbOldBgnd = data.m_drawDC.SetBkColor(secData.clrBtnFace);
		COLORREF rgbOldText = data.m_drawDC.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
		CFont* pOldFont = data.m_drawDC.SelectObject(pTempFont);

		if (pOldFont)
		{
			CRString strLabel(GetIniShort(IDS_INI_IMMEDIATE_SEND) ? IDS_SEND : IDS_QUEUE);
//FORNOW			data.m_drawDC.TextOut(x + nImgWidth + 3, y + 3, strLabel);
			data.m_drawDC.DrawText(strLabel, CRect(x+nImgWidth, y, nWidth, nHeight), 
									DT_CENTER | DT_NOCLIP | DT_EXTERNALLEADING | DT_SINGLELINE | DT_VCENTER);
			data.m_drawDC.SelectObject(pOldFont);
		}

		data.m_drawDC.SetTextColor(rgbOldText);
		data.m_drawDC.SetBkColor(rgbOldBgnd);
	}
}
