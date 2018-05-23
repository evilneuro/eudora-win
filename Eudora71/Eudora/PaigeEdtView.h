// PaigeEdtView.h : interface of the CPaigeEdtView class
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
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

#ifndef _PAIGE_EDT_VIEW_H_
#define _PAIGE_EDT_VIEW_H_

class CPaigeEdtView;
#include "paige.h"
#include "PgGlobals.h"
#include "QCProtocol.h"
#include "font.h"
#include "paige_io.h"
#include "spellpaige.h"
#include "paigestyle.h"
#include "PgEmbeddedImage.h"
#include "PgStuffBucket.h"
#include "PgHLinks.h"
#include "MoodWatch.h" //text analyser header
#include "paige.h"

#include <xstddef>
#pragma warning (disable: 4663 4244 4018 4146)
#include <stack>
#include <queue>
#pragma warning (default: 4663 4244 4018 4146)
using namespace std;

//for MoodWatch
#define MOOD_INTERVAL 500
enum {AUTO_MOOD_MAIL_TIMER =110, AUTO_BP_MAIL_TIMER};

#define SCAN_INTERVAL 500    // scan timer interval (mSec) (used for auto spell check and emoticon scanning)

//enum {TRIPLE_CLICK_TIMER = 100, DRAG_DROP_SCROLL_TIMER, DRAG_SELECT_TIMER, AUTO_SPELL_TIMER,AUTO_MOOD_MAIL_TIMER };
enum {TRIPLE_CLICK_TIMER = 100, DRAG_DROP_SCROLL_TIMER, DRAG_SELECT_TIMER, SCAN_TIMER};

//	Forward declarations
class CTocDoc;
class PaigeStyleChanger;


typedef void  (*LPPRINTDONECB ) (void * pData);

typedef queue<int> INT_QUEUE;
typedef queue<select_pair> SELECT_PAIR_QUEUE;
typedef stack<undo_ref> UNDO_REF_STACK;

extern CString ExecutableDir;

enum 
{	
	/***** Uses MAIN STACK only *****/
	// Delete text
	undo_drag = 14,						// undo_delete
	// Insert HR
	undo_insert_hr,						// undo_format
	// Insert text
	undo_drop,							// undo_app_insert
	undo_ole_paste,						// undo_app_insert
	undo_translation,					// undo_app_insert
	undo_emoticon_insertion,			// undo_app_insert
	
	/***** Use SECONDARY STACK also *****/
	// Insert text with replacement
	undo_ole_paste_delete,				// undo_app_insert		2nd - undo_delete
	undo_drag_drop_move,				// undo_app_insert		2nd - undo_delete
	undo_translation_replace,			// undo_app_insert		2nd - undo_delete
	undo_emoticon_insertion_replace,	// undo_app_insert		2nd - undo_delete
	undo_spelling_replace,				// undo_app_insert		2nd - undo_delete	
	undo_overwrite,						// undo_typing			2nd - undo_fwd_delete
	// Insert HR in between text AFTER inserting CR
	undo_insert_hr_withCR				// undo_format			2nd - undo_app_insert
}; 


#define UNDO_STACK_MAX 200
typedef CTypedPtrList< CPtrList, undo_ref* > CUndoStack;

#pragma warning(disable : 4663)

class CPaigePrintInfo 
{
public:

	CPaigePrintInfo() : m_pPrinterFont(NULL), m_pHeaderFont(NULL), m_pPen(NULL),
				m_lPenWidth(0), m_uOldMappingMode(0), m_ulBufferSize(0),
				m_ulXScreenRes(0), m_ulYScreenRes(0), visArea(0), pageArea(0)
				{ m_currSelection.begin = 0; m_currSelection.end = 0; } 

	CRect	m_theBodyRect;
	CFontInfo*	m_pPrinterFont;
	CFontInfo*	m_pHeaderFont;
	CPen*	m_pPen;
	CRect	m_HeaderTextRect;
	CRect	m_FooterTextRect;
	LONG	m_lPenWidth;
	UINT	m_uOldMappingMode;   
	ULONG	m_ulBufferSize;
	ULONG	m_ulXScreenRes;
	ULONG	m_ulYScreenRes;
	shape_ref visArea;
	shape_ref pageArea;
	INT_QUEUE			m_fontSizeQueue;
	SELECT_PAIR_QUEUE	m_offsetsQueue;
	UNDO_REF_STACK		m_printUndoStack;
	select_pair			m_currSelection;
	pg_ref				m_printPgRef;
};


