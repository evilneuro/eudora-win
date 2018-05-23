// EUDORA.CPP
//

#include "stdafx.h"
#include <afxcmn.h>
#include <afxrich.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <afxmt.h>

#include "QCUtils.h"

#include "resource.h"
#include "mainfrm.h"
#include "mdichild.h"
#include "mdidrag.h"
#include "doc.h" 
#include "textfile.h"
#include "rs.h"
#include "debug.h"
#include "cursor.h"
#include "fileutil.h"
#include "guiutils.h"
#include "QCWorkerSocket.h"
#include "summary.h"					   
#include "tocdoc.h"
#include "tocview.h"
#include "font.h"
#include "nickdoc.h"
#include "signatur.h"
#include "compmsgd.h"
#include "utils.h"
#include "TipOfDayDlg.h"

#include "TridentPreviewView.h"	// Just so I can use IsKindOf() on a window when sending mousewheels.
#include "CompMessageFrame.h"
#include "ReadMessageFrame.h"

#include "pop.h"
#include "HostList.h"

#include "headervw.h"
#include "controls.h"
#include "filtersd.h"
#include "filtersv.h"

#include "SearchDoc.h"
#include "SearchView.h"

#include "address.h"

#include "eumapi.h"
#include "ddeclien.h"
#include "ddeserve.h"

#include "ems-wglu.h"
#include "trnslate.h"

#include "RInstDlg.h"
#include "persona.h"

#include "mtransop.h"

#include "AboutDlg.h"
#include "ClipboardMgr.h"

#include "ReadMessageDoc.h"

// OG
#include "PaigeEdtView.h"
#include "PgCompMsgView.h"
#include "PgFixedCompMsgView.h"
#include "PgDocumentFrame.h"
#include "eudora.h"

// IMAP
#include "ImapMailMgr.h"
// END IMAP

// smohanty: 03/09/98
#include "MemMgrSerializedRegionIface.h"

#include "QCSharewareManager.h"
#include "QCLabelDirector.h"
#include "QCCommandStack.h"
#include "QCAutomationDirector.h"
#include "QCMailboxDirector.h"
#include "QCPluginDirector.h"
#include "QCRecipientDirector.h"
#include "QCSignatureDirector.h"
#include "QCStationeryDirector.h"
#include "QCPersonalityDirector.h"
#include "Automation.h"
#include "TraceFile.h"
#include "WizardPropSheet.h"
#include "QCGraphics.h"
#include "QCImageList.h"
#include "TocFrame.h"
#include "AutoCompleteSearcher.h"
#include "ExceptionHandler.h"

#include "QCTaskManager.h"
#include "SpoolMgr.h"
#include "QCRas.h"

#ifdef _DEBUG
#include "DMMDumpLogDlgIface.h"
#endif // _DEBUG

#ifdef _DEBUG
#undef THIS_FILE
#ifndef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#endif
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// the command directors
QCCommandStack			g_theCommandStack;
QCAutomationDirector	g_theAutomationDirector;
QCMailboxDirector		g_theMailboxDirector;
QCPluginDirector		g_thePluginDirector;
QCRecipientDirector		g_theRecipientDirector;
QCSignatureDirector		g_theSignatureDirector;
QCStationeryDirector	g_theStationeryDirector;
QCPersonalityDirector	g_thePersonalityDirector;

#ifdef EXPIRING
	// The Evaluation Time Stamp object
	#include "timestmp.h"
	CTimeStamp	g_TimeStamp;
#endif

CLIPFORMAT cfEmbeddedObject;
CLIPFORMAT cfRTF;
CLIPFORMAT cfRTO;
CLIPFORMAT cfRTWO; // without objects


// The Eudora Personality object
CPersonality	g_Personalities;


// overwrite mode for editing is global state; we make this a simple global,
// so we're not calling AfxGetApp for every single keystroke the user types.
bool g_bOverwriteMode = false;


//DEBUG: to help with LMOS testing.
//Set this to true to test non-uidl servers; Eudora would not use UIDL cmd if this flag is set
bool g_bDisableUIDL = false;

// quicktime shutdown (so we don't have to include the header)
extern "C" void PgMultimediaShutdown();


//
// The Eudora DDE Server object.
//
static CDDEServer theDDEServer;


/////////////////////////////////////////////////////////////////////////////
// CEudoraApp commands


// App command to run the dialog
void CEudoraApp::OnAppAbout()
{
	CAboutDlg aboutDlg(FALSE);
	
	aboutDlg.DoModal();
}

void CEudoraApp::OnTipOfTheDay()
{
	if (gbAutomationExternalStart) return;

	short nShowTipOfTheDay = ::GetIniShort(IDS_INI_SHOW_TIP_OF_THE_DAY);
	short nCurrentTip = ::GetIniShort(IDS_INI_CURRENT_TIP_OF_THE_DAY);

	CString strTipFilename(::ExecutableDir);
	ASSERT(strTipFilename.GetLength() > 0);
	ASSERT(strTipFilename[strTipFilename.GetLength() - 1] == '\\');
	strTipFilename += "eudora.tip";

	QCTipOfDayDlg dlg(strTipFilename, ++nCurrentTip, nShowTipOfTheDay);
	dlg.DoModal();
	
	::SetIniShort(IDS_INI_SHOW_TIP_OF_THE_DAY, short(dlg.GetShowAtStartup()));
	::SetIniShort(IDS_INI_CURRENT_TIP_OF_THE_DAY, short(dlg.GetCurrentTip()));
}


void CEudoraApp::OnUpdateTipOfTheDay(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


/////////////////////////////////////////////////////////////////////////////
// CEudoraApp

BEGIN_MESSAGE_MAP(CEudoraApp, CWinApp)
	//{{AFX_MSG_MAP(CEudoraApp)
	ON_COMMAND(ID_FILE_CHECKMAIL, OnCheckMail)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_TIP_OF_THE_DAY, OnTipOfTheDay)
	ON_UPDATE_COMMAND_UI(ID_TIP_OF_THE_DAY, OnUpdateTipOfTheDay)
	ON_UPDATE_COMMAND_UI(ID_FILE_CHECKMAIL, OnUpdateCheckMail)
	//}}AFX_MSG_MAP

	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)

	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)

	//ON_THREAD_MESSAGE(ID_DO_POST_PROCESSING, OnDoPostProcessing)
	
// Debug Memory manager Command handlers.
// smohanty: 05/29/97.
#ifdef __DEBUG__
        ON_COMMAND(ID_DMM_DUMPLOG, DMMDumpLog)
        ON_COMMAND(ID_DMM_HIGHWATERMARK, DMMHighWatermark)
        ON_COMMAND(ID_DMM_CURRENT_ALLOC, DMMCurrentAlloc)
        ON_COMMAND(ID_DMM_HEAP_TOGGLE,  DMMHeapToggle)
        ON_UPDATE_COMMAND_UI(ID_DMM_DUMPLOG, UpdateDMMDumpLog)
        ON_UPDATE_COMMAND_UI(ID_DMM_HIGHWATERMARK, UpdateDMMHighWatermark)
        ON_UPDATE_COMMAND_UI(ID_DMM_CURRENT_ALLOC, UpdateDMMCurrentAlloc)
        ON_UPDATE_COMMAND_UI(ID_DMM_HEAP_TOGGLE, UpdateDMMHeapToggle)
#endif // __DEBUG__
END_MESSAGE_MAP()

// Eudora Debug Manager Debugger Output support.
// smohanty: 05/27/97
#ifdef __DEBUG__

unsigned int g_nMinDumpSize;

extern "C"
flag DumpBlocksLogCallback(void *pv, c_char * const buf, pDBF_OutType, void *mi)
{
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> aco;
    DMMDumpHandler *pdmmDumpHandler = static_cast<DMMDumpHandler *>(pv);

	MemMgrAllocInfo *mInfo = static_cast<MemMgrAllocInfo *>(mi);
    if(mInfo)
	{
		if(mInfo->SizeBytes >= g_nMinDumpSize)
			pdmmDumpHandler->ProcessOutput(buf);
	}
	else
		pdmmDumpHandler->ProcessOutput(buf);

    return(TRUE);
}
#endif // __DEBUG__

extern BOOL  g_bPopupOn;

/////////////////////////////////////////////////////////////////////////////
// CEudoraApp construction

CEudoraApp::CEudoraApp()
{
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> aco;
#ifdef __DEBUG__
    OutputDebugString("Dumping memory log in CEudoraApp ctor\n");
//    DumpBlocksLog(static_cast<void *>(&bo), DumpBlocksLogCallback, 0);
    DumpBlocksLogByLabelAndFileMT(static_cast<void *>(&dmmDumpHandler),
                                DumpBlocksLogCallback, 0, 0, 0);
#endif
	m_pResList = NULL;
	m_hFindHandle = INVALID_HANDLE_VALUE;
	m_fdOwner = -1;
	m_hOwner = NULL;
	m_hOurWnd = NULL;
	m_TransManager = NULL;
	m_LastOverwritePressed = 0;

	//
	// We use this as a clue to see whether we've run our
	// CEudoraApp::InitInstance().  It's possible that the outer
	// CEudoraApp wrapper shuts things down before we ever get to
	// run our CEudoraApp::InitInstance().
	//
	::memset(&m_OSVersion, 0, sizeof(m_OSVersion));
}

CEudoraApp::~CEudoraApp()
{
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> aco;
    ::ShutdownDebugMemoryManagerMT();
}

// Debug Memory Manager command and update handlers.
#ifdef __DEBUG__
void
CEudoraApp::DMMDumpLog()
{
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> aco;
    //DMMDumpLogDialog dumpDlg(NULL, DumpBlocksLogCallback);
	DMMDumpLogDialog dumpDlg;
    if( dumpDlg.DoModal() == IDOK)
	{
		AutoSerializedRegionMT<MemMgrSerializedRegionMT> aco;
		char label[512]={0};
		char file[512] = {0};
		strncpy(label, dumpDlg.GetLabel(), 511);
		strncpy(file, dumpDlg.GetFile(), 511);
		g_nMinDumpSize = dumpDlg.GetMinSize();
		
		DMM_DH bo;
		DumpBlocksLogByLabelAndFileMT(static_cast<void *>(&bo), DumpBlocksLogCallback, 0,
				((strlen(label) > 0) ? label : 0),
				((strlen(file) > 0)  ? file : 0));
		_heapmin();
		g_nMinDumpSize = 0; //reset for the next run

	}

}

void
CEudoraApp::DMMHighWatermark()
{
	// Check if the app hasn't allocated any tocs yet
	if (!TocTemplate)
		return;

	POSITION pos = TocTemplate->GetFirstDocPosition();
	while (pos)
	{
		char buf[512];
		CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (doc)
		{
			sprintf(buf, "%4d Sums\t%s\n", doc->NumSums(), doc->Name());
			OutputDebugString(buf);
		}
	}
}

