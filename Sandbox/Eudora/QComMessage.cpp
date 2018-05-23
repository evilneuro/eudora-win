// QComMessage.cpp : Implementation of QComMessage

#include "stdafx.h"

#include "Automation.h"
#include "mime.h"
#include "MimeStorage.h"
#include "msgdoc.h"
#include "msgutils.h"
#include "QCAutomationDirector.h"
#include "QCMailboxCommand.h"
#include "QComApplication.h"
#include "QComFolder.h"
#include "QComMessage.h"
#include "QComMessages.h"
#include "summary.h"
#include "TextReader.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// QComMessage

STDMETHODIMP QComMessage::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IEuMessage,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

QComMessage::QComMessage()
{
	m_ID = 0L;
	m_pParentList = NULL;
	m_pParentFolder = NULL;
	m_bNewDoc = false;
	m_pDoc = NULL;
}

QComMessage::~QComMessage()
{

}

QComMessage* QComMessage::Create(void)
{
	HRESULT hr;
	CComObject<QComMessage>* p = NULL;
	hr = CComObject<QComMessage>::CreateInstance(&p);
	if (S_OK != hr) return NULL;
	p->AddRef();
	return p;
}

void QComMessage::SetParents(QComMessages* pParentList)
{
	if (!pParentList) return;
	m_pParentList = pParentList;
	m_pParentFolder = pParentList->GetParent();
}

bool QComMessage::GetDoc(void)
{
	CSummary* pSum= m_pParentList->FindSummaryByID(m_ID);
	if (!pSum) return false;
	m_pDoc = pSum->FindMessageDoc();
	m_bNewDoc = (pSum->FindMessageDoc() == NULL);
	m_pDoc = pSum->GetMessageDoc();
	if (!m_pDoc) return false;
	return true;
}

void QComMessage::ReleaseDoc(void)
{
	if (m_bNewDoc)
	{
		m_pDoc->OnCloseDocument();
		m_bNewDoc = false;
	}
	m_pDoc = NULL;
}

STDMETHODIMP QComMessage::get_RefCount(long * pVal)
{
	CAutomationCall c;
	*pVal = m_dwRef;
	return S_OK;
}

