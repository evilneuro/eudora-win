// WebBrowserView.h : header file
//
#ifdef WEB_BROWSER_WAZOO

#ifndef WEBBROWSERVIEW_H
#define WEBBROWSERVIEW_H

#include "TridentView.h"

/////////////////////////////////////////////////////////////////////////////
// CWebBrowserView view

class CWebBrowserView : public CTridentView
{
protected:
	DECLARE_DYNCREATE(CWebBrowserView)

// Attributes
public:

// Operations
public:
	//
	// Normally, the constructor and destructor are protected since
	// the MFC document template stuff takes care of object creation
	// and destruction.  However, since this is now owned by a Wazoo
	// window, these need to be public.
	//
	CWebBrowserView();           
	virtual ~CWebBrowserView();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWebBrowserView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual BOOL DoOnDisplayPlugin( IHTMLElement* pElement );
	virtual BOOL WriteTempFile( CFile& theFile, CString& szStyleSheetFormat );
	virtual BOOL LoadMessage();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWebBrowserView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
#ifdef ADWARE_PROTOTYPE
private:
	void CWebBrowserView::OnTimer(UINT);
#endif
};

/////////////////////////////////////////////////////////////////////////////
#endif //WEBBROWSERVIEW_H

#endif //WEB_BROWSER_WAZOO