void
CEudoraApp::DMMCurrentAlloc()
{
}

void
CEudoraApp::DMMHeapToggle()
{
}

void
CEudoraApp::UpdateDMMDumpLog(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
}

void
CEudoraApp::UpdateDMMHighWatermark(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
}

void
CEudoraApp::UpdateDMMCurrentAlloc(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
}

void
CEudoraApp::UpdateDMMHeapToggle(CCmdUI *pCmdUI)
{
    pCmdUI->SetText((fIsHeapSolventMT() == TRUE)
		    ? "&FakeOutOfMemory" : "&UnfakeOutOfMemory");
}

#endif // __DEBUG__

void CEudoraApp::OnUpdateCheckMail(CCmdUI* pCmdUI)
{
	CString CheckMailText;
	
	GetMailCheckTime(CheckMailText);

	// Get rid of Virtual Key text
	int nLen = CheckMailText.GetLength();
	int nPos;
	if ( g_bPopupOn )	
		if ( ( nPos = CheckMailText.Find('\t')) != -1)
			nLen= nPos;

	pCmdUI->SetText(CheckMailText.Left(nLen));
}

void CEudoraApp::UpdateMailCheckLabel()
{
	CString CheckMailText;
	CMenu* FileMenu;
	CMainFrame*	pMainFrame;
	UINT		uOffset;
	BOOL		bMaximized;

	bMaximized = FALSE;
	pMainFrame = ( CMainFrame* ) AfxGetMainWnd();
	pMainFrame->MDIGetActive( &bMaximized );
	uOffset = ( bMaximized ? 1 : 0 );

	FileMenu = pMainFrame->GetMenu()->GetSubMenu( MAIN_MENU_FILE + uOffset );
	
	this->GetMailCheckTime(CheckMailText);

	FileMenu->ModifyMenu(ID_FILE_CHECKMAIL, MF_BYCOMMAND | MF_STRING, ID_FILE_CHECKMAIL, CheckMailText);

}

void CEudoraApp::GetMailCheckTime(CString &CheckMailText)
{
	
	if (m_NextCheckMail)
	{
		char Time[64];
		::TimeDateStringMT(Time, m_NextCheckMail, FALSE);
		AfxFormatString1(CheckMailText, IDS_FILE_CHECK_MAIL_TIME, Time);
	}
	else
		CheckMailText.LoadString(IDS_FILE_CHECK_MAIL);
}

void CEudoraApp::OnCheckMail()
{
	if (ShiftDown())
	{
		CMailTransferOptionsDialog dlg;
		dlg.DoModal();
	}
	else
	{
		GetMail(kManualMailCheckBits);

		UpdateMailCheckLabel();
	}
}