STDMETHODIMP QComMessage::get_From(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	try
	{
		CSummary* pSum= m_pParentList->FindSummaryByID(m_ID);
		if (!pSum) return E_FAIL;
		CString str = pSum->GetFrom();
		USES_CONVERSION;
		*pVal = A2BSTR(str);
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP QComMessage::get_To(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	try
	{
		CString str;
		if (GetDoc())
		{
			char* pText = m_pDoc->GetText();
			if (pText)
			{
				char * pTo = HeaderContents(IDS_HEADER_TO, pText);
				if (pTo)
				{
					str = pTo;
					delete [] pTo;
					USES_CONVERSION;
					*pVal = A2BSTR(str);
					ReleaseDoc();
					return S_OK;
				}
			}
			ReleaseDoc();
		}
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return E_FAIL;
}

STDMETHODIMP QComMessage::get_Subject(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	try
	{
		CSummary* pSum= m_pParentList->FindSummaryByID(m_ID);
		if (!pSum) return E_FAIL;
		CString str = pSum->GetSubject();
		USES_CONVERSION;
		*pVal = A2BSTR(str);
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP QComMessage::get_Body(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	try
	{
		if (!GetDoc()) return E_FAIL;
		CString str;
		if (!m_pDoc->m_QCMessage.IsInit())
		{
			char* szFullMessage = m_pDoc->GetFullMessage();
			if (!szFullMessage)
			{
				ReleaseDoc();
				return E_FAIL;
			}
			m_pDoc->m_QCMessage.Init( m_pDoc->m_MessageId, szFullMessage );
			delete [] szFullMessage;
		}
		m_pDoc->m_QCMessage.GetBody(str);
		ReleaseDoc();
		USES_CONVERSION;
		*pVal = A2BSTR(str);
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP QComMessage::get_BodyAsHTML(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	try
	{
		if (!GetDoc()) return E_FAIL;
		CString str;
		if (!m_pDoc->m_QCMessage.IsInit())
		{
			char* szFullMessage = m_pDoc->GetFullMessage();
			if (!szFullMessage) return E_FAIL;
			m_pDoc->m_QCMessage.Init( m_pDoc->m_MessageId, szFullMessage );
			delete [] szFullMessage;
		}
		m_pDoc->m_QCMessage.GetBodyAsHTML(str);
		ReleaseDoc();
		USES_CONVERSION;
		*pVal = A2BSTR(str);
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP QComMessage::get_ID(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = m_ID;
	return S_OK;
}

STDMETHODIMP QComMessage::get_Index(long * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	*pVal = m_Index;
	return S_OK;
}

STDMETHODIMP QComMessage::get_Date(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	try
	{
		CSummary* pSum= m_pParentList->FindSummaryByID(m_ID);
		if (!pSum) return E_FAIL;
		CString str = pSum->GetDate();
		USES_CONVERSION;
		*pVal = A2BSTR(str);
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP QComMessage::get_Priority(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	try
	{
		CSummary* pSum= m_pParentList->FindSummaryByID(m_ID);
		if (!pSum) return E_FAIL;
		CString str = pSum->GetPriority();
		USES_CONVERSION;
		*pVal = A2BSTR(str);
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP QComMessage::get_HeaderInfo(enHeaderField HeaderField, BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	try
	{
		int field;
		switch (HeaderField)
		{
		case hfAttachments: field = IDS_HEADER_ATTACHMENTS; break;
		case hfBcc: field = IDS_HEADER_BCC; break;
		case hfCc: field = IDS_HEADER_CC; break;
		case hfEmbeddedContent: field = IDS_HEADER_EMBEDDED_CONTENT; break;
		case hfFrom: field = IDS_HEADER_FROM; break;
		case hfHeaderDate: field = IDS_HEADER_DATE; break;
		case hfHeaderStatus: field = IDS_HEADER_STATUS; break;
		case hfInReplyTo: field = IDS_HEADER_IN_REPLY_TO; break;
		case hfMessageID: field = IDS_HEADER_MESSAGE_ID; break;
		case hfPersona: field = IDS_HEADER_PERSONA; break;
		case hfPrecedence: field = IDS_HEADER_PRECEDENCE; break;
		case hfPriority: field = IDS_HEADER_PRIORITY; break;
		case hfPriorityMatch: field = IDS_HEADER_PRIORITY_MATCH; break;
		case hfReferences: field = IDS_HEADER_REFERENCES; break;
		case hfReplyTo: field = IDS_HEADER_REPLY_TO; break;
		case hfSubject: field = IDS_HEADER_SUBJECT; break;
		case hfTo: field = IDS_HEADER_TO; break;
		default: return E_FAIL;
		}

		CString str("");
		if (GetDoc())
		{
			char* pText = m_pDoc->GetText();
			if (pText)
			{
				char * pTo = HeaderContents(field, pText);
				if (pTo)
				{
					str = pTo;
					delete [] pTo;
				}
				USES_CONVERSION;
				*pVal = A2BSTR(str);
				ReleaseDoc();
				return S_OK;
			}
			ReleaseDoc();
		}
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return E_FAIL;
}

STDMETHODIMP QComMessage::get_AttachmentListAsString(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	if (!GetDoc()) return E_FAIL;
	CString str;
	if (!m_pDoc->m_QCMessage.IsInit())
	{
		char* szFullMessage = m_pDoc->GetFullMessage();
		if (!szFullMessage)
		{
			ReleaseDoc();
			return E_FAIL;
		}
		m_pDoc->m_QCMessage.Init( m_pDoc->m_MessageId, szFullMessage );
		delete [] szFullMessage;
	}
	m_pDoc->m_QCMessage.GetAttachments(str);
	ReleaseDoc();
	USES_CONVERSION;
	*pVal = A2BSTR(str);
	return S_OK;
}

STDMETHODIMP QComMessage::Move(IEuFolder * NewParentFolder)
{
	if (!NewParentFolder) return E_FAIL;
	CAutomationCall c;
	try
	{
		long ID = GetID();
		if (!m_pParentList) return E_FAIL;
		CSummary* pSum = m_pParentList->FindSummaryByID(ID);
		if (!pSum) return E_FAIL;
		QComFolder* pNew = (QComFolder*)NewParentFolder;
		if (!pNew->m_bIsMailbox) return E_FAIL;
		if (!pNew->m_pCommand) return E_FAIL;
		g_theAutomationDirector.MoveMessage(pSum, pNew->m_pCommand);
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP QComMessage::get_Status(enMessageStatus * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	long ID = GetID();
	if (!m_pParentList) return E_FAIL;
	CSummary* pSum = m_pParentList->FindSummaryByID(ID);
	if (!pSum) return E_FAIL;
	*pVal = (enMessageStatus)pSum->m_State;
	return S_OK;
}

STDMETHODIMP QComMessage::put_Status(enMessageStatus newVal)
{
	CAutomationCall c;
	long ID = GetID();
	if (!m_pParentList) return E_FAIL;
	if (newVal < msUnread || newVal > msTimeQueued) return E_FAIL;
	CSummary* pSum = m_pParentList->FindSummaryByID(ID);
	if (!pSum) return E_FAIL;
	pSum->SetState((char)newVal);
	return S_OK;
}

STDMETHODIMP QComMessage::get_RawMessage(BSTR * pVal)
{
	CheckReturnPointer(pVal);
	CAutomationCall c;
	try
	{
		if (!GetDoc()) return E_FAIL;
		CString str;
		if (!m_pDoc->m_QCMessage.IsInit())
		{
			char* szFullMessage = m_pDoc->GetFullMessage();
			if (!szFullMessage)
			{
				return E_FAIL;
				ReleaseDoc();
			}
			m_pDoc->m_QCMessage.Init( m_pDoc->m_MessageId, szFullMessage );
			delete [] szFullMessage;
		}
		m_pDoc->m_QCMessage.GetRawMessage(str);
		ReleaseDoc();
		USES_CONVERSION;
		*pVal = A2BSTR(str);
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}


STDMETHODIMP QComMessage::get_BodyAsSimpleText(BSTR * pVal)
{
	// Setup automation call
	CheckReturnPointer(pVal);
	CAutomationCall c;
	short x;
	
	try
	{
		// Get message summary
		x = 1;
		CSummary* pSum= m_pParentList->FindSummaryByID(m_ID);
		if (!pSum) return E_FAIL;

		// Get message doc
		x = 2;
		if (!GetDoc()) return E_FAIL;

		// Get message body
		x = 3;
		CString str;
		if (!m_pDoc->m_QCMessage.IsInit())
		{
			char* szFullMessage = m_pDoc->GetFullMessage();
			if (!szFullMessage)
			{
				ReleaseDoc();
				return E_FAIL;
			}
			m_pDoc->m_QCMessage.Init( m_pDoc->m_MessageId, szFullMessage );
			delete [] szFullMessage;
		}
		x = 4;
		m_pDoc->m_QCMessage.GetBody(str);
		x = 5;
		ReleaseDoc();
		x = 6;

		// Make copy of body to work with
		char* MessageBody = ::SafeStrdupMT(str);
		int len = strlen(MessageBody);
		x = 7;

		// Strip message if HTML
		if (pSum->IsHTML())
		{
			x = 8;
			TextReader tr;
			x = 9;
			len = tr.StripHtml(MessageBody, len);
			x = 10;
			MessageBody[len] = 0;
			x = 11;
		}
		x = 12;
		
		// Strip message if RTF
		if (pSum->IsXRich())
		{
			x = 13;
			TextReader tr;
			x = 14;
			len = tr.StripRich(MessageBody, len);
			x = 15;
			MessageBody[len] = 0;
			x = 16;
		}
		x = 17;

		// Setup return value
		str = MessageBody;
		USES_CONVERSION;
		*pVal = A2BSTR(str);

		// Free allocated memory
		delete [] MessageBody;
	}
	catch (...)
	{
		ASSERT(0);
		return E_FAIL;
	}
	return S_OK;
}
