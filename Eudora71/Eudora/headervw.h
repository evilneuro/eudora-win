// HeaderView.h : header file
//
// Copyright (c) 1993-2001 by QUALCOMM, Incorporated
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

// protect against multilple includes
#ifndef _HEADERVIEW_H_
#define _HEADERVIEW_H_

/////////////////////////////////////////////////////////////////////////////

#include <afxtempl.h>
#include "resource.h"

#include "PooterButton.h"
#include "AutoCompleteSearcher.h"
#include "QCProtocol.h"
#include "MoodWatch.h"
#include "font.h"

/////////////////////////////////////////////////////////////////////////////
// CHeaderView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class HeaderDirtyPair;    // fwd decl
typedef HeaderDirtyPair* HeaderDirtyPairPtr;
class HeaderDirtyPair {
public:
	long LineStartX;
	long LineStartY;
	long LineEndX;
	long LineEndY;
    HeaderDirtyPair* next;
	int nMoodWordType;
};

class PgCompMsgView;

// CHeaderField:
// Allows treating Static/Edit combo as a single field.
//
// Right now, this is a very simple implimentation. It subclasses a Static,
// and Edit in tandem, and moves them around as a pair. The moving is very
// rudimentary, and totally subject to change (i.e. a Hack).

#define ID_MOOD_PAIR  1
#define ID_SPELL_PAIR 2
#define ID_BP_PAIR 3

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
	BOOL	m_bMoodDirty;
	BOOL	m_bSpellDirty;
	BOOL	m_bBPDirty;
	BOOL	m_bBPSpeakDirty;
	BOOL	m_bBPWarnDlgDirty;
	HeaderDirtyPair* m_pMoodDirtyPair;
	HeaderDirtyPair* m_pSpellDirtyPair;
	HeaderDirtyPair* m_pBPDirtyPair;
	BOOL m_bServicingSpellTimer;

public:
	int LineHeight();
	int m_numLines;
	BOOL m_bInvalid;


	// enumerations
	enum Direction { NdgNil, NdgUp, NdgDown, NdgLeft, NdgRight };

	// construction
	CHeaderField();

	// destruction
	~CHeaderField();

protected:
	//afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKillFocus(CWnd* pNewWnd); 
	afx_msg void OnDestroy();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnCheckSpelling();	

	void OnACListSelectionMade();
	void OnACListDisplayed();
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void SelectAttachments();
	BOOL IsMissSpelledWord (CPoint point,int& StartOffset, int& EndOffset);
	
public:
	// operations
	int OnCheckSpelling(BOOL bAutoCheck=FALSE);	
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
	BOOL IsMoodDirty() { return m_bMoodDirty; }
	void SetMoodDirty(bool bStatus) { m_bMoodDirty = bStatus;}
	BOOL IsBPDirty() { return m_bBPDirty; }
	void SetBPDirty(bool bStatus) { m_bBPDirty = bStatus; }
	BOOL IsBPSpeakDirty() { return m_bBPSpeakDirty; }
	BOOL IsBPWarnDlgDirty() { return m_bBPWarnDlgDirty; }
	void SetBPSpeakDirty(bool bStatus) { m_bBPSpeakDirty = bStatus; }
	void SetBPWarnDlgDirty(bool bStatus) { m_bBPWarnDlgDirty = bStatus; }
	int AddFieldMoodText(CMoodWatch *pMoodWatch);
	int DrawLine(TAEAllMatches *pTaeAllMatches, int nLineNum);
	int DrawMisspelledLine(int nStartOffset, int nEndOffset);
	int DrawBPAddressesLine(int nStartOffset, int nEndOffset);

	void ClearAllDirtyPairs();
	void QueueDirtyPair(int nPairType, 
		CPoint StartPt, CPoint EndPt, int nWordType=0);
	bool DequeueDirtyPair(int nPairType, 
		CPoint* StartPt, CPoint *EndPt, int *nWordType );
	void SetSpellTimer();
	void KillSpellTimer();
	void ClearAllMoodDirtyPairs();
	void ClearAllSpellDirtyPairs();
	void ClearAllBPDirtyPairs();
	void DoSpellCheck();
	bool DoBPCheck(CStringList * pAddressList = NULL);
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
class CHeaderView : public CFormView, public QCProtocol
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
	int m_nMoodScore;
	//bool m_bBPWarning;

protected:
		int		m_LastChar;
		int		m_DoAutoComplete;
		CStringArray m_AutoArray;
		BOOL m_bJustQueue;
		PgCompMsgView *		m_pPgCompMsgView;

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
    BOOL IsHeaderMoodDirty();
	BOOL IsHeaderBPDirty();
	BOOL IsHeaderBPSpeakDirty();
	BOOL IsHeaderBPWarnDlgDirty();
	bool DoBPCheck(CStringList * pAddressList = NULL);
	BOOL IsSelSubject();
	static CFontInfo &	GetEditFont() { return s_editFont; }
	static CFontInfo &	GetLabelFont() { return s_labelFont; }

    // Attachment stuff
    void AddAttachment(const char* Filename);
	void SelectAttachments();
	int OnCheckSpelling(BOOL bAutoCheck=FALSE);
    BOOL ScrollToShowCaret();

    CCompMessageDoc* GetDocument();
    int GetCurrentHeader() { return m_CurrentHeader; }
    BOOL ProcessPooters( int nVirtKey );
	void SetHeaderMoodDirty(bool bStatus);
	void SetHeaderBPDirty(bool bStatus);
	void SetHeaderBPSpeakDirty(bool bStatus);
	void SetHeaderBPWarnDlgDirty(bool bStatus);
	int AddMoodText(CMoodWatch *pMoodWatch, int nHeaderField);

	// QCProtocol Overrides.
	virtual BOOL DoFindFirst(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);
	virtual BOOL DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);
	int GetMoodScore(){return m_nMoodScore;}
	void SetMoodScore(int nMoodScore) {m_nMoodScore = nMoodScore;}
	int  ValidateFields(bool bMoodCheck, bool bBPWarning);
	void SetJustQueue(BOOL bJustQueue) {m_bJustQueue= bJustQueue;}
	int GetJustQueue() {return m_bJustQueue;}
	int GetNumberOfAddrInHeader();
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
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);

	// Keep our own font info. This prevents us from changing our display
	// font when Eudora's Screen Font changes.
	static int			s_nObjectCount;
	static CFontInfo	s_editFont;
	static CFontInfo	s_labelFont;

	static BOOL m_fShortcutTableInitialized;
	static char m_ShortcutTable[MAXHEADERS];
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif		// _HEADERVIEW_H_