void CEudoraApp::OnAppExit()
{
	CPopHost *theHost	= NULL;
	
	// If Shift was held down while doing a File->Exit, then just save the
	// positions of the currently open windows, but don't quit
	if (ShiftDown())
		((CMainFrame*)AfxGetMainWnd())->SaveOpenWindows(FALSE);
	else
	{
#ifdef JUNK
		if(gPopHostList != NULL)
			{
			// For each host in the list we need to delete him/her and his/her list...	    
		    while(!gPopHostList->IsEmpty())
		    	{
		     	// Lets get the host...
		     	theHost	= (CPopHost *)gPopHostList->RemoveTail();
		    	delete theHost;
		    	}
		    gPopHostList->RemoveAll();		
			}
#endif	// JUNK

		CWinApp::OnAppExit();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CEudoraApp initialization

CMultiDocTemplate* TextFileTemplate = NULL;
CMultiDocTemplate* TocTemplate = NULL;
CMultiDocTemplate* ReadMessageTemplate = NULL;
CMultiDocTemplate* CompMessageTemplate = NULL;
CMultiDocTemplate* FindTemplate = NULL;
CMultiDocTemplate* SearchTemplate = NULL;
CMultiDocTemplate* FiltersTemplate = NULL;
CMultiDocTemplate* NicknamesTemplate = NULL;
CMultiDocTemplate* SignatureTemplate = NULL;

//	=========================================================================================
BOOL 
CEudoraApp::InitApplication()
{
	return CWinApp::InitApplication();
}

//	=========================================================================================

BOOL CEudoraApp::InitInstance()
{
	// Guard against re-entrancy of multiple copies on the same set of mailboxes
	// by using a named mutex
	CMutex StartupMutex(FALSE, "EudoraStartupMutex");
	CSingleLock StartupMutexLock(&StartupMutex, TRUE);
	ASSERT(StartupMutexLock.IsLocked());

	//
	// Make sure we're running on a compatible OS.  
	// If not, we're outta here.
	//
	m_OSVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&m_OSVersion);

	if (m_OSVersion.dwMajorVersion < 4)
	{
		::AfxMessageBox(IDS_NEED_VERSION4, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	// Initialize the OLE DLLs
	if (!AfxOleInit())
	{
		::AfxMessageBox(IDP_OLE_INIT_FAILED, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	//
	// 3D effects for controls.  This has to go before the possible
	// "return" statements out of this method because it has to be
	// balanced with a corresponding Unregister() call in ExitInstance().
	//
	HINSTANCE hInst = AfxGetInstanceHandle();
	ResourceStatus resourceStatus = RESOURCE_OKAY;


// This preprocessor symbol is not used any more. The
// resource DLL is *always* used. Subhashis Mohanty. 04/03/97.

// #ifdef USE_RESOURCE_DLL
	// if release version add the default resource dll
	// ( contains resources that can be overidden via plug-ins )

	resourceStatus = LoadResourceDLL();
	switch (resourceStatus)
	{
	case RESOURCE_OKAY:
		break;
	case RESOURCE_VERSION_MISMATCH:
		if (IDNO == ::AfxMessageBox(IDS_RESDLL_MISMATCH, MB_YESNO | MB_ICONQUESTION))
			return FALSE;
		break;
	case RESOURCE_NOT_LOADED:
		::AfxMessageBox( IDS_NO_RESDLL_32 );
		break;
	case RESOURCE_UNKNOWN:
	default:
		ASSERT(0);
		resourceStatus = RESOURCE_UNKNOWN;
		break;
	}

// #endif // USE_RESOURCE_DLL

	g_theCommandStack.NewCommandMap();

	g_Personalities.Init();

	// Check if application was run with -Embedding set by system
	if (RunEmbedded())
	{
		// Remote application has started Eudora as Automation Server

		gbAutomationRunning = true;
		gbAutomationExternalStart = true;
		
	}

	// Check if application was run with -Automation set by system
	if (RunAutomated())
	{
		// Remote application is querying Eudora interfaces
		return TRUE;
	}

	// Enable ActiveX controls in dialogs
	// do this prior to creating any windows
	AfxEnableControlContainer();

	// Set Eudora palette to the colors specified
	// in the resources plus colors for the current color sceme
	SetSafetyPalette(IDB_SAFETY_PALETTE);

	// Load shared Eudora images lists
	LoadImageLists();

	// Initialize ATL OLE Module
	_Module.InitObjectMap(m_hInstance);

	COMMANDLINE_TYPE CmdType = GetCommandType(m_lpCmdLine);

	switch( CmdType )
	{
		case CL_ATTACH:
		case CL_STATIONERY:
		case CL_MAILTO:
		case CL_MESSAGE:
			// can an existing instance handle it?
			if ( ProcessAttachmentWEI( CmdType ) )
				return FALSE;

			// set up the command line based on the registry
			if (!StartEudoraWithReg())
				return FALSE;

			// there could be a remote instance running...
			if (IsEudoraRunning())
				return FALSE;
			break;

		default:
			
			if( gbAutomationRunning )
			{
				if( !StartEudoraWithReg() )
				{
					return FALSE;
				}
				
				// it's not clear to me that this is really the right thing to do.
				// But, it's always worked this way so ???
				if (IsEudoraRunning())
					return FALSE;
			}
			else
			{
				// use the command line to establish the working environment
				if (!GetDirs(m_lpCmdLine))
					return FALSE;

				if (IsEudoraRunning())
					return FALSE;

				RegisterCommandLine(m_lpCmdLine);
			}
			break;
	}

	// Change where the exception log will go to be the file Exception.log in the mail directory
	char buf[MAX_PATH + 16];
	sprintf(buf, "%sException.log", (LPCTSTR)EudoraDir);
	g_MSJExceptionHandler.SetLogFileName(buf);

	// Start up debugger if it's present
	sprintf(buf, "%sDebugger.exe", (LPCTSTR)ExecutableDir);
	if (FileExistsMT(buf))
	{
		DWORD dwProcessID = GetCurrentProcessId();
		TCHAR ProcIdBuf[16];
		_ultot(dwProcessID, ProcIdBuf, 10);
		ShellExecute(NULL, NULL, buf, ProcIdBuf, NULL, SW_HIDE);
	}

	// Check ini file to see if automation is enabled
	if (GetIniShort(IDS_INI_AUTOMATION_ENABLED))
	{
		AutomationEnable();
//	 	gbAutomationEnabled = true;
	}

	// Check to see if command line is "/EnableAutomation"
	CString str = m_lpCmdLine;
	if (str.CompareNoCase("/EnableAutomation") == 0)
	{
		AutomationEnable();
		return FALSE;
	}

	// Check to see if command line is "/DisableAutomation"
	if (str.CompareNoCase("/DisableAutomation") == 0)
	{
		AutomationDisable();
		return FALSE;
	}

	// make sure the Embedded dir exists
	MakeEmbeddedDir();

	// add Eudora Plug Ins
	LoadEPIs();

	RegisterIcon();
	RegisterStationery();
	RegisterFormats();
	m_pszProfileName = strdup( INIPath );

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	//
	// Splash screen if wanted.
	//
	// Now we give the splash screen "child" status, so we can't create it until
	// after the MainFrame is valid.
	//
	CWnd *pAboutDlg = NULL;
	if (GetIniShort(IDS_INI_NO_SPLASH_SCREEN) == FALSE)
		pAboutDlg = new CAboutDlg(TRUE/*, pMainFrame*/);

	
	// Load up the user-defined fonts
	SetupFonts();

	// Get the debug process going
	//if (!InitDebug())
	if (! QCLogFileMT::InitDebug( GetIniLong(IDS_INI_DEBUG_LOG_LEVEL),
								  GetIniLong(IDS_INI_DEBUG_LOG_FILE_SIZE),
								  EudoraDir,
								  GetIniString(IDS_INI_DEBUG_LOG_FILE_NAME)) )
	{
		//
		// Still have a big problem.  Could not open the debug logfile for writing.
		// The sane thing is to bail so that subsequent logging calls don't
		// misbehave.
		//
		return FALSE;
	}

		// Load up Translation DLL's
//	InitTranslators();	// This used to be here, but we moved it so that plugins would get the right value for Eudora's HWnd

	// Lets init our variables here...
	this->m_LastIdle				= 0;			// Our last state execution time holder
	this->m_NextCheckMail 			= 0;
	this->m_NextSendQueued 			= 0;
	this->m_ForceCheckMail 			= FALSE;

	
	// Possibly disable Save Password feature
	if (GetIniShort(IDS_DISABLE_SAVE_PASSWORD))
		SetIniShort(IDS_INI_SAVE_PASSWORD, FALSE);
		
	// limit Check Mail frequency
	short CMI = GetIniShort(IDS_INI_CHECK_FOR_MAIL);
	if (CMI)
	{
		SetIniShort(IDS_INI_CHECK_FOR_MAIL,
			std::max(CMI, GetIniShort(IDS_MIN_MAIL_CHECK_INTERVAL)));
	}
   	
   	// Make sure only one encoding type is selected
   	if (GetIniShort(IDS_INI_SEND_UUENCODE))
   	{
   		SetIniShort(IDS_INI_SEND_MIME, FALSE);
   		SetIniShort(IDS_INI_SEND_BINHEX, FALSE);
   	}
   	else if (GetIniShort(IDS_INI_SEND_BINHEX))
   	{
   		SetIniShort(IDS_INI_SEND_MIME, FALSE);
   		SetIniShort(IDS_INI_SEND_UUENCODE, FALSE);
   	}

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	// Text File class
	TextFileTemplate = new CMultiDocTemplate(
		IDR_NEWFILE,
		RUNTIME_CLASS(CTextFileDoc),
		RUNTIME_CLASS(PgDocumentFrame),
		RUNTIME_CLASS(CPaigeEdtView));
	AddDocTemplate(TextFileTemplate);

	// Toc class
	TocTemplate = new CMultiDocTemplate(
		IDR_TOC,
		RUNTIME_CLASS(CTocDoc),
		RUNTIME_CLASS(CTocFrame),
		RUNTIME_CLASS(CTocView));
	AddDocTemplate(TocTemplate);

	// Read message class
	ReadMessageTemplate = new CMultiDocTemplate(
		IDR_READMESS,
		RUNTIME_CLASS(CReadMessageDoc),
		RUNTIME_CLASS(CReadMessageFrame),
		RUNTIME_CLASS(CEditView));
	AddDocTemplate(ReadMessageTemplate);

	// Composition message class
	CompMessageTemplate = new CMultiDocTemplate(
		IDR_COMPMESS,
		RUNTIME_CLASS(CCompMessageDoc),
		RUNTIME_CLASS(CCompMessageFrame),
		RUNTIME_CLASS(CPaigeEdtView));
	AddDocTemplate(CompMessageTemplate);

	// Search class
	SearchTemplate = new CMultiDocTemplate(
		IDR_SEARCH_WND,
		RUNTIME_CLASS(CSearchDoc),
		RUNTIME_CLASS(CMDIChild),
		RUNTIME_CLASS(CSearchView));
	AddDocTemplate(SearchTemplate);

	// Filters class
	FiltersTemplate = new CMultiDocTemplate(
		IDR_FILTERS,
		RUNTIME_CLASS(CFiltersDoc),
		RUNTIME_CLASS(CFrameWnd),		// dummy value
		RUNTIME_CLASS(CView));			// dummy value
	AddDocTemplate(FiltersTemplate);

	// Nicknames class
	NicknamesTemplate = new CMultiDocTemplate(
		IDR_NICKNAMES,
		RUNTIME_CLASS(CNicknamesDoc),
		RUNTIME_CLASS(CFrameWnd),		// dummy value
		RUNTIME_CLASS(CView));			// dummy value
	AddDocTemplate(NicknamesTemplate);

	// Signature class
	SignatureTemplate = new CMultiDocTemplate(
		IDR_TEXTFILE,
		RUNTIME_CLASS(CSignatureDoc),
		RUNTIME_CLASS(PgDocumentFrame),
		RUNTIME_CLASS(CSignatureView)); 
	AddDocTemplate(SignatureTemplate);

	// build all the command objects
	if (g_theStationeryDirector.Build() == FALSE)
		return FALSE;
	
	if (g_theMailboxDirector.Build() == FALSE)
		return FALSE;
	
	if (g_theRecipientDirector.Build() == FALSE)
		return FALSE;
	
	if (g_theSignatureDirector.Build() == FALSE)
		return FALSE;
	
	if (g_thePersonalityDirector.Build() == FALSE)
		return FALSE;
	
	// Initialize the label director -- must be destroyed when done
	if (!QCLabelDirector::Initialize())
		return (FALSE);

	// Initialize the Shareware Manager -- must be destroyed when done
	if (!QCSharewareManager::Initialize())
		return (FALSE);

	//
	// Create and initialize the One True CFiltersDoc Object(tm).
	//
	VERIFY(CreateFiltersDoc());

	//
	// Create and initialize the One True CNicknamesDoc Object(tm).
	//
	ASSERT(NULL == g_Nicknames);
	g_Nicknames = (CNicknamesDoc *) NewChildDocument(NicknamesTemplate);
	if (! g_Nicknames->ReadTocs())
	{
		g_Nicknames->OnCloseDocument();
		return FALSE;
	}

	//
	// Create and initialize the One True CNicknamesDoc Object(tm).
	//
	ASSERT(NULL==g_AutoCompleter);
	g_AutoCompleter = new AutoCompList(CRString(IDS_RECIP_HIST_LIST));
	if (g_AutoCompleter)
		g_AutoCompleter->ReadAutoCompleteList();

	//
	// Load up some bitmaps into global variables for faster performance
	//
	QCLoadBitmap( IDB_ATTACHMENT, g_AttachmentBitmap );

	::QCLoadBitmap(IDB_PRIOR_MENU_HIGHEST, g_PriorityHighestMenuBitmap);
	::QCLoadBitmap(IDB_PRIOR_MENU_HIGH, g_PriorityHighMenuBitmap);
	::QCLoadBitmap(IDB_PRIOR_MENU_NORMAL, g_PriorityNormalMenuBitmap);
	::QCLoadBitmap(IDB_PRIOR_MENU_LOW, g_PriorityLowMenuBitmap);
	::QCLoadBitmap(IDB_PRIOR_MENU_LOWEST, g_PriorityLowestMenuBitmap);

	::QCLoadBitmap(IDB_STATUS_UNREAD, g_StatusUnreadBitmap);
	::QCLoadBitmap(IDB_STATUS_READ, g_StatusReadBitmap);
	::QCLoadBitmap(IDB_STATUS_REPLIED, g_StatusRepliedBitmap);
	::QCLoadBitmap(IDB_STATUS_FORWARDED, g_StatusForwardedBitmap);
	::QCLoadBitmap(IDB_STATUS_REDIRECT, g_StatusRedirectBitmap);
	::QCLoadBitmap(IDB_STATUS_UNSENDABLE, g_StatusUnsendableBitmap);
	::QCLoadBitmap(IDB_STATUS_SENDABLE, g_StatusSendableBitmap);
	::QCLoadBitmap(IDB_STATUS_QUEUED, g_StatusQueuedBitmap);
	::QCLoadBitmap(IDB_STATUS_SENT, g_StatusSentBitmap);
	::QCLoadBitmap(IDB_STATUS_UNSENT, g_StatusUnsentBitmap);
	::QCLoadBitmap(IDB_STATUS_TIME_QUEUED, g_StatusTimeQueuedBitmap);
	::QCLoadBitmap(IDB_STATUS_RECOVERED, g_StatusRecoveredBitmap);

	::QCLoadBitmap(IDB_SERVER_LEAVE, g_ServerLeaveBitmap);
	::QCLoadBitmap(IDB_SERVER_FETCH, g_ServerFetchBitmap);
	::QCLoadBitmap(IDB_SERVER_DELETE, g_ServerDeleteBitmap);
	::QCLoadBitmap(IDB_SERVER_FETCH_DELETE, g_ServerFetchDeleteBitmap);

	if (GetIniShort(IDS_INI_CENTER_UNREAD_STATUS))
	{
		//
		// Since these bitmaps are used on menu items, center the 
		// bitmaps vertically in the space available.
		//
		int nCheckMenuHeight = ::GetSystemMetrics(SM_CYMENUCHECK);

		::FitTransparentBitmapToHeight(g_PriorityHighestMenuBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_PriorityHighMenuBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_PriorityNormalMenuBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_PriorityLowMenuBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_PriorityLowestMenuBitmap, nCheckMenuHeight);

		::FitTransparentBitmapToHeight(g_StatusUnreadBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusReadBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusRepliedBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusForwardedBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusRedirectBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusUnsendableBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusSendableBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusQueuedBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusSentBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusUnsentBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusTimeQueuedBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusRecoveredBitmap, nCheckMenuHeight);

		::FitTransparentBitmapToHeight(g_ServerLeaveBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_ServerFetchBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_ServerDeleteBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_ServerFetchDeleteBitmap, nCheckMenuHeight);
	}

#ifdef IMAP4
	// Initialize the IMAP service provider.
	//
	CImapMailMgr::InitializeImapProvider();

#endif

	
	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
		m_pMainWnd = pMainFrame;

		// Load up Translation DLL's We have to do this here because InitTranslators requires the Mainframe exists.
		// We need it to be after the new mainframe is created so we can get a pointer to the window, but BEFORE
		// The frame is loaded because that's where the buttons and menu items loaded by the plugin director are
		// umm... loaded.
	InitTranslators();
	if ( g_thePluginDirector.Build() == FALSE )
		return FALSE;
		/////////////////////////////

#ifdef THREADED
	QCTaskManager::Create();  //Create the task manager for handling the background tasks
#endif

	if (!pMainFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW)) // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Oh sure, I bet you're going to try and paste something here too. Everyone wants to be right after the creation
	// of the mainframe. GET IN LINE BUDDY!

#ifdef EXPIRING
	// initialize the evaluation timestamp
	char szPath[ 256 ];
	if ( GetModuleFileName(AfxGetInstanceHandle(), szPath, sizeof(szPath) ))
		if ( g_TimeStamp.Read( szPath ) )
			g_TimeStamp.ReadRegistry();

	DWORD dwDaysLeft = g_TimeStamp.GetDaysRemaining();
	if ( dwDaysLeft <= 0 )
	{
		AfxGetMainWnd()->PostMessage(WM_USER_EVAL_EXPIRED);
	}
	else if ( dwDaysLeft < 7 )
	{
		WarnDialog(0, IDS_TIME_BOMB_DAYS_LEFT_BETA, (int)dwDaysLeft);
	}
#endif

	//we need LMOS hosts ready so we can recover from left over .RCV files
	CHostList::CreateNewHostList();

	// Delete any left over spool files...
	QCSpoolMgrMT::Init(g_Personalities);

	//Load and Initialize winsock library
	//if (!GetIniShort(IDS_INI_NET_IMMEDIATE_CLOSE))
	QCWinSockLibMT::LoadWSLibrary();
	QCRasLibrary::Init(GetIniShort(IDS_INI_RAS_USE_EXISTING_CONN)?true:false);

	//
	// Immediately after the main window is initialized, we need to
	// inform any visible wazoo containers to choose an active
	// wazoo and perform the Just In Time display initializations for
	// those wazoos.
	//
	pMainFrame->SetActiveWazooWindows();

	// Reset DS resize logic.
	pMainFrame->ResetDSResize();

	// Because this init takes so long, we should force window updates throughout
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

	// We need to wait until the OWNER.LOK file is created until we can let go of the
	// startup mutex, which allows other instances to see if they can start up.
	RegisterOwnership();
	VERIFY(StartupMutexLock.Unlock());

	// Because this init takes so long, we should force window updates throughout
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

	//
	// Customized toolbars can contain user-defined command id's which
	// correspond to user-defined menu items.  Therefore, we must
	// postpone the load of the toolbar buttons until *after* the
	// user-defined menus are built.
	//
#ifdef CTB
	if (! pMainFrame->LoadCustomToolbar())
		return FALSE;
#endif

	// Make sure main window is behind the splash screen
	if (pAboutDlg)
		pMainFrame->SetWindowPos(pAboutDlg, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE);

	// If the passed in show state is normal, then get the saved state from the last session
	if (m_nCmdShow == SW_NORMAL)
	{
		int WindowState = GetIniShort(IDS_INI_MAIN_WINDOW_STATE);
		// Don't make it active, though, or it will hide the splash screen
		if (pAboutDlg && WindowState == SW_NORMAL)
			WindowState = SW_SHOWNOACTIVATE;

		pMainFrame->ShowWindow(WindowState);
	}
	else
		pMainFrame->ShowWindow(m_nCmdShow);

	pMainFrame->m_bStartedIconic = pMainFrame->IsIconic();
	
	// Because this init takes so long, we should force window updates throughout
	pMainFrame->UpdateWindow();
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

	// Check if there are any queued messages
	SetQueueStatus();

	// Because this init takes so long, we should force window updates throughout
	pMainFrame->UpdateWindow();
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

#ifdef DELPHI
	// verify required fields are present
	if (!CheckDelphiConfig())
		return (FALSE);
#endif

	//
	// Initialize Eudora's DDE Server.  This should be done before we
	// do the MAPI swapperoo.  Otherwise, the initialization could
	// stall while displaying the "could not swap" error dialog before
	// the DDE stuff has a chance to initialize.  If this stall
	// occurs, we have a deadlock situation where the MAPI DLL which
	// launched this copy of Eudora were expecting Eudora's DDE server
	// to be up and running.  How's that for twisted?
	//
	theDDEServer.Initialize();

	// Check out which MAPI DLLs should be where
	if (GetIniShort(IDS_INI_MAPI_USE_NEVER))
		MAPIUninstall();
	else
		MAPIInstall();

	if (GetIniShort(IDS_INI_NETSCAPE_MAILTO))
	{
		// Register Eudora as Netscape's mailto URL handler
		CDDEClient dde_client;
		dde_client.RegisterNetscapeProtocol("mailto");
	}

	// Because this init takes so long, we should force window updates throughout
	pMainFrame->UpdateWindow();
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

	// Moved here from just before "theDDEServer.Initialize()" above, to avoid
	// IMAP conflicts with Netscape!! (JOK - 7/17/98).
	//
	// Load up windows from previous session
	pMainFrame->LoadOpenWindows();

	// Because this init takes so long, we should force window updates throughout
	pMainFrame->UpdateWindow();

	// Destroy the splash screen -- if needed
	if (pAboutDlg)
		pAboutDlg->DestroyWindow();


	// Fix Bug #3481: Dialog boxes don't have correct background color
	//
	// Seems the SEC crap combined with our wierdness makes our dlgs
	// always gray. So we need to force the color to the system colors.
	//
	// Of course this doesn't help if you change colors during a
	// Eudora session.

	SetDialogBkColor(GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_BTNTEXT));

	// ==================== Now do all the interactive checking ====================

	RegisterMailto();

	// Put up New Account Wizard if no return address specified
	const char* ra = GetReturnAddress();
	if (!ra || !*ra)
	{
		SetIniShort(IDS_INI_LAST_SETTINGS_CATEGORY, 0);
		pMainFrame->PostMessage(WM_COMMAND, ID_SPECIAL_NEWACCOUNT);
	}
	else
	{
		// Show the tip-o-day only if the Wizard is not shown.
		if (::GetIniShort(IDS_INI_SHOW_TIP_OF_THE_DAY))
			::PostMessage(pMainFrame->GetSafeHwnd(), WM_COMMAND, ID_TIP_OF_THE_DAY, 0);
	}
	
	// now that Eudora's running attach files
	if ( CmdType == CL_ATTACH || CmdType == CL_MAILTO )
		pMainFrame->CreateMessage(m_lpCmdLine);
	else if ( CmdType == CL_STATIONERY ) 
	{
		CCompMessageDoc* Comp = NewCompDocumentWith(m_lpCmdLine);

		if (Comp)
			Comp->m_Sum->Display();
	}
	else if ( CmdType == CL_MESSAGE ) 
	{
		NewMessageFromFile( m_lpCmdLine );
	}

	DoCheckProFilters();

	g_bDisableUIDL = GetIniShort(IDS_INI_DEBUG_DISABLE_UIDL)?true:false;


	// Begin IMAP4 stuff:

	// Tell IMAP that the app is now initialized:
	//
	CImapMailMgr::NotifyAppInitialized();

	// And resync any IMAP mailboxes that may have been opened:
	pMainFrame->PostMessage(WM_COMMAND, ID_MBOX_IMAP_RESYNC_OPENED);

// End IMAP4 stuff:

	return (TRUE);
}

void CEudoraApp::ResetLastIdle()
{
	m_LastIdle = time(NULL);
}

unsigned long CEudoraApp::GetLastIdle()
{
	return m_LastIdle;
}

/*
void CEudoraApp::OnDoPostProcessing(WPARAM wParam, LPARAM)
{
	QCGetTaskManager()->DoPostProcessCmd((CTaskInfoMT *)wParam);
}
*/

BOOL CEudoraApp::PreTranslateMessage(MSG* pMsg)
{
    // If the user is doing something (which we define as pressing a key or
    // pressing a mouse button) then record the current time so that we can
    // check for idle time down in CEudoraApp::OnIdle()
    UINT mes = pMsg->message;

    if ( mes == WM_KEYDOWN )	// Switched form Keyup because people were letting go of the shift key too soon.
    {
//Exception generator
//int* pi = NULL;
//*pi = 0;
		bool bOptionKeysPressed = ((::GetAsyncKeyState( VK_SHIFT ) < 0) ||
                                   (::GetAsyncKeyState( VK_CONTROL ) < 0));

		unsigned long thetime = time(NULL);
        if (pMsg->wParam == VK_INSERT)
		{	
			if (!bOptionKeysPressed && GetIniShort(IDS_INI_ALLOW_OVERWRITE_MODE) && (thetime - m_LastOverwritePressed > 1)) // One second
			{
				g_bOverwriteMode = !g_bOverwriteMode;
			}
			else if(bOptionKeysPressed)
				m_LastOverwritePressed = thetime;	
		}
	}

    if( ((mes >= WM_KEYFIRST && mes <= WM_KEYLAST) && (mes != WM_KEYUP) && (mes != WM_SYSKEYUP))
        || (mes >= WM_MOUSEMOVE + 1 && mes <= WM_MOUSELAST)
        || (mes >= WM_NCLBUTTONDOWN && mes <= WM_NCMBUTTONDBLCLK) )
    {
        m_LastIdle = time(NULL);

        if (IsVersion4() && ((CMainFrame *)AfxGetMainWnd())->GetCurrentIconID() >= IDI_HAVE_MAIL)
            ((CMainFrame *)AfxGetMainWnd())->SetIcon(FALSE,0);
    }
	else if (mes == WM_MOUSEWHEEL )
	{
		POINT point;
		point.x = (short) LOWORD(pMsg->lParam);    // horizontal position of pointer
		point.y = (short) HIWORD(pMsg->lParam);    // vertical position of pointer

		CWnd *TheWindow = CWnd::WindowFromPoint(point);

		if (!(TheWindow->IsKindOf(RUNTIME_CLASS(CView))))	// The window the mouse was over is not a Cview based class. 
															// We're going to try to pass it to the trident preview pane.
															// If it were a CView based class then we just bail, it's not
															// What we want.
		{
			CWnd *ParentWnd = TheWindow->GetParent();
			if ( ParentWnd && ParentWnd->IsKindOf(RUNTIME_CLASS( CTridentPreviewView)) )
			{

				CWnd *ChildWnd = ParentWnd->GetDescendantWindow(0);
				if (ChildWnd)
					ChildWnd->SendMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);
				return TRUE;
			}
	
		}
	}
        
    return (CWinApp::PreTranslateMessage(pMsg));
}

void CEudoraApp::SaveStdProfileSettings()
{
	CWinApp::SaveStdProfileSettings();
}

int CEudoraApp::ExitInstance()
{
	// Clean up the directors/managers
	VERIFY(QCLabelDirector::Destroy());
	VERIFY(QCSharewareManager::Destroy());

	POSITION pos;

	if (ReadMessageTemplate)
	{
		pos = ReadMessageTemplate->GetFirstDocPosition();
		while (pos)
		{
			// If we get here, it means that we didn't clean up correctly
			ASSERT(FALSE);
			CMessageDoc* doc = (CMessageDoc*)ReadMessageTemplate->GetNextDoc(pos);
			if (doc && doc->m_Sum)
			{
				// This prevents a crash on exit because the summary
				// will have been deleted by this point
				doc->m_Sum = NULL;
			}
		}
	}

	if (CompMessageTemplate)
	{
		pos = CompMessageTemplate->GetFirstDocPosition();
		while (pos)
		{
			// If we get here, it means that we didn't clean up correctly
			ASSERT(FALSE);
			CMessageDoc* doc = (CMessageDoc*)CompMessageTemplate->GetNextDoc(pos);
			if (doc && doc->m_Sum)
			{
				// This prevents a crash on exit because the summary
				// will have been deleted by this point
				doc->m_Sum = NULL;
			}
		}
	}

	pos = m_FilterList.GetHeadPosition();
	while( pos )
	{
		delete m_FilterList.GetNext( pos );
	}
	

	if (g_AutoCompleter)
	{
		g_AutoCompleter->SaveToDisk();
		delete g_AutoCompleter;
		g_AutoCompleter = NULL;
	}

	if (g_Nicknames)
	{
		g_Nicknames->m_bAutoDelete = TRUE;
		g_Nicknames->OnCloseDocument();
		ASSERT(NULL == g_Nicknames);
	}
	
	if (m_OSVersion.dwMajorVersion)
	{
		//
		// Only run the following if it looks like we actually ran
		// through the InitInstance() method.
		//
		SaveStdProfileSettings();
	    IniStringCleanUp();
		OwnershipCleanup();
		
		//Flush the log file and cleanup
		QCLogFileMT::CleanupDebug();

		// Shutdown Eudora's DDE Server.
		theDDEServer.Uninitialize();
	}
    
	delete m_TransManager;

	// perform fabulous silly shutdown procedure
	PgSharedAccess::SillyShutdown();

	// nuke quicktime
	PgMultimediaShutdown();

#ifdef IMAP4
	// Terminate services of the IMAP service provider.
	CImapMailMgr::Shutdown ();
#endif


#ifdef THREADED
	// Delete all the spool managers
	QCSpoolMgrMT::Shutdown();
	QCTaskManager::Destroy();  //Create the task manager for handling the background tasks
#endif

	//Shutdown winsock dll
	QCWinSockLibMT::FreeWSLibrary();


	// remove temp icon files create when viewing images
	CleanupIcons();

	// Free global palettes
	FreePalettes();

    // This should really be done in OnAppExit() 'cept it doesn't get called.
    // Since we never have multiple instances this is equivilent...
	if (m_OSVersion.dwMajorVersion)
		FreeResources();

	// Cleanup ATL Module
	_Module.Term();
//	ShutdownDebugMemoryManagerMT();	
	
	g_theCommandStack.Pop();

	return (CWinApp::ExitInstance());
}


// NewChildDocument
// Create a new document based on the given template
//
CDocument* CEudoraApp::NewChildDocument(CDocTemplate* templ)
{
	CDocument* doc = templ->CreateNewDocument();
	
	ASSERT_VALID(doc);
	
	VERIFY(doc->OnNewDocument());
	
	return (doc);
}

// NewChildFrame
// Create a new frame based on the template specified by the given template,
// and associate it to the document pDOC
//
CFrameWnd* CEudoraApp::NewChildFrame(CDocTemplate* templ, CDocument* pDoc)
{
	CFrameWnd* frame = templ->CreateNewFrame(pDoc, NULL);
	
	ASSERT_VALID(frame);
	
	if (frame)
		templ->InitialUpdateFrame(frame, pDoc, TRUE);
		
	return (frame);
}

// NewChildDocFrame
// Create a new document and frame based on the given template
//
CDocument* CEudoraApp::NewChildDocFrame(CDocTemplate* templ, BOOL OnlyOne /*= TRUE*/)
{
	CDocument* doc = NULL;
	
	if (OnlyOne)
	{
		POSITION pos = templ->GetFirstDocPosition();
		if (pos)
		{
			doc = TocTemplate->GetNextDoc(pos);
			pos = doc->GetFirstViewPosition();
			if (pos)
			{
				CView* View = doc->GetNextView(pos);
				QCWorksheet* Frame;
				if (View && (Frame = (QCWorksheet *) View->GetParentFrame()))
				{
					ASSERT_KINDOF(QCWorksheet, Frame);
					if (Frame->IsIconic())
						Frame->MDIRestore();
					else
						Frame->MDIActivate();
				}
				return (doc);
			}
		}
	}

	if (!doc)
		doc = NewChildDocument(templ);
	
	if (doc)
		NewChildFrame(templ, doc);

	return (doc);
}                       

BOOL CEudoraApp::IsIdleMessage( MSG* pMsg )
{
	// Return FALSE if the message just dispatched should _not_
	// cause OnIdle to be run.	Messages which do not usually
	// affect the state of the user interface and happen very
	// often are checked for.

	if (!CWinApp::IsIdleMessage(pMsg))
		return FALSE;

	// WM_TIMER messages that aren't the general one shouldn't cause idle processing
	if (pMsg && pMsg->message == WM_TIMER &&
		pMsg->wParam != ((CMainFrame*)AfxGetMainWnd())->GetGeneralTimerID())
	{
		return FALSE;
	}

	return TRUE;
}

// OnIdle
//
// Do things in the background, like automatic mail checks and mailbox TOC saves.
// This routine gets called as result of a WM_TIMER message in the main window.
//
BOOL CEudoraApp::OnIdle( LONG lCount )
{
//TRACE1("                                  Before CWinApp::OnIdle(): CEudoraApp::OnIdle(), lCount = %d\n", lCount);
	if (CWinApp::OnIdle(lCount))
	{
//TRACE1("                                  After CWinApp::OnIdle()  TRUE: CEudoraApp::OnIdle(), lCount = %d\n", lCount);
		return TRUE;
	}
//TRACE1("                                  After CWinApp::OnIdle() FALSE: CEudoraApp::OnIdle(), lCount = %d\n", lCount);

	unsigned long	currentTime	= time(NULL);
	unsigned long	deltaTime	= currentTime - m_LastIdle;
	long 			CMI			= GetIniLong(IDS_INI_CHECK_FOR_MAIL);
	unsigned long	nAutoSaveEvery		= GetIniLong(IDS_INI_AUTO_SAVE_MSGS_EVERY);
	unsigned long	ulTaskMgrWaitTime	= GetIniLong(IDS_INI_TASKMGR_WAITTIME);

	// Sort mailboxes marked to be sorted immediately
	POSITION TocPos = (TocTemplate? TocTemplate->GetFirstDocPosition() : NULL);
	while (TocPos)
	{
		CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(TocPos);
		// Sort the toc and break out after doing just one because
		// we don't want to take up a lot of time
		if (doc->SortNowIfNeeded())
			break;
	}

	static unsigned long ulCheckTimeForAutoSave = currentTime + 60; //currentTime + nAutoSaveEvery*60;
	//check every 1 minute for possible auto-saves
	if( nAutoSaveEvery && (currentTime >= ulCheckTimeForAutoSave) && (deltaTime > 1))
	{
		//auto save
		POSITION pos = CompMessageTemplate->GetFirstDocPosition();
		while (pos)
		{
			CCompMessageDoc* doc = (CCompMessageDoc*)CompMessageTemplate->GetNextDoc(pos);
			if (doc && doc->IsModified())
			{
				if( (currentTime - doc->m_NextAutoSaveTime) >= nAutoSaveEvery*60)
				{
					// Write out the toc and break out after doing just one because
					// we don't want to take up a lot of time
					CCursor::Ignore( TRUE );
					doc->OnSaveDocument(NULL);
					CCursor::Ignore( FALSE );
				//TRACE3("OnIdle: Msg %s saved at %s w/ auto-save time %s", doc->m_Sum->GetTitle(), 
				//			ctime((long *)&currentTime), ctime((long *)&(doc->m_NextAutoSaveTime)) );
				
					doc->m_NextAutoSaveTime = currentTime;
				}
			}
		}

		ulCheckTimeForAutoSave = currentTime + 60;
	}

	// Check for a Remote Instance
	static unsigned long lastOwnerLockCheck = 0;
	unsigned long freq = (unsigned long)GetIniShort(IDS_INI_CHECK_OWNER_FREQ);
	if ( freq > 0 && ( currentTime - lastOwnerLockCheck ) >= freq )
	{
		lastOwnerLockCheck = currentTime;

		if ( ! CheckOwnership() )
			return FALSE;
	}


	// Call the Plugin Idle Function making sure it's required first
	if (m_TransManager && CTranslatorManager::m_PluginIdle)
		m_TransManager->IdleEveryone(deltaTime, 0);


	// Check mail if told to, and we've past the next time to check mail.
	// Skipping checking mail if on a notebook in DC (battery) mode with the Setting flag on.
	SYSTEM_POWER_STATUS systemPower;
	if (m_ForceCheckMail ||
		((CMI || m_NextCheckMail) && currentTime >= m_NextCheckMail && !GetIniShort(IDS_INI_CONNECT_OFFLINE) &&
			(!GetIniShort(IDS_INI_DONOT_CHECK_BATTERY) || !GetSystemPowerStatus(&systemPower) || systemPower.ACLineStatus != 0)
		))
	{
		// Set next time to check mail
		m_NextCheckMail = currentTime + CMI * 60;
		// Check it
		GetMail(kAutomaticMailCheckBits);
		
		if (m_ForceCheckMail) 
			m_ForceCheckMail = FALSE;
		
		// Make sure it doesn't happen right away
		if ( CMI )
			m_NextCheckMail = max(m_NextCheckMail, (unsigned long)(time(NULL) + 60U));
		else
			m_NextCheckMail = 0;

		UpdateMailCheckLabel();
	}
	else if ( m_NextSendQueued && currentTime >= m_NextSendQueued 
				&& deltaTime > (unsigned long)GetIniShort(IDS_INI_IDLE_TIME) 
				&& !GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{
		// if delayed messages are put in the queue, and their time has come...
	 	SendQueuedMessages();
	}


#ifdef THREADED

	if( QCGetTaskManager()->IsPostProcessingRequested())
	{
		//Do post proceesing based on the request process command queue not being empty
		QCGetTaskManager()->DoPostProcessing();
	}

	if(deltaTime > ulTaskMgrWaitTime )
	{
		//if any of the tasks need post proceesing, request processing time
		if( QCGetTaskManager()->NeedsPostProcessing())
			QCGetTaskManager()->RequestPostProcessing();
	}

#endif


	if (deltaTime > 2)
	{
		// Periodically write out TOCs that are modified
		TocPos = (TocTemplate? TocTemplate->GetFirstDocPosition() : NULL);
		
		while (TocPos)
		{
			CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(TocPos);
			if (doc && doc->IsModified())
			{
				// Write out the toc and break out after doing just one because
				// we don't want to take up a lot of time
				CCursor::Ignore( TRUE );
				doc->OnSaveDocument(NULL);
				CCursor::Ignore( FALSE );
				break;
			}
		}
	}

	if (deltaTime > 30)
	{
		ReclaimTOCs();
	}

	return FALSE;
}

// --------------------------------------------------------------------------
//
// HandleRegError [PROTECTED]
//
// Call this function when any registry function fails. A dialog will be
// displayed with the error message and the full key path.
//
// Scott Manjourides, 7/28/98
//
void CEudoraApp::HandleRegError(HKEY hkey, LPCTSTR pRegPath, UINT nErrID)
{
	CRString sErrMsg(nErrID);
	CString sRegPath = pRegPath;

	if (HKEY_CLASSES_ROOT == hkey)
	{
		sRegPath = CString("HKEY_CLASSES_ROOT\\") + sRegPath;
	}
	else if (HKEY_CURRENT_USER == hkey)
	{
		sRegPath = CString("HKEY_CURRENT_USER\\") + sRegPath;
	}
	else if (HKEY_LOCAL_MACHINE == hkey)
	{
		sRegPath = CString("HKEY_LOCAL_MACHINE\\") + sRegPath;
	}
	else if (HKEY_USERS == hkey)
	{
		sRegPath = CString("HKEY_USERS\\") + sRegPath;
	}
	else if (HKEY_PERFORMANCE_DATA == hkey)
	{
		sRegPath = CString("HKEY_PERFORMANCE_DATA\\") + sRegPath;
	}
#if (WINVER >= 0x0400)
	else if (HKEY_CURRENT_CONFIG == hkey)
	{
		sRegPath = CString("HKEY_CURRENT_CONFIG\\") + sRegPath;
	}
	else if (HKEY_DYN_DATA == hkey)
	{
		sRegPath = CString("HKEY_DYN_DATA\\") + sRegPath;
	}
#endif // (WINVER >= 0x0400)

	CString ErrMsg = sErrMsg + CString("\nKey Name = \"") + sRegPath + CString("\"");
	MessageBox(NULL, (LPCTSTR) ErrMsg, AfxGetAppName(), MB_OK | MB_ICONERROR);
}

// --------------------------------------------------------------------------

BOOL CEudoraApp::AddToRegistry(HKEY hkey, const char *regPath, const char *value, const char *valueData, bool bShowError /* = false */){
    HKEY  hkResult;
	LONG err = 0; 

	DWORD dwDisposition;

	// register command line in the root
	err = RegCreateKeyEx(hkey, (LPCTSTR)regPath,0,NULL,	
    						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
    						&hkResult,&dwDisposition);
	
	if (ERROR_SUCCESS == err)
	{
		err = RegSetValueEx(hkResult, value, 0, REG_SZ, (const unsigned char*)valueData, strlen(valueData)+1);
		RegCloseKey(hkResult);

		if ((bShowError) && (ERROR_SUCCESS != err))
		{
			HandleRegError(hkey, regPath, IDS_REG_FAILED_WRITE_ERR); // Could not write to the key
		}
	}
	else
	{
		if (bShowError)
			HandleRegError(hkey, regPath, IDS_REG_FAILED_OPEN_ERR); // Could not create/open the key
	}

	return ((ERROR_SUCCESS == err) ? TRUE : FALSE);
}

BOOL CEudoraApp::RegisterIcon()
{
	char buf[_MAX_PATH + 1]; 
	
	CRString appTitle(AFX_IDS_APP_TITLE);

    BOOL err = AddToRegistry(HKEY_CLASSES_ROOT, appTitle, NULL, appTitle);

	// register the icon
	appTitle += CRString(IDS_REG_ICON);

	if (GetModuleFileName(AfxGetInstanceHandle(), buf, sizeof(buf)))
	{
 		GetShortPathName(buf,buf, sizeof(buf));
		err = AddToRegistry(HKEY_CLASSES_ROOT, appTitle, NULL, buf);
    } 
	return (err);
} 

BOOL CEudoraApp::RegisterMailto()
{
    HKEY  hkResult;
	long len = _MAX_PATH;
	char app[_MAX_PATH + 1]; 
	char cmd[_MAX_PATH + 1];
	char longApp[_MAX_PATH + 1];
	BOOL err;
		 
	GetModuleFileName(AfxGetInstanceHandle(), longApp, sizeof(longApp));
 	GetShortPathName(longApp,app, sizeof(app));
	
	strcpy(cmd,app);
	lstrcat(cmd," /m %1");

	err = AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAILTO_ICON), NULL, app);

	// This will add to main registy ... only do this if not the default mailto and user ok's 
	char defMailto[_MAX_PATH + 1]; 
	defMailto[0] = 0;
	// What's currently registered?
	err = RegOpenKey(HKEY_CLASSES_ROOT, (LPCTSTR)CRString(IDS_REG_MAILTO),&hkResult);
	err = RegQueryValue(hkResult,NULL, defMailto, &len);
	RegCloseKey(hkResult);

	// oops, it's no eudora as the default mailer
	char *appName = app;
	char *slash = strrchr(app, SLASH);
	if (slash)
		appName = ++slash;

    if (defMailto[0] && !strstr(defMailto, appName))
	{
		BOOL warn = GetIniShort(IDS_INI_WARN_DEFAULT_MAILTO);
		BOOL overWrite = GetIniShort(IDS_INI_DEFAULT_MAILTO_OVERWRITE);

        // Just overwrite or ask
		if (!gbAutomationRunning)
		{
			if ((!warn && overWrite) || warn &&
				(YesNoDialog(IDS_INI_WARN_DEFAULT_MAILTO, IDS_INI_DEFAULT_MAILTO_OVERWRITE,IDS_WARN_DEFAULT_MAILTO) == IDOK))

			{
				if (!AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_MAILTO), NULL, cmd, false)) // change to true to show the HKEY error
				{
					::ErrorDialog(IDS_REG_MAILTO_ERR);
				}
			}
		}
	}
	else if( !strstr(defMailto, app))
	{
		//We are the default program, but if we are a new version, register this new path in place of the old one..
		if (!AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_MAILTO), NULL, cmd, false)) // change to true to show the HKEY error
		{
			::ErrorDialog(IDS_REG_MAILTO_ERR);
		}
	}


	// This will work for IE 3.0 +
	err = AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAILTO), NULL, CRString(IDS_EUDORA));
	err = AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAILTO_PROT_COMMAND), NULL, cmd);
	err = AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAILTO_COMMAND), NULL, cmd);
	err = AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAILTO_PROT), NULL, "URL:MailTo Protocol");

	// This is for IE 4.0
	char MapiDLL[_MAX_PATH + 1];
	strcpy(MapiDLL, cmd);
	char* ExeStart = strrchr(MapiDLL, '\\') + 1;
	strcpy(ExeStart, "EuMAPI32.dll");
	err = AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAILTO), "DLLPath", MapiDLL);

 
	DWORD dwDisposition;
	unsigned long val = 0x02;

	err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR)CRString(IDS_REG_HLM_MAILTO_PROT),0,NULL,	
    						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
    						&hkResult,&dwDisposition);
	

	err = RegSetValueEx(hkResult,(LPCTSTR)CRString(IDS_REG_HLM_MAILTO_PROT_URL),0,REG_SZ,(const unsigned char*)"",1);
	RegCloseKey(hkResult);

	err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR)CRString(IDS_REG_HLM_MAILTO_PROT),0,NULL,	
    						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
    						&hkResult,&dwDisposition);
	

	err = RegSetValueEx(hkResult,(LPCTSTR)CRString(IDS_REG_HLM_MAILTO_PROT_ED),0,REG_BINARY,(CONST BYTE *)&val,sizeof(unsigned long));
	RegCloseKey(hkResult);

	return (err);
} 

