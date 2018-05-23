////////////////////////////////////////////////////////////////////////
//
// CDDEServer
// Class instantiated and used by DDE server side.
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef WIN32
#include <afxcmn.h>
#include <afxrich.h>
#else
typedef unsigned long ULONG;
#endif

#include "ddestr.h"
#include "ddeserve.h"
#include "ddeclien.h"
#include "doc.h"
#include "nickdoc.h"

#include "resource.h"	// for MAINFRM.H
#include "mainfrm.h"

#include "summary.h"	// for MSGDOC.H
#include "msgdoc.h"		// for COMPMSGD.H
#include "compmsgd.h"

#include "tocdoc.h"
#include "urledit.h"



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//
// Statics.
//
CDDEServer* CDDEServer::m_pTheServer = NULL;


////////////////////////////////////////////////////////////////////////
// CDDEServer [public, constructor]
//
////////////////////////////////////////////////////////////////////////
CDDEServer::CDDEServer(void) :
	m_InstId(NULL),
	m_pCallbackFn(NULL),
	m_pServiceName(NULL),
	m_pSystemTopic(NULL),
	m_pWWWOpenURLTopic(NULL),
	m_pMAPITopic(NULL),
	m_pExpandNicknameTopic(NULL),
	m_pGetNextMessageIdTopic(NULL),
	m_pPutMessageByIdTopic(NULL),
	m_pDeleteMessageByIdTopic(NULL),
	m_pMAPIOriginatorItem(NULL),
	m_pMAPINicknamesItem(NULL),
	m_pMAPIServerVersionItem(NULL)
{
//	TRACE0("CDDEServer::CDDEServer\n");
}


////////////////////////////////////////////////////////////////////////
// ~CDDEServer [public, destructor]
//
////////////////////////////////////////////////////////////////////////
CDDEServer::~CDDEServer(void)
{
	//
	// FORNOW, empty ... but should we call uninitialize explicitly?
	//
//	TRACE0("CDDEServer::~CDDEServer\n");
}


