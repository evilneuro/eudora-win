// CFullFeatureNagTask.h: interface for the full feature nagging classes.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CFULLFEATURENAGTASK__INCLUDED_)
#define _CFULLFEATURENAGTASK__INCLUDED_
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CNagTask.h"


//////////////////////////////////////////////////////////////////////
// CFullFeatureNagDialog: Dialog for full feature nag
//////////////////////////////////////////////////////////////////////

class CFullFeatureNagDialog : public CNagDialog
{
public:
	CFullFeatureNagDialog(CNagTask* lpntNagTask, UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CFullFeatureNagDialog();

	virtual void			OnOK();
	virtual void			DoDataExchange(CDataExchange* pDX);

	virtual void			GetDialogText(CString & out_szDialogText);

private:
	CString					m_lpszText;
	CEdit					m_edit;
};


//////////////////////////////////////////////////////////////////////
// CFullFeatureNagTask: Particular behavior for the full feature task
//////////////////////////////////////////////////////////////////////

class CFullFeatureNagTask : public CNagTask
{
public:

	CFullFeatureNagTask(CNagTaskEntryP initEntry);
	virtual ~CFullFeatureNagTask();

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

#endif // !defined(_CFULLFEATURENAGTASK__INCLUDED_)
