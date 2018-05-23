// SpellEditLexDialog.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "SpellDialog.h"

#include "SpellEditLexDialog.h"
#include "rs.h"
#include "guiutils.h"
#include "helpcntx.h"

// Include the files in the correct order to allow leak checking with malloc.
// CRTDBG_MAP_ALLOC already defined in stdafx.h
#include <stdlib.h>
#include <crtdbg.h>

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellEditLexDialog dialog

CSpellEditLexDialog::CSpellEditLexDialog(
CSpellDialog*	pParent, 
short			sEditLexID )
	: CDialog(CSpellEditLexDialog::IDD, pParent)
{
	m_pParent = pParent;
	m_sEditLexID = sEditLexID;

	//{{AFX_DATA_INIT(CSpellEditLexDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSpellEditLexDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellEditLexDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


// @PRIVATE_FUNCTION: copyLexToLb
// The copyLexToLb function updates a list box with the contents
// of a lexicon.
// @END
//

void CSpellEditLexDialog::copyLexToLb()
{
	S32			bfrSz;
	CWaitCursor theCursor;
	CListBox*	pLB;

	VERIFY( pLB = ( CListBox* ) GetDlgItem( IDC_LEX_LB ) );
	pLB->ResetContent();

	// Allocate a buffer large enough to hold the lexicon's contents.
	if (m_pParent->my_SpellCheck_GetLexInfo(m_sEditLexID, &bfrSz, NULL, NULL, NULL) >= 0)
	{
		ASSERT( bfrSz < 32767 );							// 16-bit limit
		SpellCheck_CHAR* bfr = (SpellCheck_CHAR*)malloc((size_t)bfrSz);	// 16-bit limit

		if (bfr != NULL)
		{
			const SpellCheck_CHAR* p;

			m_pParent->my_SpellCheck_GetLex(m_sEditLexID, bfr, bfrSz);
			for (p = bfr; *p != 0; p += strlen((char*)p) + 1)
			{
				pLB->AddString( ( char* ) p );
			}

			free (bfr);
		}
	}
}



BEGIN_MESSAGE_MAP(CSpellEditLexDialog, CDialog)
	//{{AFX_MSG_MAP(CSpellEditLexDialog)
	ON_BN_CLICKED(IDC_LEX_CLEAR_BTN, OnLexClearBtn)
	ON_BN_CLICKED(IDC_LEX_ADD_BTN, OnLexAddBtn)
	ON_BN_CLICKED(IDC_LEX_DEL_BTN, OnLexDelBtn)
	ON_LBN_SELCHANGE(IDC_LEX_LB, OnSelchangeLexLb)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellEditLexDialog message handlers



BOOL CSpellEditLexDialog::OnInitDialog() 
{
	S16			lexType;
	
	CDialog::OnInitDialog();
	

	m_pParent->my_SpellCheck_GetLexInfo( m_sEditLexID, NULL, NULL, &lexType, NULL);

	
	// Place a copy of the lexicon's contents into the dialog
	// list box.
	copyLexToLb();

	// Set the dialog's caption to the type of lexicon being
	// edited.
	switch (lexType)
	{
		case SpellCheck_CHANGE_LEX_TYPE:
			SetWindowText((LPCTSTR)CRString(IDS_SPELL_EDIT_CHANGE_TITLE));
			break;
		case SpellCheck_IGNORE_LEX_TYPE:
			SetWindowText((LPCTSTR)CRString(IDS_SPELL_EDIT_USER_TITLE));
			break;
		case SpellCheck_SUGGEST_LEX_TYPE:
			SetWindowText((LPCTSTR)CRString(IDS_SPELL_EDIT_SUGGEST_TITLE));
			break; 
		default:
			ASSERT(FALSE);
			break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CSpellEditLexDialog::OnLexClearBtn() 
{
	// Delete all words from the lexicon.
	if (MessageBox((LPCTSTR)CRString(IDS_SPELL_CONFIRM_DELETE), (LPCTSTR)CRString(IDS_EUDORA),
		MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		m_pParent->my_SpellCheck_ClearLex( m_sEditLexID);
		copyLexToLb();
	}
}


void CSpellEditLexDialog::OnLexAddBtn() 
{
	CEdit*				pWordEdit;
	SpellCheck_CHAR			szWord[SpellCheck_MAX_WORD_SZ * 2];
	const SpellCheck_CHAR*	szRepWord;
	S16					lexType;
	int					r;

	m_pParent->my_SpellCheck_GetLexInfo(m_sEditLexID, NULL, NULL, &lexType, NULL);
	
	VERIFY( pWordEdit = ( CEdit* ) GetDlgItem(IDC_LEX_WORD_EDIT) );
	szRepWord = 0;
	szWord[0] = 0;

	// Add a new word to the lexicon. Note that the "word" must be
	// a word:replacement pair if the lexicon is change-type or
	// suggest-type.
	pWordEdit->GetWindowText((char*)szWord, sizeof(szWord));

	// 
	// Adding words with leading whitespace confuses the dictionary,
	// so let's trim any leading whitespace before continuing.
	//
	{
		CString trim_word = (const char *) szWord;
		trim_word.TrimLeft();
		strcpy((char *) szWord, trim_word);
	}

	if (strlen((char *) szWord) <= 0)
		return;	// can't add empty strings

	if (lexType == SpellCheck_CHANGE_LEX_TYPE || lexType == SpellCheck_SUGGEST_LEX_TYPE)
	{
		SpellCheck_CHAR* p = (SpellCheck_CHAR*)strchr((char*) szWord, ':');

		if (p == NULL)
		{
			ErrorDialog(IDS_SPELL_ERR_ADD_COLON);
			return;
        }
		*p = 0;
		szRepWord = p + 1;
    }

	r = m_pParent->my_SpellCheck_AddToLex(m_sEditLexID, szWord, szRepWord);
	
	if (r == SpellCheck_OUT_OF_MEMORY_ERR)
	{
		ErrorDialog(IDS_SPELL_ERR_LEX_FULL);
		return;
	}
	else if (r < 0)
	{
		ErrorDialog(IDS_SPELL_ERR_ADD);
		return;
	}

	copyLexToLb();
}

void CSpellEditLexDialog::OnLexDelBtn() 
{
	CEdit*				pWordEdit;
	SpellCheck_CHAR			szWord[SpellCheck_MAX_WORD_SZ * 2];

	VERIFY( pWordEdit = ( CEdit* ) GetDlgItem(IDC_LEX_WORD_EDIT) );
	pWordEdit->GetWindowText((char*)szWord, sizeof( szWord));

	m_pParent->my_SpellCheck_DelFromLex(m_sEditLexID, szWord);
	
	copyLexToLb();
}


void CSpellEditLexDialog::OnSelchangeLexLb() 
{
	CEdit*			pWordEdit;
	CListBox*		pLexLB;
	int				n;
	char			szSelWord[SpellCheck_MAX_WORD_SZ * 2];
	char*			p;

	VERIFY( pLexLB = ( CListBox* ) GetDlgItem(IDC_LEX_LB) );	
	VERIFY( pWordEdit = ( CEdit* ) GetDlgItem(IDC_LEX_WORD_EDIT) );

	// The selected item in the list box has
	// changed. Copy the new selection to the Word
	// field.
	n = (int) pLexLB->GetCurSel();
	
	if( n >= 0 )
	{
		pLexLB->GetText(n, szSelWord );

		// Convert word:replacement to word
		p = strchr( szSelWord, ':');
		if (p != NULL)
			*p = 0;
		
		pWordEdit->SetWindowText( szSelWord );
	}
}


long CSpellEditLexDialog::OnHelp(WPARAM /*wParam*/, LPARAM lParam)
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

LRESULT CSpellEditLexDialog::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	CContextHelp((HWND)wParam);

	return TRUE;
}