extern PG_PASCAL (void) myPostPaginateProc(paige_rec_ptr pg, text_block_ptr block,
		smart_update_ptr update_info, long lineshift_begin, long lineshift_end,
		short action_taken_verb);

//	Forward declarations
class DirtyPair;
class Emoticon;
class EmoticonInText;

//	Type declarations
typedef DirtyPair* DirtyPairPtr;
typedef deque<EmoticonInText>	EmoticonsInTextList;


//	I'm not sure exactly why, but in adding OnCmdMsg I had to change
//	protected PgSharedAccess to public PgSharedAccess in order for
//	OnCmdMsg to actually be called via a base pointer. MFC does not
//	like multiple inheritence. Somehow the protected derivation
//	must have affected the vtable in a harmful way...
class CPaigeEdtView : public CView, public PgSharedAccess, public QCProtocol
{
	friend void PgProcessHyperlinks( pg_ref pg );
	friend PG_PASCAL (void) myPostPaginateProc(paige_rec_ptr pg, text_block_ptr block,
		smart_update_ptr update_info, long lineshift_begin, long lineshift_end,
		short action_taken_verb);
	
	friend class CPgOleDataSource;
private:
	//BOOL m_mouseDown;
	short m_pgModifiers;
	unsigned short m_mouseState;
	COleDropTarget m_dropTarget;
	CPoint m_mousePt;
	BOOL m_bDidAFind;

	BOOL m_bIncludeHeaderFooters;     // Should a header/footer be done
	BOOL m_bDontDeactivateHighlight;

	// change counting
	unsigned long m_chngCount, m_ccLastSaved;
	bool bHasBeenInitiallyUpdated;
	bool m_bHasBeenInitiallyDrawn;

    // this keeps track of text ranges that need spell checking
    // and/or auto-url highlighting.
    bool m_checkdastuff;
    time_t last_typed;
    DirtyPairPtr m_pDirtyPair,m_pMoodDirtyPair;
    void QueueDirtyPair( long begin, long end );
    bool DequeueDirtyPair( long* begin, long* end);
    void QueueMoodDirtyPair( long begin, long end, int nWordType =0 );
    bool DequeueMoodDirtyPair( long* begin, long* end ,int* nWordType =NULL);
	

protected: // create from serialization only
	CPaigeEdtView();
	DECLARE_DYNCREATE(CPaigeEdtView)

	// change counting
	enum ucs_verb {kDocOnly = 0, kAddChange, kUndoChange, kRedoChange};
	void UpdateChangeState( ucs_verb ucsVerb = kDocOnly );
	long SaveChangeState(){return (m_ccLastSaved = m_chngCount);}
	bool HasChanged(){return (m_ccLastSaved != m_chngCount);}
	CSpellPaige m_spell;

	BOOL MakeAutoURLSpaghetti(long start, long end);
	BOOL CleanURLonPosteriorDelete(long selectBegin);
	virtual BOOL LaunchURL(LPCSTR szURL, LPCSTR szLinkText = NULL);
	virtual int DoMoodMailCheck(CMoodWatch *pmoodwatch = NULL);
	int QueueBadWords(TAEAllMatches *sTaeAllMatches, select_pair *pMoodTextRng);
	int ClearBadWords();
	int ClearBadWords(select_pair_ptr pSel);
	int DrawBadWords();
	//bool IsWithinURLRange( select_pair_ptr within = NULL );
	long GetAttachmentID(long offset);

	pg_ref m_paigeRef;
	CPaigeStyle* m_styleEx;
	CRect m_pgBorder;
	CUndoStack m_undoStack, m_redoStack;
	//Supplement Undo, Redo stacks for pasting (replace selection) and 
	// Drag n Drop (Move op within the app)
	CUndoStack m_deleteUndoStack, m_deleteRedoStack;
	bool m_fRO, m_bAllowStyled, m_bAllowTables;
	bool m_bMoodMailDirty; //Boolean variable for Mood Mail
	bool m_bServicingSpellMoodBPTimer;
	CPaigePrintInfo m_thePrintInfo;
	void* m_pABOrigView;//this will be NULL - UNLESS we are using this view for address book preview printing.

	// Link warning tooltip style popup text
	static long			s_nLastCheckedHyperlinkID;
	static long			s_nCurrentWarningHyperlinkID;

