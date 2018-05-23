#ifndef __WAZOOBARMGR_H__
#define __WAZOOBARMGR_H__

// WazooBarMgr.h : header file
//
// CWazooBarMgr
// Owner for all of main frame's wazoo bars (containers)


/////////////////////////////////////////////////////////////////////////////
// CWazooBarMgr control bar

class CWazooBar;

class CWazooBarMgr
{
// Construction
public:
	CWazooBarMgr();
	virtual ~CWazooBarMgr();

// Operations
public:

	BOOL CreateInitialWazooBars(CMDIFrameWnd* pMainFrame);
	CWazooBar* CreateNewWazooBar(CMDIFrameWnd* pMainFrame);

	void DestroyAllWazooBars();
	BOOL UnlinkWazooBar(CWazooBar* pWazooBar);

	BOOL LoadWazooBarConfigFromIni();
	BOOL SaveWazooBarConfigToIni() const;

	BOOL ActivateWazooWindow(CMDIFrameWnd* pMainFrame, CRuntimeClass* pRuntimeClass);

	CWazooBar* GetWazooBarWithFocus() const;
	void SetActiveWazooWindows();

	CWazooBar *FindWazooWnd(CRuntimeClass *pRuntimeClass) const;

private:
	enum DefaultStateType { DST_SIZE_FIXED, DST_SIZE_RELATIVE, DST_SHOWHIDE, DST_ALL };
	bool SetDefaultWazooBarState(CWazooBar *pWazooBar, int nIndex, DefaultStateType tWhichType = DST_ALL);

	CWazooBar *EnsureGroupExists(CRuntimeClass **);
	void LoadWazooBarIdsFromIni(CDWordArray& dwArrayBarIds) const;
	void SaveWazooBarIdsToIni() const;
	BOOL GetNextBarId(CString& strIds, DWORD& dwId) const;

	CObList m_WazooBarList;
	BOOL m_bUseDefaultConfig;

	static int s_nNextBarNumber;
};

#endif //__WAZOOBARMGR_H__
