#if !defined(AFX_SPELLDIALOG_H__4056D7B2_08FF_11D1_9818_00805FD2F268__INCLUDED_)
#define AFX_SPELLDIALOG_H__4056D7B2_08FF_11D1_9818_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SpellDialog.h : header file
//
#define SpellCheck_DLL_ FALSE

#include "ssce.h"

#define SpellCheck_CHAR unsigned char 

#define MAX_SEARCH_DEPTH		3
#define ALT_WORD_SCORE_RANGE	20		// accept only top 20%
#define MIN_ALT_WORD_SCORE		50		// ignore scores < 50%
#define MAX_SUGGESTIONS			32
#define MAX_MAIN_LEX_FILES		5		// max files forming main lexicon
#define MAX_FILE_NAME			( _MAX_FNAME + _MAX_EXT + 2 )

// Lexicon selectors:
#define SpellCheckD_PERM_CHANGE_LEX	0
#define SpellCheckD_PERM_IGNORE_LEX	1
#define SpellCheckD_PERM_SUGGEST_LEX	2


typedef S16 (CALLBACK* ADDTOLEXFUNCPTR)(S16, S16, const SpellCheck_CHAR*, const SpellCheck_CHAR*);
typedef S16 (CALLBACK* CHECKBLOCKFUNCPTR)(S16, S16, U32, SpellCheck_CHAR*, S16, SpellCheck_CHAR*, S16);
typedef S16 (CALLBACK* CHECKWORDFUNCPTR)(S16, U32, const SpellCheck_CHAR*, SpellCheck_CHAR*, S16) ;
typedef S16 (CALLBACK* CLEARLEXFUNCPTR)(S16, S16);
typedef S16 (CALLBACK* CLOSEBLOCKFUNCPTR)(S16, S16 ) ;
typedef S16 (CALLBACK* CLOSELEXFUNCPTR)(S16, S16) ;
typedef S16 (CALLBACK* CLOSESESSIONFUNCPTR)(S16) ;
typedef S16 (CALLBACK* CREATELEXFUNCPTR)(S16, const char FAR *, S16, S16) ;
typedef S16 (CALLBACK* DELBLOCKWORDFUNCPTR)(S16, S16) ;
typedef S16 (CALLBACK* DELFROMLEXFUNCPTR)(S16, S16, const SpellCheck_CHAR*);
typedef S16 (CALLBACK* GETBLOCKFUNCPTR)(S16, S16, SpellCheck_CHAR FAR *, S32) ;
typedef S16 (CALLBACK* GETBLOCKINFOFUNCPTR)(S16, S16, S32 FAR *, S32 FAR *, S32 FAR *, S32 FAR *) ;
typedef S16 (CALLBACK* GETBLOCKWORDFUNCPTR)(S16, S16, SpellCheck_CHAR FAR *, S16) ;
typedef S32 (CALLBACK* GETLEXFUNCPTR)(S16, S16, SpellCheck_CHAR*, S32);
typedef S16 (CALLBACK* GETLEXINFOFUNCPTR)(S16, S16, S32*, S16*, S16*, S16*);
typedef U32 (CALLBACK* GETOPTIONFUNCPTR)(S16, U32) ;
typedef S16 (CALLBACK* NEXTBLOCKWORDFUNCPTR)(S16, S16) ;
typedef S16 (CALLBACK* OPENBLOCKFUNCPTR)(S16, SpellCheck_CHAR FAR *, S32, S32, S16) ;
typedef S16 (CALLBACK* OPENLEXFUNCPTR)(S16, const char FAR *,S32) ;
typedef S16 (CALLBACK* OPENSESSIONFUNCPTR)(void) ;
typedef S16 (CALLBACK* REPLACEBLOCKWORDFUNCPTR)(S16, S16, const SpellCheck_CHAR FAR *) ;
typedef U32 (CALLBACK* SETOPTIONFUNCPTR)(S16, U32, U32) ;
typedef S16 (CALLBACK* SUGGESTFUNCPTR)(S16, const SpellCheck_CHAR FAR *, S16, SpellCheck_CHAR FAR *, S32, S16 FAR *, S16) ;
typedef void (CALLBACK* VERSIONFUNCPTR)(S16 FAR *, S16 FAR *) ;

// Used for popup menu
#define COMMAND_ID_ADD			100
#define COMMAND_ID_IGNOREALL	101
#define COMMAND_ID_DELETE_WORD	102
#define COMMAND_ID_WORD			103

class CSpell;

/////////////////////////////////////////////////////////////////////////////
// CSpellDialog dialog

class CSpellDialog : public CDialog
{
	DECLARE_DYNAMIC(CSpellDialog)

	// handle to the dll
	HINSTANCE		m_hSpellLib;

