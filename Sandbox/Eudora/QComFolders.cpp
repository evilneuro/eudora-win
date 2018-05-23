// QComFolders.cpp : Implementation of QComFolders

#include "stdafx.h"
#include "Automation.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QComFolder.h"
#include "QComFolders.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern QCMailboxDirector g_theMailboxDirector;

/////////////////////////////////////////////////////////////////////////////
// QComFolders

QComFolders::QComFolders()
{
	m_pParent = NULL;
}

QComFolders::~QComFolders()
{
	if (!IsEmpty())
	{
		QComFolder* pFolder;
		POSITION pos = GetHeadPosition();
		while (pos)
		{
			pFolder = (QComFolder*)GetNext(pos);
			pFolder->AutoFree();
		}
		RemoveAll();
	}
}

STDMETHODIMP QComFolders::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IEuFolders,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

bool QComFolders::InsertAtEnd(QComFolder* pNewFolder)
{
	bool rc = true;
	try
	{
		CPtrList::AddTail((CObject*)pNewFolder);
	}
	catch (...)
	{
		ASSERT(0);
		rc = false;
	}
	InternalUpdateFolderIndexes();
	return rc;
}

bool QComFolders::InsertAtIndex(QComFolder* pNewFolder, long Index)
{
	bool rc = true;
	QComFolder* pFolder;
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		POSITION pos2 = pos;
		pFolder = (QComFolder*)GetNext(pos);
		if (!pFolder) continue;
		if (pFolder->m_Index >= Index)
		{
			// Found folder to insert before
			try
			{
				CPtrList::InsertBefore(pos2, (CObject*)pNewFolder);
			}
			catch (...)
			{
				ASSERT(0);
				rc = false;
			}
			InternalUpdateFolderIndexes();
			return rc;
		}
	}
	ASSERT(0);
	return false;
}

void QComFolders::Remove(POSITION pos)
{
	// Free COM object
	QComFolder* pFolder = NULL;
	pFolder = (QComFolder *)GetAt(pos);
	pFolder->AutoFree();

	// Remove from linked list
	RemoveAt(pos);
}

void QComFolders::InternalUpdateFolderIndexes(void)
{
	// Should be called only for an object containing the list
	// of all folders in a mailstore / account
	long Index = 0;
	QComFolder* pFolder;
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		pFolder = (QComFolder*)GetNext(pos);
		if (!pFolder) break;
		pFolder->m_Index = Index;
		Index++;
	}
	return;
}

QComFolder* QComFolders::FindFolderByID(long ID)
{
	QComFolder* pFolder;
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		pFolder = (QComFolder*)GetNext(pos);
		if (!pFolder) break;
		if (pFolder->m_ID == ID)
		{
			return pFolder;
		}
	}
	return NULL;
}

QComFolder* QComFolders::FindFolderByIndex(long Index)
{
	// Use one-based index for VB
	if ((Index < 1) || (Index > GetCount()))
		return NULL;

	// Find folder with matching index
	POSITION pos;
	pos = FindIndex(Index - 1);
	if (!pos) return NULL;
	QComFolder* pFolder = NULL;
	pFolder = (QComFolder *)GetAt(pos);
	return pFolder;
}

QComFolders* QComFolders::Create(QComFolder* pParent)
{
	HRESULT hr;
	CComObject<QComFolders>* p = NULL;
	hr = CComObject<QComFolders>::CreateInstance(&p);
	if (S_OK != hr) return NULL;
	p->m_pParent = pParent;
	p->AddRef();
	return p;
}

STDMETHODIMP QComFolders::get_Count(long* retval)
{
	CheckReturnPointer(retval);
	CAutomationCall c;
	*retval = GetCount();
	return S_OK;
}

STDMETHODIMP QComFolders::get_Item(long Index, VARIANT* retval)
{
	CheckReturnPointer(retval);
	CAutomationCall c;
	// Check and initialize return value
	if (retval == NULL) return E_POINTER;
	VariantInit(retval);
	retval->vt = VT_UNKNOWN;
	retval->punkVal = NULL;

	// Find folder
	QComFolder* pFolder = FindFolderByIndex(Index);
	if (!pFolder) return E_FAIL;

	// Return dispatch pointer to folder
	LPDISPATCH pDisp = NULL;
	HRESULT hr;
	hr = pFolder->AutoQueryInterface(IID_IDispatch, (void**)&pDisp);
	if (hr != S_OK) return E_FAIL;
	retval->vt = VT_DISPATCH;
	retval->pdispVal = pDisp;
	return S_OK;
}

