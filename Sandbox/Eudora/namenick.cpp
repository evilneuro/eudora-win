// NAMENICK.CPP
//
// Code for handling the Nickname naming dialog, and Make Nickname


#include "stdafx.h"
#ifdef WIN32
#include <afxcmn.h>
#include <afxrich.h>
#endif
#include <ctype.h>

#include <QCUtils.h>

#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "fileutil.h"
#include "doc.h"
#include "nickdoc.h"
#include "3dformv.h"
#include "helpxdlg.h"
#include "namenick.h"
#include "guiutils.h"
#include "tocview.h"
#include "msgdoc.h"
#include "compmsgd.h"
#include "address.h"
#include "msgutils.h"
#include "mainfrm.h"

//#include "urledit.h"	// for LOOKUP.H
//#include "lookup.h"

//#include "LookupWazooWnd.h"
#include "DirectoryServicesWazooWndNewIface.h"
#include "NicknamesWazooWnd.h"
#include "PgReadMsgPreview.h"
#include "TridentPreviewView.h"
#include "TocFrame.h"

#include "QCSharewareManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CNameNickDialog dialog


CNameNickDialog::CNameNickDialog(UINT Type, CNicknameFile* pSelectedNicknameFile /*= NULL*/, CWnd* pParent /*= NULL*/)
	: CHelpxDlg(CNameNickDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNameNickDialog)
	//}}AFX_DATA_INIT
	m_NewName = "";
	m_PutOnRecipientList = FALSE;
	m_CreateNicknameFile = FALSE;
	m_Type = Type;
	m_pNicknameFile = NULL;
	m_MakeNickAdd = FALSE;

	//
	// Caller is giving us a hint which nickname file item is selected
	// in the view.
	//
	m_pSelectedNicknameFile = pSelectedNicknameFile;	// can be NULL
}

BOOL CNameNickDialog::OnInitDialog()
{
	CHelpxDlg::OnInitDialog();
	
	ASSERT(NULL == m_pNicknameFile);
	ASSERT(g_Nicknames);

//FORNOW	if (! LoadNicknames())
//FORNOW	{
//FORNOW		OnCancel();
//FORNOW		return (FALSE);
//FORNOW	}
	
	// Set the title of the dialog depending on what action is being done
	UINT TitleID;
	switch (m_Type)
	{
	case IDC_NEW:					
		TitleID = IDS_NICK_NEW_TITLE;		
		break;
	case ID_SPECIAL_MAKENICKNAME:
		TitleID = IDS_NICK_MAKE_TITLE;
		break;
	case IDCANCEL:
		TitleID = IDS_NICK_NEW_TITLE;		
		if (NULL == m_pSelectedNicknameFile)
		{
			ASSERT(0);		// caller must specify a target nickname file
			return FALSE;
		}
		break;
	default:
		ASSERT(FALSE);
		return (FALSE);
	}
	SetWindowText(CRString(TitleID));
	
	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		// New and Make Nickname get list of nickname files
		if (m_Type == IDC_NEW || m_Type == ID_SPECIAL_MAKENICKNAME)
		{
			int select_idx = 0;
			for (CNicknameFile* NickFile = g_Nicknames->GetFirstNicknameFile();
				 NickFile != NULL;
				 NickFile = g_Nicknames->GetNextNicknameFile())
			{
				//
				// Don't list read-only or invisible(peanut) nickname files as candidates for
				// new nicknames.
				//
				if ((NickFile->m_ReadOnly == FALSE) && (NickFile->m_Invisible == FALSE))
				{
					int idx = m_Filename.AddString(NickFile->m_Name);

					if (m_pSelectedNicknameFile == NickFile)
						select_idx = idx;
				}
			}

			m_Filename.SetCurSel(select_idx);
			m_Filename.ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_NICK_FILE_PROMPT)->ShowWindow(SW_SHOWNA);
		}
		else if (m_Type == IDCANCEL)
		{
			//
			// Add the one and only nickname file as the target in the combo box.
			//
			m_Filename.AddString(m_pSelectedNicknameFile->m_Name);
			m_Filename.SetCurSel(0);
			m_Filename.ShowWindow(SW_SHOWNA);
			m_Filename.EnableWindow(FALSE);
			GetDlgItem(IDC_NICK_FILE_PROMPT)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_NICK_FILE_PROMPT)->EnableWindow(FALSE);
		}

		if (m_Type == IDC_NEW)
		{
			// Unhide the "make it a file" checkbox.
			CButton* p_checkbox = (CButton *) GetDlgItem(IDC_CREATE_NICKNAME_FILE);
			ASSERT(p_checkbox != NULL);
			p_checkbox->ShowWindow(SW_SHOWNA);
		}
	}

	CenterWindow();
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}

void CNameNickDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNameNickDialog)
	DDX_Control(pDX, IDC_NICK_FILENAME_COMBO, m_Filename);
	DDX_Text(pDX, IDC_NEW_NAME, m_NewName);
	DDV_MaxChars(pDX, m_NewName, CNickname::MAX_NAME_LENGTH);
	DDX_Check(pDX, IDC_RECIPIENT_LIST, m_PutOnRecipientList);
	DDX_Check(pDX, IDC_CREATE_NICKNAME_FILE, m_CreateNicknameFile);
	//}}AFX_DATA_MAP
}