BOOL CEudoraApp::RegisterStationery()
{
	char app[_MAX_PATH + 6];
		 
	GetModuleFileName(AfxGetInstanceHandle(), app, sizeof(app)); 
 	GetShortPathName(app,app, sizeof(app));
	lstrcat(app," \"%1\"");

	return(AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_STATIONERY), NULL, app));
} 


BOOL CEudoraApp::RegisterCommandLine(char *cmdLine)
{
    HKEY  hkResult;
	DWORD dwDisposition;
	LONG err = 0;

	char val[_MAX_PATH + _MAX_PATH + _MAX_PATH + 3]; // 3 = 2 spaces and a NULL
	char app[_MAX_PATH + 1];
	char eDir[_MAX_PATH + 1];
	char iniPath[_MAX_PATH + 1];

	val[0] = app[0] = eDir[0] = iniPath[0] = '\0';

	//strip	tailing SLASH
	strcpy(eDir,EudoraDir);
	if (eDir[strlen(eDir)-1] == SLASH)
		eDir[strlen(eDir)-1] = 0;
		 
	GetModuleFileName(AfxGetInstanceHandle(), app, sizeof(app)); 
 	VERIFY(GetShortPathName(app,app, sizeof(app)) > 0);
 	VERIFY(GetShortPathName(eDir,eDir, sizeof(eDir)) > 0);
 	VERIFY(GetShortPathName(INIPath,iniPath, sizeof(iniPath)) > 0);
	sprintf(val, "%s %s %s", app, eDir, (const char *)iniPath);

	// register command line in the root
	AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_SOFTWARE), NULL, val);

	// register command line in the current user
	char buf[_MAX_PATH + 1];
	strcpy(buf, CRString(IDS_REG_SOFTWARE_COMMAND));
	err = RegCreateKeyEx(HKEY_CURRENT_USER, (LPCTSTR)buf,0,NULL,	
    						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
    						&hkResult,&dwDisposition);
	
	strcpy(buf, CRString(IDS_REG_SOFTWARE_CURRENT));
	err = RegSetValueEx(hkResult,buf,0,REG_SZ,(const unsigned char*)val,strlen(val)+1);
	RegCloseKey(hkResult);

	return ( (BOOL)err );	
}

