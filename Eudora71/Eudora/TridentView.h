// TridentView.h : interface of the CTridentView class
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//


#ifndef TRIDENTVIEW_H
#define TRIDENTVIEW_H

#include <docobj.h>
#include <ocidl.h>
#include "QCProtocol.h"

// For GetBodyAsHTMLForUseInT
#include "MimeStorage.h"


// trident search flags
#define SEARCHFLAG_MATCHCASE			4
#define SEARCHFLAG_MATCHWHOLEWORD		2

#define DIRECTION_FOWARD				1000000
#define DIRECTION_BACKWARD				-1000000
#define DIRECTION_FORWARDFROMINSERTION 	0

interface IHTMLElement;

// Forward declarations
class CMessageDoc;
class CSummary;
class CTocDoc;


class CTridentView : public CView, public IOleInPlaceFrame, public IOleCommandTarget, public QCProtocol
{
public:
	BSTR				m_bstrBookmark;
	virtual BOOL DoFindFirst(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);
	virtual BOOL DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);


protected: // create from serialization only

    CString             m_szTmpFile;
    static UINT s_uTmpFileUnique;
    CString             m_szBaseFileName;
    BOOL                m_bFixedupSource;
    INT                 m_nIsVisible;
	bool				m_bWasConcentrated;
	bool				m_bCanConcentrate;

    CTridentView();
    DECLARE_DYNAMIC(CTridentView)

    IHTMLElement* GetSelectedElement( LPCSTR szTag );

    CPoint m_ptFileAttachment;
    BOOL m_bEnableAttachmentMenus;
    BOOL QueryElement_FileAttachment( CPoint pt, IHTMLElement** ppElement = NULL );
    BOOL DoCopyAttachment( IHTMLElement* piFileAttachment );
	BOOL GetAttachmentURL( IHTMLElement* piFileAttachment, CString &szHRef );

	BOOL HandleLeftMouseClick(CPoint pt);

	IHTMLElement *		GetAnchorElement(
							CPoint		point);

    virtual BOOL LaunchURL();
    virtual BOOL LaunchURL( CPoint pt );
    virtual BOOL LaunchURL( IHTMLElement* pElement, int command=0);
	virtual BOOL DoSpecialAttachment( IHTMLElement* pElement );
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

	virtual CTocDoc *		GetTocDoc() = 0;

	virtual int				GetNumMessageDocs() const = 0;
	virtual CMessageDoc *	GetMessageDoc(int in_nMessageIndex = 0, bool * out_bLoadedDoc = NULL, CSummary ** out_ppSummary = NULL) = 0;
	virtual bool			IsNoMessageToDisplayAllowed() const = 0;
	virtual void			SetTitleIfNecessary(CMessageDoc * in_pDoc);
	virtual BOOL			UseFixedFont() const = 0;
	virtual bool			ShouldShowAllHeaders() const = 0;
	virtual ContentConcentrator::ContextT	GetContentConcentratorContext(bool in_bIsBlahBlahBlah = false) const = 0;

	virtual void			WriteHeaders(
									CFile &				theFile,
									CMessageDoc *		pDoc,
									const CString &		szHeaders,
									BOOL				bShowTabooHeaders) = 0;

protected:
	virtual BOOL			WriteTempFile(
									CFile &								theFile,
									ContentConcentrator::ContextT		in_context = ContentConcentrator::kCCFullViewContext,
									bool								in_bIsBlahBlahBlah = false);
	virtual BOOL LoadMessage();
	BOOL Print(BOOL bDialog, BOOL bDoPreview = FALSE);

// Generated message map functions
protected:
	void					OnUpdateSearchForSelection(
									CCmdUI *							pCmdUI,
									UINT								nMenuItemWithSelectionFormat,
									UINT								nMenuItemDefaultText);
	void					OnSearchInEudoraForSelection(
									bool								in_bSelectParent,
									bool								in_bSelectAll);

	//{{AFX_MSG(CTridentView)
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintOne();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
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
	afx_msg void OnAttachmentAction(UINT nID);
	afx_msg void OnUpdateAttachmentAction(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSearchWebForSelection(CCmdUI* pCmdUI);
	afx_msg void OnSearchWebForSelection();
	afx_msg void OnUpdateSearchEudoraForSelection(CCmdUI* pCmdUI);
	afx_msg void OnSearchEudoraForSelection();
	afx_msg void OnUpdateSearchMailboxForSelection(CCmdUI* pCmdUI);
	afx_msg void OnSearchMailboxForSelection();
	afx_msg void OnUpdateSearchMailfolderForSelection(CCmdUI* pCmdUI);
	afx_msg void OnSearchMailfolderForSelection();

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
	virtual int GetTotalHeight();

	BOOL	IsReady();
	
	void SetFont(const char* szFontName, INT iSize);

    virtual BOOL GetSelectedText(CString& szText);
	virtual BOOL GetSelectedTextAndTrim(
						CString &		out_szText,
						bool			in_bAllowMultipleLines = true,
						bool			in_bDoSuperTrim = false,
						int				in_nMaxLength = 0,
						bool			in_bUseElipsisIfTruncating = false);
    virtual BOOL GetSelectedHTML(CString& szHTML);
    virtual BOOL GetAllText(CString& szText);
    virtual BOOL GetAllHTML(CString& szHTML);
	virtual BOOL PasteOnDisplayHTML(const char* szHTML, bool bSign);
	virtual BOOL RefreshMessage();

	virtual void		ConvertEmoticonImageTagsToTriggers(CString & io_szHTML);

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
};


/////////////////////////////////////////////////////////////////////////////
#endif
