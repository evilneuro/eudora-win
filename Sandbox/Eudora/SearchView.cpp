// SearchView.cpp : implementation file
//

#include "stdafx.h"

#include <dos.h>

#include "eudora.h"
#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "fileutil.h"
#include "summary.h" 
#include "doc.h"
#include "ListCtrlEx.h"
#include "tocdoc.h"
#include "msgdoc.h"
#include "msgframe.h"
#include "mdichild.h"
#include "SearchDoc.h"
//#include "mboxtree.h"
#include "QCMailboxDirector.h"
#include "MBoxTreeCtrlCheck.h"
#include "QCMailboxCommand.h"
#include "TocView.h"
#include "Persona.h"
#include "LabelComboBox.h"
#include "guiutils.h" // ErrorDialog, EscapePressed
// #include "QCUtils.h" // RemovePrefixMT
#include "SearchCriteria.h"
#include "SearchEngine.h"
#include "SearchResult.h"
#include "QCFindMgr.h"

#include "Progress.h"
#include "EudoraMsgs.h"

#include "SearchView.h"

// --------------------------------------------------------------------------

extern QCMailboxDirector		g_theMailboxDirector;

// --------------------------------------------------------------------------

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// --------------------------------------------------------------------------

#ifdef _DEBUG
// #define _MY_TRACE_
#define _MY_TICK_TRACE_
#endif // _DEBUG

#define CONTROL_EDGE_SPACING		(5)
#define STATUS_TEXT_MIN_WIDTH		(20)
#define MAX_LISTCOUNT				(SHRT_MAX)

#define TICK_DELTA                  (100)

UINT msgInitFinal = ::RegisterWindowMessage("msgInitFinal");

// --------------------------------------------------------------------------

inline BOOL ControlDown()
  	{ return ((GetKeyState(VK_CONTROL) < 0) ? TRUE : FALSE); }

inline BOOL RightShift()
  	{ return ((GetKeyState(VK_RSHIFT) < 0) ? TRUE : FALSE); }

// --------------------------------------------------------------------------

void MyTrace(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vsntprintf(szBuffer, (sizeof(szBuffer)/sizeof(TCHAR)), lpszFormat, args);

	// was there an error? was the expanded string too long?
	ASSERT(nBuf >= 0);

	OutputDebugString(szBuffer);

	va_end(args);
}

// --------------------------------------------------------------------------

class CMsgResult
{
public:
	CMsgResult()
		: m_sPath(""), m_nMsgID(0), m_sSearchStr(""),
			m_sMbxName(""), m_sSubject(""), m_sWho(""), m_DateSecs(0) { }

	CMsgResult(const SearchResult &result)
		: m_sPath(result.GetMbxPath()), m_nMsgID(result.GetMsgID()), m_sSearchStr(""),
			m_sMbxName(result.GetMbxPath()),
			m_sWho(result.GetWho()), m_DateSecs(result.GetSeconds())
	{
		// Instead of saving the original subject, we only
		// save what we are going to need for sorting
		m_sSubject = RemoveSubjectPrefixMT((LPCSTR)result.GetSubject());
	}

	CMsgResult &operator=(const CMsgResult &copy)
	{
		m_sPath = copy.m_sPath;
		m_sSearchStr = copy.m_sSearchStr;
		m_nMsgID = copy.m_nMsgID;
		m_sMbxName = copy.m_sMbxName;
		m_sSubject = copy.m_sSubject;
		m_sWho = copy.m_sWho;
		m_DateSecs = copy.m_DateSecs;

		return (*this);
	}

	CString m_sPath, m_sSearchStr;
	long m_nMsgID;

	// Sorting stuff -- these are only used in sorting, not display
	CString m_sMbxName, m_sSubject, m_sWho;
	long m_DateSecs;
}; // CMsgResult

// --------------------------------------------------------------------------

class SingleCritState
{
public:
	SingleCritState();
	SingleCritState(const SingleCritState &); // Copy constructor
	SingleCritState(const CriteriaObject &); // Init constructor
	~SingleCritState();

	SingleCritState &operator=(const SingleCritState &);
	bool UpdateObj(const CriteriaObject &obj);

	CriteriaObject m_CurObj;
	CriteriaVerbType m_CurVerbType;
	CriteriaValueType m_CurValueType;
	bool m_bCurShowUnits;
	CString m_UnitsStr;
};

// --------------------------------------------------------------------------

int CResultsListCtrl::CompareItems(LPARAM lpOne, LPARAM lpTwo, int nCol)
{
	return (CSearchView::CompareResultItems(lpOne, lpTwo, nCol));
}

int CSearchView::CompareResultItems(LPARAM lpOne, LPARAM lpTwo, int nCol)
{
	CMsgResult *pMR1 = (CMsgResult *) lpOne;
	CMsgResult *pMR2 = (CMsgResult *) lpTwo;

	switch (nCol)
	{
		case COLUMN_MAILBOX:
		{
			return (pMR1->m_sMbxName.CompareNoCase(pMR2->m_sMbxName));
		}
		break;

		case COLUMN_WHO:
		{
			return (pMR1->m_sWho.CompareNoCase(pMR2->m_sWho));
		}
		break;

		case COLUMN_DATE:
		{
			const long nSec1 = pMR1->m_DateSecs;
			const long nSec2 = pMR2->m_DateSecs;

			return ((nSec1 == nSec2) ? 0 : (nSec1 < nSec2 ? (-1) : (+1)));
		}
		break;

		case COLUMN_SUBJECT:
		{
			LPCSTR pSub1 = (LPCSTR) pMR1->m_sSubject;
			LPCSTR pSub2 = (LPCSTR) pMR2->m_sSubject;

			return (stricmp(pSub1, pSub2));
		}
		break;
	}

	ASSERT(0);
	return (0);
}


// --------------------------------------------------------------------------
// CSearchView

IMPLEMENT_DYNCREATE(CSearchView, C3DFormView)

// --------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CSearchView, C3DFormView)
	//{{AFX_MSG_MAP(CSearchView)
	ON_MESSAGE(msgFindMsgMaiboxSel, OnMsgMailboxSelect)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)
	ON_REGISTERED_MESSAGE(msgInitFinal, OnInitFinal)
	ON_BN_CLICKED(IDC_SEARCHWND_BEGIN_BTN, OnOk)
	ON_BN_CLICKED(IDC_SEARCHWND_MORE_BTN, OnMoreBtn)
	ON_BN_CLICKED(IDC_SEARCHWND_LESS_BTN, OnLessBtn)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_SEARCHWND_RESULTS_TAB, OnTabSelchange)
	ON_REGISTERED_MESSAGE(msgListCtrlEx_LBtnDblClk, OnMsgListDblClk)
	ON_REGISTERED_MESSAGE(msgListCtrlEx_ReturnKey, OnMsgListReturnKey)
	ON_REGISTERED_MESSAGE(msgListCtrlEx_DeleteKey, OnMsgListDeleteKey)
	ON_REGISTERED_MESSAGE(msgListCtrlEx_RBtn, OnMsgListRightClick)
	ON_REGISTERED_MESSAGE(msgMBoxTreeCtrlCheck_CheckChange, OnMsgTreeCheckChange)
	ON_CBN_SELCHANGE(IDC_CRITERIA_CATEGORY_COMBO_1, OnSelchangeCriteriaCategoryCombo1)
	ON_CBN_SELCHANGE(IDC_CRITERIA_CATEGORY_COMBO_2, OnSelchangeCriteriaCategoryCombo2)
	ON_CBN_SELCHANGE(IDC_CRITERIA_CATEGORY_COMBO_3, OnSelchangeCriteriaCategoryCombo3)
	ON_CBN_SELCHANGE(IDC_CRITERIA_CATEGORY_COMBO_4, OnSelchangeCriteriaCategoryCombo4)
	ON_CBN_SELCHANGE(IDC_CRITERIA_CATEGORY_COMBO_5, OnSelchangeCriteriaCategoryCombo5)
	ON_EN_CHANGE(IDC_CRITERIA_TEXT_EDIT_1, OnChangeTextEdit1)
	ON_EN_CHANGE(IDC_CRITERIA_TEXT_EDIT_2, OnChangeTextEdit2)
	ON_EN_CHANGE(IDC_CRITERIA_TEXT_EDIT_3, OnChangeTextEdit3)
	ON_EN_CHANGE(IDC_CRITERIA_TEXT_EDIT_4, OnChangeTextEdit4)
	ON_EN_CHANGE(IDC_CRITERIA_TEXT_EDIT_5, OnChangeTextEdit5)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// --------------------------------------------------------------------------
CSearchView::CSearchView()
	: C3DFormView(CSearchView::IDD), m_bInitilized(FALSE), m_nCheckCount(0), m_nFoundCount(0), m_nIMAPSkippedCount(0), m_bShowFoundCount(FALSE),
	  m_CurCritCount(0), m_CriteriaBottomPos(0), m_bInitdOffsets(false), m_bInitFinal(false),
	  m_MsgResultArr(NULL), m_MsgResultArrCount(0), m_UseFastWay(true)
{
	//{{AFX_DATA_INIT(CSearchView)
	//}}AFX_DATA_INIT

//	if (RightShift())
		m_UseFastWay = false; // Fast way may be unsafe; use slow way for now

#ifdef _MY_TICK_TRACE_
	if (m_UseFastWay)
		::MyTrace("CSearchView initializing with FAST code.\n");
	else
		::MyTrace("CSearchView initializing with SLOW code.\n");
#endif // _MY_TICK_TRACE_

	m_CritState = new SingleCritState[MAX_CRITERIA];

	for (int idx=0; idx < MAX_CRITERIA; idx++)
		m_bCritInitd[idx] = false;
}

// --------------------------------------------------------------------------

CSearchView::~CSearchView()
{
	delete[] m_CritState;
	m_CritState = NULL;
}

// --------------------------------------------------------------------------

void 
CSearchView::DoDataExchange(CDataExchange* pDX)
{
	C3DFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSearchView)
	DDX_Control(pDX, IDC_SEARCHWND_RESULTS_LIST, m_ResultsList);
	DDX_Control(pDX, IDC_SEARCHWND_RESULTS_TAB, m_ResultsMbxTabCtrl);
	DDX_Control(pDX, IDC_SEARCHWND_MAILBOX_TREE, m_MBoxTree);
	DDX_Control(pDX, IDC_SEARCHWND_RESULTS_STATIC, m_ResultsStatic);
	DDX_Control(pDX, IDC_SEARCHWND_BEGIN_BTN, m_BeginBtn);
	DDX_Control(pDX, IDC_SEARCHWND_MORE_BTN, m_MoreBtn);
	DDX_Control(pDX, IDC_SEARCHWND_LESS_BTN, m_LessBtn);
	DDX_Control(pDX, IDC_SEARCHWND_AND_RADIO, m_AndRadioBtn);
	DDX_Control(pDX, IDC_SEARCHWND_OR_RADIO, m_OrRadioBtn);

	DDX_Control(pDX, IDC_CRITERIA_NUM_UNITS_STATIC_1, m_NumStatic[0]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_EDIT_1, m_NumEdit[0]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_SPIN_1, m_NumSpin[0]);
	DDX_Control(pDX, IDC_CRITERIA_CATEGORY_COMBO_1, m_CategoryCombo[0]);
	DDX_Control(pDX, IDC_CRITERIA_TEXT_COMPARE_COMBO_1, m_TextCompareCombo[0]);
	DDX_Control(pDX, IDC_CRITERIA_EQUAL_COMBO_1, m_EqualCompareCombo[0]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_COMPARE_COMBO_1, m_NumCompareCombo[0]);
	DDX_Control(pDX, IDC_CRITERIA_DATE_COMPARE_COMBO_1, m_DateCompareCombo[0]);
	DDX_Control(pDX, IDC_CRITERIA_TEXT_EDIT_1, m_TextEdit[0]);
	DDX_Control(pDX, IDC_CRITERIA_STATUS_COMBO_1, m_StatusCombo[0]);
	DDX_Control(pDX, IDC_CRITERIA_LABEL_COMBO_1, m_LabelCombo[0]);
	DDX_Control(pDX, IDC_CRITERIA_PERSONA_COMBO_1, m_PersonaCombo[0]);
	DDX_Control(pDX, IDC_CRITERIA_PRIORITY_COMBO_1, m_PriorityCombo[0]);

	DDX_Control(pDX, IDC_CRITERIA_NUM_UNITS_STATIC_2, m_NumStatic[1]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_EDIT_2, m_NumEdit[1]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_SPIN_2, m_NumSpin[1]);
	DDX_Control(pDX, IDC_CRITERIA_CATEGORY_COMBO_2, m_CategoryCombo[1]);
	DDX_Control(pDX, IDC_CRITERIA_TEXT_COMPARE_COMBO_2, m_TextCompareCombo[1]);
	DDX_Control(pDX, IDC_CRITERIA_EQUAL_COMBO_2, m_EqualCompareCombo[1]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_COMPARE_COMBO_2, m_NumCompareCombo[1]);
	DDX_Control(pDX, IDC_CRITERIA_DATE_COMPARE_COMBO_2, m_DateCompareCombo[1]);
	DDX_Control(pDX, IDC_CRITERIA_TEXT_EDIT_2, m_TextEdit[1]);
	DDX_Control(pDX, IDC_CRITERIA_STATUS_COMBO_2, m_StatusCombo[1]);
	DDX_Control(pDX, IDC_CRITERIA_LABEL_COMBO_2, m_LabelCombo[1]);
	DDX_Control(pDX, IDC_CRITERIA_PERSONA_COMBO_2, m_PersonaCombo[1]);
	DDX_Control(pDX, IDC_CRITERIA_PRIORITY_COMBO_2, m_PriorityCombo[1]);

	DDX_Control(pDX, IDC_CRITERIA_NUM_UNITS_STATIC_3, m_NumStatic[2]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_EDIT_3, m_NumEdit[2]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_SPIN_3, m_NumSpin[2]);
	DDX_Control(pDX, IDC_CRITERIA_CATEGORY_COMBO_3, m_CategoryCombo[2]);
	DDX_Control(pDX, IDC_CRITERIA_TEXT_COMPARE_COMBO_3, m_TextCompareCombo[2]);
	DDX_Control(pDX, IDC_CRITERIA_EQUAL_COMBO_3, m_EqualCompareCombo[2]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_COMPARE_COMBO_3, m_NumCompareCombo[2]);
	DDX_Control(pDX, IDC_CRITERIA_DATE_COMPARE_COMBO_3, m_DateCompareCombo[2]);
	DDX_Control(pDX, IDC_CRITERIA_TEXT_EDIT_3, m_TextEdit[2]);
	DDX_Control(pDX, IDC_CRITERIA_STATUS_COMBO_3, m_StatusCombo[2]);
	DDX_Control(pDX, IDC_CRITERIA_LABEL_COMBO_3, m_LabelCombo[2]);
	DDX_Control(pDX, IDC_CRITERIA_PERSONA_COMBO_3, m_PersonaCombo[2]);
	DDX_Control(pDX, IDC_CRITERIA_PRIORITY_COMBO_3, m_PriorityCombo[2]);

	DDX_Control(pDX, IDC_CRITERIA_NUM_UNITS_STATIC_4, m_NumStatic[3]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_EDIT_4, m_NumEdit[3]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_SPIN_4, m_NumSpin[3]);
	DDX_Control(pDX, IDC_CRITERIA_CATEGORY_COMBO_4, m_CategoryCombo[3]);
	DDX_Control(pDX, IDC_CRITERIA_TEXT_COMPARE_COMBO_4, m_TextCompareCombo[3]);
	DDX_Control(pDX, IDC_CRITERIA_EQUAL_COMBO_4, m_EqualCompareCombo[3]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_COMPARE_COMBO_4, m_NumCompareCombo[3]);
	DDX_Control(pDX, IDC_CRITERIA_DATE_COMPARE_COMBO_4, m_DateCompareCombo[3]);
	DDX_Control(pDX, IDC_CRITERIA_TEXT_EDIT_4, m_TextEdit[3]);
	DDX_Control(pDX, IDC_CRITERIA_STATUS_COMBO_4, m_StatusCombo[3]);
	DDX_Control(pDX, IDC_CRITERIA_LABEL_COMBO_4, m_LabelCombo[3]);
	DDX_Control(pDX, IDC_CRITERIA_PERSONA_COMBO_4, m_PersonaCombo[3]);
	DDX_Control(pDX, IDC_CRITERIA_PRIORITY_COMBO_4, m_PriorityCombo[3]);

	DDX_Control(pDX, IDC_CRITERIA_NUM_UNITS_STATIC_5, m_NumStatic[4]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_EDIT_5, m_NumEdit[4]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_SPIN_5, m_NumSpin[4]);
	DDX_Control(pDX, IDC_CRITERIA_CATEGORY_COMBO_5, m_CategoryCombo[4]);
	DDX_Control(pDX, IDC_CRITERIA_TEXT_COMPARE_COMBO_5, m_TextCompareCombo[4]);
	DDX_Control(pDX, IDC_CRITERIA_EQUAL_COMBO_5, m_EqualCompareCombo[4]);
	DDX_Control(pDX, IDC_CRITERIA_NUM_COMPARE_COMBO_5, m_NumCompareCombo[4]);
	DDX_Control(pDX, IDC_CRITERIA_DATE_COMPARE_COMBO_5, m_DateCompareCombo[4]);
	DDX_Control(pDX, IDC_CRITERIA_TEXT_EDIT_5, m_TextEdit[4]);
	DDX_Control(pDX, IDC_CRITERIA_STATUS_COMBO_5, m_StatusCombo[4]);
	DDX_Control(pDX, IDC_CRITERIA_LABEL_COMBO_5, m_LabelCombo[4]);
	DDX_Control(pDX, IDC_CRITERIA_PERSONA_COMBO_5, m_PersonaCombo[4]);
	DDX_Control(pDX, IDC_CRITERIA_PRIORITY_COMBO_5, m_PriorityCombo[4]);

	//}}AFX_DATA_MAP
}

