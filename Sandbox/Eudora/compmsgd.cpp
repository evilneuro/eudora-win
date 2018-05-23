// COMPMSGD.CPP
//

#include "stdafx.h"

#include <afxcmn.h>
#include <QCUtils.h>

#include "resource.h"
#include "rs.h"
#include "helpxdlg.h"
#include "doc.h"
#include "cursor.h"
#include "fileutil.h"
#include "summary.h"
#include "msgdoc.h"
#include "compmsgd.h"
#include "mdichild.h"
#include "CompMessageFrame.h"
#include "font.h"
#include "bmpcombo.h"
#include "3dformv.h"
#include "header.h"		// for MIME.H
#include "mime.h"		// for TextReader class
#include "headervw.h"
#include "msgframe.h"
#include "tocdoc.h"
#include "utils.h"
#include "guiutils.h"
#include "address.h"
#include "eudora.h"
#include "progress.h"
#include "sendmail.h"
#include "changeq.h"
#include "mainfrm.h"
#include "filtersd.h"
#include "tocview.h"
#include "SaveAsDialog.h"
#include "msgutils.h"
#include "pop.h"
#include "password.h"
#include "persona.h"
#include "msgopts.h"

#include "PgMsgView.h"
#include "PgCompMsgView.h"
#include "Text2Html.h"

#include "ems-wglu.h"
#include "trnslate.h"  

#include "helpcntx.h"
#include "utils.h"
#include "nickdoc.h"
#include "QCWorkerSocket.h"
#include "NewMbox.h"
#include "QCProtocol.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPluginCommand.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"
#include "QCStationeryCommand.h"
#include "QCStationeryDirector.h"
#include "QCPersonalityCommand.h"
#include "QCPersonalityDirector.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "Automation.h"
#include "AutoCompleteSearcher.h"
#include "QCSharewareManager.h"

#define DIM( a ) ( sizeof( a ) / sizeof( a[0] ) )

extern CString EudoraDir;
extern QCCommandStack		g_theCommandStack;
extern QCMailboxDirector	g_theMailboxDirector;
extern QCRecipientDirector	g_theRecipientDirector;
extern QCStationeryDirector	g_theStationeryDirector;
extern QCPersonalityDirector g_thePersonalityDirector;
extern QCPluginDirector g_thePluginDirector;

#ifdef _DEBUG
#undef THIS_FILE
#ifndef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#endif
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CCompMessageDoc

IMPLEMENT_DYNCREATE(CCompMessageDoc, CMessageDoc)

CCompMessageDoc::CCompMessageDoc() :
	m_FindHeaderIndex(-1)			// negative value indicates that no Find actions have been executed
{
	m_HasBeenSaved = FALSE;
	m_StationeryApplied = FALSE;
	m_MessageId = "";	// not really required, but...
	m_bIsStationery = FALSE;
}

BOOL CCompMessageDoc::InitializeNew
(
	const char* To,	
	const char* From, 
	const char* Subject,
	const char* Cc, 
	const char* Bcc, 
	const char* Attachments, 
	const char* Body,
	const char* ECHeaders
)
{
	m_Headers[HEADER_TO] = To;
	m_Headers[HEADER_FROM] = From? From : GetReturnAddress();
	m_Headers[HEADER_SUBJECT] = Subject;
	m_Headers[HEADER_CC] = Cc;
	m_Headers[HEADER_BCC] = Bcc;
	m_Headers[HEADER_ATTACHMENTS] = Attachments;

	m_HeadersInvalidFlag[HEADER_TO] = FALSE;
	m_HeadersInvalidFlag[HEADER_FROM] = FALSE;
	m_HeadersInvalidFlag[HEADER_SUBJECT] = FALSE;
	m_HeadersInvalidFlag[HEADER_CC] = FALSE;
	m_HeadersInvalidFlag[HEADER_BCC] = FALSE;
	m_HeadersInvalidFlag[HEADER_ATTACHMENTS] = FALSE;
	

	SetText(Body);

	AssignMessageId();	// each message gets a unique id
	m_NextAutoSaveTime = time(NULL);  //get the Creation time stamp
	m_ExtraHeaders.Format("%s<%s>\r\n", (LPCTSTR)CRString(IDS_HEADER_MESSAGE_ID), (LPCTSTR)m_MessageId);

	m_QCMessage.Init(m_MessageId, m_Text, FALSE);
	if ( ECHeaders )
	{
		m_QCMessage.InitMap( ECHeaders );
		m_QCMessage.NukeCIDs();
	}

    // Take out attachments that are not found
	if (!IsHeaderEmpty(HEADER_ATTACHMENTS))
	{
        BOOL MissingFile = FALSE; 
		char* a = m_Headers[HEADER_ATTACHMENTS].GetBuffer(m_Headers[HEADER_ATTACHMENTS].GetLength());
        char* semi;
		BOOL  foundSemi = FALSE;
		while(strlen(a) > 0)  //while (1)
		{
			if ( semi = strchr(a, ';') )
			{
				*semi = 0;
				foundSemi = TRUE;
			}
			char* filename = a;
			while (*filename == ' ' || *filename == '\t')
				filename++;
			if (!::FileExistsMT(filename))
			{
				MissingFile = TRUE;
				if (foundSemi)
					strcpy(a, semi + 1);
				else
					break;
			}
			else
			{
				if (foundSemi)
				{
					*semi = ';';
					a = semi + 1;
				}
				else
					break;
            }
			
			foundSemi = FALSE;
		}

		m_Headers[HEADER_ATTACHMENTS].ReleaseBuffer();
		if (MissingFile)
        	ErrorDialog(IDS_ERR_MISSING_ATTACHMENT);
	}

	m_Sum = new CSummary;
	CTocDoc* OutToc;

	if (!m_Sum || !(OutToc = GetOutToc()))
		return (FALSE);
		
	OutToc->AddSum(m_Sum);
		
	if (!IsHeaderEmpty(HEADER_TO))
	{
		char buf[128];
		strncpy(buf, GetHeaderLine(HEADER_TO), sizeof(buf));
		buf[sizeof(buf) - 1] = 0;
		m_Sum->SetFrom(GetRealName(buf));
	}
	m_Sum->SetSubject(GetHeaderLine(HEADER_SUBJECT));
	m_Sum->m_State = MS_UNSENDABLE;
	ReallySetTitle(m_Sum->MakeTitle());

	if (GetIniShort(IDS_INI_SEND_MIME))		m_Sum->SetFlag(MSF_MIME);
	if (GetIniShort(IDS_INI_SEND_UUENCODE))	m_Sum->SetFlag(MSF_UUENCODE);
	if (GetIniShort(IDS_INI_USE_QP))		m_Sum->SetFlag(MSF_QUOTED_PRINTABLE);
	if (GetIniShort(IDS_INI_WORD_WRAP))		m_Sum->SetFlag(MSF_WORD_WRAP);
	if (GetIniShort(IDS_INI_TABS_IN_BODY))	m_Sum->SetFlag(MSF_TABS_IN_BODY);
	if (GetIniShort(IDS_INI_KEEP_COPIES))	m_Sum->SetFlag(MSF_KEEP_COPIES);
	if (GetIniShort(IDS_INI_TEXT_AS_DOC))	m_Sum->SetFlag(MSF_TEXT_AS_DOC);

	switch (IsFancy(Body))
	{
	case IS_FLOWED:
		m_Sum->SetFlagEx(MSFEX_FLOWED);
		break;
	case IS_HTML:
		m_Sum->SetFlagEx(MSFEX_HTML);
		// fall through
	case IS_RICH:
		m_Sum->SetFlag(MSF_XRICH);
		break;
	}

	RemoveIniFromCache(IDS_INI_DEFAULT_TRANSLATOR);
	CString defINITrans = GetIniString(IDS_INI_DEFAULT_TRANSLATOR); 
	
	CString defFlaggedTrans; 
	((CEudoraApp *)AfxGetApp())->GetTranslators()->GetDefOutTransString(defFlaggedTrans); 

	if (!defINITrans.IsEmpty() && !defFlaggedTrans.IsEmpty())
		defINITrans += ',';
	defINITrans += defFlaggedTrans;
	
	if (!defINITrans.IsEmpty())
	{
		defINITrans = "<" + defINITrans;
		defINITrans += ">";
		m_Sum->SetTranslators(defINITrans, TRUE);
	}

	return (TRUE);
}

BOOL CCompMessageDoc::OnNewDocument()
{
	if (!CMessageDoc::OnNewDocument())
		return (FALSE);

	return (TRUE);
}

CCompMessageDoc::~CCompMessageDoc()
{
	if ( m_bIsStationery )
	{
		CTocDoc* OutToc = GetOutToc();
		CSummary* Sum = m_Sum;
		if (Sum)
		{
			// Remove the link between the message doc and the summary because otherwise
			// deleting the summary (done in RemoveSum()) will cause the message doc to
			// be deleted.  And look where we are?  The destructor of the message doc!
			// That's a double delete, my friend, and that will make you crash and burn!
			m_Sum = NULL;
			OutToc->RemoveSum(Sum);
		}
	}
}


void CCompMessageDoc::SetTitle(const char* pszTitle)
{
	if (!m_bIsStationery)
	{
		// The title should only have to be set once.  This prevents screen flicker when saving.
		if (m_strTitle.IsEmpty())
			CDocument::SetTitle(pszTitle);
	}
	else
	{
		CString title;

		int filenameStart = m_strPathName.ReverseFind(SLASH);
		if (filenameStart > 0)
		{
			//Strip the Path
			title = m_strPathName.Right(m_strPathName.GetLength() - filenameStart - 1);
			//Strip the Extension
			if ( title.ReverseFind('.') > 0 )
				title = title.Left(title.GetLength() - 4);
		}
		else 
			title = CRString(IDS_UNTITLED);

		title += " - ";
		title += CRString(IDR_STATIONERY);

		CDocument::SetTitle(title);
	}
}


// BADSTUFF: this routine has been badly hacked to "know" about Paige message
// views, but still return a CCompMessageView. Hopefully this will die before
// I have to check-in this module, but ya never know.
CView* CCompMessageDoc::GetView()
{
	// Note: this routine is symptomatic of a general architectural problem
	// with our "design". A document should not care about the types of
	// views that are attached to it.

	POSITION pos = GetFirstViewPosition();
	while ( pos ) {
		CView* pCV = GetNextView( pos );
		VERIFY( pCV );
		BOOL isCV = pCV->IsKindOf( RUNTIME_CLASS(PgMsgView) );

		if ( isCV )
			return pCV;
	}
		
	return NULL;
}

CView* CCompMessageDoc::GetCompView()
{
	//This routine currently only returns the PgCompMsgView
	//Don't try to use this to get a *preview* View

	POSITION pos = GetFirstViewPosition();
	while ( pos ) {
		CView* pCV = GetNextView( pos );
		VERIFY( pCV );
		//BOOL isHV = pCV->IsKindOf( RUNTIME_CLASS(CHeaderView) );
		BOOL isCV = pCV->IsKindOf( RUNTIME_CLASS(PgCompMsgView) );
		//if ( !isHV )
		if ( isCV )
			return pCV;
	}
		
	return NULL;
}


CHeaderView* CCompMessageDoc::GetHeaderView()
{
	// Note: this routine is symptomatic of a general architectural problem
	// with our "design". A document should not care about the types of
	// views that are attached to it.

	POSITION pos = GetFirstViewPosition();
	while ( pos ) {
		CView* pHV = GetNextView( pos );
		VERIFY( pHV );
		BOOL isHV = pHV->IsKindOf( RUNTIME_CLASS(CHeaderView) );

		if ( isHV )
			return (CHeaderView*) pHV;
	}
		
	return NULL;
}

BOOL CCompMessageDoc::AddAttachment( const char* Filename )
{
	VERIFY( Filename );
	CHeaderView* pHV = GetHeaderView();
	pHV->AddAttachment( Filename );

	return TRUE;
}


BOOL CCompMessageDoc::SaveModified()
{
	if (m_bIsStationery)
	{
		if ( IsModified() )
		{
			//Hasn't ever been saved before
			if ( m_strPathName.Compare("A") == 0 )
			{
				CRString prompt(IDS_SAVE_STATIONERY_CHANGES);
				switch (AfxMessageBox(prompt, MB_YESNOCANCEL))
				{
					case IDCANCEL:
						return (FALSE);
					case IDYES:
						OnFileSaveAsStationery();

						//If the user chose cancel in the SaveAs dlg, dont' close the window
						if ( m_strPathName.Compare("A") == 0 )
							return (FALSE);
						
						break;
					case IDNO:
						SetModifiedFlag(FALSE);
						break;
				}
			}
			else
			{
				//Resave/Update the document??
				CString prompt;
				AfxFormatString1(prompt, IDS_SAVE_CHANGES, m_strPathName);
				switch (AfxMessageBox(prompt, MB_YESNOCANCEL))
				{
					case IDCANCEL:
						return (FALSE);       // don't continue

					case IDYES:
						{
						JJFile theFile;

						if (FAILED(theFile.Open( m_strPathName, O_CREAT | O_TRUNC | O_WRONLY)))
						return (FALSE);
					
						CMessageDoc::OnFileSave();
						WriteAsText( &theFile, TRUE );

						// If so, either Save or Update, as appropriate
						//if (!DoSave(m_strPathName))
						//	return (FALSE);       // don't continue
						}
						break;
					
					case IDNO:
						// If we get here, it may be the case that the user hit
						// No when asked to save the changes.  If this is so, then the modified
						// flag needs to be cleared so the save prompt doesn't come up again.
						SetModifiedFlag(FALSE);
						//return (FALSE);
						break;

					default:
						ASSERT(FALSE);
						return (FALSE);       // don't continue
				}
			}
		}

		//Except for the case of cancel, close the window and 
		//remove the summary from the Out Toc
		//CTocDoc* OutToc = GetOutToc();

		OnCloseDocument();

		//if ( !m_Sum || !(OutToc) )
		//	OutToc->RemoveSum(m_Sum);


		//Closed and deleted from the outbox, so don't continuue
		//OnMessageDelete();

		return FALSE;	
	}

	//For other comp messages
	if (IsModified())
	{
		if (CMessageDoc::SaveModified() == FALSE)
			return (FALSE);

		if (m_HasBeenSaved == FALSE)
		{
			//
			// If we get this far, then the user has chosen to trash this
			// message.  So, now we need to check whether there are any
			// MAPI attachments.  If the user's MAPI settings are such
			// deleting MAPI attachments is allowed, then blow away each
			// MAPI attachment file.
			//
			if (m_Sum->IsMAPI())
			{
				GetText();		// make sure body is read up
				CString attachments(GetHeaderLine(HEADER_ATTACHMENTS));
				if (! attachments.IsEmpty())
				{
					char attachments_dir[_MAX_PATH + 1];
					GetIniString(IDS_INI_AUTO_RECEIVE_DIR, attachments_dir, sizeof(attachments_dir));
					if (attachments_dir[0] == '\0')
						wsprintf(attachments_dir,"%s%s", (const char *) EudoraDir, (const char *) CRString(IDS_ATTACH_FOLDER));
					const int ATTACHMENTS_DIR_LEN = strlen(attachments_dir);

					int idx;
					while ((idx = attachments.Find(';')) != -1)
					{
						//
						// If the attachment file is in the Eudora attachments
						// directory, then delete the file.
						//
						CString attachment(attachments.Left(idx));
						if (strnicmp(attachments_dir, attachment, ATTACHMENTS_DIR_LEN) == 0)
							::FileRemoveMT(attachment);

						//
						// Removed processed attachment filename by shifting string.
						//
						attachments = attachments.Right(attachments.GetLength() - (idx + 1));
					}

					//
					// Delete the last file in the list, if it is in the Eudora
					// attachments directory.
					//
					if (! attachments.IsEmpty())
					{
						if (strnicmp(attachments_dir, attachments, ATTACHMENTS_DIR_LEN) == 0)
							::FileRemoveMT(attachments);
					}
				}
			}

			// what he said, but for the .msg command line stuff
			if ( m_Sum->IsAutoAttached() )
			{
				// extract the attachment filename
				CString AttachStr(GetHeaderLine(HEADER_ATTACHMENTS));
				char* Attach = AttachStr.GetBuffer(AttachStr.GetLength());
			
				while (Attach && *Attach)
				{
					char *t = strchr(Attach, ';');
					if (t)
					{
						*t++ = 0;
						if (*t == ' ')
							t++;
					}

					// expects a fully-qualified filename
					::FileRemoveMT( Attach );

					Attach = t;
				}
			}
		}
	}
		
	return (TRUE);
}


/////////////////////////////////////////////////////////////////////////////
// CSendStylesDialog dialog

class CSendStylesDialog : public CDialog
{
// Construction
public:
	CSendStylesDialog(CWnd* pParent = NULL);   // standard constructor

	virtual BOOL OnInitDialog();

// Dialog Data
	//{{AFX_DATA(CSendStylesDialog)
	enum { IDD = IDD_SENDING_STYLES };
	CButton	m_SendPlainAndStyled;
	CButton	m_WarnCheckbox;
	CButton	m_SendStyled;
	CButton	m_SendPlain;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendStylesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSendStylesDialog)
	afx_msg void OnSendPlain();
	afx_msg void OnSendPlainAndStyled();
	afx_msg void OnSendStyled();
	afx_msg void OnButtonPress(UINT ButtonID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CSendStylesDialog dialog


CSendStylesDialog::CSendStylesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSendStylesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSendStylesDialog)
	//}}AFX_DATA_INIT

	// We better not be creating this dialog if the warning is off
	ASSERT(GetIniShort(IDS_INI_WARN_QUEUE_STYLED_TEXT));
}