////////////////////////////////////////////////////////////////////////
// Initialize [public]
//
// Called once per object to start DDEML.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEServer::Initialize(void)
{
//	TRACE0("CDDEServer::Initialize\n");
	if (m_InstId != NULL)
	{
		ASSERT(0);			// caller blew it, we're already initialized
		return FALSE;
	}

	ASSERT(NULL == m_pCallbackFn);
	ASSERT(NULL == m_pServiceName);
	ASSERT(NULL == m_pSystemTopic);
	ASSERT(NULL == m_pWWWOpenURLTopic);
	ASSERT(NULL == CDDEServer::m_pTheServer);
	ASSERT(NULL == m_pMAPITopic);
	ASSERT(NULL == m_pExpandNicknameTopic);
	ASSERT(NULL == m_pGetNextMessageIdTopic);
	ASSERT(NULL == m_pGetMessageByIdTopic);
	ASSERT(NULL == m_pPutMessageByIdTopic);
	ASSERT(NULL == m_pDeleteMessageByIdTopic);
	ASSERT(NULL == m_pMAPIOriginatorItem);
	ASSERT(NULL == m_pMAPINicknamesItem);
	ASSERT(NULL == m_pMAPIServerVersionItem);

	//
	// Hack alert!  The callback function has to know which DDE server
	// object to use.
	//
	CDDEServer::m_pTheServer = this;

#ifdef WIN32
	m_pCallbackFn = (PFNCALLBACK) DDEServerCallback;
#else
	m_pCallbackFn = (PFNCALLBACK) MakeProcInstance((FARPROC) DDEServerCallback, AfxGetInstanceHandle());
#endif // WIN32
	if (NULL == m_pCallbackFn)
	{
		Uninitialize();
		return FALSE;
	}

	switch (::DdeInitialize(&m_InstId, 
	    					m_pCallbackFn, 
	    					APPCLASS_STANDARD | 
	    					CBF_FAIL_ADVISES | 
	    					CBF_FAIL_EXECUTES |
	    					CBF_FAIL_POKES,
	    					0))
	{
	case DMLERR_NO_ERROR:
		// everything is beautiful....
		ASSERT(m_InstId != NULL);
		break;
	case DMLERR_DLL_USAGE:
	case DMLERR_INVALIDPARAMETER:
	case DMLERR_SYS_ERROR:
		ASSERT(NULL == m_InstId);
		Uninitialize();
		return FALSE;
	default:
		ASSERT(NULL == m_InstId);
		Uninitialize();
		return FALSE;
	}

	//
	// Now, allocate DDE string objects for topic/item names.
	//
	if (NULL == (m_pMAPIServerVersionItem = new CDDEString(m_InstId, "ServerVersion")))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pMAPIOriginatorItem = new CDDEString(m_InstId, "Originator")))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pMAPINicknamesItem = new CDDEString(m_InstId, "Nicknames")))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pGetNextMessageIdTopic = new CDDEString(m_InstId, "GetNextMessageId")))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pGetMessageByIdTopic = new CDDEString(m_InstId, "GetMessageById")))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pPutMessageByIdTopic = new CDDEString(m_InstId, "PutMessageById")))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pDeleteMessageByIdTopic = new CDDEString(m_InstId, "DeleteMessageById")))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pExpandNicknameTopic = new CDDEString(m_InstId, "ExpandNickname")))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pMAPITopic = new CDDEString(m_InstId, "MAPI")))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pWWWOpenURLTopic = new CDDEString(m_InstId, "WWW_OpenURL")))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pSystemTopic = new CDDEString(m_InstId, SZDDESYS_TOPIC)))
	{
		Uninitialize();
		return FALSE;
	}

	if (NULL == (m_pServiceName = new CDDEString(m_InstId, "EUDORA")))
	{
		Uninitialize();
		return FALSE;
	}

	//
	// Last step.  Register the service name for this server with DDEML.
	//
	if (! ::DdeNameService(m_InstId, m_pServiceName->GetHandle(), NULL, DNS_REGISTER))
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// Uninitialize [public]
//
// Called once per object shutdown DDEML.
//
////////////////////////////////////////////////////////////////////////
void CDDEServer::Uninitialize(void)
{
//	TRACE0("CDDEServer::Uninitialize\n");
	//
	// Assume that if the service name object exists, that it was
	// registered.
	//
	if (m_pServiceName)
	{
		ASSERT(m_InstId);
		::DdeNameService(m_InstId, m_pServiceName->GetHandle(), NULL, DNS_UNREGISTER);
		delete m_pServiceName;
		m_pServiceName = NULL;
	}

	if (m_pSystemTopic)
	{
		delete m_pSystemTopic;
		m_pSystemTopic = NULL;
	}

	if (m_pWWWOpenURLTopic)
	{
		delete m_pWWWOpenURLTopic;
		m_pWWWOpenURLTopic = NULL;
	}

	if (m_pMAPITopic)
	{
		delete m_pMAPITopic;
		m_pMAPITopic = NULL;
	}

	if (m_pExpandNicknameTopic)
	{
		delete m_pExpandNicknameTopic;
		m_pExpandNicknameTopic = NULL;
	}

	if (m_pDeleteMessageByIdTopic)
	{
		delete m_pDeleteMessageByIdTopic;
		m_pDeleteMessageByIdTopic = NULL;
	}

	if (m_pPutMessageByIdTopic)
	{
		delete m_pPutMessageByIdTopic;
		m_pPutMessageByIdTopic = NULL;
	}

	if (m_pGetMessageByIdTopic)
	{
		delete m_pGetMessageByIdTopic;
		m_pGetMessageByIdTopic = NULL;
	}

	if (m_pGetNextMessageIdTopic)
	{
		delete m_pGetNextMessageIdTopic;
		m_pGetNextMessageIdTopic = NULL;
	}

	if (m_pMAPINicknamesItem)
	{
		delete m_pMAPINicknamesItem;
		m_pMAPINicknamesItem = NULL;
	}

	if (m_pMAPIOriginatorItem)
	{
		delete m_pMAPIOriginatorItem;
		m_pMAPIOriginatorItem = NULL;
	}

	if (m_pMAPIServerVersionItem)
	{
		delete m_pMAPIServerVersionItem;
		m_pMAPIServerVersionItem = NULL;
	}

	if (m_InstId != NULL)
	{
	    ::DdeUninitialize(m_InstId);
		m_InstId = NULL;
	}

	if (m_pCallbackFn != NULL)
	{
#ifndef WIN32
	    FreeProcInstance(FARPROC(m_pCallbackFn));
#endif // WIN32
		m_pCallbackFn = NULL;
	}

	CDDEServer::m_pTheServer = NULL;
}


////////////////////////////////////////////////////////////////////////
// OnConnect [public]
//
// Handler for the XTYP_CONNECT transaction.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEServer::OnConnect(
	HSZ hszTopic,
	HSZ hszService,
	CONVCONTEXT* pContext,
	BOOL isSameInstance)
{
//	TRACE0("CDDEServer::OnConnect\n");
	if (m_pSystemTopic->IsEqual(hszTopic))
		return FALSE;		// FORNOW, let's not handle the System topic
	else if (m_pWWWOpenURLTopic->IsEqual(hszTopic))
		return TRUE;
	else if (m_pMAPITopic->IsEqual(hszTopic))
		return TRUE;
	else if (m_pExpandNicknameTopic->IsEqual(hszTopic))
		return TRUE;
	else if (m_pGetNextMessageIdTopic->IsEqual(hszTopic))
		return TRUE;
	else if (m_pGetMessageByIdTopic->IsEqual(hszTopic))
		return TRUE;
	else if (m_pPutMessageByIdTopic->IsEqual(hszTopic))
		return TRUE;
	else if (m_pDeleteMessageByIdTopic->IsEqual(hszTopic))
		return TRUE;
	return FALSE;
}   


