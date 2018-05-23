// SpellDialog.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "QCUtils.h"
#include "SpellDialog.h"
#include "SpellOptionsDialog.h"
#include "SpellEditLexDialog.h"
#include "rs.h"
#include "spell.h"
#include "fileutil.h"
#include "helpcntx.h"
#include "guiutils.h"

#include "QCSharewareManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellDialog dialog


CSpellDialog::CSpellDialog( CSpell*	pSpell, CWnd* pParent )
	: CDialog(CSpellDialog::IDD, pParent)
{
	m_hSpellLib = NULL;
	m_sPermChangeLexId	= -1;
	m_sPermSuggestLexId	= -1;
	m_sTmpChangeLexId	= -1;
	m_sTmpSuggestLexId	= -1;
	m_sTmpIgnoreLexId	= -1;
	m_blkId = -1;
	m_sid = -1;
	m_bDeleteBtn = FALSE;  // Is replace button set to "Delete"? 
	m_sPermIgnoreLexId = -1;
	m_bUseSugTempForProbWord = FALSE;  
	m_bSuggestingFor = FALSE;
	m_bAUserChange = TRUE;
	m_uDefID = IDC_IGNORE; // Default pushbutton in Check Word Dialog
	m_nMainLexFiles	= 0;
	m_pSpell = pSpell;
	m_bAlwaysSuggest = TRUE;
	m_bFoundMisspelling = FALSE;
	m_rectWnd.left = -1;
	m_rectWnd.top = -1;
	m_rectWnd.bottom = -1;
	m_rectWnd.right = -1;
	m_bHasBeenInitialized = FALSE;
	m_bAutoCheck = FALSE;
	m_iError = LoadSpellLib();
	if (m_iError == 0)
		Init();

	//{{AFX_DATA_INIT(CSpellDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CSpellDialog::~CSpellDialog()
{
	closeSpellCheck();
	m_sid = -1;
	CloseSpellLib(); 
}

void CSpellDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


S16 CSpellDialog::my_SpellCheck_AddToLex(S16 lexId, const SpellCheck_CHAR FAR *word, const SpellCheck_CHAR FAR *otherWord)
{	return((m_pfnAddToLex)(m_sid, lexId, word,otherWord));}

S16 CSpellDialog::my_SpellCheck_CheckBlock(S16 blkId, U32 options, SpellCheck_CHAR FAR *errWord, S16 errWordSz, SpellCheck_CHAR FAR *repWord, S16 repWordSz)
{	return((m_pfnCheckBlock)(m_sid, blkId, options, errWord, errWordSz, repWord, repWordSz));}

S16 CSpellDialog::my_SpellCheck_CheckWord(U32 options, const SpellCheck_CHAR FAR *word, SpellCheck_CHAR FAR *replWord, S16 replWordSz)
{	return((m_pfnCheckWord)(m_sid, options, word, replWord, replWordSz));}

S16 CSpellDialog::my_SpellCheck_ClearLex(S16 lexId) 
{	return((m_pfnClearLex)(m_sid, lexId));}

S16 CSpellDialog::my_SpellCheck_CloseBlock(S16 blkId) 
{	return((m_pfnCloseBlock)(m_sid,  blkId) );}

S16 CSpellDialog::my_SpellCheck_CloseLex(S16 lexId)
{	return((m_pfnCloseLex)(m_sid,  lexId));}

S16 CSpellDialog::my_SpellCheck_CloseSession(S16 m_sid)
{	return((m_pfnCloseSession)(m_sid));}

S16 CSpellDialog::my_SpellCheck_CreateLex(const char FAR *fileName, S16 type, S16 lang)
{	return((m_pfnCreateLex)(m_sid, fileName, type, lang));}

S16 CSpellDialog::my_SpellCheck_DelBlockWord(S16 blkId)
{	return((m_pfnDelBlockWord)(m_sid, blkId));}

S16 CSpellDialog::my_SpellCheck_DelFromLex(S16 lexId, const SpellCheck_CHAR FAR *word)
{	return((m_pfnDelFromLex)(m_sid, lexId, word));}

S32 CSpellDialog::my_SpellCheck_GetBlock(S16 blkId, SpellCheck_CHAR FAR *block, S32 blkSz)
{	return((m_pfnGetBlock)( m_sid,  blkId, block, blkSz));}

S16 CSpellDialog::my_SpellCheck_GetBlockInfo(S16 blkId, S32 FAR *blkLen, S32 FAR *blkSz, S32 FAR *curPos, S32 FAR *wordCount)
{	return((m_pfnGetBlockInfo)(m_sid,  blkId, blkLen, blkSz, curPos, wordCount));}

S16 CSpellDialog::my_SpellCheck_GetBlockWord(S16 blkId, SpellCheck_CHAR FAR *word, S16 wordSz)
{	return((m_pfnGetBlockWord)(m_sid,  blkId,  word, wordSz));}

S32 CSpellDialog::my_SpellCheck_GetLex(S16 lexId, SpellCheck_CHAR FAR *lexBfr, S32 lexBfrSz)
{	return((m_pfnGetLex)(m_sid, lexId, lexBfr, lexBfrSz));}

S16 CSpellDialog::my_SpellCheck_GetLexInfo(S16 lexId, S32 FAR *size, S16 FAR *format, S16 FAR *type, S16 FAR *lang)
{	return((m_pfnGetLexInfo)(m_sid, lexId, size,format,type, lang));}

U32 CSpellDialog::my_SpellCheck_GetOption(U32 option)
{	return((m_pfnGetOption)(m_sid, option));}

S16 CSpellDialog::my_SpellCheck_NextBlockWord(S16 blkId)
{	return((m_pfnNextBlockWord)( m_sid,  blkId));}

S16 CSpellDialog::my_SpellCheck_OpenBlock(SpellCheck_CHAR FAR *block, S32 blkLen, S32 blkSz, S16 copyBlock)
{	return((m_pfnOpenBlock)(m_sid, block,  blkLen, blkSz, copyBlock));}

S16 CSpellDialog::my_SpellCheck_OpenLex(const char FAR *fileName,S32 memBudget)
{	return((m_pfnOpenLex)(m_sid, fileName, memBudget));}

S16 CSpellDialog::my_SpellCheck_OpenSession(void)
{	return((m_pfnOpenSession)());}

S16 CSpellDialog::my_SpellCheck_ReplaceBlockWord(S16 blkId, const SpellCheck_CHAR FAR *word)
{	return((m_pfnReplaceBlockWord)(m_sid,blkId,word));}

U32 CSpellDialog::my_SpellCheck_SetOption(U32 option, U32 value)
{	return((m_pfnSetOption)(m_sid, option, value));}

S16 CSpellDialog::my_SpellCheck_Suggest(const SpellCheck_CHAR FAR *word, S16 depth, SpellCheck_CHAR FAR *suggBfr, S32 suggBfrSz, S16 FAR *scores, S16 scoreSz)
{	return((m_pfnSuggest)(m_sid, word, depth, suggBfr, suggBfrSz, scores, scoreSz));}

void CSpellDialog::my_SpellCheck_Version(S16 FAR *major, S16 FAR *minor)
 {	(m_pfnVersion)(major,minor);}


// @PRIVATE_FUNCTION: saveOptionMask
// Save the option settings indicated by an SpellCheck option mask to the
// profile file.
// @END
//
void CSpellDialog::saveOptionMask(unsigned long mask)
{
	SetIniShort(IDS_INI_IGNORE_CAPPED_WORDS,
		(short) ( (mask & SpellCheck_IGNORE_CAPPED_WORD_OPT)? 1 : 0) );
	SetIniShort(IDS_INI_IGNORE_ALL_CAPS,
		(short) ( (mask & SpellCheck_IGNORE_ALL_CAPS_WORD_OPT)? 1 : 0) );
	SetIniShort(IDS_INI_IGNORE_MIXED_DIGITS,
		(short) ( (mask & SpellCheck_IGNORE_MIXED_DIGITS_OPT)? 1 : 0) );
	SetIniShort(IDS_INI_IGNORE_MIXED_CASE,
		(short) ( (mask & SpellCheck_IGNORE_MIXED_CASE_OPT)? 1 : 0) );
	SetIniShort(IDS_INI_REPORT_DOUBLED_WORDS,
		(short) ( (mask & SpellCheck_REPORT_DOUBLED_WORD_OPT)? 1 : 0) );
	SetIniShort(IDS_INI_SUGGEST_SPLIT_WORDS,
		(short) ( (mask & SpellCheck_SUGGEST_SPLIT_WORDS_OPT)? 1 : 0) );
	SetIniShort(IDS_INI_SUGGEST_PHONETICS,
		(short) ( (mask & SpellCheck_SUGGEST_PHONETIC_OPT)? 1 : 0) );
	SetIniShort(IDS_INI_SUGGEST_TYPOGRAPHIC,
		(short) ( (mask & SpellCheck_SUGGEST_TYPOGRAPHICAL_OPT)? 1 : 0) );
	
}

void CSpellDialog::parseFileList(const char* fileList, char fileNames[][MAX_FILE_NAME],
	size_t maxFileNames, UINT* nFileNames)
{
	const char* c = fileList;

	*nFileNames = 0;
	while (*c != 0 && size_t(*nFileNames) < maxFileNames)
	{
		char* dst = fileNames[*nFileNames];
		size_t n = 0;

		while (*c != '\0' && *c != ',' && n < sizeof(fileNames[*nFileNames]))
		{
			*dst++ = *c++;
			++n;
		}
		*dst = 0;
		if (*c == ',')
			++c;
		*nFileNames += 1;
	}
}


// @PRIVATE_FUNCTION: getSuggestions
// The getSuggestions function obtains suggested alternatives for a
// misspelled word and displays them in the suggestions list box.
// @END
//
INT CSpellDialog::getSuggestions(const SpellCheck_CHAR* word )
{
	SpellCheck_CHAR suggBfr[SpellCheck_MAX_WORD_SZ * MAX_SUGGESTIONS + 1];
	static S16 scores[MAX_SUGGESTIONS];
	int n = 0;
	CWaitCursor wait;
	
	m_pWndSuggestList->SendMessage( LB_RESETCONTENT, 0, 0L);
	
	if (my_SpellCheck_Suggest(word, m_sSearchDepth, suggBfr, sizeof(suggBfr), scores,
		MAX_SUGGESTIONS) >= 0)
	{
		const SpellCheck_CHAR* p;
		int topScore = scores[0];

		// Display the best words in the list box.
		for (p = suggBfr; *p != 0; p += strlen((char*)p) + 1)
		{
			if (scores[n] < MIN_ALT_WORD_SCORE ||
				topScore - scores[n] > ALT_WORD_SCORE_RANGE)
			{
				break;
			}
			m_pWndSuggestList->SendMessage( LB_ADDSTRING, 0, (LPARAM)p);
			++n;
		}
	}
	
	return (n);
}


void CSpellDialog::RunChecker()
{
	// Shareware: In reduced feature mode, no spell check
	if (!UsingFullFeatureSet())
		return;

	// FULL-FEATURE
	
	SpellCheck_CHAR	szProbWord[SpellCheck_MAX_WORD_SZ];
	SpellCheck_CHAR	szRepWord[SpellCheck_MAX_WORD_SZ];
	INT			iResult;
	INT			n;
	char		szFirstWord[SpellCheck_MAX_WORD_SZ];


	// Disable all the button controls. They will be re-enabled as
	// necessary.
	m_pWndIgnoreBtn->EnableWindow(		FALSE);
	m_pWndIgnoreAllBtn->EnableWindow(FALSE);
	m_pWndReplaceBtn->EnableWindow(FALSE);
	m_pWndReplaceAllBtn->EnableWindow(FALSE);
	m_pWndAddToLexBtn->EnableWindow(FALSE);
	m_pWndSuggestBtn->EnableWindow(FALSE);
//	m_pWndDeleteBtn->EnableWindow(FALSE);
//	m_pWndCancelBtn->EnableWindow(FALSE);
	
	// Reset all defaults 
	m_pWndReplaceBtn->SetWindowText((LPCTSTR)CRString(IDS_SPELL_CHANGE));
	m_pWndReplaceAllBtn->SetWindowText((LPCTSTR)CRString(IDS_SPELL_CHANGE_ALL));

	m_pWndSuggestList->SendMessage(LB_RESETCONTENT, 0, 0L);
	
	if ( m_uDefID != IDC_IGNORE )
	{
       	SendMessage( DM_SETDEFID, IDC_IGNORE, 0 );
		m_uDefID = IDC_IGNORE;
	}
	
	m_bAUserChange = FALSE;
	
	m_pWndRepWordEdit->SetWindowText("");
	
	iResult = m_pSpell->my_CheckBlock( m_ulOptionsMask, szProbWord, szRepWord );
	
	if( iResult & SpellCheck_END_OF_BLOCK_RSLT )
		EndDialog( IDOK );

	if( iResult & SpellCheck_MISSPELLED_WORD_RSLT )
	{ 
		m_bSuggestingFor = FALSE; 
		m_bUseSugTempForProbWord = FALSE;
		m_pSpell->myHilightWord();		// highlight the misspelled word  *****
		m_bAUserChange = FALSE;
		m_pWndProbLblTxt->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_UNKNOWN) );
		m_pWndProbWordTxt->SetWindowText( (char *)szProbWord);
		m_pWndRepWordEdit->SetWindowText( (char *)szRepWord);
		m_pWndIgnoreBtn->EnableWindow( TRUE);
		m_pWndIgnoreAllBtn->EnableWindow( TRUE);
		
		if (m_sPermIgnoreLexId >= 0)
			m_pWndAddToLexBtn->EnableWindow( TRUE);

		if (m_bAlwaysSuggest)
		{
			n = getSuggestions(szProbWord);

			if (n > 0)
			{
				// Display the first suggestion in the Replacement field.
				m_pWndSuggestList->SendMessage(LB_GETTEXT, 0, (LPARAM) szFirstWord);
				m_bAUserChange = FALSE;
				m_pWndRepWordEdit->SetWindowText( szFirstWord);
			}
		}
		else
		{
			// Let the user decide when to locate suggestions.
			m_sSearchDepth = 1;
			m_pWndSuggestBtn->EnableWindow( TRUE );
		}

		m_pWndReplaceBtn->EnableWindow(TRUE);
		m_pWndReplaceAllBtn->EnableWindow(TRUE);
//		m_pWndCancelBtn->EnableWindow(TRUE);
	}
	else if (iResult & SpellCheck_SUGGEST_WORD_RSLT)
	{
		m_pSpell->myHilightWord();
		m_bAUserChange = FALSE;
		m_pWndProbLblTxt->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_CONSIDER_CHANGING));
		m_pWndProbWordTxt->SetWindowText( (char *)szProbWord);
		m_pWndRepWordEdit->SetWindowText( (char *)szRepWord);
		m_pWndIgnoreBtn->EnableWindow( TRUE);

		if (m_bAlwaysSuggest)
		{
			// Note that we don't copy the first suggestion to the
			// replacement field, because the szRepWord parameter
			// returned by SpellCheck_CheckBlock is the best suggestion.
			getSuggestions(szProbWord);
		}
		else
		{
			// Let the user decide when to locate suggestions.
			m_sSearchDepth = 1;
			m_pWndSuggestBtn->EnableWindow( TRUE);
		}
		m_pWndReplaceBtn->EnableWindow( TRUE);
		m_pWndReplaceAllBtn->EnableWindow( TRUE);
