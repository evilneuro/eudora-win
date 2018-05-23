// SearchView.h : header file
//
#ifndef __SEARCHVIEW_H__
#define __SEARCHVIEW_H__

/////////////////////////////////////////////////////////////////////////////
// CSearchView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "3dformv.h"

#include "ListCtrlEx.h"
//#include "mboxtree.h"
#include "MBoxTreeCtrlCheck.h"
#include "LabelComboBox.h"

#include "SearchCriteria.h"
#include "SearchResult.h"

#define MAX_CRITERIA (5)

// --------------------------------------------------------------------------

class CResultsListCtrl : public CListCtrlEx
{
protected:	
	virtual int CompareItems(LPARAM lpOne, LPARAM lpTwo, int nCol);
};

// --------------------------------------------------------------------------

class SingleCritState;
class CMsgResult;

class CSearchView : public C3DFormView
{
	DECLARE_DYNCREATE(CSearchView)

public:
	static int CompareResultItems(LPARAM lpOne, LPARAM lpTwo, int nCol);

protected:
	CSearchView();			// protected constructor used by dynamic creation

	virtual ~CSearchView();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
			void OnInitialUpdate();

	enum { COLUMN_MAILBOX = 0, COLUMN_WHO, COLUMN_DATE, COLUMN_SUBJECT };
	enum { TAB_RESULTS_IDX = 0, TAB_MAILBOXES_IDX = 1 };
	enum { TAB_AND_IDX = 0, TAB_OR_IDX = 1 };

	void UpdateMbxText();
	void UpdateResultsText();
	void UpdateStatusText();
	void UpdateSearchBtn();
	void UpdateMoreLessBtn();
	void UpdateAndOrBtns();
	void UpdateTabContents();

	void ResizeColumns();
	void ResizeControls(int cx, int cy);

	void UpdateState(int nIdx); // Called when category changes
	void InitializeCriteriaCtrls(int nIdx);
	void ResizeCriteriaCtrls(int nIdx, int cx, int cy);
	void SetupDynamicCriteriaCtrls(int nIdx); // Call when category changes

	int MoveControl(CWnd *pWnd, int nLeft, int nRight, int nTop);
	void StretchTopControl(CWnd *pWnd, int nTop);

	bool IsCriteriaValid(int nIdx);
	bool GetCriteria(int nIdx, SearchCriteria &criteria);
	bool DoSearch();
	bool GetCtrlCurItemData(CComboBox &cb, DWORD &dw);
	bool GetCtrlCurItemText(CComboBox &cb, CString &str);

	BOOL OpenMsg(CMsgResult *pMR);
	BOOL OpenTOC(CMsgResult *pMR);
	BOOL DeleteMsg(CMsgResult *pMR, int nIdx);

	void AddResult(const SearchResult &result);
	void ClearResults(bool bPump = true);
	bool RemoveCriteria();
	void HideAllControls();
	void HideControls(int nIdx);
	bool AddNewCriteria();
	void OnCategoryChange(int nIdx);
	void ResizeAllCriteriaCtrls();
	void ResizeCriteriaCtrls(int nIdx);
	void UpdateCritBotPos();
	void AdjustControlTopPos();
	void ResizeTabContents();
	void AdjustWinMinHeight(int nMinWinHeight);
	void ClearList();
	void InitMem();
	void SetCriteriaFocus(int nIdx);

	BOOL UpdateListInfo(CSummary *pSum, int nIdx);
	BOOL UpdateListText(const SearchResult &result, int nIdx);

	CMsgResult *NewMsgResult(const SearchResult &result);

	bool DeleteSelItems();

	bool DoDelete(CSummary *pSum);
	bool DoDelete(/* const */ list<CSummary *> &SumList);
	
	bool RemoveFromResultsList(int nIdx);
	bool RemoveFromResultsList(/* const */ list<int> &IdxList);

	bool VerifyDelete(CSummary *pSum);
	bool VerifyDelete(/* const */ list<CSummary *> &SumList);
	bool VerifyDelete(bool bUnread, bool bQueued, bool bSendable);
	
	bool GetSumList(list<CSummary *> &SumList, /* const */ list<int> &IdxList);
	BOOL PumpEsc(DWORD nDeltaTick); // Pump the msg queue

	bool LoadCriteria();
	bool SaveCriteria();
	bool GetCriteriaIniString(int nIdx, CString &str);
	bool SetCriteriaIniString(int nIdx, LPCSTR str);

