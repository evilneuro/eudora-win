// CAdFailureNagTask.h: interface for the ad failure nagging classes.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CADFAILURENAGTASK__INCLUDED_)
#define _CADFAILURENAGTASK__INCLUDED_
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CNagTask.h"


//////////////////////////////////////////////////////////////////////
// CAdFailureNagDialog: Dialog for ad failure nag
//////////////////////////////////////////////////////////////////////

class CAdFailureNagDialog : public CNagDialog
{
public:
	CAdFailureNagDialog(CNagTask* lpntNagTask, UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CAdFailureNagDialog();

	virtual void			OnOK();
	virtual void			OnCancel();

	virtual void			GetDialogText(CString & out_szDialogText);
};


//////////////////////////////////////////////////////////////////////
// CDeadbeatNagDialog: Dialog for deadbeat nag
//////////////////////////////////////////////////////////////////////

class CDeadbeatNagDialog : public CNagDialog
{
public:
	CDeadbeatNagDialog(CNagTask* lpntNagTask, UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CDeadbeatNagDialog();

	virtual void			OnOK();
	virtual void			OnCancel();
	virtual BOOL			OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void			DoDataExchange(CDataExchange* pDX);

	virtual void			GetDialogText(CString & out_szDialogText);

private:
	CString					m_lpszText;
	BOOL					m_bCancelHit;
};


//////////////////////////////////////////////////////////////////////
// CAdFailureNagTask: Particular behavior for the ad failure task
//////////////////////////////////////////////////////////////////////

class CAdFailureNagTask : public CNagTask
{
public:
	CAdFailureNagTask(CNagTaskEntryP initEntry);
	virtual ~CAdFailureNagTask();

	virtual bool			DoNag(NagTimes context);
		// overriden to do the actual registration

	virtual void			MaybeCancelNag(void *pData);
		// override to see if we should cancel this nag

	virtual CWnd*			CreateNagWindow(void);
		// create the appropriate nag window for the given nag

	virtual void			DoAction(int iAction = IDOK);
		// do the action associated with the given nag, 
		// by default this is called on "ok" exit from dialog

private:
	CTime					m_tLastAdRetrieval;
	BOOL					m_bIsDeadbeat;
};

#endif // !defined(_CADFAILURENAGTASK__INCLUDED_)