//		m_pWndCancelBtn->EnableWindow( TRUE);
	}
	else if (iResult & SpellCheck_UNCAPPED_WORD_RSLT)
	{
		m_pSpell->myHilightWord();
		m_pWndProbLblTxt->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_CAPITALIZATION));
		m_pWndProbWordTxt->SetWindowText( (char *)szProbWord);
		m_pWndIgnoreBtn->EnableWindow( TRUE);
		m_bAUserChange = FALSE;
		m_pWndRepWordEdit->SetWindowText( (char *)szRepWord);
		m_pWndReplaceBtn->EnableWindow( TRUE);
//		m_pWndCancelBtn->EnableWindow( TRUE);
	}
	else if (iResult & SpellCheck_MIXED_CASE_WORD_RSLT)
	{
		m_pSpell->myHilightWord();
		m_pWndProbLblTxt->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_MIXED_CASE));
		m_pWndProbWordTxt->SetWindowText( (char *)szProbWord);
		m_pWndIgnoreBtn->EnableWindow( TRUE);
		m_bAUserChange = FALSE;
		m_pWndRepWordEdit->SetWindowText( (char *)szRepWord);
		m_pWndReplaceBtn->EnableWindow( TRUE);
//		m_pWndCancelBtn->EnableWindow( TRUE);
	}
	else if (iResult & SpellCheck_DOUBLED_WORD_RSLT)
	{
		m_pSpell->myHilightWord();
		m_pWndProbLblTxt->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_DOUBLED_WORD));
		m_pWndProbWordTxt->SetWindowText( (char *)szProbWord);
		m_pWndIgnoreBtn->EnableWindow( TRUE);
		m_pWndReplaceBtn->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_DELETE));
		m_pWndReplaceAllBtn->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_DELETE_ALL));
		m_bDeleteBtn = TRUE;
		m_pWndReplaceBtn->EnableWindow(		TRUE); 
		m_pWndReplaceAllBtn->EnableWindow(	FALSE);
