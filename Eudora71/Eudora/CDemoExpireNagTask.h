// CDemoExpireNagTask.h: interface for the demo expire nagging classes.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CDEMOEXPIRENAGTASK__INCLUDED_)
#define _CDEMOEXPIRENAGTASK__INCLUDED_
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CNagTask.h"


//////////////////////////////////////////////////////////////////////
// CDemoExpireNagDialog: Dialog for demo expire nag
//////////////////////////////////////////////////////////////////////

class CDemoExpireNagDialog : public CNagDialog
{
public:
	CDemoExpireNagDialog(CNagTask* lpntNagTask, UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CDemoExpireNagDialog();

	virtual void			OnOK();
	virtual BOOL			OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void			DoDataExchange(CDataExchange* pDX);

	virtual void			GetDialogText(CString & out_szDialogText);
};


//////////////////////////////////////////////////////////////////////
// CDemoExpireNagTask: Particular behavior for the demo expire task
//////////////////////////////////////////////////////////////////////

class CDemoExpireNagTask : public CNagTask
{
public:

	CDemoExpireNagTask(CNagTaskEntryP initEntry);
	virtual ~CDemoExpireNagTask();

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

#endif // !defined(_CDEMOEXPIRENAGTASK__INCLUDED_)