////////////////////////////////////////////////////////////////////////
// OnOK [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNameNickDialog::OnOK()
{
	if (!UpdateData(TRUE) || m_NewName.IsEmpty())
		return;
	
	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		//
		// Are we just creating a new nickname file?  If so, then do some
		// error checking, then pass it off to the nickname file creation
		// routine.
		//
		if ((IDC_NEW == m_Type) && m_CreateNicknameFile)
		{
	#ifndef WIN32
			//
			// For Win16, the CNicknameFile objects seem to always store
			// filename names in all upper case.  Therefore, in the case
			// where a user has just created a new file, let's make sure
			// that the filename is all upper case, just for consistency.
			//
			m_NewName.MakeUpper();
	#endif // !WIN32

	#ifdef WIN32
			// Check for illegal characters in base part of filename
			int idx = m_NewName.FindOneOf("/\\:*?\"<>|");
			if (idx >= 0)
			{
				ErrorDialog(IDS_NICKFILE_BAD_CHARACTER, char(m_NewName[idx]));
				return;
			}
	#else
			// Check for illegal characters in base part of 8.3 filenames
			int idx = m_NewName.FindOneOf("., /\\:*?\"<>|");
			if (idx >= 0)
			{
				ErrorDialog(IDS_NICKFILE_BAD_CHARACTER, char(m_NewName[idx]));
				return;
			}
			else if (m_NewName.GetLength() > 8)
			{
				//
				// Since we don't have the equivalent of the DESCMAP.PCE
				// file for nickname files, let's restrict the name to
				// 8 characters so that we can assume 8.3 format on all
				// platforms.
				//
				ErrorDialog(IDS_NICKFILE_TOO_LONG);
				return;
			}
	#endif // WIN32
			else if (NicknameFileExists(m_NewName + CRString(IDS_NICK_FILE_EXTENSION)))
			{
				ErrorDialog(IDS_ERR_NICKFILE_EXISTS, (const char*) (m_NewName + CRString(IDS_NICK_FILE_EXTENSION)));
				return;
			}
			else if (! CreateNewNicknameFile(m_NewName + CRString(IDS_NICK_FILE_EXTENSION)))
				return;

			CHelpxDlg::OnOK();		// dismiss dialog
			return;
		}
	}

	CNickname nickname(m_NewName);
	if (! nickname.HasValidName())
	{
		ErrorDialog(IDS_ERR_NICK_INVALID_CHAR);
		return;
	}

	//
	// Determine which target Nickname file we're dealing with, then
	// initialize 'm_pNicknameFile' to point to the target Nickname file.
	//
	switch (m_Type)
	{
	case IDC_NEW:
	case ID_SPECIAL_MAKENICKNAME:
		{
			//
			// Figure out the Nickname file index from the combo box index.
			// Non-trivial because the combo box listing doesn't include
			// readonly nickname files.
			//
			ASSERT(g_Nicknames != NULL);
			int combo_idx = m_Filename.GetCurSel();
			for (m_pNicknameFile = g_Nicknames->GetFirstNicknameFile();
			     m_pNicknameFile != NULL;
				 m_pNicknameFile = g_Nicknames->GetNextNicknameFile())
			{
				if (m_pNicknameFile->m_ReadOnly == FALSE && m_pNicknameFile->m_Invisible == FALSE && combo_idx-- <= 0)
					break;
			}
		}
		break;
	case IDCANCEL:
		m_pNicknameFile = m_pSelectedNicknameFile;
		break;
	default:
		ASSERT(0);
		return;
	}
	
	if (! m_pNicknameFile)
	{
		ASSERT(0);
		return;
	}

	//
	// Depending on which "type" of make nickname operation we're
	// doing, either check for a duplicate nickname in the target file
	// or not.  If we do check for and detect a duplicate, then
	// display an error message, then bail without dismissing the
	// dialog.
	//
	switch (m_Type)
	{
	case IDC_NEW:
	case IDCANCEL:
		if (m_pNicknameFile->FindNickname(m_NewName))
		{
			m_pNicknameFile = NULL;
			ErrorDialog(IDS_ERR_NICK_NAME_USED);
			return;
		}
		break;
	case ID_SPECIAL_MAKENICKNAME:
		break;
	default:
		ASSERT(0);
		return;
	}

	CHelpxDlg::OnOK();
}


////////////////////////////////////////////////////////////////////////
// OnMakeFileClicked [protected]
//
// Handles state changes for "make it a file" check box.
////////////////////////////////////////////////////////////////////////
void CNameNickDialog::OnMakeFileClicked()
{
	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		// Retrieve "make it a file" button state.
		if (! UpdateData(TRUE))
			return;
		
		if (m_CreateNicknameFile)
		{
			//
			// Checked, so disable nickname "file selection" combo box,
			// its corresponding label, and the "put on recipient list"
			// checkbox.
			//
			GetDlgItem(IDC_NICK_FILENAME_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_NICK_FILE_PROMPT)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECIPIENT_LIST)->EnableWindow(FALSE);
		}
		else
		{
			//
			// Unchecked, so enable nickname "file selection" combo box,
			// its corresponding label, and the "put on recipient list"
			// checkbox.
			//
			GetDlgItem(IDC_NICK_FILENAME_COMBO)->EnableWindow(TRUE);
			GetDlgItem(IDC_NICK_FILE_PROMPT)->EnableWindow(TRUE);
			GetDlgItem(IDC_RECIPIENT_LIST)->EnableWindow(TRUE);
		}
	}
	else
	{
		// REDUCED FEATURE
		ASSERT(0); // this should only occur in full feature mode
	}
}


