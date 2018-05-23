/////////////////////////////////////////////////////////////////////////////
// DropEdit.h : header file for SECDropEdit - subclassed edit with little
//			    drop down button.
//
// Stingray Software Extension Classes
// Copyright (C) 1996-1997 Stingray Software Inc,
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
//	Author:		John
//

#ifndef __CURREDIT_H__
#define __CURREDIT_H__

#ifdef WIN32

// The SECCurrencyEdit provides an extensible class for entering
// and displaying custom-formatted currency data.  Input data parsing
// and output display formatting can be customized by descending new
// classes from SECCurrencyEdit::Format and SECCurrencyEdit.
//
// The SECCurrencyEdit supports some of the EM_* edit control message
// interface, and can thus be manipulated to some degree using a CEdit
// object (not recommended, but possible if required for compatibility
// with existing code).  The following lists the messages supported.
//
// Windows Messages		Edit Control Messages
// ----------------		---------------------
// WM_COPY				EM_CANUNDO
// WM_CUT				EM_EMPTYUNDOBUFFER
// WM_GETFONT			EM_GETSEL
// WM_PASTE				EM_SETREADONLY
// WM_SETFONT			EM_SETSEL
// WM_SETREDRAW			EM_UNDO
// WM_SETTEXT
// WM_UNDO


#ifndef __DROPEDIT_H__
#include "dropedit.h"
#endif

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL


class SECCurrencyEdit : public SECDropEdit //public CWnd
{
public:

	// The following nested helper class provides the core currency
	// formatting and parsing methods.

	class Format
	{
	public:
		// Construction/destruction.  Note: if the default initialization
		// does not occur, the object will in a valid but decidedly 
		// American state.

		Format(BOOL bInitializeFromSystemDefaults = TRUE);
		virtual ~Format() {}

		// Property retrieval and setting methods for all the currency
		// display options.  Some important notes: 
		//
		// 1) Setting the thousand separator to the null character ('\0') 
		//    will prevent its use.
		// 2) Setting the decimal digits to negative one (-1) will use 
		//    as many digits are necessary to display the number.  If
		//    the number of digits is larger than required, the output
		//    will be padded with the padding character.
		//
		// Negative Format  Positive Format
		//
		// 0	($1)		0	$1 
		// 1	-$1			1	1$ 
		// 2	$-1			2	$ 1
		// 3	$1-			3	1 $
		// 4	(1$)
		// 5	-1$			These formats are taken directly from
		// 6	1-$			Microsoft's documentation regarding
		// 7	1$-			the international section of WIN.INI.
		// 8	-1 $		The SECCurrencyEdit uses them for the
		// 9	-$ 1		positive and negative formats given via
		// 10	$ 1-		the formatting object.

		BOOL IsLeadingZeroEnabled() const
			{ return m_bLeadingZero; }
		void EnableLeadingZero(BOOL b)
			{ m_bLeadingZero = b; }
		BOOL IsDecimalSeparatorLineEnabled() const
			{ return m_bDecimalSeparatorLine; }
		void EnableDecimalSeparatorLine(BOOL b)
			{ m_bDecimalSeparatorLine = b; }
		LPCTSTR GetMonetarySymbol() const
			{ return m_strMonetarySymbol; }
		void SetMonetarySymbol(LPCTSTR p)
			{ m_strMonetarySymbol = p; }
		TCHAR GetThousandSeparator() const
			{ return m_cThousandSeparator; }
		void SetThousandSeparator(TCHAR c)
			{ m_cThousandSeparator = c; }
		TCHAR GetDecimalSeparator() const
			{ return m_cDecimalSeparator; }
		void SetDecimalSeparator(TCHAR c)
			{ m_cDecimalSeparator = c; }
		TCHAR GetPaddingCharacter() const
			{ return m_cPaddingCharacter; }
		void SetPaddingCharacter(TCHAR c)
			{ m_cPaddingCharacter = c; }
		BOOL GetBackgroundColor(COLORREF& cr) const;
		void SetBackgroundColor(COLORREF cr);
		BOOL GetNegativeColor(COLORREF& cr) const;
		void SetNegativeColor(COLORREF cr);
		BOOL GetPositiveColor(COLORREF& cr) const;
		void SetPositiveColor(COLORREF cr);
		COLORREF GetDecimalSeparatorLineColor() const
			{ return m_crSeparator; }
		void SetDecimalSeparatorLineColor(COLORREF cr)
			{ m_crSeparator = cr; }
		int GetPositiveFormat() const
			{ return m_iPositiveFormat; }
		void SetPositiveFormat(int i)
			{ m_iPositiveFormat = i; }
		int GetNegativeFormat() const
			{ return m_iNegativeFormat; }
		void SetNegativeFormat(int i)
			{ m_iNegativeFormat = i; }
		int GetDecimalDigits() const
			{ return m_iDecimalDigits; }
		void SetDecimalDigits(int i)
			{ m_iDecimalDigits = i; }
		int GetFractionalDigits() const
			{ return m_iFractionalDigits; }
		void SetFractionalDigits(int i)
			{ m_iFractionalDigits = i; }

