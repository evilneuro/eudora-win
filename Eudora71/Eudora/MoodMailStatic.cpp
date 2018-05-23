// MoodMailStatic.cpp : implementation file
//

#include "stdafx.h"
#include "eudoraexe.h"
#include "MoodMailStatic.h"
#include "resource.h"
#include "rs.h"
#include "QCImageList.h"
#include "DebugNewHelpers.h"


IMPLEMENT_BUTTON(CMoodMailStatic)
/////////////////////////////////////////////////////////////////////////////
// CMoodMailStatic

CMoodMailStatic::CMoodMailStatic()
{
//	m_listMoodImages.Create(IDB_IL_MOOD_MAIL, 16, 0, RGB(192, 192, 192)); 
	m_nScore = 0;
}

BOOL CMoodMailStatic::AddImages()
{
	return TRUE;
}

CMoodMailStatic::~CMoodMailStatic()
{
}

void CMoodMailStatic::AdjustSize()
{
	SECStdBtn::AdjustSize();

}
void CMoodMailStatic::SetScore(int nScore)
{
	m_nScore = nScore;
}

int CMoodMailStatic::GetScore()
{
	return m_nScore;
}

////////////////////////////////////////////////////////////////////////
// DrawFace [protected, virtual]
//
// Override of SECStdBtn::DrawFace() implemenation.  Handles the details
// of drawing the face of the wider-than-average "Send/Queue" button.
////////////////////////////////////////////////////////////////////////
void CMoodMailStatic::DrawFace(SECBtnDrawData& data, BOOL bForce, int& x, int& y, 
						 int& nWidth, int& nHeight, int nImgWidth)
{
/*	static BOOL bImagesAdded = FALSE;
	if (!bImagesAdded)
	{
		AddImages();
		bImagesAdded = TRUE;
	}

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
	}*/

	//
	// Set Send or Queue bitmap per "Immediate Send" setting.  Christie
	// will hate me, but it is assumed that the toolbar bitmaps for
	// the Send and Queue images are adjacent -- Send first, Queue
	// second.
	//
	/*int nRealImage = m_nImage;
	if (! GetIniShort(IDS_INI_IMMEDIATE_SEND))
		nRealImage++;
	
	// Now draw the button image
	data.m_drawDC.BitBlt(xImg, yImg, nImgWidth, m_pToolBar->GetImgHeight(), 
						 &data.m_bmpDC, m_nImage * m_pToolBar->GetImgWidth(), 
					 0, SRCCOPY);*/
	//m_listMoodImages.Draw(&data.m_drawDC, m_nScore, CPoint(8,4), ILD_NORMAL);
		// Button is up
	// Lets start with filling the background
	extern SEC_AUX_DATA secData;
	data.m_drawDC.FillSolidRect(x, y, nWidth, nHeight, secData.clrBtnFace);
	if(GetIniShort( IDS_INI_MOOD_MAIL_CHECK ))
		g_theMoodImageList.Draw(m_nScore, 5, 4, &data.m_drawDC, RGB(192, 192, 192));
}