// --------------------------------------------------------------------------

void CSearchView::OnInitialUpdate()
{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::OnInitialUpdate()\n");
#endif // _MY_TRACE_

	if (m_bInitilized)
	{
#ifdef _MY_TRACE_
::MyTrace("    CSearchView::OnInitialUpdate() [SKIPPED]\n");
#endif // _MY_TRACE_
		return; // guard against bogus double initializations
	}

	C3DFormView::OnInitialUpdate();

	// This is to fix the bug (#4552) where the focus is in the criteria
	// edit field, but no cursor.
	//
	// Here goes my theory. When the dialog is created, whomever has the focus
	// is not enabled. This is a bad state in MFC: something that cannot get
	// focus has focus. So, we need to put the focus somewhere that will
	// not be disabled during dialog init. Let's try the MORE button.
	//
	// Of course near the end of the init we set the focus to the correct
	// edit field.
	//
	m_MoreBtn.SetFocus();

	m_SaveStrSeperator = CRString(IDS_SEARCH_SAVE_CHR_SEPARATOR).GetAt(0);

	// SECDateTimeCtrl::ShortDate
	// SECDateTimeCtrl::LongDate
	m_DateTimeCtrl[0].SetFormat(_T(CRString(IDS_SEARCH_DATE_CTRL_DISPLAY_FORMAT))); // "MMMM d, yyyy"
	VERIFY(m_DateTimeCtrl[0].AttachDateTimeCtrl(IDC_CRITERIA_DATE_CTRL_1, this, SEC_DTS_CALENDAR));
	m_DateTimeCtrl[1].SetFormat(_T(CRString(IDS_SEARCH_DATE_CTRL_DISPLAY_FORMAT))); // "MMMM d, yyyy"
	VERIFY(m_DateTimeCtrl[1].AttachDateTimeCtrl(IDC_CRITERIA_DATE_CTRL_2, this, SEC_DTS_CALENDAR));
	m_DateTimeCtrl[2].SetFormat(_T(CRString(IDS_SEARCH_DATE_CTRL_DISPLAY_FORMAT))); // "MMMM d, yyyy"
	VERIFY(m_DateTimeCtrl[2].AttachDateTimeCtrl(IDC_CRITERIA_DATE_CTRL_3, this, SEC_DTS_CALENDAR));
	m_DateTimeCtrl[3].SetFormat(_T(CRString(IDS_SEARCH_DATE_CTRL_DISPLAY_FORMAT))); // "MMMM d, yyyy"
	VERIFY(m_DateTimeCtrl[3].AttachDateTimeCtrl(IDC_CRITERIA_DATE_CTRL_4, this, SEC_DTS_CALENDAR));
	m_DateTimeCtrl[4].SetFormat(_T(CRString(IDS_SEARCH_DATE_CTRL_DISPLAY_FORMAT))); // "MMMM d, yyyy"
	VERIFY(m_DateTimeCtrl[4].AttachDateTimeCtrl(IDC_CRITERIA_DATE_CTRL_5, this, SEC_DTS_CALENDAR));

	m_ResultsList.SetHighlightType(LVEX_HIGHLIGHT_ROW);

	m_ResultsList.InsertColumn(COLUMN_MAILBOX,   CRString(IDS_SEARCH_COLUMN_HEADER_MAILBOX)); // "Mailbox"
	m_ResultsList.InsertColumn(COLUMN_WHO,       CRString(IDS_SEARCH_COLUMN_HEADER_WHO));     // "Who"
	m_ResultsList.InsertColumn(COLUMN_DATE,      CRString(IDS_SEARCH_COLUMN_HEADER_DATE));    // "Date"
	m_ResultsList.InsertColumn(COLUMN_SUBJECT,   CRString(IDS_SEARCH_COLUMN_HEADER_SUBJECT)); // "Subject"

	m_ResultsList.SetColumnWidth(COLUMN_MAILBOX, GetIniShort(IDS_INI_SEARCH_MBOX_WIDTH));
	m_ResultsList.SetColumnWidth(COLUMN_WHO, GetIniShort(IDS_INI_SEARCH_WHO_WIDTH));
	m_ResultsList.SetColumnWidth(COLUMN_DATE, GetIniShort(IDS_INI_SEARCH_DATE_WIDTH));
	m_ResultsList.SetColumnWidth(COLUMN_SUBJECT, GetIniShort(IDS_INI_SEARCH_SUBJECT_WIDTH));


//	VERIFY(m_ResultsList.SetColumnAlignment(COLUMN_MAILBOX, LVCFMT_RIGHT));

	{
		// This is pretty ugly. The TC_ITEM needs to be filled with the
		// text for the tab titles. Unfortunately MFC insists that the
		// strings be passed as LPSTR via a struct.
		//
		// This means we need to create the strings (on the stack), then
		// cast them to a LPSTR. Yuck.

		CRString sResults(IDS_SEARCH_TAB_TITLE_RESULTS); // "Results"
		CRString sMailboxes(IDS_SEARCH_TAB_TITLE_MAILBOXES); // "Mailboxes"

		TC_ITEM TabCtrlItem;
		TabCtrlItem.mask = TCIF_TEXT;
		TabCtrlItem.pszText = (LPSTR) LPCSTR(sResults);
		m_ResultsMbxTabCtrl.InsertItem( TAB_RESULTS_IDX, &TabCtrlItem );

		TabCtrlItem.pszText = (LPSTR) LPCSTR(sMailboxes);
		m_ResultsMbxTabCtrl.InsertItem( TAB_MAILBOXES_IDX, &TabCtrlItem );
	}

	m_AndRadioBtn.SetCheck(1);
	m_OrRadioBtn.SetCheck(0);

//	m_ResultsMbxTabCtrl.ModifyStyle( 0, WS_CLIPCHILDREN);

	VERIFY(m_MBoxTree.Init()); // Initialize the tree control

	// Setup the tree check boxes
	VERIFY(m_MBoxTree.InitStateImageList(IDB_TREECTRL_CHECKMARKS, 16, RGB(255,255,255)));
	m_MBoxTree.SetContainerFlag();

	// Fill the tree control from the mailbox director
	g_theMailboxDirector.InitializeMailboxTreeControl( &m_MBoxTree, 0, NULL );

	// Select the first root
	m_MBoxTree.CheckAll();
//	m_MBoxTree.ExpandAll();

	// We want the list/tree ctrls to be children of the tab
	m_MBoxTree.SetParent(&m_ResultsMbxTabCtrl);
	m_ResultsList.SetParent(&m_ResultsMbxTabCtrl);
//	m_ResultsStatic.SetParent(&m_ResultsMbxTabCtrl); // This doesn't seem to work

	// But we want the dialog to get all the notification msgs
	m_MBoxTree.SetOwner(this);
	m_ResultsList.SetOwner(this);
	m_ResultsList.SetEatReturnKey(true); // Ignore old val

	m_ResultsMbxTabCtrl.SetCurSel(TAB_MAILBOXES_IDX); // Bring the mailboxes tab to the front
	UpdateTabContents();
	UpdateResultsText();
	UpdateMbxText();
	UpdateSearchBtn();

	m_bInitilized = TRUE;

	CMDIChild *pParentFrame = (CMDIChild *) GetParentFrame();
	ASSERT(pParentFrame);
	ASSERT_KINDOF(CMDIChild, pParentFrame);

	pParentFrame->RecalcLayout();

	// From CScrollView::ResizeParentToFit() in viewscrl.cpp
	{
		// determine current size of the client area as if no scrollbars present
		CRect rectClient;
		GetWindowRect(rectClient);
		CRect rect = rectClient;
		CalcWindowRect(rect);
		rectClient.left += rectClient.left - rect.left;
		rectClient.top += rectClient.top - rect.top;
		rectClient.right -= rect.right - rectClient.right;
		rectClient.bottom -= rect.bottom - rectClient.bottom;
		rectClient.OffsetRect(-rectClient.left, -rectClient.top);
		ASSERT(rectClient.left == 0 && rectClient.top == 0);

		// determine desired size of the view
		CRect rectView(0, 0, m_totalDev.cx, m_totalDev.cy);
//		if (bShrinkOnly)
//		{
//			if (rectClient.right <= m_totalDev.cx)
//				rectView.right = rectClient.right;
//			if (rectClient.bottom <= m_totalDev.cy)
//				rectView.bottom = rectClient.bottom;
//		}
		CalcWindowRect(rectView, CWnd::adjustOutside);
		rectView.OffsetRect(-rectView.left, -rectView.top);
		ASSERT(rectView.left == 0 && rectView.top == 0);
//		if (bShrinkOnly)
//		{
//			if (rectClient.right <= m_totalDev.cx)
//				rectView.right = rectClient.right;
//			if (rectClient.bottom <= m_totalDev.cy)
//				rectView.bottom = rectClient.bottom;
//		}

		// dermine and set size of frame based on desired size of view
		CRect rectFrame;
		CFrameWnd* pFrame = GetParentFrame();
		ASSERT_VALID(pFrame);
		pFrame->GetWindowRect(rectFrame);
		CSize size = rectFrame.Size();
		size.cx += rectView.right - rectClient.right;
		size.cy += rectView.bottom - rectClient.bottom;

		// CHANGE: Instead of moving the window, we want to set the min size
		
//		pFrame->SetWindowPos(NULL, 0, 0, size.cx, size.cy,
//			SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		m_MinSize = size;
	}

//	CRect rct;
//	pParentFrame->GetWindowRect(rct);
//	pParentFrame->SetMinTrackSize(CPoint(rct.Width(), rct.Height()));

	pParentFrame->SetMinTrackSize(CPoint(m_MinSize.cx, m_MinSize.cy));

	CRect ClientRct;
	GetClientRect(ClientRct);
	ResizeControls(ClientRct.Width(), ClientRct.Height());
	
	if (!m_bInitFinal)
	{
		PostMessage(msgInitFinal, 0, 0);
		return;
	}
}

// --------------------------------------------------------------------------

LONG CSearchView::OnInitFinal(WPARAM, LPARAM) // Called AFTER controls are init'd
{
#ifdef _MY_TRACE_
	::MyTrace("CSearchView::OnInitFinal(WPARAM, LPARAM)\n");
#endif // _MY_TRACE_

	if (m_bInitFinal)
	{
#ifdef _MY_TRACE_
		::MyTrace("    CSearchView::OnInitFinal(WPARAM, LPARAM) [SKIPPED]\n");
#endif // _MY_TRACE_
		return (0);
	}

	m_bInitFinal = true;

	if (!m_bInitdOffsets)
	{
		int nMinTop = INT_MAX;
		CRect rct;

		// First get the absolute pos of the top of the ctrls in question
		// At the same time find the min of all the top pos's
		m_MoreBtn.GetWindowRect(rct);
		nMinTop = __min((m_OffsetMoreBtn = rct.top), nMinTop);

		m_LessBtn.GetWindowRect(rct);
		nMinTop = __min((m_OffsetLessBtn = rct.top), nMinTop);

		m_AndRadioBtn.GetWindowRect(rct);
		nMinTop = __min((m_OffsetAndRadio = rct.top), nMinTop);

		m_OrRadioBtn.GetWindowRect(rct);
		nMinTop = __min((m_OffsetOrRadio = rct.top), nMinTop);

		m_BeginBtn.GetWindowRect(rct);
		nMinTop = __min((m_OffsetBeginBtn = rct.top), nMinTop);

		m_ResultsMbxTabCtrl.GetWindowRect(rct);
		nMinTop = __min((m_OffsetTabCtrl = rct.top), nMinTop);

		m_ResultsStatic.GetWindowRect(rct);
		nMinTop = __min((m_OffsetResultsStatic = rct.top), nMinTop);

		// Get the bottom of the window for min window size
//		GetWindowRect(rct);
//		m_OffsetMinWinBottom = rct.bottom - nMinTop;

		// Now make everything relative to the minimum
		// One of these will become zero (the min)
		m_OffsetMoreBtn -= nMinTop;
		m_OffsetLessBtn -= nMinTop;
		m_OffsetAndRadio -= nMinTop;
		m_OffsetOrRadio -= nMinTop;
		m_OffsetBeginBtn -= nMinTop;
		m_OffsetTabCtrl -= nMinTop;
		m_OffsetResultsStatic -= nMinTop;

		m_bInitdOffsets = true;

		AdjustControlTopPos();
	}

	m_MBoxTree.SelectSetFirstVisible(m_MBoxTree.GetRootItem());

	HideAllControls();
	LoadCriteria();
	UpdateSearchBtn();

	return (0);
}