		// Reads values as necessary from registry, INI file, etc. to
		// match settings specified by user in Control Panel.

		void InitializeFromSystemDefaults();

		// The following method clears any previously specified custom 
		// color use.

		void ClearCustomColors()
			{ m_iColorUseFlags = 0; }

		// The following methods convert between a numeric value and a
		// string representation.  If customization beyond that required
		// by the basic Format object is requied, derive your own class
		// and override the following methods; then derive your own version 
		// of SECCurrencyEdit and override its CreateFormatObject method
		// to provide an object of your descendant class

		virtual BOOL ParseValue(LPCTSTR pcsz, double& d) const;
		virtual void FormatValue(double d, CString& str) const;

		// Operators - virtual to allow descendant handling.

		virtual Format& operator=(const Format& rhs);
		virtual BOOL operator==(const Format& rhs) const;
		virtual BOOL operator!=(const Format& rhs) const
			{ return !operator==(rhs); }

	private:
		BOOL m_bLeadingZero;			// leading 0 for numbers -1 < n < 1 ?
		BOOL m_bDecimalSeparatorLine;	// vertical line for decimal point?
		CString m_strMonetarySymbol;	// currency symbol (default: $)
		TCHAR m_cThousandSeparator;		// groups decimal digits (default: ,)
		TCHAR m_cDecimalSeparator;		// decimal point (default: .)
		TCHAR m_cPaddingCharacter;		// used to pad to left of decimal
		COLORREF m_crBackground;		// background color of control window
		COLORREF m_crNegative;			// color of text when value is negative
		COLORREF m_crPositive;			// color of text when value is negative
		COLORREF m_crSeparator;			// color of decimal separator line
		int m_iColorUseFlags;			// internal color flags variable
		int m_iPositiveFormat;			// WIN.INI format for positive values
		int m_iNegativeFormat;			// WIN.INI format for negative values
		int m_iDecimalDigits;			// number of characters left of decimal
		int m_iFractionalDigits;		// number of digits to right of decimal
	};

	// Construction/destruction.

	SECCurrencyEdit();
	virtual ~SECCurrencyEdit();

	// Replaces an existing edit control.

	BOOL Initialize(CWnd* pwndParent, UINT uiControlID);

	// Gets or sets the actual numeric value directly.

	void GetValue(double& d) const
		{ d = m_dValue; }
	void SetValue(double d, BOOL bRedraw = TRUE);

	// Gets of sets the display formatting data.

	void GetFormat(Format& f);
	void SetFormat(const Format& f);

	// The alignment is actually maintained via edit ES_* style values;
	// these methods are provided largely for convenience.

	enum Align { left = 0, center = 1, right = 2 };
	Align GetAlignment() const;
	void SetAlignment(Align a);

	// The read-only feature is actually mantained via the normal edit
	// control ES_READONLY style; these methods are provided largely
	// for convenience.  The EM_SETREADONLY message will also work.

	BOOL IsReadOnly() const;
	void SetReadOnly(BOOL b);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECCurrencyEdit)
	//}}AFX_VIRTUAL

