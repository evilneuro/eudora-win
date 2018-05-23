// HeaderView.h : header file
//

// protect against multilple includes
#ifndef _HEADERVIEW_H_
#define _HEADERVIEW_H_

/////////////////////////////////////////////////////////////////////////////

#include <afxtempl.h>
#include "resource.h"

#include "PooterButton.h"
#include "AutoCompleteSearcher.h"

/////////////////////////////////////////////////////////////////////////////
// CHeaderView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

// CHeaderField:
// Allows treating Static/Edit combo as a single field.
//
// Right now, this is a very simple implimentation. It subclasses a Static,
// and Edit in tandem, and moves them around as a pair. The moving is very
// rudimentary, and totally subject to change (i.e. a Hack).
class CHeaderView;
class CHeaderField : public AutoCompCEdit
{
	DECLARE_DYNCREATE(CHeaderField)
	
	// attributes
	CHeaderView*	m_pWndParent;
	CStatic*		m_pwndLabel;
	int				m_lineHeight;
	bool			m_beenExpanded;
	AutoCompleterListBox* m_ACListBox;
	bool			m_DoDropDown;

public:
	int LineHeight();
	int m_numLines;
	BOOL m_bInvalid;


	// enumerations
	enum Direction { NdgNil, NdgUp, NdgDown, NdgLeft, NdgRight };

	// construction
	CHeaderField();
	CHeaderField( CHeaderView* pWndParent, UINT idcStatic, UINT idcEdit )
		{m_pWndParent = NULL; Attach( pWndParent, idcStatic, idcEdit );}

	// destruction
	~CHeaderField();

protected:
	//afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCheckSpelling();	
	void OnACListSelectionMade();
	void OnACListDisplayed();
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void SelectAttachments();
	
public:
	// operations
	void GetACListWindowRect(LPRECT rct);
	BOOL Attach( CHeaderView* pWndParent, UINT idcStatic, UINT idcEdit );
	void Noodge( Direction ndgDir, int ndgBy );
	void Grow( int nLines );
	void Shrink( int nLines );
	void SetLabelWidth( LONG lLabelWidth );
	void SetExpanded(bool value);
	bool GetExpanded();
	void FinishNNAutoComplete();
	void FinishFilenameAutoComplete();
	BOOL PreTranslateMessage(MSG* pMsg);
	CString GetText();
	CString GetLabelText();

	void SetAsInvalid(BOOL bInvalid) {m_bInvalid = bInvalid;}
	BOOL IsInvalid() { return (m_bInvalid == 1); }

	void MoveLabelWindow( LPRECT pRect )
	{
		m_pwndLabel->MoveWindow( pRect );
	}

	void GetLabelWindowRect( LPRECT pRect ) const
	{
		m_pwndLabel->GetWindowRect( pRect );
	}

	int CheckSpelling();

	DECLARE_MESSAGE_MAP()
};


// CHeaderView:
// Impliments the "Header" portion of our Split Window, message composition
// MDI Child Window.

class CCompMessageDoc;
class CHeaderView : public CFormView
{
	friend class CHeaderField;

	CTypedPtrList<CPtrList, CHeaderField*> m_headerFields;
	CSize m_docSize;
	CSize m_pageSize;
	CSize m_lineSize;
	int m_headerRows;
	int m_extraRows;
	int m_CurrentHeader;
	LONG	m_lMaxDocSize;
	AutoCompList*	m_BadNames;

	CBrush		CtlBrush;

	BOOL fUpdating;
	BOOL fSizing;

protected:
		int		m_LastChar;
		int		m_DoAutoComplete;

protected:
	CHeaderView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CHeaderView)

// Form Data
public:
	//{{AFX_DATA(CHeaderView)
	enum { IDD = IDD_HEADER_TMPL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:
	enum
	{
		MAXHEADERS = 6,
		DONT_KNOW = -1
	};

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHeaderView)
	public:
	virtual void OnInitialUpdate();

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LONG OnScrollToShowCaret( UINT, LONG );
	//}}AFX_VIRTUAL

	afx_msg long OnContextMenu(WPARAM wParam, LPARAM lParam);

    afx_msg void OnFromPooter();
	afx_msg void OnKillFocusTo();
	afx_msg void OnKillFocusCc();
	afx_msg void OnKillFocusBcc();
	afx_msg void OnKillFocusAttachments();

	afx_msg void OnSetFocusTo();
	afx_msg void OnSetFocusFrom();
	afx_msg void OnSetFocusSubject();
	afx_msg void OnSetFocusCc();
	afx_msg void OnSetFocusBcc();
	afx_msg void OnSetFocusAttachments();
	afx_msg void OnUpdateFontCombo(CCmdUI* pCmdUI);

	afx_msg void OnUpdateMessageAttachFile(CCmdUI* pCmdUI);

	afx_msg BOOL OnDynamicCommand(
	UINT uID );

	afx_msg void OnUpdateDynamicCommand(
	CCmdUI* pCmdUI);

	void OnKillFocusRecipient(UINT nID);
	BOOL PreTranslateMessage(MSG* pMsg);


// Implementation
protected:
	virtual ~CHeaderView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
    BOOL IsEmpty(int header);
    void SaveToDoc();
    void SetHeaderFromDoc();
    CString GetText( int header );
    BOOL GetText( int nHeader, CString& labelText, CString& fieldText );
    void SetText( int header, const char* szText );
    char GetShortcutLetter(int nHeader);
    CHeaderField* CHeaderView::GetHeaderCtrl( int nField );
    void SetFocusToHeader( int nField );
    BOOL SetFocusToHeaderWithShortcut( int nChar );
    BOOL IsHeaderInvalid(int nHeader);
    // Attachment stuff
    void AddAttachment(const char* Filename);
    void SelectAttachments();
    BOOL ScrollToShowCaret();

    CCompMessageDoc* GetDocument();
    int GetCurrentHeader() { return m_CurrentHeader; }
    BOOL ProcessPooters( int nVirtKey );

protected:
	// Generated message map functions
	//{{AFX_MSG(CHeaderView)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnOpenAttachment();
	afx_msg void OnUpdateOpenAttachment(CCmdUI* pCmdUI);
	afx_msg void OnKillFocusHeader();
	//}}AFX_MSG
	afx_msg void OnUpdateEdit( UINT nID );
	afx_msg void OnChangeEdit( UINT nID );

	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPasteAsQuote(CCmdUI* pCmdUI);

	static BOOL m_fShortcutTableInitialized;
	static char m_ShortcutTable[MAXHEADERS];
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif		// _HEADERVIEW_H_
