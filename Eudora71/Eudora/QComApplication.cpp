// QComApplication.cpp : Implementation of QComApplication

#include "stdafx.h"

#include "Eudora.h"
#include "EudoraExe.h"
#include "..\Version.h"
#include "Automation.h"
#include "compmsgd.h"
#include "mainfrm.h"
#include "persona.h"
#include "QComApplication.h"
#include "QComFolder.h"
#include "QComFolders.h"
#include "QCAutomationDirector.h"
#include "resource.h"
#include "rs.h"
#include "Summary.h"
#include "TraceFile.h"
#include "guiutils.h"

#include "DebugNewHelpers.h"


#pragma warning(disable:4786)

/////////////////////////////////////////////////////////////////////////////
// QComApplication

QComApplication::QComApplication()
{
	// Set application COM object pointer to this object
	g_pApplication = this;

	// Asynchronously notify this app that the 
	// application COM object has been created
	g_pApp->m_pMainWnd->PostMessage(WM_USER_AUTOMATION_MSG, 0, 0);

	// Initialize data
	m_bInteractive = 0;
	m_strTemp = "";
	m_strPath = "";
	m_strName = "";
	m_strFullName = "";
	m_AppActiveObjectID = 0;

	// Get application path and name
	int c = 0;
	int i = -1;
	LPTSTR p;
	p = m_strFullName.GetBuffer(_MAX_PATH + 1);
	GetModuleFileName(AfxGetInstanceHandle(), p, _MAX_PATH + 1);
	
	while (*p != 0)
	{
		if (*p == '\\')
			i = c;
		c++;
		p++; 
	}
	m_strFullName.ReleaseBuffer();
	m_strPath = m_strFullName.Left(i);

	// Create list of all folders
	m_pRootFolder = QComFolder::Create();
	m_pRootFolder->Init(NULL, ROOTFOLDER, ROOTFOLDER, ROOTFOLDER, false, false, 0);
}

QComApplication::~QComApplication()
{
	m_pRootFolder->AutoFree();
}

void QComApplication::OnClose()
{
	FireOnClose();
}

QComApplication* QComApplication::Create(void)
{
	HRESULT hr;
	CComObject<QComApplication>* p = NULL;
	hr = CComObject<QComApplication>::CreateInstance(&p);
	if (S_OK != hr) return NULL;
	((IEuApplication*)p)->AddRef();
	return p;
}

void QComApplication::RegisterActiveObject(void)
{
	// Register the application in the Running Object Table
	::RegisterActiveObject((IUnknown*)((IEuApplication*)this),
							CLSID_EuApplication,
							ACTIVEOBJECT_WEAK /* ACTIVEOBJECT_STRONG */,
							&m_AppActiveObjectID);
}

void QComApplication::UnregisterActiveObject(void)
{
	if (!m_AppActiveObjectID) return;
	HRESULT hr = S_OK;
	hr = RevokeActiveObject(m_AppActiveObjectID, NULL);
	m_AppActiveObjectID = 0;
}

bool QComApplication::GetMainRect(LPRECT pRect)
{
	// return 1 if successful
	if (!g_pApp->m_pMainWnd) return 0;
	g_pApp->m_pMainWnd->GetWindowRect(pRect);
	return 1;
}

bool QComApplication::SetMainRect(LPRECT pR)
{
	// return 1 if successful
	if (!g_pApp->m_pMainWnd) return 0;
	if (!g_pApp->m_pMainWnd->SetWindowPos(
		NULL, 
		pR->left, 
		pR->top, 
		pR->right - pR->left,
		pR->bottom - pR->top,
		SWP_NOACTIVATE | SWP_NOZORDER)) return 0;
	return 1;
}

STDMETHODIMP QComApplication::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IEuApplication,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP QComApplication::get_FullName(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	// get full name of application (path plus exe name)
	USES_CONVERSION;
	*pVal = A2BSTR(m_strFullName);
	return S_OK;
}

STDMETHODIMP QComApplication::get_Path(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	// get application path (not including app name)
	USES_CONVERSION;
	*pVal = A2BSTR(m_strPath);
	return S_OK;
}