BOOL CSendStylesDialog::OnInitDialog()
{
	if (!CDialog::OnInitDialog())
		return FALSE;

	if (GetIniShort(IDS_INI_SEND_PLAIN_ONLY))
	{
		m_SendPlain.SetFocus();
		SetDefID(IDC_SEND_PLAIN);
	}
	else if (GetIniShort(IDS_INI_SEND_STYLED_ONLY))
	{
		m_SendStyled.SetFocus();
		SetDefID(IDC_SEND_STYLED);
	}
	else
	{
		// This better be set if the above two aren't
		ASSERT(GetIniShort(IDS_INI_SEND_PLAIN_AND_STYLED));
		m_SendPlainAndStyled.SetFocus();
		SetDefID(IDC_SEND_PLAIN_AND_STYLED);
	}

	// We set the focus, so return FALSE
	return FALSE;
}

void CSendStylesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendStylesDialog)
	DDX_Control(pDX, IDC_SEND_PLAIN_AND_STYLED, m_SendPlainAndStyled);
	DDX_Control(pDX, IDC_WARN_CHECKBOX, m_WarnCheckbox);
	DDX_Control(pDX, IDC_SEND_STYLED, m_SendStyled);
	DDX_Control(pDX, IDC_SEND_PLAIN, m_SendPlain);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSendStylesDialog, CDialog)
	//{{AFX_MSG_MAP(CSendStylesDialog)
	ON_COMMAND_EX(IDC_SEND_PLAIN, OnButtonPress)
	ON_COMMAND_EX(IDC_SEND_STYLED, OnButtonPress)
	ON_COMMAND_EX(IDC_SEND_PLAIN_AND_STYLED, OnButtonPress)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendStylesDialog message handlers

void CSendStylesDialog::OnButtonPress(UINT ButtonID)
{
	if (m_WarnCheckbox.GetCheck())
	{
		// A little verbose, but it prevents a setting from being turned off, then on again,
		// which would cause a default setting to be written to the .INI file
		switch (ButtonID)
		{
		case IDC_SEND_PLAIN:
			SetIniShort(IDS_INI_SEND_PLAIN_ONLY,		TRUE);
			SetIniShort(IDS_INI_SEND_STYLED_ONLY,		FALSE);
			SetIniShort(IDS_INI_SEND_PLAIN_AND_STYLED,	FALSE);
			break;
		case IDC_SEND_STYLED:
			SetIniShort(IDS_INI_SEND_PLAIN_ONLY,		FALSE);
			SetIniShort(IDS_INI_SEND_STYLED_ONLY,		TRUE);
			SetIniShort(IDS_INI_SEND_PLAIN_AND_STYLED,	FALSE);
			break;
		case IDC_SEND_PLAIN_AND_STYLED:
			SetIniShort(IDS_INI_SEND_PLAIN_ONLY,		FALSE);
			SetIniShort(IDS_INI_SEND_STYLED_ONLY,		FALSE);
			SetIniShort(IDS_INI_SEND_PLAIN_AND_STYLED,	TRUE);
			break;
		default:
			// This better not happpen
			ASSERT(0);
			break;
		}

		SetIniShort(IDS_INI_WARN_QUEUE_STYLED_TEXT, FALSE);
	}

	EndDialog(ButtonID);
}


// BadRecipient
//
// Tells whether or not there is a problem with a particular
// recipient or there are no recipients at all.
//
BOOL CCompMessageDoc::BadRecipient()
{
	char* Expanded;
	BOOL bIsEmpty = TRUE;

	Expanded = ExpandAliases(GetHeaderLine(HEADER_TO), TRUE, TRUE, TRUE, TRUE);
	if (!Expanded)
		return TRUE;
	bIsEmpty = bIsEmpty && !*Expanded;
	delete [] Expanded;

	Expanded = ExpandAliases(GetHeaderLine(HEADER_CC), TRUE, TRUE, TRUE, TRUE);
	if (!Expanded)
		return TRUE;
	// Can't send with just Cc: recipient, so don't update bIsEmpty
	delete [] Expanded;

	Expanded = ExpandAliases(GetHeaderLine(HEADER_BCC), TRUE, TRUE, TRUE, TRUE);
	if (!Expanded)
		return TRUE;
	bIsEmpty = bIsEmpty && !*Expanded;
	delete [] Expanded;

	if (bIsEmpty)
	{
		if (!gbAutomationCall)
		{
			// BOG: this is a hack for queueing through MAPI; it really ain't
			// the way to go in the long run, but...
			AfxGetMainWnd()->SetForegroundWindow();
			// EndHack

			ErrorDialog(IDS_ERR_COMP_NEED_RCPT);
		}

		return TRUE;
	}

	return FALSE;
}

BOOL CCompMessageDoc::Queue(BOOL autoSend /*= FALSE*/)
{
    // BADSTUFF
    CView* View = GetView();

    if ( !m_Sum || m_Sum->CantEdit() ) {
        ASSERT( FALSE );
        return FALSE;
    }

    // Spell check if necessary
    if ( View && !autoSend && GetIniShort( IDS_INI_SPELL_ON_QUEUE ) ) {
        QCProtocol*     pProtocol = QCProtocol::QueryProtocol( QCP_SPELL, ( CObject* )View );

        if( pProtocol == NULL ) 
            return FALSE;
                
        int ret = pProtocol->CheckSpelling( TRUE );

        if ( ret == IDCANCEL )
            return FALSE;

        if ( ret < 0 ) {
            if ( GetIniShort( IDS_INI_SPELL_ON_QUEUE_WARN ) ) {
                if ( !gbAutomationCall ) {
                    if ( WarnDialog( IDS_INI_SPELL_ON_QUEUE_WARN,
                                     IDS_WARN_SPELL_ON_QUEUE_FAIL ) != IDOK )

                        return FALSE;
                }
            }
        }
    }

	if (BadRecipient())
		return (FALSE);

	const char* Subject = GetHeaderLine(HEADER_SUBJECT);
	if (!autoSend && GetIniShort(IDS_INI_WARN_QUEUE_NO_SUBJECT) && (!Subject || !*Subject))
	{
		if (!gbAutomationCall)
		{
			if (WarnDialog(IDS_INI_WARN_QUEUE_NO_SUBJECT, IDS_WARN_QUEUE_NO_SUBJECT) != IDOK)
				return (FALSE);
		}
	}

	const char* Attachments = GetHeaderLine(HEADER_ATTACHMENTS);
	
	if (!autoSend && GetIniShort(IDS_INI_WARN_QUEUE_BIG_MESSAGE) )
	{
		long EstSize = ::SafeStrlenMT(GetText());	// 9/16 Changed from GetBody());

		if ( Attachments && *Attachments )
		{
			// add the size of the attachments

			while (1)
			{
				const char* End = strchr(Attachments, ';');
				char Filename[_MAX_PATH + 1];

				if (!End)
					End = Attachments + strlen(Attachments);
				while (*Attachments == ' ')
					Attachments++;
				int Length = End - Attachments;
				strncpy(Filename, Attachments, Length);
				Filename[Length] = 0;

				if (*Filename)
				{
					CString fc(Filename);
					char* FilenameCopy = fc.GetBuffer(fc.GetLength() + 1);
					CFileStatus Status;

					if (!CFile::GetStatus(FilenameCopy, Status))
					{
						ErrorDialog(IDS_ERR_OPEN_ATTACH, Filename);
						return (FALSE);
					}

					EstSize += ((Status.m_size * 4L) / 3L);
				}

				if (!*End)
					break;
				
				Attachments = End + 1;
			}
		}

		long BigSize = GetIniLong(IDS_INI_WARN_QUEUE_BIG_MESSAGE_SIZE) * 1024L;

		if (!gbAutomationCall)
		{
			if (EstSize > BigSize && WarnDialog(IDS_INI_WARN_QUEUE_BIG_MESSAGE,
				IDS_WARN_QUEUE_BIG_MESSAGE, (EstSize / 1024L)) != IDOK)
			{
				return (FALSE);
			}
		}
	}
	
	// Here we're going to look through the recipient headers and check them for names we can add to
	// Our AutoCompleter History list. We've previously turned on an expanded flag if the header has
	// undergone expansion. This is important because if you type a nickname and it's expanded the
	// names that were expanded out are not something you've typed and therefore shouldn't be added
	// to a list of "Typing Shortcuts"
	

	CHeaderView* pHView = GetHeaderView();
	CHeaderField* pHField = NULL;

	// We grab the addresses from each header and then put
	// them in the list if they should be put there.
/*	if (pHView)
	{
		CString addresses = "";

		pHField = pHView->GetHeaderCtrl(HEADER_TO);

		if (pHField && pHField->GetExpanded() == false)
		{// We have a candidate for name grabbing.
			addresses += pHField->GetText();
			addresses += ", ";
			pHField = NULL;
		}

		pHField = pHView->GetHeaderCtrl(HEADER_CC);

		if (pHField && pHField->GetExpanded() == false)
		{// We have a candidate for name grabbing.
			addresses += pHField->GetText();
			addresses += ", ";
			pHField = NULL;
		}

		pHField = pHView->GetHeaderCtrl(HEADER_BCC);

		if (pHField && pHField->GetExpanded() == false)
		{// We have a candidate for name grabbing.
			addresses += pHField->GetText();
		}

		bool done = false;
			
		while (!done)
		{
			CString oneaddress;
			int length = addresses.Find(',');
			if (length == -1)
			{
				done = true;
				if (addresses.GetLength() == 0)  
					break;

				oneaddress = addresses;
			}
			else
			{
				oneaddress = addresses.Left(length);
				addresses = addresses.Right(addresses.GetLength() - (length+1));
			}

			oneaddress.TrimLeft();
			oneaddress.TrimRight();

			char *nameToAdd = oneaddress.GetBuffer(0);

			if (g_AutoCompleter)
				g_AutoCompleter->Add(nameToAdd);

			oneaddress.ReleaseBuffer();
		}

	}
*/



	// Copy the text in the headers from the control to the document
//	if (View && IsModified())
//		View->SaveHeaders();

//	if (IsModified() && !OnSaveDocument(NULL))
	if(!OnSaveDocument(NULL))
		return (FALSE);
	
	// Here's the scoop behind the MSFEX_SEND_PLAIN and MSFEX_SEND_STYLED flags:
	//
	//   If both the message and the signature contain no styles, then both
	//   flags are unset.  If there are some styles present, then the two flags
	//   tell which versions to send.  So a flag being set means that there are
	//   some styles in the message, and that particular version (plain or styled)
	//   of the message should be sent.
	//

	// If we've saved a message with styles only because it has excerpts, then the MSFEX_SEND_PLAIN flag
	// will be set (and MSFEX_SEND_STYLED won't be set) so that we wind up just sending plain text.
	const BOOL bBodyExcerptOnly = (m_Sum->GetFlagsEx() & MSFEX_SEND_PLAIN) && !(m_Sum->GetFlagsEx() & MSFEX_SEND_STYLED);
	BOOL bSendStyles = FALSE;

	if (m_Sum->IsXRich() && !bBodyExcerptOnly)
		bSendStyles = TRUE;
	else
	{
		// If only the signature has styles, then IDS_INI_STYLED_SIG controls whether
		// or not a styled signature should get sent with styles or just plain.
		char* Sig = NULL;
		if (GetIniShort(IDS_INI_SEND_STYLED_SIG) &&
			(Sig = GetSignature(this, FALSE)) && IsFancy(Sig) >= IS_RICH)
		{
			bSendStyles = TRUE;
		}
		delete [] Sig;
	}

	if (!bSendStyles)
	{
		if (!bBodyExcerptOnly)
		{
			m_Sum->UnsetFlagEx(MSFEX_SEND_PLAIN);
			m_Sum->UnsetFlagEx(MSFEX_SEND_STYLED);
		}
	}
	else
	{
		if (!autoSend && GetIniShort(IDS_INI_WARN_QUEUE_STYLED_TEXT))
		{
			// Let user tell us what to send
			CSendStylesDialog dlg;
			switch (dlg.DoModal())
			{
			case IDCANCEL:
				return (FALSE);
			case IDC_SEND_PLAIN:
				m_Sum->SetFlagEx(MSFEX_SEND_PLAIN);
				m_Sum->UnsetFlagEx(MSFEX_SEND_STYLED);
				break;
			case IDC_SEND_STYLED:
				m_Sum->UnsetFlagEx(MSFEX_SEND_PLAIN);
				m_Sum->SetFlagEx(MSFEX_SEND_STYLED);
				break;
			case IDC_SEND_PLAIN_AND_STYLED:
				m_Sum->SetFlagEx(MSFEX_SEND_PLAIN);
				m_Sum->SetFlagEx(MSFEX_SEND_STYLED);
				break;
			default:
				// Should never get here...
				ASSERT(0);
			}
		}
		else
		{
			// Use setting to determine what to send
			if (GetIniShort(IDS_INI_SEND_PLAIN_AND_STYLED))
			{
				m_Sum->SetFlagEx(MSFEX_SEND_PLAIN);
				m_Sum->SetFlagEx(MSFEX_SEND_STYLED);
			}
			else if (GetIniShort(IDS_INI_SEND_STYLED_ONLY))
			{
				m_Sum->UnsetFlagEx(MSFEX_SEND_PLAIN);
				m_Sum->SetFlagEx(MSFEX_SEND_STYLED);
			}
			else
			{
				m_Sum->SetFlagEx(MSFEX_SEND_PLAIN);
				m_Sum->UnsetFlagEx(MSFEX_SEND_STYLED);
			}
		}
	}

	if (!m_Sum->GetTranslators().IsEmpty())
	{
		CTranslatorManager* trans = ((CEudoraApp *)AfxGetApp())->GetTranslators();
		if ( !trans->CanXLateMessageOut(this, EMSF_Q4_TRANSMISSION | EMSF_Q4_COMPLETION) )
			return (FALSE);
		//
		// Do we have any OnCompletion translators? 
		// If so, go ahead and translate the message now.
		//
		if (GetIniShort(IDS_INI_ALLOW_COMPLETION_PLUGINS) && trans->CanXLateMessageOut(this, EMSF_Q4_COMPLETION ) )
		{
			char cooked[_MAX_PATH];
			HRESULT hr = OnCompletionTranslate(cooked);
			if ( FAILED(hr) )
				return (FALSE);
			//
			// Delete attachments from header and attach newly
			// translated file (.msg).  Clear Body as well.
			//

			m_Sum->SetFlagEx(MSFEX_MIME_ATTACHED);
			m_Sum->SetFlagEx(MSFEX_AUTO_ATTACHED);
			AddAttachment(cooked);
			SetHeaderLine(HEADER_ATTACHMENTS, cooked);
			m_Sum->SetTranslators("");
			
			QCProtocol*	pProtocol = QCProtocol::QueryProtocol( QCP_TRANSLATE, ( CObject* )GetView() );
			if( pProtocol == NULL ) 
				return FALSE;
			
			pProtocol->SetAllText("", FALSE);
			if (!OnSaveDocument(NULL))
				return (FALSE);
			// make read only
		}
	}

// No need to do this anymore because saving the message sets the date/time
// of the summary.  That's because the Date: header should contain the last
// date/time the message was edited, not when it was queued/sent.
//
//	//If msg could not be sent, then it is queued with the current time again. 
//	//Since the send code changes the time zone minutes, we need to reset it.
//	m_Sum->m_Seconds = time(NULL);
//	m_Sum->m_TimeZoneMinutes = 0;
//	m_Sum->FormatDate();

	m_Sum->SetState(MS_QUEUED);

	// Close up message window
	if (m_Sum->m_FrameWnd)
	{
		// Close the window, but don't destroy the document because
		// we're going to need it later when we send the message
		m_bAutoDelete = FALSE;
		OnCloseDocument();
		m_bAutoDelete = TRUE;
	}

	SetQueueStatus();

	return (TRUE);
}

HRESULT CCompMessageDoc::OnCompletionTranslate(char* cookedMsg)
{
	HRESULT hr;
	char* ptr;
	int err;
	//
	// Create an object to encode the message and send its
	// MIME parts to the spool file.  
	//
	QCSMTPMessage SendMsg(this);
	CString raw = GetTmpFileNameMT();

	hr = SendMsg.Start(raw);
	if ( FAILED(hr) ) goto fail;
	
//	hr = SendMsg.WriteHeaders();
//	if ( FAILED(hr) ) goto fail;

	//
	//	Save MIME encoded body to temp file
	//

	hr= SendMsg.WriteBody();
	if ( FAILED(hr) ) goto fail;
	hr = SendMsg.End();  

	//	Translate it and store it in new file called COOKED.
	//
	char cooked[_MAX_PATH];
	*cooked = 0;
	err = GetTransMan()->XLateMessageOut(EMSF_Q4_TRANSMISSION | EMSF_Q4_COMPLETION |
												EMSF_WHOLE_MESSAGE | 
												EMSF_REQUIRES_MIME | 
												EMSF_GENERATES_MIME, 
												this, 
												raw, 
												cooked);
	DeleteFile(raw);
	if ( err != EMSR_OK )
	{
		DeleteFile(cooked);
		CloseProgress();
		return E_FAIL;
	}
	//
	// Rename the file from .tmp to .msg
	//
	strcpy(cookedMsg,cooked);
	strlwr(cookedMsg);
	ptr = strstr(cookedMsg,".tmp");
	if (ptr)
	{
		strcpy(ptr,".msg");
		hr = ::FileRenameMT(cooked,cookedMsg);
	}
	CloseProgress();
	
	return hr;

fail:	
	HRESULT hr2;
	hr2 = SendMsg.End();  // what to do about return value here...
	DeleteFile(raw);
	CloseProgress();

	return hr;

}

const char* CCompMessageDoc::SetHeaderLine(int i, const char* NewText)
{
	CHeaderView* View = NULL;

	if ( i == HEADER_CURRENT )
	{
		View = GetHeaderView();

		if (View)
    		i = View->GetCurrentHeader();

		if ( i < 0 || i >= MaxHeaders )
			return NULL;
	}

	ASSERT(i >= 0 && i < MaxHeaders);

	if ( i < NumHeaders )
	{
		if (View == NULL)
			View = GetHeaderView();

		if (View)
    		View->SetText(i, NewText);
	}
		
	return (m_Headers[i] = NewText);
}