//		m_pWndCancelBtn->EnableWindow(		TRUE);
	}
	else if (iResult & SpellCheck_CHANGE_WORD_RSLT)
	{
		// Automatically replace the current word with the indicated
		// replacement.
		//SpellCheck_ReplaceBlockWord(sid, blkId, szRepWord);    
		m_pSpell->my_ReplaceBlockWord(szProbWord, szRepWord);
	}

	//When a misspelt word is highlighted in PaigeEdtView, Paige steals the focus and 
	//hence the caret. To get the caret back, we set the focus back to the dialog and 
	//then set it to the "Change To"  Edit control.
	SetFocus();
	m_pWndRepWordEdit->SetFocus();
	m_pWndRepWordEdit->ShowCaret();

//	SendMessage(WM_NEXTDLGCTL,(WPARAM)( m_pWndRepWordEdit->GetSafeHwnd() ),MAKELPARAM(1,0));
}


int CSpellDialog::LoadSpellLib()
{
	char buf[64];
    CRString whichDLL(IDS_SPELL_DLL32);
	
	if (!m_hSpellLib)
    {
       	if ((m_hSpellLib = LoadLibrary ((const char *)whichDLL)) < (HANDLE)32)
		{
			ErrorDialog(IDS_SPELL_BASE_ERROR + 17, (const char *)whichDLL);
			m_hSpellLib = NULL;
			return -1;
		}
		else
        {
#define SET_PA(fnName, fnNumber) \
	*((FARPROC*)&fnName) = GetProcAddress(m_hSpellLib, (LPSTR)GetIniString(fnNumber, buf, sizeof(buf)))
		   // Configure all the function handles
            SET_PA(m_pfnAddToLex,	IDS_SPELL_DLL_ADD_LEX);
            SET_PA(m_pfnCheckBlock,	IDS_SPELL_DLL_CHK_BLK);
            SET_PA(m_pfnCheckWord,	IDS_SPELL_DLL_CHK_WRD);
            SET_PA(m_pfnClearLex,	IDS_SPELL_DLL_CLR_LEX);
            SET_PA(m_pfnCloseBlock,	IDS_SPELL_DLL_CLS_BLK);
            SET_PA(m_pfnCloseLex,	IDS_SPELL_DLL_CLS_LEX);
            SET_PA(m_pfnCloseSession,IDS_SPELL_DLL_CLS_SES);
            SET_PA(m_pfnCreateLex,	IDS_SPELL_DLL_CRE_LEX);
            SET_PA(m_pfnDelBlockWord,IDS_SPELL_DLL_DEL_BLK_WRD);
            SET_PA(m_pfnDelFromLex,	IDS_SPELL_DLL_DEL_LEX);
            SET_PA(m_pfnGetBlock,	IDS_SPELL_DLL_GET_BLK);
            SET_PA(m_pfnGetBlockInfo,IDS_SPELL_DLL_GET_BLK_INFO);
            SET_PA(m_pfnGetBlockWord,IDS_SPELL_DLL_GET_BLK_WRD);
            SET_PA(m_pfnGetLex,		IDS_SPELL_DLL_GET_LEX);
            SET_PA(m_pfnGetLexInfo,	IDS_SPELL_DLL_GET_LEX_INFO);
            SET_PA(m_pfnGetOption,	IDS_SPELL_DLL_GET_OPTION);
            SET_PA(m_pfnNextBlockWord,IDS_SPELL_DLL_NXT_BLK_WRD);
            SET_PA(m_pfnOpenBlock,	IDS_SPELL_DLL_OPEN_BLK);
            SET_PA(m_pfnOpenLex,		IDS_SPELL_DLL_OPEN_LEX);
            SET_PA(m_pfnOpenSession,	IDS_SPELL_DLL_OPEN_SES);
            SET_PA(m_pfnReplaceBlockWord,IDS_SPELL_DLL_REP_BLK_WRD);
            SET_PA(m_pfnSetOption,	IDS_SPELL_DLL_SET_OPTION);
            SET_PA(m_pfnSuggest,		IDS_SPELL_DLL_SUGGEST);
            SET_PA(m_pfnVersion,		IDS_SPELL_DLL_VER);
		}
	}
	return (0);
}


