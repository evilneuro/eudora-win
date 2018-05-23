// QComMessages.cpp : Implementation of QComMessages

#include "stdafx.h"
#include "Automation.h"
#include "QCAutomationDirector.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QComFolder.h"
#include "QComMessage.h"
#include "QComMessages.h"
#include "summary.h"
#include "TocDoc.h"

#include "DebugNewHelpers.h"

extern QCMailboxDirector g_theMailboxDirector;

/////////////////////////////////////////////////////////////////////////////
// QComMessages

STDMETHODIMP QComMessages::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IEuMessages,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

QComMessages::QComMessages()
{
	m_pParent = NULL;
	m_bListReady = false;
}

QComMessages::~QComMessages()
{
	FreeMessageList();
}

bool QComMessages::UpdateTocPointers(void)
{
	m_pTocDoc = NULL;
	CString strFolderName = m_pParent->GetFullName();
	
	m_pTocDoc = GetToc((LPCSTR)strFolderName);

	ASSERT(m_pTocDoc);

	if (!m_pTocDoc)
		return false;

	return true;
}

long QComMessages::GetCount(void)
{
	if (!m_pParent)
		return 0L;

	if (!m_pParent->m_bIsMailbox)
		return 0L;

	if ((!m_bListReady) && (!CreateMessageList()))
		return 0L;

	if (IsEmpty())
		return 0L;

	return CPtrList::GetCount();
}

// --------------------------------------------------------------------------
//
// CreateMessageList [PRIVATE]
//
bool QComMessages::CreateMessageList(void)
{
	ASSERT(m_pParent);

	if (!m_pParent)
		return false;

	if (!m_pParent->m_bIsMailbox)
		return false;

	if (!UpdateTocPointers())
		return false;

	if (!m_pTocDoc)
		return false;
	
	// Free before creating new list
	FreeMessageList();

	CSumList &		listSums = m_pTocDoc->GetSumList();

	// Enumerate through summary list and record mail messages ids
	long MessageID;
	POSITION pos = listSums.GetHeadPosition();
	long Index = 1;
	while (pos)
	{
		CSummary* pSum = listSums.GetNext(pos);
		if (pSum)
		{
			MessageID = pSum->GetUniqueMessageId();
			if (MessageID > 0)
			{
				// Create COM message object
				QComMessage* pNewMessage = NULL;
				pNewMessage = QComMessage::Create();
				if (pNewMessage)
				{
					// Add it into our list
					pNewMessage->SetID(MessageID);
					pNewMessage->SetIndex(Index);
					pNewMessage->SetParents(this);
					Index++;
					InsertAtEnd(pNewMessage);
				}
			}
		}
	}
	return true;
}

void QComMessages::FreeMessageList(void)
{
	if (!m_pParent) return;
	if (!m_pParent->m_bIsMailbox) return;
	if (!IsEmpty())
	{
		QComMessage* pMessage;
		POSITION pos = GetHeadPosition();
		while (pos)
		{
			pMessage = (QComMessage*)GetNext(pos);
			pMessage->AutoFree();
		}
		RemoveAll();
	}
}

QComMessage* QComMessages::FindMessageByIndex(long Index)
{
	if (!m_pParent) return NULL;
	if (!m_pParent->m_bIsMailbox) return NULL;

	// Create list if it is ready
	if (!m_bListReady)
	{
		if (!CreateMessageList()) return NULL;
	}

	// Use one-based index for VB
	if ((Index < 1) || (Index > GetCount()))
		return NULL;

	// Find Message with matching index
	POSITION pos;
	pos = FindIndex(Index - 1);
	if (!pos) return NULL;
	QComMessage* pMessage = NULL;
	pMessage = (QComMessage *)GetAt(pos);
	return pMessage;
}

QComMessage* QComMessages::FindMessageByID(long ID)
{
	if (!m_pParent) return NULL;
	if (!m_pParent->m_bIsMailbox) return NULL;

	// Create list if it is ready
	if (!m_bListReady)
	{
		if (!CreateMessageList()) return NULL;
	}

	// Find Message with matching ID
	QComMessage* pMessage = NULL;
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		pMessage = (QComMessage*)GetNext(pos);
		if (pMessage)
		{
			if (pMessage->GetID() == ID)
			{
				return pMessage;
			}
		}
	}
	return NULL;
}

bool QComMessages::InsertAtEnd(QComMessage* pNewMessage)
{
	bool rc = true;
	try
	{
		CPtrList::AddTail((CObject*)pNewMessage);
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in QComMessages::InsertAtEnd" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in QComMessages::InsertAtEnd" );
		rc = false;
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in QComMessages::InsertAtEnd" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception (not std::bad_alloc) in QComMessages::InsertAtEnd" );
		rc = false;
	}

	return rc;
}