////////////////////////////////////////////////////////////////////////
// OnWildConnect [public]
//
// Handler for the XTYP_WILDCONNECT transaction.
//
////////////////////////////////////////////////////////////////////////
HDDEDATA CDDEServer::OnWildConnect(
	UINT wFmt,
	HSZ hszTopic,
	HSZ hszService,
	CONVCONTEXT* pContext,
	BOOL isSameInstance)
{
//	TRACE0("CDDEServer::OnWildConnect\n");
	if ((hszService != m_pServiceName->GetHandle()) && (hszService != NULL))
	{
		//
		// We only support the advertised Service.
		//
		return NULL;
	}

	//
	// Worst case number of topics, plus one extra for NULL.
	//
	const int MAX_TOPICS = 9;
	HSZPAIR hszpair_array[MAX_TOPICS];
	int idx = 0;

#ifdef FORNOW
	if ((NULL == hszTopic) || m_pSystemTopic->IsEqual(hszTopic))
	{
		ASSERT(idx < MAX_TOPICS);
		hszpair_array[idx].hszSvc = m_pServiceName->GetHandle();
		hszpair_array[idx++].hszTopic = m_pSystemTopic->GetHandle();
	}
#endif // FORNOW
	if ((NULL == hszTopic) || m_pWWWOpenURLTopic->IsEqual(hszTopic))
	{
		ASSERT(idx < MAX_TOPICS);
		hszpair_array[idx].hszSvc = m_pServiceName->GetHandle();
		hszpair_array[idx++].hszTopic = m_pWWWOpenURLTopic->GetHandle();
	}
	if ((NULL == hszTopic) || m_pMAPITopic->IsEqual(hszTopic))
	{
		ASSERT(idx < MAX_TOPICS);
		hszpair_array[idx].hszSvc = m_pServiceName->GetHandle();
		hszpair_array[idx++].hszTopic = m_pMAPITopic->GetHandle();
	}
	if ((NULL == hszTopic) || m_pExpandNicknameTopic->IsEqual(hszTopic))
	{
		ASSERT(idx < MAX_TOPICS);
		hszpair_array[idx].hszSvc = m_pServiceName->GetHandle();
		hszpair_array[idx++].hszTopic = m_pExpandNicknameTopic->GetHandle();
	}
	if ((NULL == hszTopic) || m_pGetNextMessageIdTopic->IsEqual(hszTopic))
	{
		ASSERT(idx < MAX_TOPICS);
		hszpair_array[idx].hszSvc = m_pServiceName->GetHandle();
		hszpair_array[idx++].hszTopic = m_pGetNextMessageIdTopic->GetHandle();
	}
	if ((NULL == hszTopic) || m_pGetMessageByIdTopic->IsEqual(hszTopic))
	{
		ASSERT(idx < MAX_TOPICS);
		hszpair_array[idx].hszSvc = m_pServiceName->GetHandle();
		hszpair_array[idx++].hszTopic = m_pGetMessageByIdTopic->GetHandle();
	}
	if ((NULL == hszTopic) || m_pPutMessageByIdTopic->IsEqual(hszTopic))
	{
		ASSERT(idx < MAX_TOPICS);
		hszpair_array[idx].hszSvc = m_pServiceName->GetHandle();
		hszpair_array[idx++].hszTopic = m_pPutMessageByIdTopic->GetHandle();
	}
	if ((NULL == hszTopic) || m_pDeleteMessageByIdTopic->IsEqual(hszTopic))
	{
		ASSERT(idx < MAX_TOPICS);
		hszpair_array[idx].hszSvc = m_pServiceName->GetHandle();
		hszpair_array[idx++].hszTopic = m_pDeleteMessageByIdTopic->GetHandle();
	}

	ASSERT(idx < MAX_TOPICS);
	hszpair_array[idx].hszSvc = NULL;
	hszpair_array[idx++].hszTopic  = NULL; 

	return DdeCreateDataHandle(m_InstId, LPBYTE(hszpair_array), sizeof(HSZPAIR) * idx, 0L, 0, wFmt, 0);
}



