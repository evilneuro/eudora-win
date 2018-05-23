// UsgStatsDoc.h: interface for the CUsageStatisticsDoc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USGSTATSDOC_H__2705E759_3BFD_11D4_8559_0008C7D3B6F8__INCLUDED_)
#define AFX_USGSTATSDOC_H__2705E759_3BFD_11D4_8559_0008C7D3B6F8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MDIchild.h"
#include "doc.h"
#include "resource.h"
#include "sbardlg.h"

class CUsageStatisticsDoc : public CDoc  
{
	DECLARE_DYNCREATE(CUsageStatisticsDoc)
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUsageStatisticsDoc)
	public:
	virtual BOOL OnNewDocument();	
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

	CUsageStatisticsDoc();
	virtual ~CUsageStatisticsDoc();
	void UpdateAllAttachedViews();	
	BOOL SaveModified();

};

class CUsageStatisticsDlgBar : public CDialogBar 
{
public:	
	CUsageStatisticsDlgBar();
	short GetSelectedPeriod() 
	{ return m_sPeriodSelected; }

	BOOL IsMoreStatisticsClicked()
	{ return m_bMoreStatistics; }	
	
// Dialog Data
	//{{AFX_DATA(CUsageStatisticsDlgBar)	
	CComboBox	m_CBPeriod;
	CString	m_csPeriod;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUsageStatisticsDlgBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CUsageStatisticsDlgBar)
	afx_msg void OnSelchangePeriodCombo();
	afx_msg void OnMoreStatistics();	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL	m_bMoreStatistics;	// More Statistics or Basic ?
	short	m_sPeriodSelected;	// Currently Selected Period
		
};

class CUsageStatisticsFrame : public CMDIChild
{
public:
	DECLARE_DYNCREATE(CUsageStatisticsFrame)
public:
	CUsageStatisticsFrame();
	~CUsageStatisticsFrame();	

	BOOL PreCreateWindow( CREATESTRUCT& cs );
	
public:
	// This is made public 'coz it's accessed in the UsageStatisticsView class. I know i could
	// have made a member function, but i would leave it as it is.
	CUsageStatisticsDlgBar m_secDialogBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMDIChild)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimeChange();
	afx_msg void OnClose();	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
};




#endif // !defined(AFX_USGSTATSDOC_H__2705E759_3BFD_11D4_8559_0008C7D3B6F8__INCLUDED_)