// --------------------------------------------------------------------------

void CSearchView::HideAllControls()
{
	for (int idx = 0; idx < MAX_CRITERIA; idx++)
	{
		InitializeCriteriaCtrls(idx);
		HideControls(idx);
	}

	m_CurCritCount = 0;

	UpdateCritBotPos();
	AdjustControlTopPos();
}

// --------------------------------------------------------------------------

void CSearchView::HideControls(int nIdx)
{
	m_CategoryCombo[nIdx].ShowWindow(SW_HIDE);

	m_TextCompareCombo[nIdx].ShowWindow(SW_HIDE);
	m_EqualCompareCombo[nIdx].ShowWindow(SW_HIDE);
	m_NumCompareCombo[nIdx].ShowWindow(SW_HIDE);
	m_DateCompareCombo[nIdx].ShowWindow(SW_HIDE);

	m_NumStatic[nIdx].ShowWindow(SW_HIDE);
	m_NumEdit[nIdx].ShowWindow(SW_HIDE);
	m_NumSpin[nIdx].ShowWindow(SW_HIDE);
	m_TextEdit[nIdx].ShowWindow(SW_HIDE);
	m_StatusCombo[nIdx].ShowWindow(SW_HIDE);
	m_LabelCombo[nIdx].ShowWindow(SW_HIDE);
	m_PersonaCombo[nIdx].ShowWindow(SW_HIDE);
	m_PriorityCombo[nIdx].ShowWindow(SW_HIDE);
	m_DateTimeCtrl[nIdx].ShowWindow(SW_HIDE);
}

// --------------------------------------------------------------------------

void CSearchView::UpdateCritBotPos()
{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::UpdateCritBotPos()\n");
#endif // _MY_TRACE_

	if (m_CurCritCount < 1)
		m_CriteriaBottomPos = 0;
	else
	{
		CRect rct;
		m_CategoryCombo[m_CurCritCount - 1].GetWindowRect(rct);
		ScreenToClient(rct);
		m_CriteriaBottomPos = rct.bottom;
	}
}

// --------------------------------------------------------------------------

bool CSearchView::RemoveCriteria()
{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::RemoveCriteria()\n");
#endif // _MY_TRACE_

	ASSERT(m_CurCritCount > 0);
	ASSERT(m_CurCritCount <= MAX_CRITERIA);

	if (m_CurCritCount <= 0)
		return (false);

	HideControls(--m_CurCritCount);

	UpdateMoreLessBtn();
	UpdateAndOrBtns();
	UpdateCritBotPos();
	AdjustControlTopPos();

	return (true);
}

// --------------------------------------------------------------------------

bool CSearchView::AddNewCriteria()
{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::AddNewCriteria()\n");
#endif // _MY_TRACE_

	ASSERT(m_CurCritCount >= 0);
	ASSERT(m_CurCritCount < MAX_CRITERIA);

	if (m_CurCritCount >= MAX_CRITERIA)
		return (false);

	int nNewIdx = (m_CurCritCount++);

	InitializeCriteriaCtrls(nNewIdx);
	m_CategoryCombo[nNewIdx].ShowWindow(SW_SHOW);
	OnCategoryChange(nNewIdx);
	SetCriteriaFocus(nNewIdx);

	UpdateMoreLessBtn();
	UpdateAndOrBtns();
	UpdateCritBotPos();
	AdjustControlTopPos();

	return (true);
}

// --------------------------------------------------------------------------

void CSearchView::OnCategoryChange(int nIdx)
{
	UpdateState(nIdx);

	if (nIdx < m_CurCritCount)
	{
		ResizeCriteriaCtrls(nIdx);
		SetupDynamicCriteriaCtrls(nIdx);
		UpdateSearchBtn();
	}
}

// --------------------------------------------------------------------------

void CSearchView::ResizeAllCriteriaCtrls()
{
	for (int idx = 0; idx < m_CurCritCount; idx++)
		ResizeCriteriaCtrls(idx);
}

// --------------------------------------------------------------------------

void CSearchView::ResizeCriteriaCtrls(int nIdx)
{
	ASSERT(nIdx < m_CurCritCount);

	CRect rct;
	GetClientRect(rct);

	ResizeCriteriaCtrls(nIdx, rct.Width(), nIdx * 30);
}

// --------------------------------------------------------------------------

void CSearchView::AdjustControlTopPos()
{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::AdjustControlTopPos()\n");
#endif // _MY_TRACE_

	ASSERT(m_bInitdOffsets);
	if (!m_bInitdOffsets)
		return;

	const int nTop = (m_CriteriaBottomPos + 10);

	MoveControl(&m_MoreBtn, -1, -1, nTop + m_OffsetMoreBtn);
	MoveControl(&m_LessBtn, -1, -1, nTop + m_OffsetLessBtn);
	MoveControl(&m_AndRadioBtn, -1, -1, nTop + m_OffsetAndRadio);
	MoveControl(&m_OrRadioBtn, -1, -1, nTop + m_OffsetOrRadio);
	MoveControl(&m_BeginBtn, -1, -1, nTop + m_OffsetBeginBtn);
	MoveControl(&m_ResultsStatic, -1, -1, nTop + m_OffsetResultsStatic);
	StretchTopControl(&m_ResultsMbxTabCtrl, nTop + m_OffsetTabCtrl);

	AdjustWinMinHeight(nTop + m_MinSize.cy);
	ResizeTabContents();
}

// --------------------------------------------------------------------------

void CSearchView::AdjustWinMinHeight(int nMinWinHeight)
{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::AdjustWinMinHeight(%d)\n", nMinWinHeight);
#endif // _MY_TRACE_

	ASSERT(m_bInitdOffsets);
	if (!m_bInitdOffsets)
		return;

	CMDIChild *pParentFrame = (CMDIChild *) GetParentFrame();
	
	ASSERT(pParentFrame);
	ASSERT_KINDOF(CMDIChild, pParentFrame);
	
	CPoint pt(pParentFrame->GetMinTrackSize());

	if (pt.y != nMinWinHeight)
	{
		pt.y = nMinWinHeight;
		pParentFrame->SetMinTrackSize(pt);

		CRect rct;
		pParentFrame->GetWindowRect(rct);

		if (nMinWinHeight > rct.Height())
		{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::AdjustWinMinHeight: Expanding window to new min height (%d)\n", nMinWinHeight);
#endif // _MY_TRACE_
			pParentFrame->SetWindowPos(NULL, 0, 0, rct.Width(), nMinWinHeight, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		}
	}
}

// --------------------------------------------------------------------------

void CSearchView::StretchTopControl(CWnd *pWnd, int nTop)
{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::StretchTopControl(%d)\n", nTop);
#endif // _MY_TRACE_

	CRect CtrlRct;

	pWnd->GetWindowRect(CtrlRct);
	ScreenToClient(CtrlRct);

	CtrlRct.top = nTop;

	pWnd->MoveWindow(CtrlRct);
}

// --------------------------------------------------------------------------

void CSearchView::InitializeCriteriaCtrls(int nIdx)
{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::InitializeCriteriaCtrls(%d)\n", nIdx);
#endif // _MY_TRACE_

	if (m_bCritInitd[nIdx])
		return; // Only need to init each criteria once

	m_CategoryCombo[nIdx].ResetContent();

	int ret;

	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_ANYWHERE))) >= 0); // "Anywhere"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_ANYWHERE) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_HEADERS))) >= 0); // "Headers"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_HEADERS) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_BODY))) >= 0); // "Body"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_BODY) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_ATTACHNAMES))) >= 0); // "Attachment Name(s)"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_ATTACHNAMES) != CB_ERR);

	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_SUMMARY))) >= 0); // "Summary"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_SUMMARY) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_STATUS))) >= 0); // "Status"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_STATUS) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_PRIORITY))) >= 0); // "Priority"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_PRIORITY) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_ATTACHCOUNT))) >= 0); // "Attachment Count"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_ATTACHCOUNT) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_LABEL))) >= 0); // "Label"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_LABEL) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_DATE))) >= 0); // "Date"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_DATE) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_SIZE))) >= 0); // "Size"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_SIZE) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_AGE))) >= 0); // "Age"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_AGE) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_PERSONA))) >= 0); // "Personality"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_PERSONA) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_MBXNAME))) >= 0); // "Mailbox Name"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_MAILBOXNAME) != CB_ERR);

	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_TO))) >= 0); // "To"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_TO) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_FROM))) >= 0); // "From"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_FROM) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_SUBJECT))) >= 0); // "Subject"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_SUBJECT) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_CC))) >= 0); // "CC"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_CC) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_BCC))) >= 0); // "BCC"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_BCC) != CB_ERR);
	VERIFY((ret = m_CategoryCombo[nIdx].AddString(CRString(IDS_SEARCH_CATEGORYSTR_ANYRECIPIENT))) >= 0); // "Any recipient"
	VERIFY(m_CategoryCombo[nIdx].SetItemData(ret, CRITERIA_OBJECT_ANYRECIP) != CB_ERR);

	VERIFY(m_CategoryCombo[nIdx].SetCurSel(0) != CB_ERR);

	// --------------------------------------------------

	m_TextCompareCombo[nIdx].ResetContent();

	VERIFY((ret = m_TextCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_CONTAINS))) >= 0); // "contains"
	VERIFY(m_TextCompareCombo[nIdx].SetItemData(ret, CRITERIA_TEXT_COMPARE_CONTAINS) != CB_ERR);
	VERIFY((ret = m_TextCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_CONTAINSWORD))) >= 0); // "contains word"
	VERIFY(m_TextCompareCombo[nIdx].SetItemData(ret, CRITERIA_TEXT_COMPARE_CONTAINSWORD) != CB_ERR);
	VERIFY((ret = m_TextCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_DOESNOTCONTAIN))) >= 0); // "does not contain"
	VERIFY(m_TextCompareCombo[nIdx].SetItemData(ret, CRITERIA_TEXT_COMPARE_DOESNOTCONTAIN) != CB_ERR);
	VERIFY((ret = m_TextCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_IS))) >= 0); // "is"
	VERIFY(m_TextCompareCombo[nIdx].SetItemData(ret, CRITERIA_TEXT_COMPARE_IS) != CB_ERR);
	VERIFY((ret = m_TextCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_ISNOT))) >= 0); // "is not"
	VERIFY(m_TextCompareCombo[nIdx].SetItemData(ret, CRITERIA_TEXT_COMPARE_ISNOT) != CB_ERR);
	VERIFY((ret = m_TextCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_STARTSWITH))) >= 0); // "starts with"
	VERIFY(m_TextCompareCombo[nIdx].SetItemData(ret, CRITERIA_TEXT_COMPARE_STARTSWITH) != CB_ERR);
	VERIFY((ret = m_TextCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_ENDSWITH))) >= 0); // "ends with"
	VERIFY(m_TextCompareCombo[nIdx].SetItemData(ret, CRITERIA_TEXT_COMPARE_ENDSWITH) != CB_ERR);
	VERIFY((ret = m_TextCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_MATCHESREGEXP))) >= 0); // "matches regexp"
	VERIFY(m_TextCompareCombo[nIdx].SetItemData(ret, CRITERIA_TEXT_COMPARE_MATCHESREGEXP) != CB_ERR);

	VERIFY(m_TextCompareCombo[nIdx].SetCurSel(0) != CB_ERR);

	// --------------------------------------------------

	m_EqualCompareCombo[nIdx].ResetContent();

	VERIFY((ret = m_EqualCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_IS))) >= 0); // "is"
	VERIFY(m_EqualCompareCombo[nIdx].SetItemData(ret, CRITERIA_EQUAL_COMPARE_IS) != CB_ERR);
	VERIFY((ret = m_EqualCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_ISNOT))) >= 0); // "is not"
	VERIFY(m_EqualCompareCombo[nIdx].SetItemData(ret, CRITERIA_EQUAL_COMPARE_ISNOT) != CB_ERR);

	VERIFY(m_EqualCompareCombo[nIdx].SetCurSel(0) != CB_ERR);

	// --------------------------------------------------

	m_NumCompareCombo[nIdx].ResetContent();

	VERIFY((ret = m_NumCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_IS))) >= 0); // "is"
	VERIFY(m_NumCompareCombo[nIdx].SetItemData(ret, CRITERIA_NUM_COMPARE_IS) != CB_ERR);
	VERIFY((ret = m_NumCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_ISNOT))) >= 0); // "is not"
	VERIFY(m_NumCompareCombo[nIdx].SetItemData(ret, CRITERIA_NUM_COMPARE_ISNOT) != CB_ERR);
	VERIFY((ret = m_NumCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_ISGREATERTHAN))) >= 0); // "is greater than"
	VERIFY(m_NumCompareCombo[nIdx].SetItemData(ret, CRITERIA_NUM_COMPARE_ISGREATERTHAN) != CB_ERR);
	VERIFY((ret = m_NumCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_ISLESSTHAN))) >= 0); // "is less than"
	VERIFY(m_NumCompareCombo[nIdx].SetItemData(ret, CRITERIA_NUM_COMPARE_ISLESSTHAN) != CB_ERR);

	VERIFY(m_NumCompareCombo[nIdx].SetCurSel(0) != CB_ERR);

	// --------------------------------------------------

	m_DateCompareCombo[nIdx].ResetContent();

	VERIFY((ret = m_DateCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_IS))) >= 0); // "is"
	VERIFY(m_DateCompareCombo[nIdx].SetItemData(ret, CRITERIA_DATE_COMPARE_IS) != CB_ERR);
	VERIFY((ret = m_DateCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_ISNOT))) >= 0); // "is not"
	VERIFY(m_DateCompareCombo[nIdx].SetItemData(ret, CRITERIA_DATE_COMPARE_ISNOT) != CB_ERR);
	VERIFY((ret = m_DateCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_ISAFTER))) >= 0); //"is after" 
	VERIFY(m_DateCompareCombo[nIdx].SetItemData(ret, CRITERIA_DATE_COMPARE_ISAFTER) != CB_ERR);
	VERIFY((ret = m_DateCompareCombo[nIdx].AddString(CRString(IDS_SEARCH_COMPARESTR_ISBEFORE))) >= 0); // "is before"
	VERIFY(m_DateCompareCombo[nIdx].SetItemData(ret, CRITERIA_DATE_COMPARE_ISBEFORE) != CB_ERR);

	VERIFY(m_DateCompareCombo[nIdx].SetCurSel(0) != CB_ERR);

	// --------------------------------------------------

	m_LabelCombo[nIdx].InitLabels();

	// --------------------------------------------------

	m_NumEdit[nIdx].SetWindowText(CRString(IDS_SEARCH_NUMEDIT_INIT_VALUE)); // "0"
	m_NumSpin[nIdx].SetBuddy(&m_NumEdit[nIdx]);
	m_NumSpin[nIdx].SetRange(0, UD_MAXVAL);

	// --------------------------------------------------

	m_PersonaCombo[nIdx].ResetContent();

	LPSTR pszList = g_Personalities.List();
	while (pszList && *pszList)
	{
		m_PersonaCombo[nIdx].AddString(pszList);
		pszList += strlen(pszList) + 1;
	}

	VERIFY(m_PersonaCombo[nIdx].SetCurSel(0) != CB_ERR);

	// --------------------------------------------------

	m_PriorityCombo[nIdx].ResetContent();
	VERIFY((ret = m_PriorityCombo[nIdx].AddString(CRString(IDS_PRIORITY_HIGHEST))) >= 0); // "Highest"
	VERIFY(m_PriorityCombo[nIdx].SetItemData(ret, CRITERIA_VALUE_PRIORITY_HIGHEST) != CB_ERR);
	VERIFY((ret = m_PriorityCombo[nIdx].AddString(CRString(IDS_PRIORITY_HIGH))) >= 0); // "High"
	VERIFY(m_PriorityCombo[nIdx].SetItemData(ret, CRITERIA_VALUE_PRIORITY_HIGH) != CB_ERR);
	VERIFY((ret = m_PriorityCombo[nIdx].AddString(CRString(IDS_PRIORITY_NORMAL))) >= 0); // "Normal"
	VERIFY(m_PriorityCombo[nIdx].SetItemData(ret, CRITERIA_VALUE_PRIORITY_NORMAL) != CB_ERR);
	VERIFY((ret = m_PriorityCombo[nIdx].AddString(CRString(IDS_PRIORITY_LOW))) >= 0); // "Low"
	VERIFY(m_PriorityCombo[nIdx].SetItemData(ret, CRITERIA_VALUE_PRIORITY_LOW) != CB_ERR);
	VERIFY((ret = m_PriorityCombo[nIdx].AddString(CRString(IDS_PRIORITY_LOWEST))) >= 0); // "Lowest"
	VERIFY(m_PriorityCombo[nIdx].SetItemData(ret, CRITERIA_VALUE_PRIORITY_LOWEST) != CB_ERR);

	VERIFY(m_PriorityCombo[nIdx].SetCurSel(0) != CB_ERR);

	// --------------------------------------------------

	m_StatusCombo[nIdx].ResetContent();

	static int statusTable[] = {
		IDS_STATUS_UNREAD, CRITERIA_VALUE_STATUS_UNREAD,
		IDS_STATUS_READ, CRITERIA_VALUE_STATUS_READ,
		IDS_STATUS_REPLIED, CRITERIA_VALUE_STATUS_REPLIED,
		IDS_STATUS_FORWARDED, CRITERIA_VALUE_STATUS_FORWARDED,
		IDS_STATUS_REDIRECTED, CRITERIA_VALUE_STATUS_REDIRECTED,
		IDS_STATUS_SENT, CRITERIA_VALUE_STATUS_SENT,
		IDS_STATUS_SENDABLE, CRITERIA_VALUE_STATUS_SENDABLE,
		IDS_STATUS_UNSENT, CRITERIA_VALUE_STATUS_UNSENT,
		IDS_STATUS_QUEUED, CRITERIA_VALUE_STATUS_QUEUED,
		IDS_STATUS_TIME_QUEUED, CRITERIA_VALUE_STATUS_TIME_QUEUED,
		IDS_STATUS_UNSENDABLE, CRITERIA_VALUE_STATUS_UNSENDABLE,
		IDS_STATUS_RECOVERED, CRITERIA_VALUE_STATUS_RECOVERED,
		0,0};
	int i;

	for (i=0;statusTable[i];i+=2)
	{
		VERIFY((ret = m_StatusCombo[nIdx].AddString(CRString(statusTable[i]))) >= 0);
		VERIFY(m_StatusCombo[nIdx].SetItemData(ret, statusTable[i+1]) != CB_ERR);
	}

	VERIFY(m_StatusCombo[nIdx].SetCurSel(0) != CB_ERR);

	// --------------------------------------------------

	m_bCritInitd[nIdx] = true;
	UpdateState(nIdx);
}

