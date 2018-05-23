// PropertyPageRead.cpp : implementation file
//

#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>

#include "resource.h"

#include "eudora.h"
#include "rs.h"
#include "doc.h"
#include "cursor.h"
#include "fileutil.h"
#include "summary.h"
#include "msgdoc.h"
#include "readmsgd.h"
#include "persona.h"
#include "pop.h"
#include "PropertyPageRead.h"
#include "utils.h"
#include "colorlis.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CPropertyPageRead1, CPropertyPage)
IMPLEMENT_DYNCREATE(CPropertyPageRead2, CPropertyPage)

/*/////////////////////////////////////////////////////////////////////////////
// CLabelButton

CLabelButton::CLabelButton()
{
	m_Label = 0;
}

CLabelButton::~CLabelButton()
{
}

void CLabelButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CRect rect(lpDIS->rcItem);
	
	// Draw focus rect
	if ((lpDIS->itemAction & ODA_FOCUS) == ODA_FOCUS)
	{
		pDC->DrawFocusRect(&rect);
		return;
	}
	
	// The control doesn't change its look for selection
	if (!(lpDIS->itemAction & ODA_DRAWENTIRE))
		return;

	// Background color is the selected color with a black frame
	CBrush Brush(m_Label?
		GetIniLong(IDS_INI_LABEL_COLOR_1 + m_Label - 1) : GetSysColor(COLOR_WINDOW));
	pDC->SelectObject(&Brush);
	pDC->SelectStockObject(BLACK_PEN);
	pDC->Rectangle(&rect);
	pDC->SelectStockObject(BLACK_BRUSH);
	
	// Text color is window background color
	if (m_Label)
	{
		const char* Text = GetIniString(IDS_INI_LABEL_TEXT_1 + m_Label - 1);
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(GetSysColor(COLOR_WINDOW));
		pDC->DrawText(Text, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
	}

	// Draw focus rect
	if (lpDIS->itemState & ODS_FOCUS)
		pDC->DrawFocusRect(&rect);
}

BEGIN_MESSAGE_MAP(CLabelButton, CButton)
	//{{AFX_MSG_MAP(CLabelButton)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLabelButton message handlers
*/
/////////////////////////////////////////////////////////////////////////////
// CPropertyPageRead1 property page

CPropertyPageRead1::CPropertyPageRead1() : CPropertyPage(CPropertyPageRead1::IDD)
{
	//{{AFX_DATA_INIT(CPropertyPageRead1)
	m_sMailbox = _T("");
	m_iPriority = -1;
	m_iPersonality = -1;
	m_iStatus = -1;
	m_sSubject = _T("");
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
}

CPropertyPageRead1::~CPropertyPageRead1()
{
	m_pDoc = NULL;
}

void CPropertyPageRead1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertyPageRead1)
	DDX_Control(pDX, IDC_PROP_LABEL, m_LabelButton);
	DDX_Control(pDX, IDC_PROP_PERSON, m_cbPersonality);
	DDX_Text(pDX, IDC_PROP_MAILBOX, m_sMailbox);
	DDX_CBIndex(pDX, IDC_PROP_PRIORITY, m_iPriority);
	DDX_CBIndex(pDX, IDC_PROP_PERSON, m_iPersonality);
	DDX_CBIndex(pDX, IDC_PROP_STATUS, m_iStatus);
	DDX_Text(pDX, IDC_PROP_SUBJECT, m_sSubject);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertyPageRead1, CPropertyPage)
	//{{AFX_MSG_MAP(CPropertyPageRead1)
	ON_CBN_SELCHANGE(IDC_PROP_PERSON, OnSelchangePersonality)
	ON_CBN_SELCHANGE(IDC_PROP_STATUS, OnSelchangeStatus)
	ON_CBN_SELCHANGE(IDC_PROP_PRIORITY, OnSelchangePriority)
	ON_EN_CHANGE(IDC_PROP_SUBJECT, OnChangeSubject)
	ON_BN_CLICKED(IDC_PROP_LABEL, OnLabel)
	// Owner draw stuff for label menus
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP

	// Label changing messages
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_NONE, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_1, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_2, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_3, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_4, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_5, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_6, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_7, OnUpdateMessageLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_NONE, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_1, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_2, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_3, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_4, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_5, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_6, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_7, SetLabel)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropertyPageRead2 property page

CPropertyPageRead2::CPropertyPageRead2() : CPropertyPage(CPropertyPageRead2::IDD)
{
	//{{AFX_DATA_INIT(CPropertyPageRead2)
	m_iServerStatus = -1;
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
}

CPropertyPageRead2::~CPropertyPageRead2()
{
	m_pDoc = NULL;
}