	// dll functions
	ADDTOLEXFUNCPTR			m_pfnAddToLex;
	CHECKBLOCKFUNCPTR		m_pfnCheckBlock;
	CHECKWORDFUNCPTR		m_pfnCheckWord;
	CLEARLEXFUNCPTR			m_pfnClearLex;
	CLOSEBLOCKFUNCPTR		m_pfnCloseBlock;
	CLOSELEXFUNCPTR			m_pfnCloseLex;
	CLOSESESSIONFUNCPTR		m_pfnCloseSession;
	CREATELEXFUNCPTR		m_pfnCreateLex;
	DELBLOCKWORDFUNCPTR		m_pfnDelBlockWord;
	DELFROMLEXFUNCPTR		m_pfnDelFromLex;
	GETBLOCKFUNCPTR			m_pfnGetBlock;
	GETBLOCKINFOFUNCPTR		m_pfnGetBlockInfo;
	GETBLOCKWORDFUNCPTR		m_pfnGetBlockWord;
	GETLEXFUNCPTR			m_pfnGetLex;
	GETLEXINFOFUNCPTR		m_pfnGetLexInfo;
	GETOPTIONFUNCPTR		m_pfnGetOption;
	NEXTBLOCKWORDFUNCPTR	m_pfnNextBlockWord;
	OPENBLOCKFUNCPTR		m_pfnOpenBlock;
	OPENLEXFUNCPTR			m_pfnOpenLex;
	OPENSESSIONFUNCPTR		m_pfnOpenSession;
	REPLACEBLOCKWORDFUNCPTR	m_pfnReplaceBlockWord;
	SETOPTIONFUNCPTR		m_pfnSetOption;
	SUGGESTFUNCPTR			m_pfnSuggest;
	VERSIONFUNCPTR			m_pfnVersion;
	
		
	SpellCheck_CHAR	m_szSuggestingForTempWord[SpellCheck_MAX_WORD_SZ];

	// Temporary lexicon ids:
	short	m_sTmpIgnoreLexId;
	short	m_sTmpSuggestLexId;
	short	m_sTmpChangeLexId;
	
	// Id of SpellCheck block to check:
	short	m_blkId;

	// SpellCheck Session id:
	short	m_sid;

	BOOL	m_bDeleteBtn;  // Is replace button set to "Delete"? 

	// Permanent lexicon ids:
	short	m_sPermIgnoreLexId;
	short	m_sPermChangeLexId;
	short	m_sPermSuggestLexId;

	// Permanent lexicon file names:
	char	m_szPermChangeLexFileName[MAX_FILE_NAME];
	char	m_szPermIgnoreLexFileName[MAX_FILE_NAME];
	char	m_szPermSuggestLexFileName[MAX_FILE_NAME];

	UINT	m_uDefID; // Default pushbutton in Check Word Dialog

	BOOL	m_bAUserChange;	// Must be set to false anytime the replace word edit ctrl is modified by the program

	BOOL 	m_bSuggestingFor;
	BOOL 	m_bUseSugTempForProbWord;  

	// SpellCheck_CheckBlock options:
	ULONG	m_ulOptionsMask;

	// Path where lexicon files reside:
	char	m_szUserLexPath[_MAX_PATH];
	char	m_szLexPath[ _MAX_PATH ];

	// TRUE if suggestions are given automatically after a misspelled word is
	// encountered; FALSE if user must select Suggest button for suggestion:
	//
	BOOL	m_bAlwaysSuggest;

	// Default language id used when creating lexicons:
	char	m_byLanguage;

	// Current SpellCheck_Suggest search depth (used only if alwaysSuggest is FALSE:
	short	m_sSearchDepth;

	CSpell* m_pSpell; 
	RECT	m_rectWnd;
	BOOL	m_bFoundMisspelling;
	INT		m_iError;
	CWnd*	m_pWndIgnoreBtn;
	CWnd*	m_pWndIgnoreAllBtn;
	CWnd*	m_pWndReplaceBtn;
	CWnd*	m_pWndReplaceAllBtn;
	CWnd*	m_pWndAddToLexBtn;
	CWnd*	m_pWndSuggestBtn;
//	CWnd*	m_pWndCancelBtn;
    CWnd* m_pWndJustDoIt;
	CWnd*	m_pWndSuggestList;
	CWnd*	m_pWndProbLblTxt;
	CWnd*	m_pWndProbWordTxt;
	CWnd*	m_pWndRepWordEdit;  

	bool m_bHasBeenInitialized;
    BOOL m_bAutoCheck;
	// Main lexicon file name(s). The main lexicon may be implemented
	// via several files:
	//
	char	m_arrMainLexFileNames[MAX_MAIN_LEX_FILES][MAX_FILE_NAME];
	UINT	m_nMainLexFiles;

	// Main lexicon ids:
	short	m_arrMainLexIds[MAX_MAIN_LEX_FILES];

	void	busy(BOOL on);
	INT		getSuggestions(const SpellCheck_CHAR* word );
	void	RunChecker();

	void parseFileList(const char* fileList, char fileNames[][MAX_FILE_NAME],
		size_t maxFileNames, UINT* nFileNames);