const char* CCompMessageDoc::GetHeaderLine( int i )
{
	CHeaderView* pHView = NULL;

	if ( i == HEADER_CURRENT )
	{
		pHView = GetHeaderView();

		if (pHView)
    		i = pHView->GetCurrentHeader();
	}

	if ( i >= 0 && i < MaxHeaders )
	{
		if ( i < NumHeaders ) {
			if ( pHView == NULL )
				pHView = GetHeaderView();
  
			if ( pHView ) {
				m_Headers[i] = pHView->GetText( i );
			}
		}

		return ( m_Headers[i] );
	}
		
	ASSERT( 0 );
	return NULL;
}

BOOL CCompMessageDoc::IsHeaderEmpty(int i)
{
	const char* Header = GetHeaderLine(i);
	
	return (Header && *Header? FALSE : TRUE);
}

void CCompMessageDoc::SetHeaderAsInvalid(int nHeader, BOOL bInvalid)
{
	m_HeadersInvalidFlag[nHeader] = bInvalid;
}

BOOL CCompMessageDoc::Read()
{
	UINT	uLen;

	if (!CMessageDoc::Read())
		return (FALSE);

	//
	// m_ExtraHeaders is an additive variable so let's clean it out just
	// in case some residue is left.  The reason for this is because PgMsgView
	// chooses to nullify m_text to conserve memory.
	//
	m_ExtraHeaders="";

	// get the Translation X-Header
	char *Trans = HeaderContents(IDS_TRANS_XHEADER, m_Text);
	if (Trans)
	{
		m_Sum->SetTranslators(Trans);
		delete []Trans;
	}

	// get the Signature X-Header
	char *sigstr = HeaderContents(IDS_SIGNATURE_XHEADER, m_Text);
	if (sigstr)
	{
		char *cp = sigstr;

		if (*cp == '<' && *(cp + strlen(cp) - 1) == '>')
		{
			cp++;
			*(cp + strlen(cp) - 1) = '\0';
			m_Sum->m_SigSelected = m_Sum->m_SigHdr = cp;
		}
		delete[] sigstr;
	}

	// get the Persona X-Header
	char *Persona = HeaderContents(IDS_PERSONA_XHEADER, m_Text);
	if (Persona)
	{
		char *cp = Persona;

		if (*cp == '<' && *(cp + strlen(cp) - 1) == '>')
		{
			cp++;
			*(cp + strlen(cp) - 1) = '\0';
			m_Sum->SetPersona( cp );
		}
		delete [] Persona;
	}

	// get the Precedence Header
	char *prec = HeaderContents(IDS_PRECEDENCE_HEADER, m_Text);
	if (prec)
	{
		m_Sum->SetPrecedence(prec);
		delete [] prec;
	}

	// Go through the message and parcel out each item
	char* t = m_Text;
	while (t && *t != '\r' && *t != '\n' && *t)
	{
		char *	start = t;
		char *	end;
		BOOL	bConsumed = FALSE;

		// strip "\r\n"
		if ( end = strchr( t, '\n') )
		{
			if ( end[ -1 ] == '\r')
				end[ -1 ] = '\0';
			end[0] = '\0';
		}
		else
		{
			//
			// What we have here is a corrupted message that doesn't
			// have a CR/LF to terminate the current line of the message.
			// Bail the header processing loop, placing a canned
			// blurb into the message body stating that the message is
			// corrupted.
			//
			ASSERT( FALSE );	// every line should have a delimiter
			delete [] m_Text;
			CRString corrupted(IDS_ERR_CORRUPTED_MESSAGE);
			m_Text = new char[corrupted.GetLength() + 1];
			if (m_Text)
				strcpy(m_Text, corrupted);
			t = NULL;
			break;
		}

		char* colon = strchr( t, ':' );
		if ( colon )
		{
			// Check if this is a header we use?
			int i = FindRStringIndexI(IDS_HEADER_TO, IDS_HEADER_REFERENCES, t, colon - t);

			// If found, fill in the value for display.
			if (i >= 0)
			{
				// Get the value for the header
				if (*++colon == ' ')
					colon++;
				m_Headers[i] = colon;
				bConsumed = TRUE;
			}
		}

		while ( ! bConsumed )	// will only run once
		{
			bConsumed = TRUE;

			char ours[ 80 ];

			QCLoadString( IDS_TRANS_XHEADER, ours, sizeof( ours ) );
			if ( strstr( start, ours ) == start )
				continue;

			QCLoadString( IDS_SIGNATURE_XHEADER, ours, sizeof( ours ) );
			if ( strstr( start, ours ) == start )
				continue;

			QCLoadString( IDS_PERSONA_XHEADER, ours, sizeof( ours ) );
			if ( strstr( start, ours ) == start )
				continue;

			QCLoadString( IDS_PRECEDENCE_HEADER, ours, sizeof( ours ) );
			if ( strstr( start, ours ) == start )
				continue;

			// save extra headers
			if (! m_ExtraHeaders.IsEmpty() )
				m_ExtraHeaders += "\r\n";
			m_ExtraHeaders += start;
		}

		// go on to next header
		t = end + 1;
	}

	// delimit the extra headers
	if (! m_ExtraHeaders.IsEmpty() )
		m_ExtraHeaders += "\r\n";

	// default From: if need be
	if ( m_Headers[ HEADER_FROM ].IsEmpty() )
	{
		m_Headers[ HEADER_FROM ] = GetReturnAddress();
	}

#ifndef unix
	int hasLF = 1;
#else
	int hasLF = 0;
#endif

	
	// find the begining of the body which start with a \n
	// step over it so the body start at the first line
	if (t && (t = strchr(t + hasLF, '\n')))
		strcpy(m_Text, t + 1);

	// strip off the trailing \r\n we added on save
	if ( ( uLen = strlen( m_Text ) ) >= 2 )
	{
		if( ( m_Text[ uLen - 1 ] == '\n' ) && ( m_Text[ uLen - 2 ] == '\r' ) ) 
		{
			// truncate it at the \r\n
			m_Text[ uLen - 2 ] = '\0';
		}
	}


	m_HasBeenSaved = TRUE;


	//Reset these flags only if they haven't been set yet..they should never be set to TRUE
	m_HeadersInvalidFlag[HEADER_FROM] = FALSE;
	m_HeadersInvalidFlag[HEADER_SUBJECT] = FALSE;
	m_HeadersInvalidFlag[HEADER_ATTACHMENTS] = FALSE;

	return (TRUE);
}


// Write
BOOL CCompMessageDoc::Write()
{
	return (Write((JJFile*)NULL));
}

BOOL CCompMessageDoc::Write(JJFile* out)
{
	char* mes = NULL;
	int CreatedJJFile = FALSE;

	CTocDoc *OutToc = GetOutToc();
	if (!OutToc)
		return (FALSE);
	m_Sum->m_TheToc = OutToc;

	// Save the flags
	if (!out)
	{
		CHeaderView* pHView = GetHeaderView();
		
		// BADSTUFF - Here we just hacked it to use our Paige derived view. This
		// sucks just as bad; we need to use protocols to solve this view-
		// dependancy stuff.
		CView* View = GetView();
		if ( View && pHView )
		{
			pHView->SaveToDoc();
			((PgMsgView*)View)->SaveInfo();

			if (!IsHeaderEmpty(HEADER_ATTACHMENTS))
				m_Sum->SetFlag(MSF_HAS_ATTACHMENT);
			else
				m_Sum->UnsetFlag(MSF_HAS_ATTACHMENT);

			OutToc->SetModifiedFlag();
		}
	}

    // Let's not waste time and disk space, shall we
    if (!out && m_HasBeenSaved && !IsModified())
        return (TRUE);
	 else
		OutToc->SetModifiedFlag();

	while (1)
	{
		char buf[256];
		
		if (!out)
		{
			out = new JJFile;
			CreatedJJFile = TRUE;
			if (FAILED(out->Open(OutToc->MBFilename(), O_APPEND | O_RDWR)))
				break;
		}
	
		// Get the offset of the start of the message
		long Start = 0;
		out->Tell(&Start);
		ASSERT(Start >= 0);
	
		// Write From line
		CTime Time(CTime::GetCurrentTime());
		if (Time.GetTime() < 0)
			Time = 0;
		CString FromString = ::FormatTimeMT(Time.GetTime(), GetIniString(IDS_FROM_CTIME_FORMAT));
		if (FAILED(out->PutLine(FromString))) break;
	
		// Write out headers
		int i = 0;
		for (; i < MaxHeaders; i++)
		{
			if (FAILED(out->Put(CRString(IDS_HEADER_TO + i))) || FAILED(out->Put(" "))) break;
	
			char* t = m_Headers[i].GetBuffer(m_Headers[i].GetLength());
			char* HL = t;
			char* n = t;
			while (n && (*n == ' ' || *n == '\t'))
				n++;
			for (; n && *n; n++)
			{
				if (*n == '\n')
					*t++ = ' ';
				else if (*n != '\r')
					*t++ = *n;
			}
			if (t)
			{
				*t = 0;
				if (FAILED(out->Put(HL))) break;
			}
			if (FAILED(out->PutLine())) break;
			m_Headers[i].ReleaseBuffer();
		}
		// Did something go wrong?
		if (i != MaxHeaders) break;

		// Add any extra headers
		if ( ! m_ExtraHeaders.IsEmpty() )
		{
			if (FAILED(out->Put(m_ExtraHeaders))) break;
		}

		// Put the Translator Header
		CString TransString;
		CString Tltrs = m_Sum->GetTranslators();
		if (!Tltrs.IsEmpty())
		{
			TransString.Format(CRString(IDS_TRANS_XHEADER_FORMAT),(const char *)Tltrs);
			if (FAILED(out->PutLine(TransString))) break;
		}
	
		// Put the Signature Header  
		CString SigString = m_Sum->m_SigSelected;
		if (!SigString.IsEmpty())
		{
			CString szTemp;
			szTemp.Format(CRString(IDS_SIGNATURE_XHEADER_FORMAT),(const char *)SigString);
			if (FAILED(out->PutLine(szTemp))) break;
			m_Sum->m_SigHdr = m_Sum->m_SigSelected;
		}

		// Put the Persona Header
		CString PersonaString;
		CString Persona = m_Sum->GetPersona();
		if (!Persona.IsEmpty())
		{
			PersonaString.Format(CRString(IDS_PERSONA_XHEADER_FORMAT),(const char *)Persona);
			if (FAILED(out->PutLine(PersonaString))) break;
		}
	
		// Put the Precedence Header
		CString PrecString;
		CString Precedence = m_Sum->GetPrecedence();
		if (!Precedence.IsEmpty())
		{
			PrecString.Format(CRString(IDS_PRECEDENCE_HEADER_FORMAT),(const char *)Precedence);
			if (FAILED(out->PutLine(PrecString))) break;
		}

		// BOG: write the "embedded object headers"; these don't go out
		// over the wire.
		CString eoHeaders;
		m_QCMessage.GetEmbeddedObjectHeaders( eoHeaders );
		if ( !eoHeaders.IsEmpty() ) {
			if (FAILED(out->Put(eoHeaders) < 0)) break;
			if (FAILED(out->PutLine())) break;
		}

		// Separate Header section from the Body of the message
		if (FAILED(out->PutLine())) break;
	
		if (!m_Text) break;
		
		//If we add space or quote at the beginning of lines starting
		//with "From ", then we should read the body text again. Else, the
		//send code just calls GetFullMessage which detects m_text is already
		//loaded up. But we just wrote some spaces or >s into the mbx file,
		//but haven't changed m_text itself.
		BOOL bNeedReRead = FALSE;

		char* Text = m_Text;
		do
		{
			long len = strcspn(Text, "\n");
			if (Text[len] == '\n')
				len++;
	
	        // Quote From lines
			//For html msgs we add a space since spaces at the beginning of the line
			//are collapsed anyway on display
			//For all other msg types we quote the from line
			if ( m_Sum->IsHTML() )
			{
				if ( Text[0] == 'F' && Text[1] == 'r' && Text[2] == 'o' && 
					 Text[3] == 'm' && Text[4] == ' ' ) 
				{
					if (FAILED(out->Put(' '))) break;
					bNeedReRead = TRUE;
				}
			}
			else if ( IsFromLine(Text) )
			{
				if ( FAILED(out->Put(' '))) break;
				bNeedReRead = TRUE;
			}
	
			if (FAILED(out->Put(Text, len))) break;
			Text += len;
		} while (*Text);
		// Did something go wrong?
		if (*Text) break;
	
		// add the body separator
		if (FAILED(out->PutLine())) break;
	
		// Update the message summary
		CSummary::m_lBegin = Start;
		out->Seek(Start);
		m_Sum->SetFrom(NULL);
		m_Sum->SetSubject(NULL);
//		m_Sum->SetDate(NULL);
	
		// Save State and Date since Build() changes it
		short state = m_Sum->m_State;
		strcpy(buf, m_Sum->m_Date);
		m_Sum->m_TheToc->m_DelSpace += m_Sum->m_Length;
		m_Sum->m_TheToc->m_MesSpace -= m_Sum->m_Length;
		m_Sum->Build(out);
		m_Sum->m_TheToc->m_MesSpace += m_Sum->m_Length;
		m_Sum->m_TheToc->m_TotalSpace += m_Sum->m_Length;
		m_Sum->SetState( (char)state);
		m_Sum->SetDate(time(NULL) + (GetGMTOffset() * 60));
	
		if (m_Sum->IsQueued() == FALSE)
		{
			m_Sum->SetState(!IsHeaderEmpty(HEADER_TO) || !IsHeaderEmpty(HEADER_BCC)?
				MS_SENDABLE : MS_UNSENDABLE);
		}
	
		// Possibly change the title info
		ReallySetTitle(m_Sum->MakeTitle());
	
		if (CreatedJJFile)
		{
			out->Close();
			OutToc->Write();
			if (m_HasBeenSaved)
				OutToc->m_NeedsCompact = TRUE;
			else
				m_HasBeenSaved = TRUE;
			SetModifiedFlag(FALSE);
			delete out;
		}

		if (bNeedReRead)
		{
			delete [] m_Text;
			m_Text = NULL;
		}

		return (TRUE);
	}

	if (out && CreatedJJFile)
		delete out;
		
	return (FALSE);
}


////////////////////////////////////////////////////////////////////////
// FindFirst [public, virtual]
//
// Implementation of pure virtual defined in base class.
////////////////////////////////////////////////////////////////////////
BOOL CCompMessageDoc::FindFirst(const CString& searchStr, BOOL matchCase, BOOL wholeWord, BOOL bOpenWnd /* = TRUE */)
{

	m_FindHeaderIndex = 0;	// force search to start at first header
	m_FindIndex = 0;		// force search to start at top of message
	BOOL result = FALSE;

	//
	// Make copies of the search string and the actual message text.
	// Yes, it's inefficient to ALWAYS make copies, but since most
	// searches are case-insensitive, we'll probably end up making a
	// copy anyway.  We need these local copies so that we can call
	// MakeLower() and strlwr() on them.
	//
	CString search_str(searchStr);

	//
	// Force message data to be read up from file.
	//
	GetText();

	//
	// Kick off the search, first looking through each header in turn.
	// If there is no match in a header, then continue the search in
	// the message body.
	//
	for (int i = m_FindHeaderIndex; i <= 6; i++)
	{
		//
		// Get the text to scan in this pass, searching from the last
		// find position.
		//
		CString msg_text;
		if (i < 6)
			msg_text = (char *) GetHeaderLine(i);
		else 
		{
			msg_text = (char *) GetText();

			//
			// Strip text-enriched formatting codes, if any.
			//
/*	fix		if (m_Sum->IsXRich())
			{
				TextReader reader;

				const OLD_LENGTH = msg_text.GetLength();
				char* p_msgtext = msg_text.GetBuffer(OLD_LENGTH + 4);
				int new_length = reader.StripRich(p_msgtext, OLD_LENGTH, TRUE);
				ASSERT(new_length < OLD_LENGTH);
				p_msgtext[new_length] = '\0';
				msg_text.ReleaseBuffer(new_length);
			}
*/		}
		const char* p_msgtext = msg_text;

		if (! matchCase)
		{
			msg_text.MakeLower();
			search_str.MakeLower();
		}

		ASSERT(m_FindIndex >= 0);
		if (m_FindIndex > 0)
		{
			ASSERT(m_FindIndex < long(msg_text.GetLength()));
			p_msgtext += m_FindIndex;
		}
		
		//
		// Do the search...
		//
		char* p_match = strstr(p_msgtext, search_str);
		if (p_match)
		{
			//
			// Got a hit.
			//
			result = TRUE;

			m_FindIndex += p_match - p_msgtext;
			ASSERT(m_FindIndex >= 0);

			if (!bOpenWnd)
				return (TRUE);

			//
			// Open the message window, if necessary, making it the active MDI child.
			//
			ASSERT(m_Sum != NULL);
			ASSERT(m_Sum->IsComp());
			m_DidFindOpenView = (GetFirstViewPosition() == NULL);
			m_Sum->Display();

			//
			// Get the one and only comp message window for this document.
			//
			CView* p_compmsgv = GetCompView();
			if (NULL == p_compmsgv)
			{
				ASSERT(0);
				m_FindIndex = -1;
				return FALSE;
			}
			//
			// 32-bit implementation.  FORNOW, should call a generic
			// "get header view" function here whenever BOGDON gets
			// around to writing one.
			//
			CCompMessageFrame* p_frame = (CCompMessageFrame *) p_compmsgv->GetParentFrame();
			ASSERT(p_frame && p_frame->IsKindOf(RUNTIME_CLASS(CCompMessageFrame)));
			CHeaderView* p_hdrview = p_frame->GetHeaderView();
			ASSERT(p_hdrview && p_hdrview->IsKindOf(RUNTIME_CLASS(CHeaderView)));
			ASSERT(m_FindHeaderIndex >= 0);
			if (m_FindHeaderIndex < 6)
			{
				//
				// Got a hit in the header, so we need to grab the
				// proper edit control and tell the edit control
				// exactly where to do the selection.
				//
				CHeaderField* p_editctrl = p_hdrview->GetHeaderCtrl(m_FindHeaderIndex);
				if (p_editctrl != NULL)
				{
					//
					// We need to set the active view ourselves
					// so that the keyboard focus ends up where
					// we want it.  FORNOW, this is probably
					// overkill since the comp message view
					// splitter comes up with the CHeaderView pane
					// as the active view by default.
					//
					p_frame->SetActiveView(p_hdrview, TRUE);

					//
					// The following call is a very special hack
					// to fool the CFormView-derived object to set
					// the focus where we want it and not where it
					// thinks it wants it.
					// 
					p_hdrview->SetFocusToHeader(m_FindHeaderIndex);

					FindSetSel32(/* p_compmsgv,*/ p_hdrview, p_editctrl, int(m_FindIndex), int(m_FindIndex + search_str.GetLength()));
				//	p_editctrl->SetSel(m_FindIndex, m_FindIndex + search_str.GetLength(), FALSE);
					m_FindIndex++;
				}
			}
			else
			{
				QCProtocol*	pProtocol = QCProtocol::QueryProtocol( QCP_FIND, p_compmsgv );

				if( (	pProtocol == NULL ) ||
					(	pProtocol->DoFindFirst( searchStr, matchCase, wholeWord, TRUE ) == FALSE ) )
				{
					return FALSE;
				}
				result = TRUE;

			}
			break;
		}
		else
		{
			//
			// No match in this header, so try next header, if any.
			//
			m_FindHeaderIndex++;
			m_FindIndex = 0;
		}
	}

	//
	// Set index so that subsequent FindNext() calls don't choke on
	// the fact that there is no view for this doc.
	//
	if (! result)
	{
		m_FindHeaderIndex = -1;
		m_FindIndex = -1;
	}
	return result;
}