void CPropertyPageRead2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertyPageRead2)
	DDX_Control(pDX, IDC_PROP_LOCK, m_LockButton);
	DDX_Control(pDX, IDC_PROP_EDIT, m_EditButton);
	DDX_Control(pDX, IDC_PROP_BLAH, m_BlahButton);
	DDX_Control(pDX, IDC_PROP_DEL_SERVER, m_DelServerButton);
	DDX_Radio(pDX, IDC_PROP_FETCH_DEL_SERVER, m_iServerStatus);
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertyPageRead2, CPropertyPage)
	//{{AFX_MSG_MAP(CPropertyPageRead2)
	ON_BN_CLICKED(IDC_PROP_LOCK, OnLock)
	ON_BN_CLICKED(IDC_PROP_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_PROP_BLAH, OnBlah)
	ON_BN_CLICKED(IDC_PROP_DEL_SERVER, OnDelServer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageRead1 message handlers

BOOL CPropertyPageRead1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_cbPersonality.ResetContent();
	LPSTR lpPersonalities = g_Personalities.List();
	CString Persona = "";								//Still always start with the Default personality

	do
	{
		Persona = lpPersonalities;
		if (!Persona.IsEmpty())
		{
			m_cbPersonality.AddString(Persona);
			// advance to next personality
			lpPersonalities += strlen( lpPersonalities ) + 1;
		}

	} while (!Persona.IsEmpty());

	
	if (m_pDoc)
	{
		int sel = ((CReadMessageDoc*)m_pDoc)->m_Sum->m_Label;
		m_LabelButton.m_Label = ( sel>=0 ? sel: 0);
		m_LabelButton.Invalidate();

		m_sMailbox = _T("");
		// deduct 1 from the provided priority to use the 0 based combobox.
		m_iPriority = ((CReadMessageDoc*)m_pDoc)->m_Sum->m_Priority - 1;
		m_iPersonality = m_cbPersonality.SelectString(0,
						((CReadMessageDoc*)m_pDoc)->m_Sum->GetPersona());
		if (m_iPersonality < 0)
			m_iPersonality = 0;;

		m_iStatus = ((CReadMessageDoc*)m_pDoc)->m_Sum->m_State;
		m_sSubject = ((CReadMessageDoc*)m_pDoc)->m_Sum->m_Subject;
	}
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPropertyPageRead1::OnSelchangePersonality() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_sPersona.Empty();
	int nIndex = m_cbPersonality.GetCurSel();
	if (nIndex == CB_ERR)
		nIndex = 0;

	m_cbPersonality.GetLBText(nIndex, m_sPersona );
//	((CReadMessageDoc*)m_pDoc)->m_Sum->SetPersona(m_sPersona);
	((CPropertyPanel*)GetParent())->UpdateApply();
}
void CPropertyPageRead1::OnSelchangeStatus() 
{
	UpdateData(TRUE);
//	if (m_pDoc)
//		((CReadMessageDoc*)m_pDoc)->m_Sum->m_State = (short) m_iStatus;	
	((CPropertyPanel*)GetParent())->UpdateApply();
}

void CPropertyPageRead1::OnSelchangePriority() 
{
	UpdateData(TRUE);
//	if (m_pDoc)
//		((CReadMessageDoc*)m_pDoc)->m_Sum->m_Priority = (short) (m_iPriority + 1);
	((CPropertyPanel*)GetParent())->UpdateApply();

}

void CPropertyPageRead1::OnChangeSubject() 
{
	UpdateData(TRUE);
//	if (m_pDoc)
//		((CReadMessageDoc*)m_pDoc)->m_Sum->m_Subject = m_sSubject;
	((CPropertyPanel*)GetParent())->UpdateApply();

}

//extern HMENU g_ChangeHMENU;

void CPropertyPageRead1::OnUpdateMessageLabel(CCmdUI* pCmdUI)
{
//	pCmdUI->Enable(InLabelSelect);
	
	// Let the main frame window do the drawing of the menu items
//	pCmdUI->ContinueRouting();
	if (pCmdUI->m_pMenu)
	{
		UINT State = pCmdUI->m_pMenu->GetMenuState(pCmdUI->m_nIndex, MF_BYPOSITION) | MF_OWNERDRAW;
		VERIFY(pCmdUI->m_pMenu->ModifyMenu(pCmdUI->m_nIndex, MF_BYPOSITION | State, pCmdUI->m_nID));
	}
}

BOOL CPropertyPageRead1::SetLabel(UINT nID)
{
	m_LabelButton.m_Label = (int)(nID - ID_MESSAGE_LABEL_1 + 1);
	m_LabelButton.Invalidate(FALSE);

	return (TRUE);
}

