// CRegisterNagTask.h: interface for the register nagging classes.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CREGISTERNAGTASK__INCLUDED_)
#define _CREGISTERNAGTASK__INCLUDED_
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CNagTask.h"


//////////////////////////////////////////////////////////////////////
// CRegisterNagDialog: Dialog for register nag
//////////////////////////////////////////////////////////////////////

class CRegisterNagDialog : public CNagDialog
{
public:
	CRegisterNagDialog(CNagTask* lpntNagTask, UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CRegisterNagDialog();

	virtual void			OnOK();

	virtual void			GetDialogText(CString & out_szDialogText);
};


//////////////////////////////////////////////////////////////////////
// CRegisterNagTask: Particular behavior for the register task
//////////////////////////////////////////////////////////////////////

class CRegisterNagTask : public CNagTask
{
public:

	CRegisterNagTask(CNagTaskEntryP initEntry);
	virtual ~CRegisterNagTask();

	virtual bool			DoNag(NagTimes context);
		// overriden to do the actual registration

	virtual void			MaybeCancelNag(void *pData);
		// override to see if we should cancel this nag

	virtual CWnd*			CreateNagWindow(void);
		// create the appropriate nag window for the given nag

	virtual void			DoAction(int iAction = IDOK);
		// do the action associated with the given nag, 
		// by default this is called on "ok" exit from dialog
};

#endif // !defined(_CREGISTERNAGTASK__INCLUDED_)
