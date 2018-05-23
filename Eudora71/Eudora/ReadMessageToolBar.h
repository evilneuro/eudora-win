// ReadMessageToolBar.h: interface for the CReadMessageToolBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_READMESSAGETOOLBAR_H__F2AD9DE1_2874_11D1_8423_00805FD2F268__INCLUDED_)
#define AFX_READMESSAGETOOLBAR_H__F2AD9DE1_2874_11D1_8423_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCChildToolBar.h"

class CReadMessageToolBar : public QCChildToolBar  
{
public:
	CReadMessageToolBar();
	virtual ~CReadMessageToolBar();

	DECLARE_MESSAGE_MAP()

public:
	
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#endif // !defined(AFX_READMESSAGETOOLBAR_H__F2AD9DE1_2874_11D1_8423_00805FD2F268__INCLUDED_)