protected:
	virtual BOOL ShowCalculator();

	// Override of the SECDropEdit for handling the button click.

	virtual void OnClicked()
		{ ShowCalculator(); }

	// Data structure used for maintaining selection details.
	// Two special settings should be noted: 0, -1 means select 
	// all and -1, 0 means no selection.

	typedef struct tagSelData
	{
		int iStart, iStop;
		tagSelData() : iStart(-1), iStop(0) {}
		tagSelData(const tagSelData& s) :
			iStart(s.iStart), iStop(s.iStop) {}
		void Set(int i, int j) { iStart = i; iStop = j; }
		void Clear() { iStart = -1; iStop = 0; }
		void Normalize()
		{
			int iTmp;
			if (iStart > iStop)
			{
				iTmp = iStop;
				iStop = iStart;
				iStart = iTmp;
			}
		}
		tagSelData& operator=(const tagSelData& rhs)
		{
			if (this != &rhs)
			{
				iStart = rhs.iStart;
				iStop = rhs.iStop;
			}
			return *this;
		}

	} SELDATA;

	// Data structure used for maintaining undo data.

	typedef struct tagUndoData
	{
		CString strText;
		int iCaretPos;
		SELDATA sel;

		tagUndoData() : iCaretPos(-1) {}
		tagUndoData(const tagUndoData& u) : 
			strText(u.strText), iCaretPos(u.iCaretPos), sel(u.sel) {}
		tagUndoData& operator=(const tagUndoData& rhs)
		{
			if (this != &rhs)
			{
				strText = rhs.strText;
				iCaretPos = rhs.iCaretPos;
				sel = rhs.sel;
			}
			return *this;
		}

	} UNDODATA;

	BOOL m_bCanUndo;		// undo buffer has something?
	BOOL m_bParseOnSetText;	// parse value on WM_SETTEXT?
	BOOL m_bRedraw;			// is redraw allowed?
	BOOL m_bMouseSelect;	// are we mouse selecting?
	UNDODATA m_Undo;		// undo buffer
	double m_dValue;		// numeric value
	Format* m_pFormat;		// current format settings
	HFONT m_hFont;			// font set via WM_SETFONT
	int m_iCaretPos;		// caret character position
	SELDATA m_Sel;			// selection information

	// Descendant classes may override the following members to provide
	// a custom formatting object, right-button popup menu, background 
	// drawing, save their own data for an undo operation, etc.

	virtual Format* CreateFormatObject() const;
	virtual CMenu* CreatePopupMenu() const;
	virtual void DeletePopupMenu(CMenu* p) const;
	virtual void DoBackspace();
	virtual void DoDelete();
	virtual void DoInsert(LPCTSTR pcsz);
	virtual void EraseBackground(CDC* pDC);
	virtual void SaveDataForUndo(LPCTSTR p = NULL, int iCaret = -1);
	virtual void StartEdit(int iCaretPos);
	virtual void DrawDecimalSeparatorLine(CDC& DC, int x);

	// Non-virtual members; descendant classes may utilize these, but should 
	// not override them.

	BOOL HasSel() const
		{ return m_Sel.iStart != -1 && m_Sel.iStop != m_Sel.iStart; }
	BOOL SetSel(int iStart, int iStop);
	BOOL WeAreMouseSelecting() const
		{ return m_bMouseSelect; }
	BOOL IsEntireNumberSelected() const;
	COLORREF GetBackgroundColor() const;
	COLORREF GetTextColor() const;
	int PointToCaretPos(const CPoint& pt);
	void CalculateTextRect(CRect& rc);
	void DoInsert(TCHAR c)
		{ DoInsert(CString(c)); }
	void GetSelectedText(CString& s) const;
	void MoveCaret(int iChar, BOOL bUpdateSel = FALSE);
	void PaintingCore(CDC& DC);
	void QueueRedraw(BOOL bEraseBackground = TRUE)
		{ if (m_bRedraw) InvalidateRect(NULL, bEraseBackground); }
	void ReplaceSelString(LPCTSTR pcszNew, CString& str) const;
	void SetWindowTextNoParse(LPCTSTR p);

	// Handlers provided by ClassWizard.

	//{{AFX_MSG(SECCurrencyEdit)
	afx_msg void OnPaint();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	// Edit control message handlers not provided by ClassWizard.

	afx_msg LRESULT OnCanUndo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmptyUndoBuffer(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetSel(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetReadOnly(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetSel(WPARAM wParam, LPARAM lParam);

	// Standard message handlers not provided by ClassWizard.

	afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCopy(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCut(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPaste(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetRedraw(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUndo(WPARAM wParam, LPARAM lParam);

	// Command handlers for right mouse button popup menu.

	afx_msg void OnEditUndo();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditClear();
	afx_msg void OnEditSelectAll();

	DECLARE_MESSAGE_MAP()
};

// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif	// WIN32

#endif	// __CURREDIT_H__
