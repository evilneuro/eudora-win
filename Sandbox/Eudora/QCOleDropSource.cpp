/**********************************************************************/
/*                         QCOleDropSource                            */
/**********************************************************************/

#include "stdafx.h"

#include "QCOleDropSource.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


////////////////////////////////////////////////////////////////////////
// QCOleDropSource [public, constructor]
//
////////////////////////////////////////////////////////////////////////
QCOleDropSource::QCOleDropSource(BOOL bWantTrackerGhost /*=FALSE*/) :
	m_bWantTrackerGhost(bWantTrackerGhost)
{
	m_rectLast.SetRectEmpty();
}


////////////////////////////////////////////////////////////////////////
// ~QCOleDropSource [public, virtual, destructor]
//
////////////////////////////////////////////////////////////////////////
QCOleDropSource::~QCOleDropSource()
{
}


////////////////////////////////////////////////////////////////////////
// GiveFeedback [public, virtual]
//
////////////////////////////////////////////////////////////////////////
SCODE QCOleDropSource::GiveFeedback(DROPEFFECT dropEffect)
{
	if (m_bWantTrackerGhost)
	{
		if (DROPEFFECT_NONE == dropEffect)
			EraseGhostRect();
		else
			DrawGhostRect();
	}

	if ((dropEffect & DROPEFFECT_LINK) == DROPEFFECT_LINK)
	{
		//
		// We're not really performing a "link" operation here.  By
		// convention, we use DROPEFFECT_LINK to indicate that the
		// drop target is capable of "inserting" the dragged wazoo at
		// the given tab location.  Therefore, we override the default
		// "link" cursor with a different standard cursor.
		//
		::SetCursor(::LoadCursor(NULL, IDC_UPARROW));
		return S_OK;
	}

	return COleDropSource::GiveFeedback(dropEffect);
}


////////////////////////////////////////////////////////////////////////
// QueryContinueDrag [public, virtual]
//
////////////////////////////////////////////////////////////////////////
SCODE QCOleDropSource::QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState)
{
	if (m_bWantTrackerGhost)
	{
		if (bEscapePressed || (dwKeyState | MK_RBUTTON) || (0 == (dwKeyState | MK_LBUTTON)))
		{
			//
			// Detected that the drag operation is about to end.  Erase
			// the ghost rect so that we don't leave any screen garbage.
			//
			EraseGhostRect();
		}
	}

	return COleDropSource::QueryContinueDrag(bEscapePressed, dwKeyState);
}


////////////////////////////////////////////////////////////////////////
// DrawGhostRect [protected]
//
////////////////////////////////////////////////////////////////////////
void QCOleDropSource::DrawGhostRect()
{
	ASSERT(m_bWantTrackerGhost);

	CWnd* pMainWnd = ::AfxGetMainWnd();
	CDC* pDC = pMainWnd->GetDC();

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	pMainWnd->ScreenToClient(&ptCursor);

	CRect rectNew(ptCursor.x - 80, ptCursor.y - 60, ptCursor.x + 80, ptCursor.y + 60);

	if (m_rectLast.IsRectEmpty())
		pDC->DrawDragRect(&rectNew, CSize(4, 4), NULL, CSize(0, 0));
	else
		pDC->DrawDragRect(&rectNew, CSize(4, 4), &m_rectLast, CSize(4, 4));

	pMainWnd->ReleaseDC(pDC);
	
	//
	// Remember the position of the last ghost rect.
	//
	m_rectLast = rectNew;
}


////////////////////////////////////////////////////////////////////////
// EraseGhostRect [protected]
//
////////////////////////////////////////////////////////////////////////
void QCOleDropSource::EraseGhostRect()
{
	ASSERT(m_bWantTrackerGhost);

	CWnd* pMainWnd = ::AfxGetMainWnd();
	CDC* pDC = pMainWnd->GetDC();

	if (! m_rectLast.IsRectEmpty())
		pDC->DrawDragRect(&m_rectLast, CSize(4, 4), NULL, CSize(0, 0));

	pMainWnd->ReleaseDC(pDC);
	
	//
	// Remember the position of the last ghost rect.
	//
	m_rectLast.SetRectEmpty();
}