BOOL CEudoraApp::StartEudoraWithReg()
{
    HKEY  hkResult;
	DWORD dwType = REG_SZ;
	char buf[_MAX_PATH];
	char cmdLine[_MAX_PATH];
	LONG err;
	unsigned long buflen = _MAX_PATH;
	
	// HKEY_CURRENT_USER registry
	strcpy(buf, CRString(IDS_REG_SOFTWARE_COMMAND));
	err = RegOpenKeyEx(HKEY_CURRENT_USER, (LPCTSTR)buf,0,KEY_ALL_ACCESS,&hkResult);
	strcpy(buf, "Current");
	err = RegQueryValueEx(hkResult,buf, NULL, &dwType, (unsigned char*)cmdLine, &buflen);
	RegCloseKey(hkResult);
	
	if (err)
	{
		// root registry
		strcpy(buf, CRString(IDS_REG_SOFTWARE));
		err = RegOpenKeyEx(HKEY_CLASSES_ROOT, (LPCTSTR)buf,0,KEY_ALL_ACCESS,&hkResult);
		err = RegQueryValueEx(hkResult,NULL, NULL, &dwType, (unsigned char*)cmdLine, &buflen);
		RegCloseKey(hkResult);
	}

	buf[0] = 0;
	char * c = strchr(cmdLine, ' ');
	if (c)
		strcpy(buf,++c);
	 
	if (err || !GetDirs(buf))
	{
		ErrorDialog(IDS_ERR_CANNOT_START_WITH_ATTACH);
		return FALSE;
	}
	
	return (TRUE);	
}


