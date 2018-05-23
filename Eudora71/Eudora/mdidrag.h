#ifndef __MDIDRAG_H__
#define __MDIDRAG_H__

// MDIDRAG.H
//
// Subclassed CMDIChild, to enable minimized MDI child windows
// to be OLE drop targets.
//

#include "mdichild.h"



///////////////////////////////////////////////////////////////////////
// CMDIChildOleDropTarget
//
// Private class that overrides CView-based implementation of 
// COleDropTarget to work with the CMDIChildTarget.
///////////////////////////////////////////////////////////////////////
class CMDIChildOleDropTarget : public COleDropTarget
{
	friend class CMDIChildTarget;		// private class

private:
	CMDIChildOleDropTarget()
	{
	}
	virtual ~CMDIChildOleDropTarget()
	{
	}

public:
	// Overridables
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
								   DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
								  DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
						DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject,
								DROPEFFECT dropDefault, DROPEFFECT dropList,
								CPoint point);

private:
	CMDIChildOleDropTarget(const CMDIChildOleDropTarget&);	// not implemented
	void operator=(const CMDIChildOleDropTarget&);		// not implemented
};


/////////////////////////////////////////////////////////////////////////////
// CMDIChildTarget frame

class CMDIChildTarget : public CMDIChild
{
	DECLARE_DYNCREATE(CMDIChildTarget)
protected:
    // The applications windows all need to respond with the type of wnd they are
    // so this is where they store that information.
    
	CMDIChildTarget();			// protected constructor used by dynamic creation
	virtual ~CMDIChildTarget();

	// Generated message map functions
	//{{AFX_MSG(CMDIChildTarget)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CMDIChildOleDropTarget m_dropTarget;
};

#endif // __MDIDRAG_H__
