// SearchView.h : header file
//
// Copyright (c) 1999-2005 by QUALCOMM, Incorporated
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



#ifndef __SEARCHVIEW_H__
#define __SEARCHVIEW_H__

/////////////////////////////////////////////////////////////////////////////
// CSearchView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "3dformv.h"

#include "CoolButton.h"
#include "ListCtrlEx.h"
#include "MBoxTreeCtrlCheck.h"
#include "LabelComboBox.h"

#include "SearchCriteria.h"
#include "SearchResult.h"
#include "SearchManager.h"

#include "QICommandClient.h"

#include "xml_parser_base.h"


#define MAX_CONTROLS_CRITERIA (5)

// --------------------------------------------------------------------------

class CResultsListCtrl : public CListCtrlEx
{
  protected:	
	virtual int							CompareItems(LPARAM lpOne, LPARAM lpTwo, int nCol);
};

// --------------------------------------------------------------------------

class SingleCritState;
class CMsgResult;

class CSearchView : public C3DFormView, public QICommandClient
{
	DECLARE_DYNCREATE(CSearchView)

  public:
    //  Constants
	static const long					kX1IconWidth = 23;
	static const long					kX1IconHeight = 16;

	class XMLParser : public XmlParser
	{
	  public:
		//	Constants
		static const long				kDataFormatVersion = 1;

		static const char *				kXMLBaseContainer;
		static const char *				kKeyDataFormatVersion;
		static const char *				kKeySearchCriteriaCount;
		static const char *				kKeySearchCriteria;
		static const char *				kKeySearchCriterion;
		static const char *				kKeySortColumns;
		static const char *				kKeySortBy;

										XMLParser();

		//	Accessors
		long							GetSearchCriteriaCount() const { return m_nSearchCriteriaCount; }
		const CString &					GetSearchCriterion(short in_nIndex) const { return m_szSearchCriteria[in_nIndex]; }

	  private:
		enum XMLElementIDs
		{
			id_baseContainer,
			id_keyDataFormatVersion,
			id_keySearchCriteriaCount,
			id_keySearchCriteria,
			id_keySearchCriterion,
			id_keySortColumns,
			id_keySortBy
		};

		static ElementMap				elementMapArr[];

		bool							initElementMap(
											ElementMap **			out_pMap);
		void							handleData(
											int						in_nID,
											const char *			in_pData,
											int						in_nDataLength);
		void							ProcessDataForID(
											int						in_nID);
		int								startElement(
											int						in_nID,
											const char *			in_szName,
											const char **			in_AttributeArr);
		int								endElement(
											int						in_nID,
											const char *			in_szName);

		deque<int>						m_elementIDsQueue;
		CString							m_szElementData;
		long							m_nSearchCriteriaCount;
		long							m_nCurrentSearchCriterion;
		CString							m_szSearchCriteria[MAX_CONTROLS_CRITERIA];
	};

	bool								ShouldUseIndexedSearch() const
											{ return SearchManager::Instance()->ShouldUseIndexedSearch() && m_bUseIndexedSearchIfAvailable; }

	static int							CompareResultItems(
											LPARAM						lpOne,
											LPARAM						lpTwo,
											int							nCol);

	// The sole QICommandClient interface
	virtual void						Notify(
											QCCommandObject *			pObject,
											COMMAND_ACTION_TYPE			theAction,
											void *						pData = NULL);

	static void							NotifyIndexedSearchAvailabilityChanged();

	static void							UpdatePathnamesInAllSearchResults(
											const CString &				in_szOldPathname, 
											const CString &				in_szNewPathname);
	void								UpdatePathnamesInSearchResults(
											const CString &				in_szOldPathname, 
											const CString &				in_szNewPathname);
	static void							UpdateMailboxNamesInAllSearchResults(
											const CString &				in_szOldMailboxPath, 
											const char *				in_szNewMailboxPath,
											const char *				in_szNewMailboxName);
	void								UpdateMailboxNamesInSearchResults(
											const CString &				in_szOldMailboxPath, 
											const char *				in_szNewMailboxPath,
											const char *				in_szNewMailboxName);