BEGIN_MESSAGE_MAP(CNameNickDialog, CHelpxDlg)
	//{{AFX_MSG_MAP(CNameNickDialog)
	ON_BN_CLICKED(IDC_CREATE_NICKNAME_FILE, OnMakeFileClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CChooseNicknameFileDlg dialog

////////////////////////////////////////////////////////////////////////
// CChooseNicknameFileDlg [public, constructor]
//
// The 'pSourceNicknameFile' gives us a hint which nickname file
// contains the singly-selected "source" nickname.  That way, we can
// inhibit the display of the source nickname file -- i.e., it doesn't
// make sense to list the source nickname file as a choosable target.
////////////////////////////////////////////////////////////////////////
CChooseNicknameFileDlg::CChooseNicknameFileDlg
(
	CNicknameFile* pSourceNicknameFile /*=NULL*/, 
	CWnd* pParent /*=NULL*/
) :
	CHelpxDlg(CChooseNicknameFileDlg::IDD, pParent),
	m_pSourceNicknameFile(pSourceNicknameFile),			// can be NULL
	m_pTargetNicknameFile(NULL)
{
	//{{AFX_DATA_INIT(CChooseNicknameFileDlg)
	//}}AFX_DATA_INIT
}

////////////////////////////////////////////////////////////////////////
// OnInitDialog [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL CChooseNicknameFileDlg::OnInitDialog()
{
	// Shareware: Reduced feature mode only allows one nickname file, so no CChooseNicknameFileDlg
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE
		ASSERT(0);
		return (TRUE);
	}

	CHelpxDlg::OnInitDialog();
	
//FORNOW	if (! LoadNicknames())
//FORNOW	{
//FORNOW		OnCancel();
//FORNOW		return (FALSE);
//FORNOW	}
	
	//
	// Populate list box with all writable nickname files.
	//
	ASSERT(g_Nicknames);
	for (CNicknameFile* p_nickfile = g_Nicknames->GetFirstNicknameFile();
	     p_nickfile != NULL;
		 p_nickfile = g_Nicknames->GetNextNicknameFile())
	{
		//
		// Don't add read-only nickname files and don't add the source
		// nickname file which is already selected.
		//
		if (p_nickfile->m_ReadOnly == FALSE && p_nickfile->m_Invisible == FALSE &&
			m_pSourceNicknameFile != p_nickfile)
		{
			m_FileListBox.AddString(p_nickfile->m_Name);
		}
	}

	m_FileListBox.SetCurSel(0);
	
	CenterWindow();
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}


void CChooseNicknameFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseNicknameFileDlg)
	DDX_Control(pDX, IDC_LIST_NICKNAME_FILE, m_FileListBox);
	//}}AFX_DATA_MAP
}

////////////////////////////////////////////////////////////////////////
// OnOK [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CChooseNicknameFileDlg::OnOK()
{
	if (! UpdateData(TRUE))
		return;
	
	//
	// Determine which Nickname file we're dealing with.  Set
	// 'm_pTargetNicknameFile' here.  Open up a local scope to define
	// some local variables.
	//
	{
		//
		// Figure out the Nickname file from the list box index.
		// Non-trivial because the list box doesn't include readonly
		// nickname files.
		//
		ASSERT(g_Nicknames != NULL);
		int sel_idx = m_FileListBox.GetCurSel();
		for (m_pTargetNicknameFile = g_Nicknames->GetFirstNicknameFile();
			 m_pTargetNicknameFile != NULL;
			 m_pTargetNicknameFile = g_Nicknames->GetNextNicknameFile())
		{
			if (m_pTargetNicknameFile->m_ReadOnly == FALSE && m_pTargetNicknameFile->m_Invisible == FALSE &&
			    m_pTargetNicknameFile != m_pSourceNicknameFile &&
			    sel_idx-- <= 0)
				break;
		}
	}
	
	if (! m_pTargetNicknameFile)
	{
		// nothing selected?
		ASSERT(0);
		return;
	}

	CHelpxDlg::OnOK();
}