STDMETHODIMP QComApplication::get_Name(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	// get application name
	USES_CONVERSION;
	m_strTemp = g_pApp->m_pszAppName;
	*pVal = A2BSTR(m_strTemp);
	return S_OK;
}

STDMETHODIMP QComApplication::get_Application(IEuApplication** pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	// this is the application COM object
	*pVal = (IEuApplication *)this;
	return S_OK;
}

STDMETHODIMP QComApplication::get_Folders(IEuFolders** pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	QComFolders* pFolders = m_pRootFolder->GetFolders();
	if (!pFolders) return E_FAIL;
	pFolders->AutoAddRef();
	*pVal = (IEuFolders*)pFolders;
	return S_OK;
}

STDMETHODIMP QComApplication::get_RootFolder(IEuFolder** pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	QComFolder* pFolder = m_pRootFolder;
	if (!pFolder) return E_FAIL;
	pFolder->AutoAddRef();
	*pVal = (IEuFolder*)pFolder;
	return S_OK;
}

STDMETHODIMP QComApplication::get_Parent(IEuApplication** pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	// there is no parent app
	*pVal = NULL;
	return S_OK;
}

STDMETHODIMP QComApplication::get_Caption(BSTR* pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	// safety
	if (!g_pApp->m_pMainWnd) return E_FAIL;
	
	// get main frame window text
	USES_CONVERSION;
	*pVal = NULL;
	if (g_pApp->m_pMainWnd->GetWindowText(m_strTemp.GetBuffer(200), 200))
	{
		*pVal = A2BSTR(m_strTemp);
	}
	m_strTemp.ReleaseBuffer();
	return S_OK;
}

STDMETHODIMP QComApplication::put_Caption(BSTR newVal)
{
	CAutomationCall c;
	// safety
	if (!g_pApp || !g_pApp->m_pMainWnd) return E_FAIL;
	
	// set main frame window text
	m_strTemp = newVal;
	g_pApp->m_pMainWnd->SetWindowText(m_strTemp);
	return S_OK;
}

STDMETHODIMP QComApplication::get_Interactive(BOOL * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = true;
	return S_OK;
}

STDMETHODIMP QComApplication::get_Visible(BOOL * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = false;
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (pMainFrame)
	{
		*pVal = pMainFrame->IsWindowVisible();
	}
	return S_OK;
}

STDMETHODIMP QComApplication::put_Visible(BOOL newVal)
{
	CAutomationCall c;
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (pMainFrame)
	{
		if (newVal)
		{
			pMainFrame->ShowWindow(SW_RESTORE);
		}
		else
		{
			pMainFrame->ShowWindow(SW_HIDE);
		}
	}
	return S_OK;
}

STDMETHODIMP QComApplication::get_StatusBar(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	USES_CONVERSION;
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (pMainFrame)
	{
		if (pMainFrame->m_wndStatusBar.IsVisible())
		{
			pMainFrame->m_wndStatusBar.GetPaneText(0, m_strTemp);
			*pVal = A2BSTR(m_strTemp);
		}
	}
	return S_OK;
}

STDMETHODIMP QComApplication::put_StatusBar(BSTR newVal)
{
	CAutomationCall c;
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (pMainFrame)
	{
		if (pMainFrame->m_wndStatusBar.IsVisible())
		{
			m_strTemp = newVal;
			pMainFrame->m_wndStatusBar.SetPaneText(0, m_strTemp, true);
		}
	}
	return S_OK;
}

STDMETHODIMP QComApplication::get_Left(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	RECT rect;
	if (!GetMainRect(&rect)) return E_FAIL;
	*pVal = rect.left;
	return S_OK;
}

STDMETHODIMP QComApplication::put_Left(long newVal)
{
	CAutomationCall c;
	RECT rect;
	if (!GetMainRect(&rect)) return E_FAIL;
	long delta = newVal - rect.left;
	rect.left += delta;
	rect.right += delta;
	if (!SetMainRect(&rect)) return E_FAIL;
	return S_OK;
}

STDMETHODIMP QComApplication::get_Width(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	RECT rect;
	if (!GetMainRect(&rect)) return E_FAIL;
	*pVal = rect.right - rect.left;
	return S_OK;
}

