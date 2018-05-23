// PaigetView.h : interface of the CPaigeEdtView class
//
/////////////////////////////////////////////////////////////////////////////

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
#include "paige.h"

#include <xstddef>
#pragma warning (disable: 4663 4244 4018 4146)
#include <stack>
#include <queue>
#pragma warning (default: 4663 4244 4018 4146)
using namespace std;

typedef queue<int> INT_QUEUE;
typedef queue<select_pair> SELECT_PAIR_QUEUE;
typedef stack<undo_ref> UNDO_REF_STACK;

enum 
{	
	/***** Uses MAIN STACK only *****/
	// Delete text
	undo_drag = 14,				// undo_delete
	// Insert HR
	undo_insert_hr,				// undo_format
	// Insert text
	undo_drop,					// undo_app_insert
	undo_ole_paste,				// undo_app_insert
	undo_translation,			// undo_app_insert
	
	/***** Use SECONDARY STACK also *****/
	// Insert text with replacement
	undo_ole_paste_delete,		//undo_app_insert		2nd - undo_delete
	undo_drag_drop_move,		//undo_app_insert		2nd - undo_delete
	undo_translation_replace,	//undo_app_insert		2nd - undo_delete	
	undo_overwrite,				//undo_typing			2nd - undo_fwd_delete
	// Insert HR in between text AFTER inserting CR
	undo_insert_hr_withCR		//undo_format			2nd - undo_app_insert
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

class DirtyPair;    // fwd decl
typedef DirtyPair* DirtyPairPtr;
	
class CPaigeEdtView : public CView, protected PgSharedAccess, public QCProtocol
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

	BOOL m_bIncludeHeaderFooters;     // Should a header/footer be done
	BOOL m_bDontDeactivateHighlight;
	BOOL m_bUpdatingScrollbars;

	// change counting
	unsigned long m_chngCount, m_ccLastSaved;
	CPaigeStyle* m_styleEx;
	bool bHasBeenInitiallyUpdated;

    // this keeps track of text ranges that need spell checking
    // and/or auto-url highlighting.
    bool m_checkdastuff;
    time_t last_typed;
    DirtyPairPtr m_pDirtyPair;
    void QueueDirtyPair( long begin, long end );
    bool DequeueDirtyPair( long* begin, long* end );

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

	long GetAttachmentID(long offset);

	pg_ref m_paigeRef;
	CRect m_pgBorder;
	CUndoStack m_undoStack, m_redoStack;
	//Supplement Undo, Redo stacks for pasting (replace selection) and 
	// Drag n Drop (Move op within the app)
	CUndoStack m_deleteUndoStack, m_deleteRedoStack;
	bool m_fRO, m_bAllowStyled;

	CPaigePrintInfo m_thePrintInfo;

public:
	BOOL m_bFixedWidth;
	CPoint m_prevPoint;

// Attributes
public:
// OG
//	CPaigeEdtDoc* GetDocument();
	void SetAllowStyled(bool bStyle = true) { m_bAllowStyled = bStyle; }
	void SetReadOnly( bool bRO = true ) { m_fRO = bRO; }
	bool IsReadOnly(){return (m_fRO != false);}
	int	 CheckSpelling(BOOL autoCheck = FALSE);
	void ScrollToCursor(long offset = CURRENT_POSITION);
	BOOL QuietPrint();
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
		bool bSign=FALSE);
	
	BOOL GetSelectedText( CString& szText);

	BOOL GetSelectedHTML(CString& szHTML );

	BOOL GetAllText(
		CString& szText );

	BOOL GetAllHTML(
		CString& szText );

	BOOL SetSelectedText(
		const char* szText,bool bSign );

	BOOL SetAllText(
		const char* szText,bool bSign );

	BOOL SetSelectedHTML(
		const char* szHTML,bool bSign );

	BOOL SetAllHTML(
		const char* szHTML,bool bSign );

	BOOL PasteOnDisplayHTML( 
		const char* szHTML,bool bSign );

	virtual BOOL GetMessageAsHTML(
		CString& szText,
		BOOL includeHeaders);

	virtual BOOL GetMessageAsText(
		CString& szText,
		BOOL includeHeaders);

	BOOL DoFindFirst( 
		const CString&	szSearch, 
		BOOL			bMatchCase, 
		BOOL			bWholeWord, 
		BOOL			bSelect );

	BOOL DoFindNext( 
		const CString&	szSearch, 
		BOOL			bMatchCase, 
		BOOL			bWholeWord, 
		BOOL			bSelect );
	BOOL Find( 
		long			offset,
		const CString&	szSearch, 
		BOOL			bMatchCase, 
		BOOL			bWholeWord, 
		BOOL			bSelect );

	int GetTotalHeight();
	
	int StrPgCmp(
		const char* pSrcText,
		select_pair& sel,
		BOOL			bMatchCase=TRUE, 
		BOOL			bWholeWord=FALSE, 
		BOOL			bSelect=FALSE);

	char* GetPgText(char* pDstText, int nMaxLen, select_pair& sel, bool bSelection = TRUE);

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
	void OnCopy(COleDataSource *clipBoardStuff);
	void AddRefToTail(CUndoStack &theStack, undo_ref *theRef);
	void RemoveRef(CUndoStack &theRefStack, BOOL fromTail = TRUE); //From Tail or Head
	void SelectionMarginClicked(UINT nFlags, CPoint point);
	bool SetPaigePalette(void);	
	int GetStyleState(int style);
	int GetJustifyState(int style);

	// BOG: hack for drop-menus on sec toolbars. basically forces input
	// capture to be taken away from the toolbar.
	void sec_toolbar_menu_hack(){SetCapture(); SetFocus(); ReleaseCapture();}