int CSpellDialog::CloseSpellLib()
{
	if (m_hSpellLib) 
	{
		FreeLibrary(m_hSpellLib);
	}

	m_hSpellLib = NULL;
    m_pfnAddToLex = NULL;
    m_pfnCheckBlock = NULL;
    m_pfnCheckWord = NULL;
    m_pfnClearLex = NULL;
    m_pfnCloseBlock = NULL;
    m_pfnCloseLex = NULL;
    m_pfnCloseSession = NULL;
    m_pfnCreateLex = NULL;
    m_pfnDelBlockWord = NULL;
    m_pfnDelFromLex = NULL;
    m_pfnGetBlock = NULL;
    m_pfnGetBlockInfo = NULL;
    m_pfnGetBlockWord = NULL;
    m_pfnGetLex = NULL;					   
    m_pfnGetLexInfo = NULL;
    m_pfnGetOption = NULL;
    m_pfnNextBlockWord = NULL;
    m_pfnOpenBlock = NULL;
    m_pfnOpenLex = NULL;
    m_pfnOpenSession = NULL;
    m_pfnReplaceBlockWord = NULL;
    m_pfnSetOption = NULL;
    m_pfnSuggest = NULL;
	m_pfnVersion = NULL;
	return 0;
}


// @PRIVATE_FUNCTION: openSpellCheck
// The openSpellCheck function opens an SpellCheck session, including lexicons.
// @END
//
INT CSpellDialog::openSpellCheck()
{
	char pathName[_MAX_PATH];
	char userPathName[_MAX_PATH];
	int PathLen;
	int UserPathLen;
	int i;

	if (m_sid >= 0)
		return (0);
		
	strcpy(pathName, m_szLexPath);
	strcat(pathName, SLASHSTR);
	strcpy(userPathName, m_szUserLexPath);
	strcat(userPathName, SLASHSTR);
	PathLen = strlen(pathName);
	UserPathLen = strlen(userPathName);

	// Open an SpellCheck session.
	m_sid = my_SpellCheck_OpenSession();
 	if (m_sid < 0)
	{
//		wsprintf(str, "Error %d opening session", sid);
//		logDiagnostic(str);
		return (m_sid);
	}

	// Open the lexicons within the session.
	if ((m_sTmpChangeLexId = my_SpellCheck_CreateLex(NULL, SpellCheck_CHANGE_LEX_TYPE, m_byLanguage)) < 0)
	{
//		wsprintf(str, "Error %d opening temp change lexicon", tmpChangeLexId);
//		logDiagnostic(str);
		closeSpellCheck();
		m_sid = -1;
		return (m_sTmpChangeLexId);
	}
	if ((m_sTmpIgnoreLexId = my_SpellCheck_CreateLex(NULL, SpellCheck_IGNORE_LEX_TYPE, m_byLanguage)) < 0)
	{
//		wsprintf(str, "Error %d opening temp ignore lexicon", tmpIgnoreLexId);
//		logDiagnostic(str);
		closeSpellCheck();
		m_sid = -1;
		return (m_sTmpIgnoreLexId);
	}

	if ((m_sTmpSuggestLexId = my_SpellCheck_CreateLex(NULL, SpellCheck_SUGGEST_LEX_TYPE, m_byLanguage)) < 0)
	{
//		wsprintf(str, "Error %d opening temp suggest lexicon", tmpSuggestLexId);
//		logDiagnostic(str);
		closeSpellCheck();
		m_sid = -1;
		return (m_sTmpSuggestLexId);
	}

	// Open the permanent lexicons, creating them if necessary.
	strcpy(userPathName + UserPathLen, m_szPermChangeLexFileName);
	if ((m_sPermChangeLexId = my_SpellCheck_OpenLex(userPathName, 0L)) < 0)
	{
		if ((m_sPermChangeLexId =
			my_SpellCheck_CreateLex(userPathName, SpellCheck_CHANGE_LEX_TYPE, m_byLanguage)) < 0)
		{
//			wsprintf(str, "Error %d opening %s", pathName, m_sPermChangeLexId);
//			logDiagnostic(str);
			closeSpellCheck();
			m_sid = -1;
			return (m_sPermChangeLexId);
		}
	}

	strcpy(userPathName + UserPathLen, m_szPermIgnoreLexFileName);
	if ((m_sPermIgnoreLexId = my_SpellCheck_OpenLex(userPathName, 0L)) < 0)
	{
		if ((m_sPermIgnoreLexId =
			my_SpellCheck_CreateLex(userPathName, SpellCheck_IGNORE_LEX_TYPE, m_byLanguage)) < 0)
		{
//			wsprintf(str, "Error %d opening %s", pathName, m_sPermIgnoreLexId);
//			logDiagnostic(str);
			closeSpellCheck();
			m_sid = -1;
			return (m_sPermIgnoreLexId);
		}
	}

	strcpy(userPathName + UserPathLen, m_szPermSuggestLexFileName);
	if ((m_sPermSuggestLexId = my_SpellCheck_OpenLex(userPathName, 0L)) < 0)
	{
		if ((m_sPermSuggestLexId =
			my_SpellCheck_CreateLex(userPathName, SpellCheck_SUGGEST_LEX_TYPE, m_byLanguage)) < 0)
		{
//			wsprintf(str, "Error %d opening %s", pathName, m_sPermSuggestLexId);
//			logDiagnostic(str);
			closeSpellCheck();
			m_sid = -1;
			return (m_sPermSuggestLexId);
		}
	}

	// Open the main lexicon files.
	for (i = 0; i < (INT) m_nMainLexFiles; ++i)
	{
		strcpy(pathName + PathLen, m_arrMainLexFileNames[i]);
		if ((m_arrMainLexIds[i] = my_SpellCheck_OpenLex(pathName, 0L)) < 0)
		{
//			wsprintf(str, "Error %d opening %s", m_arrMainLexIds[i], pathName);
//			logDiagnostic(str);
			closeSpellCheck();
			m_sid = -1;
			return (m_arrMainLexIds[i]);
		}
	}

	return (0);
}