	// Format Painter
	style_info		m_styleInfo;
	font_info		m_fontInfo;
	par_info		m_parInfo;
	bool			m_bCopiedStyleInfo;
	bool			m_bCopiedParInfo;

public:
	BOOL m_bFixedWidth;
	CPoint m_prevPoint;
	void SetABOrigView(void* pView = NULL) { m_pABOrigView = pView;}
	void* GetABOrigView(){ return  m_pABOrigView;} 
	void DontDeactivateHighLight(BOOL bVal = TRUE) { m_bDontDeactivateHighlight = bVal;}
	void AllowTables(bool bAllow = true) { m_bAllowTables = bAllow;}
// Attributes
public:
// OG
//	CPaigeEdtDoc* GetDocument();
	void SetAllowStyled(bool bStyle = true) { m_bAllowStyled = bStyle; }
	void SetReadOnly( bool bRO = true ) { m_fRO = bRO; }
	int	 CheckSpelling(BOOL autoCheck = FALSE);
	void MarkWordNotMisspelled(const char * in_szWord);
	BOOL HasSelection();
	void ScrollToCursor(long offset = CURRENT_POSITION);
	int MapScrollPos(int nPos, int sourceMax, int dstMax);
	
	//Generic GetTextAs func, used by all other Get funcs.
	BOOL GetTextAs(
		pg_ref theRef,
		CString &text,
		PgDataTranslator::FormatType format = PgDataTranslator::kFmtHtml );

	//Generic SetTextAs func, used by all other Set funcs.
	BOOL SetTextAs(
		const char* text,
		const select_pair_ptr sel,
		PgDataTranslator::FormatType format = PgDataTranslator::kFmtHtml,
		bool bSign = false,
		bool bPrepareUndo = true,
		bool bScrollToCursor = true);
	
	virtual CTocDoc *		GetTocDoc();

	virtual BOOL GetSelectedText(CString& szText);
	virtual BOOL GetSelectedTextAndTrim(
						CString &		out_szText,
						bool			in_bAllowMultipleLines = true,
						bool			in_bDoSuperTrim = false,
						int				in_nMaxLength = 0,
						bool			in_bUseElipsisIfTruncating = false);
	virtual BOOL GetSelectedHTML(CString& szHTML);
	virtual BOOL GetAllText(CString& szText);
	virtual BOOL GetAllHTML(CString& szText);

	virtual BOOL SetSelectedText(const char* szText, bool bSign);
	virtual BOOL SetAllText(const char* szText, bool bSign);
	virtual BOOL SetSelectedHTML(const char* szHTML, bool bSign);
	virtual BOOL SetAllHTML(const char* szHTML, bool bSign);
	virtual BOOL PasteOnDisplayHTML(const char* szHTML, bool bSign);

	virtual BOOL GetMessageAsHTML(CString& szText, BOOL includeHeaders);
	virtual BOOL GetMessageAsText(CString& szText, BOOL includeHeaders);

	virtual BOOL DoFindFirst(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);
	virtual BOOL DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);
	BOOL Find(long offset, const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);

	virtual int GetTotalHeight();
	virtual bool IsReadOnly(){return (m_fRO != false);}
	virtual BOOL QuietPrint();
	
	int StrPgCmp(const char* pSrcText, select_pair& sel, BOOL bMatchCase=TRUE, BOOL bWholeWord=FALSE, BOOL bSelect=FALSE);
	char* GetPgText(char* pDstText, int nMaxLen, select_pair& sel, bool bSelection = TRUE, long * out_pTextLen = NULL);

	void PrepareUndo(short verb, long *insert_ref = MEM_NULL, BOOL usedeleteStack = FALSE);
	static short GetPaigeUndoVerb( short EudoraUndoVerb );

	BOOL ClearStyle();
	BOOL ClearFormat();
	BOOL ClearHyperlink();

// Operations
private:
	// Helper Functions
//	void ExportData(COleDataSource *clipBoardStuff, FormatType format);
//	void ImportData(COleDataObject *clipBoardData, FormatType format);
	void AddRefToTail(CUndoStack &theStack, undo_ref *theRef);
	void RemoveRef(CUndoStack &theRefStack, BOOL fromTail = TRUE); //From Tail or Head
	void SelectionMarginClicked(UINT nFlags, CPoint point);

	void			FindSelectedParagraph(
							select_pair &			in_sel,
							long &					out_nParaBegin,
							long &					out_nParaEnd);

	bool SetPaigePalette(void);	
	int GetStyleState(int style);
	int GetJustifyState(int style);

	// BOG: hack for drop-menus on sec toolbars. basically forces input
	// capture to be taken away from the toolbar.
	void sec_toolbar_menu_hack(){SetCapture(); SetFocus(); ReleaseCapture();}

	// support for EDIT/RICHEDIT windows messages
	BOOL EditWindowProc( UINT message, WPARAM wParam, LPARAM lParam, LPLONG response );

