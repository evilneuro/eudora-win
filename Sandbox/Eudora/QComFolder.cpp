// QComFolder.cpp : Implementation of QComFolder

#include "stdafx.h"
#include "Automation.h"
#include "QCCommandActions.h"
#include "QCMailboxCommand.h"
#include "QCImapMailboxCommand.h"
#include "QComApplication.h"
#include "QComFolder.h"
#include "QComFolders.h"
#include "QComMessages.h"
#include "TocDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// QComFolder

STDMETHODIMP QComFolder::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IEuFolder,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

QComFolder::QComFolder()
{
	m_ID = GetUniqueID();
	m_pParent = NULL;
	m_Index = 0;
	m_Level = 0;
	m_pCommand = NULL;
	m_bIsImap = false;
	m_bIsMailbox = false;
	m_pFolders = QComFolders::Create(this);
	m_pMessages = QComMessages::Create(this);
}

QComFolder::~QComFolder()
{
	// Release objects
	if (m_pFolders)	m_pFolders->AutoFree();
	if (m_pMessages) m_pMessages->AutoFree();
}

QComFolder* QComFolder::Create(void)
{
	HRESULT hr;
	CComObject<QComFolder>* p = NULL;
	hr = CComObject<QComFolder>::CreateInstance(&p);
	if (S_OK != hr) return NULL;
	p->AddRef();
	return p;
}

QComFolders* QComFolder::GetFolders(void)
{ 
	return m_pFolders;
}
	
QComMessages* QComFolder::GetMessages(void)
{ 
	return m_pMessages;
}
	
QCMailboxCommand* QComFolder::GetMailboxCommand(void)
{
	// This is a likely place to end up with an invalid
	// pointer if things get out of hand in Eudora
	// Let's check it ...
	if (!m_pCommand) return NULL;
	bool success = true;
	try
	{
		CString str = m_pCommand->GetName();
		if (str.GetLength() < 1) success = false;
	}
	catch (...)
	{
		ASSERT(0);
		success = false;
	}
	if (success)
	{
		return m_pCommand;
	}
	return NULL;
}

bool QComFolder::Init(QComFolder* pParent, QCMailboxCommand* pCommand, short Level)
{
	LPCSTR Name = pCommand->GetName();
	LPCSTR FullName = pCommand->GetPathname();
	m_pCommand = pCommand;
	// Determine if it's a mailbox
	bool bIsMailbox = false;
	switch (pCommand->GetType())
	{
	case MBT_IN:
	case MBT_OUT:
	case MBT_TRASH:
	case MBT_REGULAR:
		bIsMailbox = true;
		break;
#ifdef IMAP4
	case MBT_IMAP_MAILBOX:
		bIsMailbox = true;
#endif
	}

	// Determine if it's IMAP
	bool bIsImap = false;
#ifdef IMAP4
	if (pCommand->IsKindOf(RUNTIME_CLASS(QCImapMailboxCommand)))
	{
		bIsImap = true;
	}
#endif
	
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
	_splitpath(FullName, szDrive, szDir, NULL, NULL);
	CString strDrive = szDrive;
	CString strDir = szDir;
	CString strPath = strDrive + strDir;
	return Init(pParent, Name, strPath, FullName, bIsMailbox, bIsImap, Level);
}

bool QComFolder::Init(
	QComFolder* pParent,
	LPCSTR Name,
	LPCSTR Path,
	LPCSTR FullName,
	bool bIsMailbox,
	bool bIsImap,
	short Level)
{
	// Set attributes
	m_pParent = pParent;
	m_Name = Name;
	m_Path = Path;
	m_FullName = FullName;
	m_bIsMailbox = bIsMailbox,
	m_bIsImap = bIsImap,
	m_Level = Level;
	return true;
}

QComFolder* QComFolder::InsertNewChild(QCMailboxCommand* pc)
{
	QComFolder* pNewFolder;
	pNewFolder = QComFolder::Create();
	pNewFolder->Init(this, pc, (short)(GetLevel() + 1));
	m_pFolders->InsertAtEnd(pNewFolder);
	return pNewFolder;
}

void QComFolder::Unlink(void)
{
	ASSERT(m_pParent);
	if (!m_pParent) return;
	QComFolders* pFolderList = NULL;
	pFolderList = m_pParent->GetFolders();
	ASSERT(pFolderList);
	if (!pFolderList) return;
	POSITION pos;
	pos = pFolderList->Find((CObject*)this);
	ASSERT(pos);
	if (!pos) return;
	pFolderList->RemoveAt(pos);
}

STDMETHODIMP QComFolder::get_Name(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	// Get folder name
	USES_CONVERSION;
	*pVal = A2BSTR(m_Name);
	return S_OK;
}

STDMETHODIMP QComFolder::get_Path(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	// Get folder path
	USES_CONVERSION;
	*pVal = A2BSTR(m_Path);
	return S_OK;
}

STDMETHODIMP QComFolder::get_FullName(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	// Get folder path
	USES_CONVERSION;
	*pVal = A2BSTR(m_FullName);
	return S_OK;
}

STDMETHODIMP QComFolder::get_Level(short * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = m_Level;
	return S_OK;
}

STDMETHODIMP QComFolder::get_RefCount(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = m_dwRef;
	return S_OK;
}

STDMETHODIMP QComFolder::get_Folders(IEuFolders** pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	((IEuFolders*)m_pFolders)->AddRef();
	*pVal = m_pFolders;
	return S_OK;
}

STDMETHODIMP QComFolder::get_Messages(IEuMessages** pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	((IEuMessages*)m_pMessages)->AddRef();
	*pVal = m_pMessages;
	return S_OK;
}

STDMETHODIMP QComFolder::get_Index(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = m_Index;
	return S_OK;
}

STDMETHODIMP QComFolder::Open()
{
	CAutomationCall c;
	CTocDoc* pTocDoc = GetToc(m_FullName);
	if (pTocDoc)
	{
		pTocDoc->Display();
	}
	return S_OK;
}


STDMETHODIMP QComFolder::Move(IEuFolder * NewParentFolder)
{
	CAutomationCall c;
	try
	{
		// Get source mail folder command item
		if (!NewParentFolder) return E_FAIL;
		if (!m_pCommand) return E_FAIL;
		
		// Get destination mail folder command item
		QComFolder* pNewParentFolder = (QComFolder*)NewParentFolder;
		QCMailboxCommand* pDest = pNewParentFolder->GetMailboxCommand();
		if (!pDest)
		{
			// If it's the root folder, it's ok
			if (pNewParentFolder->GetLevel() !=0)
			{
				ASSERT(0);
				return E_FAIL;
			}
		}
		
		// Transfer mail folder
		m_pCommand->Execute(CA_GRAFT, pDest);
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP QComFolder::get_bCanContainMessages(BOOL * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = (true == m_bIsMailbox);
	return S_OK;
}

STDMETHODIMP QComFolder::get_bCanContainSubFolders(BOOL * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = (false == m_bIsMailbox);
	return S_OK;
}

STDMETHODIMP QComFolder::get_bIsImapFolder(BOOL * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = (true == m_bIsImap);
	return S_OK;
}

STDMETHODIMP QComFolder::get_bContainsUnreadMessages(BOOL * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	try
	{
		// Get mail folder command item
		if (!m_pCommand) return E_FAIL;
		
		// Get type
		UnreadStatusType status = m_pCommand->GetStatus();
		switch (status)
		{
		case US_UNKNOWN:
		case US_NO:
			*pVal = false;
			break;
		case US_YES:
			*pVal = true;
			break;
		}
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP QComFolder::get_ID(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = m_ID;
	return S_OK;
}
