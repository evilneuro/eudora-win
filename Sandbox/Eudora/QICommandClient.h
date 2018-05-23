// QICommandClient.h: interface for the QICommandClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QICOMMANDCLIENT_H__3068C5A4_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
#define AFX_QICOMMANDCLIENT_H__3068C5A4_AB8E_11D0_97B6_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class QCCommandObject;
extern enum COMMAND_ACTION_TYPE;

class QICommandClient  
{
protected:
	QICommandClient();

public:
	virtual ~QICommandClient();

	virtual void Notify( 
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL ) = 0;

};

#endif // !defined(AFX_QICOMMANDCLIENT_H__3068C5A4_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
