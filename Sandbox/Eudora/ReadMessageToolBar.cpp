// ReadMessageToolBar.cpp: implementation of the CReadMessageToolBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eudora.h"
#include "ReadMessageToolBar.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CReadMessageToolBar::CReadMessageToolBar()
{

}

CReadMessageToolBar::~CReadMessageToolBar()
{

}

BEGIN_MESSAGE_MAP(CReadMessageToolBar, QCChildToolBar)
	ON_WM_SIZE()
END_MESSAGE_MAP()



// Override OnSize if you want a different form of
// resize handling and don't set CBRS_STRETCH_ON_SIZE
void CReadMessageToolBar::OnSize(UINT nType, int cx, int cy) 
{
	CEdit*		pEdit;
	CRect		theRect;
	INT			iNewWidth;
	INT			iNewYPos;
	CStatic*	pStatic;

	if( nType != SIZE_MINIMIZED )
	{
		pEdit = ( CEdit* ) GetDlgItem( IDC_SUBJECT );

		if( pEdit != NULL )
		{
			pEdit->GetWindowRect( &theRect );
			ScreenToClient( &theRect );
			
			iNewWidth = cx - theRect.left - 1;
			iNewYPos = ( m_szTbarDockHorz.cy - theRect.Height() ) / 2;
			
			if( iNewYPos < 0 )
			{
				iNewYPos = 0;
			}

			if( ( iNewWidth > 5 ) || ( theRect.top != iNewYPos ) )
			{
				if ( iNewWidth <= 5 )
				{
					iNewWidth = theRect.Width();
				}

				pEdit->SetWindowPos(	NULL, 
										theRect.left, 
										iNewYPos, 
										iNewWidth, 
										theRect.Height(),			
										SWP_NOACTIVATE | SWP_NOZORDER );
			}
		}

		pStatic = ( CStatic* ) GetDlgItem( ID_SUBJECT_STATIC );
		
		if( pStatic != NULL )
		{
			pStatic->GetWindowRect( &theRect );
			ScreenToClient( &theRect );

			iNewYPos = ( m_szTbarDockHorz.cy - theRect.Height() ) / 2;

			if( iNewYPos < 0 )
			{
				iNewYPos = 0;
			}

			if( theRect.top != iNewYPos ) 
			{
				pStatic->SetWindowPos(	NULL, 
										theRect.left, 
										iNewYPos, 
										theRect.Width(), 
										theRect.Height(),			
										SWP_NOACTIVATE | SWP_NOZORDER );
			}
		}
	}

	m_szTbarDockHorz.cx = cx;
	cy = m_szTbarDockHorz.cy;
	QCChildToolBar::OnSize(nType, cx, cy);
}