// --------------------------------------------------------------------------

int CSearchView::MoveControl(CWnd *pWnd, int nLeft, int nRight, int nTop)
{
	CRect CtrlRct;

	pWnd->GetWindowRect(CtrlRct);
	ScreenToClient(CtrlRct);
	int nWidth = 0;

	if ((nLeft < 0) || (nRight < 0)) // One or both < 0
	{
		nWidth = CtrlRct.Width();

		if ((nLeft >= 0) || (nRight >= 0)) // One or both >= 0
		{
			if (nLeft < 0)
			{
				CtrlRct.left = nRight - nWidth; // Left < 0 && right >= 0
				CtrlRct.right = nRight;
			}
			else
			{
				CtrlRct.right = nLeft + nWidth; // Left >= 0 && right < 0
				CtrlRct.left = nLeft;
			}
		}
	}
	else
	{
		// Left >= 0 && right >= 0
		CtrlRct.right = nRight;
		CtrlRct.left = nLeft;
	}

	ASSERT(CtrlRct.Width() > 0);

	const int nOffsetY = nTop - CtrlRct.top;
	CtrlRct.OffsetRect(0, nOffsetY);

	pWnd->MoveWindow(CtrlRct);

	if (nRight < 0)
		return (CtrlRct.right);

	return (CtrlRct.left);
}

// --------------------------------------------------------------------------

void CSearchView::SetCriteriaFocus(int nIdx)
{
	ASSERT(nIdx < m_CurCritCount);

	if (nIdx >= m_CurCritCount)
		return;

	CWnd *pWnd = NULL;

	switch (m_CritState[nIdx].m_CurValueType)
	{
		case CRITERIA_VALUE_TEXT_TYPE:
		{
			pWnd = &m_TextEdit[nIdx];
		}
		break;

		case CRITERIA_VALUE_AGE_TYPE:
		case CRITERIA_VALUE_ATTACHCOUNT_TYPE:
		case CRITERIA_VALUE_SIZE_TYPE:
		{
			pWnd = &m_NumEdit[nIdx];
		}
		break;

		case CRITERIA_VALUE_STATUS_TYPE:
		{
			pWnd = &m_StatusCombo[nIdx];
		}
		break;

		case CRITERIA_VALUE_LABEL_TYPE:
		{
			pWnd = &m_LabelCombo[nIdx];
		}
		break;

		case CRITERIA_VALUE_PERSONA_TYPE:
		{
			pWnd = &m_PersonaCombo[nIdx];
		}
		break;

		case CRITERIA_VALUE_PRIORITY_TYPE:
		{
			pWnd = &m_PriorityCombo[nIdx];
		}
		break;

		case CRITERIA_VALUE_DATE_TYPE:
		{
			pWnd = &m_DateTimeCtrl[nIdx];
		}
		break;

		default: ASSERT(0); break;
	}

	pWnd->SetFocus();
}

// --------------------------------------------------------------------------

void CSearchView::ResizeCriteriaCtrls(int nIdx, int cx, int top)
{
	ASSERT(nIdx < m_CurCritCount);

	const int nFirstThird = cx / 3;
	const int nSecondThird = (cx * 2) / 3;

	MoveControl(&m_CategoryCombo[nIdx],     CONTROL_EDGE_SPACING, nFirstThird - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);

	switch (m_CritState[nIdx].m_CurVerbType)
	{
		case CRITERIA_VERB_TEXT_COMPARE_TYPE:
		{
			MoveControl(&m_TextCompareCombo[nIdx],  nFirstThird + CONTROL_EDGE_SPACING, nSecondThird - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);
		}
		break;

		case CRITERIA_VERB_EQUAL_COMPARE_TYPE:
		{
			MoveControl(&m_EqualCompareCombo[nIdx], nFirstThird + CONTROL_EDGE_SPACING, nSecondThird - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);
		}
		break;

		case CRITERIA_VERB_NUM_COMPARE_TYPE:
		{
			MoveControl(&m_NumCompareCombo[nIdx],   nFirstThird + CONTROL_EDGE_SPACING, nSecondThird - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);
		}
		break;

		case CRITERIA_VERB_DATE_COMPARE_TYPE:
		{
			MoveControl(&m_DateCompareCombo[nIdx],  nFirstThird + CONTROL_EDGE_SPACING, nSecondThird - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);
		}
		break;

		default: ASSERT(0); break;
	}

	switch (m_CritState[nIdx].m_CurValueType)
	{
		case CRITERIA_VALUE_TEXT_TYPE:
		{
			MoveControl(&m_TextEdit[nIdx],          nSecondThird + CONTROL_EDGE_SPACING, cx - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);
		}
		break;

		case CRITERIA_VALUE_AGE_TYPE:
		case CRITERIA_VALUE_ATTACHCOUNT_TYPE:
		case CRITERIA_VALUE_SIZE_TYPE:
		{
			if (m_CritState[nIdx].m_bCurShowUnits)
			{
				// The static text should be right-edge flush and 4 pixels down from top of other controls
				int nLeft = MoveControl(&m_NumStatic[nIdx], -1, cx - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top + 4); // Unknown LEFT input

				nLeft -= CONTROL_EDGE_SPACING;

				nLeft = MoveControl(&m_NumSpin[nIdx], -1, nLeft, CONTROL_EDGE_SPACING + top); // Unknown LEFT input
				MoveControl(&m_NumEdit[nIdx], nSecondThird + CONTROL_EDGE_SPACING, nLeft, CONTROL_EDGE_SPACING + top);
			}
			else
			{
				const int nLeft = MoveControl(&m_NumSpin[nIdx], -1, cx - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top); // Unknown LEFT input
				MoveControl(&m_NumEdit[nIdx], nSecondThird + CONTROL_EDGE_SPACING, nLeft, CONTROL_EDGE_SPACING + top);
			}
		}
		break;

		case CRITERIA_VALUE_STATUS_TYPE:
		{
			MoveControl(&m_StatusCombo[nIdx],       nSecondThird + CONTROL_EDGE_SPACING, cx - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);
		}
		break;

		case CRITERIA_VALUE_LABEL_TYPE:
		{
			MoveControl(&m_LabelCombo[nIdx],        nSecondThird + CONTROL_EDGE_SPACING, cx - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);
		}
		break;

		case CRITERIA_VALUE_PERSONA_TYPE:
		{
			MoveControl(&m_PersonaCombo[nIdx],      nSecondThird + CONTROL_EDGE_SPACING, cx - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);
		}
		break;

		case CRITERIA_VALUE_PRIORITY_TYPE:
		{
			MoveControl(&m_PriorityCombo[nIdx],     nSecondThird + CONTROL_EDGE_SPACING, cx - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);
		}
		break;

		case CRITERIA_VALUE_DATE_TYPE:
		{
			MoveControl(&m_DateTimeCtrl[nIdx],     nSecondThird + CONTROL_EDGE_SPACING, cx - CONTROL_EDGE_SPACING, CONTROL_EDGE_SPACING + top);
		}
		break;

		default: ASSERT(0); break;
	}
}

// --------------------------------------------------------------------------

void CSearchView::UpdateState(int nIdx) // Called when category changes
{
	ASSERT(m_bCritInitd[nIdx]);

	// Take current selection from category and setup state for other controls

	const int nSel = m_CategoryCombo[nIdx].GetCurSel();

	ASSERT(CB_ERR != nSel); // no selection

	if (nSel >= 0)
	{
		DWORD dwData = m_CategoryCombo[nIdx].GetItemData(nSel);
		CriteriaObject obj(static_cast<CriteriaObject>(dwData));
		m_CritState[nIdx].UpdateObj(obj);
	}
}

// --------------------------------------------------------------------------

void CSearchView::SetupDynamicCriteriaCtrls(int nIdx) // Called when category changes
{
	ASSERT(nIdx < m_CurCritCount);

	// Verbs
	CriteriaVerbType verb = m_CritState[nIdx].m_CurVerbType;

	m_TextCompareCombo[nIdx].ShowWindow((CRITERIA_VERB_TEXT_COMPARE_TYPE == verb) ? SW_SHOW : SW_HIDE);
	m_EqualCompareCombo[nIdx].ShowWindow((CRITERIA_VERB_EQUAL_COMPARE_TYPE == verb) ? SW_SHOW : SW_HIDE);
	m_NumCompareCombo[nIdx].ShowWindow((CRITERIA_VERB_NUM_COMPARE_TYPE == verb) ? SW_SHOW : SW_HIDE);
	m_DateCompareCombo[nIdx].ShowWindow((CRITERIA_VERB_DATE_COMPARE_TYPE == verb) ? SW_SHOW : SW_HIDE);

	// Values
	CriteriaValueType value = m_CritState[nIdx].m_CurValueType;

	m_TextEdit[nIdx].ShowWindow((CRITERIA_VALUE_TEXT_TYPE == value) ? SW_SHOW : SW_HIDE);

	{
		const bool bShowInt = (CRITERIA_VALUE_ATTACHCOUNT_TYPE == value) || (CRITERIA_VALUE_SIZE_TYPE == value) || (CRITERIA_VALUE_AGE_TYPE == value);

		m_NumEdit[nIdx].ShowWindow((bShowInt) ? SW_SHOW : SW_HIDE);
		m_NumSpin[nIdx].ShowWindow((bShowInt) ? SW_SHOW : SW_HIDE);
	}

	if (m_CritState[nIdx].m_bCurShowUnits)
	{
		ASSERT(!m_CritState[nIdx].m_UnitsStr.IsEmpty());
		m_NumStatic[nIdx].SetWindowText(m_CritState[nIdx].m_UnitsStr);
	}
	m_NumStatic[nIdx].ShowWindow(m_CritState[nIdx].m_bCurShowUnits ? SW_SHOW : SW_HIDE);

	m_StatusCombo[nIdx].ShowWindow((CRITERIA_VALUE_STATUS_TYPE == value) ? SW_SHOW : SW_HIDE);
	m_LabelCombo[nIdx].ShowWindow((CRITERIA_VALUE_LABEL_TYPE == value) ? SW_SHOW : SW_HIDE);
	m_PersonaCombo[nIdx].ShowWindow((CRITERIA_VALUE_PERSONA_TYPE == value) ? SW_SHOW : SW_HIDE);
	m_PriorityCombo[nIdx].ShowWindow((CRITERIA_VALUE_PRIORITY_TYPE == value) ? SW_SHOW : SW_HIDE);
	m_DateTimeCtrl[nIdx].ShowWindow((CRITERIA_VALUE_DATE_TYPE == value) ? SW_SHOW : SW_HIDE);
}

// --------------------------------------------------------------------------

bool CSearchView::IsCriteriaValid(int nIdx)
{
	ASSERT(nIdx < m_CurCritCount);

	return (true);
}

// --------------------------------------------------------------------------

bool CSearchView::GetCtrlCurItemData(CComboBox &cb, DWORD &dw)
{
	int nSel = cb.GetCurSel();

	ASSERT(CB_ERR != nSel);
	if (CB_ERR == nSel)
		return (false);

	DWORD dwVal = cb.GetItemData(nSel);
	ASSERT(CB_ERR != dwVal);

	if (CB_ERR == dwVal)
		return (false);

	dw = dwVal;
	return (true);
}

bool CSearchView::GetCtrlCurItemText(CComboBox &cb, CString &str)
{
	int nSel = cb.GetCurSel();

	ASSERT(CB_ERR != nSel);
	if (CB_ERR == nSel)
		return (false);

	cb.GetLBText(nSel, str);

	return (true);
}