	void								GetSuggestedFileName(
											CString &					out_strSuggestedFileName);
	void								LoadCriteriaFromXML(
											const char *				in_pszPathName);
	void								WriteCurrentSearchCriteria(
											const char *				in_pszPathName);

  protected:
										CSearchView();			// protected constructor used by dynamic creation

	virtual								~CSearchView();
	virtual void						DoDataExchange(
											CDataExchange *				pDX);	// DDX/DDV support
	void								OnInitialUpdate();

	enum { COLUMN_MAILBOX = 0, COLUMN_WHO, COLUMN_DATE, COLUMN_SUBJECT };
	enum { TAB_RESULTS_IDX = 0, TAB_MAILBOXES_IDX = 1 };
	enum { TAB_AND_IDX = 0, TAB_OR_IDX = 1 };

	void								UpdateMbxText();
	void								UpdateResultsText();
	void								UpdateStatusText();
	void								UpdateSearchBtn();
	void								UpdateSearchBtnMenu();
	void								UpdateMoreLessBtn();
	void								UpdateAndOrBtns();
	void								UpdateTabContents();

	void								ResizeColumns();
	void								ResizeControls(int cx, int cy);

	void								UpdateState(int nIdx); // Called when category changes
	void								InitializeCriteriaCtrls(int nIdx);
	void								InitializeTextCompareCombo(int nIndex);
	void								ResizeCriteriaCtrls(int nIdx, int cx, int cy);
	void								SetupDynamicCriteriaCtrls(int nIdx); // Call when category changes
	void								InitializeCriteriaValues(int nIdx); // Fill in any defaults when changing category

	BOOL								CreateCriteriaCtrls(int nIdx);

	int									MoveControl(CWnd *pWnd, int nLeft, int nRight, int nTop);
	void								StretchTopControl(CWnd *pWnd, int nTop);
	void								SetComboDropDownSize(CComboBox *pComboBox);

	bool								IsCriteriaValid(int nIdx);
	bool								GetCriteria(int nIdx, SearchCriteria &criteria);
	bool								DoSearch();
	bool								GetCtrlCurItemData(CComboBox &cb, DWORD &dw);
	bool								GetCtrlCurItemText(CComboBox &cb, CString &str);

	BOOL								OpenMsg(CMsgResult *pMR);
	BOOL								OpenTOC(CMsgResult * pMR, CObArray * pArrayOpenedTOCs);

	void								AddResult(const SearchResult &result);
	void								ClearResults(bool bPump = true);
	bool								RemoveCriteria();
	void								HideAllControls();
	void								HideControls(int nIdx);
	bool								AddNewCriteria();
	void								OnCategoryChange(int nIdx);
	void								ResizeAllCriteriaCtrls();
	void								ResizeCriteriaCtrls(int nIdx);
	void								UpdateCritBotPos();
	void								AdjustControlTopPos();
	void								ResizeTabContents();
	void								AdjustWinMinHeight(int nMinWinHeight);
	void								ClearList();
	void								InitMem();
	void								SetCriteriaFocus(int nIdx);

	BOOL								UpdateListInfo(CSummary *pSum, int nIdx);
	BOOL								UpdateListText(const SearchResult &result, int nIdx);

	CMsgResult *						NewMsgResult(const SearchResult &result);
	
	bool								DoDelete(CSummary * pSum, int iMsgIdx);
	
	bool								RemoveFromResultsList(int nIdx);
	bool								RemoveFromResultsList(/* const */ std::list<int> &IdxList);

	bool								VerifyDelete(CSummary *pSum);
	bool								VerifyDelete(/* const */ std::list<CSummary *> &SumList);
	bool								VerifyDelete(bool bUnread, bool bQueued, bool bSendable);
	
