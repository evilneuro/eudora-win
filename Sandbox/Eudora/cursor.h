// CURSOR.H
//
// Cursor class because MFC forgot one

#ifndef _CURSOR_H_
#define _CURSOR_H_

class CCursor
{
public:
	CCursor(LPCSTR CursorName = IDC_WAIT);
	~CCursor();

	void Restore();

	static int	m_Count;
	static BOOL	m_bIgnore;
	static void Ignore( BOOL bIgnore );

private:
	HCURSOR m_PrevCursor;
	BOOL	m_SetCursor;
};

#endif