// ProcessAttachmentWithExistingInstance()
BOOL CEudoraApp::ProcessAttachmentWEI(int CmdType)
{
	HWND otherEudoraHWND = ::FindWindow(EudoraMainWindowClass,NULL);

	if (otherEudoraHWND)
	{ 
		// if command line contained attachment have running eudora attach it
		if (m_lpCmdLine && *m_lpCmdLine && 
				( CmdType == CL_ATTACH		|| 
				  CmdType == CL_STATIONERY	||  
				  CmdType == CL_MAILTO		|| 
				  CmdType == CL_MESSAGE ) )
		{
	    	COPYDATASTRUCT cds;
		
			if ( CmdType == CL_ATTACH || CmdType == CL_MAILTO)
				cds.dwData = 1/*ATTACH_FILE*/;				
			else if(CmdType == CL_STATIONERY)
				cds.dwData = 2/*STATIONERY_FILE*/;	
			else if ( CmdType == CL_MESSAGE )
				cds.dwData = 3/*MESSAGE_FILE*/;

			cds.lpData = m_lpCmdLine;				//  Point to text
			cds.cbData = strlen(m_lpCmdLine) + 1;	//  Length of string

			::SendMessage(otherEudoraHWND, WM_COPYDATA, NULL, (LPARAM)&cds);
		}
		
		if (::IsIconic(otherEudoraHWND)) // Check if app is minimized
			::ShowWindow(otherEudoraHWND, SW_RESTORE); // Activate and display app, restoring from minimized
		else
			::SetForegroundWindow(otherEudoraHWND); // Bring app to front

		return TRUE;
	}
	return FALSE;
}