	bool								GetSumList(
											std::list<CSummary *> &		SumList,
											std::list<int> &			IdxList,
											bool						bAllowTrash,
											bool						bAllowOut,
											bool						bAllowJunk,
											int *						iNumRemoved);
	bool								GetJunkSumList(
											std::list<CSummary *> &		SumList,
											std::list<int> &			IdxList,
											int *						iNumRemoved);
	BOOL								PumpEsc(DWORD nDeltaTick); // Pump the msg queue

	void								ReloadCriteria();
	bool								LoadCriteria();
	bool								SaveCriteria();
	bool								GetCriteriaIniString(int nIdx, CString &str);
	bool								SetCriteriaIniString(int nIdx, LPCSTR str);

	// ----- variables -----

	char								m_SaveStrSeperator;

	BOOL								m_bInitilized;
	UINT								m_nCheckCount;
	int									m_nFoundCount;
	int									m_nIMAPSkippedCount;

	BOOL								m_bShowFoundCount;

	bool								m_UseFastWay;
	std::list<CMsgResult *>				m_MsgResultList;
	CMsgResult *						m_MsgResultArr;
	unsigned int						m_MsgResultArrCount;

	bool								m_bUseIndexedSearchIfAvailable;
	bool								m_bShouldIncludeDeletedIMAPMessages;
	bool								m_bSearchButtonMenuInitialized;

	MultSearchCriteria m_MSC;
	
	int m_CurCritCount;
	SingleCritState *m_CritState;
	bool m_bCritInitd[MAX_CONTROLS_CRITERIA];
	bool 								m_bCritCreated[MAX_CONTROLS_CRITERIA];

	int 								m_CriteriaBottomPos;

	// Ctrl Vert Offsets -- the results list and mbx tree are displayed inside tab ctrl (so don't need em here)
    int 								m_VertOffsetX1Static;
    int 								m_VertOffsetX1Icon;
    int 								m_HorizOffsetX1Static;
    int 								m_HorizOffsetX1Icon;
    int 								m_HorizOffsetResultsStatic;
	int									m_OffsetMoreBtn;
	int									m_OffsetLessBtn;
	int									m_OffsetOptionsBtn;
	int									m_OffsetAndRadio;
	int									m_OffsetOrRadio;
	int									m_OffsetBeginBtn;
	int									m_OffsetTabCtrl;
	int									m_OffsetResultsStatic;
	bool 								m_bInitdOffsets;
	CSize 								m_MinSize;

	bool 								m_bInitFinal;

	QCCommandObject	*					m_pNewMboxCommand;

	void								HandleSelectMessage(
											QCMailboxCommand *				in_pMbxCmd,
											bool							in_bSelectParentFolder = false);