	// ----- variables -----

	char m_SaveStrSeperator;

	BOOL m_bInitilized;
	UINT m_nCheckCount;
	int m_nFoundCount;
	int m_nIMAPSkippedCount;

	BOOL m_bShowFoundCount;

	bool m_UseFastWay;
	list<CMsgResult *> m_MsgResultList;
	CMsgResult *m_MsgResultArr;
	unsigned int m_MsgResultArrCount;

	MultSearchCriteria m_MSC;
	
	int m_CurCritCount;
	SingleCritState *m_CritState;
	bool m_bCritInitd[MAX_CRITERIA];

	int m_CriteriaBottomPos;

	// Ctrl Vert Offsets -- the results list and mbx tree are displayed inside tab ctrl (so don't need em here)
	int m_OffsetMoreBtn, m_OffsetLessBtn, m_OffsetAndRadio, m_OffsetOrRadio, m_OffsetBeginBtn, m_OffsetTabCtrl, m_OffsetResultsStatic;
	bool m_bInitdOffsets;
	CSize m_MinSize;

	bool m_bInitFinal;

	// Generated message map functions
	//{{AFX_MSG(CSearchView)
	afx_msg LONG OnMsgMailboxSelect(WPARAM, LPARAM);
	afx_msg LONG OnInitFinal(WPARAM, LPARAM);
	afx_msg void OnOk();
	afx_msg void OnMoreBtn();
	afx_msg void OnLessBtn();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTabSelchange(NMHDR* pNMHDR, LRESULT* pResult); 
	afx_msg LONG OnMsgListDblClk(WPARAM, LPARAM);
	afx_msg LONG OnMsgListReturnKey(WPARAM, LPARAM);
	afx_msg LONG OnMsgListDeleteKey(WPARAM, LPARAM);
	afx_msg LONG OnMsgListRightClick(WPARAM, LPARAM);
	afx_msg LONG OnMsgTreeCheckChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSelchangeCriteriaCategoryCombo1();
	afx_msg void OnSelchangeCriteriaCategoryCombo2();
	afx_msg void OnSelchangeCriteriaCategoryCombo3();
	afx_msg void OnSelchangeCriteriaCategoryCombo4();
	afx_msg void OnSelchangeCriteriaCategoryCombo5();
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	afx_msg void OnChangeTextEdit1();
	afx_msg void OnChangeTextEdit2();
	afx_msg void OnChangeTextEdit3();
	afx_msg void OnChangeTextEdit4();
	afx_msg void OnChangeTextEdit5();
	//}}AFX_MSG

	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()


public:
	//{{AFX_DATA(CSearchView)
	enum { IDD = IDR_SEARCH_WND };
	CStatic	m_NumStatic[MAX_CRITERIA];
	CEdit	m_NumEdit[MAX_CRITERIA];
	CSpinButtonCtrl	m_NumSpin[MAX_CRITERIA];
	CComboBox	m_CategoryCombo[MAX_CRITERIA];
	CComboBox	m_TextCompareCombo[MAX_CRITERIA];
	CComboBox	m_EqualCompareCombo[MAX_CRITERIA];
	CComboBox	m_NumCompareCombo[MAX_CRITERIA];
	CComboBox	m_DateCompareCombo[MAX_CRITERIA];
	CEdit		m_TextEdit[MAX_CRITERIA];
	CComboBox	m_StatusCombo[MAX_CRITERIA];
	CLabelComboBox	m_LabelCombo[MAX_CRITERIA];
	CComboBox	m_PersonaCombo[MAX_CRITERIA];
	CComboBox	m_PriorityCombo[MAX_CRITERIA];
	SECDateTimeCtrl m_DateTimeCtrl[MAX_CRITERIA];

	CResultsListCtrl m_ResultsList;
	CTabCtrl m_ResultsMbxTabCtrl;
	CMBoxTreeCtrlCheck m_MBoxTree;
	CStatic m_ResultsStatic;
	CButton m_AndRadioBtn, m_OrRadioBtn;
	CButton m_BeginBtn, m_MoreBtn, m_LessBtn;
	//}}AFX_DATA

	bool ProcessResults(list<SearchResult> &results);
};

/////////////////////////////////////////////////////////////////////////////

#endif // __SEARCHVIEW_H__