BOOL CEudoraApp::IsEudoraRunning()
{
	// Allow multiple instances on different working directories
	if ( GetIniShort(IDS_INI_OWNER_LOK) && ! EudoraDir.IsEmpty() )
	{
		CString cOwnerFile = EudoraDir;
		cOwnerFile += "OWNER.LOK";
		while (1)
		{
			m_fdOwner = open( cOwnerFile, O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE );
			if ( m_fdOwner != -1 )
			{
				// try to read an owner id
				DWORD dwTemp;
				int bytes = read( m_fdOwner, (LPVOID)&dwTemp, sizeof( DWORD ) );
				if ( bytes ==  sizeof( DWORD ) )
				{
					// someone's already using this Eudora directory - are they local?
					m_hOwner = (HWND)dwTemp;
					if ( IsLocalInstance( m_hOwner ) )
					{
						return LocalLateral();
					}

					int bytes = read( m_fdOwner, (LPVOID)&dwTemp, sizeof( DWORD ) );
					if ( ( bytes == sizeof(DWORD) && dwTemp == REMOTE_INSTANCE ) || 
						! IsLocalPath( cOwnerFile ) )
					{
						const BOOL bStopRunning = RemoteInstance();
						// If told to exit remote instance, then wait for
						// the remote instance to go away and try again
						if (!bStopRunning && m_hOwner == 0)
						{
							for (int i = 0; i < 10; i++)
							{
								if (!FileExistsMT(cOwnerFile))
									break;
								Sleep(2000);
							}
							continue;
						}

						return bStopRunning;
					}

					// it's not a local instance nor was it a remote instance
					// assume it's a stale local instance
					m_hOwner = NULL;
				}
			}
			break;
		}
	}

	return FALSE;	// let it rip...
}

