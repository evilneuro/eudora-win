// BidentView.h : interface of the CBidentView class
//
/////////////////////////////////////////////////////////////////////////////
#ifndef BIDENTVIEW_H
#define BIDENTVIEW_H

#include <docobj.h>
#include <ocidl.h>
#include "QCProtocol.h"

class CBidentView : public CView, public IOleInPlaceFrame, public IOleCommandTarget, public QCProtocol
{

protected: // create from serialization only

	CString		m_szTmpFile;
	static UINT	s_uTmpFileUnique;
	CString		m_szURL;
	ULONG		m_lReadyState;

	CBidentView();
	DECLARE_DYNAMIC(CBidentView)

	void SetExplorerOptions();

	void WriteDefaults(
	CFile&			theFile,
	const CString&	szFontName);

public:

	void SetFontSize(
	UINT uSize );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBidentView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	virtual void OnDraw(CDC* pDC);
// Implementation
public:
	virtual ~CBidentView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	void MorphMHTML( char ** ppHTML );

	virtual BOOL WriteTempFile( CFile& theFile, BOOL bShowTabooHeaders ) = 0;
	virtual BOOL LoadMessage();

// Generated message map functions
protected:
	//{{AFX_MSG(CBidentView)
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFilePrint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	afx_msg void OnViewSource();
	afx_msg void OnSendToBrowser();
	afx_msg void OnCopy();
	afx_msg void OnUpdateCopy(CCmdUI* pCmdUI);

	afx_msg void OnSpecialAddAsRecipient();
	afx_msg void OnUpdateAddAsRecipient(CCmdUI* pCmdUI);

	afx_msg LONG FixupSource(
	WPARAM w,
	LPARAM l);

// Interface support
protected:

    ULONG           m_cRef;

	void ExecCommandOnNullSet( ULONG ucmdID, VARIANT* pVar1 = NULL, VARIANT* pVar2 = NULL, BOOL bShowUI = FALSE );

    //Other object needs
	BOOL			m_fCreated;
	TCHAR*			m_pchPath;
	HWND			m_hWndObj;			// The object's window

    class CSite*	m_pSite;            //Site holding object        

    IOleInPlaceActiveObject *m_pIOleIPActiveObject;

	BOOL CreateDocObject( TCHAR* pchPath );
	DWORD GetCommandStatus( ULONG );
	void ExecCommand( ULONG ulcmdID, VARIANT* pVar1 = NULL, VARIANT* pVar2 = NULL, BOOL bShowUI = FALSE );

public:

	//Shared IUnknown implementation
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	//IOleInPlaceFrame implementation
    STDMETHODIMP         GetWindow(HWND *);
    STDMETHODIMP         ContextSensitiveHelp(BOOL);
    STDMETHODIMP         GetBorder(LPRECT);
    STDMETHODIMP         RequestBorderSpace(LPCBORDERWIDTHS);
    STDMETHODIMP         SetBorderSpace(LPCBORDERWIDTHS);
    STDMETHODIMP         SetActiveObject(LPOLEINPLACEACTIVEOBJECT
                             , LPCOLESTR);
    STDMETHODIMP         InsertMenus(HMENU, LPOLEMENUGROUPWIDTHS);
    STDMETHODIMP         SetMenu(HMENU, HOLEMENU, HWND);
    STDMETHODIMP         RemoveMenus(HMENU);
    STDMETHODIMP         SetStatusText(LPCOLESTR);
    STDMETHODIMP         EnableModeless(BOOL);
    STDMETHODIMP         TranslateAccelerator(LPMSG, WORD);

	//IOleCommandTarget
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds
        , OLECMD prgCmds[], OLECMDTEXT *pCmdText);
    
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID
        , DWORD nCmdexecopt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);


	virtual BOOL IsEditing();

	BOOL	QuietPrint();

	BOOL	IsReady();
#ifdef ADWARE_PROTOTYPE
public:
	DWORD	GetDocHostUIFlags() {return m_DocHostUIFlags;}
	void	SetDocHostUIFlags(DWORD dwFlags) {m_DocHostUIFlags = dwFlags;}
private:	
	DWORD	m_DocHostUIFlags;
#endif
};


/////////////////////////////////////////////////////////////////////////////
#endif