CSummary* QComMessages::FindSummaryByID(long ID)
{
	if (!m_pTocDoc) return NULL;

	CSumList &		listSums = m_pTocDoc->GetSumList();

	return listSums.GetByMessageId(ID);
}

void QComMessages::Remove(POSITION pos)
{
	// Free COM object
	QComMessage* pMessage = NULL;
	pMessage = (QComMessage *)GetAt(pos);
	pMessage->AutoFree();

	// Remove from linked list
	RemoveAt(pos);
}

QComMessages* QComMessages::Create(QComFolder* pParent)
{
	HRESULT hr;
	CComObject<QComMessages>* p = NULL;
	hr = CComObject<QComMessages>::CreateInstance(&p);
	if (S_OK != hr) return NULL;
	p->m_pParent = pParent;
	p->AddRef();
	return p;
}

STDMETHODIMP QComMessages::get_Count(long* retval)
{
	CheckReturnPointer(retval);
	CAutomationCall c;
	*retval = GetCount();
	return S_OK;
}

STDMETHODIMP QComMessages::get_Item(long Index, VARIANT* retval)
{
	CheckReturnPointer(retval);
	CAutomationCall c;

	// Check and initialize return value
	if (retval == NULL) return E_POINTER;
	VariantInit(retval);
	retval->vt = VT_UNKNOWN;
	retval->punkVal = NULL;

	// Find message
	QComMessage* pMessage = FindMessageByIndex(Index);
	if (!pMessage) return E_FAIL;

	// Return dispatch pointer to Message
	LPDISPATCH pDisp = NULL;
	HRESULT hr;
	hr = pMessage->AutoQueryInterface(IID_IDispatch, (void**)&pDisp);
	if (hr != S_OK) return E_FAIL;
	retval->vt = VT_DISPATCH;
	retval->pdispVal = pDisp;
	return S_OK;
}

STDMETHODIMP QComMessages::get_ItemByID(long ID, VARIANT* retval)
{
	CheckReturnPointer(retval);
	CAutomationCall c;

	// Check and initialize return value
	if (retval == NULL) return E_POINTER;
	VariantInit(retval);
	retval->vt = VT_UNKNOWN;
	retval->punkVal = NULL;

	// Find message
	QComMessage* pMessage = FindMessageByID(ID);
	if (!pMessage) return E_FAIL;

	// Return dispatch pointer to message
	LPDISPATCH pDisp = NULL;
	HRESULT hr;
	hr = pMessage->AutoQueryInterface(IID_IDispatch, (void**)&pDisp);
	if (hr != S_OK) return E_FAIL;
	retval->vt = VT_DISPATCH;
	retval->pdispVal = pDisp;
	return S_OK;
}

STDMETHODIMP QComMessages::get__NewEnum(IUnknown** ppEnum)
{
	CheckReturnPointer(ppEnum);
	CAutomationCall c;

	// Check for valid pointer
	if (ppEnum == NULL) return E_POINTER;

	// Set default return value
	*ppEnum = NULL;

	// Get number of items Message list
	long nSize = GetCount();

	// Create list of messages
	if (nSize)
	{
		if (!CreateMessageList()) return E_FAIL;
	}

	// Create variant array of that size
	long nArraySize = nSize;
	if (!nSize) nArraySize = 1;
	CComVariant* VarVect = NULL;
	VarVect = DEBUG_NEW_NOTHROW CComVariant[nArraySize];
	if (!VarVect) return E_OUTOFMEMORY;

	if (nSize)
	{
		// Populate variant array with pointers to mail Messages
		POSITION pos;
		long Index = 0;
		QComMessage* pMessage;
		pos = GetHeadPosition();
		LPDISPATCH lpDisp = NULL;
		HRESULT hr;
		while (pos)
		{
			pMessage = (QComMessage*)GetNext(pos);
			if (pMessage)
			{
				hr = pMessage->AutoQueryInterface(IID_IDispatch, (void**)&lpDisp);
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
	enumvar* p = DEBUG_NEW_NOTHROW enumvar;
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

STDMETHODIMP QComMessages::get_RefCount(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = m_dwRef;
	return S_OK;
}

STDMETHODIMP QComMessages::UpdateList()
{
	if (!m_pParent) return E_FAIL;
	if (!m_pParent->m_bIsMailbox) return E_FAIL;
	if (!CreateMessageList()) return E_FAIL;
	return S_OK;
}

STDMETHODIMP QComMessages::RemoveByID(long ID)
{
	CAutomationCall c;
	CSummary* pSum = FindSummaryByID(ID);
	if (!pSum) return E_FAIL;
	g_theAutomationDirector.RemoveMessage(pSum);
	return S_OK;
}
