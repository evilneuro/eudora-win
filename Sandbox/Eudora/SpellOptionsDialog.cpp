// SpellOptionsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "SpellOptionsDialog.h"
#include "helpcntx.h"

#define SpellCheck_DLL_ FALSE

#include "ssce.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellOptionsDialog dialog



CSpellOptionsDialog::CSpellOptionsDialog(CWnd* pParent, ULONG ulMask, BOOL bAlwaysSuggest )
	: CDialog(CSpellOptionsDialog::IDD, pParent)
{
	m_ulOptionsMask = ulMask;
	m_bAlwaysSuggest = bAlwaysSuggest;
	//{{AFX_DATA_INIT(CSpellOptionsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSpellOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellOptionsDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpellOptionsDialog, CDialog)
	//{{AFX_MSG_MAP(CSpellOptionsDialog)
	ON_BN_CLICKED(IDC_ALWAYS_SUGGEST_BTN, OnAlwaysSuggestBtn)
	ON_BN_CLICKED(IDC_IGNORE_CAPS_BTN, OnIgnoreCapsBtn)
	ON_BN_CLICKED(IDC_IGNORE_MIXED_CASE_BTN, OnIgnoreMixedCaseBtn)
	ON_BN_CLICKED(IDC_IGNORE_MIXED_DIGITS_BTN, OnIgnoreMixedDigitsBtn)
	ON_BN_CLICKED(IDC_REPORT_DOUBLED_WORDS_BTN, OnReportDoubledWordsBtn)
	ON_BN_CLICKED(IDS_INI_IGNORE_ALL_CAPS, OnIniIgnoreAllCaps)
	ON_BN_CLICKED(IDS_INI_SUGGEST_PHONETICS, OnIniSuggestPhonetics)
	ON_BN_CLICKED(IDS_INI_SUGGEST_SPLIT_WORDS, OnIniSuggestSplitWords)
	ON_BN_CLICKED(IDS_INI_SUGGEST_TYPOGRAPHIC, OnIniSuggestTypographic)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_REPORT_MIXED_CASE_BTN, OnReportMixedCaseBtn)
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellOptionsDialog message handlers

BOOL CSpellOptionsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CheckDlgButton( IDC_REPORT_MIXED_CASE_BTN,
	  (m_ulOptionsMask & SpellCheck_REPORT_MIXED_CASE_OPT) != 0);

	CheckDlgButton(	IDC_REPORT_DOUBLED_WORDS_BTN,
	  (m_ulOptionsMask & SpellCheck_REPORT_DOUBLED_WORD_OPT) != 0);

	CheckDlgButton(	IDC_IGNORE_CAPS_BTN,
	  (m_ulOptionsMask & SpellCheck_IGNORE_CAPPED_WORD_OPT) != 0);

	CheckDlgButton(	IDS_INI_IGNORE_ALL_CAPS,
	  (m_ulOptionsMask & SpellCheck_IGNORE_ALL_CAPS_WORD_OPT) != 0);

	CheckDlgButton(	IDC_IGNORE_MIXED_DIGITS_BTN,
	  (m_ulOptionsMask & SpellCheck_IGNORE_MIXED_DIGITS_OPT) != 0);

	CheckDlgButton(	IDC_IGNORE_MIXED_CASE_BTN,
	  (m_ulOptionsMask & SpellCheck_IGNORE_MIXED_CASE_OPT) != 0);

	CheckDlgButton(	IDS_INI_SUGGEST_SPLIT_WORDS,
	  (m_ulOptionsMask & SpellCheck_SUGGEST_SPLIT_WORDS_OPT) != 0);

	CheckDlgButton(	IDS_INI_SUGGEST_PHONETICS,
	  (m_ulOptionsMask & SpellCheck_SUGGEST_PHONETIC_OPT) != 0);

	CheckDlgButton(	IDS_INI_SUGGEST_TYPOGRAPHIC,
	  (m_ulOptionsMask & SpellCheck_SUGGEST_TYPOGRAPHICAL_OPT) != 0);

	CheckDlgButton(	IDC_ALWAYS_SUGGEST_BTN, m_bAlwaysSuggest );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpellOptionsDialog::OnAlwaysSuggestBtn() 
{
	m_bAlwaysSuggest = IsDlgButtonChecked(IDC_ALWAYS_SUGGEST_BTN);
}

