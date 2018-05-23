// AutoCompleteSearcher.h
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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


#ifndef AUTO_COMPLETER_SEARCH
#define AUTO_COMPLETER_SEARCH

#include "ListCtrlEx.h"

#include "QCImageList.h"		// c_dalew 9/30/99 - Added so we can use QCImageList.


class AutoCompNode
{
private:
	CString m_Data;

protected:
	friend class AutoCompList;

	AutoCompNode* next;

	const char *GetData() { return m_Data; }
	AutoCompNode(const char* data);
};

class AutoCompList
{
private:
	CString m_Filename;
	AutoCompNode* m_Head;
	int m_size;
	int m_MaxSize;
	BOOL m_bDirty;

public:
	AutoCompList(const char* filename);
	~AutoCompList();

	void PullNamesFromStringAndAdd(const char* string);
	void SetMaxSize(int MaxSize){m_MaxSize = MaxSize;}
	bool ReadAutoCompleteList();
	void SaveToDisk();
	bool Add(const char* Data, bool ToEnd = false, bool bAddOnlyEmailIDs = false);
	bool Remove(const char* Data);
	CStringList* FindListOfACStartingWith(const char* Start);
	CStringList* FindListOfFilesStartingWith(const char* Start);
	const char* FindACStartingWith(const char* Start);
	CString AutoCompList::GetAllAC();
	bool Contains(const char* data);
};

AutoCompList* GetAutoCompList();
void DeleteAutoCompList();


class MyListCtrl;
class AutoCompleterListBox;

class DropDownListBox : public CDialog
{

public:
	MyListCtrl*				m_ListCtrl;
	QCImageList				m_imageList;		// c_dalew 9/30/99 - Changed CImageList to QCImageList.
	AutoCompleterListBox*	m_pAutoCompleter;
	CEdit*			m_pTheField;
	bool					m_UserHasSized;

public:
	DropDownListBox(CEdit* pField, AutoCompleterListBox* pAutocompleter);
	~DropDownListBox();
	void MoveWindow(CRect rect);

	int FindItem( LV_FINDINFO* pFindInfo, int nStart = -1);
	int GetCurSel();
	BOOL SetCheck(int nItem, BOOL fCheck = TRUE);
	BOOL GetCheck(int nItem);
	int GetItemCount();
	BOOL DeleteAllItems();
	BOOL InsertItem(int nItem, LPCSTR lpszItem, int nImage);
	BOOL SetItemData(int nItem, DWORD dwData);
	BOOL SetColumn(int nCol, const LVCOLUMN* pColumn);
	BOOL DeleteItem(int nItem);
	BOOL GetItem(LVITEM* pItem);
	DWORD GetItemData(int nItem);

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


class CHeaderField;
class DSEdit;

class AutoCompleterListBox
{
protected:
	AutoCompCEdit* m_pEditControl;
	DropDownListBox* m_AutoCompList;
	int m_MinWidth;
	bool m_HotRollover;
	bool m_bForceSelection;

public:
	void OnDeleteKey();
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
	void AddToACListBox(LPCTSTR string, int nPicture, DWORD NNItem = 0);

	void RemoveLastItem();
	void RemoveFirstItem();
	void OnArrowKey(bool Up, bool AndChangeEditField = false);
	void InitList(bool Showlist = true);
	void SetControlToItem(int nIndex, bool PreserveCase = true);
	void SetControlToItemDS(int nIndex, bool PreserveCase = true);
	void SelectionMade();
	BOOL IsPoppedUp();
	void TogglePoppedUpState();
	BOOL ShouldShow();
	BOOL ContainsItems();
	void SaveWindowHeight();
	BOOL SetCheck(int nItem, BOOL fCheck = TRUE);
	void InitListDSDropDown(bool Showlist = true);
	CEdit* GetEditControl() {return m_pEditControl;}
	void ForceSelection() { m_bForceSelection = true; }
};


class MyListCtrl : public CListCtrlEx
{
	DropDownListBox* pParent;
public:
	MyListCtrl(DropDownListBox* parent){pParent = parent;};
	~MyListCtrl(){};

	BOOL PreTranslateMessage( MSG* pMsg );
};


#endif