bool CSearchView::GetCriteria(int nIdx, SearchCriteria &criteria)
{
	ASSERT(nIdx < m_CurCritCount);

	SearchCriteria TmpCriteria(criteria);

	VERIFY(TmpCriteria.SetCurObj(m_CritState[nIdx].m_CurObj));

	switch (m_CritState[nIdx].m_CurVerbType)
	{
		case CRITERIA_VERB_TEXT_COMPARE_TYPE:
		{
			DWORD dwVal = 0;
			if (!GetCtrlCurItemData(m_TextCompareCombo[nIdx], dwVal))
				return (false);

			VERIFY(TmpCriteria.SetCurVerbTextCmp(static_cast<CriteriaVerbTextCompare>(dwVal)));
		}
		break;

		case CRITERIA_VERB_EQUAL_COMPARE_TYPE:
		{
			DWORD dwVal = 0;
			if (!GetCtrlCurItemData(m_EqualCompareCombo[nIdx], dwVal))
				return (false);

			VERIFY(TmpCriteria.SetCurVerbEqualCmp(static_cast<CriteriaVerbEqualCompare>(dwVal)));
		}
		break;

		case CRITERIA_VERB_NUM_COMPARE_TYPE:
		{
			DWORD dwVal = 0;
			if (!GetCtrlCurItemData(m_NumCompareCombo[nIdx], dwVal))
				return (false);

			VERIFY(TmpCriteria.SetCurVerbNumCmp(static_cast<CriteriaVerbNumCompare>(dwVal)));
		}
		break;

		case CRITERIA_VERB_DATE_COMPARE_TYPE:
		{
			DWORD dwVal = 0;
			if (!GetCtrlCurItemData(m_DateCompareCombo[nIdx], dwVal))
				return (false);

			VERIFY(TmpCriteria.SetCurVerbDateCmp(static_cast<CriteriaVerbDateCompare>(dwVal)));
		}
		break;

		default: ASSERT(0); break;
	}

	switch (m_CritState[nIdx].m_CurValueType)
	{
		case CRITERIA_VALUE_TEXT_TYPE:
		{
			CString str;
			m_TextEdit[nIdx].GetWindowText(str);

			if (str.IsEmpty())
				return (false);

			VERIFY(TmpCriteria.SetCurValueText(static_cast<CriteriaValueText>(str)));
		}
		break;

		case CRITERIA_VALUE_AGE_TYPE:
		{
			CString str;
			m_NumEdit[nIdx].GetWindowText(str);

			if (str.IsEmpty())
				return (false);

			int nVal = atoi(str);

			VERIFY(TmpCriteria.SetCurValueAge(static_cast<CriteriaValueAge>(nVal)));
		}
		break;

		case CRITERIA_VALUE_ATTACHCOUNT_TYPE:
		{
			CString str;
			m_NumEdit[nIdx].GetWindowText(str);

			if (str.IsEmpty())
				return (false);

			int nVal = atoi(str);

			VERIFY(TmpCriteria.SetCurValueAttachCount(static_cast<CriteriaValueAttachCount>(nVal)));
		}
		break;

		case CRITERIA_VALUE_SIZE_TYPE:
		{
			CString str;
			m_NumEdit[nIdx].GetWindowText(str);

			if (str.IsEmpty())
				return (false);

			int nVal = atoi(str);

			VERIFY(TmpCriteria.SetCurValueSize(static_cast<CriteriaValueSize>(nVal)));
		}
		break;

		case CRITERIA_VALUE_STATUS_TYPE:
		{
			DWORD dwVal = 0;
			if (!GetCtrlCurItemData(m_StatusCombo[nIdx], dwVal))
				return (false);

			VERIFY(TmpCriteria.SetCurValueStatus(static_cast<CriteriaValueStatus>(dwVal)));
		}
		break;

		case CRITERIA_VALUE_LABEL_TYPE:
		{
			const unsigned int nLabelIdx = m_LabelCombo[nIdx].GetCurLabel();

			VERIFY(TmpCriteria.SetCurValueLabel(static_cast<CriteriaValueLabel>(nLabelIdx)));
		}
		break;

		case CRITERIA_VALUE_PERSONA_TYPE:
		{
			CString str;
			if (!GetCtrlCurItemText(m_PersonaCombo[nIdx], str))
				return (false);

			VERIFY(TmpCriteria.SetCurValuePersona(static_cast<CriteriaValuePersona>(str)));
		}
		break;

		case CRITERIA_VALUE_PRIORITY_TYPE:
		{
			DWORD dwVal = 0;
			if (!GetCtrlCurItemData(m_PriorityCombo[nIdx], dwVal))
				return (false);

			VERIFY(TmpCriteria.SetCurValuePriority(static_cast<CriteriaValuePriority>(dwVal)));
		}
		break;

		case CRITERIA_VALUE_DATE_TYPE:
		{
			COleDateTime TmpDate = m_DateTimeCtrl[nIdx].GetDateTime();
			TmpDate.SetDate(TmpDate.GetYear(), TmpDate.GetMonth(), TmpDate.GetDay()); // Zero time

			VERIFY(TmpCriteria.SetCurValueDate(static_cast<CriteriaValueDate>(TmpDate)));
		}
		break;

		default: ASSERT(0); break;
	}

	criteria = TmpCriteria;

	return (true);
}

/////////////////////////////////////////////////////////////////////////////
// CSearchView message handlers

void CSearchView::OnOk() // Search BTN
{
	//	CWaitCursor waiting; // Show wait cursor until scope ends

	m_BeginBtn.EnableWindow(FALSE); // Disbale the "Search" button for visual feedback

	m_ResultsMbxTabCtrl.SetCurSel(TAB_RESULTS_IDX); // Bring the results tab to the front
	UpdateTabContents();

	MainProgress(CRString(IDS_SEARCH_FINDING_MESSAGES));
	Progress( 0, CRString(IDS_SEARCH_PROGRESS_CLEARINGRESULTS), 100 ); // "Clearing results..."
	m_ResultsStatic.SetWindowText(CRString(IDS_SEARCH_PROGRESS_CLEARINGRESULTS)); // "Clearing results..."
	ClearResults();
	
	m_ResultsList.UpdateWindow(); // Update results list window, which should be empty -- visual feedback
	
	Progress(CRString(IDS_SEARCH_PROGRESS_SEARCHING)); // "Searching..."
	m_ResultsStatic.SetWindowText(CRString(IDS_SEARCH_PROGRESS_SEARCHING)); // "Searching..."
	if (!DoSearch())
		ErrorDialog(IDS_SEARCH_INTERNAL_ERROR);

	CloseProgress();

	UpdateResultsText();
	m_ResultsList.UpdateWindow(); // Update the results list window, which now has the results
	m_BeginBtn.EnableWindow(TRUE); // Re-Enable "Search" button
}

// --------------------------------------------------------------------------

void CSearchView::OnMoreBtn()
{
	const bool bHadFocus = (GetFocus() == (&m_MoreBtn));

	AddNewCriteria();

	if ((bHadFocus) && (!m_MoreBtn.IsWindowEnabled()))
		m_LessBtn.SetFocus();
}

// --------------------------------------------------------------------------

void CSearchView::OnLessBtn()
{
	const bool bHadFocus = (GetFocus() == (&m_LessBtn));

	RemoveCriteria();

	if ((bHadFocus) && (!m_LessBtn.IsWindowEnabled()))
		m_MoreBtn.SetFocus();
}

// --------------------------------------------------------------------------