protected:
	void CPointToPgPoint(CPoint *cPoint, co_ordinate_ptr PgPoint) const;
	embed_ref GetEmbedRectAtPoint(CPoint & ptMouse, CRect & rect) const;
	virtual bool NewPaigeObject(long AddFlags = 0, long AddFlags2 = 0);
	virtual void DeletePaigeObject();
	//long InchesToPixels(float inches);
	//float PixelsToInches(long pixels);
	
	short GetPaigeVerbForScrollCode(UINT nSBCode);
	virtual void UpdateScrollBars( bool bSetPage = false, bool bAllowScrollToView = true, bool * pbNoScrollingNeeded = NULL );
	virtual void SetPgAreas( /*CPaigeEdtView *view*/ );

	virtual void OnCopy(COleDataSource *clipBoardStuff);
	void GuardBeforeImportingNative();
	void CleanupAfterImportingNative();
	void AdjustMouseIfEmbedded(co_ordinate & pgMousePt);
	bool OnPaste(COleDataObject *clipBoardData, short undoVerb = 0, PgDataTranslator::FormatType clipFormat = 0);
	int TabToSpaces();
	void FindEmoticonTriggerLocations(const select_pair & in_selectPair, EmoticonsInTextList & in_emoticonsInText);
	void ReplaceEmoticonTriggersWithImages(EmoticonsInTextList & in_emoticonsInText);

public:
	virtual BOOL			OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaigeEdtView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//virtual void Serialize(CArchive& ar);
	virtual void OnInitialUpdate();
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
	
	// Printer specific Functions
	// overridden -- or -- inherited functions
	protected:
	virtual BOOL	OnPreparePrinting(CPrintInfo* pInfo);
	virtual	void	OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual	void	OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual	void	OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void    OnFilePrintPreview();
	
	// Our functions  
	virtual	void	DoPageHeader(CDC* pDC, CPrintInfo* pInfo);
	virtual	void	DoPageFooter(CDC* pDC, CPrintInfo* pInfo);
    

	// The header and footer text functions provide the actual text in the
	// header or footer...you should override these to provide different text..
    virtual void	GetHeaderText(LPTSTR theText, int nSize);
    virtual	void	GetFooterText(LPTSTR theText, int nSize); 

	void UpdateBlockFmt(CCmdUI* pCmdUI, unsigned char* stylename );



	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPaigeEdtView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CSafetyPalette	m_Palette;
	CToolTipCtrl	m_ToolTip;
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual void	ApplyStyleChange(PaigeStyleChanger & paigeStyleChanger, bool bNeedToPrepareUndo = true);

	bool					find_signature(
									select_pair_ptr						pSignatureRange,
									select_pair_ptr						pSearchWithin = NULL);
	bool					PreventDestroyingSignatureStyle(
									bool								bIsDelete,
									bool								bIsFowardDelete);

	virtual void			InsertEmoticonAtCurrentPosition(Emoticon * pEmoticon);

// Generated message map functions
protected:
	void					OnUpdateSearchForSelection(
									CCmdUI *							pCmdUI,
									UINT								nMenuItemWithSelectionFormat,
									UINT								nMenuItemDefaultText);
	void					OnSearchInEudoraForSelection(
									bool								in_bSelectParent,
									bool								in_bSelectAll);
	void					ChangeParaJustification(
									short								in_nNewJustification);

	//{{AFX_MSG(CPaigeEdtView)
