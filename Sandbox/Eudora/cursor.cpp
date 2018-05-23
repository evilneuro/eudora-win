// CURSOR.CPP
//
// Cursor class because MFC forgot one

#include "stdafx.h"

#include "cursor.h"
#include "utils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


static BOOL OverOurWindow()
{
	POINT pt;
	CWnd* MainWindow = AfxGetMainWnd();
	HWND hwnd;

	GetCursorPos(&pt);
	hwnd = WindowFromPoint(pt);
#ifdef unix	// if hwnd==0, warning will follow
	if (hwnd)
#endif
	if (MainWindow && IsChild(MainWindow->m_hWnd, hwnd))
		return (TRUE);

	return (FALSE);
}


int CCursor::m_Count = 0;
BOOL CCursor::m_bIgnore = FALSE;

CCursor::CCursor(LPCSTR CursorName /*= IDC_WAIT*/)
{
	m_PrevCursor = NULL;
	m_SetCursor = FALSE;

	if ( ! m_bIgnore && OverOurWindow() )
	{
		HCURSOR NewCursor = ::LoadCursor(NULL, CursorName);
		if ( ! NewCursor )
			NewCursor = QCLoadCursor(CursorName);
		if ( NewCursor )
		{
			m_PrevCursor = SetCursor(NewCursor);
			m_SetCursor = TRUE;
		}
	}

	m_Count++;
}

CCursor::~CCursor()
{
	Restore();
	m_Count--;
}

void CCursor::Restore()
{
	if ( m_SetCursor && OverOurWindow())
	{
		SetCursor(m_PrevCursor);
		m_PrevCursor = NULL;
		m_SetCursor = FALSE;
	}
}

void CCursor::Ignore( BOOL bIgnore )
{
	m_bIgnore = bIgnore;
}