void CSearchView::OnDestroy()
{
	SaveCriteria();

	LV_COLUMN col;
	col.mask = LVCF_WIDTH;

	for (int iCOL = 0; m_ResultsList.GetColumn(iCOL, &col); iCOL++)
	{
		ASSERT(col.cx > 0 && col.cx < 30000);  // Make sure it isn't some bizarre size
		switch (iCOL)
		{
		case COLUMN_DATE:
			// Date Column
			SetIniShort(IDS_INI_SEARCH_DATE_WIDTH, short(col.cx));
			break;
		case COLUMN_WHO:
			// Who Column
			SetIniShort(IDS_INI_SEARCH_WHO_WIDTH, short(col.cx));
			break;
		case COLUMN_SUBJECT:
			// Subject Column
			SetIniShort(IDS_INI_SEARCH_SUBJECT_WIDTH, short(col.cx));
			break;
		case COLUMN_MAILBOX:
			// Mailbox Column
			SetIniShort(IDS_INI_SEARCH_MBOX_WIDTH, short(col.cx));
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	ClearResults(false);

	if (m_UseFastWay)
	{
		if (m_MsgResultArr)
		{
			free(m_MsgResultArr);
			m_MsgResultArr = NULL;
		}
	}

	C3DFormView::OnDestroy();
}

// --------------------------------------------------------------------------

bool CSearchView::DoSearch()
{
	// Get a list of mailbox commands from the tree control
	list<QCMailboxCommand*> cmdlist;
	if (!m_MBoxTree.GetSelectedList(&cmdlist))
		return (false);

	// Build the list of criteria
	//MultSearchCriteria m_MSC;
	m_MSC.GetCriteriaList()->clear(); // erase(msc.GetCriteriaList()->begin(), msc.GetCriteriaList()->end());
	SearchCriteria criteria;

	if (m_AndRadioBtn.GetCheck() == 1)
	{
		m_MSC.SetOpAND();
	}
	else if (m_OrRadioBtn.GetCheck() == 1)
	{
		m_MSC.SetOpOR();
	}
	else
	{
		ASSERT(0);
		return (false);
	}

	for (int idx = 0; idx < m_CurCritCount; idx++)
	{
		if (GetCriteria(idx, criteria))
			m_MSC.Add(criteria); // Only add the valid criteria lines
	}

	// Do a sanity check. We disable the search btn when there are zero
	// valid criteria entries. So there should always be at least one
	// criteria when we get to this point.
	ASSERT(m_MSC.GetCriteriaList()->size() > 0);

	// Build the list of mbx names
	list<CString> strlst;
	list<QCMailboxCommand*>::iterator cmditr;
	for (cmditr = cmdlist.begin(); cmditr != cmdlist.end(); cmditr++)
	{
		strlst.push_back( (*cmditr)->GetPathname() );
	}

	// Avoid destructor calls, make all pointers NULLS
	for (list<QCMailboxCommand*>::iterator i = cmdlist.begin(); i != cmdlist.end(); i++)
		(*i) = (QCMailboxCommand *) NULL;

	// Erase the list
	cmdlist.erase(cmdlist.begin(), cmdlist.end());

	SearchEngine search_eng(&m_MSC, &strlst);
	ResultsCBType cbfn = makeCallback( (ResultsCBType*)0, (*this), &CSearchView::ProcessResults);

#ifdef _MY_TICK_TRACE_
	DWORD nStartTick = GetTickCount();
#endif // _MY_TICK_TRACE_

	if (!search_eng.Start(cbfn, TICK_DELTA))
	{
		ASSERT(0);
		return (false);
	}

#ifdef _MY_TICK_TRACE_
	DWORD nEndTick = GetTickCount();
	DWORD nDelta = nEndTick -  nStartTick;

	::MyTrace("CSearchView::DoSearch: Search completed in %lu ticks, finding %d matches.\n", nDelta, m_nFoundCount);
#endif // _MY_TICK_TRACE_

	m_nIMAPSkippedCount = search_eng.GetImapSkipCount();

	return (true);
}

// --------------------------------------------------------------------------

bool CSearchView::ProcessResults(list<SearchResult> &results)
{
//	::Beep(2500, 10);

	bool bEsc = false;

	if (results.size() > 0)
	{
		m_ResultsList.SetRedraw(FALSE); // Turn off redraw for the results list

		list<SearchResult>::iterator itr;
		for (itr = results.begin(); itr != results.end(); itr++)
		{
			AddResult(*itr);
			if (bEsc = (PumpEsc(TICK_DELTA) == TRUE))
				break;
		}

		m_ResultsList.SetRedraw(TRUE); // Turn redraw back on for the results list

		UpdateResultsText();
	}

	return (!bEsc);
}

// --------------------------------------------------------------------------

void CSearchView::AddResult(const SearchResult &result)
{
	const int nInsertIdx = m_ResultsList.InsertItem(m_ResultsList.GetItemCount(), NULL);

	ASSERT(nInsertIdx != (-1));
	if (nInsertIdx != (-1))
	{
		CMsgResult *pMR = NewMsgResult(result);

		ASSERT(pMR);
		if (!pMR)
			return;

		ASSERT(result.GetMsgID() != (-1));

		// Set the column text
		VERIFY(UpdateListText(result, nInsertIdx));

		// Set the item data
		VERIFY(m_ResultsList.SetItemData(nInsertIdx, (DWORD) pMR));
	}
}

// --------------------------------------------------------------------------

void CSearchView::InitMem()
{
	if (!m_MsgResultArr)
	{
		m_MsgResultArr = (CMsgResult *) malloc(MAX_LISTCOUNT * sizeof(CMsgResult));
		m_MsgResultArrCount = 0;
	}
}

// --------------------------------------------------------------------------

CMsgResult *CSearchView::NewMsgResult(const SearchResult &result)
{
	CMsgResult *pMsg = NULL;

	if (m_UseFastWay)
	{
		ASSERT(m_MsgResultArrCount < MAX_LISTCOUNT);

		if (m_MsgResultArrCount >= MAX_LISTCOUNT)
			return (NULL);

		if (!m_MsgResultArr)
		{
			InitMem();
		}
		
		pMsg = (CMsgResult *) ((m_MsgResultArr) + (m_MsgResultArrCount++));

//		CMsgResult *pPlaceNew = new(pMsg) CMsgResult(result);
		//	CMsgResult msg(result);
		
		//	(*pMsg) = msg;
//		memcpy(pMsg, &msg, sizeof(CMsgResult));
	}
	else
	{
		pMsg = new CMsgResult(result);
		m_MsgResultList.push_back(pMsg);
	}

	return (pMsg);
}

// --------------------------------------------------------------------------

void CSearchView::ClearResults(bool bPump /* = true */)
{
#ifdef _MY_TICK_TRACE_
	::MyTrace("CSearchView::ClearResults: Starting to clear %lu msgs...\n", m_MsgResultList.size());
	DWORD nStartTick = GetTickCount();
#endif // _MY_TICK_TRACE_

	if (m_UseFastWay)
	{
		m_MsgResultArrCount = 0;
	}
	else
	{
		// Use STL list
		CMsgResult *pMR = NULL;
		list<CMsgResult *>::iterator itr;
		for (itr = m_MsgResultList.begin(); itr != m_MsgResultList.end(); ++itr)
		{
			ASSERT(*itr);
			delete (*itr);

			if (bPump)
				PumpEsc(TICK_DELTA); // Pump the msg queue to avoid app lockup
		}

		m_MsgResultList.clear();
	}

#ifdef _MY_TICK_TRACE_
	DWORD nMidTick = GetTickCount();
#endif // _MY_TICK_TRACE_

	m_nIMAPSkippedCount = 0;

	ClearList();
	UpdateResultsText();
//	ResizeColumns();

#ifdef _MY_TICK_TRACE_
	DWORD nEndTick = GetTickCount();

	DWORD nDeltaTick = nEndTick - nStartTick;
	DWORD nEraseDelta = nMidTick - nStartTick;

	::MyTrace("CSearchView::ClearResults: Overall %lu ticks of which %lu ticks to clean mem.\n", nDeltaTick, nEraseDelta);
#endif // _MY_TICK_TRACE_
}

// --------------------------------------------------------------------------

void CSearchView::ClearList()
{
	m_ResultsList.DeleteAllItems();
}

// --------------------------------------------------------------------------

void CSearchView::ResizeColumns()
{
	// Resize the last column to fill width of list

	CRect ClientRct;
	m_ResultsList.GetClientRect(ClientRct);

	const int nMbxColWidth = m_ResultsList.GetColumnWidth(COLUMN_MAILBOX);
	const int nWhoColWidth = m_ResultsList.GetColumnWidth(COLUMN_WHO);
	const int nDateColWidth = m_ResultsList.GetColumnWidth(COLUMN_DATE);
	const int nSubjectColWidth = m_ResultsList.GetColumnWidth(COLUMN_SUBJECT);

	const int nClientRectWidth = ClientRct.Width();
	const int nRemainWidth = nClientRectWidth - (nMbxColWidth + nWhoColWidth + nDateColWidth);

	if ((nRemainWidth > 0) && (nRemainWidth > nSubjectColWidth))
		m_ResultsList.SetColumnWidth(COLUMN_SUBJECT, nRemainWidth);
}

// --------------------------------------------------------------------------

void CSearchView::ResizeTabContents()
{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::ResizeTabContents()\n");
#endif // _MY_TRACE_

	CRect DisplayRct;
	m_ResultsMbxTabCtrl.GetWindowRect(DisplayRct);
	ScreenToClient(DisplayRct);
	m_ResultsMbxTabCtrl.AdjustRect(FALSE, DisplayRct);

	DisplayRct.top += 2;

	ClientToScreen(DisplayRct);
	m_ResultsMbxTabCtrl.ScreenToClient(DisplayRct);

	// These are children of the tab control
	m_ResultsList.MoveWindow(DisplayRct);
	m_MBoxTree.MoveWindow(DisplayRct);
}

// --------------------------------------------------------------------------

void CSearchView::ResizeControls(int cx, int cy)
{
	// Resize the tab rect to the window
	{
		CRect TabRct;	
		m_ResultsMbxTabCtrl.GetWindowRect(TabRct);
		ScreenToClient(TabRct);
		TabRct.bottom = cy - CONTROL_EDGE_SPACING;
		TabRct.right = cx - CONTROL_EDGE_SPACING;
		m_ResultsMbxTabCtrl.MoveWindow(TabRct);
	}

	// Place the controls in the tab display area
	ResizeTabContents();

	// Align the results text
	{
		CRect TabRct(0,0,0,0);

		if (m_ResultsMbxTabCtrl.GetItemCount() > 0)
			m_ResultsMbxTabCtrl.GetItemRect(m_ResultsMbxTabCtrl.GetItemCount() - 1, TabRct);

		CRect StatusRct;	
		m_ResultsStatic.GetWindowRect(StatusRct);
		ScreenToClient(StatusRct);
		StatusRct.left = TabRct.right + CONTROL_EDGE_SPACING + CONTROL_EDGE_SPACING + CONTROL_EDGE_SPACING;
		StatusRct.right = cx - CONTROL_EDGE_SPACING;

		if (StatusRct.Width() < STATUS_TEXT_MIN_WIDTH) // Minimum width
			StatusRct.left = StatusRct.right - STATUS_TEXT_MIN_WIDTH;

		m_ResultsStatic.MoveWindow(StatusRct);
	}

/*	// Align the mbx text
	{
		CRect ChkRct(0,0,0,0);

		CWnd *pWnd = GetDlgItem(IDC_SEARCHWND_SUMMARIESONLY_CHK);
		ASSERT(pWnd);

		if (pWnd)
		{
			pWnd->GetWindowRect(ChkRct);
			ScreenToClient(ChkRct);
		}

		CRect StatusRct;	
		m_MbxStatic.GetWindowRect(StatusRct);
		ScreenToClient(StatusRct);
		StatusRct.left = ChkRct.right + CONTROL_EDGE_SPACING;
		StatusRct.right = cx - CONTROL_EDGE_SPACING;

		if (StatusRct.Width() < STATUS_TEXT_MIN_WIDTH) // Minimum width
			StatusRct.left = StatusRct.right - STATUS_TEXT_MIN_WIDTH;

		m_MbxStatic.MoveWindow(StatusRct);
	}
*/
	// Move the search button right-aligned and extend the edit field
	{
//		CRect EditRct;
//		m_InputEdit.GetWindowRect(EditRct);
//		ScreenToClient(EditRct);

		CRect BtnRct;
		m_BeginBtn.GetWindowRect(BtnRct);
		ScreenToClient(BtnRct);

		const int nBtnWidth = BtnRct.Width();

		BtnRct.right = cx - CONTROL_EDGE_SPACING;
		BtnRct.left = BtnRct.right - nBtnWidth;
		//	EditRct.right = BtnRct.left - CONTROL_EDGE_SPACING;

		//	m_InputEdit.MoveWindow(EditRct);
		m_BeginBtn.MoveWindow(BtnRct);
	}

//	ResizeColumns();
}

// --------------------------------------------------------------------------

void CSearchView::OnSize(UINT nType, int cx, int cy)
{
#ifdef _MY_TRACE_
::MyTrace("CSearchView::OnSize(%d, %d)\n", cx, cy);
#endif // _MY_TRACE_

	C3DFormView::OnSize(nType, cx, cy);

	// Don't bother if the controls haven't been initialized.
	if (!m_bInitilized)
	{
#ifdef _MY_TRACE_
::MyTrace("    CSearchView::OnSize(%d, %d) [SKIPPED]\n", cx, cy);
#endif // _MY_TRACE_
		return;
	}

	ResizeControls(cx, cy); // Non-criteria controls
	ResizeAllCriteriaCtrls();
}

// --------------------------------------------------------------------------

BOOL CSearchView::OpenMsg(CMsgResult *pMR)
{
	ASSERT(pMR);

	if (pMR)
	{
		CTocDoc *pTocDoc = ::GetToc(pMR->m_sPath, NULL, FALSE, FALSE);

		ASSERT(pTocDoc);
		if (!pTocDoc)
			return (FALSE);

		CSummary *pSum = pTocDoc->m_Sums.GetByMessageId(pMR->m_nMsgID);

		ASSERT(pSum);
		if (!pSum)
			return (FALSE);

		CMessageDoc *pDoc = pSum->GetMessageDoc();

		ASSERT(pDoc);
		if (!pDoc)
			return (FALSE);

		//Try to hilite the search string to show the search context in the message
		bool bWholeWord = false;
		CString strSearch = "";
		
		MultSearchCriteria::eStart start = MultSearchCriteria::FIRST;
		while(m_MSC.GetSearchString(strSearch, &bWholeWord, start))
		{
			//search engine doesnt support case-sensitive searches, now; may change later
			if( pDoc->FindFirst(strSearch, false /*no match case*/, bWholeWord, TRUE))
			{
				QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
				ASSERT(pFindMgr);

				//update global find text, so F3 works
				pFindMgr->UpdateLastFindState(strSearch, false, bWholeWord);
				break;
			}
			start = MultSearchCriteria::NEXT;
		}

		// Make sure window is shown, just in case above selection
		// of found text doesn't cause the message window to open.
		if (!pSum->m_FrameWnd || pSum->m_FrameWnd->IsWindowVisible() == FALSE)
			pSum->Display();
	
		return (TRUE);
	}

	return (FALSE);
}

// --------------------------------------------------------------------------

BOOL CSearchView::DeleteMsg(CMsgResult *pMR, int nIdx)
{
	ASSERT(pMR);

	if (pMR)
	{
		CTocDoc *pTocDoc = ::GetToc(pMR->m_sPath, NULL, FALSE, FALSE);

		ASSERT(pTocDoc);
		if (!pTocDoc)
			return (FALSE);

		CSummary *pSum = pTocDoc->m_Sums.GetByMessageId(pMR->m_nMsgID);

		ASSERT(pSum);
		if (!pSum)
			return (FALSE);

		if (VerifyDelete(pSum))
			VERIFY(DoDelete(pSum));

		return (TRUE);
	}

	return (FALSE);
}

// --------------------------------------------------------------------------

BOOL CSearchView::UpdateListInfo(CSummary *pSum, int nIdx)
{
	SearchResult TmpResult(pSum);
	VERIFY(UpdateListText(TmpResult, nIdx));

	CMsgResult *pMR = (CMsgResult *) m_ResultsList.GetItemData(nIdx);
	ASSERT(pMR);

	if (pMR)
	{
		// Copy the new data into the old result location
		CMsgResult NewResult(TmpResult);
		(*pMR) = NewResult;

		return (TRUE);
	}

	return (FALSE);
}

// --------------------------------------------------------------------------

BOOL CSearchView::UpdateListText(const SearchResult &result, int nIdx)
{
	m_ResultsList.SetItemText(nIdx, COLUMN_MAILBOX, result.GetMbxName() );
	m_ResultsList.SetItemText(nIdx, COLUMN_WHO, result.GetWho());
	m_ResultsList.SetItemText(nIdx, COLUMN_DATE, result.GetDate());
	m_ResultsList.SetItemText(nIdx, COLUMN_SUBJECT, result.GetSubject());

	return (TRUE);
}

// --------------------------------------------------------------------------

BOOL CSearchView::OpenTOC(CMsgResult *pMR)
{
	ASSERT(pMR);

	if (pMR)
	{
		CTocDoc *pTocDoc = ::GetToc(pMR->m_sPath, NULL, FALSE, FALSE);

		ASSERT(pTocDoc);
		if (!pTocDoc)
			return (FALSE);

		CSummary *pSum = pTocDoc->m_Sums.GetByMessageId(pMR->m_nMsgID);

		ASSERT(pSum);
		if (!pSum)
			return (FALSE);

		CTocView* pTocView = pTocDoc->GetView();

		// If there is a TOC view already, remove any selection
		if (pTocView)
			pTocView->SelectAll(FALSE, TRUE);

		const BOOL bDisplayed = pTocDoc->Display();
		
		ASSERT(bDisplayed);
		if (bDisplayed)
			pSum->Select();
	}

	return (TRUE);
}

// --------------------------------------------------------------------------

bool CSearchView::GetSumList(list<CSummary *> &SumList, /* const */ list<int> &IdxList)
{
	SumList.clear();

	CMsgResult *pMR = NULL;
	CTocDoc *pTocDoc = NULL;
	CSummary *pSum = NULL;

	list<int>::iterator itr = IdxList.begin();
	while (itr != IdxList.end())
	{
		pMR = (CMsgResult *) m_ResultsList.GetItemData(*itr);
		ASSERT(pMR);
		if (!pMR)
			continue;

		pTocDoc = ::GetToc(pMR->m_sPath, NULL, FALSE, FALSE);
		ASSERT(pTocDoc);
		if (!pTocDoc)
			continue;

		pSum = pTocDoc->m_Sums.GetByMessageId(pMR->m_nMsgID);
		ASSERT(pSum);
		if (!pSum)
			continue;

		const int MBType = pTocDoc->m_Type;
		if (MBType == MBT_TRASH)
		{
			IdxList.erase(itr++);
		}
		else
		{
			SumList.push_back(pSum);
			++itr;
		}
	}

	return (true);
}

// --------------------------------------------------------------------------

bool CSearchView::VerifyDelete(bool bUnread, bool bQueued, bool bSendable)
{
	const bool bWarnDeleteSearchResult = (GetIniShort(IDS_INI_WARN_DELETE_SEARCHRESULT) != 0);
	const bool bWarnDeleteUnread = (GetIniShort(IDS_INI_WARN_DELETE_UNREAD) != 0);
	const bool bWarnDeleteQueued = (GetIniShort(IDS_INI_WARN_DELETE_QUEUED) != 0);
	const bool bWarnDeleteUnsent = (GetIniShort(IDS_INI_WARN_DELETE_UNSENT) != 0);

	if (bWarnDeleteSearchResult)
	{
		// Deleting messages from the results list will move the messages to the trash. Continue?
		if (WarnDialog(IDS_INI_WARN_DELETE_SEARCHRESULT, IDS_WARN_DELETE_SEARCHRESULT) != IDOK)
			return (false);
	}
	else if (bUnread && bWarnDeleteUnread)
	{
		if (WarnDialog(IDS_INI_WARN_DELETE_UNREAD, IDS_WARN_DELETE_UNREAD) != IDOK)
			return (false);
	}
	else if (bQueued && bWarnDeleteQueued)
	{
		if (WarnDialog(IDS_INI_WARN_DELETE_QUEUED, IDS_WARN_DELETE_QUEUED) != IDOK)
			return (false);
	}
	else if (bSendable && bWarnDeleteUnsent)
	{
		if (WarnDialog(IDS_INI_WARN_DELETE_UNSENT, IDS_WARN_DELETE_UNSENT) != IDOK)
			return (false);
	}

	return (true);
}

// --------------------------------------------------------------------------

bool CSearchView::VerifyDelete(CSummary *pSum)
{
	const bool bUnread = (pSum->m_State == MS_UNREAD);
	const bool bQueued = (pSum->IsQueued() == TRUE);
	const bool bSendable = (pSum->IsSendable() == TRUE);

	return (VerifyDelete(bUnread, bQueued, bSendable));
}

// --------------------------------------------------------------------------

// VerifyDelete [PROTECTED]
//
// Return true if delete should happen; false cancels delete action

bool CSearchView::VerifyDelete(/* const */ list<CSummary *> &SumList)
{
	bool bUnread = false, bQueued = false, bSendable = false;

	for (list<CSummary *>::iterator itr = SumList.begin(); itr != SumList.end(); ++itr)
	{
		if (!bUnread)
			bUnread = ((*itr)->m_State == MS_UNREAD);

		if (!bQueued)
			bQueued = (((*itr)->IsQueued()) == TRUE);

		if (!bSendable)
			bSendable = (((*itr)->IsSendable()) == TRUE);

		if (bUnread && bQueued && bSendable)
			break; // Optimization -- stop looking if everything is true
	}

	return (VerifyDelete(bUnread, bQueued, bSendable));
}

// --------------------------------------------------------------------------

bool CSearchView::RemoveFromResultsList(int nIdx)
{
	return (m_ResultsList.DeleteItem(nIdx) == TRUE);
}

// --------------------------------------------------------------------------

bool CSearchView::RemoveFromResultsList(/* const */ list<int> &IdxList)
{
	Progress(0, CRString(IDS_SEARCH_PROGRESS_EMPTYINGLIST), IdxList.size() ); // "Emptying list..."

	int nOffset = 0;
	bool bEsc = false;

	for (list<int>::iterator itr = IdxList.begin(); itr != IdxList.end(); ++itr)
	{
		if (!RemoveFromResultsList( (*itr) - (nOffset++) ))
		{
			bEsc = true;
			break;
		}

		ProgressAdd(1);

		if (bEsc = (PumpEsc(TICK_DELTA) == TRUE))
			break;
	}

	CloseProgress();

	return (!bEsc);
}

// --------------------------------------------------------------------------

bool CSearchView::DoDelete(CSummary *pSum)
{
	ASSERT(pSum);

	CTocDoc *pTocDoc = pSum->m_TheToc;
	ASSERT(pTocDoc);

	const int MBType = pTocDoc->m_Type;
	ASSERT(MBType != MBT_TRASH);
	if (MBType == MBT_TRASH)
		return (false); // We do not allow search results to delete from TRASH

#ifdef IMAP4
	// If this is an IMAP mailbox, just flag messages and get out.
	if (pTocDoc->IsImapToc())
	{
		// Pass this to the IMAP command object.
		//
		QCMailboxCommand* pImapFolder = g_theMailboxDirector.FindByPathname( (const char *) pTocDoc->MBFilename() );
		if( pImapFolder != NULL )
			pImapFolder->Execute( CA_DELETE_MESSAGE, pSum);
		else
		{
			ASSERT(0);
		}

		return (true);
	}
#endif // IMAP4

	CTocDoc *TrashToc = GetTrashToc();
	if (TrashToc)
	{
		//
		// We no longer call Xfer() here since we want to go through
		// the command director logic for CA_TRANSFER_TO.  The command
		// director notifies the TOC window containing this message
		// that it is okay to invoke the "auto-mark-as-read" logic
		// since the mailbox was changed due to an explicit user
		// action.  The Xfer() function does not notify the TOC
		// window.
		//
		//doc->Xfer(TrashToc);
		QCMailboxCommand* pFolder = g_theMailboxDirector.FindByPathname( (const char *) TrashToc->MBFilename() );
		if( pFolder != NULL )
			pFolder->Execute( CA_TRANSFER_TO, pSum);
		else
		{
			ASSERT(0);
		}
	}

	return (true);
}

// --------------------------------------------------------------------------

bool CSearchView::DoDelete(/* const */ list<CSummary *> &SumList)
{
//	// Clean up any unsaved open composition windows
//	if (MBType == MBT_OUT)
//	{
//		SelCount = m_SumListBox.GetSelCount();
//		for (int i = m_SumListBox.GetCount() - 1; i >= 0 && SelCount; i--)
//		{
//			if (m_SumListBox.GetSel(i) > 0)
//			{
//				CSummary* Sum = m_SumListBox.GetItemDataPtr(i);
//				CCompMessageDoc* doc = (CCompMessageDoc*)Sum->FindMessageDoc();
//				if (doc && doc->m_HasBeenSaved == FALSE)
//					doc->OnSaveDocument(NULL);
//				SelCount--;
//			}
//		}
//	}

	Progress(0, CRString(IDS_SEARCH_PROGRESS_DELETING), SumList.size()); // "Deleting..."

	bool bEsc = false;

	for (list<CSummary *>::iterator itr = SumList.begin(); itr != SumList.end(); ++itr)
	{
		VERIFY(DoDelete(*itr));
		ProgressAdd(1);

		if (bEsc = (PumpEsc(TICK_DELTA) == TRUE))
			break;
	}

	CloseProgress();

	return (!bEsc);
}

// --------------------------------------------------------------------------

bool CSearchView::DeleteSelItems()
{
	list<int> SelIdxList;
	if (!m_ResultsList.GetCurSel(&SelIdxList))
		return (false);

	if (SelIdxList.size() > 0)
	{
		int nFirstSel = (*SelIdxList.begin());

		list<CSummary *> SumList;
		if (!GetSumList(SumList, SelIdxList))
			return (false);

		// Ask user if they want to delete
		if (VerifyDelete(SumList))
		{
			m_ResultsList.SetRedraw(FALSE); // Turn off redraw for the results list
			CWaitCursor wait_cursor;

			VERIFY(RemoveFromResultsList(SelIdxList));
			VERIFY(DoDelete(SumList));

			if ((m_ResultsList.GetCurSel(&SelIdxList)) && (SelIdxList.size() < 1))
			{
				const int nNewSel = __min((m_ResultsList.GetItemCount() - 1), nFirstSel);
				m_ResultsList.SetCurSel(nNewSel);
			}

			m_ResultsList.SetRedraw(TRUE); // Turn redraw back on for the results list
		}
	}

	return (true);
}

// --------------------------------------------------------------------------

LONG CSearchView::OnMsgListDeleteKey(WPARAM wParam, LPARAM lParam)
{
	UINT nRepCnt = (UINT) wParam;
	CListCtrlEx *pList = (CListCtrlEx *) lParam;

	ASSERT(pList == (&m_ResultsList));

	if (pList == (&m_ResultsList))
		VERIFY(DeleteSelItems());

	return (0); // Ignored
}

// --------------------------------------------------------------------------

LONG CSearchView::OnMsgListRightClick(WPARAM wParam, LPARAM lParam)
{
	UINT nFlags = (UINT) wParam;
	CPoint point((DWORD)lParam); // screen-based point of click

	CPoint clientPt = point;
	m_ResultsList.ScreenToClient(&clientPt);
	int idx = m_ResultsList.HitTest(clientPt);

	// Ensure we right-clicked on a valid item. Although the ListCtrl should
	// never tell us to handle an invalid right-click, this is a safety check.

	ASSERT(idx != (-1));
	if (idx == (-1))
		return (0);

	// Popup menu stuff
	CMenu menu;
	menu.CreatePopupMenu();

	const UINT nMENUID_OpenTOC = 1;
	const UINT nMENUID_OpenMSG = 2;
	const UINT nMENUID_DeleteMSG = 3;

	menu.AppendMenu(MF_STRING, nMENUID_OpenMSG, LPCSTR(CRString(IDS_SEARCH_RESULTS_POPUPMENU_OPENMSG))); // "Open Message\tEnter"
	menu.AppendMenu(MF_STRING, nMENUID_OpenTOC, LPCSTR(CRString(IDS_SEARCH_RESULTS_POPUPMENU_OPENMBX))); // "Open Mailbox\tCtrl+Enter"
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, nMENUID_DeleteMSG, LPCSTR(CRString(IDS_SEARCH_RESULTS_POPUPMENU_DELMSG))); // "Delete Message\tDel"

	DWORD sel = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON
					| TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, this);

	menu.DestroyMenu();

	// Make sure we selected something
	if (0 != sel)
	{
		if (nMENUID_DeleteMSG == sel) // Delete message (no looping needed)
		{
			VERIFY(DeleteSelItems()); // Deletes all selected items in one easy-as-pie call
		}
		else
		{
			// Loop through selected items and do our thang
			for (int i = m_ResultsList.GetNextItem( -1, LVNI_ALL | LVNI_SELECTED); i != -1; i = m_ResultsList.GetNextItem( i, LVNI_ALL | LVNI_SELECTED))
			{
				CMsgResult *pMR = (CMsgResult *) m_ResultsList.GetItemData(i);

				switch (sel)
				{
					case nMENUID_OpenTOC: // Open TOC
					{
						OpenTOC(pMR);
					}
					break;

					case nMENUID_OpenMSG: // Open message
					{
						OpenMsg(pMR);
					}
					break;

					default: // Bad, very bad
						ASSERT(0);
				}
			}
		}
	}

	return (0); // Ignored
}

