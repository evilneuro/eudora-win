// MsgLabel.h: interface for the CMsgLabel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGLABEL_H__C80BD29C_E9B1_11D1_94C8_00805F9BF4D7__INCLUDED_)
#define AFX_MSGLABEL_H__C80BD29C_E9B1_11D1_94C8_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CMsgLabel  
{
public:
	CMsgLabel();
	CMsgLabel(const COLORREF& clr, LPCSTR str);
	CMsgLabel(const CMsgLabel &copy);
	virtual ~CMsgLabel();

	bool SetText(const CString &txt);
	bool SetColor(const COLORREF& clr);

	LPCSTR GetText() const;
	COLORREF GetColor() const;

protected:
	COLORREF m_Color;
	CString m_Text;
};

#endif // !defined(AFX_MSGLABEL_H__C80BD29C_E9B1_11D1_94C8_00805F9BF4D7__INCLUDED_)
