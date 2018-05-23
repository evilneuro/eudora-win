// QCProtocol.h: interface for the QCProtocol class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCPROTOCOL_H__F2AD9DE3_2874_11D1_8423_00805FD2F268__INCLUDED_)
#define AFX_QCPROTOCOL_H__F2AD9DE3_2874_11D1_8423_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCChildToolBar.h"

enum QCPROTOCOLID 
{
	QCP_FIND,
	QCP_QUIET_PRINT,
	QCP_SAVE_INFO,
	QCP_GET_MESSAGE,
	QCP_TRANSLATE,
	QCP_SPELL,
	QCP_METRICS,
	QCP_FORMAT_TOOLBAR,
	QCP_READ_TOOLBAR
};


class QCProtocol  
{
public:
	QCProtocol();
	virtual ~QCProtocol();
	
	virtual BOOL CheckSpelling(BOOL);
	virtual BOOL DoFindFirst( 
	const CString&	szSearch, 
	BOOL			bMatchCase, 
	BOOL			bWholeWord, 
	BOOL			bSelect );

	virtual BOOL DoFindNext( 
	const CString&	szSearch, 
	BOOL			bMatchCase, 
	BOOL			bWholeWord, 
	BOOL			bSelect );

	virtual BOOL QuietPrint();
	virtual void SaveInfo();
		
	static QCProtocol* QueryProtocol(
	QCPROTOCOLID	theID, 
	CObject*		pObject );

	virtual QCChildToolBar* GetFormatToolbar();
	virtual QCChildToolBar* GetToolbar();

	virtual BOOL GetSelectedText(
	CString& szText );

	virtual BOOL GetAllText(
	CString& szText );

	virtual BOOL GetSelectedHTML(
	CString& szHTML );

	virtual BOOL GetAllHTML(
	CString& szHTML );

	virtual BOOL SetSelectedText(
	const char* szText,bool bSign );

	virtual BOOL SetAllText(
	const char* szText,bool bSign );

	virtual BOOL SetSelectedHTML(
	const char* szHTML,bool bSign );

	virtual BOOL SetAllHTML(
	const char* szHTML,bool bSign );

	virtual BOOL PasteOnDisplayHTML(
	const char* szHTML,bool bSign );

	virtual BOOL GetMessageAsHTML(
	CString& msg,
	BOOL IncludeHeaders = FALSE);

	virtual BOOL GetMessageAsText(
	CString& msg,
	BOOL IncludeHeaders = FALSE); 

	virtual int GetTotalHeight();

	virtual bool IsReadOnly(){ return FALSE; }
};

#endif // !defined(AFX_QCPROTOCOL_H__F2AD9DE3_2874_11D1_8423_00805FD2F268__INCLUDED_)
