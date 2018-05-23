#ifndef AUTO_COMPLETER_SEARCH
#define AUTO_COMPLETER_SEARCH

#include "ListCtrlEx.h"

class CHeaderView;
class CCompMessageFrame;
class CHeaderField;
class DropDownListBox;
class AutoCompleterListBox;
class DSEdit;

#ifdef _DEBUG
#ifdef new
#undef new
#endif
#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif
#endif

class MyListCtrl : public CListCtrlEx
{
	DropDownListBox* pParent;
public:
	MyListCtrl(DropDownListBox* parent){pParent = parent;};
	~MyListCtrl(){};

	BOOL PreTranslateMessage( MSG* pMsg );
#ifdef _DEBUG
        void * __cdecl
	operator new(size_t size, const char *file, int line)
	{
	    return ::new(file, line) char[size];
	}
        void * __cdecl
	operator new(size_t size, char *file, int line)
	{
	    return ::new(file, line) char[size];
	}
        void * __cdecl
	operator new(size_t size, int, const char *file,
				    int line)
	{
	    return ::new(file, line) char[size];
	}
        void * __cdecl
	operator new(size_t size, int, char *file, int line)
	{
	    return ::new(file, line) char[size];
	}
#else
        void * __cdecl operator new(size_t size)
	{
	    return ::new char[size];
	}
#endif
        void operator delete(void *pv)
	{
	    delete [] ((char *) pv);
	}
};

class AutoCompNode{

private:
	char*	m_Data;

protected:
	friend class AutoCompList;

	AutoCompNode* next;

	char *GetData();
	AutoCompNode(char *data);
	~AutoCompNode();
};

class AutoCompList
{
private:
	char * m_Filename;
	AutoCompNode* m_Head;
	int size;
	int m_MaxSize;

public:
	AutoCompList(char *filename);
	AutoCompList(CString filename);
	~AutoCompList();

	void PullNamesFromStringAndAdd(CString string);
	void SetMaxSize(int MaxSize){m_MaxSize = MaxSize;}
	bool ReadAutoCompleteList();
	void SaveToDisk();
	bool Add(char *Data, bool ToEnd = false);
	bool Remove(char *Data);
	CStringList *FindListOfACStartingWith(char* Start);
	CStringList *FindListOfFilesStartingWith(char* Start);
	char *FindACStartingWith(char* Start);
};

// The New Message AutoCompletion Object
extern AutoCompList*		g_AutoCompleter;

class DropDownListBox : public CDialog
{

public:
	MyListCtrl*				m_ListCtrl;
	CImageList				m_imageList;
	AutoCompleterListBox*	m_pAutoCompleter;
	CEdit*			m_pTheField;
	bool					m_UserHasSized;

public:
	DropDownListBox::DropDownListBox(CEdit* pField, AutoCompleterListBox* pAutocompleter);
	DropDownListBox::~DropDownListBox();
	void DropDownListBox::MoveWindow(CRect rect);

	int FindItem( LV_FINDINFO* pFindInfo, int nStart = -1);
	int GetCurSel();
	BOOL SetCheck(int nItem, BOOL fCheck = TRUE);
	BOOL GetCheck(int nItem);
	int GetItemCount();
	BOOL DeleteAllItems();
	BOOL InsertItem(int nItem, LPCSTR lpszItem, int nImage);
	BOOL SetColumn(int nCol, const LVCOLUMN* pColumn);
	BOOL DeleteItem(int nItem);
	BOOL GetItem(LVITEM* pItem);
	BOOL EnsureVisible(int nItem, BOOL bPartialOK);
	CString GetItemText(int nItem, int nSubItem);
	BOOL PreTranslateMessage( MSG* pMsg );
	MyListCtrl* GetListBox() {return m_ListCtrl;}
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DropDownListBox)
	//}}AFX_VIRTUAL
	// Generated message map functions
protected:
	//{{AFX_MSG(DropDownListBox)
		afx_msg void OnTimer(UINT nID);
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnSizing( UINT nSide, LPRECT lpRect );
		afx_msg void OnPaint( );
		afx_msg void OnSetFocus( CWnd* pOldWnd );
		afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
		afx_msg BOOL OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class AutoCompCEdit : public CEdit
{
	DECLARE_DYNAMIC( AutoCompCEdit )
	virtual void OnACListDisplayed(void) = 0;
	virtual void OnACListSelectionMade(void) = 0;
};

class AutoCompleterListBox
{
protected:
	AutoCompCEdit* m_pEditControl;
	DropDownListBox* m_AutoCompList;
	int m_MinWidth;
	bool m_HotRollover;

public:
	AutoCompleterListBox(CHeaderField* pHeaderField);
	AutoCompleterListBox(DSEdit* pDSQueryField, int minwidth = -1);
	~AutoCompleterListBox();

	void SetControlToCheck();
	void SetListToControlDS(bool SetToZeroIfNotFound = true);
	void SetListToControl(bool SetToZeroIfNotFound = true);
	bool isHotRollover(){return m_HotRollover;}
	void SetHotRollover(bool state){m_HotRollover = state;}
	void SetMinWidth(int width);
	void DoACListBox();
	void ReCalcACListBoxPos();
	void KillACListBox();
	void AddToEndOfACListBox(char *string, int nPicture);
	void RemoveLastItem();
	void RemoveFirstItem();
	void OnArrowKey(bool Up, bool AndChangeEditField = false);
	void InitList(bool Showlist = true);
	void SetControlToItem(int nIndex, bool PreserveCase = true);
	void SetControlToItemDS(int nIndex, bool PreserveCase = true);
	void SelectionMade();
	BOOL IsPoppedUp();
	void TogglePoppedUpState();
	BOOL ContainsMultItems();
	BOOL ContainsItems();
	void SaveWindowHeight();
	BOOL SetCheck(int nItem, BOOL fCheck = TRUE);
	void InitListDSDropDown(bool Showlist = true);
	CEdit* GetEditControl() {return m_pEditControl;}
};



#endif