////////////////////////////////////////////////////////////////////////
// FindNext [public, virtual]
//
// Implementation of pure virtual defined in base class.
////////////////////////////////////////////////////////////////////////
BOOL CCompMessageDoc::FindNext(const CString& searchStr, BOOL matchCase, BOOL wholeWord)
{
	//
	// Check for previous Find action, redirecting to FindFirst() if
	// there was no previous Find.
	//
	if (m_FindHeaderIndex < 0)
		return FindFirst(searchStr, matchCase, wholeWord);

	//
	// Get the one and only comp message window for this document.
	//
	CView* p_compmsgv = GetCompView();
	if (NULL == p_compmsgv)
	{
		ASSERT(0);
		m_FindIndex = -1;
		return FALSE;
	}

	BOOL result = FALSE;		// returned

	if (6 == m_FindHeaderIndex)
	{
		//
		// Special case for 32-bit implementation.  If we had a
		// previous hit in the message body, we just need to delegate
		// to QCProtocol to find the next hit, if any.
		// 
		QCProtocol*	pProtocol = QCProtocol::QueryProtocol( QCP_FIND, p_compmsgv );
		m_Sum->Display();
		
		if( (	pProtocol == NULL ) ||
			(	pProtocol->DoFindNext( searchStr, matchCase, wholeWord, TRUE ) == FALSE ) )
		{
			return FALSE;
		}
		result=TRUE;
	}
	else
	{
		//
		// Run a loop to scan remaining header controls, if any, plus the
		// actual message text, starting where we left off from last time.
		// Make copies of the search string and the actual message text.
		// Yes, it's inefficient to ALWAYS make copies, but since most
		// searches are case-insensitive, we'll probably end up making a
		// copy anyway.  We need these local copies so that we can call
		// MakeLower() and strlwr() on them.
		//
		CString search_str(searchStr);

		//
		// Kick off the search, first looking through each header in turn.
		// If there is no match in a header, then continue the search in
		// the message body.
		//
		for (int i = m_FindHeaderIndex; i <= 6; i++)
		{
			//
			// Get the text to scan in this pass, searching from the last
			// find position.
			//
			CString msg_text;
			if (i < 6)
				msg_text = (char *) GetHeaderLine(i);
			else
			{
				msg_text = (char *) GetText();

				//
				// Strip text-enriched formatting codes, if any.
				//
/* fix				if (m_Sum->IsXRich())
				{
					TextReader reader;

					const OLD_LENGTH = msg_text.GetLength();
					char* p_msgtext = msg_text.GetBuffer(OLD_LENGTH + 4);
					int new_length = reader.StripRich(p_msgtext, OLD_LENGTH, TRUE);
					ASSERT(new_length < OLD_LENGTH);
					p_msgtext[new_length] = '\0';
					msg_text.ReleaseBuffer(new_length);
				}
*/			}

			const char* p_msgtext = msg_text;

			if (! matchCase)
			{
				msg_text.MakeLower();
				search_str.MakeLower();
			}

			ASSERT(m_FindIndex >= 0);
			if (m_FindIndex > 0)
			{
				ASSERT(m_FindIndex < long(msg_text.GetLength()));
				p_msgtext += m_FindIndex;
			}
			
			//
			// Do the search...
			//
			char* p_match = strstr(p_msgtext, search_str);
			if (p_match)
			{
				//
				// Found a match!
				//
				m_FindIndex += p_match - p_msgtext;		// save position for next time
				ASSERT(m_FindIndex >= 0);

				m_Sum->Display();			// already open, but make it the active child window

				//
				// 32-bit implementation.  FORNOW, should call a generic
				// "get header view" method here someday.
				//
				CCompMessageFrame* p_frame = (CCompMessageFrame *) p_compmsgv->GetParentFrame();
				ASSERT(p_frame && p_frame->IsKindOf(RUNTIME_CLASS(CCompMessageFrame)));
				CHeaderView* p_hdrview = p_frame->GetHeaderView();
				ASSERT(p_hdrview && p_hdrview->IsKindOf(RUNTIME_CLASS(CHeaderView)));
				ASSERT(m_FindHeaderIndex >= 0);

				if (m_FindHeaderIndex < 6)
				{
					//
					// Got a hit in the header, so we need to grab the
					// proper edit control and do the selection ourselves.
					//
					CHeaderField* p_editctrl = p_hdrview->GetHeaderCtrl(m_FindHeaderIndex);
					if (p_editctrl != NULL)
					{
						//
						// We need to set the active view ourselves
						// so that the keyboard focus ends up where
						// we want it.
						//
						p_frame->SetActiveView(p_hdrview, TRUE);

						//
						// The following call is a very special hack to
						// fool the CFormView-derived object to set the
						// focus where we want it and not where it thinks
						// it wants it.
						// 
						p_hdrview->SetFocusToHeader(m_FindHeaderIndex);

						FindSetSel32(/* p_compmsgv,*/ p_hdrview, p_editctrl, int(m_FindIndex), int(m_FindIndex + search_str.GetLength()));
					//	p_editctrl->SetSel(m_FindIndex, m_FindIndex + search_str.GetLength(), FALSE);
						m_FindIndex++;
						result = TRUE;
					}
				}
				else
				{
					//
					// Got a hit in the body, so first deselect
					// everything in the headers, then perform
					// ANOTHER search in the body using the built-in
					// QCProtocol method.
					//
					FindSetSel32(/*p_compmsgv, */p_hdrview, NULL, -1, -1);
					QCProtocol*	pProtocol = QCProtocol::QueryProtocol( QCP_FIND, p_compmsgv );

					if( (	pProtocol == NULL ) ||
						(	pProtocol->DoFindFirst( searchStr, matchCase, wholeWord, TRUE ) == FALSE ) )
					{
						return FALSE;
					}
					result = TRUE;
				}
				break;
			}
			else
			{
				//
				// No match in this header, so try next header, if any.
				//
				m_FindHeaderIndex++;
				m_FindIndex = 0;
			}
		}
	}

	//
	// Set indices so that subsequent FindNext() calls don't choke on
	// the fact that there is no view for this doc.
	//
	if (! result)
	{
		m_FindHeaderIndex = -1;
		m_FindIndex = -1;
	}
	return result;

}


////////////////////////////////////////////////////////////////////////
// FindAgain [public, virtual]
//
// Implementation of pure virtual defined in base class.
////////////////////////////////////////////////////////////////////////
BOOL CCompMessageDoc::FindAgain(const CString& searchStr, BOOL matchCase, BOOL wholeWord)
{
	if (m_FindHeaderIndex < 0)
	{
		//
		// This is an unusual, but legal, case where the user didn't find
		// a hit last time we searched in an open message.
		// 
		return FindFirst(searchStr, matchCase, wholeWord);
	}

	if (FindNext(searchStr, matchCase, wholeWord))
		return TRUE;

	return FindFirst(searchStr, matchCase, wholeWord);
}



////////////////////////////////////////////////////////////////////////
// FindSetSel32 [protected]
//
// Helper function for the 32-bit Find Engine which knows how to do the
// selection correctly for the split comp message window.  The
// problem is that you must clear any existing selection from all
// of the edit controls before you can set a new selection.  Otherwise,
// you end up with multiple, disjoint selections on the screen.
//
////////////////////////////////////////////////////////////////////////

void CCompMessageDoc::FindSetSel32(
//	CCompMessageView* pCompMessageView, 	//(i) lower rich edit view pane (required)
	CHeaderView* pHeaderView,				//(i) upper form view pane (required)
	CEdit* pEditCtrl,						//(i) header control containing text to be selected (optional)
	int startIndex,							//(i) starting offset for pEditCtrl (optional)
	int endIndex)							//(i) ending offst for pEditCtrl (optional)
{

	if (/*NULL == pCompMessageView ||*/ NULL == pHeaderView)
	{
		ASSERT(0);		// caller blew it
		return;
	}

	//
	// The actual type returned by GetHeaderCtrl() is actually
	// CHeaderField, which is a derivation of CEdit.  Also, the type
	// returned by GetBodyCntl() is actually CCompBody, which is also
	// a derivation of CEdit.  However, we don't need any special
	// subclassed edit control behavior, so force a generic CEdit.
	//
	// FORNOW, the 32-bit header doesn't support an Attachments header
	// line.
	//
	for (int i = 0; i <= 5; i++)
	{
		if (i < 5)
		{
			CEdit* p_edit = (CEdit *) pHeaderView->GetHeaderCtrl(i);
			if (p_edit != NULL)
			{
				ASSERT(p_edit->IsKindOf(RUNTIME_CLASS(CEdit)));
				p_edit->SetSel(0, 0);							// deselect all
			}
		}
//		else
//		{
//			pCompMessageView->GetEditCtrl().SetSel(0, 0);
//		}
	}

	//
	// Finally, select the stuff that we were interested in the first
	// place.  If the caller didn't provide an edit control, don't
	// panic -- that means they just wanted to deselect everything in
	// the header, presumably because we're about to select something
	// in the body.
	//
	if (pEditCtrl)
	{
		ASSERT(pEditCtrl->IsKindOf(RUNTIME_CLASS(CEdit)));
		pEditCtrl->SetFocus();
		pEditCtrl->SetSel(startIndex, endIndex, FALSE);
	}
}


BEGIN_MESSAGE_MAP(CCompMessageDoc, CMessageDoc)
	//{{AFX_MSG_MAP(CCompMessageDoc)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_CHANGEQUEUEING, OnUpdateChangeQueueing)
	ON_COMMAND(ID_MESSAGE_CHANGEQUEUEING, OnChangeQueueing)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_SENDABLE, OnUpdateStatusSendable)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_QUEUED, OnUpdateStatusQueued)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_TIME_QUEUED, OnUpdateStatusTimedQueue)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_SENT, OnUpdateStatusSent)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_UNSENT, OnUpdateStatusUnsent)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE_AS_STATIONERY, OnFileSaveAsStationery)
	ON_COMMAND(ID_MESSAGE_SENDIMMEDIATELY, OnSend)
//	ON_COMMAND(ID_MESSAGE_ATTACHFILE, OnAttachFile)
	ON_COMMAND(ID_MESSAGE_DELETE, OnMessageDelete)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_ATTACHFILE, OnCanModify)
	//}}AFX_MSG_MAP

	ON_COMMAND_EX(ID_MESSAGE_STATUS_SENDABLE, OnStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_QUEUED, OnStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_TIME_QUEUED, OnStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_SENT, OnStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_UNSENT, OnStatus) 
	ON_COMMAND( ID_FCC_NEW_MBOX_IN_ROOT, OnFCCNewInRoot )
	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand )

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCompMessageDoc commands

void CCompMessageDoc::OnUpdateChangeQueueing(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Sum->IsSendable());
}

void CCompMessageDoc::OnChangeQueueing()
{
	if (!m_Sum || m_Sum->CantEdit())
		ASSERT(FALSE);
	else
	{
		CChangeQueueing dlg(m_Sum->m_State == MS_TIME_QUEUED? m_Sum->m_Seconds : 0);
		
		if (dlg.DoModal() == IDOK)
		{
			dlg.ChangeSummary(m_Sum);
			if (FlushQueue)
				SendQueuedMessages();
		}
	}
}

void CCompMessageDoc::OnUpdateStatusSendable(CCmdUI* pCmdUI)
{
	BOOL Enable = FALSE;

	if (m_Sum)
	{
		switch (m_Sum->m_State)
		{
		case MS_SENDABLE:
		case MS_QUEUED:
		case MS_TIME_QUEUED:
			Enable = TRUE;
			break;
		}
	}

	pCmdUI->Enable(Enable);
}

void CCompMessageDoc::OnUpdateStatusQueued(CCmdUI* pCmdUI)
{
	BOOL Enable = FALSE;

	if (m_Sum)
	{
		switch (m_Sum->m_State)
		{
		case MS_QUEUED:
		case MS_SENDABLE:
		case MS_TIME_QUEUED:
			Enable = TRUE;
			break;
		}
	}

	pCmdUI->Enable(Enable);
}

void CCompMessageDoc::OnUpdateStatusTimedQueue(CCmdUI* pCmdUI)
{
	BOOL Enable = FALSE;

	if (m_Sum)
	{
		switch (m_Sum->m_State)
		{
		case MS_TIME_QUEUED:
		case MS_SENDABLE:
		case MS_QUEUED:
			Enable = TRUE;
			break;
		}
	}

	pCmdUI->Enable(Enable);
}

void CCompMessageDoc::OnUpdateStatusSent(CCmdUI* pCmdUI)
{
	BOOL Enable = FALSE;

	if (m_Sum)
	{
		switch (m_Sum->m_State)
		{
		case MS_SENT:
		case MS_UNSENT:
			Enable = TRUE;
			break;
		}
	}

	pCmdUI->Enable(Enable);
}

void CCompMessageDoc::OnUpdateStatusUnsent(CCmdUI* pCmdUI)
{
	BOOL Enable = FALSE;

	if (m_Sum)
	{
		switch (m_Sum->m_State)
		{
		case MS_UNSENT:
		case MS_SENT:
			Enable = TRUE;
			break;
		}
	}

	pCmdUI->Enable(Enable);
}

BOOL CCompMessageDoc::OnStatus(UINT StatusMenuID)
{
	if (!m_Sum)
		return (FALSE);

	int NewStatus = m_Sum->m_State;

	switch (StatusMenuID)
	{
	case ID_MESSAGE_STATUS_SENDABLE:
		NewStatus = MS_SENDABLE;
		break;
	case ID_MESSAGE_STATUS_QUEUED:
		NewStatus = MS_QUEUED;
		break;
	case ID_MESSAGE_STATUS_TIME_QUEUED:
		NewStatus = MS_TIME_QUEUED;
		break;
	case ID_MESSAGE_STATUS_SENT:
		NewStatus = MS_SENT;
		break;
	case ID_MESSAGE_STATUS_UNSENT:
		NewStatus = MS_UNSENT;
		break;
	default:
		ASSERT(FALSE);
		break;
	}

	m_Sum->SetState(char(NewStatus));
	SetQueueStatus();

	return (TRUE);
}

// hack city
void CCompMessageDoc::OnFilePrint() 
{
	CView* pRchView = GetView();
  pRchView->SendMessage( WM_COMMAND, ID_FILE_PRINT );
}

void CCompMessageDoc::OnFilePrintPreview()
{
	CView* pRchView = GetView();
	pRchView->SendMessage( WM_COMMAND, ID_FILE_PRINT_PREVIEW );
}



// this stuff supports processing multiple documents via one MessageOptions setting
static BOOL bInGroup;
static BOOL bFirstInGroup;
static CString csGroupPersona;
static CString csGroupStationery;

void StartGroup( void )
{
	// Obsolete?  SD 7/13/99 bInGroup = TRUE;
	// Obsolete?  SD 7/13/99 bFirstInGroup = TRUE;
}

void EndGroup( void )
{
	// Obsolete?  SD 7/13/99 bInGroup = FALSE;
	// Obsolete?  SD 7/13/99 bFirstInGroup = FALSE;
}