// --------------------------------------------------------------------------

LONG CSearchView::OnMsgListDblClk(WPARAM wParam, LPARAM lParam)
{
	const BOOL bControlDown = ControlDown();

	UINT uFlags = (UINT) wParam; // MK_CONTROL, MK_LBUTTON, MK_MBUTTON, MK_RBUTTON, MK_SHIFT
	CPoint point((DWORD)lParam); // screen-based point of click

	CPoint clientPt = point;
	m_ResultsList.ScreenToClient(&clientPt);
	int idx = m_ResultsList.HitTest(clientPt);

	ASSERT(idx != (-1));
	if (idx == (-1))
		return (0);

	CMsgResult *pMR = (CMsgResult *) m_ResultsList.GetItemData(idx);

	if (bControlDown)
		VERIFY(OpenTOC(pMR));
	else
		VERIFY(OpenMsg(pMR));

	return (0); // Ignored
}

// --------------------------------------------------------------------------

LONG CSearchView::OnMsgListReturnKey(WPARAM wParam, LPARAM lParam)
{
	const BOOL bControlDown = ControlDown();

	const UINT nRepCnt = (const UINT) wParam;
	const CListCtrlEx *pList = (const CListCtrlEx *) lParam;

	ASSERT(pList == (&m_ResultsList));

	if (pList == (&m_ResultsList))
	{
		list<int> SelList;
		if (!m_ResultsList.GetCurSel(&SelList))
			return (0);

		for (list<int>::iterator iter = SelList.begin(); iter != SelList.end(); iter++)
		{
			int nIdx = (*iter);
			CMsgResult *pMR = (CMsgResult *) m_ResultsList.GetItemData(nIdx);

			if (bControlDown)
				VERIFY(OpenTOC(pMR));
			else
				VERIFY(OpenMsg(pMR));
		}
	}

	return (0); // Ignored
}

// --------------------------------------------------------------------------

void CSearchView::OnTabSelchange(NMHDR* pNMHDR, LRESULT* pResult) // TCN_SELCHANGE from IDC_SEARCHWND_RESULTS_TAB
{
	UpdateTabContents();
}

// --------------------------------------------------------------------------

void CSearchView::UpdateTabContents()
{
	int nCurTab = m_ResultsMbxTabCtrl.GetCurFocus();

	ASSERT( (TAB_RESULTS_IDX == nCurTab) | (TAB_MAILBOXES_IDX == nCurTab) );

	switch (nCurTab)
	{
		case (TAB_RESULTS_IDX): // Results Tab
		{
			m_ResultsList.ShowWindow(SW_SHOW);
			m_MBoxTree.ShowWindow(SW_HIDE);
		}
		break;

		case (TAB_MAILBOXES_IDX): // Mailboxes Tab
		{
			m_ResultsList.ShowWindow(SW_HIDE);
			m_MBoxTree.ShowWindow(SW_SHOW);
		}
		break;
	};
}

// --------------------------------------------------------------------------

void CSearchView::UpdateStatusText()
{
	CString txt = "";

	if (m_bShowFoundCount)
	{
		CString str;

		// First do num of matches
		if (0 == m_nFoundCount)
			str = LPCTSTR(CRString(IDS_SEARCH_STATUS_NOMATCHES)); // "No matches"
		else if (1 == m_nFoundCount)
			str = LPCTSTR(CRString(IDS_SEARCH_STATUS_ONEMATCH)); // "One match"
		else
			str.Format(CRString(IDS_SEARCH_STATUS_N_MATCHES_FMT), m_nFoundCount); // "%d matches"
		str += ' ';

		// Add to main text
		txt += str;

		// Do skipped count, if any
		if (m_nIMAPSkippedCount > 0)
		{
			
			str.Format(CRString(IDS_SEARCH_STATUS_SKIPPED_FMT), m_nIMAPSkippedCount); // "(%d skipped)"
			str += ' ';
			txt += str;
		}

		// Do mbx count
		str.Format(CRString(IDS_SEARCH_STATUS_MBXCOUNT_FMT), m_nCheckCount); // "in %u mailbox"

		if (m_nCheckCount > 1)
			str += CRString(IDS_SEARCH_STATUS_MBXCOUNT_PLURAL); // "es"

		txt += str;
	}
	else
	{
		if (0 == m_nCheckCount)
			txt = CRString(IDS_SEARCH_MBXSEL_NOMBX); // "No mailboxes selected"
		else if (1 == m_nCheckCount)
			txt = CRString(IDS_SEARCH_MBXSEL_ONEMBX); // "One mailbox selected"
		else
			txt.Format(CRString(IDS_SEARCH_MBXSEL_ONEMBX_N_MBX_FMT), m_nCheckCount); // "%u mailboxes selected"
	}

	m_ResultsStatic.SetWindowText(txt);
}

// --------------------------------------------------------------------------

void CSearchView::UpdateMbxText()
{
	m_nCheckCount = m_MBoxTree.GetCheckCount();
	m_bShowFoundCount = FALSE; // Lose found info when you change the mailbox checks

	UpdateStatusText();
}

// --------------------------------------------------------------------------

void CSearchView::UpdateResultsText()
{
	m_bShowFoundCount = TRUE;
	m_nFoundCount = m_ResultsList.GetItemCount();
	UpdateStatusText();
}

// --------------------------------------------------------------------------

LONG CSearchView::OnMsgTreeCheckChange(WPARAM wParam, LPARAM lParam)
{
	UpdateMbxText();
	UpdateSearchBtn();

	return (0); // ignored
}

// --------------------------------------------------------------------------

void CSearchView::UpdateAndOrBtns()
{
	m_AndRadioBtn.EnableWindow(m_CurCritCount > 1);
	m_OrRadioBtn.EnableWindow(m_CurCritCount > 1);
}

// --------------------------------------------------------------------------

void CSearchView::UpdateMoreLessBtn()
{
	m_LessBtn.EnableWindow(m_CurCritCount > 1);
	m_MoreBtn.EnableWindow(m_CurCritCount < MAX_CRITERIA);
}

// --------------------------------------------------------------------------

void CSearchView::UpdateSearchBtn()
{
	bool bEnabled = false;
	SearchCriteria criteria;

	for (int idx = 0; (!bEnabled) && (idx < m_CurCritCount); idx++)
		bEnabled = GetCriteria(idx, criteria);

	m_BeginBtn.EnableWindow(bEnabled ? TRUE : FALSE);
}

// --------------------------------------------------------------------------

void CSearchView::OnSelchangeCriteriaCategoryCombo1() // IDC_CRITERIA_CATEGORY_COMBO_1
{
	OnCategoryChange(0);
}

// --------------------------------------------------------------------------

void CSearchView::OnSelchangeCriteriaCategoryCombo2() // IDC_CRITERIA_CATEGORY_COMBO_2
{
	OnCategoryChange(1);
}

// --------------------------------------------------------------------------

void CSearchView::OnSelchangeCriteriaCategoryCombo3() // IDC_CRITERIA_CATEGORY_COMBO_3
{
	OnCategoryChange(2);
}

// --------------------------------------------------------------------------

void CSearchView::OnSelchangeCriteriaCategoryCombo4() // IDC_CRITERIA_CATEGORY_COMBO_4
{
	OnCategoryChange(3);
}

// --------------------------------------------------------------------------

void CSearchView::OnSelchangeCriteriaCategoryCombo5() // IDC_CRITERIA_CATEGORY_COMBO_5
{
	OnCategoryChange(4);
}

// --------------------------------------------------------------------------

void CSearchView::OnChangeTextEdit1() // IDC_CRITERIA_TEXT_EDIT_1
{
	UpdateSearchBtn();
}

// --------------------------------------------------------------------------

void CSearchView::OnChangeTextEdit2() // IDC_CRITERIA_TEXT_EDIT_2
{
	UpdateSearchBtn();
}

// --------------------------------------------------------------------------

void CSearchView::OnChangeTextEdit3() // IDC_CRITERIA_TEXT_EDIT_3
{
	UpdateSearchBtn();
}

// --------------------------------------------------------------------------

void CSearchView::OnChangeTextEdit4() // IDC_CRITERIA_TEXT_EDIT_4
{
	UpdateSearchBtn();
}

// --------------------------------------------------------------------------

void CSearchView::OnChangeTextEdit5() // IDC_CRITERIA_TEXT_EDIT_5
{
	UpdateSearchBtn();
}

//
// PumpEsc
//
// Returns TRUE if ESC has been pressed; FALSE otherwise
//