//	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysColorChange();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditPasteSpecial();
	afx_msg void OnEditPasteAsQuotation();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnFormatFont();
	afx_msg void OnUpdateFormatFont(CCmdUI* pCmdUI);
	afx_msg void OnFormatStyles();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnTextBold();
	afx_msg void OnUpdateTextBold( CCmdUI* pCmdUI );
	afx_msg void OnTextItalic();
	afx_msg void OnUpdateTextItalic( CCmdUI* pCmdUI );
	afx_msg void OnTextUnderline();
	afx_msg void OnUpdateTextUnderline( CCmdUI* pCmdUI );
	afx_msg void OnTextStrikeout();
	afx_msg void OnUpdateTextStrikeout( CCmdUI* pCmdUI );
	afx_msg void OnTextPlain();
	afx_msg void OnUpdateTextPlain( CCmdUI* pCmdUI );
	afx_msg void OnIndentIn();
	afx_msg void OnIndentOut();
	afx_msg void OnMarginsNormal();
	afx_msg void OnUpdateIndentCommand( CCmdUI* pCmdUI );
	afx_msg void OnParaCenter();
	afx_msg void OnUpdateParaCenter( CCmdUI* pCmdUI );
	afx_msg void OnParaLeft();
	afx_msg void OnUpdateParaLeft( CCmdUI* pCmdUI );
	afx_msg void OnLastTextColor();
	afx_msg void OnUpdateLastTextColor( CCmdUI* pCmdUI );
	afx_msg void OnParaRight();
	afx_msg void OnUpdateParaRight( CCmdUI* pCmdUI );
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnEditInsertLink();
	afx_msg void OnUpdateInsertLink(CCmdUI* pCmdUI);
	afx_msg LRESULT OnUpdateImage(WPARAM, LPARAM);

	//afx_msg void OnEditInsertTable();
	//afx_msg void OnUpdateEditInsertTable(CCmdUI* pCmdUI);
	//afx_msg void OnEditDeleteTable();
	//afx_msg void OnUpdateEditDeleteTable(CCmdUI* pCmdUI);
	
	afx_msg void OnSpecialAddAsRecipient();
	afx_msg void OnUpdateAddAsRecipient(CCmdUI* pCmdUI);
	afx_msg void OnInsertPicture();
	afx_msg void OnUpdateInsertPicture(CCmdUI* pCmdUI);
	afx_msg void OnInsertPictureLink();
	afx_msg void OnEditInsertHR();
	afx_msg void OnUpdateEditInsertHR(CCmdUI* pCmdUI);
	afx_msg void OnFontChange();
	afx_msg void OnUpdateFontCombo(CCmdUI* pCmdUI);
	afx_msg void OnFormatPainter();
	afx_msg void OnUpdateFormatPainter(CCmdUI * pCmdUI);
	afx_msg void OnClearFormatting();
	afx_msg void OnUpdateClearFormatting(CCmdUI* pCmdUI);
	afx_msg void OnColorChange( UINT uColorID );
	afx_msg void OnTextSizeChange( UINT uSizeID );
	afx_msg	void OnUpdateEditStyle( CCmdUI* pCmdUI );
	afx_msg void OnBlkfmtBullettedList();
	afx_msg void OnUpdateBlkfmtBullettedList(CCmdUI* pCmdUI);
	
//	afx_msg void OnBlkfmtBq();
//	afx_msg void OnUpdateBlkfmtBQ(CCmdUI* pCmdUI);
//	afx_msg void OnBlkfmtH1();
//	afx_msg void OnUpdateBlkfmtH1(CCmdUI* pCmdUI);
//	afx_msg void OnBlkfmtH2();
//	afx_msg void OnUpdateBlkfmtH2(CCmdUI* pCmdUI);
//	afx_msg void OnBlkfmtH3();
//	afx_msg void OnUpdateBlkfmtH3(CCmdUI* pCmdUI);
//	afx_msg void OnBlkfmtH4();
//	afx_msg void OnUpdateBlkfmtH4(CCmdUI* pCmdUI);
//	afx_msg void OnBlkfmtH5();
//	afx_msg void OnUpdateBlkfmtH5(CCmdUI* pCmdUI);
//	afx_msg void OnBlkfmtH6();
//	afx_msg void OnUpdateBlkfmtH6(CCmdUI* pCmdUI);
//	afx_msg void OnBlkfmtNormal();
//	afx_msg void OnUpdateBlkfmtNormal(CCmdUI* pCmdUI);
	
	afx_msg void OnEditWrapSelection();
	afx_msg void OnUpdateNeedSelEdit(CCmdUI* pCmdUI);
	afx_msg BOOL OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message );
	afx_msg void OnBlkfmtFixedwidth();
	afx_msg void OnUpdateBlkfmtFixedwidth(CCmdUI* pCmdUI);
	afx_msg void OnOpenAttachment();
	afx_msg void OnUpdateOpenAttachment(CCmdUI* pCmdUI);
	afx_msg void OnAddQuote();
	afx_msg void OnUpdateQuoteCommand(CCmdUI* pCmdUI);
	afx_msg void OnRemoveQuote();
	afx_msg void OnUpdateSearchWebForSelection(CCmdUI* pCmdUI);
	afx_msg void OnSearchWebForSelection();
	afx_msg void OnUpdateSearchEudoraForSelection(CCmdUI* pCmdUI);
	afx_msg void OnSearchEudoraForSelection();
	afx_msg void OnUpdateSearchMailboxForSelection(CCmdUI* pCmdUI);
	afx_msg void OnSearchMailboxForSelection();
	afx_msg void OnUpdateSearchMailfolderForSelection(CCmdUI* pCmdUI);
	afx_msg void OnSearchMailfolderForSelection();