void CSpellOptionsDialog::OnIgnoreCapsBtn() 
{
	if (IsDlgButtonChecked(IDC_IGNORE_CAPS_BTN))
		m_ulOptionsMask |= SpellCheck_IGNORE_CAPPED_WORD_OPT;
	else
		m_ulOptionsMask &= ~SpellCheck_IGNORE_CAPPED_WORD_OPT;
}

void CSpellOptionsDialog::OnIgnoreMixedCaseBtn() 
{
	if (IsDlgButtonChecked(IDC_IGNORE_MIXED_CASE_BTN))
		m_ulOptionsMask |= SpellCheck_IGNORE_MIXED_CASE_OPT;
	else
		m_ulOptionsMask &= ~SpellCheck_IGNORE_MIXED_CASE_OPT;			
}

void CSpellOptionsDialog::OnIgnoreMixedDigitsBtn() 
{
	if (IsDlgButtonChecked(IDC_IGNORE_MIXED_DIGITS_BTN))
		m_ulOptionsMask |= SpellCheck_IGNORE_MIXED_DIGITS_OPT;
	else
		m_ulOptionsMask &= ~SpellCheck_IGNORE_MIXED_DIGITS_OPT;
}

void CSpellOptionsDialog::OnReportDoubledWordsBtn() 
{
	if (IsDlgButtonChecked(IDC_REPORT_DOUBLED_WORDS_BTN))
		m_ulOptionsMask |= SpellCheck_REPORT_DOUBLED_WORD_OPT;
	else
		m_ulOptionsMask &= ~SpellCheck_REPORT_DOUBLED_WORD_OPT;		
}

void CSpellOptionsDialog::OnIniIgnoreAllCaps() 
{
	if (IsDlgButtonChecked(IDS_INI_IGNORE_ALL_CAPS))
		m_ulOptionsMask |= SpellCheck_IGNORE_ALL_CAPS_WORD_OPT;
	else
		m_ulOptionsMask &= ~SpellCheck_IGNORE_ALL_CAPS_WORD_OPT;
}

void CSpellOptionsDialog::OnIniSuggestPhonetics() 
{
	if (IsDlgButtonChecked( IDS_INI_SUGGEST_PHONETICS))
		m_ulOptionsMask |= SpellCheck_SUGGEST_PHONETIC_OPT;
	else
		m_ulOptionsMask &= ~SpellCheck_SUGGEST_PHONETIC_OPT;
}

void CSpellOptionsDialog::OnIniSuggestSplitWords() 
{
	if (IsDlgButtonChecked(IDS_INI_SUGGEST_SPLIT_WORDS))
		m_ulOptionsMask |= SpellCheck_SUGGEST_SPLIT_WORDS_OPT;
	else
		m_ulOptionsMask &= ~SpellCheck_SUGGEST_SPLIT_WORDS_OPT;
}

void CSpellOptionsDialog::OnIniSuggestTypographic() 
{
	if (IsDlgButtonChecked(IDS_INI_SUGGEST_TYPOGRAPHIC))
		m_ulOptionsMask |= SpellCheck_SUGGEST_TYPOGRAPHICAL_OPT;
	else
		m_ulOptionsMask &= ~SpellCheck_SUGGEST_TYPOGRAPHICAL_OPT;
}

void CSpellOptionsDialog::OnReportMixedCaseBtn() 
{
	if (IsDlgButtonChecked(IDC_REPORT_MIXED_CASE_BTN))
		m_ulOptionsMask |= SpellCheck_REPORT_MIXED_CASE_OPT;
	else
		m_ulOptionsMask &= ~SpellCheck_REPORT_MIXED_CASE_OPT;
}


long CSpellOptionsDialog::OnHelp(WPARAM /*wParam*/, LPARAM lParam)
{
	// Called when the user presses F1.

	// Ignore if user pressed Shift-F1
//	if ( !ShiftDown() )
//	{
		LPHELPINFO lphi;
		lphi = (LPHELPINFO)lParam;
		if ( lphi->iContextType == HELPINFO_WINDOW)
			CContextHelp((HWND)lphi->hItemHandle);
//	}

	return TRUE;	
}

void CSpellOptionsDialog::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	CContextHelp((HWND)wParam);
}