STDMETHODIMP QComApplication::put_Width(long newVal)
{
	CAutomationCall c;
	RECT rect;
	if (!GetMainRect(&rect)) return E_FAIL;
	rect.right = rect.left + newVal;
	if (!SetMainRect(&rect)) return E_FAIL;
	return S_OK;
}

STDMETHODIMP QComApplication::get_Top(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	RECT rect;
	if (!GetMainRect(&rect)) return E_FAIL;
	*pVal = rect.top;
	return S_OK;
}

STDMETHODIMP QComApplication::put_Top(long newVal)
{
	CAutomationCall c;
	RECT rect;
	if (!GetMainRect(&rect)) return E_FAIL;
	long delta = newVal - rect.top;
	rect.top += delta;
	rect.bottom += delta;
	if (!SetMainRect(&rect)) return E_FAIL;
	return S_OK;
}

STDMETHODIMP QComApplication::get_Height(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	RECT rect;
	if (!GetMainRect(&rect)) return E_FAIL;
	*pVal = rect.bottom - rect.top;
	return S_OK;
}

STDMETHODIMP QComApplication::put_Height(long newVal)
{
	CAutomationCall c;
	RECT rect;
	if (!GetMainRect(&rect)) return E_FAIL;
	rect.bottom = rect.top + newVal;
	if (!SetMainRect(&rect)) return E_FAIL;
	return S_OK;
}

STDMETHODIMP QComApplication::CloseEudora()
{
	CAutomationCall c;
	if (!g_pApp->m_pMainWnd) return E_FAIL;
	gbAutomatedShutdown = true;
	g_pApp->m_pMainWnd->PostMessage(WM_CLOSE, 0, 0);
	return S_OK;
}

STDMETHODIMP QComApplication::get_RefCount(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = m_dwRef;
	return S_OK;
}

STDMETHODIMP QComApplication::get_ProcessID(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	DWORD ProcessID;
	ProcessID = GetCurrentProcessId();
	*pVal = (long)ProcessID;
	return S_OK;
}

STDMETHODIMP QComApplication::get_InBox(IEuFolder** pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	QComFolder* pFolder = NULL;
	CRString In(IDS_IN_MBOX_NAME);
	if (!g_theAutomationDirector.FindFolderByName(In, &pFolder)) return E_FAIL;
	if (!pFolder) return E_FAIL;

	// I *think* this is incorrect - FindFolderByName already incremented the
	// inbox ref count. I think that this extra ref count increment is what
	// necessitates the sledge hammer approach in AutoFree (in QComObject.h).
	// However, I'm not sure and the probable bug here combined with the sledge
	// hammer approach to freeing seem to work correctly. Since this is for
	// a bug fix release (7.0.1) I think it's the best approach to stick with
	// for the time being.
	pFolder->AutoAddRef();
	*pVal = pFolder;
	return S_OK;
}

STDMETHODIMP QComApplication::CheckMail(VARIANT Password)
{
	CAutomationCall c;
	gbAutomationCommandQueued = true;
	g_theAutomationDirector.m_bCommandCheckMail = true;
	if (VT_BSTR == Password.vt)
	{
		CString csPass = Password.bstrVal;
		g_theAutomationDirector.UsePassword(csPass);
	}
	else
	{
		g_theAutomationDirector.UsePassword("");
	}
	return S_OK;
}

STDMETHODIMP QComApplication::Folder(BSTR Name, BOOL FindRecursive, IEuFolder** pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	QComFolder* pFolder = NULL;
	CString strName = Name;
	bool bRecursive = (FindRecursive != 0);
	if (!g_theAutomationDirector.FindFolderByName(strName, &pFolder, bRecursive)) return E_FAIL;
	if (!pFolder) return E_FAIL;

	// I *think* this is incorrect - FindFolderByName already incremented the
	// folder ref count. I think that this extra ref count increment is what
	// necessitates the sledge hammer approach in AutoFree (in QComObject.h).
	// However, I'm not sure and the probable bug here combined with the sledge
	// hammer approach to freeing seem to work correctly. Since this is for
	// a bug fix release (7.0.1) I think it's the best approach to stick with
	// for the time being.
	pFolder->AutoAddRef();
	*pVal = pFolder;
	return S_OK;
}

