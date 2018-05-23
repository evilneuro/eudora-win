#ifndef __MDI_CHILD_H__
#define __MDI_CHILD_H__

// MDICHILD.H
//
// Class for MDI child windows
//

#include "workbook.h"

/////////////////////////////////////////////////////////////////////////////
// CMDIChild frame

class CMDIChild : public QCWorksheet
{
	DECLARE_DYNCREATE(CMDIChild)
protected:
    // The applications windows all need to respond with the type of wnd they are
    // so this is where they store that information.
    
	CMDIChild();			// protected constructor used by dynamic creation
	virtual ~CMDIChild();

	BOOL m_bMinTrackValid;
	POINT m_ptMinTrackSize;

	// Generated message map functions
	//{{AFX_MSG(CMDIChild)
	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:	
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
		CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
 		
 	// Return the window type for this window...
 	// DO NOT MAKE THIS FUNCTION VIRTUAL!!!
 	UINT GetTemplateID();
 	
	// Used to control how a child window is initially displayed
	static int m_nCmdShow;
	virtual void ActivateFrame(int nCmdShow = -1);

	// Sets the m_ptMinTrackSize
	void SetMinTrackSize(POINT pt);
	POINT GetMinTrackSize() const;
};

/////////////////////////////////////////////////////////////////////////////


#endif // __MDI_CHILD_H__