/*
	As far as I can tell, every message Eudora ever sends is created by this
	method.  For sure, New Message, Reply, Reply All, Reply With, Forward, 
	Redirect, and all the XXX To equiv's go through here.

    I've added Stationery and Persona.  The precedence that all this stuff
	gets applied to the new message is as follows (lowest to highest);

	1) To, From, Subject, Cc, Bcc, Attachments, Body 
	2) Default Stationery   - if no explicit Stationery is specified and the 
						      Personality (including Dominant) has a default Stationery, 
						      it's applied.
	3) Explicit Stationery  - if specified, is applied instead of (2).  
	4) Default Personality  - If no explicit Personality is specified, the Stationery's 
						      Personality will be used. If no Stationery was applied the
							  Dominant personality will be used.  The Personality is used
							  to determine the From line, Signature, and how the message is
							  sent.
	5) Explicit Personality - if specified, is used instead of (4) to determine From line,
	                          Signature, and how the message is sent.

	Note that applying Stationery can change the effective Personality.  Hence if a
	Personality's default Stationery was created by a different Personality...

	User input supplied via the Message Options dialog rules over parameters.

  Prophecy -
	Someday Dominant and Persona.IsEmpty() are not going to be equivilent.
	We will be revisiting this...
*/
CCompMessageDoc* NewCompDocument
(
	const char* To			/*= NULL*/, 
	const char* From		/*= NULL*/,
	const char* Subject		/*= NULL*/, 
	const char* Cc			/*= NULL*/, 
	const char* Bcc			/*= NULL*/,
	const char* Attachments /*= NULL*/, 
	const char* Body		/*= NULL*/,
	const char* Stationery	/*= NULL*/,
	const char* Persona     /*= NULL*/,
	const char	ResponseType /*= 0*/,
	const char* ECHeaders /* = NULL*/
)
{
	CCompMessageDoc* NewCompDoc = NULL;
	
#ifdef COMMERCIAL

	CString csStationery = Stationery;
	CString	csPersona = Persona;
	CString csStatPersona;
	CString csCurPersona = g_Personalities.GetCurrent();


#if 0
// BOG: this is the old Shift-* sets the stationary/personality feature,
// which has now gone back to Shift-Reply == ReplyAll. Woohoo!

	if ( ( Stationery == NULL ) && ShiftDown())	
	{
		CMessageOptions			dlg;
		QCStationeryCommand*	pCommand;

//		char statname[ 80 ];
//		GetStatItemName( csStationery, statname, sizeof( statname) );
		dlg.m_Persona = csPersona;

		pCommand = g_theStationeryDirector.FindByPathname( csStationery );

		if( pCommand )
		{
			dlg.m_Stationery = pCommand->GetName();
		}

		int result = dlg.DoModal();
		if ( result == IDCANCEL )
			return NewCompDoc;
		if ( result == IDOK )
		{
			csPersona = dlg.m_Persona;
			csStationery = dlg.m_Stationery;
		}
	}
#endif


	// check for groups of documents
	if ( bFirstInGroup )
	{
		csGroupPersona = csPersona;
		csGroupStationery = csStationery;
		bFirstInGroup = FALSE;
	}

	if ( bInGroup )
	{
		csPersona = csGroupPersona;
		csStationery = csGroupStationery;
	}

	// make sure we get the right persona's default stationery
	if ( ! csPersona.IsEmpty() && g_Personalities.IsA( csPersona ) )
		g_Personalities.SetCurrent( csPersona );

#endif // COMMERCIAL

	// create the new doc with the defaults
	NewCompDoc = (CCompMessageDoc*)NewChildDocument(CompMessageTemplate);
	if (NewCompDoc)
		NewCompDoc->InitializeNew(To, From, Subject, Cc, Bcc, Attachments, Body, ECHeaders);
	else
		return (NewCompDoc);	// error creating document


	//No need for Persona and stationery in 4.1 Light
#ifdef COMMERCIAL

	// Do not apply stationery if redirecting
	// or if call is from Automation
	if ((MS_REDIRECT != ResponseType) || gbAutomationCall)
	{
		// determine which (if any) stationery to apply
		if ( csStationery.IsEmpty() )
		{
			const char * defaultStat = GetIniString(IDS_INI_STATIONERY);
			if (defaultStat && *defaultStat)
				csStationery = defaultStat;
		}

		// apply the stationery (may specify a personality)
		if ( ! csStationery.IsEmpty() )
		{
			CString	szFileName;

			// csStationery might be fully qualified already
			if ( strchr( csStationery, '\\' ) )
			{
				szFileName = csStationery;
			}
			else
			{
				QCStationeryCommand*	pCommand;
				
				pCommand = g_theStationeryDirector.Find( csStationery );				
				
				if( pCommand )
				{
					szFileName = pCommand->GetPathname();
				}
			}

			if( ( szFileName != "" ) && ::FileExistsMT( szFileName ) )
			{
				CCompStationeryDoc NewStatDoc;
				NewStatDoc.Read( szFileName);
				NewCompDoc->ApplyStationery( &NewStatDoc );

				// look for a personality change
				if ( NewStatDoc.m_Sum != NULL )
				{
					csStatPersona = NewStatDoc.m_Sum->GetPersona();
				}
			}

			//ReallySetTitle();
		}
	}

	//
	// determine which personality we really want to use according to
	// Keith's Rule:  Stationery Rules(tm) when it comes to personality.
	//
//FORNOW	if ( csPersona.IsEmpty() )
//FORNOW	{
	if ( ! csStatPersona.IsEmpty() && g_Personalities.IsA( csStatPersona ) )
	{
		// use the stationery's persona
		csPersona = csStatPersona;
		g_Personalities.SetCurrent( csPersona );
	}
//FORNOW	}

	// Add personality overrides
	NewCompDoc->ApplyPersona( csPersona );
	
	g_Personalities.SetCurrent( csCurPersona );	// go back to default persona

#else
	//for Light 4.1 - don't know whethere we really need to apply persona, but anyways :)
	// Add personality overrides
	NewCompDoc->ApplyPersona( Persona );
#endif // COMMERCIAL


	return (NewCompDoc);
}

CCompMessageDoc* NewCompDocumentWith(const char * fileName)
{
	char StrippedFileName[MAX_PATH + 1];
	int len;

	strcpy(StrippedFileName, *fileName == '"'? fileName + 1 : fileName);
	len = strlen(StrippedFileName);
	if (StrippedFileName[len - 1] == '"')
		StrippedFileName[len - 1] = 0;

	// Make sure the file is there
	if (::FileExistsMT(StrippedFileName))
	{
		// ( To, From, Subject, Cc, Bcc, Attachments, Body, Stationery, Persona )
		CCompMessageDoc* NewCompDoc = NewCompDocument( "", "", "", "", "", "", "", StrippedFileName, "" );
		return (NewCompDoc);
	}

	return (NULL);
}

CCompMessageDoc* NewCompDocumentAs(const CString& strPersona)
{
	// Make sure the persona exists
	if (g_Personalities.IsA(strPersona))
	{
		// ( To, From, Subject, Cc, Bcc, Attachments, Body, Stationery, Persona )
		CCompMessageDoc* pNewCompDoc = NewCompDocument("", "", "", "", "", "", "", "", strPersona);
		return pNewCompDoc;
	}

	ASSERT(0);
	return NULL;
}

BOOL CCompMessageDoc::ApplyStationery(CCompStationeryDoc *statDoc)
{
	CCompMessageDoc* msgdoc = (CCompMessageDoc *)m_Sum->GetMessageDoc();
	for (int i = 0; i < NumHeaders; i++)
	{
		char sepChar = 0;
		switch(i)
		{
			case HEADER_FROM:  //don't use this
				break;
			case HEADER_SUBJECT:
				{
					CString newSubject;
					CString oldString = GetHeaderLine(i);
					const char *subHdr = statDoc->GetHeaderLine(i);
					if (subHdr && *subHdr)
					{
						if (oldString.IsEmpty())
							SetHeaderLine(HEADER_SUBJECT,subHdr); 
						else 
						{
							newSubject.Format(CRString(IDS_SUBJECT_CHANGE),subHdr, ( const char* )oldString);
							SetHeaderLine(HEADER_SUBJECT,newSubject);
						}
					}
					else
						SetHeaderLine(HEADER_SUBJECT,oldString); 
				}
				break;
			case HEADER_TO:
			case HEADER_CC:
			case HEADER_BCC:
				sepChar = ',';
			case HEADER_ATTACHMENTS:
				{
					CString oldString = GetHeaderLine(i);
					const char *statHdr = statDoc->GetHeaderLine(i);
					if (statHdr && *statHdr && strcmp(statHdr, " "))
					{
						if (!oldString.IsEmpty())
						{
							if (sepChar)
							{
								oldString += sepChar;
								oldString += " ";
							}
							oldString += statHdr;
 							SetHeaderLine(i,oldString);
						}
						else
 							SetHeaderLine(i,statHdr);
					}
				}
				break;
		}
	}
	// Add A Line
	if (m_Text && *m_Text)
		CatText("\r\n");
	//Applying Plain Stat to a styled msg causes linebreaks to disappear
	if (statDoc->m_Sum)
	{
		if ((this->m_Sum->IsXRich()) || (this->m_Sum->IsHTML()))
		{
			if ((statDoc->m_Sum->IsXRich()) || (statDoc->m_Sum->IsHTML()))
				CatText(statDoc->GetText());
			else
			{
				CString szHtmlOn = "<" + CRString(IDS_MIME_HTML) + ">";
				CString szHtmlOff = "</" + CRString(IDS_MIME_HTML) + ">";

				CString statText = Text2Html(statDoc->GetText(), TRUE, TRUE);
				CatText(szHtmlOn);
				CatText(statText);
				CatText(szHtmlOff);
			}				
		}
		else
			CatText(statDoc->GetText());
	}
	else
		CatText(statDoc->GetText());	// 9/16 changed from GetBody()

	m_QCMessage.Init(CCompMessageDoc::m_MessageId, CCompMessageDoc::m_Text, FALSE);

	// Do we have summary info?
	if (statDoc->m_Sum)
	{
		BOOL bAddRich = m_Sum->IsXRich();
		BOOL bAddHTML = m_Sum->IsHTML();

		// copy the stationary flags
		m_Sum->CopyFlags( statDoc->m_Sum );
		
		if ( bAddRich ) 
			m_Sum->SetFlag( MSF_XRICH );
		if ( bAddHTML ) 
			m_Sum->SetFlagEx( MSFEX_HTML );
		
		m_Sum->SetPriority(statDoc->m_Sum->m_Priority);
		m_Sum->SetTranslators(statDoc->m_Sum->GetTranslators(), TRUE);
		m_Sum->SetSignature(statDoc->m_Sum->m_SigHdr);
		m_Sum->SetPersona( statDoc->m_Sum->GetPersona() );
	}

	m_strPathName.Empty();
	CCompMessageDoc::m_strPathName = statDoc->m_strPathName;
	m_StationeryApplied = TRUE;

	return (TRUE);
}

// Format the From line for redirect
void CCompMessageDoc::SetupRedirect( const char * oldFrom /* = NULL */ )
{
	const char* Return = GetReturnAddress();

	if (!IsRedirect())
		m_RedirectFrom = oldFrom ? oldFrom : GetHeaderLine(HEADER_FROM);

	char *NewFrom = new char[m_RedirectFrom.GetLength() + ::SafeStrlenMT(Return) + 100];
	sprintf(NewFrom, CRString(IDS_REDIRECT_FORMAT), (const char *)m_RedirectFrom, Return);
	SetHeaderLine(HEADER_FROM,NewFrom);
	delete [] NewFrom;
}

// From: and signature are always based of Persona
BOOL CCompMessageDoc::ApplyPersona( CString Persona )
{
	if ( ! Persona.IsEmpty() && ! g_Personalities.IsA( Persona ) )
		return FALSE;

	CRString DomDude( IDS_DOMINANT );
	if ( Persona == DomDude )
		Persona.Empty();	// we prefer the "" over "<Dominant>"

	CString CurPersona = g_Personalities.GetCurrent();

	if ( Persona != CurPersona ) 
		g_Personalities.SetCurrent( Persona );

	m_Sum->SetPersona( Persona );

#ifdef COMMERCIAL
	// IDS_INI_SIGNATURE_NAME must be the same string as IDS_INI_PERSONA_SIGNATURE
	char szEntry[ 80 ];

	// emulate 2.2 behavior
	if ( ! m_StationeryApplied && GetIniShort( IDS_INI_USE_SIGNATURE ) )
	{
		// look for ini entry and default to "Standard" if none found
		char szKey[ 64 ];
		QCLoadString( IDS_INI_SIGNATURE_NAME, szKey, sizeof( szKey ) );
		g_Personalities.GetProfileString( Persona, szKey, "NotThereAtAll", szEntry, sizeof( szEntry ) );
		if ( strcmp( szEntry, "NotThereAtAll" ) == 0 )
		{
			CRString csStandard( IDS_STANDARD_SIGNATURE );
			strcpy( szEntry, csStandard );
		}
	}
	else
	{
		GetIniString( IDS_INI_SIGNATURE_NAME, szEntry, sizeof( szEntry ) );
	}

	// Stationery signatures normally rule...
	// If we are editing a stationery and change the persona, then we want the
	//persona's signature
	BOOL bStationeryRules = GetIniShort( IDS_INI_SIGNATURE_PRECEDENCE );
	if ( IsStationery() || ! m_StationeryApplied || ( m_StationeryApplied && !bStationeryRules ) )
	{
		//If the personality had no default signature, we want to set the signature to <none>
		//Note the signature could be Empty or "No Default", Set signature handles all cases
		m_Sum->SetSignature( szEntry );
	}
#else

	if ( GetIniShort( IDS_INI_USE_SIGNATURE ) )
	{

		CRString csStandard( IDS_STANDARD_SIGNATURE );
		CRString csNone( IDS_SIGNATURE_NONE);
		CRString csAlternate( IDS_ALTERNATE_SIGNATURE32);

		CString strSig = GetIniString( IDS_INI_SIGNATURE_NAME);
		if(strSig == csStandard)
			m_Sum->SetSignature( csStandard );
		else if(strSig == csAlternate)
			m_Sum->SetSignature( csAlternate );
		
	}

#endif //COMMERCIAL

	// fix up the From: line
	if (IsRedirect())
		SetupRedirect();
	else
		SetHeaderLine( HEADER_FROM, GetReturnAddress() );

	// restore current persona if we changed it
	if ( ( Persona != CurPersona ) )
		g_Personalities.SetCurrent( CurPersona );

	return TRUE;
}

void CCompMessageDoc::ReadEudoraInfo(CString &sEudoraIniV2)
{
	char buf[_MAX_PATH+1];
	CString EudoraFile;
	GetIniString(IDS_INI_AUTO_RECEIVE_DIR, buf, sizeof(buf));
	if (buf[0] == '\0')
		EudoraFile = EudoraDir + CRString(IDS_ATTACH_FOLDER) 
						+ "\\" + CRString(IDS_REGISTER_EUDORA_COPY); 
	else
		EudoraFile = CString(buf) + "\\" + CRString(IDS_REGISTER_EUDORA_COPY);
	wsprintf(buf, "%s", EudoraFile);

	// Let's get the new INI file
	JJFile *pNewFile = new JJFile;
	if (!pNewFile)
		return;
	if (FAILED(pNewFile->Open(buf, O_CREAT | O_WRONLY)))
	{
		delete pNewFile;
        pNewFile = NULL;
		return;
	}

	// Let's get the Eudora INI file
	JJFile *pINIFile = new JJFile;
	if (!pINIFile)
		return;
	if (FAILED(pINIFile->Open((const char*)INIPath, O_RDONLY)))
	{
		delete pINIFile;
        pINIFile = NULL;
		return;
	}

	CString sLine;
	CString sPassword;
	sPassword = CRString(IDS_INI_SAVE_PASSWORD_TEXT);
	CString sDialUpPassword;
	sDialUpPassword = CRString(IDS_INI_SAVE_DIALUP_PASSWORD_TEXT);
	long lNumBytesRead = 0;

	do
	{
		char* pszLine = sLine.GetBuffer(1024);
		HRESULT hrGet = pINIFile->GetLine(pszLine, 1024, &lNumBytesRead);
		sLine.ReleaseBuffer();
		if (SUCCEEDED(hrGet) && (lNumBytesRead > 0))
		{
			if ((sLine.Find(sPassword) == -1) && (sLine.Find(sDialUpPassword) == -1))
				pNewFile->PutLine(sLine);
			sLine.Empty();
		}
	} while (lNumBytesRead > 0);

	if (lNumBytesRead != -1)
		sEudoraIniV2 = EudoraFile;

	pINIFile->Close();
	pNewFile->Close();
	
	delete pNewFile;
	delete pINIFile;
}

void CCompMessageDoc::ReadSystemInfo(CString &sAttach, CString &sBody)
{
	CString sEudoraIniV2;

	ReadEudoraInfo(sEudoraIniV2);
	
	// Attach Eudora_C.Ini
	if (!sEudoraIniV2.IsEmpty())
	{
		if (sAttach.IsEmpty() == FALSE)
		{
			if (';' != sAttach[sAttach.GetLength() - 1])
				sAttach += "; ";
			else if (' ' != sAttach[sAttach.GetLength() - 1])
				sAttach += ' ';
		}
		sAttach += sEudoraIniV2 + ";";
	}
	
	CString sPlatform;
	CString sVersion;
	CString sProcessor;
	CString sMachineType;
	CString sTotalPhys;
	CString sTotalVirtual;
	ReadPlatform(sPlatform, sVersion, sMachineType, sProcessor, sTotalPhys, sTotalVirtual);

	int i=0;
	CString sWinsock;

	if(QCWinSockLibMT::LoadWSLibrary())
	{
		WORD wVersionRequested = MAKEWORD(1, 1);
		WSADATA wsaData;
		for (i = 0; i < sizeof(wsaData.szDescription); i++)
			wsaData.szDescription[i] = 0;
		if (QCWinSockLibMT::WSAStartup(wVersionRequested, &wsaData) == 0)
		{
			sWinsock = wsaData.szDescription;
			QCWinSockLibMT::WSACleanup();
		}

	}

	sBody.Format(GetIniString(IDS_REGISTER_BODY2), (LPCTSTR)sPlatform, (LPCTSTR)sMachineType,
					(LPCTSTR)sVersion, (LPCTSTR)sProcessor, (LPCTSTR)sTotalPhys,
					(LPCTSTR)sTotalVirtual, (LPCTSTR)CRString(IDS_VERSION), GetIniString(IDS_INI_REG_NUMBER), (LPCTSTR)sWinsock);
}