STDMETHODIMP QComApplication::FolderByID(long ID, BOOL FindRecursive, IEuFolder** pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	QComFolder* pFolder = NULL;
	bool bRecursive = (FindRecursive != 0);
	if (!g_theAutomationDirector.FindFolderByID(ID, &pFolder, bRecursive)) return E_FAIL;
	if (!pFolder) return E_FAIL;
	pFolder->AutoAddRef();
	*pVal = pFolder;
	return S_OK;
}

STDMETHODIMP QComApplication::QueueMessage(BSTR To, BSTR Subject, BSTR Cc, BSTR Bcc, BSTR Attach, BSTR Body)
{
	CAutomationCall c;
	CString csTo = To;
	CString csSubject = Subject;
	CString csCc = Cc;
	CString csBcc = Bcc;
	CString csAttachments = Attach;
	CString csBody = Body;
	CString csEmpty = "";
	CCompMessageDoc* pCompDoc = NULL;



	
	// Set personality
	//!! default to dominant for now
	CString csPersona("");
	g_Personalities.SetCurrent(csPersona);

	// Create the message
	pCompDoc = NewCompDocument(
		csTo, 
		NULL,
		csSubject, 
		csCc, 
		csBcc,
		csAttachments, 
		csBody);
	if (!pCompDoc) return E_FAIL;


	// Queue the message
	pCompDoc->m_Sum->UnsetFlag(MSF_USE_SIGNATURE | MSF_ALT_SIGNATURE);
//	pCompDoc->m_Sum->UnsetFlag(MSF_TEXT_AS_DOC);
	pCompDoc->m_Sum->SetFlagEx(MSFEX_AUTO_ATTACHED);
	pCompDoc->Queue(TRUE);
		
	bool bAuto_Send = TRUE;

	if (GetIniShort(IDS_INI_WARN_COM_AUTO_SEND))
			{
		 	CFrameWnd* p_mainframe = (CFrameWnd *) AfxGetMainWnd();
			ASSERT(p_mainframe != NULL);
			p_mainframe->ActivateFrame();
			SetForegroundWindow(p_mainframe->GetSafeHwnd());

			if ( WarnYesNoDialog(IDS_INI_WARN_COM_AUTO_SEND, IDS_WARN_COM_AUTO_SEND) == IDOK)
				{
				bAuto_Send = FALSE;
				}
			}

	if (!bAuto_Send)
		{
			//
			// Not auto-sending, so make sure Eudora gets
			// brought to the foreground, even if it is
			// minimized.  This gives a hint to the user
			// that they need to finish addressing the
			// message and then send it on its way.
			//
			CFrameWnd* p_mainframe = (CFrameWnd *) AfxGetMainWnd();
			ASSERT(p_mainframe != NULL);
			p_mainframe->ActivateFrame();
			SetForegroundWindow(p_mainframe->GetSafeHwnd());
			//
			// Display the outgoing message window.
			//
			pCompDoc->m_Sum->Display();
			pCompDoc->SetModifiedFlag();
		}

	return S_OK;
}

STDMETHODIMP QComApplication::SendQueuedMessages()
{
	CAutomationCall c;
	gbAutomationCommandQueued = true;
	g_theAutomationDirector.m_bCommandSendMail = true;
	return S_OK;
}

STDMETHODIMP QComApplication::get_VersionMajor(short * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = (short)EUDORA_VERSION1;
	return S_OK;
}

STDMETHODIMP QComApplication::get_VersionMinor(short * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = (short)EUDORA_VERSION2;
	return S_OK;
}

STDMETHODIMP QComApplication::get_VersionMinor2(short * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = (short)EUDORA_VERSION3;
	return S_OK;
}

STDMETHODIMP QComApplication::get_VersionBuild(short * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = (short)EUDORA_VERSION4;
	return S_OK;
}

STDMETHODIMP QComApplication::CompactFolders()
{
	CAutomationCall c;
	gbAutomationCommandQueued = true;
	g_theAutomationDirector.m_bCommandCompactFolders = true;
	return S_OK;
}

STDMETHODIMP QComApplication::EmptyTrash()
{
	CAutomationCall c;
	gbAutomationCommandQueued = true;
	g_theAutomationDirector.m_bCommandEmptyTrash = true;
	return S_OK;
}
