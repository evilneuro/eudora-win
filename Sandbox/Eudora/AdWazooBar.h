#ifndef __ADWAZOOBAR_H__
#define __ADWAZOOBAR_H__

// AdWazooBar.h : header file
//
// CAdWazooBar
// Class that handles the particular wazoo container that holds the Ad window


#include "WazooBar.h"
#include "QCCommandActions.h"
#include "QICommandClient.h"

///////////////////////////////////////////////////////////////////////
// CAdWazooBar
//
///////////////////////////////////////////////////////////////////////
class QCCommandObject;

class CAdWazooBar : public CWazooBar, public QICommandClient
{
	DECLARE_DYNCREATE(CAdWazooBar)

// Construction
protected:
	CAdWazooBar() {}		// not used!
public:
	CAdWazooBar(CWazooBarMgr* pManager) : CWazooBar(pManager) {}
	virtual ~CAdWazooBar();

// Attributes
public:

// Operations
public:
	BOOL Create(CWnd* pParentWnd);
	virtual BOOL SetDefaultWazooConfig(int nBar);
	virtual BOOL LoadWazooConfigFromIni();
	virtual BOOL SaveWazooConfigToIni();

// Overrides
public:
	virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

	virtual void Notify(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction, void* pData = NULL);

	virtual void OnExtendContextMenu(CMenu* pMenu);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdWazooBar)
protected:
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CAdWazooBar)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	afx_msg void OnCmdSECMDIFloat();
	afx_msg void OnTabsShowSingleTab();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void UpdateState();

	int m_ImageWidth, m_ImageHeight;
};

#endif //__ADWAZOOBAR_H__