//}}AFX_MSG

	afx_msg BOOL OnDynamicCommand(UINT uID);
	afx_msg	void OnUpdateDynamicCommand(CCmdUI * pCmdUI);
	afx_msg void OnLastEmoticon();
	afx_msg void OnUpdateLastEmoticon( CCmdUI* pCmdUI );

	DECLARE_MESSAGE_MAP()
};


// list item based on Paige "select_pair"
class DirtyPair : public select_pair {
    DirtyPair* next;
// Added for MoodWatch
	int nMoodWordType;
    friend class CPaigeEdtView;
};


// Abstract base class, which is passed as a parameter to ApplyStyleChange
// Subclass this with an appropriate implementation for any style changes
// to a selection that needs to be broken down by paragraph.
class PaigeStyleChanger
{
public:
	virtual void		GetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex) = 0;
	virtual bool		PrepareToSetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex) = 0;
	virtual void		SetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex) = 0;
	virtual bool		AreStylesIncompatible() = 0;
};


// Concrete implementation of PaigeStyleChanger which changes the
// excerpt (quote) level of the selected text.
class PaigeExcerptLevelChanger : public PaigeStyleChanger
{
public:
						PaigeExcerptLevelChanger(bool bOn);
	
	virtual void		GetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual bool		PrepareToSetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual void		SetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual bool		AreStylesIncompatible();

protected:
	bool				m_bOn;
	par_info			m_mask, m_info[2];
};

// Concrete implementation of PaigeStyleChanger which changes the indent level
// of the selected text.
class PaigeIndentLevelChanger : public PaigeStyleChanger
{
public:
						PaigeIndentLevelChanger(pg_fixed indentDelta);
	
	virtual void		GetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual bool		PrepareToSetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual void		SetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual bool		AreStylesIncompatible();

protected:
	pg_fixed			m_indentDelta;
	long				m_nHTMLBulletLevel[2];
	pg_indents			m_mask, m_indent[2];
};

// Concrete implementation of PaigeStyleChanger which makes the indent level
// of the selected text be it's base level (i.e. as far left as possible,
// taking into account bullet indenting).
class PaigeMarginsNormalChanger : public PaigeStyleChanger
{
public:
						PaigeMarginsNormalChanger();
	
	virtual void		GetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual bool		PrepareToSetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual void		SetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual bool		AreStylesIncompatible();

protected:
	bool				m_bIsBullet[2];
	pg_indents			m_mask, m_indent;
	int					m_numChangesRequired;
	bool				m_bLastGetStyleInfoChangeRequired;
};

// Concrete implementation of PaigeStyleChanger which changes the justification
// of the selected text.
class PaigeJustificationChanger : public PaigeStyleChanger
{
public:
						PaigeJustificationChanger(short in_nNewJustification);
	
	virtual void		GetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual bool		PrepareToSetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual void		SetStyleInfo(pg_ref pg, CPaigeStyle * pgStyle, select_pair_ptr pSel, int infoIndex);
	virtual bool		AreStylesIncompatible();

protected:
	short				m_nNewJustification;
	par_info			m_mask, m_info[2];
};

//This class will used to queue the dirty pairs. We can use CObList to add the elements and 
// delete the elements. This would be more generic.
class CObDirtyPair : public CObject
{
public :
	CObDirtyPair (int nBegin, int nEnd)
	{
		m_nBegin = nBegin;
		m_nEnd = nEnd;
	}
private:
	int m_nBegin;
	int m_nEnd;
    friend class CPaigeEdtView;
};


// OG
/*
#ifndef _DEBUG  // debug version in PaigeEdtView.cpp
inline CPaigeEdtDoc* CPaigeEdtView::GetDocument()
   { return (CPaigeEdtDoc*)m_pDocument; }
#endif
*/


/////////////////////////////////////////////////////////////////////////////

#endif	// _PAIGE_EDT_VIEW_H_