	INT		openSpellCheck();
	void	closeSpellCheck();
	int		LoadSpellLib();
	int		CloseSpellLib();
	void	saveOptionMask(unsigned long mask);
	void	SetOptions();
	BOOL	Init();
	
protected:

	friend class CSpellEditLexDialog;

	S16 my_SpellCheck_AddToLex(S16 lexId, const SpellCheck_CHAR FAR *word, const SpellCheck_CHAR FAR *otherWord);
	S16 my_SpellCheck_CheckBlock(S16 blkId, U32 options, SpellCheck_CHAR FAR *errWord, S16 errWordSz, SpellCheck_CHAR FAR *repWord, S16 repWordSz);
	S16 my_SpellCheck_ClearLex(S16 lexId); 
	S16 my_SpellCheck_CloseBlock(S16 blkId); 
	S16 my_SpellCheck_CloseLex(S16 lexId);
	S16 my_SpellCheck_CloseSession(S16 sid);
	S16 my_SpellCheck_CreateLex(const char FAR *fileName, S16 type, S16 lang);
	S16 my_SpellCheck_DelBlockWord(S16 blkId);
	S16 my_SpellCheck_DelFromLex(S16 lexId, const SpellCheck_CHAR FAR *word);
	S32 my_SpellCheck_GetBlock(S16 blkId, SpellCheck_CHAR FAR *block, S32 blkSz);
	S16 my_SpellCheck_GetBlockInfo(S16 blkId, S32 FAR *blkLen, S32 FAR *blkSz, S32 FAR *curPos, S32 FAR *wordCount);
	S16 my_SpellCheck_GetBlockWord(S16 blkId, SpellCheck_CHAR FAR *word, S16 wordSz);
	S32 my_SpellCheck_GetLex(S16 lexId, SpellCheck_CHAR FAR *lexBfr, S32 lexBfrSz);
	S16 my_SpellCheck_GetLexInfo(S16 lexId, S32 FAR *size, S16 FAR *format, S16 FAR *type, S16 FAR *lang);
	U32 my_SpellCheck_GetOption(U32 option);
	S16 my_SpellCheck_NextBlockWord(S16 blkId);
	S16 my_SpellCheck_OpenBlock(SpellCheck_CHAR FAR *block, S32 blkLen, S32 blkSz, S16 copyBlock);
	S16 my_SpellCheck_OpenLex(const char FAR *fileName,S32 memBudget);
	S16 my_SpellCheck_OpenSession(void);
	S16 my_SpellCheck_ReplaceBlockWord(S16 blkId, const SpellCheck_CHAR FAR *word);
	U32 my_SpellCheck_SetOption(U32 option, U32 value);
	S16 my_SpellCheck_Suggest(const SpellCheck_CHAR FAR *word, S16 depth, SpellCheck_CHAR FAR *suggBfr, S32 suggBfrSz, S16 FAR *scores, S16 scoreSz);
	void my_SpellCheck_Version(S16 FAR *major, S16 FAR *minor);

// Construction
public:
	CSpellDialog( CSpell* pSpell, CWnd* pParent = NULL );   // standard constructor
	virtual ~CSpellDialog();

	INT		GetLastError() { return m_iError; };
	BOOL	FoundMisspelling() { return m_bFoundMisspelling; }
	void	SetMisspellings( BOOL b ) { m_bFoundMisspelling = b; }
	ULONG	GetOptionsMask() { return m_ulOptionsMask; }
	BOOL	Popup(char* pWord, POINT& point, BOOL doubledWord = FALSE,
		int nHideIgnoreMenu = 0);
	BOOL	DoCoolSpell( BOOL bSilent = FALSE );
    int DoModalSpell( BOOL bAutoCheck = FALSE );
	void	AddToLex(SpellCheck_CHAR* probWord);
	void	IgnoreAll(SpellCheck_CHAR* probWord);
	bool	IsDialogOpen() { return m_bHasBeenInitialized; }
	BOOL	m_bJustQueue;

	S16 my_SpellCheck_CheckWord(U32 options, const SpellCheck_CHAR FAR *word, SpellCheck_CHAR FAR *replWord, S16 replWordSz);

// Dialog Data
	//{{AFX_DATA(CSpellDialog)
	enum { IDD = IDD_SPELL_CHECK_BLOCK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpellDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnChangeReplacementWord();
	afx_msg void OnIgnore();
	afx_msg void OnIgnoreAll();
	afx_msg void OnReplaceIt();
	afx_msg void OnReplaceAll();
	afx_msg void OnAddToLex();
	afx_msg void OnSuggest();
	afx_msg void OnDblclkSuggestList();
	afx_msg void OnSelchangeSuggestList();
	afx_msg void OnOptions();
	afx_msg void OnJustDoIt();
	afx_msg void OnEditLex();
	//}}AFX_MSG

	void EndDialog(int nResult);	
	afx_msg LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	afx_msg long OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDelete();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLDIALOG_H__4056D7B2_08FF_11D1_9818_00805FD2F268__INCLUDED_)