void CCompMessageDoc::ReadPlatform(CString &sPlatform, CString &sVer,
								 CString &sMachineType, CString &sProcessor,
								 CString &sTotalPhys, CString &sTotalVirtual)
{
	sVer.Format("%lu.%02lu.%04u", GetMajorVersion(), GetMinorVersion(), LOWORD(GetBuildNumber()));
	if (IsWinNT())
		sPlatform.Format(GetIniString(IDS_REGISTER_WINNT), (LPCTSTR)sVer);
	else if (IsWin98())
		sPlatform.Format(GetIniString(IDS_REGISTER_WIN98), (LPCTSTR)sVer);
	else if (IsWin95())
		sPlatform.Format(GetIniString(IDS_REGISTER_WIN95), (LPCTSTR)sVer);
	else if (IsWin32s())
		sPlatform.Format(GetIniString(IDS_REGISTER_WIN32S), (LPCTSTR)sVer);
	
	char stemp[20]={0};
	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);
	switch(sinf.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_INTEL:
			sMachineType = CRString(IDS_REGISTER_PROCESS_INTEL);
			switch(sinf.wProcessorLevel)
			{
				case 3:
					sProcessor = "80386";
					break;
				case 4:
					sProcessor = "80486";
					break;
				case 5:
					sProcessor = "Pentium";
					break;
				case 6:
					sProcessor = "Pentium II";
					break;
				default:
					sProcessor = CString(ultoa(sinf.dwProcessorType, stemp, 10));
			}
			break;

		case PROCESSOR_ARCHITECTURE_MIPS:
			sMachineType = CRString(IDS_REGISTER_PROCESS_MIPS);
			sProcessor = CString(ultoa(sinf.dwProcessorType, stemp, 10)) + " " +
						CString(itoa(sinf.wProcessorLevel, stemp, 10));
			break;

		case PROCESSOR_ARCHITECTURE_ALPHA:
			sMachineType = CRString(IDS_REGISTER_PROCESS_ALPHA);
			sProcessor = CString(ultoa(sinf.dwProcessorType, stemp, 10)) + " " +
						CString(itoa(sinf.wProcessorLevel, stemp, 10));
			break;

		case PROCESSOR_ARCHITECTURE_PPC:
			sMachineType = CRString(IDS_REGISTER_PROCESS_PPC);
			switch(sinf.wProcessorLevel)
			{
				case 1:
					sProcessor = "601";
					break;
				case 3:
					sProcessor = "603";
					break;
				case 4:
					sProcessor = "604";
					break;
				case 6:
					sProcessor = "603+";
					break;
				case 9:
					sProcessor = "604+";
					break;
				case 20:
					sProcessor = "620";
					break;
				default:
					sProcessor = sMachineType;
					break;
			}
			break;
		default:
			sMachineType = CRString(IDS_REGISTER_PROCESS_UNKNOWN);
			sProcessor = sMachineType;
	}

	MEMORYSTATUS mst;
	mst.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&mst);
	sTotalPhys.Format("%lu Kb", (mst.dwTotalPhys / 1024L));
	sTotalVirtual.Format("%lu Kb", (mst.dwTotalVirtual / 1024L));
}

int QueueStatus = QS_NONE_QUEUED;

// SetQueueStatus
//
// Sets the QueueStatus flag to it's correct state.
//
void SetQueueStatus()
{
	CTocDoc* OutToc = GetOutToc();
	unsigned long	nextSendTime = 0;
	
	if (!OutToc)
		return;

	time_t Now = time(NULL);
	time_t Later = Now + 12 * 60L * 60L;		// 12 hours from now
	POSITION pos = OutToc->m_Sums.GetHeadPosition();
	
	QueueStatus = QS_NONE_QUEUED;
	while (pos)
	{
		CSummary* Sum = OutToc->m_Sums.GetNext(pos);
		if (Sum->IsQueued() && !Sum->m_FrameWnd)
		{
			time_t SumLocalSeconds = Sum->m_Seconds + Sum->m_TimeZoneMinutes * 60;

			if (Sum->m_State == MS_QUEUED || SumLocalSeconds <= Now)
				QueueStatus |= QS_READY_TO_BE_SENT;
			else if (SumLocalSeconds <= Later)
				QueueStatus |= QS_DELAYED_WITHIN_12;
			else
				QueueStatus |= QS_DELAYED_MORE_THAN_12;
			
			if (Sum->m_State == MS_TIME_QUEUED && Sum->GetDate()[0] != 0)
			{
				if (!nextSendTime || nextSendTime > (unsigned long)SumLocalSeconds)
					nextSendTime = 	SumLocalSeconds;
			}
			
		}
	}
	// Set time so mail will be sent when the time comes
 	((CEudoraApp *)AfxGetApp())->SetNextSendMail(nextSendTime);

	// Minimize icon may have to change to reflect queued message status
	((CMainFrame*)AfxGetMainWnd())->SetIcon();
}

/*

SendQueuedResult SendOne(CSummary* Sum,SendQueuedResult InResult, CFilterActions *filt)
{
	CTocDoc*			OutToc = GetOutToc();
	CCompMessageDoc*	doc;
	QCMailboxCommand*	pCommand;
	BOOL				CreatedDoc = FALSE;
						
	if (Sum->m_FrameWnd)
	{
		doc = (CCompMessageDoc*)Sum->m_FrameWnd->GetActiveDocument();
		if (doc->IsModified())
		{
			switch (AlertDialog(IDD_SEND_CHANGED, Sum->MakeTitle()))
			{
			case IDC_DONT_SEND:
				return (InResult);
			case IDOK:
				if (doc->OnSaveDocument(NULL) == FALSE)
					return (InResult);
				break;
			case IDC_SEND_ORIGINAL:
				// Get old contents back
				doc->SetModifiedFlag(FALSE);
				if (((CMessageDoc*)doc)->Read() == FALSE)
					return (InResult);
				break;
			}
		}
		doc->Queue(TRUE);
	}
		
//FORNOW	doc = (CCompMessageDoc*)Sum->GetMessageDoc();
	if (!(doc = (CCompMessageDoc *) Sum->FindMessageDoc()))
	{
		CreatedDoc = TRUE;
		doc = (CCompMessageDoc *) Sum->GetMessageDoc();
	}

	if (!doc || !doc->GetText())
		return (InResult);
	ASSERT_KINDOF(CCompMessageDoc, doc);
	
	// save the BCC line for processessing	
	char *OldLine = (char *)::SafeStrdupMT(doc->GetHeaderLine(HEADER_BCC));
	
	if (SendMessage(doc) == 1)
	{
		Sum->SetState(MS_SENT);
		SetIniShort(IDS_INI_SUCCESSFUL_SEND, 1);
		if (OutToc->GetView())
			OutToc->GetView()->UpdateWindow();

		//FCC Stuff
		if (OldLine)
		{
			int len = ::SafeStrlenMT(OldLine);
			CString sOldLine(OldLine);
			CString sMbox;
			BOOL endOfLine=FALSE;
			int ch=131;
			int beginPos=0;
			int endPos=0;
			BOOL bFound=FALSE;
			CString sTemp;
			CString sPath;
			CString sName;
			while (!endOfLine)
			{
				ch = 131;
				beginPos = sOldLine.Find(char(ch));
				if (beginPos == -1)
				{
					endOfLine=TRUE;
					continue;
				}
				beginPos++;
				ch = sOldLine[beginPos];
				beginPos++;
				sOldLine = sOldLine.Mid(beginPos);
				endPos = sOldLine.Find(',');
				if (endPos == -1)
				{
					endOfLine=TRUE;
					sMbox = sOldLine;
				}
				else
				{
					sMbox = sOldLine.Mid(0, endPos);
				}
				pCommand = g_theMailboxDirector.FindByNamedPath( sMbox );

				if( pCommand != NULL )
				{
					OutToc->Xfer(	GetToc(	pCommand->GetPathname(), 
											pCommand->GetName(),
											FALSE, 
											FALSE), 
									Sum, 
									FALSE, 
									TRUE);					
				}
				else
				{
					char sText[256];
					sprintf(sText, CRString(IDS_FCC_LOST_MBOX), sMbox);
					AfxMessageBox(sText);
				}
			}
			delete [] OldLine;
		}

		if ( !(filt->FilterOne(Sum, WTA_OUTGOING) & FA_TRANSFER ) &&
			 !Sum->KeepCopies())
		{
			OutToc->Xfer(GetTrashToc(), Sum, TRUE);
		}
			
		OutToc->SetModifiedFlag();
//FORNOW		delete doc;
		if (CreatedDoc)
		{
			Sum->NukeMessageDocIfUnused();
			doc = NULL;
		}
	}
	else
	{
//FORNOW		delete doc;
		if (CreatedDoc)
		{
			Sum->NukeMessageDocIfUnused();
			doc = NULL;
		}
			
		// If there wasn't something wrong with the message, then there
		// was a problem with the network, so quit sending
		if (Sum->m_State != MS_UNSENDABLE)
			return (SQR_MAJOR_ERROR);
			                
        // Otherwise, this message is amiss, so display it, and continue sending
		Sum->Display();
		return (SQR_UNSENT_MESSAGE);
	}
	
	return (InResult);
}

  */
BOOL FlushQueue = FALSE;

//extern CPOP* gPOP;

// SendQueuedMessages
//
// Send all messages that are currently queued.  If any fail, bring the window
// back so the user has a chance to edit the contents.
//
SendQueuedResult SendQueuedMessages(int WhichToSend /*= QS_READY_TO_BE_SENT*/,
									BOOL bMultiPersona /*= TRUE*/)
{
	/*
	static SendQueuedResult Result;
	BOOL CreatedNetConnection = (NetConnection == NULL);
	time_t Now = time(NULL);
	time_t Later = Now + 12 * 60L * 60L;	// 12 hours from now
	int NumToSend = 0;
	*/
	

	// Plugins want to know what that we're about to send
	((CEudoraApp *)AfxGetApp())->GetTranslators()->IdleEveryone(0, EMIDLE_PRE_SEND);

#ifdef THREADED
		if ( SUCCEEDED(SendQueuedMessages2(WhichToSend, bMultiPersona, TRUE)))
			return SQR_ALL_OK;
		else
			return SQR_MAJOR_ERROR;
#endif

}



/*
	char Server[128];
	POSITION pos;
	CSummary* Sum;

	Result = SQR_ALL_OK;	// assume the best in case...

	// If no messages meet the sending criteria, then we're done

	// This wont let expired messages be sent i
	//if (QueueStatus < WhichToSend)
	
	// get out'a here if there aren't any queued messages
	if (QueueStatus <= QS_NONE_QUEUED)
		return (SQR_ALL_OK);

	CTocDoc* OutToc = GetOutToc();
	if (!OutToc)
		return (Result);

	// Count how many messages to send
	for (pos = OutToc->m_Sums.GetHeadPosition(); pos; )
	{
		Sum = OutToc->m_Sums.GetNext(pos);
		if (Sum->m_State == MS_QUEUED ||
			(Sum->m_State == MS_TIME_QUEUED && (Sum->m_Seconds <= Now ||
			Sum->m_Seconds <= Later && WhichToSend <= QS_DELAYED_WITHIN_12 ||
			WhichToSend == QS_DELAYED_MORE_THAN_12)))
		{
			NumToSend++;
		}
	}

	if (!NumToSend)
		return (SQR_ALL_OK);

	CString	homie =	g_Personalities.GetCurrent();

	LPSTR lpPersonalities = g_Personalities.List();
	lpPersonalities += strlen( lpPersonalities ) + 1;	//Skip <Dominant>
	CString Persona = "";								//Still always start with the Default personality

	do
	{
		if ( bMultiPersona )
			g_Personalities.SetCurrent( Persona );
		else
			Persona = homie;	// send mail for the current persona

		if (!gPOP)
		{
			// If we haven't created the POP object yet, and we're sending via POP or we
			// have to validate the POP password, then check mail via the POP object methods
			if (GetIniShort(IDS_INI_USE_POP_SEND) ||
				(GetIniShort(IDS_REQUIRE_POP_LOGIN_TO_SEND) && POPPassword.IsEmpty()))
			{
				GetMail(kCheckPassword | kNetworkBit | kPOPBit | kLogonBit | kSendMailBit);
				return (Result);
			}
		}
		else if (GetIniShort(IDS_REQUIRE_POP_LOGIN_TO_SEND) && POPPassword.IsEmpty())
		{
			// If we did log on to the POP server, but the password failed and it's
			// required to send mail, then just get out of here
			return (SQR_MAJOR_ERROR);
		}

		GetIniString(IDS_INI_SMTP_SERVER, Server, sizeof(Server));
		::TrimWhitespaceMT(Server);
		char* s;
		if (!*Server && (s = strrchr(GetIniString(IDS_INI_POP_ACCOUNT), '@')))
			strcpy(Server, s + 1);
 
	   	// If the server name is Hesiod, StartSMTP queries the Hesiod server for the
		// users POP server.  The users SMTP Hesiod server is ignored at this time.
		CFilterActions filt;

		// are there any messages for this server?
		NumToSend = 0;
		for (pos = OutToc->m_Sums.GetHeadPosition(); pos; )
		{
			Sum = OutToc->m_Sums.GetNext(pos);
			if ( Sum->GetPersona() == Persona )
			{
				if (Sum->m_State == MS_QUEUED ||
					(Sum->m_State == MS_TIME_QUEUED && (Sum->m_Seconds <= Now ||
					Sum->m_Seconds <= Later && WhichToSend <= QS_DELAYED_WITHIN_12 ||
					WhichToSend == QS_DELAYED_MORE_THAN_12)))
				{
					NumToSend++;
				}
			}
		}

		if (NumToSend)
		{
			if ( StartSMTP(Server) >= 0 && DoSMTPIntro() >= 0 && filt.StartFiltering() )
			{
	    		CountdownProgress(CRString(IDS_SMTP_MESSAGES_LEFT), NumToSend);
    	
				Result = SQR_ALL_OK;

				for (pos = OutToc->m_Sums.GetHeadPosition(); pos && NumToSend && Result != SQR_MAJOR_ERROR; )
				{
					Sum = OutToc->m_Sums.GetNext(pos);
					if ( Sum->GetPersona() == Persona )
					{
						if (Sum->m_State == MS_QUEUED ||
							(Sum->m_State == MS_TIME_QUEUED && (Sum->m_Seconds <= Now ||
							Sum->m_Seconds <= Later && WhichToSend <= QS_DELAYED_WITHIN_12 ||
							WhichToSend == QS_DELAYED_MORE_THAN_12)))
						{
							CFilterActions *fptr = NULL;
							fptr = &filt;
							Result = SendOne(Sum, Result, fptr);

							NumToSend--;
							DecrementCountdownProgress();
						}
					}
				}
			}

			FlushQueue = FALSE;
			EndSMTP(Result == SQR_MAJOR_ERROR);
			if (OutToc->IsModified())
				OutToc->Write();
			SetQueueStatus();
			filt.EndFiltering();
		}

		// advance to next personality
		Persona = lpPersonalities;
		lpPersonalities += strlen( lpPersonalities ) + 1;

	} while ( bMultiPersona && ! Persona.IsEmpty() );

	g_Personalities.SetCurrent( homie );

	if ( NetConnection )
	{
		if (CreatedNetConnection && GetIniShort(IDS_INI_AUTO_CONNECTION))
		{
			delete NetConnection;
			NetConnection = NULL;
		}
		else
			NetConnection->NukeCursor();
	}

	if (OutToc->GetView())
		OutToc->GetView()->m_SumListBox.SetRedraw(TRUE);

	if (CreatedNetConnection)
		CloseProgress();

	return (Result);
}
*/



IMPLEMENT_DYNCREATE(CCompStationeryDoc, CCompMessageDoc)

CCompStationeryDoc::CCompStationeryDoc()
{
	m_File = NULL;
}

CCompStationeryDoc::~CCompStationeryDoc()
{
	delete m_Sum;
	delete m_File;
}

BOOL CCompStationeryDoc::Read(const char *Filename)
{
	if (!::FileExistsMT(Filename)) 
		return (FALSE);

	m_File = new JJFile();
	if (NULL == m_File)
		return FALSE;
	if (FAILED(m_File->Open(Filename, O_RDONLY)))
	{
		delete m_File;
		m_File = NULL;
		return FALSE;
	}
	
	const unsigned int MaxLength = 62 * 1024L;
	m_Text = new char[MaxLength + 1];
	m_BufSize = MaxLength;
	if (!m_Text)
	{
		m_BufSize = 0;
		return FALSE;
	}
	memset(m_Text,0,MaxLength);
	m_File->Read(m_Text, MaxLength);


	// get the Translation X-Header
	char *Trans = HeaderContents(IDS_TRANS_XHEADER, m_Text);

	// get the Signature X-Header
	char *cp;
	char *sigstr = HeaderContents(IDS_SIGNATURE_XHEADER, m_Text);
	if (sigstr)
	{
		cp = sigstr;

		if (*cp == '<' && *(cp + strlen(cp) - 1) == '>')
		{
			cp++;
			*(cp + strlen(cp) - 1) = '\0';
		}
	}

	// get the Persona X-Header
	char *cpPersona;
	char *Persona = HeaderContents(IDS_PERSONA_XHEADER, m_Text);
	if (Persona)
	{
		char * cp = Persona;

		if (*cp == '<' && *(cp + strlen(cp) - 1) == '>')
		{
			cp++;
			*(cp + strlen(cp) - 1) = '\0';
		}

		cpPersona = cp;
	}

	// Go through the message and parcel out each item
	char* t = m_Text;
	while (t && *t != '\r' && *t != '\n' && *t)
	{
		char* colon = strchr(t, ':');
		if (!colon)
		{
			t = strchr(t, '\n');
			if (t)
				t++;
			continue;
		}
		// Check if this is a header we use?
		int i = FindRStringIndexI(IDS_HEADER_TO, IDS_STATIONERY_TOC_HEADER, t, colon - t);

		// Get the value for the header
		if (!(t = strchr(colon, '\n')))
			continue;
		if (t[-1] == '\r')
			t[-1] = 0;
		*t++ = '\0';
		if (*++colon == ' ')
			colon++;
		// If found, fill in the value for display.  Otherwise,
		// move onto the next header.
		if (i >= 0 && i < MaxHeaders &&
			i != (IDS_HEADER_IN_REPLY_TO - IDS_HEADER_TO) &&
			i != (IDS_HEADER_REFERENCES - IDS_HEADER_TO) )
		{
			m_Headers[i] = colon;
		}
		else if (i == MaxHeaders)
		{
			m_Sum = new CSummary;
			WORD flgVal = WORD(atoi(colon));
			m_Sum->SetFlag(flgVal);
			char *c  = strchr(colon, ' ');
			if (c)
			{	
				colon = c;
				short prior = short(atoi(colon));
				m_Sum->m_Priority = prior;
				if (Trans)
				{	
					m_Sum->SetTranslators(Trans, TRUE);
					delete Trans;
				}
				if (sigstr)
				{
					m_Sum->m_SigSelected = m_Sum->m_SigHdr = cp;
					delete [] sigstr;
				}

				if (Persona)
				{
					m_Sum->SetPersona( cpPersona );
					delete Persona;
				}
			}

		}

	}


#ifndef unix
	int hasLF = 1;
#else
	int hasLF = 0;
#endif
	// find the begining of the body which start with a \n
	// step over it so the body start at the first line
	if (t && (t = strchr(t + hasLF, '\n')))
		strcpy(m_Text, t + 1);

	//Best place to stuff in the Pathname
	m_strPathName.Empty();
	m_strPathName = Filename;

	m_HasBeenSaved = TRUE;

	return (TRUE);
}