protected:
	void CPointToPgPoint(CPoint *cPoint, co_ordinate_ptr PgPoint);
	bool NewPaigeObject();
	void DeletePaigeObject();
	//long InchesToPixels(float inches);
	//float PixelsToInches(long pixels);
	
	virtual void UpdateScrollBars( bool bSetPage = false );
	virtual void SetPgAreas( /*CPaigeEdtView *view*/ );

	void OnPaste(COleDataObject *clipBoardData, short undoVerb = 0, PgDataTranslator::FormatType clipFormat = 0);
	int TabToSpaces();

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaigeEdtView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void Serialize(CArchive& ar);
	virtual void OnInitialUpdate();
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	
	// Printer specific Functions
	// overridden -- or -- inherited functions
	protected:
	virtual BOOL	OnPreparePrinting(CPrintInfo* pInfo);
	virtual	void	OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual	void	OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual	void	OnPrint(CDC* pDC, CPrintInfo* pInfo);
	
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
	CSafetyPalette m_Palette;

// Generated message map functions
protected:
	//{{AFX_MSG(CPaigeEdtView)
//	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSysColorChange(WPARAM wParam, LPARAM lParam);
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
	afx_msg void OnTextPlain();
	afx_msg void OnUpdateTextPlain( CCmdUI* pCmdUI );
	afx_msg void OnIndentIn();
	afx_msg void OnIndentOut();
	afx_msg void OnUpdateIndentOut( CCmdUI* pCmdUI );
	afx_msg void OnParaCenter();
	afx_msg void OnUpdateParaCenter( CCmdUI* pCmdUI );
	afx_msg void OnParaLeft();
	afx_msg void OnUpdateParaLeft( CCmdUI* pCmdUI );
	afx_msg void OnParaRight();
	afx_msg void OnUpdateParaRight( CCmdUI* pCmdUI );
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnEditInsertLink();
	afx_msg void OnUpdateInsertLink(CCmdUI* pCmdUI);

	//afx_msg void OnEditInsertTable();
	//afx_msg void OnUpdateEditInsertTable(CCmdUI* pCmdUI);
	//afx_msg void OnEditDeleteTable();
	//afx_msg void OnUpdateEditDeleteTable(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIndentIn(CCmdUI* pCmdUI);
	
	afx_msg void OnSpecialAddAsRecipient();
	afx_msg void OnUpdateAddAsRecipient(CCmdUI* pCmdUI);
	afx_msg void OnInsertPicture();
	afx_msg void OnUpdateInsertPicture(CCmdUI* pCmdUI);
	afx_msg void OnEditInsertHR();
	afx_msg void OnUpdateEditInsertHR(CCmdUI* pCmdUI);
	afx_msg void OnFontChange();
	afx_msg void OnUpdateFontCombo(CCmdUI* pCmdUI);
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
	afx_msg void OnUpdateAddQuote(CCmdUI* pCmdUI);
	afx_msg void OnRemoveQuote();
	afx_msg void OnUpdateRemoveQuote(CCmdUI* pCmdUI);

//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


// list item based on Paige "select_pair"
class DirtyPair : public select_pair {
    DirtyPair* next;

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