void CPropertyPageRead1::OnLabel()
{
	CRect rect;
	CLabelMenu Menu;
	Menu.LoadMenu(IDR_CONTEXT_POPUPS);
	m_pLabelMenu = (CLabelMenu *)((Menu.GetSubMenu(3))->GetSubMenu(5));
	
	if(m_pLabelMenu != NULL)
	{
		m_LabelButton.GetWindowRect(&rect);
		m_pLabelMenu->TrackPopupMenu(0, rect.left, rect.bottom, this);
	}	

	UpdateData(TRUE);
//	if (m_pDoc)
//		((CReadMessageDoc*)m_pDoc)->m_Sum->m_Label = (short)m_LabelButton.m_Label;
	((CPropertyPanel*)GetParent())->UpdateApply();
}

void CPropertyPageRead1::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS)
{
	UINT MenuID = lpMIS->itemID;

	// Is This A Label
	if ((MenuID >= ID_MESSAGE_LABEL_1) && (MenuID <= ID_MESSAGE_LABEL_7))
		m_pLabelMenu->MeasureItem(lpMIS);		
}

void CPropertyPageRead1::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS)
{
	UINT MenuID = lpDIS->itemID & 0xFFFF;
	
	// Is This A Label
	if ((MenuID >= ID_MESSAGE_LABEL_1) && (MenuID <= ID_MESSAGE_LABEL_7))
		m_pLabelMenu->DrawItem(lpDIS);		
}	

void CPropertyPageRead1::Apply()
{
	if (m_pDoc && ((CReadMessageDoc*)m_pDoc)->m_Sum)
	{
		// Setting the summary State
		if (((CReadMessageDoc*)m_pDoc)->m_Sum->m_State != (short) m_iStatus)
			((CReadMessageDoc*)m_pDoc)->m_Sum->m_State = (short) m_iStatus;

		// Setting the summary Priority
		if (((CReadMessageDoc*)m_pDoc)->m_Sum->m_Priority != (short) (m_iPriority + 1))
			((CReadMessageDoc*)m_pDoc)->m_Sum->m_Priority != (short) (m_iPriority + 1);

		// Setting the summary Personality 
		((CReadMessageDoc*)m_pDoc)->m_Sum->SetPersona(m_sPersona);

		// Setting the summary Label 
		((CReadMessageDoc*)m_pDoc)->m_Sum->m_Label = (short)m_LabelButton.m_Label;

		// Setting the summary Subject 
		strcpy(((CReadMessageDoc*)m_pDoc)->m_Sum->m_Subject, m_sSubject);
	}
	else
		AfxMessageBox("Can not access document");

}

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageRead2 message handlers

BOOL CPropertyPageRead2::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	if (m_pDoc)
	{
//		m_LockButton.
//		m_EditButton
		m_BlahButton.SetCheck(((CReadMessageDoc*)m_pDoc)->m_Sum->ShowAllHeaders());

		m_iServerStatus = GetServerStatus();
		;
//		m_DelServerButton
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CPropertyPageRead2::GetServerStatus()
{
	int Index = -1;
	// Server status
//	if (GetIniShort(IDS_INI_MBOX_SHOW_SERVER_STATUS))
//	{
		//CMsgRecord* pMsgRecord = GetMsgByHash( ((CReadMessageDoc*)m_pDoc)->m_Sum );
		CLMOSRecord LMOSRecord((CReadMessageDoc*)m_pDoc)->m_Sum);
		CMsgRecord* pMsgRecord = LMOSRecord.GetMsgRecord();
		if ( pMsgRecord )
		{
			BOOL RetFlag = (pMsgRecord->GetRetrieveFlag() != LMOS_DONOT_RETRIEVE /*1*/ 
									&& pMsgRecord->GetSkippedFlag() == LMOS_SKIP_MESSAGE /*0*/);
			BOOL DelFlag = (pMsgRecord->GetDeleteFlag() != LMOS_DONOT_DELETE /*1*/);
			if (RetFlag)
				Index = 1;
			if (DelFlag)
				Index = 2;
		}
//	}
	return Index;
}

void CPropertyPageRead2::OnLock()
{
	UpdateData(TRUE);
	((CPropertyPanel*)GetParent())->UpdateApply();
}

void CPropertyPageRead2::OnEdit()
{
	UpdateData(TRUE);
	((CPropertyPanel*)GetParent())->UpdateApply();
}

void CPropertyPageRead2::OnBlah()
{
	UpdateData(TRUE);
	((CPropertyPanel*)GetParent())->UpdateApply();
}

void CPropertyPageRead2::OnDelServer()
{
	UpdateData(TRUE);
	((CPropertyPanel*)GetParent())->UpdateApply();
}

void CPropertyPageRead2::Apply()
{
	if (m_BlahButton)
		((CReadMessageDoc*)m_pDoc)->m_Sum->SetShowAllHeaders(m_BlahButton.GetCheck());
}