////////////////////////////////////////////////////////////////////////
// OnConnectConfirm [public]
//
// Handler for the XTYP_CONNECT_CONFIRM transaction.  Provides server
// with the conversion handle for the established conversation.
//
////////////////////////////////////////////////////////////////////////
void CDDEServer::OnConnectConfirm(
	HCONV hConv,
	HSZ hszTopic,
	HSZ hszService,
	BOOL isSameInstance)
{
//	TRACE0("CDDEServer::OnConnectConfirm\n");
#ifdef FORNOW
	CDDEServerConv* p_conv;

	if (m_pSystemTopic->IsEqual(hszTopic))
		p_conv = new CSystemConv;
	else if (m_pMAPITopic->IsEqual(hszTopic))
		p_conv = new CMAPIConv;
	else if (m_pExpandNicknameTopic->IsEqual(hszTopic))
		p_conv = new CExpandNicknameConv;
	else if (m_pGetNextMessageIdTopic->IsEqual(hszTopic))
		p_conv = new CGetNextMessageIdConv;
	else if (m_pGetMessageByIdTopic->IsEqual(hszTopic))
		p_conv = new CGetMessageByIdConv;
	else if (m_pPutMessageByIdTopic->IsEqual(hszTopic))
		p_conv = new CPutMessageByIdConv;
	else if (m_pDeleteMessageByIdTopic->IsEqual(hszTopic))
		p_conv = new CDeleteMessageByIdConv;

	if (p_conv != NULL)
		p_conv->Create(hConv, hszTopic, hszService, isSameInstance);
#endif // FORNOW
}