	// Generated message map functions
	//{{AFX_MSG(CSearchView)
	afx_msg LONG						OnMsgReloadCriteria(WPARAM, LPARAM);
	afx_msg LONG						OnMsgMailboxSelect(WPARAM, LPARAM);
	afx_msg LONG						OnMsgParentFolderSelect(WPARAM, LPARAM);
	afx_msg LONG						OnMsgAllMailboxesSelect(WPARAM, LPARAM);
	afx_msg void						OnOk();
	afx_msg void						OnMoreBtn();
	afx_msg void						OnLessBtn();
	afx_msg void						OnDestroy();
	afx_msg void						OnSize(UINT nType, int cx, int cy);
	afx_msg void						OnTabSelchange(NMHDR* pNMHDR, LRESULT* pResult); 
	afx_msg LONG						OnMsgListDblClk(WPARAM, LPARAM);
	afx_msg LONG						OnMsgListReturnKey(WPARAM, LPARAM);
	afx_msg LONG						OnMsgListDeleteKey(WPARAM, LPARAM);
	afx_msg LONG						OnMsgListRightClick(WPARAM, LPARAM);
	afx_msg LONG						OnMsgTreeCheckChange(WPARAM wParam, LPARAM lParam);
	afx_msg void						OnSelchangeCriteriaCategoryCombo1();
	afx_msg void						OnSelchangeCriteriaCategoryCombo2();
	afx_msg void						OnSelchangeCriteriaCategoryCombo3();
	afx_msg void						OnSelchangeCriteriaCategoryCombo4();
	afx_msg void						OnSelchangeCriteriaCategoryCombo5();
	afx_msg void						OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void						OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	afx_msg void						OnUpdateNeedsSelection(CCmdUI* pCmdUI);
	afx_msg void						OnUpdateDelete(CCmdUI* pCmdUI);
	afx_msg void						OnUpdateJunk(CCmdUI* pCmdUI);
	afx_msg void						OnUpdateNotJunk(CCmdUI* pCmdUI);
	afx_msg void						OnUpdateResponse(CCmdUI* pCmdUI);
	afx_msg void						OnUpdateDynamicCommand(CCmdUI* pCmdUI);
	afx_msg BOOL						OnComposeMessage(UINT uID);
	afx_msg BOOL						OnOpenCommand(UINT uID);
	afx_msg void						OnDeleteMessages();
	afx_msg BOOL						OnJunkCommand(UINT nID);
	afx_msg BOOL						OnDynamicCommand(UINT uID);
	afx_msg void						OnChangeTextEdit1();
	afx_msg void						OnChangeTextEdit2();
	afx_msg void						OnChangeTextEdit3();
	afx_msg void						OnChangeTextEdit4();
	afx_msg void						OnChangeTextEdit5();	
	afx_msg void						OnEditSelectAll();
	afx_msg void						OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void						OnUpdateUseIndexedSearch(CCmdUI* pCmdUI);
	afx_msg void						OnUpdateFindDeletedIMAPMessages(CCmdUI* pCmdUI);
	afx_msg void						OnUseIndexedSearch();
	afx_msg void						OnFindDeletedIMAPMessages();
	//}}AFX_MSG

	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()



  public:
	enum { IDD = IDR_SEARCH_WND };
	CStatic								m_NumStatic[MAX_CONTROLS_CRITERIA];
	CEdit								m_NumEdit[MAX_CONTROLS_CRITERIA];
	CSpinButtonCtrl						m_NumSpin[MAX_CONTROLS_CRITERIA];
	CComboBox							m_CategoryCombo[MAX_CONTROLS_CRITERIA];
	CComboBox							m_TextCompareCombo[MAX_CONTROLS_CRITERIA];
	CComboBox							m_EqualCompareCombo[MAX_CONTROLS_CRITERIA];
	CComboBox							m_NumCompareCombo[MAX_CONTROLS_CRITERIA];
	CComboBox							m_DateCompareCombo[MAX_CONTROLS_CRITERIA];
	CEdit								m_TextEdit[MAX_CONTROLS_CRITERIA];
	CComboBox							m_StatusCombo[MAX_CONTROLS_CRITERIA];
	CLabelComboBox						m_LabelCombo[MAX_CONTROLS_CRITERIA];
	CComboBox							m_PersonaCombo[MAX_CONTROLS_CRITERIA];
	CComboBox							m_PriorityCombo[MAX_CONTROLS_CRITERIA];
	SECDateTimeCtrl						m_DateTimeCtrl[MAX_CONTROLS_CRITERIA];

	CResultsListCtrl					m_ResultsList;
	CTabCtrl							m_ResultsMbxTabCtrl;
	CMBoxTreeCtrlCheck					m_MBoxTree;
	CStatic								m_ResultsStatic;
    CStatic								m_PoweredByX1Static;
    CStatic								m_PoweredByX1Icon;
	CButton								m_AndRadioBtn;
	CButton								m_OrRadioBtn;
	CCoolButton							m_BeginBtn;
	CButton								m_MoreBtn;
	CButton								m_LessBtn;
	CCoolButton							m_OptionsBtn;

	virtual BOOL						OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	bool								ProcessResults(std::deque<SearchResult> &results);
};


#endif // __SEARCHVIEW_H__