// @PRIVATE_FUNCTION: closeSpellCheck
// The closeSpellCheck function closes an SpellCheck session, including lexicons.
// @END
//
void CSpellDialog::closeSpellCheck()
{
	if (m_sid >= 0)
	{
		UINT u;

		for (u = 0; u < m_nMainLexFiles; ++u)
		{
			if (m_arrMainLexIds[u] >= 0)
				my_SpellCheck_CloseLex( m_arrMainLexIds[u]);
		}
		
		if (m_sPermSuggestLexId > 0)	my_SpellCheck_CloseLex(m_sPermSuggestLexId);
        if (m_sPermIgnoreLexId > 0)		my_SpellCheck_CloseLex(m_sPermIgnoreLexId);
        if (m_sPermChangeLexId > 0)		my_SpellCheck_CloseLex(m_sPermChangeLexId);
        if (m_sTmpSuggestLexId > 0)		my_SpellCheck_CloseLex(m_sTmpSuggestLexId);
        if (m_sTmpIgnoreLexId > 0)		my_SpellCheck_CloseLex(m_sTmpIgnoreLexId);
        if (m_sTmpChangeLexId > 0)		my_SpellCheck_CloseLex(m_sTmpChangeLexId);

		my_SpellCheck_CloseSession(m_sid);
    }
}


// set the options that CSpell::CheckBlock() doesn't support
void CSpellDialog::SetOptions()
{
	U32 prev;
	if ( m_ulOptionsMask & SpellCheck_SUGGEST_PHONETIC_OPT )
	{
		prev = my_SpellCheck_SetOption( SpellCheck_SUGGEST_PHONETIC_OPT, 1 );
	}
	else
	{
		prev = my_SpellCheck_SetOption( SpellCheck_SUGGEST_PHONETIC_OPT, 0 );
	}

	if ( m_ulOptionsMask & SpellCheck_SUGGEST_TYPOGRAPHICAL_OPT )
	{
		prev = my_SpellCheck_SetOption( SpellCheck_SUGGEST_TYPOGRAPHICAL_OPT, 1 );
	}
	else
	{
		prev = my_SpellCheck_SetOption( SpellCheck_SUGGEST_TYPOGRAPHICAL_OPT, 0 );
	}

	if ( m_ulOptionsMask & SpellCheck_SUGGEST_SPLIT_WORDS_OPT )
	{
		prev = my_SpellCheck_SetOption( SpellCheck_SUGGEST_SPLIT_WORDS_OPT, 1 );
	}
	else
	{
		prev = my_SpellCheck_SetOption( SpellCheck_SUGGEST_SPLIT_WORDS_OPT, 0 );
	}

}
BOOL CSpellDialog::Init() 
{
	INT		iRet;
	INT		iEnd;
	char	fileList[MAX_FILE_NAME * MAX_MAIN_LEX_FILES + 1];

	// Obtain lexicon names from the SpellCheck profile file.
	GetIniString(IDS_INI_LEX_PATH, m_szLexPath, sizeof(m_szLexPath));
	
	if (!*m_szLexPath)
		strcpy(m_szLexPath, ExecutableDir);
	
	iEnd = strlen(m_szLexPath) - 1;
	
	if( m_szLexPath[ iEnd ] == SLASH )
	{
		m_szLexPath[ iEnd ] = 0;
	}

	GetIniString(IDS_INI_USER_LEX_PATH, m_szUserLexPath, sizeof(m_szUserLexPath));
	
	if (!*m_szUserLexPath)
		strcpy(m_szUserLexPath, EudoraDir);
	
	iEnd = strlen(m_szUserLexPath) - 1;
	
	if (m_szUserLexPath[iEnd] == SLASH)
		m_szUserLexPath[iEnd] = 0;

	GetIniString(IDS_INI_MAIN_LEX_FILES, fileList, sizeof(fileList));

	parseFileList(fileList, m_arrMainLexFileNames, MAX_MAIN_LEX_FILES, &m_nMainLexFiles );

	GetIniString(IDS_INI_USER_CHANGE_LEX, m_szPermChangeLexFileName, sizeof(m_szPermChangeLexFileName));

	GetIniString(IDS_INI_USER_IGNORE_LEX, m_szPermIgnoreLexFileName, sizeof(m_szPermIgnoreLexFileName));

	GetIniString(IDS_INI_USER_SUGGEST_LEX, m_szPermSuggestLexFileName, sizeof( m_szPermSuggestLexFileName));

    // Obtain the SpellCheck Dialogs-specific profile settings.
    m_ulOptionsMask = SpellCheck_REPORT_SPELLING_OPT;
	if (GetIniShort(IDS_INI_IGNORE_CAPPED_WORDS))	m_ulOptionsMask |= SpellCheck_IGNORE_CAPPED_WORD_OPT;
	if (GetIniShort(IDS_INI_IGNORE_ALL_CAPS))		m_ulOptionsMask |= SpellCheck_IGNORE_ALL_CAPS_WORD_OPT;
	if (GetIniShort(IDS_INI_IGNORE_MIXED_DIGITS))	m_ulOptionsMask |= SpellCheck_IGNORE_MIXED_DIGITS_OPT;
	if (GetIniShort(IDS_INI_IGNORE_MIXED_CASE))		m_ulOptionsMask |= SpellCheck_IGNORE_MIXED_CASE_OPT;
	if (GetIniShort(IDS_INI_REPORT_DOUBLED_WORDS))	m_ulOptionsMask |= SpellCheck_REPORT_DOUBLED_WORD_OPT;
	if (GetIniShort(IDS_INI_SUGGEST_SPLIT_WORDS))	m_ulOptionsMask |= SpellCheck_SUGGEST_SPLIT_WORDS_OPT;
	if (GetIniShort(IDS_INI_SUGGEST_PHONETICS))		m_ulOptionsMask |= SpellCheck_SUGGEST_PHONETIC_OPT;
	if (GetIniShort(IDS_INI_SUGGEST_TYPOGRAPHIC))	m_ulOptionsMask |= SpellCheck_SUGGEST_TYPOGRAPHICAL_OPT;

	m_ulOptionsMask |= SpellCheck_SPLIT_CONTRACTED_WORDS_OPT;

	m_bAlwaysSuggest = ( BOOL ) GetIniShort(IDS_INI_ALWAYS_SUGGEST);
	m_byLanguage = (char) GetIniShort(IDS_INI_LEX_LANGUAGE);

	// Open an SpellCheck session 
	
	if ( ( iRet = openSpellCheck() ) < 0 )
	{
		m_iError = iRet;
		if (m_hWnd != NULL) 
		{
			// Since this method (Init) is called from the constructor, guard this call to 
			// avoid a spurious ASSERT in debug mode.  A more careful code review might lead
			// to eliminating this call all together, since the dialog wasn't really ever created.
			EndDialog( IDCANCEL );
		}
		return TRUE;
	}

	if( m_bAlwaysSuggest )
	{
		m_sSearchDepth = SpellCheck_AUTO_SEARCH_DEPTH;
	}
	else
	{
		m_sSearchDepth = 1;
	}

	// set the options that CheckBlock() doesn't support
	SetOptions();
  
	return TRUE;
}