////////////////////////////////////////////////////////////////////////
// OnRequest [public]
//
// Handler for the XTYP_REQUEST transaction.  This is a generic response
// that is conversation independent.
//
////////////////////////////////////////////////////////////////////////
HDDEDATA CDDEServer::OnRequest(
	UINT wFmt,
	HSZ hszTopic,
	HSZ hszItem)
{
//	TRACE0("CDDEServer::OnRequest\n");

	//
	// We only support the CF_TEXT format.
	//
	if (CF_TEXT == wFmt)
	{
		if (m_pWWWOpenURLTopic->IsEqual(hszTopic))
		{
			CString params(CDDEString(m_InstId, hszItem));		// working copy (modifiable)
//			TRACE1("CDDEServer::OnRequest: got WWW_OpenURL = %s\n", (const char *) params);

			//
			// Break the mailto parameter string into individual
			// parameters that Eudora can use.  The original mailto
			// string is assumed to be of the form:
			//
			//    "mailto:address@domain.com","",4,0,"tempfile","",
			//
			// We are interested in the 1st parameter and the 5th parameter.
			//
			CString mailto;
			CString tempfile;
			for (int argcount = 1; !params.IsEmpty(); argcount++)
			{
				CString arg;
				if (params[0] == '"')
				{
					//
					// Deal with quoted argument by finding closing
					// quote.
					//
					arg = params.Right(params.GetLength() - 1);		// strip leading quote
					int idx = arg.Find('"');
					if (idx != -1)
					{
						arg = arg.Left(idx);

						//
						// Remove arg from front of parameter string.
						//
						ASSERT((arg.GetLength() + 2) <= params.GetLength());
						params = params.Right(params.GetLength() - (arg.GetLength() + 2));

						//
						// Strip comma-separator, if any, from parameter string.
						//
						if ((! params.IsEmpty()) && params[0] == ',')
							params = params.Right(params.GetLength() - 1);
					}
					else
					{
						ASSERT(0);			// malformed input (missing close quote)
						params.Empty();
					}
				}
				else
				{
					//
					// Deal with unquoted argument by finding terminating
					// comma, if any.
					//
					int idx = params.Find(',');
					if (idx != -1)
					{
						arg = params.Left(idx);
						params = params.Right(params.GetLength() - idx - 1);
					}
					else
					{
						arg = params;
						params.Empty();
					}
				}

				switch (argcount)
				{
				case 1:
					mailto = arg;
					break;
				case 5:
					tempfile = arg;
					break;
				default:
					break;
				}
			}

			DWORD retval = 0;				// zero return value indicates failure
			if (tempfile.IsEmpty())
			{
				CString to;
				CString cc;
				CString bcc;
				CString subject;
				CString body;

				if (CURLEdit::ParseMailtoArgs(mailto, to, cc, bcc, subject, body))
				{
					//
					// Create Eudora composition window with as many fields
					// filled as possible, then display it.
					//
					CMainFrame* p_mainframe = (CMainFrame *) AfxGetMainWnd();
					ASSERT(p_mainframe != NULL);
					ASSERT(p_mainframe->IsKindOf(RUNTIME_CLASS(CMainFrame)));

					p_mainframe->NewMessage(
						(char *) ((const char *) to),		// To
						NULL,								// From
						(char *) ((const char *) subject),	// Subject
						(char *) ((const char *) cc),		// Cc
						(char *) ((const char *) bcc),		// Bcc
						NULL,								// Attachments
						(char *) ((const char *) body));	// Body

					retval = 42;			// non-zero value indicates success
				}
				else
				{
//					TRACE1("CDDEServer::OnRequest: WWW_OpenURL dropping unsupported mailto %s\n", (const char *) mailto);
					retval = 0xFFFFFFFF;	// indicates URL not handled
				}
			}
			else
			{
				//
				// We can't handle anything other than a simple
				// mailto, so tell Netscape we punted.
				//
//				TRACE2("CDDEServer::OnRequest: WWW_OpenURL dropping file-based mailto %s (%s)\n", (const char *) mailto, (const char *) tempfile);
				retval = 0xFFFFFFFF;		// indicates URL not handled
			}

			HDDEDATA h_data = DdeCreateDataHandle(m_InstId, LPBYTE(&retval), sizeof(DWORD), 0, hszItem, CF_TEXT, 0);
			return h_data;
		}
		else if (m_pMAPITopic->IsEqual(hszTopic))
		{
			//
			// Determine which MAPI Item we're dealing with and process
			// any recognized request.
			//
//			TRACE0("CDDEServer::OnRequest, checking MAPI/Originator...\n");
			if (m_pMAPIOriginatorItem->IsEqual(hszItem))
			{
//				TRACE0("CDDEServer::OnRequest, got MAPI/Originator!\n");
				CString originator(GetReturnAddress());
				if (originator.IsEmpty())
					return NULL;
				return DdeCreateDataHandle(m_InstId, LPBYTE((const char *) originator), originator.GetLength() + 1, 0, hszItem, CF_TEXT, 0);
			}

//			TRACE0("CDDEServer::OnRequest, checking MAPI/Nicknames...\n");
			if (m_pMAPINicknamesItem->IsEqual(hszItem))
			{
//				TRACE0("CDDEServer::OnRequest, got MAPI/Nicknames!\n");
				CString nicknames;
//FORNOW				if (! LoadNicknames())
//FORNOW					return NULL;
				ASSERT(g_Nicknames != NULL);
				if (! g_Nicknames->GetNicknames(nicknames))
					return NULL;
				else if (nicknames.IsEmpty())
					return NULL;

				return DdeCreateDataHandle(m_InstId, LPBYTE((const char *) nicknames), nicknames.GetLength() + 1, 0, hszItem, CF_TEXT, 0);
			}

//			TRACE0("CDDEServer::OnRequest, checking MAPI/ServerVersion...\n");
			if (m_pMAPIServerVersionItem->IsEqual(hszItem))
			{
				//
				// The version string consists of at least three comma-separated 
				// fields as follows:
				//
				//    "Human-readable Comment",MajorVersion,MinorVersion,ExeType
				//
				// For example:
				//
				//    "Eudora Pro MAPI Server, Version 4.0",4,0,32
				//
				// Note that the double-quotes are needed to allow embedded 
				// commas in strings.
				//
				// Change History
				// ==============
				//    DDE Server Version 3.0:
				//        Eudora Pro 3.0, released October 1996.
				//    DDE Server Version 4.0: 
				//        Eudora Pro 3.0.1, to be released January 1997.
				//        This requires a major version change since the
				//        server now generates two new data buffer keywords,
				//        "UNRD" for unread messages, and "RCPT" for messages
				//        which are marked as needing a return receipt.
				//
//				TRACE0("CDDEServer::OnRequest, got MAPI/ServerVersion!\n");
#ifdef WIN32
				CString version("\"Eudora Pro MAPI Server, Version 4.0 (32)\",4,0,32");		// FORNOW, hardcoded
#else
				CString version("\"Eudora Pro MAPI Server, Version 4.0 (16)\",4,0,16");		// FORNOW, hardcoded
#endif //WIN32
				return DdeCreateDataHandle(m_InstId, LPBYTE((const char *) version), version.GetLength() + 1, 0, hszItem, CF_TEXT, 0);
			}
		}
		else if (m_pExpandNicknameTopic->IsEqual(hszTopic))
		{
//			TRACE0("CDDEServer::OnRequest, got ExpandNickname/<Nickname>!\n");
			char* p_expanded = ExpandAliases((const char *) CString(CDDEString(m_InstId, hszItem)));
			if (NULL == p_expanded)
				return NULL;
			HDDEDATA h_data = DdeCreateDataHandle(m_InstId, LPBYTE(p_expanded), strlen(p_expanded) + 1, 0, hszItem, CF_TEXT, 0);
			delete p_expanded;
			return h_data;
		}
		else if (m_pGetNextMessageIdTopic->IsEqual(hszTopic))
		{

			//
			// Parse parameter string of the form:
			//
			//   dwMessageId,dwSortByDate,dwUnreadOnly
			//
			// containing 32-bit hex numbers into a message id value
			// and two boolean flags.
			//
			CString params(CDDEString(m_InstId, hszItem));
//			TRACE1("CDDEServer::OnRequest, got GetNextMessageId(%s)\n", (const char *) params);

			unsigned long message_id = 0;
			if ((! ProcessLongParam(params, message_id)) || (0 == message_id))
				return NULL;		// malformed number (zero message ID indicates caller blew it)

			unsigned long ulong = 0;
			BOOL sort_by_date = FALSE;
			BOOL unread_only = FALSE;

			if (! ProcessLongParam(params, ulong))
				return NULL;		// malformed number
			if (ulong)
				sort_by_date = TRUE;

			if (! ProcessLongParam(params, ulong))
				return NULL;		// malformed number
			if (ulong)
				unread_only = TRUE;

			ASSERT(params.IsEmpty());

			//
			// Get a pointer to the Eudora Inbox object, then let it
			// do all the work.
			//
			CTocDoc* p_inbox = ::GetInToc();
			if (NULL == p_inbox)
			{
				ASSERT(0);
				return NULL;
			}
			if (! p_inbox->GetNextMessageId(message_id, sort_by_date, unread_only))
				return NULL;

			if (0 == message_id)
			{
				ASSERT(0);			// should never return "0" as a valid message id
				return NULL;
			}

			//
			// Convert integer back to string passed back through DDE.
			//
			CString message_id_str;
			message_id_str.Format("0x%08lX", message_id);
			HDDEDATA h_data = DdeCreateDataHandle(m_InstId, LPBYTE((const char *) message_id_str), message_id_str.GetLength() + 1, 0, hszItem, CF_TEXT, 0);
			return h_data;
		}
		else if (m_pGetMessageByIdTopic->IsEqual(hszTopic))
		{
			//
			// Parse parameter string of the form:
			//
			//   dwMessageId,dwBodyAsFile,dwEnvelopeOnly,dwMarkAsRead,dwWantAttachments
			//
			// containing 32-bit hex numbers into a message id value
			// and four boolean flags.
			//
			CString params(CDDEString(m_InstId, hszItem));
//			TRACE1("CDDEServer::OnRequest, got GetMessageById(%s)\n", (const char *) params);

			unsigned long message_id = 0;
			if ((! ProcessLongParam(params, message_id)) || (0 == message_id))
				return NULL;		// malformed number (zero message ID indicates caller blew it)

			unsigned long ulong = 0;
			BOOL body_as_file = FALSE;
			BOOL envelope_only = FALSE;
			BOOL mark_as_read = FALSE;
			BOOL want_attachments = FALSE;

			if (! ProcessLongParam(params, ulong))
				return NULL;		// malformed number
			if (ulong)
				body_as_file = TRUE;

			if (! ProcessLongParam(params, ulong))
				return NULL;		// malformed number
			if (ulong)
				envelope_only = TRUE;

			if (! ProcessLongParam(params, ulong))
				return NULL;		// malformed number
			if (ulong)
				mark_as_read = TRUE;

			if (! ProcessLongParam(params, ulong))
				return NULL;		// malformed number
			if (ulong)
				want_attachments = TRUE;

			ASSERT(params.IsEmpty());

			//
			// Get a pointer to the Eudora Inbox object, then let it
			// do all the work.
			//
			CTocDoc* p_inbox = ::GetInToc();
			if (NULL == p_inbox)
			{
				ASSERT(0);
				return NULL;
			}
			CString message_data;
			if (! p_inbox->GetMessageById(message_data, message_id, body_as_file, envelope_only, mark_as_read, want_attachments))
				return NULL;

			//
			// Pass the string data back through DDE.
			//
			HDDEDATA h_data = DdeCreateDataHandle(m_InstId, LPBYTE((const char *) message_data), message_data.GetLength() + 1, 0, hszItem, CF_TEXT, 0);
			return h_data;
		}
		else if (m_pPutMessageByIdTopic->IsEqual(hszTopic))
		{
//			TRACE0("CDDEServer::OnRequest, got PutMessageById\n");

			//
			// Parse parameter string of the form:
			//
			//   szMessageData
			//
			// where szMessageData is the encoded message data (which can be
			// arbitrarily large).
			//
			CString message_data(CDDEString(m_InstId, hszItem));
			if (message_data.IsEmpty())
				return NULL;

			//
			// Get a pointer to the Eudora Inbox object, then let it
			// do all the work. 
			//
			CTocDoc* p_inbox = ::GetInToc();
			if (NULL == p_inbox)
			{
				ASSERT(0);
				return NULL;
			}

			// At this point, 'params' contains the message data.
			unsigned long message_id = 0;
			if (! p_inbox->PutMessageById(message_data, message_id))
				return NULL;
			if (0 == message_id)
			{
				ASSERT(0);			// should never return "0" as a valid message id
				return NULL;
			}

			//
			// Convert integer back to string passed back through DDE.
			//
			CString message_id_str;
			message_id_str.Format("0x%08lX", message_id);
			HDDEDATA h_data = DdeCreateDataHandle(m_InstId, LPBYTE((const char *) message_id_str), message_id_str.GetLength() + 1, 0, hszItem, CF_TEXT, 0);
			return h_data;
		}
		else if (m_pDeleteMessageByIdTopic->IsEqual(hszTopic))
		{
			//
			// Parse parameter string of the form:
			//
			//   dwMessageId
			//
			// containing a 32-bit hex number into a message id value.
			//
			CString params(CDDEString(m_InstId, hszItem));
//			TRACE1("CDDEServer::OnRequest, got DeleteMessageById(%s)\n", (const char *) params);

			unsigned long message_id = 0;
			if ((! ProcessLongParam(params, message_id)) || (0 == message_id))
				return NULL;		// malformed number (zero message ID indicates caller blew it)

			ASSERT(params.IsEmpty());

			//
			// Get a pointer to the Eudora Inbox object, then let it
			// do all the work.
			//
			CTocDoc* p_inbox = ::GetInToc();
			if (NULL == p_inbox)
			{
				ASSERT(0);
				return NULL;
			}
			if (! p_inbox->DeleteMessageById(message_id))
				return NULL;

			//
			// We can return any non-NULL value to indicate success.
			//
			HDDEDATA h_data = DdeCreateDataHandle(m_InstId, LPBYTE("1"), strlen("1") + 1, 0, hszItem, CF_TEXT, 0);
			return h_data;
		}
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// OnRegister [public]
//
// Handler for the XTYP_REGISTER transaction.
//
////////////////////////////////////////////////////////////////////////
void CDDEServer::OnRegister(
	HSZ hszBaseServiceName,
	HSZ hszInstanceServiceName)
{
    CString base_service_name(CDDEString(m_InstId, hszBaseServiceName));
	CString instance_service_name(CDDEString(m_InstId, hszInstanceServiceName));

//	TRACE2("CDDEServer::OnRegister: servicename=%s, instname=%s\n",
//		   (const char *) base_service_name,
//		   (const char *) instance_service_name);

	if (base_service_name.CompareNoCase("NETSCAPE") == 0)
	{
		CMainFrame* p_mainframe = (CMainFrame *) AfxGetMainWnd();
		if (p_mainframe)
		{
			//
			// Register this instance of Eudora as the handler for
			// mailto URL's in Netscape.  It is better to do this
			// asynchronously by posting a message rather than
			// doing it here ... otherwise, the DDE thread
			// called from Netscape blocks and times out since
			// Netscape is not able to service the registration
			// request.
			//
			ASSERT(p_mainframe->IsKindOf(RUNTIME_CLASS(CMainFrame)));
			p_mainframe->PostMessage(WM_USER_REGISTER_MAILTO);
		}
	}
}


////////////////////////////////////////////////////////////////////////
// OnUnregister [public]
//
// Handler for the XTYP_REGISTER transaction.
//
////////////////////////////////////////////////////////////////////////
void CDDEServer::OnUnregister(
	HSZ hszBaseServiceName,
	HSZ hszInstanceServiceName)
{
//	TRACE2("CDDEServer::OnUnregister: servicename=%s, instname=%s\n",
//		   (const char *) CString(CDDEString(m_InstId, hszBaseServiceName)),
//		   (const char *) CString(CDDEString(m_InstId, hszInstanceServiceName)));

	//
	// FORNOW, do nothing...
	//
}


////////////////////////////////////////////////////////////////////////
// DDEServerCallback [private, static]
//
////////////////////////////////////////////////////////////////////////
#ifdef WIN32
HDDEDATA CALLBACK CDDEServer::DDEServerCallback(
#else
HDDEDATA _export CALLBACK CDDEServer::DDEServerCallback(
#endif // WIN32
	UINT wType, 
	UINT wFmt, 
	HCONV hConv,
	HSZ hsz1, 
	HSZ hsz2, 
	HDDEDATA hDDEData, 
	DWORD dwData1, 
	DWORD dwData2)
{
//	TRACE0("CDDEServer::DDEServerCallback\n");

	//
	// Look up the One True Server object.
	//
	if (NULL == CDDEServer::m_pTheServer)
	{
		ASSERT(0);
		return NULL;
	}

	//
	// Look up the conversation object for the given conversation handle.
	//
	//FORNOW CDDEServerConv* p_conv = NULL;
	//FORNOW if (hConv && (wType != XTYP_CONNECT_CONFIRM))
	//FORNOW 	p_conv = FromHandle(hConv);
	
	switch (wType)
	{
	//
	// Conversation connections.
	//
	case XTYP_CONNECT:
		//
		// Somebody's trying to connect to us, but make sure we 
		// support the topic.
		//
		return (HDDEDATA) CDDEServer::m_pTheServer->OnConnect(hsz1, hsz2, (CONVCONTEXT *) dwData1, BOOL(dwData2));
	case XTYP_WILDCONNECT:
		//
		// Somebody's trying to enumerate all topics that we support.
		//
		return (HDDEDATA) CDDEServer::m_pTheServer->OnWildConnect(wFmt, hsz1, hsz2, (CONVCONTEXT FAR *) dwData1, BOOL(dwData2));

	//
	// Conversation start/stop.
	//
	case XTYP_CONNECT_CONFIRM:
		//
		// Save the conversation handle for this newly started
		// conversation.
		//
		CDDEServer::m_pTheServer->OnConnectConfirm(hConv, hsz1, hsz2, BOOL(dwData2));
		break;
	case XTYP_DISCONNECT:
		//
		// Destroy the conversation handle for this newly halted
		// conversation.
		//
		//FORNOW ASSERT(p_conv);
		//FORNOW p_conv->OnDisconnect(BOOL(dwData2));
//		TRACE0("CDDEServerCallback, XTYP_DISCONNECT ignored\n");
		break; 

	//
	// Service name registration.
	//
	case XTYP_REGISTER:
		CDDEServer::m_pTheServer->OnRegister(hsz1, hsz2);
		break;
	case XTYP_UNREGISTER:
		CDDEServer::m_pTheServer->OnUnregister(hsz1, hsz2);
		break;

	//
	// Conversation transactions.
	//
	case XTYP_REQUEST:
		//
		// Handle request transaction.
		//
		return (HDDEDATA) CDDEServer::m_pTheServer->OnRequest(wFmt, hsz1, hsz2);

	case XTYP_ADVREQ:
		//FORNOW return (HDDEDATA) pThisConv->OnAdvReq(fmt,hsz1,hsz2,LOWORD(dwData1));
//		TRACE0("CDDEServerCallback, XTYP_ADVREQ ignored\n");
		break;
	case XTYP_POKE:
		//FORNOW return (HDDEDATA) pThisConv->OnPoke(hsz1,hsz2,hData);
//		TRACE0("CDDEServerCallback, XTYP_POKE ignored\n");
		break;
	case XTYP_EXECUTE:
		//FORNOW return (HDDEDATA) pThisConv->OnExecute(hsz1,hData);
//		TRACE0("CDDEServerCallback, XTYP_EXECUTE ignored\n");
		break;
	case XTYP_ADVSTART:
		//FORNOW return (HDDEDATA) pThisConv->OnAdvStart(hsz1,hsz2);
//		TRACE0("CDDEServerCallback, XTYP_ADVSTART ignored\n");
		break;
	case XTYP_ADVSTOP:
		//FORNOW pThisConv->OnAdvStop(hsz1,hsz2);
//		TRACE0("CDDEServerCallback: XTYP_ADVSTOP ignored\n");
		break;
	default:
		//
		// Intentionally drop the rest of the messages on the floor.
		//
		break;
	};

	return NULL; 
}


////////////////////////////////////////////////////////////////////////
// ProcessParameter [private]
//
// Private helper function for processing parameter strings containing
// unsigned double-word values.  It is common to pass comma-separated
// parameter strings as the Item part in XTYP_REQUEST transactions.
//
// The idea is to remove the next parameter from the parameter list,
// then convert it to a long integer, returning TRUE if everything went
// okay.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEServer::ProcessLongParam(CString& paramList, unsigned long& ulongVal) const
{
	if (paramList.IsEmpty())
		return FALSE;			// can't get juice from a turnip

	CString param;

	int idx = -1;
	if ((idx = paramList.Find(',')) != -1)
	{
		//
		// Strip next parameter from the front of the paramList.
		//
		param = paramList.Left(idx);
		paramList = paramList.Right(paramList.GetLength() - idx - 1);
	}
	else
	{
		param = paramList;
		paramList.Empty();
	}

	if (param.IsEmpty())
	{
		ASSERT(0);
		return FALSE;		// FORNOW, not handling empty parameters
	}

	char* p_stop_ch = NULL;
	ulongVal = strtoul(param, &p_stop_ch, 0);
	if ((NULL == p_stop_ch) || (*p_stop_ch != '\0'))
		return FALSE;		// malformed integer

	return TRUE;
}
