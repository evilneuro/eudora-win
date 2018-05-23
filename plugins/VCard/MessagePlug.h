// MessagePlug.h: interface for the CMessagePlug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGEPLUG_H__D212512A_752B_11D2_8A0E_00805F9B7487__INCLUDED_)
#define AFX_MESSAGEPLUG_H__D212512A_752B_11D2_8A0E_00805F9B7487__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CTestpluginApp;

class CMessagePlug  
{
public:
	CMessagePlug();
	CMessagePlug(CTestpluginApp* TheApp);
	virtual ~CMessagePlug();

	HICON m_Icon;
	CTestpluginApp* m_TheApp;

};

#endif // !defined(AFX_MESSAGEPLUG_H__D212512A_752B_11D2_8A0E_00805F9B7487__INCLUDED_)