BEGIN_MESSAGE_MAP(CSpellDialog, CDialog)
	//{{AFX_MSG_MAP(CSpellDialog)
	ON_WM_MOVE()
	ON_EN_CHANGE(IDC_REPLACEMENT_WORD, OnChangeReplacementWord)
	ON_BN_CLICKED(IDC_IGNORE, OnIgnore)
	ON_BN_CLICKED(IDC_IGNORE_ALL, OnIgnoreAll)
	ON_BN_CLICKED(IDC_REPLACEIT, OnReplaceIt)
	ON_BN_CLICKED(IDC_REPLACE_ALL, OnReplaceAll)
	ON_BN_CLICKED(IDC_ADD_TO_LEX, OnAddToLex)
	ON_BN_CLICKED(IDC_SUGGEST, OnSuggest)
	ON_LBN_DBLCLK(IDC_SUGGEST_LIST, OnDblclkSuggestList)
	ON_LBN_SELCHANGE(IDC_SUGGEST_LIST, OnSelchangeSuggestList)
	ON_BN_CLICKED(IDC_JUSTDOIT, OnJustDoIt)
	ON_BN_CLICKED(IDC_OPTIONS, OnOptions)
	ON_BN_CLICKED(IDC_EDIT_LEX, OnEditLex)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellDialog message handlers

void CSpellDialog::EndDialog(int nResult)
{
	CDialog::EndDialog(nResult);
	m_bHasBeenInitialized=FALSE;
}

BOOL CSpellDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();
    //
    // Lexicon was initialized in constructor
    //
    m_pWndIgnoreBtn             = GetDlgItem( IDC_IGNORE );
    m_pWndIgnoreAllBtn  = GetDlgItem( IDC_IGNORE_ALL );
    m_pWndReplaceBtn    = GetDlgItem( IDC_REPLACEIT );
    m_pWndReplaceAllBtn = GetDlgItem( IDC_REPLACE_ALL );
    m_pWndAddToLexBtn   = GetDlgItem( IDC_ADD_TO_LEX );
    m_pWndSuggestBtn    = GetDlgItem( IDC_SUGGEST );
//  m_pWndDeleteBtn         = GetDlgItem( IDC_DELETE );
//  m_pWndCancelBtn         = GetDlgItem( IDCANCEL );
    m_pWndJustDoIt      = GetDlgItem( IDC_JUSTDOIT );
    m_pWndSuggestList   = GetDlgItem( IDC_SUGGEST_LIST );
    m_pWndProbLblTxt    = GetDlgItem( IDC_PROBLEM_LBL );
    m_pWndProbWordTxt   = GetDlgItem( IDC_PROBLEM_WORD );
    m_pWndRepWordEdit   = GetDlgItem( IDC_REPLACEMENT_WORD );

    // Set the window's origin to the last remembered position (if
    // there is one). Once the user moves the dialog, it will stay
    // there.
    GetIniWindowPos( IDS_INI_SPELL_WIN_POS, m_rectWnd );

    if( ( m_rectWnd.left < m_rectWnd.right ) && ( m_rectWnd.top < m_rectWnd.bottom ) )
    {
        SetWindowPos(NULL, m_rectWnd.left, m_rectWnd.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

// Hack for Irwin
    if ( m_bAutoCheck ) {
        if ( !GetIniShort( IDS_INI_IMMEDIATE_SEND ) )
            m_pWndJustDoIt->SetWindowText( CRString( IDS_JUSTQUEUE ) );

        m_pWndJustDoIt->ModifyStyle( 0, WS_VISIBLE, SWP_NOSIZE | SWP_NOMOVE );
    }

    m_bHasBeenInitialized=TRUE;
    RunChecker();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}



void CSpellDialog::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	
	//
	// Make sure that the GetIni call in OnInitialUpdate
	// happens first.
	//
	if ( m_bHasBeenInitialized )
	{
		GetWindowRect(&m_rectWnd);
		SetIniWindowPos(IDS_INI_SPELL_WIN_POS, m_rectWnd);
	}
}


long CSpellDialog::OnHelp(WPARAM /*wParam*/, LPARAM lParam)
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

void CSpellDialog::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	CContextHelp((HWND)wParam);
}


void CSpellDialog::OnChangeReplacementWord() 
{
	SpellCheck_CHAR szRepWord[SpellCheck_MAX_WORD_SZ];
	
	// These may be used later.
	m_pWndRepWordEdit->GetWindowText( (char*)szRepWord, sizeof(szRepWord)); 

	// Has the user modified the field and or has a function call  
	// from within the program changed it?
	if (m_bAUserChange == TRUE)
	{
		// Has the user erased the suggested word? If yes, assume delete.
		if (strlen((char*)szRepWord) == 0)
		{
			m_bDeleteBtn = TRUE;
			m_pWndReplaceBtn->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_DELETE));
			m_pWndReplaceAllBtn->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_DELETE_ALL));
			m_pWndSuggestBtn->EnableWindow( FALSE);
		}
		else 
		{   
			// Is the Replace Button labeled as "Delete"?
			if ( m_bDeleteBtn)
			{
				m_pWndReplaceBtn->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_CHANGE));
				m_pWndReplaceAllBtn->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_CHANGE_ALL));
				m_bDeleteBtn = FALSE; 
			}   
			// If new text is entered in the field, the user may want to 
			// ask for suggestings based upon this new word.
			if ( !m_bSuggestingFor )
			{		
				m_sSearchDepth =1;
				m_pWndSuggestBtn->EnableWindow(TRUE);
				m_bSuggestingFor = TRUE;
			}   
			
		}
		
		// A user has made a change to the suggested word and the default
		// button has not been changed to Replace. 
		if (m_uDefID != IDC_REPLACEIT)
		{
			SendMessage(DM_SETDEFID,IDC_REPLACEIT,0);
			m_uDefID = IDC_REPLACEIT;
		}
//		runChecker = FALSE;
//		msgHandled = TRUE;
	}
	else
		m_bAUserChange = TRUE;
	
	m_pWndSuggestBtn->EnableWindow( TRUE);  
}


void CSpellDialog::OnIgnore() 
{
	RunChecker();	
}

void CSpellDialog::OnIgnoreAll() 
{
	SpellCheck_CHAR probWord[SpellCheck_MAX_WORD_SZ];
	
	m_pWndProbWordTxt->GetWindowText( (char*)probWord, sizeof(probWord));
	
	// The Ignore All button was pressed. Save the problem word
	// in the temporary ignore-type lexicon, and advance to the
	// next word.
	IgnoreAll(probWord);
		
	RunChecker();				  
}

void CSpellDialog::IgnoreAll(SpellCheck_CHAR* probWord) 
{
 	if (my_SpellCheck_AddToLex(m_sTmpIgnoreLexId, probWord, NULL) < 0)
		ErrorDialog(IDS_SPELL_ERR_SAVE_IGNORE);

}