CSummary* NewMessageFromFile(const char *fileName)
{
	CTocDoc *	OutToc = NULL;
	JJFile *	in = NULL;
	JJFile *	out = NULL;
	JJFile *	tmp = NULL;
	CSummary *	theSum = NULL;
	CRString	MIMEVersionHeader( IDS_MIME_HEADER_VERSION );
	CRString	AttachHeader( IDS_HEADER_ATTACHMENTS );
	CRString	ToHeader( IDS_HEADER_TO );
	CRString	CCHeader( IDS_HEADER_CC );
	CRString	BCCHeader( IDS_HEADER_BCC );
	CString		AttachString;
	CString		FromString;
	CTime		Time(CTime::GetCurrentTime());
	long		lStartLoc;
	long		lEndLoc;
	long		lInSize;
	long		lBodySize;
	char *		t;
	BOOL		bMimeEncoded = FALSE;

	const unsigned int BufLength = 62 * 1024L;
	char * pBuf = new char[ BufLength + 1 ];
	if ( ! pBuf )
		goto done;

	OutToc = GetOutToc();
	if (!OutToc)
		goto done;

	in = new JJFile;
	if (!in)
		goto done;
	if (FAILED(in->Open(fileName, O_RDWR)))	// does a O_BINARY open
		goto done;
	
	out = new JJFile;
	if (!out)
		goto done;
	if (FAILED(out->Open(OutToc->MBFilename(), O_APPEND | O_RDWR)))
		goto done;
	
	// Get the offset of the start of the message
	out->Tell(&lStartLoc);
	ASSERT(lStartLoc >= 0);
	
	// Write From line
	if (Time.GetTime() < 0)
		Time = 0;
	FromString = ::FormatTimeMT(Time.GetTime(), GetIniString(IDS_FROM_CTIME_FORMAT));
	if (FAILED(out->PutLine(FromString))) goto done;
	
	// get in filesize
	in->Seek( 0L, SEEK_END );
	in->Tell(&lInSize);
	ASSERT(lInSize >= 0);
	in->Seek( 0L, SEEK_SET );

	// Write out headers
	pBuf[ BufLength ] = '\0';
	if ( FAILED(in->Read( pBuf, BufLength )) ) goto done;
	t = pBuf;
	while (t && *t != '\r' && *t != '\n' && *t)
	{
		// Check if this is a MIME-Version header?
		char * end = strchr(t, '\n');
		if ( end )
		{
			if (end[-1] == '\r')	// don't duplicate \r or \n
				end[-1] = '\0';
			end[0] = '\0';

			if ( strnicmp( t, MIMEVersionHeader, strlen( MIMEVersionHeader ) ) == 0 )
				bMimeEncoded = TRUE;

			if ( strnicmp( t, AttachHeader, strlen( AttachHeader ) ) == 0 )
				AttachString = t;
			else 
				if( GetIniShort(IDS_INI_AUTO_EXPAND_NICKNAMES) && ( 
					strnicmp( t, ToHeader, strlen( ToHeader ) ) == 0 ||
					strnicmp( t, CCHeader, strlen( CCHeader ) ) == 0 ||	
					strnicmp( t, BCCHeader, strlen( BCCHeader ) ) == 0 ) )
				{
					char* ExpandedText, *colon;
					if ( (colon = strstr(t, ": ")) == 0) goto done;
					
					if (*(colon + 2))
					{
						ExpandedText = ExpandAliases(colon + 2, TRUE, FALSE, FALSE);
						if (ExpandedText)
						{
							if ( FAILED( out->Put( t, (colon + 2) - t ) ) ) goto done;
							if ( FAILED( out->PutLine( ExpandedText, strlen( ExpandedText ) ) ) ) goto done;
							delete [] ExpandedText;
						}
						else
							if ( FAILED( out->PutLine( t, strlen( t ) ) ) ) goto done;
					}
					else
						if ( FAILED( out->PutLine( t, strlen( t ) ) ) ) goto done;
				}
				
			else
				if ( FAILED( out->PutLine( t, strlen( t ) ) ) ) goto done;

			t = end + 1;
		}
		else
			ASSERT( FALSE );	// I expect to find new lines
	}

	lBodySize = lInSize - ( t - pBuf );

	// handle body
	if ( bMimeEncoded )
	{
		// if MIME-Version detected write the body to a temp file
		CString tmpname = ::GetTmpFileNameMT( "rr" );
		tmp = new JJFile;
		if (!tmp)
			goto done;
		if (FAILED(tmp->Open(tmpname, O_CREAT | O_TRUNC | O_RDWR)))		// does a O_BINARY open
			goto done;

		// save the body filename in X-Attachments
		char buf[ 512 ];
		sprintf( buf, "%s %s;", (const char*)AttachHeader, (const char*)tmpname );
		if ( FAILED( out->PutLine( buf, strlen( buf ) ) ) ) goto done;
		if ( FAILED( out->PutLine( "\r\n", 2 ) ) ) goto done;	// delimit header

		// t now points to the header/body separator (blank-line)
		while ( lBodySize )
		{
			long bytes = BufLength - ( t - pBuf );
			if ( lBodySize < bytes )
				bytes = lBodySize;

			if ( FAILED(tmp->Put( t, bytes )) ) goto done;

			lBodySize -= bytes;
			if ( lBodySize )
			{
				t = pBuf;
				if ( FAILED(in->Read( pBuf, BufLength )) ) goto done;
			}
		}

		tmp->Flush();
		tmp->Close();
	}
	else
	{
		// use original messages X-Attachment specifier
		if ( ! AttachString.IsEmpty() )
			if ( FAILED( out->PutLine( AttachString, AttachString.GetLength() ) ) ) goto done;

		// not MIME-encode so write body to the OutToc (not to exceed 62K)
		long bytes = BufLength - ( t - pBuf );
		if ( lBodySize < bytes )
			bytes = lBodySize;

		if( FAILED(out->Put( t, bytes )) ) goto done;
	}

	// Get the offset of the end of the message
	out->Tell(&lEndLoc);
	ASSERT(lEndLoc >= 0);
	out->Seek( lStartLoc );	// prepare for CSummary::Build()

	OutToc->Write();

	// init the CSummary object
	theSum = new CSummary;
	theSum->m_TheToc = OutToc;
	CSummary::m_lBegin = lStartLoc;
	theSum->Build( out, TRUE );
	theSum->SetState( MS_QUEUED );
	if (GetIniShort(IDS_INI_KEEP_COPIES))	
		theSum->SetFlag( MSF_KEEP_COPIES );
	if (bMimeEncoded)
		theSum->SetFlagEx( MSFEX_AUTO_ATTACHED );

	// add the summary to the out mailbox
	OutToc->AddSum( theSum );
	SetQueueStatus();

done:
	delete in;
	delete out;
	delete tmp;
	delete [] pBuf;

	return theSum;
}


void CCompMessageDoc::OnMessageDelete()
{
	CSummary* Sum = m_Sum;
	CTocDoc* Toc = Sum->m_TheToc;
	
	// If we haven't done anything to this message, then just get rid of it
	if (!IsModified() && !m_HasBeenSaved)
	{
		OnCloseDocument();
		Toc->RemoveSum(Sum);
	}
	else
	{
		if (Sum->CantEdit() == FALSE)
		{
			if (Sum->IsQueued() && GetIniShort(IDS_INI_WARN_DELETE_QUEUED) &&
				WarnDialog(IDS_INI_WARN_DELETE_QUEUED, IDS_WARN_DELETE_QUEUED) != IDOK)
			{
				return;
			}
			else if ((Sum->IsSendable() || Sum->m_State == MS_UNSENDABLE) &&
				GetIniShort(IDS_INI_WARN_DELETE_UNSENT) &&
				WarnDialog(IDS_INI_WARN_DELETE_UNSENT, IDS_WARN_DELETE_UNSENT) != IDOK)
			{
				return;
			}
		}

		if ( (IsModified() == FALSE) || (SaveModified()) )
			Toc->Xfer(GetTrashToc(), Sum);
    }
}


void CCompMessageDoc::OnSend()
{
	if (m_Sum && m_Sum->CantEdit() == FALSE)
	{
		if (ShiftDown())
			OnChangeQueueing();
		else
		{
			Queue();
			SetQueueStatus();
			if (GetIniShort(IDS_INI_IMMEDIATE_SEND))
				SendQueuedMessages();
		}
	}
}

void CCompMessageDoc::OnCanModify(CCmdUI* pCmdUI)
{
	if(m_Sum != NULL)
		pCmdUI->Enable(m_Sum->CantEdit() == FALSE);
}

void CCompMessageDoc::OnFileSave()
{
	CMessageDoc::OnFileSave();

	if (m_bIsStationery)
	{
		//MsgDoc initializes it to "A", so if it hasn't been
		//saved before call the SaveAs function
		if ( m_strPathName.Compare("A") == 0 )
		{
			OnFileSaveAsStationery();
			return;
		}

		JJFile theFile;

		if (FAILED(theFile.Open( m_strPathName, O_CREAT | O_TRUNC | O_WRONLY)))
		return;
	
		WriteAsText( &theFile, TRUE );

	}
}

void CCompMessageDoc::OnFileSaveAs()
{
	char		szName[_MAX_PATH + 1];
	CString		szPathName;
	JJFile		theFile;

	if( m_Sum == NULL )
	{
		return;
	}

	strcpy( szName, m_Sum->m_Subject );

	::StripIllegalMT( szName, EudoraDir );

	if( !::LongFileSupportMT( EudoraDir ) )
	{
		szName[8] = 0;
	}


	CSaveAsDialog theDlg(	szName, 
							TRUE, 
							//TRUE, 
							FALSE,
							CRString( IDS_TEXT_EXTENSION ), 
							CRString( IDS_TXT_HTML_FILE_FILTER ), 
							NULL );
		
	
	if ( theDlg.DoModal() != IDOK )
	{
		return;
	}
	
	//
	// Hack alert!  Under the 32-bit Version 4 shell, the OnOK()
	// method of dialog doesn't get called!  Therefore, this is a
	// hack workaround to manually update these INI settings
	// outside of the dialog class.  Whatta hack.
	//
	if (IsVersion4())
	{
		SetIniShort(IDS_INI_INCLUDE_HEADERS, ( short ) theDlg.m_Inc );
		SetIniShort(IDS_INI_GUESS_PARAGRAPHS, ( short ) theDlg.m_Guess);
	}

	szPathName = theDlg.GetPathName();
	
	//bIsStationery = theDlg.m_IsStat;

	// determine whether or not this is stationery by the file extension      
	//if ( !bIsStationery && 
	//	 ( szPathName.Right( 3 ).CompareNoCase(	CRString( IDS_STATIONERY_EXTENSION ) ) == 0 ) ) 
	//{                             
	//	bIsStationery = TRUE;			
	//}		


	if (FAILED(theFile.Open( szPathName, O_CREAT | O_TRUNC | O_WRONLY)))
		return;
	
	//WriteAsText( &theFile, bIsStationery );
	SaveAsFile(&theFile, szPathName);
}


BOOL CCompMessageDoc::SaveAsFile( JJFile *theFile, CString szPathName)
{

	if ( !theFile || szPathName.IsEmpty() )
		return (FALSE);

	CView* pView= GetCompView();

	QCProtocol*	view = QCProtocol::QueryProtocol( QCP_GET_MESSAGE, ( CObject* )pView );

	if (!view) 
		return (FALSE);

	CString msg;

	if ( (szPathName.Right( 3 ).CompareNoCase( CRString( IDS_HTM_EXTENSION) ) == 0 ) ||
		 (szPathName.Right( 4 ).CompareNoCase( CRString( IDS_HTML_EXTENSION) ) == 0 ) )
		view->GetMessageAsHTML(msg, GetIniShort( IDS_INI_INCLUDE_HEADERS ));
	else
	{
		view->GetMessageAsText(msg, GetIniShort( IDS_INI_INCLUDE_HEADERS ));

		if ( GetIniShort( IDS_INI_GUESS_PARAGRAPHS ) )
		{
			char* CopyText = ::SafeStrdupMT( msg );
			if (FAILED(theFile->Put( UnwrapText( CopyText ))))
			{
				delete CopyText;
				return (FALSE);
			}
			else 
			{
				delete CopyText;
				return (TRUE);
			}
		}
	}

	if ( FAILED( theFile->Put( msg ) ) )
	{
		ASSERT( 0 );
		return (FALSE);
	}

	return (TRUE);
}

void CCompMessageDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


long CCompMessageDoc::DoContextMenu(
CWnd*	pCaller,
WPARAM	wParam, 		// HWND of window receiving WM_CONTEXTMENU message
LPARAM	lParam)			// WM_CONTEXTMENU screen coordinates
{				  
	CPoint ptScreen(LOWORD(lParam), HIWORD(lParam));

	// Get the CMenu that contains all the context popups
	CMenu popupMenus;
	HMENU hMenu = QCLoadMenu(IDR_CONTEXT_POPUPS);
	if ( ! hMenu || !popupMenus.Attach(hMenu) )
	   return FALSE;
	
	CMenu* pTempPopupMenu = popupMenus.GetSubMenu(MP_POPUP_COMP_MSG);
	if (pTempPopupMenu != NULL)
	{
		//
		// Since the popup menu we get from GetSubMenu() is a pointer
		// to a temporary object, let's make a local copy of the
		// object so that we have explicit control over its lifetime.
		//
		// Note that we edit the context menu on-the-fly in order to
		// add a number of "user-defined" context menus, display the edited 
		// context menu, then remove the added "user-defined" context menus.
		//
		// This all works because we add the sub-menus in a certain order,
		// and then remove them in exactly the reverse order.  Be careful
		// if you make changes to the processing order here.
		//
		CMenu tempPopupMenu;
		tempPopupMenu.Attach(pTempPopupMenu->GetSafeHmenu());

		//
		// Insert the Attach (plug-ins) sub-menu.
		//
		CMenu theAttachMenu;
		theAttachMenu.CreatePopupMenu();
		g_thePluginDirector.NewMessageCommands( CA_ATTACH_PLUGIN, &theAttachMenu );
		::WrapMenu( theAttachMenu.GetSafeHmenu() );
		tempPopupMenu.InsertMenu(	MP_ATTACH_PLUGINS,
									MF_BYPOSITION | MF_POPUP,
									(UINT) theAttachMenu.GetSafeHmenu(), 
									CRString(IDS_ATTACH_MENU_TEXT));

		//
		// Insert the Insert Recipient sub-menu.
		//
		CMenu theRecipientMenu;
		theRecipientMenu.CreatePopupMenu();
		g_theRecipientDirector.NewMessageCommands( CA_INSERT_RECIPIENT, &theRecipientMenu );
		::WrapMenu( theRecipientMenu.GetSafeHmenu() );
		tempPopupMenu.InsertMenu(	MP_INSERT_RECIP,
									MF_BYPOSITION | MF_POPUP,
									(UINT) theRecipientMenu.GetSafeHmenu(), 
									CRString(IDS_INSERT_RECIPIENT));


		CMenu theFCCMenu;

		// Shareware: Only allow FCC in FULL FEATURE version.
		if (UsingFullFeatureSet())
		{
			// FULL FEATURE mode

			//
			// Insert the FCC sub-menu.  This includes a fixed "New..." 
			// mailbox command ID in the root of the FCC sub-menu, just
			// after the In/Out/Trash items.
			//
			theFCCMenu.CreatePopupMenu();		
			g_theMailboxDirector.NewMessageCommands( CA_INSERT_FCC, &theFCCMenu, CA_FCC_NEW );
			theFCCMenu.InsertMenu( 3, MF_BYPOSITION, ID_FCC_NEW_MBOX_IN_ROOT, CRString( IDS_MAILBOX_NEW ) );
			theFCCMenu.InsertMenu( 3, MF_BYPOSITION | MF_SEPARATOR );
			::WrapMenu( theFCCMenu.GetSafeHmenu() );
			tempPopupMenu.InsertMenu(	MP_INSERT_RECIP,
										MF_BYPOSITION | MF_POPUP,
										(UINT) theFCCMenu.GetSafeHmenu(), 
										CRString( IDS_FCC ) );
		}

#ifdef COMMERCIAL
			//
			// Insert the Change Persona sub-menu
			//
			CMenu theChangePersonaMenu;
			theChangePersonaMenu.CreatePopupMenu();
			g_thePersonalityDirector.NewMessageCommands( CA_CHANGE_PERSONA, &theChangePersonaMenu );
			::WrapMenu( theChangePersonaMenu.GetSafeHmenu() );
			int nChangePersonaPosition = tempPopupMenu.GetMenuItemCount() - 1;
			tempPopupMenu.InsertMenu(	nChangePersonaPosition,
										MF_BYPOSITION | MF_POPUP,
										(UINT) theChangePersonaMenu.GetSafeHmenu(), 
										CRString( IDS_CHANGE_PERSONA ) );
#endif // COMMERCIAL

		//
		// Insert the Message Plug-Ins sub-menu.
		//
		CMenu theMessagePluginsMenu;
		theMessagePluginsMenu.CreatePopupMenu();
		g_thePluginDirector.NewMessageCommands( CA_TRANSLATE_PLUGIN, &theMessagePluginsMenu );
		int nMessagePluginsPosition = tempPopupMenu.GetMenuItemCount();
		::WrapMenu( theMessagePluginsMenu.GetSafeHmenu() );
		tempPopupMenu.InsertMenu(	nMessagePluginsPosition,
									MF_BYPOSITION | MF_POPUP,
									(UINT) theMessagePluginsMenu.GetSafeHmenu(),
									CRString(IDS_MESSAGE_PLUGINS));

		CContextMenu::MatchCoordinatesToWindow(HWND(wParam), ptScreen);
		tempPopupMenu.TrackPopupMenu(0, ptScreen.x, ptScreen.y, AfxGetMainWnd());

		//
		// Remove the Message Plug-Ins sub-menu.
		//
		g_thePluginDirector.RemoveMessageCommands( CA_TRANSLATE_PLUGIN, &theMessagePluginsMenu );
		tempPopupMenu.RemoveMenu( nMessagePluginsPosition, MF_BYPOSITION );

#ifdef COMMERCIAL
		//
		// Remove the Change Persona sub-menu.
		//
		g_thePersonalityDirector.RemoveMessageCommands( CA_CHANGE_PERSONA, &theChangePersonaMenu );
		tempPopupMenu.RemoveMenu( nChangePersonaPosition, MF_BYPOSITION);
#endif // COMMERCIAL

		// Shareware: Only allow FCC in FULL FEATURE version.
		if (UsingFullFeatureSet())
		{
			// FULL FEATURE mode

			//
			// Remove the FCC sub-menu.
			//
			g_theMailboxDirector.RemoveMessageCommands( CA_INSERT_FCC, &theFCCMenu );
			g_theMailboxDirector.RemoveMessageCommands( CA_FCC_NEW, &theFCCMenu );
			tempPopupMenu.RemoveMenu(MP_INSERT_RECIP, MF_BYPOSITION);
		}

		//
		// Remove the Insert Recipient sub-menu.
		//
		g_theRecipientDirector.RemoveMessageCommands( CA_INSERT_RECIPIENT, &theRecipientMenu );
		tempPopupMenu.RemoveMenu(MP_INSERT_RECIP, MF_BYPOSITION);

		//
		// Remove the Attach (plug-ins) sub-menu.
		//
		g_thePluginDirector.RemoveMessageCommands(CA_ATTACH_PLUGIN, &theAttachMenu);
		tempPopupMenu.RemoveMenu(MP_ATTACH_PLUGINS, MF_BYPOSITION);

		VERIFY(tempPopupMenu.Detach());
	}
	
	return TRUE;
}