STDMETHODIMP QComFolders::get__NewEnum(IUnknown** ppEnum)
{
	CheckReturnPointer(ppEnum);
	CAutomationCall c;

	// Check for valid pointer
	if (ppEnum == NULL) return E_POINTER;

	// Set default return value
	*ppEnum = NULL;

	// Get number of items folder list
	long nSize = 0;
	if (!IsEmpty())
	{
		nSize = GetCount();
	}
	
	// Create variant array of that size
	long nArraySize = nSize;
	if (!nSize) nArraySize = 1;
	CComVariant* VarVect = NULL;
	VarVect = new CComVariant[nArraySize];
	if (!VarVect) return E_OUTOFMEMORY;

	if (nSize)
	{
		// Populate variant array with pointers to mail folders
		POSITION pos;
		long Index = 0;
		QComFolder* pFolder;
		pos = GetHeadPosition();
		LPDISPATCH lpDisp = NULL;
		HRESULT hr;
		while (pos)
		{
			pFolder = (QComFolder*)GetNext(pos);
			if (pFolder)
			{
				hr = pFolder->AutoQueryInterface(IID_IDispatch, (void**)&lpDisp);
				if (hr != S_OK) break;

				// Set variant item to dispatch pointer
				VarVect[Index].vt = VT_DISPATCH;
				VarVect[Index].pdispVal = lpDisp;
				
				// Increment and check index
				Index++;
				if (Index >= nSize) break;
			}
		}
		// See if all items were successful
		if (Index != nSize)
		{
			ASSERT(0);
			nSize = Index;
		}
	}

	// Create new IEnumVARIANT object using ATL		
	typedef CComObject<CComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> > > enumvar;
	enumvar* p = new enumvar;
	if (NULL == p) return E_OUTOFMEMORY;
	
	// Initialize enumerator
	HRESULT hRes = p->Init(
		&VarVect[0],
		&VarVect[nSize],
		NULL,
		AtlFlagCopy);
	if (FAILED(hRes))
	{
		delete p;	
		return E_OUTOFMEMORY;
	}

	// Cleanup
	delete [] VarVect;

	// Set return value to IUnknown of new IEnumVARIANT object
	hRes = p->QueryInterface(IID_IEnumVARIANT, (void**)ppEnum);
	return hRes;
}

STDMETHODIMP QComFolders::RemoveByID(long ID)
{
	CAutomationCall c;
	QComFolder* pFolder = FindFolderByID(ID);
	if (!pFolder) return E_FAIL;

	// This will trigger the delete calls and
	// everything will take care of itself
	try
	{
		// Get mail folder command item
		if (!pFolder->m_pCommand) return E_FAIL;
		
		// Delete mail folder
		pFolder->m_pCommand->Execute(CA_DELETE, NULL);
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP QComFolders::get_RefCount(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = m_dwRef;
	return S_OK;
}

STDMETHODIMP QComFolders::Add(BSTR Name, BOOL bCanContainMessages)
{
	CAutomationCall c;
	CString strName;
	strName = Name;
	QCMailboxCommand* pc = NULL;
	if (!m_pParent) return E_FAIL;
	pc = m_pParent->GetMailboxCommand();
	if (pc)
	{
		switch (pc->GetType())
		{
		case MBT_IN:
		case MBT_OUT:
		case MBT_TRASH:
		case MBT_REGULAR:
			return E_FAIL;
#ifdef IMAP4
		case MBT_IMAP_ACCOUNT:
		case MBT_IMAP_NAMESPACE:
		case MBT_IMAP_MAILBOX:
			break;
#endif
		}
	}
	MailboxType mbType = MBT_FOLDER;
	if (bCanContainMessages != 0)
	{
		mbType = MBT_REGULAR;
	}
	g_theMailboxDirector.AddCommand((LPCSTR)strName, mbType, pc);
	return S_OK;
}