void CSpellDialog::OnReplaceIt() 
{
	// Shareware: In reduced feature mode, no spell check
	if (!UsingFullFeatureSet())
		return;

	// FULL-FEATURE

	SpellCheck_CHAR probWord[SpellCheck_MAX_WORD_SZ];
	SpellCheck_CHAR repWord[SpellCheck_MAX_WORD_SZ];

	m_pWndProbWordTxt->GetWindowText( (char*)probWord, sizeof(probWord));
	m_pWndRepWordEdit->GetWindowText( (char*)repWord, sizeof(repWord)); 

	// The Replace button was pressed. Substitute the word
	// in the Replacement field for the current word in the
	// block.
	if (!m_bUseSugTempForProbWord)
	{
		m_pSpell->my_ReplaceBlockWord(probWord, repWord);

		// Add the word and the replacement to the temporary
		// Suggest-type lexicon. If the word occurs again, there's
		// a good chance the user will want to replace it.
		if (repWord[0] != NULL)
			my_SpellCheck_AddToLex(m_sTmpSuggestLexId, probWord, repWord);
	}
	else
	{
		m_pSpell->my_ReplaceBlockWord(m_szSuggestingForTempWord, repWord);
		my_SpellCheck_AddToLex(m_sTmpSuggestLexId, m_szSuggestingForTempWord, repWord);
	}

	RunChecker();
}

void CSpellDialog::OnReplaceAll() 
{
	// Shareware: In reduced feature mode, no spell check
	if (!UsingFullFeatureSet())
		return;

	// FULL-FEATURE

	SpellCheck_CHAR probWord[SpellCheck_MAX_WORD_SZ];
	SpellCheck_CHAR repWord[SpellCheck_MAX_WORD_SZ];

	m_pWndProbWordTxt->GetWindowText( (char*)probWord, sizeof(probWord));
	m_pWndRepWordEdit->GetWindowText( (char*)repWord, sizeof(repWord)); 

	// The Replace All button was pressed. Substitute the word
	// in the Replacement field for the current word in the
	// block. Also add the word and replacement to the
	// temporary Change-type lexicon, so the word will
	// automatically be replaced next time it occurs.
	if (!m_bUseSugTempForProbWord)
	{
		m_pSpell->my_ReplaceBlockWord(probWord, repWord);
		TRACE("Rep word -><%s>\n", repWord);
		if (my_SpellCheck_AddToLex(m_sTmpChangeLexId, probWord, repWord) < 0)
			ErrorDialog(IDS_SPELL_ERR_SAVE_CHANGE);
	}
	else
	{   
		m_pSpell->my_ReplaceBlockWord(m_szSuggestingForTempWord, repWord);
		TRACE("Rep word -><%s>\n", repWord);
		if (my_SpellCheck_AddToLex(m_sTmpChangeLexId, m_szSuggestingForTempWord, repWord) < 0)
			ErrorDialog(IDS_SPELL_ERR_SAVE_CHANGE);
	}

	RunChecker();
}

void CSpellDialog::OnAddToLex() 
{
	SpellCheck_CHAR probWord[SpellCheck_MAX_WORD_SZ];

	m_pWndProbWordTxt->GetWindowText( (char*)probWord, sizeof(probWord));

	AddToLex(probWord);
	RunChecker();
}

void CSpellDialog::AddToLex(SpellCheck_CHAR* probWord) 
{
 	if (my_SpellCheck_AddToLex(m_sPermIgnoreLexId, probWord, NULL) < 0)
		ErrorDialog(IDS_SPELL_ERR_SAVE_IGNORE);
}

void CSpellDialog::OnSuggest() 
{
	SpellCheck_CHAR	probWord[SpellCheck_MAX_WORD_SZ];
	SpellCheck_CHAR	repWord[SpellCheck_MAX_WORD_SZ];
	INT			n;

	m_pWndProbWordTxt->GetWindowText( (char*)probWord, sizeof(probWord));
	m_pWndRepWordEdit->GetWindowText( (char*)repWord, sizeof(repWord)); 

	// The Suggest button was pressed. Fill the Suggestions
	// list box with suggested replacements for the
	// current word.
	if (m_bSuggestingFor == TRUE)
	{  
		if (!m_bUseSugTempForProbWord)
		{
			m_bUseSugTempForProbWord = TRUE;
			strcpy((char*)m_szSuggestingForTempWord, (char*)probWord);
		}
		strcpy((char*)probWord, (char*)repWord);
		m_pWndProbLblTxt->SetWindowText( (LPCTSTR)CRString(IDS_SPELL_SUGGESTING_FOR)); 
		m_pWndProbWordTxt->SetWindowText( (char*)probWord);
	}
	
	n = getSuggestions(probWord);
	
	if (n > 0)
	{
		char firstWord[SpellCheck_MAX_WORD_SZ];

		// Display the first suggestion in the Replacement field.
		m_pWndSuggestList->SendMessage( LB_GETTEXT, 0, (LPARAM)firstWord);

		m_pWndRepWordEdit->SetWindowText( firstWord);
		SendMessage(WM_NEXTDLGCTL,(WPARAM)( m_pWndRepWordEdit->GetSafeHwnd() ),1L);  
	}

	// Advance to the next search depth in case the user
	// selects the Suggest button again.
	if (++m_sSearchDepth > MAX_SEARCH_DEPTH)
		m_pWndSuggestBtn->EnableWindow( FALSE);

	// Pressing the Suggest button doesn't properly dispose
	// of the current problem, so we're not ready to continue
	// checking the block.
}


void CSpellDialog::OnDelete() 
{
	// The Delete button was pressed. Delete the current
	// word in the block.
	my_SpellCheck_DelBlockWord(m_blkId);
	RunChecker();
}

void CSpellDialog::OnDblclkSuggestList() 
{
	SendMessage(WM_COMMAND,IDC_REPLACEIT, 0);
}

void CSpellDialog::OnSelchangeSuggestList() 
{
	int n;
	char selWord[SpellCheck_MAX_WORD_SZ];

	// The selected item in the Suggestion List has
	// changed. Copy the new selection to the Replacement
	// field.
	n = (int)m_pWndSuggestList->SendMessage(LB_GETCURSEL, 0, 0L);
	m_pWndSuggestList->SendMessage(LB_GETTEXT, n, (LPARAM)selWord);  
	m_pWndRepWordEdit->SetWindowText( selWord);	
}

void CSpellDialog::OnOptions() 
{
    // The Options button was pressed.
    CSpellOptionsDialog theDialog( this, m_ulOptionsMask, m_bAlwaysSuggest );
        
    if( theDialog.DoModal() == IDOK) {
                
        // Save the (possibly updated) options.
        m_ulOptionsMask = theDialog.GetOptionsMask();
        saveOptionMask( m_ulOptionsMask );
        m_bAlwaysSuggest = theDialog.GetAlwaysSuggest();
        SetIniShort( IDS_INI_ALWAYS_SUGGEST, (short) m_bAlwaysSuggest );

        // set the options that CheckBlock() doesn't support
        SetOptions();
    }
}