BOOL CEudoraApp::IsLocalInstance( HWND hWnd )
{
	char buf[ 80 ];

	// try to find EUDORA in the window's title bar text
	int bytes;
	if ( ::IsWindow(hWnd) )
	{
		buf[ sizeof(buf) - 1 ] = '\0';
		if ( bytes = ::GetWindowText(hWnd, buf, sizeof(buf) - 1) )
		{
			strupr( buf );
			if ( strstr( buf, "EUDORA" ) )
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CEudoraApp::IsLocalPath( CString csPath )
{
	BOOL ret = TRUE;
	char path[ _MAX_PATH ];
	int drive;

	strncpy( path, csPath, _MAX_PATH - 1 );
	path[ _MAX_PATH - 1 ] = '\0';

	if ( strlen( path ) >= 2 )
	{
		if ( path[ 1 ] == ':' )
		{
			// non-UNC fully qualified path
			path[ 3 ] = '\0';
			strupr( path );
		}
		else if ( strstr( path, "\\\\" ) == path )
		{
			// UNC fully qualified path
			// UNCs are \\machine\sharename\ - find end of machine name
			char * end = strchr( &path[ 2 ], '\\' );
			if ( end )
			{
				// now find the end of the sharename
				end = strchr( end + 1, '\\' );
				if ( end )
					*(end + 1) = '\0';
			}
		}
		else
		{
			drive = _getdrive();		// 1 = 'A', 2 = 'B'
			drive -= 1;					// make em 0-indexed

			path[ 0 ] = char(drive + 'A');
			path[ 1 ] = ':';
			path[ 2 ] = '\\';
			path[ 3 ] = '\0';
		}

		// path now has the drive's root path

		// GetDriveType() isn't documented as working with UNCs under 95...
		// if it fails we'll return IsLocal == TRUE which is not that bad.
		// (I'd bet the documentation is wrong; this probably does work under 95)
		UINT type = GetDriveType( path );
		if ( type == DRIVE_REMOTE || type == DRIVE_UNKNOWN  )
			ret = FALSE;
	}

	return ret;
}

BOOL CEudoraApp::LocalLateral()
{
	if ( ::IsWindow( m_hOwner )	)
		::SetForegroundWindow( m_hOwner );

	return TRUE;	// causes this instance to terminate
}

BOOL CEudoraApp::RemoteInstance()
{
	CRemoteInstance theDlg;

	int result = theDlg.DoModal();
	switch ( result )
	{
	case CRemoteInstance::TERMINATE_REMOTE :
		// causes our HWND to be written to OWNER.LOC
		// this will subsequently cause a well-behaved Eudora to teminate itself
		m_hOwner = 0;
		RegisterOwnership();
		break;

	case CRemoteInstance::IGNORE_REMOTE :
		// leaves m_hOwner set to the remote's HWND
		// returning FALSE below causes this instance to continue
		break;

	case CRemoteInstance::TERMINATE_LOCAL :
	default:
		return TRUE;	// causes this instance to terminate
		break;
	}

	return FALSE;	// causes this instance to continue
}

void CEudoraApp::RegisterOwnership( void )
{
	HWND hOurWnd = m_pMainWnd? m_pMainWnd->m_hWnd : 0;

	// m_hOwner could be a remote instance (we might be sneekin' in the backdoor)
	if ( ! m_hOwner )
	{
		if ( m_fdOwner != -1 )
		{
			// make us the owner
			lseek( m_fdOwner, 0L, SEEK_SET );
			DWORD	dwTemp = (DWORD)hOurWnd;
			write( m_fdOwner, &dwTemp, sizeof( DWORD ) );

			if (hOurWnd && IsLocalPath( EudoraDir ) )
				dwTemp = LOCAL_INSTANCE;
			else
				dwTemp = REMOTE_INSTANCE;

			write( m_fdOwner, &dwTemp, sizeof( DWORD ) );

			// make sure it gets to disk...
			if (hOurWnd)
				_commit( m_fdOwner );
			else
			{
				close(m_fdOwner);
				m_fdOwner = -1;
			}
		}
		
		m_hOwner = hOurWnd;	// we are the owner
	}

	// save our hWnd for use in OwnershipCleanup() and runtime checks
	// m_pMainWnd->m_hWnd disappears early in the shutdown process
	m_hOurWnd = hOurWnd;
}

BOOL CEudoraApp::CheckOwnership( void )
{
	// see if we're still owners of the mailbox
	HWND hOwnerWnd = NULL;

	if ( m_fdOwner != -1 )
	{
		// get the current owner
		long retl = lseek( m_fdOwner, 0L, SEEK_SET );
		if ( retl != -1 )
		{
			DWORD	dwTemp;
			int reti = read( m_fdOwner, (LPVOID)&dwTemp, sizeof( DWORD ) );
			if ( reti == sizeof( DWORD ) )
				hOwnerWnd = (HWND)dwTemp;
		}

		// if not, then terminate the app
		if ( m_hOwner && hOwnerWnd != m_hOwner )
		{
#ifdef _DEBUG
			AfxMessageBox("PostQuitMessage() in CheckOwnership");
			ASSERT(0);
#endif
			::PostQuitMessage( 0 );
			return FALSE;
		}
	}

	return TRUE;
}

void CEudoraApp::OwnershipCleanup( void )
{
	HWND hOwnerWnd = NULL;

	if ( m_fdOwner != -1 )
	{
		// get the current owner
		lseek( m_fdOwner, 0L, SEEK_SET );
		DWORD	dwTemp;
		read( m_fdOwner, (LPVOID)&dwTemp, sizeof( DWORD ) );
		hOwnerWnd = (HWND)dwTemp;

		// close the ownership file
		close( m_fdOwner );
		m_fdOwner = -1;

		// if we owned it, then get rid of the file
		if ( m_hOurWnd && ( !hOwnerWnd || hOwnerWnd == m_hOurWnd ) )
		{
			CString cOwnerFile = EudoraDir;
			cOwnerFile += "OWNER.LOK";
			unlink( cOwnerFile );
		}
	}
}

void CEudoraApp::WinHelp(DWORD dwData, UINT nCmd /*=NULL*/)
{	
	// Clicking Shift-F1 and selecting a menu item
	//  brings up the help topic in a secondary
	//  window.  Better to display it in a popup.
	if ( (nCmd==NULL) || ( nCmd==HELP_CONTEXT )	)
		nCmd=HELP_CONTEXTPOPUP;

	CWinApp::WinHelp(dwData, nCmd);
 	return; 
}           

void CEudoraApp::SetExternalCheckMail(BOOL bWaitForIdle)
{

	if ( bWaitForIdle )
		SetNextMailCheck(time(NULL));
	else
		m_ForceCheckMail = TRUE; 
}


void CEudoraApp::OnFileOpen()
{
	CFileDialog dlg(TRUE,
					CRString(IDS_TEXT_EXTENSION),
					NULL,
					OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_OVERWRITEPROMPT, 
					//CRString(IDS_STATIONERY_FILE_FILTER32),
					CRString(IDS_MAIN_OPENFILE_FILTER),
					AfxGetMainWnd());
	
	if (dlg.DoModal() == IDOK)
	{
		CString fileSel = dlg.GetPathName(); 
		fileSel.MakeLower();
		CString sta = "." + CRString(IDS_STATIONERY_EXTENSION);
		if (fileSel.Find(sta) > 0)
		{
			CCompMessageDoc* Comp = NewCompDocumentWith(fileSel);
			Comp->SetIsStationery();
				
			if (Comp)
				Comp->m_Sum->Display();
		}

		else
			CWinApp::OpenDocumentFile(fileSel);
	}
}

void CEudoraApp::OnFileNew()
{
	//Had to explicitly use the TextFileTemplate bcos even the
	//Signature template uses IDR_TEXT_FILE which causes the DocManager
	//to put up a Dialog box to choose between the two.
	TextFileTemplate->OpenDocumentFile(NULL, TRUE);	
}

CEudoraApp::ResourceStatus CEudoraApp::LoadResourceDLL( void )
{
	char	appPath[_MAX_PATH];
	char	dllPath[_MAX_PATH];
	char *	temp;
	ResourceStatus status = RESOURCE_NOT_LOADED;

	appPath[0] = '\0';		// good hygiene
	dllPath[0] = '\0';		// good hygiene

	if ( ! m_pResList )
		m_pResList = new CPtrList;

	if ( m_pResList )
	{
  		if (GetModuleFileName(AfxGetInstanceHandle(), appPath, sizeof(appPath)))
		{
			strcpy(dllPath, appPath);
			temp = strrchr(dllPath, SLASH);
			if (temp)
			{
				temp[1] = 0;
			}
		}

		if ( dllPath[ 0 ] == '\0' )
			strcpy( dllPath, "\\" );


		if ( dllPath[ strlen( dllPath ) - 1 ] != '\\' )
			strcat( dllPath, "\\" );

		strcat( dllPath, "EUDORA32.DLL" );

		HINSTANCE hDLL = LoadLibrary( dllPath );
		if ( hDLL )
		{
			FARPROC pfInitDLL = GetProcAddress( hDLL, "INITDLL" );
				
			// 32-bit resource DLLs and plug-ins do not have an INITDLL fuction
			if ( ! pfInitDLL )
			{
				m_pResList->AddHead( (LPVOID)(DWORD)hDLL );
				status = RESOURCE_OKAY;
			}

			if ((RESOURCE_OKAY == status) && (!::IsMatchingResourceDLL(appPath, dllPath)))
				status = RESOURCE_VERSION_MISMATCH;
		}
	}

	return status;
}

void CEudoraApp::LoadEPIs( void )
{
	if ( ! m_pResList )
		m_pResList = new CPtrList;

	// last dll added is searched first my MFC
	if ( m_pResList )
	{
		// check the application dir
		AddEPIs( ExecutableDir );

		if ( ExecutableDir != EudoraDir )
		{
			// check the working dir
			AddEPIs( EudoraDir );
		}
	}
}

void CEudoraApp::FreeResources( void )
{
	// walk the list freeing libs as we go
	// finally free the list itself
	if ( m_pResList )
	{
		POSITION pos;

		for( pos = m_pResList->GetHeadPosition(); pos != NULL; )
		{
			HINSTANCE hInst = (HINSTANCE)m_pResList->GetNext( pos );
			if ( hInst && hInst != AfxGetInstanceHandle() )
				FreeLibrary( hInst );
		}

		delete m_pResList;
	}
}

void CEudoraApp::AddEPIs( CString & clsDirectory )
{
	CString		clsPath;
	
	if ( m_pResList )
	{
		FindFirstEPI( clsDirectory, clsPath );

		while ( ! clsPath.IsEmpty() )
		{
			// add the resource dll
			HINSTANCE hDLL = ::LoadLibrary( clsPath );
			if ( hDLL )
			{
				FARPROC pfInitDLL = ::GetProcAddress( hDLL, "INITDLL" );
				
				// 32-bit resource DLLs and plug-ins do not have an INITDLL fuction
				if ( ! pfInitDLL )
					m_pResList->AddHead( (LPVOID)(DWORD)hDLL );
				else
					::FreeLibrary( hDLL );
			}
			
			// look for another
			FindNextEPI( clsPath );
		}
	}

}

void CEudoraApp::FindFirstEPI( CString & clsDirectory, CString & clsEPIPath )
{
	// build up a sorted list of all EPIs
	CString	TmpPath;

	m_EPIList.RemoveAll();

	FindFirstEPIFile( clsDirectory, TmpPath );

	while ( ! TmpPath.IsEmpty() )
	{
		m_EPIList.Add( TmpPath );
		
		// look for another
		FindNextEPIFile( TmpPath );
	}

	// now start walking the sorted list 'z'..'a'
	m_Pos = m_EPIList.GetTailPosition();

	FindNextEPI( clsEPIPath );
}

void CEudoraApp::FindNextEPI( CString & clsEPIPath )
{
	if ( m_Pos )
		clsEPIPath = m_EPIList.GetPrev( m_Pos );
	else
		clsEPIPath = "";
}

void CEudoraApp::FindFirstEPIFile( CString & clsDirectory, CString & clsEPIPath )
{
	char	szFileSpec[ 255 ];

	// build up the wildcard specifier for Eudora Plug In's
	strcpy( szFileSpec, clsDirectory );
	if ( szFileSpec[ 0 ] == '\0' )
		strcpy( szFileSpec, "\\" );

	if ( szFileSpec[ strlen( szFileSpec ) - 1 ] != '\\' )
		strcat( szFileSpec, "\\" );

	m_clsFindDir = szFileSpec;

	strcat( szFileSpec, "*.EPI" );

	// see if any exist
	if (m_hFindHandle != INVALID_HANDLE_VALUE) 
	{
		FindClose( m_hFindHandle );
		m_hFindHandle = INVALID_HANDLE_VALUE;
	}

	m_FindData.dwFileAttributes = _A_NORMAL;

	m_hFindHandle = FindFirstFile( szFileSpec, &m_FindData );
	if (m_hFindHandle == INVALID_HANDLE_VALUE) 
	{
		clsEPIPath = "";
	}
	else
	{
		clsEPIPath = m_clsFindDir;
		clsEPIPath += m_FindData.cFileName;
	}
}

void CEudoraApp::FindNextEPIFile( CString & clsEPIPath )
{
	BOOL bResult;

	bResult = FindNextFile( m_hFindHandle, &m_FindData );
	if ( !bResult )
	{
		FindClose( m_hFindHandle );
		m_hFindHandle = INVALID_HANDLE_VALUE;
		clsEPIPath = "";
	}
	else
	{
		clsEPIPath = m_clsFindDir;
		clsEPIPath += m_FindData.cFileName;
	}
}

BOOL CEudoraApp::InitTranslators()
{
	CString transDir = EudoraDir + CRString(IDS_TRANS_DIRECTORY);
	CString transExeDir = ExecutableDir + CRString(IDS_TRANS_DIRECTORY);

	// Check to see if these are one in the same
	int numDir = 2;
	if (!transDir.CompareNoCase(transExeDir))
		numDir--;

	// see if we need to create "Trans" directory
	WIN32_FIND_DATA wfd;
	HANDLE findHandle;
	findHandle = FindFirstFile((const char *)transExeDir,&wfd);
	if ( findHandle == INVALID_HANDLE_VALUE)
		mkdir(transExeDir);
	else
		FindClose(findHandle);

	// Load 'em up
	m_TransManager = new CTranslatorManager((short)numDir, transDir, transExeDir);

	// Dump a trace of the translators loaded for debugging
#ifdef _DEBUG
	{
		CTranslator *t;
		CString		desc;
		short index = 0;
		while( t = m_TransManager->GetNextTranslator(&index))
		{
			t->getDescription(desc);
			TRACE1("Description: %s\n", desc);
		}
	}
#endif
	
	return TRUE;
}

void CEudoraApp::RegisterFormats()
{
	cfEmbeddedObject = (CLIPFORMAT)::RegisterClipboardFormat(_T("Embedded Object"));
	cfRTF = (CLIPFORMAT)::RegisterClipboardFormat(_T(CF_RTF));
	cfRTO = (CLIPFORMAT)::RegisterClipboardFormat(_T(CF_RETEXTOBJ));
	cfRTWO= (CLIPFORMAT)::RegisterClipboardFormat(_T(CF_RTFNOOBJS));

	VERIFY(g_theClipboardMgr.RegisterAllFormats());
}


//
// FilterList functions
//
//

void CEudoraApp::AddFilterListEntry( 
CObject* pEntry )
{
	BOOL		bTryAgain;
	POSITION	pos;

	bTryAgain = FALSE;
	
	do
	{
		try
		{
			m_FilterList.AddTail( pEntry );
		}
		catch( CMemoryException*	pExp )
		{	
			pExp->Delete();
			
			bTryAgain = !bTryAgain;
			
			if( bTryAgain )
			{
				// make some room by blowing away the oldest entry
				
				pos = m_FilterList.GetHeadPosition();
				
				if ( pos )
				{
					delete m_FilterList.RemoveHead();
				}

			}
			else
			{
				ErrorDialog( AFX_IDS_MEMORY_EXCEPTION );
			}
		}
	}
	while( bTryAgain );
}



const CObList&	CEudoraApp::GetFilterList()
{
	return m_FilterList;
}


void CEudoraApp::CleanupIcons()
{
	char Filename[_MAX_PATH + 1];

	CString iconDir = EudoraDir + CRString(IDS_ICON_FOLDER); 
	iconDir.MakeLower();

	wsprintf( Filename, "%s\\%s", iconDir, "*.jpg" );

	WIN32_FIND_DATA	Find;
	HANDLE			FindHandle;

	Find.dwFileAttributes = _A_NORMAL;
	FindHandle = FindFirstFile(Filename,&Find);
	if (FindHandle != INVALID_HANDLE_VALUE)
	{
		int Result = 1;
		while (Result)
		{
			wsprintf( Filename, "%s\\%s", iconDir, Find.cFileName );
			TRY
			{
				CFile::Remove( Filename );
			}
			CATCH( CFileException, e )
			{
				TRACE("Failed to delete %s\n", Filename);
			}
			END_CATCH
			Result = FindNextFile(FindHandle,&Find);
		}

		FindClose(FindHandle);
	}
}

void CEudoraApp::MakeEmbeddedDir()
{
	CString embeddedDir;

	WIN32_FIND_DATA wfd;
	embeddedDir = EudoraDir + CRString(IDS_EMBEDDED_FOLDER); 

	// see if we need to create "embedded" directory
	HANDLE findHandle;
	findHandle = FindFirstFile((const char *)embeddedDir,&wfd);
	if ( findHandle == INVALID_HANDLE_VALUE)
	{
		if (mkdir(embeddedDir) < 0)
			ASSERT( 0 );
	}
	else
		FindClose(findHandle);
}

//
// DoCheckProFilters [PUBLIC]
//
// Checks for full feature-only filters when running in reduced feature
// mode. Will disable filters if the user choses.
//
void CEudoraApp::DoCheckProFilters()
{
	// Shareware: Warn user if they are using restricted filters in REDUCED FEATURE mode 
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE mode
		CFilterActions filters;
		if ( filters.ContainsProActions() )
		{
			BOOL warn = GetIniShort(IDS_INI_WARN_PRO_ACTIONS);

			if ( warn )
			{
				if ( YesNoDialog(IDS_INI_WARN_PRO_ACTIONS, IDS_INI_DEFAULT_MAILTO_OVERWRITE,IDS_USE_FILTERS_PROMPT) == IDOK )
					SetIniShort(IDS_INI_USE_FILTERS, 1);
				else
					SetIniShort(IDS_INI_USE_FILTERS, 0);
			}
		}
	}
}