void CCompMessageDoc::InsertFCCInBCC(QCMailboxCommand* pCommand)
{
	// Shareware: Only allow FCC in FULL FEATURE version.
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		CString	szNewBCC;
		char*	pBCCString;

		if( pCommand == NULL )
		{
			ASSERT( 0 );
			return;
		}

		szNewBCC = g_theMailboxDirector.BuildNamedPath( pCommand );
		szNewBCC = "\x83\\" + szNewBCC;

		pBCCString = ( char* ) GetHeaderLine(HEADER_BCC); 

		if( ::SafeStrlenMT( pBCCString ) )
		{
			szNewBCC = "," + szNewBCC;
			szNewBCC = pBCCString + szNewBCC;
		}
			
		SetHeaderLine( HEADER_BCC, szNewBCC );
		SetModifiedFlag( TRUE );
	}
	else
	{
		ASSERT(0); // No FCC in REDUCED FEATURE mode
	}
}

void CCompMessageDoc::OnFCCNewInRoot()
{
	// Shareware: Only allow FCC in FULL FEATURE version.
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		QCMailboxCommand*	pCommand = NULL;
		pCommand = g_theMailboxDirector.CreateTargetMailbox( NULL, FALSE );

		if( pCommand )
		{
			ASSERT_KINDOF( QCMailboxCommand, pCommand );
			ASSERT( pCommand->GetType() == MBT_REGULAR );

				InsertFCCInBCC( pCommand );
		}
	}
	else
	{
		ASSERT(0); // FCC not allowed in REDUCED FEATURE mode
	}
}



////////////////////////////////////////////////////////////////////////
// OnDynamicCommand [protected]
//
// Handles commands specific to a comp message.  Generic stuff like
// the Transfer commands are handled in the base class CMessageDoc.
////////////////////////////////////////////////////////////////////////
BOOL CCompMessageDoc::OnDynamicCommand(UINT uID)
{
	QCCommandObject*		pCommand;
	COMMAND_ACTION_TYPE		theAction;	
	CString					szTo;
	struct TRANSLATE_DATA	theData;

	if( ! g_theCommandStack.GetCommand( ( WORD ) uID, &pCommand, &theAction ) )
	{
		return FALSE;
	}

	if( ( pCommand == NULL ) || !theAction )
	{
		return FALSE;
	}

	if( theAction == CA_FCC_NEW )
	{
		// Shareware: Only allow FCC in FULL FEATURE version.
		if (UsingFullFeatureSet())
		{
			// FULL FEATURE mode
			ASSERT_KINDOF( QCMailboxCommand, pCommand );

			pCommand = g_theMailboxDirector.CreateTargetMailbox( ( QCMailboxCommand* ) pCommand, FALSE );
			if( pCommand )
			{
				ASSERT_KINDOF( QCMailboxCommand, pCommand );
				ASSERT( ( ( QCMailboxCommand* ) pCommand)->GetType() == MBT_REGULAR );
				theAction = CA_INSERT_FCC;
			}
			else
			{
				return TRUE;
			}
		}
		else
		{
			// REDUCED FEATURE mode
			ASSERT(0); // Should not get here -- FCC disabled in REDUCED FEATURE mode
			return FALSE;
		}
	}

	if( theAction == CA_INSERT_FCC )
	{
		// Shareware: Only allow FCC in FULL FEATURE version.
		if (UsingFullFeatureSet())
		{
			// FULL FEATURE mode
			ASSERT_KINDOF( QCMailboxCommand, pCommand );
			InsertFCCInBCC( ( QCMailboxCommand* ) pCommand );
			return TRUE;
		}
		else
		{
			// REDUCED FEATURE mode
			ASSERT(0); // Should not get here -- FCC disabled in REDUCED FEATURE mode
			return FALSE;
		}
	}

	if( theAction == CA_ATTACH_PLUGIN )
	{
		pCommand->Execute( theAction, this );
		return TRUE;		
	}
	
	if( theAction == CA_TRANSLATE_PLUGIN )
	{
		if( ( theData.m_pView = GetView() ) != NULL ) 
		{
			QCProtocol*	pProtocol = QCProtocol::QueryProtocol( QCP_TRANSLATE, ( CObject* )theData.m_pView);
			if( pProtocol == NULL ) 
				return FALSE;
			
			theData.m_pProtocol = pProtocol;
			theData.m_bBuildAddresses = TRUE;
			
			pCommand->Execute( theAction, &theData );

			return TRUE;
		}
	}

	if ( theAction == CA_CHANGE_PERSONA )
	{
		ASSERT_KINDOF( QCPersonalityCommand, pCommand );
		if ( ApplyPersona( ((QCPersonalityCommand *) pCommand)->GetName() ) )
		{
			SetModifiedFlag( TRUE );
		}
		else
		{
			ASSERT(0);		// bogus persona name
		}

		return TRUE;
	}

	if ( (theAction == CA_TRANSFER_NEW ) ||
		 (theAction == CA_TRANSFER_TO ) )
	{
		//
		// Normally, the CMessageDoc base class handles the Transfer commands.
		// However, Transfer is not applicable to stationery, so bounce
		// those here.
		//
		if (m_bIsStationery)
		{
			ASSERT(0);			// theoretically shouldn't get here due to CmdUI
			return TRUE;
		}
	}

	if ( theAction == CA_FORWARD_TO )
	{
		VERIFY(m_Sum != NULL);
		pCommand->Execute( theAction, m_Sum );
		return TRUE;
	}

	return CMessageDoc::OnDynamicCommand( uID );
}


////////////////////////////////////////////////////////////////////////
// OnUpdateDynamicCommand [protected]
//
// Handles commands specific to a comp message.  Generic stuff like
// the Transfer commands are handled in the base class CMessageDoc.
////////////////////////////////////////////////////////////////////////
void CCompMessageDoc::OnUpdateDynamicCommand(
CCmdUI* pCmdUI)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	

	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) ( pCmdUI->m_nID ), &pCommand, &theAction ) )
		{
			if( theAction == CA_CHANGE_PERSONA )
			{
				QCPersonalityCommand* pPC = DYNAMIC_DOWNCAST(QCPersonalityCommand, pCommand);
				BOOL bPersonaMatch = FALSE;
				if (pPC && m_Sum)
				{
					LPCTSTR ThisPersona = m_Sum->GetPersona();
					LPCTSTR CommandPersona = pPC->GetName();
					if (stricmp(ThisPersona, CommandPersona) == 0 ||
						(!*ThisPersona && stricmp(CommandPersona, CRString(IDS_DOMINANT)) == 0))
					{
						bPersonaMatch = TRUE;
					}
				}
				pCmdUI->SetRadio( bPersonaMatch );
				pCmdUI->Enable( TRUE );
				return;
			}

			if( ( theAction == CA_FCC_NEW ) ||
				( theAction == CA_ATTACH_PLUGIN ) ||
				( ( theAction == CA_TRANSLATE_PLUGIN ) && ( GetView() != NULL ) ) ||
				( theAction == CA_INSERT_FCC ) ||
				( theAction == CA_FORWARD_TO ) )
			{
				pCmdUI->Enable( TRUE );
				return;
			}

			if ( ( theAction == CA_TRANSFER_NEW ) ||
				 ( theAction == CA_TRANSFER_TO ) )
			{
				//
				// In the CMessageDoc base class, Transfer commands
				// are always enabled.  Therefore, we need to override
				// that for stationery.
				//
				if ( m_bIsStationery )
				{
					pCmdUI->Enable(FALSE);
					return;
				}
			}
		}
	}

	CMessageDoc::OnUpdateDynamicCommand(pCmdUI);
}




BOOL CCompMessageDoc::WriteAsText(
JJFile* pFile,
BOOL	bIsStationery )
{
	char*		szBody;

	if( m_Sum == NULL )
	{
		return FALSE;
	}

	// GetText() makes sure message is read up and returns body
	szBody = GetText();

	if( bIsStationery || GetIniShort( IDS_INI_INCLUDE_HEADERS ) )
	{
		for (int i = 0; i < MaxHeaders; i++)
		{
			if (FAILED(pFile->Put(CRString(IDS_HEADER_TO + i))) ||
			    FAILED(pFile->Put(" ")) || 
				FAILED(pFile->Put(GetHeaderLine(i))) ||
			    FAILED(pFile->Put("\r\n")))
			{
				return FALSE;
			}
		}
		
		// add a header for toc stuff
		if( bIsStationery )
		{
			CString tocFlags;
			tocFlags.Format(CRString(IDS_STATIONERY_TOC_HEADER_FORMAT),m_Sum->GetFlags(), m_Sum->m_Priority);
			pFile->Put(tocFlags);
			pFile->Put("\r\n");

			// Put the Translator Header
			CString TransString;
			CString Tltrs = m_Sum->GetTranslators();
			if (!Tltrs.IsEmpty())
			{
				TransString.Format(CRString(IDS_TRANS_XHEADER_FORMAT),(const char *)Tltrs);
				pFile->Put(TransString);
				pFile->Put("\r\n");
			}
		
			// Put the Signature Header
			CString SigString = m_Sum->m_SigSelected;
			
			if (!SigString.IsEmpty())
			{
				SigString.Format(CRString(IDS_SIGNATURE_XHEADER_FORMAT),(const char *)( m_Sum->m_SigSelected ) );
				pFile->Put(SigString);
				pFile->Put("\r\n");
				m_Sum->m_SigHdr = m_Sum->m_SigSelected;
			}

			// Put the Persona Header
			CString PersonaString;
			CString Persona = m_Sum->GetPersona();
			if (!Persona.IsEmpty())
			{
				PersonaString.Format(CRString(IDS_PERSONA_XHEADER_FORMAT),(const char *)Persona);
				pFile->Put(PersonaString);
				pFile->Put("\r\n");
			}
		}
		
		if( FAILED(pFile->Put("\r\n")) )
		{
			return FALSE;
		}
	}

	
	if ( !bIsStationery && GetIniShort( IDS_INI_GUESS_PARAGRAPHS ) )
	{
		char* CopyText = ::SafeStrdupMT( szBody );
		BOOL Status = TRUE;
		if (FAILED(pFile->Put( UnwrapText( CopyText ))))
			Status = FALSE;
		delete CopyText;
		return Status;
	}

	//((PgMsgView*)GetView())->SaveInfo();
	//szBody = CMessageDoc::GetText(); 
	if ( FAILED(pFile->Put( szBody )) )
		return FALSE;
	return TRUE;
}

void CCompMessageDoc::OnFileSaveAsStationery()
{
	char		szName[_MAX_PATH + 1];
	CString		szPathName;
	JJFile		theFile;

	if( m_Sum == NULL )
	{
		return;
	}

	strcpy( szName, m_Sum->m_Subject );

	::StripIllegalMT( szName, EudoraDir );

	if( !::LongFileSupportMT( EudoraDir ) )
	{
		szName[8] = 0;
	}


	CString	statDir = EudoraDir;

	if (::LongFileSupportMT(EudoraDir))
		statDir += CRString(IDS_STATIONERY_FOLDER);
	else
		statDir += CRString(IDS_STATIONERY_FOLDER16); 

	CString defExt = CRString(IDS_STATIONERY_EXTENSION);
	CString dlgTitle = CRString(IDS_SAVE_AS_STATIONERY_TITLE);

	CFileDialog theDlg(	FALSE, 
						CRString(IDS_STATIONERY_EXTENSION), 
						szName, 
						OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | 
						OFN_OVERWRITEPROMPT | OFN_EXTENSIONDIFFERENT ,
						CRString( IDS_STATIONERY_FILE_FILTER ),
						AfxGetMainWnd() );
	theDlg.m_ofn.lpstrInitialDir = statDir;
	theDlg.m_ofn.lpstrDefExt = LPCTSTR(defExt);
	theDlg.m_ofn.lpstrTitle = LPCTSTR(dlgTitle);

	if (theDlg.DoModal() == IDOK)
	{
		szPathName = theDlg.GetPathName();
		CString dir = szPathName;
		//CString fileTitle = theDlg.GetFileTitle();

		//Append .sta even if the user entered a different extension
		//CString ext = theDlg.GetFileExt();
		//if (ext.Compare(CRString(IDS_STATIONERY_EXTENSION)) != 0)
		//{
		//	fileTitle += "." + ext;
		//	szPathName += ("." + CRString(IDS_STATIONERY_EXTENSION));
		//}

		int s = szPathName.ReverseFind(SLASH);
		if (s > 0)
			dir = szPathName.Left( s );

		if (dir.CompareNoCase(statDir) == 0 ) 
		{
			if ( ! theDlg.GetFileExt().Compare(CRString(IDS_STATIONERY_EXTENSION)) )
			{
				QCStationeryCommand* pCommand;

				pCommand = g_theStationeryDirector.AddCommand( theDlg.GetFileTitle() );
				
				//if( pCommand )		
				//	pCommand->Execute( CA_NEW );
			}
		}

		if (FAILED(theFile.Open( szPathName, O_CREAT | O_TRUNC | O_WRONLY)))
		return;
	
		CMessageDoc::OnFileSave();
		WriteAsText( &theFile, TRUE );
		m_strPathName = theDlg.GetPathName();
		SetTitle(m_strPathName);
	}
	
}


BOOL CCompMessageDoc::GetMessageHeaders(CString& hdrs)
{
	//This is for PaigeStuff. 
	//Will be changing it back to use \r\n instead of just \n, so that the
	//current WriteAsText function can use this piece of code
	
	hdrs.Empty();

	for (int i = 0; i < MaxHeaders; i++)
	{
		hdrs +=	( CRString(IDS_HEADER_TO + i) + " " + 
				GetHeaderLine(i) + "\n" );

	}
	
	// add extra header for the stationery stuff
	if( m_bIsStationery )
	{
		//Put the Stationery Header
		CString tocFlags;
		tocFlags.Format(CRString(IDS_STATIONERY_TOC_HEADER_FORMAT),m_Sum->GetFlags(), m_Sum->m_Priority);
		hdrs += (tocFlags + "\n"); 

		// Put the Translator Header
		CString TransString;
		CString Tltrs = m_Sum->GetTranslators();
		if (!Tltrs.IsEmpty())
		{
			TransString.Format(CRString(IDS_TRANS_XHEADER_FORMAT),(const char *)Tltrs);
			hdrs += ( TransString + "\n" );
		}
	
		// Put the Signature Header
		CString SigString = m_Sum->m_SigSelected;
		
		if (!SigString.IsEmpty())
		{
			SigString.Format(CRString(IDS_SIGNATURE_XHEADER_FORMAT),(const char *)( m_Sum->m_SigSelected ) );
			hdrs += ( SigString + "\n" );
			m_Sum->m_SigHdr = m_Sum->m_SigSelected;
		}

		// Put the Persona Header
		CString PersonaString;
		CString Persona = m_Sum->GetPersona();
		if (!Persona.IsEmpty())
		{
			PersonaString.Format(CRString(IDS_PERSONA_XHEADER_FORMAT),(const char *)Persona);
			hdrs += (PersonaString + "\n");
		}
	}
	
	hdrs += "\n" ;
	
	return ( TRUE );
}