void CSpellDialog::OnJustDoIt() 
{
    EndDialog( IDOK );
}


void CSpellDialog::OnEditLex() 
{
	char	userPathName[_MAX_PATH];
	int		UserPathLen;
	int		iWhichLex;
	short	sEditLexID;

	// Open an SpellCheck session if necessary.
	if ( openSpellCheck() < 0)
		return;
		
	strcpy(userPathName, m_szUserLexPath);
	strcat(userPathName, SLASHSTR);
	UserPathLen = strlen(userPathName);

	// Set editLexId to the id of the permanent lexicon indicated by
	// the type value. editLexId is used by the dialog procedure to
	// refer to the lexicon being edited.
	iWhichLex = SpellCheckD_PERM_IGNORE_LEX;
	
	switch (iWhichLex)
	{
		case SpellCheckD_PERM_CHANGE_LEX:
			sEditLexID = m_sPermChangeLexId;	
		break;
		
		case SpellCheckD_PERM_SUGGEST_LEX:
			sEditLexID = m_sPermSuggestLexId;	
		break;
		
		case SpellCheckD_PERM_IGNORE_LEX: 
		default:	
			sEditLexID = m_sPermIgnoreLexId;	
		break;
    }

	// The Edit Lex button was pressed.
	CSpellEditLexDialog theDialog( this, sEditLexID );

	theDialog.DoModal();
	
	// Close and then re-open the edited lexicon. This will save its
	// contents to disk, so any changes won't be lost if the power
	// fails.
	my_SpellCheck_CloseLex( sEditLexID );

	switch (iWhichLex)
	{
	case SpellCheckD_PERM_CHANGE_LEX:
		strcpy(userPathName + UserPathLen, m_szPermChangeLexFileName);
		m_sPermChangeLexId = my_SpellCheck_OpenLex(userPathName, 0L); 
		break;
	case SpellCheckD_PERM_IGNORE_LEX:
		strcpy(userPathName + UserPathLen, m_szPermIgnoreLexFileName);
		m_sPermIgnoreLexId = my_SpellCheck_OpenLex(userPathName, 0L);
		break;
	case SpellCheckD_PERM_SUGGEST_LEX:
		strcpy(userPathName + UserPathLen, m_szPermSuggestLexFileName);
		m_sPermSuggestLexId = my_SpellCheck_OpenLex(userPathName, 0L);
		break;
	default:
		ASSERT(FALSE);
		break;
	}

}

#define POPUP_SUGGESTIONS 5

BOOL CSpellDialog::Popup(char* pWord, POINT& point, BOOL doubledWord) 
{
	//
	// Display Popup Menu
	//
	CMenu	menu;
	menu.CreatePopupMenu( );

	SpellCheck_CHAR suggBfr[SpellCheck_MAX_WORD_SZ * MAX_SUGGESTIONS + 1];
	static S16 scores[MAX_SUGGESTIONS];
	UINT n = 0;
	const SpellCheck_CHAR* p;
	//CWaitCursor wait;
	int topScore;	

	if (my_SpellCheck_Suggest((const SpellCheck_CHAR*)pWord, m_sSearchDepth, suggBfr, sizeof(suggBfr), scores,
		POPUP_SUGGESTIONS /*MAX_SUGGESTIONS*/) >= 0)
	{
		topScore = scores[0];

		// Display the best words in the list box.
		for (p = suggBfr; *p != 0; p += strlen((char*)p) + 1)
		{
			if (scores[n] < MIN_ALT_WORD_SCORE ||
				topScore - scores[n] > ALT_WORD_SCORE_RANGE)
			{
				break;
			}
			menu.AppendMenu( MF_STRING | MF_ENABLED, n + COMMAND_ID_WORD, (const char*) p );
			++n;
		}
	
		menu.AppendMenu( MF_SEPARATOR, 1);	
	}
	int nOffsetCommand = n+COMMAND_ID_WORD;

	menu.AppendMenu( MF_STRING | MF_ENABLED,  nOffsetCommand, (LPCTSTR)CRString(IDS_SPELL_IGNORE_ALL) );
	menu.AppendMenu( MF_STRING | MF_ENABLED,  nOffsetCommand+1, (LPCTSTR)CRString(IDS_SPELL_ADD) );

	//Is it a double word?
	if (doubledWord)
		menu.AppendMenu( MF_STRING | MF_ENABLED,  nOffsetCommand+2, (LPCTSTR)CRString(IDS_SPELL_DELETE) );

	n=0;
	UINT cmdID = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, AfxGetMainWnd());
   	if (cmdID)
	{
		if ( cmdID == (UINT)nOffsetCommand )
			return COMMAND_ID_IGNOREALL;
		else if ( cmdID == (UINT)(nOffsetCommand+1) )
		{
			if (my_SpellCheck_AddToLex(m_sPermIgnoreLexId, (const SpellCheck_CHAR*)pWord, NULL) < 0)
				ErrorDialog(IDS_SPELL_ERR_SAVE_IGNORE);
			return COMMAND_ID_ADD;
		}
		else if ( doubledWord && (cmdID == (UINT)(nOffsetCommand+2) ) )
			return COMMAND_ID_DELETE_WORD;
	

		for (p = suggBfr; *p != 0; p += strlen((char*)p) + 1)
		{
			if (scores[n] < MIN_ALT_WORD_SCORE ||
				topScore - scores[n] > ALT_WORD_SCORE_RANGE)
			{
				break;
			}
			if ( n++ == (cmdID-COMMAND_ID_WORD) )
			{
				strcpy(pWord, (char*) p);
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CSpellDialog::DoCoolSpell( BOOL bSilent /* = FALSE */) 
{
    BOOL bRet = TRUE;

    // Shareware: In reduced feature mode, no spell check
    if (UsingFullFeatureSet())
    {
        // FULL-FEATURE

        SpellCheck_CHAR   szProbWord[SpellCheck_MAX_WORD_SZ];
        SpellCheck_CHAR   szRepWord[SpellCheck_MAX_WORD_SZ];
        INT         iResult;

        CWaitCursor wait;
        m_bAUserChange = FALSE;

        m_pSpell->mySetRedraw(FALSE);

        for ( ;; ) {
            iResult = m_pSpell->my_CheckBlock( m_ulOptionsMask, szProbWord, szRepWord );

            if( iResult & SpellCheck_END_OF_BLOCK_RSLT )
                break;

            if( iResult & (SpellCheck_MISSPELLED_WORD_RSLT | SpellCheck_DOUBLED_WORD_RSLT) ) {
                if ( !bSilent )
                    m_pSpell->myHilightWord(TRUE);    // highlight the misspelled word
                bRet = FALSE;
            }
        }
    }

    m_pSpell->mySetRedraw(TRUE);

    return bRet;
}

int CSpellDialog::DoModalSpell( BOOL bAutoCheck /* = FALSE */ )
{
    m_bAutoCheck = bAutoCheck;
    return DoModal();
}