BEGIN_MESSAGE_MAP(CChooseNicknameFileDlg, CHelpxDlg)
	//{{AFX_MSG_MAP(CChooseNicknameFileDlg)
	ON_LBN_DBLCLK(IDC_LIST_NICKNAME_FILE, OnFileListBoxDblClk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChooseNicknameFileDlg message handlers


////////////////////////////////////////////////////////////////////////
// OnFileListBoxDblClk [protected]
//
// Shorthand for clicking the OK button.
//
////////////////////////////////////////////////////////////////////////
void CChooseNicknameFileDlg::OnFileListBoxDblClk()
{
	OnOK();
}


static void MergeAddresses(char*& Addresses, const char* From = NULL, const char* To = NULL,
	const char* Cc = NULL, const char* Bcc = NULL)
{
    const char Newline[] = "\r\n";
	int NewLength = ::SafeStrlenMT(Addresses) + ::SafeStrlenMT(From) + ::SafeStrlenMT(To) + ::SafeStrlenMT(Cc) +
		::SafeStrlenMT(Bcc) + 8;
	char* NewBuf = new char[NewLength + 1];
	const char* ptr;
	
	if (!NewBuf)
		return;
		
	if (!Addresses)
		*NewBuf = 0;
	else
	{
		strcpy(NewBuf, Addresses);
		delete [] Addresses;
	}
	Addresses = NewBuf;

	if (To)
	{
    	for (ptr = To; *ptr && isspace((int)(unsigned char)*ptr); ptr++) {}
		if (*Addresses && *ptr)
        	strcat(Addresses, Newline);
		strcat(Addresses, To);
	}
	if (From)
	{
    	for (ptr = From; *ptr && isspace((int)(unsigned char)*ptr); ptr++) {}
		if (*Addresses && *ptr)
			strcat(Addresses, Newline);
		strcat(Addresses, From);
	}
	if (Cc)
	{
		for (ptr = Cc; *ptr && isspace((int)(unsigned char)*ptr); ptr++) {}
		if (*Addresses && *ptr)
			strcat(Addresses, Newline);
		strcat(Addresses, Cc);
	}
	if (Bcc)
	{
		for (ptr = Bcc; *ptr && isspace((int)(unsigned char)*ptr); ptr++) {}
		if (*Addresses && *ptr)
			strcat(Addresses, Newline);
		strcat(Addresses, Bcc);
	}
}

static void AddMessageAddresses(CSummary* Sum, char*& Addresses, BOOL DoAll)
{
	if (!Sum)
		return;

	BOOL NewDoc = (Sum->FindMessageDoc() == NULL);
	CMessageDoc* MsgDoc = Sum->GetMessageDoc();
	if (!MsgDoc)
		return;
		
	const char* From = NULL;
	const char* To = NULL;
	const char* Cc = NULL;
	const char* Bcc = NULL;
	
	if (Sum->IsComp())
	{
		// Need to make sure message is read if we don't have a window
		if (!Sum->m_FrameWnd)
			MsgDoc->GetText();
		CCompMessageDoc* CompMsgDoc = (CCompMessageDoc*)MsgDoc;
			
		// I don't believe we want always the FROM field [jab 5/30/95] 
		if (DoAll)
			From = CompMsgDoc->GetHeaderLine(HEADER_FROM);
		To = CompMsgDoc->GetHeaderLine(HEADER_TO);
		Cc = CompMsgDoc->GetHeaderLine(HEADER_CC);
		Bcc = CompMsgDoc->GetHeaderLine(HEADER_BCC);
	}
	else
	{
		const char* MessageText = MsgDoc->GetText();
		From = HeaderContents(IDS_HEADER_FROM, MessageText);
		if (DoAll)
		{
			To = HeaderContents(IDS_HEADER_TO, MessageText);
			Cc = HeaderContents(IDS_HEADER_CC, MessageText);
		}
	}
	
	MergeAddresses(Addresses, From, To, Cc, Bcc);

	if (Sum->IsComp() == FALSE)
	{
		delete [] (char*)From;
		delete [] (char*)To;
		delete [] (char*)Cc;
		delete [] (char*)Bcc;
	}

	if (NewDoc)
		MsgDoc->OnCloseDocument();
}


////////////////////////////////////////////////////////////////////////
// MakeAutoNickname [static]
//
// Convert a string of the form:
//
//     "LastName, FirstName" <name@domain.com>
//
// to a string of the form:
//
//     LastName_FirstName
//
// that is no longer than 32 characters long
//
////////////////////////////////////////////////////////////////////////
static void MakeAutoNickname(CString& address)
{
	// Strip everything after and including the first open bracket, if any
	address = address.SpanExcluding("<");

	// Strip (illegal) leading/trailing whitespace, if any
	address.TrimLeft();
	address.TrimRight();

	// Strip double-quotes, if any
	int idx = -1;
	while ((idx = address.Find("\"")) != -1)
		address = address.Left(idx) + address.Right(address.GetLength() - idx - 1);

	// Replace spaces and (illegal) commas with underscores
	while ((idx = address.FindOneOf(" ,")) != -1)
		address.SetAt(idx, '_');

	// Replace at-signs with hyphens
	while ((idx = address.FindOneOf("@")) != -1)
		address.SetAt(idx, '-');

	// As a final pass, shorten double-underscore sequences to single underscores.
	while ((idx = address.Find("__")) != -1)
		address = address.Left(idx) + address.Right(address.GetLength() - idx - 1);

	// Then make sure the auto-generated name meets the length requirements
	address = address.Left(32);
}


////////////////////////////////////////////////////////////////////////
// GetAutoNicknameFromWindow [static]
//
// Formats the 'autoNickname' string with an "auto" nickname, based on
// the current open window.  Returns TRUE if the autoNickname was
// successfully set.
////////////////////////////////////////////////////////////////////////
static BOOL GetAutoNicknameFromWindow(UINT windowID, CString& autoNickname)
{
	CFrameWnd* p_frame = (CFrameWnd *) AfxGetMainWnd();
	ASSERT(p_frame != NULL);
	ASSERT(p_frame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	ASSERT(autoNickname.IsEmpty());

	if (NULL == p_frame)
	{
		ASSERT(0);
		return FALSE;
	}

	switch (windowID)
	{
//FORNOW	case IDR_LOOKUP32:
//FORNOW		{
//FORNOW			//
//FORNOW			// If Make Nickname is being done on a *single*, selected item
//FORNOW			// in the PH window, then pre-populate the "new name" field of
//FORNOW			// the dialog box with a stripped down version of the name.
//FORNOW			// Don't bother with any of this if there are multiple
//FORNOW			// selected items.
//FORNOW			//
//FORNOW			CLookupView* p_lookup_view = (CLookupView *) p_frame->GetActiveFrame()->GetActiveView();
//FORNOW
//FORNOW			//
//FORNOW			// Ask the CLookupView object for a list of selected e-mail 
//FORNOW			// addresses, assumed to be of the form:
//FORNOW			//
//FORNOW			//      "Real Name" <name@domain.com>
//FORNOW			//
//FORNOW			if (p_lookup_view)
//FORNOW			{
//FORNOW				ASSERT(p_lookup_view->IsKindOf(RUNTIME_CLASS(CLookupView)));
//FORNOW
//FORNOW				CStringList address_list;
//FORNOW			 	if (p_lookup_view->GetSelectedAddresses(address_list) && 
//FORNOW					(address_list.GetCount() == 1))
//FORNOW				{
//FORNOW					CString address = address_list.RemoveHead();
//FORNOW
//FORNOW					MakeAutoNickname(address);
//FORNOW					if (! address.IsEmpty())
//FORNOW						autoNickname = address;
//FORNOW				}
//FORNOW			}
//FORNOW		}
//FORNOW		break;
	case IDR_TOC:
		{
			//
			// There are two cases here.  If the focus is in the TOC
			// list box, then we need to grab the single-selected Summary.
			// Otherwise, if the focus is in the preview pane, then we
			// need to grab the "preview" Summary cached by the TocFrame.
			//
			CView* pView = p_frame->GetActiveFrame()->GetActiveView();
			CTocView* pTocView = NULL;
			CSummary* pSummary = NULL;
			if ((pTocView = DYNAMIC_DOWNCAST(CTocView, pView)) != NULL)
			{
				//
				// Grab single-selected Summary from TOC list box --
				// don't bother with any of this if there are multiple
				// selected items.  If Make Nickname is being done on
				// a *single*, selected item in a TOC window, then
				// pre-populate the "new name" field of the dialog box
				// with a stripped down version of the sender's "Real
				// Name", if any.
				//
			 	if (pTocView->m_SumListBox.GetSelCount() == 1)
				{
					int nIndex = -1;
					pTocView->m_SumListBox.GetSelItems(1, &nIndex);

					pSummary = pTocView->m_SumListBox.GetItemDataPtr(nIndex);
				}
			}
			else if (DYNAMIC_DOWNCAST(PgReadMsgPreview, pView) ||
					 DYNAMIC_DOWNCAST(CTridentPreviewView, pView))
			{
				//
				// Grab cached Summary pointer from the TocFrame.
				//
				CTocFrame* pTocFrame = (CTocFrame *) p_frame->GetActiveFrame();
				ASSERT_KINDOF(CTocFrame, pTocFrame);

				pSummary = pTocFrame->GetPreviewSummary();
			}
			else
			{
				ASSERT(0);		// unknown View type
			}

			if (pSummary)
			{
				CString name(pSummary->m_From);

				MakeAutoNickname(name);

				if (! name.IsEmpty())
					autoNickname = name;
			}
		}
		break;
	case IDR_READMESS:
	case IDR_COMPMESS:
		{
			//
			// If Make Nickname is being done on a read or comp message
			// window, then pre-populate the "new name" field of the
			// dialog box with a stripped down version of the sender's
			// "Real Name" for read messages, or with the recipient's
			// name (To/Cc/Bcc) for comp messages.
			//
			CMessageDoc* p_msgdoc = (CMessageDoc *) p_frame->GetActiveFrame()->GetActiveDocument();
			if (p_msgdoc)
			{
				ASSERT(p_msgdoc->IsKindOf(RUNTIME_CLASS(CMessageDoc)));
				if (p_msgdoc->m_Sum)
				{
					CString name;
					if (p_msgdoc->m_Sum->IsComp())
					{
						//
						// Comp message case.  The auto nickname naming
						// only applies if there is a single recipient
						// in all of the To:, Cc:, and Bcc: headers.
						//
						CCompMessageDoc* p_compdoc = (CCompMessageDoc*) p_msgdoc;
						ASSERT(p_compdoc->IsKindOf(RUNTIME_CLASS(CCompMessageDoc)));

						//
						// Build up the recipient list to see if we can
						// form an auto-nickname from it....
						//
						for (int i = 1; i <= 3; i++)
						{
							const char* p_header = NULL;
							switch (i)
							{
							case 1:
								p_header = p_compdoc->GetHeaderLine(HEADER_TO);
								break;
							case 2:
								p_header = p_compdoc->GetHeaderLine(HEADER_CC);
								break;
							case 3:
								p_header = p_compdoc->GetHeaderLine(HEADER_BCC);
								break;
							default:
								ASSERT(0);
								p_header = NULL;
								break;
							}

							if (p_header && *p_header)
							{
								if (! name.IsEmpty())
									name += ", ";
								name += p_header;
							}
						}

						if (! name.IsEmpty())
						{
							//
							// We can't use no multi-recipient addresses
							// as an auto-nickname, ya hear?
							//
							const char* p_end = ::FindAddressEnd(name);
							if (p_end && (*p_end == ','))
								name.Empty();
						}
					}
					else
					{
						//
						// Read message case.  Just look at the "From" header.
						//
						name = p_msgdoc->m_Sum->m_From;
					}

					if (! name.IsEmpty())
					{
						MakeAutoNickname(name);

						if (! name.IsEmpty())
							autoNickname = name;
					}
				}
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}

	if (autoNickname.IsEmpty())
		return FALSE;
	else
		return TRUE;
}


////////////////////////////////////////////////////////////////////////
// MakeNickname [extern]
//
// Returns TRUE if nicknames were successfully added to the database.
////////////////////////////////////////////////////////////////////////
BOOL MakeNickname()
{
 	//
 	// PART ONE.  The whole idea is to extract an optional "auto
 	// nickname" and optional list of addresses from the "source
 	// window".  The source window can either be a Wazoo window with
 	// keyboard focus or the topmost MDI child window.
 	//
 	char* pszAddresses = NULL;
 	CString strAutoNickname; // auto-generated default nickname, if any
 	BOOL DoAll = GetIniShort(IDS_INI_REPLY_TO_ALL);
 	BOOL bNicknameWndIsActive = FALSE;
	bool bWriteNotes          = true;
 	CMainFrame* pMainFrame = (CMainFrame *) ::AfxGetMainWnd();
 	ASSERT_KINDOF(CMainFrame, pMainFrame);

 	CWazooWnd* pWazooWnd = pMainFrame->GetWazooWindowWithFocus();
 	if (pWazooWnd) 	{
	    // Check to see if the Wazoo window is a valid source of
	    // nickname info.
	    DirectoryServicesWazooWndNew *pDSWazooWnd
		= DYNAMIC_DOWNCAST(DirectoryServicesWazooWndNew, pWazooWnd);
	    if (pDSWazooWnd) {
		// If Make Nickname is being done on a *single*, selected item
		// in the PH window, then pre-populate the "new name" field of
		// the dialog box with a stripped down version of the name.
		// Don't bother with any of this if there are multiple
		// selected items.
		//
		// Ask the DirectoryServicesWazooWndNew object for a
		// list of selected e-mail addresses, assumed to be of
		// the form:
		//
		//      "Real Name" <name@domain.com>
		CStringList strAddressList;
		if (pDSWazooWnd->GetSelectedAddresses(strAddressList) && 
		    (strAddressList.GetCount() == 1)) {
		    strAutoNickname = strAddressList.RemoveHead();
		    MakeAutoNickname(strAutoNickname);
		}

		strAddressList.RemoveAll();

		// Ask the DirectoryServicesWazooWndNew object for a list
		// of selected e-mail addresses.
		if (pDSWazooWnd->GetSelectedAddresses(strAddressList))
		{
		    if (strAddressList.GetCount() > 1) {
			bWriteNotes = false;
		    }
		    // Convert the string list into a single string buffer.  We
		    // do this in two passes.  First, we need to count up how
		    // much space the buffer needs to hold all the nicknames
		    // (plus CRLFs after each one).  Then, we copy the strings
		    // into the buffer, separated by CRLF pairs.
		    int Length = 0;
		    POSITION pos = strAddressList.GetHeadPosition();
		    while (pos != NULL) {
			CString strAddress = strAddressList.GetNext(pos);
			Length += strAddress.GetLength() + 2;
		    }
		    if (!Length || !(pszAddresses = new char[Length + 1]))
			return FALSE;
		    *pszAddresses = 0;
				
		    // Now, we do a second pass to copy the addresses
		    // into the buffer we just allocated.
		    char* a = pszAddresses;
		    while (! strAddressList.IsEmpty()) {
			CString strAddress = strAddressList.RemoveHead();
			if (a != pszAddresses) {
			    *a++ = '\r';
			    *a++ = '\n';
			}
			strcpy(a, strAddress);
                        // better late than never
			ASSERT(int(strlen(pszAddresses)) <= Length);

			a += strAddress.GetLength();
		    }
		    *a = 0;
		}

		// HACK ALERT...  The Evil Goto(tm).
		goto PART3;
	    }

 		CNicknamesWazooWnd* pNicknamesWazooWnd = DYNAMIC_DOWNCAST(CNicknamesWazooWnd, pWazooWnd);
 		if (pNicknamesWazooWnd)
 		{
 			//
 			// Set flag used later to determine whether or not to 
 			// auto-save the newly-added nicknames.
 			//
 			bNicknameWndIsActive = TRUE;

 			//
 			// Fetch a list of selected CNickname objects from the view.
 			//
 			ASSERT(g_Nicknames);
 			CObList nickname_list;
 			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_FETCH_SELECTED_NICKNAMES, &nickname_list);

 			// Count up how much space the buffer needs to hold all the nicknames
 			// (plus CRLFs after each one)
 			int nLength = 0;
 			POSITION pos = nickname_list.GetHeadPosition();
 			while (pos != NULL)
 			{
 				CNickname* pNickname = (CNickname *) nickname_list.GetNext(pos);
 				ASSERT(pNickname != NULL);
 				ASSERT_KINDOF(CNickname, pNickname);
 				nLength += pNickname->GetName().GetLength() + 2;
 			}
 			if (!nLength || !(pszAddresses = new char[nLength + 1]))
 				return FALSE;
 			*pszAddresses = 0;
			
 			// Now stick the names in to the buffer
 			char* a = pszAddresses;
 			while (! nickname_list.IsEmpty())
 			{
 				CNickname* pNickname = (CNickname *) nickname_list.RemoveHead();
 				if (a != pszAddresses)
 				{
 					*a++ = '\r';
 					*a++ = '\n';
 				}
 				strcpy(a, pNickname->GetName());
 				a += pNickname->GetName().GetLength();
 				ASSERT(int(strlen(pszAddresses)) <= nLength);	// better late than never
 			}
 			*a = 0;

 			//
 			// HACK ALERT...  The Evil Goto(tm).
 			//
 			goto PART3;
 		}

 		//
 		// If we get this far, the Wazoo window is not a source of
 		// nickname info, so fall back on the topmost MDI child.
 		//
 	}

 	//
 	// PART TWO.  No usable Wazoo, so use topmost MDI child.
 	//
 	{
 		UINT WindowID = GetTopWindowID();
 		switch (WindowID)
 		{
 		case IDR_TOC:
 		case IDR_READMESS:
 		case IDR_COMPMESS:
 			break;
 		default:
// 			ASSERT(0);
 			return (FALSE);
 		}

 		ASSERT(g_Nicknames);
			
 		//
 		// To avoid a weird race condition with the nicknames child
 		// window getting activated by the CNameNickDialog, we cache
 		// the active child window doc/view pair here, BEFORE we run
 		// CNameNickDialog.  If we don't do this, then
 		// CFrameWnd::GetActiveFrame() can return the nickname child
 		// window when we expected another child window.
 		//
 		CDocument* p_active_doc = pMainFrame->GetActiveFrame()->GetActiveDocument();
 		CView* p_active_view = pMainFrame->GetActiveFrame()->GetActiveView();

 		// If Make Nickname is being done on selected nicknames in the Nicknames window,
 		// then build the list ahead of time because the Make Nickname dialog will change
 		// the selection in the list box
 		switch (WindowID)
 		{
 		case IDR_TOC:
 			{
 				GetAutoNicknameFromWindow(WindowID, strAutoNickname);

 				CTocDoc* Toc;
 				CTocView* TocView;
 				if ((Toc = (CTocDoc*) p_active_doc) &&
 					(TocView = Toc->GetView()))
 				{
 					ASSERT(Toc->IsKindOf(RUNTIME_CLASS(CTocDoc)));
 					ASSERT(TocView->IsKindOf(RUNTIME_CLASS(CTocView)));
 					int SelCount = TocView->m_SumListBox.GetSelCount();
 					int TotalCount = TocView->m_SumListBox.GetCount();
				
 					for (int i = 0; SelCount && i < TotalCount; i++)
 					{
 						if (TocView->m_SumListBox.GetSel(i) > 0)
 						{
 							CSummary* Sum = TocView->m_SumListBox.GetItemDataPtr(i);
 							if (Sum)
 								AddMessageAddresses(Sum, pszAddresses, DoAll);
 							SelCount--;
 						}
 					}
 				}
 			}
 			break;
 		case IDR_READMESS:
 		case IDR_COMPMESS:
 			{
 				GetAutoNicknameFromWindow(WindowID, strAutoNickname);

 				CMessageDoc* MsgDoc = (CMessageDoc *) p_active_doc;
 				if (MsgDoc)
 				{
 					ASSERT(MsgDoc->IsKindOf(RUNTIME_CLASS(CMessageDoc)));
 					AddMessageAddresses(MsgDoc->m_Sum, pszAddresses, DoAll);
 				}
 			}
 			break;
 		default:
 			ASSERT(0);
 			return FALSE;
 		}
 	}

	//
	// At this point, 'pszAddresses' points to the list of addresses
	// extracted from a source window.  If we don't include ourselves
	// in the replies, then strip addresses that intersect with the
	// "me" nickname from the address list.
	//
	if (pszAddresses && (strlen(pszAddresses) > 0))
	{
 		if (GetIniShort(IDS_INI_INCLUDE_SELF) == FALSE)
 			StripMe(pszAddresses);
	}

	//
	// Now, check to see if the address list is empty or not.  If it
	// is empty, then give the user a chance to bail.
	//
	if (pszAddresses && (strlen(pszAddresses) == 0))
	{
		if (IDNO == ::AfxMessageBox(CRString(IDS_MAKE_EMPTY_NICKNAME), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
		{
			// user wants to bail...
			delete [] pszAddresses;
			return FALSE;
		}
	}

 	//
 	// PART THREE.  If we get this far, 'strAutoNickname' contains the
 	// optional auto nickname and 'pszAddresses' points to the optional
 	// list of nicknames.
 	//
 PART3:
 	while (1)
 	{
 		CNameNickDialog dlg(ID_SPECIAL_MAKENICKNAME);

 		dlg.m_NewName = strAutoNickname;		// can be empty
		
 		if (dlg.DoModal() == IDCANCEL || !dlg.m_pNicknameFile)
 			break;

 		if (!pszAddresses)
 			break;

 		CNickname nickname(dlg.m_NewName);
 		ASSERT(nickname.HasValidName());
	
 		nickname.SetAddresses(pszAddresses);
 		nickname.SetAddressesLength(nickname.GetAddresses().GetLength());

		
 		if (bWriteNotes == true) {
 			//
 			// If we have access to the "Real Name", then format it properly
 			// and stuff it into the Notes field of the nickname.
 			//
 			CString real_name(pszAddresses);

 			//
 			// We can't use no multi-recipient addresses
 			// as an auto-nickname, ya hear?
 			//
 			const char* p_end = ::FindAddressEnd(real_name);
 			if (p_end && (*p_end != ','))
 			{
 				::GetRealName(real_name.GetBuffer(real_name.GetLength()));
 				real_name.ReleaseBuffer();
 				CString field;
 				field.Format("<%s:%s>", (const char *) CRString(IDS_NICK_FIELD_NAME), (const char *) real_name);
 				nickname.SetRawNotes(field);
 				nickname.SetRawNotesLength(nickname.GetRawNotes().GetLength());
 			}
 		}

 		if (! dlg.m_pNicknameFile)
 		{
 			ASSERT(0);
 			return FALSE;
 		}

 		CNickname* p_new_nickname = dlg.m_pNicknameFile->AddNickname(nickname, dlg.m_PutOnRecipientList);
 		if (p_new_nickname)
 		{
 			dlg.m_pNicknameFile->SetModified();

 			// Notify everyone that there were some changes
 			//FORNOWg_Nicknames->SetModifiedFlag();

 			//
 			// Do the auto-save behavior for the Make Nickname command,
 			// if the Nicknames window is NOT the active window.
 			//
 			if (! bNicknameWndIsActive)
 				g_Nicknames->OnSaveDocument(NULL);

 			//
 			// If the Nicknames window is open, force the RHS view to
 			// update.  The NICKNAME_WAS_CHANGED message is a kludge
 			// to force the single-selection back to an existing
 			// nickname item.  It has nothing to do with renaming
 			// nicknames.
 			//
 			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_NICKNAME_WAS_CHANGED, p_new_nickname);
 			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_DISPLAYNICKNAME, p_new_nickname);

 			delete [] pszAddresses;
 			return (TRUE);
 		}
 		else
 			break;		// user canceled, or ran out of memory
 	}

 	delete [] pszAddresses;
 	return (FALSE);
}

////////////////////////////////////////////////////////////////////////
// NicknameFileExists [extern]
//
// Returns TRUE if the user-defined nickname file exists in the user's
// nicknames folder.
////////////////////////////////////////////////////////////////////////
BOOL NicknameFileExists(const CString& nickFilename)
{
	CString pathname(EudoraDir);
	pathname += CRString(IDS_NICK_DIR_NAME);
	pathname += SLASHSTR;
	pathname += nickFilename;

	return ::FileExistsMT(pathname);
}

////////////////////////////////////////////////////////////////////////
// CreateNewNicknameFile [extern]
//
// Creates a new, empty nickname file in the user's nickname folder.
////////////////////////////////////////////////////////////////////////
BOOL CreateNewNicknameFile(const CString& newFilename)
{
	// Shareware: Reduced feature mode only allows one nickname file, so no CChooseNicknameFileDlg
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE
		ASSERT(0);
		return (FALSE);
	}

	CString pathname(EudoraDir);
	pathname += CRString(IDS_NICK_DIR_NAME);
	pathname += SLASHSTR;
	pathname += newFilename;

	// Create the nickname file
	JJFile newfile;
	if (FAILED(newfile.Open(pathname, O_WRONLY | O_CREAT)))
		return FALSE;
	newfile.Close();

	//
	// Add the new nickname file to the collection of nickname files
	// maintained by CNicknamesDoc.  Then, refresh the nicknames view.
	// 
	if (g_Nicknames)
	{
		CNicknameFile* p_nickfile = g_Nicknames->AddNicknameFile(pathname);

		if (p_nickfile != NULL)
		{
			//
			// Update the view.
			//
			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_ADDED_NEW_NICKFILE, p_nickfile);
			return TRUE;
		}
	}
	return FALSE;
}


