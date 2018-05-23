// TridentReadMessageView.h : interface of the CTridentView class
//
/////////////////////////////////////////////////////////////////////////////
#ifndef TRIDENTVIEW_H
#define TRIDENTVIEW_H

#include <docobj.h>
#include <ocidl.h>
#include "QCProtocol.h"

// trident search flags
#define SEARCHFLAG_MATCHCASE			4
#define SEARCHFLAG_MATCHWHOLEWORD		2

#define DIRECTION_FOWARD				1000000
#define DIRECTION_BACKWARD				-1000000
#define DIRECTION_FORWARDFROMINSERTION 	0

interface IHTMLElement;

class CTridentView : public CView, public IOleInPlaceFrame, public IOleCommandTarget, public QCProtocol
{

protected: // create from serialization only

    CString             m_szTmpFile;
    static UINT s_uTmpFileUnique;
    CString             m_szBaseFileName;
    BOOL                m_bFixedupSource;
    INT                 m_nIsVisible;

    CTridentView();
    DECLARE_DYNAMIC(CTridentView)

    IHTMLElement* GetSelectedElement( LPCSTR szTag );

    CPoint m_ptFileAttachment;
    BOOL m_bEnableCopyAttachment;
    BOOL QueryElement_FileAttachment( CPoint pt, IHTMLElement** ppElement = NULL );
    BOOL DoCopyAttachment( IHTMLElement* piFileAttachment );

    virtual BOOL LaunchURL();
    virtual BOOL LaunchURL( CPoint pt );
    virtual BOOL LaunchURL( IHTMLElement* pElement );
    virtual BOOL DoOnDisplayPlugin( IHTMLElement* pElement ) = 0;
    BOOL HasSelectedText();

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTridentView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	virtual void OnDraw(CDC* pDC);
// Implementation
public:
	virtual ~CTridentView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	void MorphMHTML( char ** ppHTML );

	virtual BOOL WriteTempFile( CFile& theFile, CString& szStyleSheetFormat ) = 0;
	virtual BOOL LoadMessage();
	BOOL Print(BOOL bDialog);

// Generated message map functions
protected:
	//{{AFX_MSG(CTridentView)
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFilePrint();
	afx_msg void OnUpdateFileOpenAttachment(CCmdUI* pCmdUI);
	afx_msg void OnFileOpenAttachment();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	afx_msg void OnViewSource();
	afx_msg void OnSendToBrowser();
	afx_msg void OnCopy();
	afx_msg void OnUpdateCopy(CCmdUI* pCmdUI);
	afx_msg void OnSelectAll();
	afx_msg void OnUpdateSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnCopyAttachment();
	afx_msg void OnUpdateCopyAttachment(CCmdUI* pCmdUI);

	afx_msg void OnSpecialAddAsRecipient();
	afx_msg void OnUpdateAddAsRecipient(CCmdUI* pCmdUI);
//	afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	afx_msg long FixupSource(
	WPARAM wParam,
	LPARAM lParam );

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

	virtual bool IsReadOnly(); 
	virtual BOOL QuietPrint();

	BOOL	IsReady();

	void SetFont(
        const CString&  szFontName,
        INT                             iSize);

// BOG
    virtual BOOL GetSelectedText( CString& szText );
    virtual BOOL GetSelectedHTML( CString& szHTML );
    virtual BOOL GetAllText( CString& szText );
    virtual BOOL GetAllHTML( CString& szHTML );
	virtual BOOL PasteOnDisplayHTML(const char* szHTML, bool bSign);

    void StripNBSP( char* szText ) {
        INT     i;

        for ( i = strlen( szText) - 1; i >= 0; i-- ) {
            if( ( UCHAR )( szText[ i ] ) == 0xa0 ) {
                szText[ i ] = ' ';
            }
        }
    }

    void StripBogusCRLF( char* szText ) {
        char* p1,* p2;
        for ( p1 = p2 = szText; *p1; p1++, p2++ ) {
            if ( *p2 == (CHAR)0x0A )
                if ( p2[1] == 0x0D && p2[2] == 0x0A )
                    if ( p2[3] == 0x0D && p2[4] == 0x0A )
                        p2 += 2;

            *p1 = *p2;
        }
    }
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