BOOL CSearchView::PumpEsc(DWORD nDeltaTick)
{
	static DWORD nLastTick = 0;

	//
	// Never pump more than once every nDeltaTick ticks
	//

	if ((GetTickCount() - nLastTick) < nDeltaTick)
		return (FALSE);

	//
	// Retrieve and dispatch any waiting messages, looking for ESC
	//

	BOOL bESC = ::EscapePressed();

	nLastTick = GetTickCount();

	return bESC;
}
// --------------------------------------------------------------------------

//
// FIND TEXT
//

void CSearchView::OnUpdateEditFindFindText(CCmdUI* pCmdUI) // Find (Ctrl-F)
{
	pCmdUI->Enable(TRUE);
}

void CSearchView::OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI) // Find Again (F3)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if ((pFindMgr) && (pFindMgr->CanFindAgain()))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

LONG CSearchView::OnFindReplace(WPARAM wParam, LPARAM lParam) // WM_FINDREPLACE
{
	// Pass the message to either the resutls list or the mbx tree control
	// whichever is currently shown.

	CWnd *pWnd = NULL;
	int nCurTab = m_ResultsMbxTabCtrl.GetCurFocus();

	ASSERT( (TAB_RESULTS_IDX == nCurTab) | (TAB_MAILBOXES_IDX == nCurTab) );

	switch (nCurTab)
	{
		case (TAB_RESULTS_IDX): // Results Tab
		{
			pWnd = (&m_ResultsList);
		}
		break;

		case (TAB_MAILBOXES_IDX): // Mailboxes Tab
		{
			pWnd = (&m_MBoxTree);
		}
		break;
	};

	ASSERT(pWnd);
	if (!pWnd)
		return (0);

	return (pWnd->SendMessage(WM_FINDREPLACE, wParam, lParam));
}

// --------------------------------------------------------------------------

// 
// ParseStringList
//
// Given a string which contains a list of items, this func will parse the list
// and return a pointer to each item; optionally you can replace each delim char
// with another char. 
//
//

int ParseStringList(LPSTR pStr, char cDelimChar, vector<LPCSTR> &vParseList, char cReplaceChar)
{
	const bool bReplace = (cDelimChar != cReplaceChar);
	LPSTR pLastStart, pCurPos;

	vParseList.clear();

	pLastStart = pCurPos = pStr;

	while (*pCurPos)
	{
		if ((*pCurPos) != cDelimChar)
		{
			++pCurPos;
		}
		else
		{
			// We're at the separator character
			if (bReplace)
				(*pCurPos) = cReplaceChar;

			vParseList.push_back(pLastStart);
			pLastStart = (++pCurPos);
		}
	}

	vParseList.push_back(pLastStart);

	return (vParseList.size());
}

// --------------------------------------------------------------------------

bool StrIsDigit(LPCTSTR pStr)
{
	while (*pStr)
		if (!isdigit(*pStr))
			return (false);

	return (true);
}

// --------------------------------------------------------------------------

// IDS_INI_SEARCH_CRITERIA_COUNT

//
// LoadCriteria [PROTECTED]
//

bool CSearchView::LoadCriteria()
{
	// Force the count to be between 1 and 5
	const int nCount = __min(__max(((int) GetIniShort(IDS_INI_SEARCH_CRITERIA_COUNT)), 1), 5);
	ASSERT(nCount >= 1);
	ASSERT(nCount <= 5);

	while (m_CurCritCount > nCount)
		RemoveCriteria();

	while (m_CurCritCount < nCount)
		AddNewCriteria();

	ASSERT(m_CurCritCount == nCount);

	const CString sDefaultStr
		= CRString(IDS_SEARCH_CATEGORYSTR_ANYWHERE)
			+ m_SaveStrSeperator
			+ CRString(IDS_SEARCH_COMPARESTR_CONTAINS)
			+ m_SaveStrSeperator; // "Anywhere;Contains;"

	CString str;
	GetIniString(IDS_INI_SEARCH_CRITERIA_1, str);
	InitializeCriteriaCtrls(0);
	if (!SetCriteriaIniString(0, str))
		SetCriteriaIniString(0, sDefaultStr);

	GetIniString(IDS_INI_SEARCH_CRITERIA_2, str);
	InitializeCriteriaCtrls(1);
	if (!SetCriteriaIniString(1, str))
		SetCriteriaIniString(0, sDefaultStr);

	GetIniString(IDS_INI_SEARCH_CRITERIA_3, str);
	InitializeCriteriaCtrls(2);
	if (!SetCriteriaIniString(2, str))
		SetCriteriaIniString(0, sDefaultStr);

	GetIniString(IDS_INI_SEARCH_CRITERIA_4, str);
	InitializeCriteriaCtrls(3);
	if (!SetCriteriaIniString(3, str))
		SetCriteriaIniString(0, sDefaultStr);

	GetIniString(IDS_INI_SEARCH_CRITERIA_5, str);
	InitializeCriteriaCtrls(4);
	if (!SetCriteriaIniString(4, str))
		SetCriteriaIniString(0, sDefaultStr);

	return (true);
}

// --------------------------------------------------------------------------

//
// SaveCriteria [PROTECTED]
//

bool CSearchView::SaveCriteria()
{
	VERIFY(SetIniShort(IDS_INI_SEARCH_CRITERIA_COUNT, (short) m_CurCritCount));

	CString str;
	if (GetCriteriaIniString(0, str))
		SetIniString(IDS_INI_SEARCH_CRITERIA_1, str);

	if (GetCriteriaIniString(1, str))
		SetIniString(IDS_INI_SEARCH_CRITERIA_2, str);

	if (GetCriteriaIniString(2, str))
		SetIniString(IDS_INI_SEARCH_CRITERIA_3, str);

	if (GetCriteriaIniString(3, str))
		SetIniString(IDS_INI_SEARCH_CRITERIA_4, str);

	if (GetCriteriaIniString(4, str))
		SetIniString(IDS_INI_SEARCH_CRITERIA_5, str);

	return (true);
}

bool CSearchView::GetCriteriaIniString(int nIdx, CString &str)
{
	str.Empty();

	if (!m_bCritInitd[nIdx])
	{
		str = CRString(IDS_SEARCH_CATEGORYSTR_ANYWHERE)
			+ m_SaveStrSeperator
			+ CRString(IDS_SEARCH_COMPARESTR_CONTAINS)
			+ m_SaveStrSeperator; // "Anywhere;Contains;"

		return (true);
	}

	ASSERT(m_bCritInitd[nIdx]);

	CString tmpStr;

	m_CategoryCombo[nIdx].GetWindowText(tmpStr);

	ASSERT(!tmpStr.IsEmpty());

	str += tmpStr;
	str += m_SaveStrSeperator;

	tmpStr.Empty();
	switch (m_CritState[nIdx].m_CurVerbType)
	{
		case CRITERIA_VERB_TEXT_COMPARE_TYPE:
		{
			m_TextCompareCombo[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VERB_EQUAL_COMPARE_TYPE:
		{
			m_EqualCompareCombo[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VERB_NUM_COMPARE_TYPE:
		{
			m_NumCompareCombo[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VERB_DATE_COMPARE_TYPE:
		{
			m_DateCompareCombo[nIdx].GetWindowText(tmpStr);
		}
		break;

		default: ASSERT(0); break;
	}

	ASSERT(!tmpStr.IsEmpty());

	str += tmpStr;
	str += m_SaveStrSeperator;

	tmpStr.Empty();
	switch (m_CritState[nIdx].m_CurValueType)
	{
		case CRITERIA_VALUE_TEXT_TYPE:
		{
			m_TextEdit[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VALUE_AGE_TYPE:
		{
			m_NumEdit[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VALUE_ATTACHCOUNT_TYPE:
		{
			m_NumEdit[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VALUE_SIZE_TYPE:
		{
			m_NumEdit[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VALUE_STATUS_TYPE:
		{
			m_StatusCombo[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VALUE_LABEL_TYPE:
		{
			m_LabelCombo[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VALUE_PERSONA_TYPE:
		{
			m_PersonaCombo[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VALUE_PRIORITY_TYPE:
		{
			m_PriorityCombo[nIdx].GetWindowText(tmpStr);
		}
		break;

		case CRITERIA_VALUE_DATE_TYPE:
		{
			COleDateTime TmpDate = m_DateTimeCtrl[nIdx].GetDateTime();
			tmpStr = TmpDate.Format(CRString(IDS_SEARCH_SAVEDATE_FMT)); // "%m/%d/%Y"
		}
		break;

		default: ASSERT(0); break;
	}

	str += tmpStr;

	return (true);
}

bool CSearchView::SetCriteriaIniString(int nIdx, LPCSTR pIniStr)
{
	ASSERT(m_bCritInitd[nIdx]);
	if (!m_bCritInitd[nIdx])
		return (false);

	CString CategoryStr, VerbStr, ValueStr;

	{
		LPSTR str = new char[strlen(pIniStr) + 1];
		ASSERT(str);

		strcpy(str, pIniStr);

		vector<LPCSTR> vParseList;

		const int nCount = ParseStringList(str, m_SaveStrSeperator, vParseList, '\0');

		ASSERT(3 == nCount);

		if (3 == nCount)
		{
			CategoryStr = vParseList[0];
			VerbStr = vParseList[1];
			ValueStr = vParseList[2];
		}

		delete[] str;
	}

	ASSERT(!CategoryStr.IsEmpty());
	ASSERT(!VerbStr.IsEmpty());

	if ((CategoryStr.IsEmpty()) || (VerbStr.IsEmpty()))
		return (false);

	bool bSuccess = (m_CategoryCombo[nIdx].SelectString(-1, CategoryStr) != CB_ERR);
	ASSERT(bSuccess);

	if (!bSuccess)
		return (false);

	OnCategoryChange(nIdx);

	switch (m_CritState[nIdx].m_CurVerbType)
	{
		case CRITERIA_VERB_TEXT_COMPARE_TYPE:
		{
			bSuccess = (m_TextCompareCombo[nIdx].SelectString(-1, VerbStr) != CB_ERR);
		}
		break;

		case CRITERIA_VERB_EQUAL_COMPARE_TYPE:
		{
			bSuccess = (m_EqualCompareCombo[nIdx].SelectString(-1, VerbStr) != CB_ERR);
		}
		break;

		case CRITERIA_VERB_NUM_COMPARE_TYPE:
		{
			bSuccess = (m_NumCompareCombo[nIdx].SelectString(-1, VerbStr) != CB_ERR);
		}
		break;

		case CRITERIA_VERB_DATE_COMPARE_TYPE:
		{
			bSuccess = (m_DateCompareCombo[nIdx].SelectString(-1, VerbStr) != CB_ERR);
		}
		break;

		default: ASSERT(0); bSuccess = false; break;
	}

	ASSERT(bSuccess);

	if (!bSuccess)
		return (false);

	switch (m_CritState[nIdx].m_CurValueType)
	{
		case CRITERIA_VALUE_TEXT_TYPE:
		{
			m_TextEdit[nIdx].SetWindowText(ValueStr);
		}
		break;

		case CRITERIA_VALUE_AGE_TYPE:
		case CRITERIA_VALUE_ATTACHCOUNT_TYPE:
		case CRITERIA_VALUE_SIZE_TYPE:
		{
			m_NumEdit[nIdx].SetWindowText(ValueStr);
		}
		break;

		case CRITERIA_VALUE_STATUS_TYPE:
		{
			bSuccess = (m_StatusCombo[nIdx].SelectString(-1, ValueStr) != CB_ERR);
		}
		break;

		case CRITERIA_VALUE_LABEL_TYPE:
		{
			bSuccess = (m_LabelCombo[nIdx].SelectString(-1, ValueStr) != CB_ERR);
		}
		break;

		case CRITERIA_VALUE_PERSONA_TYPE:
		{
			bSuccess = (m_PersonaCombo[nIdx].SelectString(-1, ValueStr) != CB_ERR);
		}
		break;

		case CRITERIA_VALUE_PRIORITY_TYPE:
		{
			bSuccess = (m_PriorityCombo[nIdx].SelectString(-1, ValueStr) != CB_ERR);
		}
		break;

		case CRITERIA_VALUE_DATE_TYPE:
		{
			COleDateTime TmpDate;
			bSuccess = (TmpDate.ParseDateTime(ValueStr, VAR_DATEVALUEONLY) == TRUE);

			if (bSuccess)
				m_DateTimeCtrl[nIdx].SetDateTime(TmpDate);
		}
		break;

		default: ASSERT(0); bSuccess = false; break;
	}

	ASSERT(bSuccess);

	if (!bSuccess)
		return (false);

	return (true);
}

LONG CSearchView::OnMsgMailboxSelect(WPARAM wParam, LPARAM lParam)
{
	QCMailboxCommand *pMbxCmd = (QCMailboxCommand *) wParam;
	
	ASSERT(pMbxCmd);
	if (!pMbxCmd)
		return (0);

	// Check this MBX and uncheck everything else
	HTREEITEM hItem = m_MBoxTree.FindCheck(pMbxCmd, true);

	// Now select that item, which will also scroll it in to view
	ASSERT(hItem);
	if (hItem)
	{
		if (m_MBoxTree.ItemHasChildren(hItem))
			VERIFY(m_MBoxTree.Expand(hItem, TVE_EXPAND));
		VERIFY(m_MBoxTree.Select(hItem, TVGN_CARET));
	}

	return (0);
}

// ==========================================================================
//
// SingleCritState
//
// ==========================================================================


SingleCritState::SingleCritState()
{ }

SingleCritState::SingleCritState(const SingleCritState &copy) // Copy constructor
	: m_CurObj(copy.m_CurObj),
	  m_CurVerbType(copy.m_CurVerbType),
	  m_CurValueType(copy.m_CurValueType),
	  m_bCurShowUnits(copy.m_bCurShowUnits),
	  m_UnitsStr(copy.m_UnitsStr)
{ }

SingleCritState::SingleCritState(const CriteriaObject &obj) // Init constructor
	: m_CurObj(obj)
{
	UpdateObj(m_CurObj);
}

SingleCritState::~SingleCritState()
{ }


SingleCritState &SingleCritState::operator=(const SingleCritState &copy)
{
	m_CurObj = copy.m_CurObj;
	m_CurVerbType = copy.m_CurVerbType;
	m_CurValueType = copy.m_CurValueType;
	m_bCurShowUnits = copy.m_bCurShowUnits;
	m_UnitsStr = copy.m_UnitsStr;

	return (*this);
}

bool SingleCritState::UpdateObj(const CriteriaObject &obj)
{
	m_CurObj = obj;

	if (!SearchCriteria::GetVerbValPair(m_CurObj, m_CurVerbType, m_CurValueType))
		return (false);

	if (CRITERIA_OBJECT_SIZE == m_CurObj)
	{
		m_bCurShowUnits = true;
		m_UnitsStr = CRString(IDS_SEARCH_SIZE_UNITS); // "K"
	}
	else if (CRITERIA_OBJECT_AGE == m_CurObj)
	{
		m_bCurShowUnits = true;
		m_UnitsStr = CRString(IDS_SEARCH_DATE_UNITS); // "Days"
	}
	else
	{
		m_bCurShowUnits = false;
		m_UnitsStr.Empty();
	}

	return (true);
}



