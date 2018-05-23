////////////////////////////////////////////////////////////////////////
//
// CDDEClient
// Class instantiated and used by DDE client side.
//
////////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#include "ddestr.h"
#include "ddeclien.h"
#include "wininet.h"


#ifdef _DEBUG
#define OUTPUTDEBUGSTRING(_dbg_Msg_) OutputDebugString(_dbg_Msg_)
#else
#define OUTPUTDEBUGSTRING(_dbg_Msg_)
#endif // _DEBUG



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



////////////////////////////////////////////////////////////////////////
// CDDEClient	[public, constructor]
//
////////////////////////////////////////////////////////////////////////
CDDEClient::CDDEClient(void) :
	m_InstId(NULL)
{
	// FORNOW, nothing to do
}


////////////////////////////////////////////////////////////////////////
// ~CDDEClient	[public, destructor]
//
////////////////////////////////////////////////////////////////////////
CDDEClient::~CDDEClient(void)
{
	DoLogoff_();
}


////////////////////////////////////////////////////////////////////////
// DDECallbackt [private, static]
//
// Callback function for DDE messages.
//
////////////////////////////////////////////////////////////////////////
#ifdef WIN32
HDDEDATA CALLBACK CDDEClient::DDECallback(
#else
HDDEDATA _export CALLBACK CDDEClient::DDECallback(
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
UNREFERENCED_PARAMETER(wType);
UNREFERENCED_PARAMETER(wFmt);
UNREFERENCED_PARAMETER(hConv);
UNREFERENCED_PARAMETER(hsz1);
UNREFERENCED_PARAMETER(hsz2);
UNREFERENCED_PARAMETER(hDDEData);
UNREFERENCED_PARAMETER(dwData1);
UNREFERENCED_PARAMETER(dwData2);

	OUTPUTDEBUGSTRING("CDDEClient::DDECallback\n");
    return NULL;
}


////////////////////////////////////////////////////////////////////////
// QueryServers [public]
//
// Sample code for listing all topics supported by all available
// DDE servers.
//
////////////////////////////////////////////////////////////////////////
DWORD CDDEClient::QueryServers(
	const char* pServiceName, 	//(i) service name (NULL for wildcard)
	const char* pTopicName)	 	//(i) topic name (NULL for wildcard)
{
	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

	//
	// Open an inline sub-block here to make sure the DDE string
	// objects are destroyed before the logoff occurs.
	//
	HCONVLIST h_convlist = NULL;
	{
		CDDEString service_name(m_InstId, pServiceName);
		CDDEString topic_name(m_InstId, pTopicName);
	
		h_convlist = DdeConnectList(m_InstId,
									service_name.GetHandle(),
									topic_name.GetHandle(),
									NULL,
									NULL);
	}
	
	if (NULL == h_convlist)
	{
		DoLogoff_();
		return 0L;
	}

	//
	// Walk the list of conversation handles...
	//
	DWORD count = 0;
	HCONV h_conv = NULL;
	CONVINFO convinfo;		// holds conversation data
	convinfo.cb = sizeof(CONVINFO);
	convinfo.hszSvcPartner = NULL;
	convinfo.hszTopic = NULL;

	OUTPUTDEBUGSTRING("--- begin conversation list ---\n");
	while ((h_conv = DdeQueryNextServer(h_convlist, h_conv)) != NULL)
	{
		DdeQueryConvInfo(h_conv, QID_SYNC, &convinfo);
		CDDEString service(m_InstId, convinfo.hszSvcPartner);
		CDDEString topic(m_InstId, convinfo.hszTopic);

#ifdef _DEBUG
		CString msg;
		msg.Format("Service='%s', Topic='%s'\n", (const char*) CString(service), (const char*) CString(topic));
#ifdef FORNOW
		msg = "Service='";		//FORNOW
		msg += service;			//FORNOW
		msg += "', Topic='";	//FORNOW
		msg += topic;			//FORNOW
		msg += "'\n";			//FORNOW
#endif // FORNOW
		OUTPUTDEBUGSTRING(msg);
#endif // _DEBUG

		convinfo.hszSvcPartner = NULL;
		convinfo.hszTopic = NULL;
		count++;
	}
	OUTPUTDEBUGSTRING("--- end   conversation list ---\n");

	//
	// Cleanup the conversations.
	//
	DdeDisconnectList(h_convlist);

	DoLogoff_();
	return count;
}


////////////////////////////////////////////////////////////////////////
// RegisterNetscapeProtocol [public]
//
// Sends a WWW_RegisterProtocol or a WWW_UnRegisterProtocol
// request to a running instance of Netscape, 
// which is supposed to configure Netscape to delegate handling of
// the registered protocol back to Eudora.
//
// See the Netscape DDE documentation at
// http://www.netscape.com/newsref/std/ddeapi.html for details on the
// command formats below.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::RegisterNetscapeProtocol(const char* pProtocol, BOOL isRegister /*=TRUE*/)
{
	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

	//
	// Initiate a conversation for the "register protocol" topic.
	// Open a new, inline block here to guarantee that the DDE String objects
	// destroy themselves before we do a DDE logoff.
	//
	HCONV h_conv = NULL;
	{
		CDDEString service(m_InstId, "NETSCAPE");
		char* p_topic = NULL;
		if (isRegister)
			p_topic = "WWW_RegisterProtocol";
		else
			p_topic = "WWW_UnRegisterProtocol";
		CDDEString topic(m_InstId, p_topic);
	
		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);
	}

	if (! h_conv)
	{
		DoLogoff_();
		return FALSE;
	}

	//
	// Create a data handle for the execute string.
	//
	BOOL status = TRUE;		// innocent until proven guilty
	{
		//
		// We always want to instruct Netscape to delegate the
		// given protocol to Eudora, so we must format the item
		// string as follows:
		//
		// EUDORA,PROTOCOL
		//
		CString protocol("EUDORA,");
		protocol += pProtocol;
		CDDEString dde_protocol(m_InstId, protocol);

		//
		// Send the Register Protocol request.
		//
		HDDEDATA h_ddedata = DdeClientTransaction(NULL,
												  (DWORD)-1,
												  h_conv,
												  dde_protocol.GetHandle(),
												  CF_TEXT,
												  XTYP_REQUEST,
												  5000, // ms timeout
												  NULL);

		if (FALSE == h_ddedata)
			status = FALSE;
		else
			DdeFreeDataHandle(h_ddedata);

		//
		// Done with the first conversation now.
		//
		DdeDisconnect(h_conv);
		h_conv = NULL;
	}

	//
	// Done with DDEML.
	//
	DoLogoff_();
	return status;
}
  

////////////////////////////////////////////////////////////////////////
// OpenNetscapeURL [public]
//
// Sends a URL to a running instance of Netscape, causing Netscape to
// open the URL and restore itself to the foreground.
//
// See the Netscape DDE documentation at
// http://www.netscape.com/newsref/std/ddeapi.html for details on the
// command formats below.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::OpenNetscapeURL(const char* pURL)
{
	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

	//
	// Initiate a conversation for the "open URL" topic.  Open a new,
	// inline block here to guarantee that the DDE String objects
	// destroy themselves before we do a DDE logoff.
	//
	HCONV h_conv = NULL;
	{
		CDDEString service(m_InstId, "NETSCAPE");
		CDDEString topic(m_InstId, "WWW_OpenURL");
	
		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);
	}

	if (! h_conv)
	{
		DoLogoff_();
		return FALSE;
	}

	//
	// Create a data handle for the execute string.
	//
	BOOL status = TRUE;		// innocent until proven guilty
	{
		//
		// We always want to instruct Netscape to use an existing
		// window, so we must format the item string as follows:
		//
		// URL,,0xFFFFFFFF
		//
		//CString url(pURL);

		//Need to escape the commas in the URL bcos we need to concatenate comma
		//separated parameters to the DDE call and Netscape's DDE implementation 
		//chokes on commas in the URL (considers it to be arg delimiters)
		char url[INTERNET_MAX_URL_LENGTH + 30];
		const char *commaPtr;
		const char *last = pURL;

		url[0] = '\0';
		while (last && (commaPtr = strchr(last, ',')))
		{
			strncat((char *)url, last, commaPtr - last);
			strcat((char *)url, "%2C");
			last = commaPtr + 1;
		}

		strncat((char *)url, last, strlen(pURL) - (last - pURL));

		//url += ",,0xFFFFFFFF";
		strcat(url, ",,0xFFFFFFFF");
		CDDEString open_url_request(m_InstId, url);

		//
		// Send the Open URL request.
		//
		HDDEDATA h_ddedata = DdeClientTransaction(NULL,
												  (DWORD)-1,
												  h_conv,
												  open_url_request.GetHandle(),
												  CF_TEXT,
												  XTYP_REQUEST,
												  10000, // ms timeout
												  NULL);

		if (FALSE == h_ddedata)
			status = FALSE;
		else
			DdeFreeDataHandle(h_ddedata);

		//
		// Done with the first conversation now.
		//
		DdeDisconnect(h_conv);
		h_conv = NULL;
	}

	if (status)
	{
		CDDEString service(m_InstId, "NETSCAPE");
		CDDEString topic(m_InstId, "WWW_Activate");
	
		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);

		if (! h_conv)
			status = FALSE;
		else
		{
			//
			// We want to always force Netscape to the foreground,
			// so we send it an "activate" request to activate the
			// last active Netscape window.
			//
			CDDEString activate_request(m_InstId, "0xFFFFFFFF,0x0");
	
			//
			// Send the Activate request.
			//
			HDDEDATA h_ddedata = DdeClientTransaction(NULL,
													  (DWORD)-1,
													  h_conv,
													  activate_request.GetHandle(),
													  CF_TEXT,
													  XTYP_REQUEST,
													  10000, // ms timeout
													  NULL);
			if (FALSE == h_ddedata)
				status = FALSE;
			else
				DdeFreeDataHandle(h_ddedata);

			//
			// Done with the second conversation now.
			//
			DdeDisconnect(h_conv);
			h_conv = NULL;
		}
	}

	//
	// Done with DDEML.
	//
	DoLogoff_();
	return status;
}
  

////////////////////////////////////////////////////////////////////////
// GetEudoraOriginator [public]
//
// Fetch the "originator" address from Eudora and place it in the
// provided originatorName object.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::GetEudoraOriginator(CString& originatorName)
{
	originatorName.Empty();		// good hygiene

	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

	//
	// Initiate a conversation for the "MAPI" topic.  Open a new,
	// inline block here to guarantee that the DDE String objects
	// destroy themselves before we do a DDE logoff.
	//
	HCONV h_conv = NULL;
	{
		CDDEString service(m_InstId, "EUDORA");
		CDDEString topic(m_InstId, "MAPI");
	
		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);
	}

	if (! h_conv)
	{
		DoLogoff_();
		return FALSE;		// Eudora is probably not running
	}

	//
	// Send the Originator request.  Eudora returns the data
	// and we are responsible for deleting the memory.
	//
	BOOL status = TRUE;		// innocent until proven guilty
	{
		//
		// We want the Originator info on the MAPI topic.
		//
		CDDEString item(m_InstId, "Originator");

		HDDEDATA h_ddedata = DdeClientTransaction(NULL,
												  (DWORD)-1,
												  h_conv,
												  item.GetHandle(),
												  CF_TEXT,
												  XTYP_REQUEST,
												  5000, // ms timeout
												  NULL);

		if (FALSE == h_ddedata)
			status = FALSE;
		else
		{
			//
			// Determine the length of the returned string, copy
			// the returned string to the caller's string object,
			// then blow away the returned string.
			//
			DWORD len = DdeGetData(h_ddedata, NULL, 0, 0);
			if (len > 0)
			{
				DdeGetData(h_ddedata, LPBYTE(originatorName.GetBuffer(int(len))), len, 0);
				originatorName.ReleaseBuffer();
			}
			DdeFreeDataHandle(h_ddedata);
		}
	}

	//
	// Done with the conversation now.
	//
	DdeDisconnect(h_conv);

	//
	// Done with DDEML.
	//
	DoLogoff_();
	return status;
}


////////////////////////////////////////////////////////////////////////
// GetEudoraNicknames [public]
//
// Fetch a comma-separated list of nickname names from Eudora and place 
// it in the provided nickNames object.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::GetEudoraNicknames(CString& nickNames)
{
	nickNames.Empty();		// good hygiene

	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

	//
	// Initiate a conversation for the "MAPI" topic.  Open a new,
	// inline block here to guarantee that the DDE String objects
	// destroy themselves before we do a DDE logoff.
	//
	HCONV h_conv = NULL;
	{
		CDDEString service(m_InstId, "EUDORA");
		CDDEString topic(m_InstId, "MAPI");
	
		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);
	}

	if (! h_conv)
	{
		DoLogoff_();
		return FALSE;		// Eudora is probably not running
	}

	//
	// Send the Nicknames request.  Eudora returns the data
	// and we are responsible for deleting the memory.
	//
	BOOL status = TRUE;		// innocent until proven guilty
	{
		//
		// We want the Nicknames info on the MAPI topic.
		//
		CDDEString item(m_InstId, "Nicknames");

		HDDEDATA h_ddedata = DdeClientTransaction(NULL,
												  (DWORD)-1,
												  h_conv,
												  item.GetHandle(),
												  CF_TEXT,
												  XTYP_REQUEST,
												  30000, // ms timeout
												  NULL);

		if (FALSE == h_ddedata)
			status = FALSE;
		else
		{
			//
			// Determine the length of the returned string, copy
			// the returned string to the caller's string object,
			// then blow away the returned string.
			//
			DWORD len = DdeGetData(h_ddedata, NULL, 0, 0);
			if (len > 0)
			{
#ifndef WIN32
				//
				// For 16-bit code, forcibly truncate the list at
				// 32Kb.  If we don't do this, then the CString
				// 'nickNames' will overflow and bad things will
				// happen.  Note that the last nickname could get
				// truncated into a bogus nickname.
				//
				if (len >= 0x7FFF)
				{
					ASSERT(0);
					len = 0x7FF0;		// a little less than 32Kb
				}
#endif // !WIN32

				DdeGetData(h_ddedata, LPBYTE(nickNames.GetBuffer(int(len))), len, 0);
				nickNames.ReleaseBuffer();
			}
			DdeFreeDataHandle(h_ddedata);
		}
	}

	//
	// Done with the conversation now.
	//
	DdeDisconnect(h_conv);

	//
	// Done with DDEML.
	//
	DoLogoff_();
	return status;
}


////////////////////////////////////////////////////////////////////////
// ExpandEudoraNickname [public]
//
// Given a Eudora nickname, fetch a comma-separated nickname expansion
// from Eudora.  Return TRUE if successful.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::ExpandEudoraNickname(
	const CString& nickName,	//(i) single Eudora nickname
	CString& expandedNames)		//(o) comma-separated nickname expansion
{
	expandedNames.Empty();		// good hygiene

	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

	//
	// Initiate a conversation for the "ExpandNickname" topic.  Open a new,
	// inline block here to guarantee that the DDE String objects
	// destroy themselves before we do a DDE logoff.
	//
	HCONV h_conv = NULL;
	{
		CDDEString service(m_InstId, "EUDORA");
		CDDEString topic(m_InstId, "ExpandNickname");
	
		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);
	}

	if (! h_conv)
	{
		DoLogoff_();
		return FALSE;		// Eudora is probably not running
	}

	//
	// Send the ExpandNickname request.  Eudora returns the data
	// and we are responsible for deleting the memory.
	//
	BOOL status = TRUE;		// innocent until proven guilty
	{
		CDDEString item(m_InstId, nickName);

		HDDEDATA h_ddedata = DdeClientTransaction(NULL,
												  (DWORD)-1,
												  h_conv,
												  item.GetHandle(),
												  CF_TEXT,
												  XTYP_REQUEST,
												  5000, // ms timeout
												  NULL);

		if (FALSE == h_ddedata)
			status = FALSE;
		else
		{
			//
			// Determine the length of the returned string, copy
			// the returned string to the caller's string object,
			// then blow away the returned string.
			//
			DWORD len = DdeGetData(h_ddedata, NULL, 0, 0);
			if (len > 0)
			{
#ifndef WIN32
				//
				// For 16-bit code, forcibly truncate the list at
				// 32Kb.  If we don't do this, then the CString
				// 'nickNames' will overflow and bad things will
				// happen.  Note that the last nickname could get
				// truncated into a bogus nickname.
				//
				if (len >= 0x7FFF)
				{
					ASSERT(0);
					len = 0x7FF0;		// a little less than 32Kb
				}
#endif // !WIN32

				DdeGetData(h_ddedata, LPBYTE(expandedNames.GetBuffer(int(len))), len, 0);
				expandedNames.ReleaseBuffer();
			}
			DdeFreeDataHandle(h_ddedata);
		}
	}

	//
	// Done with the conversation now.
	//
	DdeDisconnect(h_conv);

	//
	// Done with DDEML.
	//
	DoLogoff_();
	return status;
}


////////////////////////////////////////////////////////////////////////
// GetNextMessageId [public]
//
// Given a Eudora message id, fetch the next message id from Eudora,
// returning it in the caller provided string object.  Return TRUE if
// successful.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::GetNextMessageId(CString& messageId, BOOL sortByDate, BOOL unreadOnly)
{
	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

	//
	// Initiate a conversation for the "GetNextMessageId" topic.  Open
	// a new, inline block here to guarantee that the DDE String
	// objects destroy themselves before we do a DDE logoff.
	//
	HCONV h_conv = NULL;
	{
		CDDEString service(m_InstId, "EUDORA");
		CDDEString topic(m_InstId, "GetNextMessageId");
	
		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);
	}

	if (! h_conv)
	{
		DoLogoff_();
		return FALSE;		// Eudora is probably not running
	}

	//
	// Send the get next message id request.  Eudora returns the data
	// and we are responsible for deleting the memory.
	//
	BOOL status = TRUE;		// innocent until proven guilty
	{
		//
		// Format the request into a string of the form:
		//
		//         dwMessageId,dwSortByDate,dwUnreadOnly
		//
		// where dwMessageId, dwSortByDate, and dwUnreadOnly are
		// formatted as 32-bit hex numbers.
		//
		// A dwMessageId value of 0xFFFFFFFF indicates an that this is
		// the first request.  Otherwise, it should be a valid
		// non-zero Eudora message id.
		//
		// A non-zero dwSortByDate value indicates that the next item
		// it to be returned in ascending order by the message
		// received timestamp.
		//
		// A non-zero dwUnreadOnly value indicates that the next item
		// is to be the next available unread message.
		//
		//
		CString params;

		if (messageId.IsEmpty())
			params = "0xFFFFFFFF";
		else
			params = messageId;
		if (sortByDate)
			params += ",0x1";
		else
			params += ",0x0";
		if (unreadOnly)
			params += ",0x1";
		else
			params += ",0x0";

		CDDEString item(m_InstId, params);

		HDDEDATA h_ddedata = DdeClientTransaction(NULL,
												  (DWORD)-1,
												  h_conv,
												  item.GetHandle(),
												  CF_TEXT,
												  XTYP_REQUEST,
												  5000, // ms timeout
												  NULL);

		if (FALSE == h_ddedata)
			status = FALSE;
		else
		{
			//
			// Determine the length of the returned string, copy
			// the returned string to the caller's string object,
			// then blow away the returned string.
			//
			DWORD len = DdeGetData(h_ddedata, NULL, 0, 0);
			if (len > 0)
			{
				DdeGetData(h_ddedata, LPBYTE(messageId.GetBuffer(int(len))), len, 0);
				messageId.ReleaseBuffer();
			}
			DdeFreeDataHandle(h_ddedata);
		}
	}

	//
	// Done with the conversation now.
	//
	DdeDisconnect(h_conv);

	//
	// Done with DDEML.
	//
	DoLogoff_();
	return status;
}


////////////////////////////////////////////////////////////////////////
// GetMessageById [public]
//
// Given a Eudora message id, fetch the requested message data from
// the Eudora Inbox.  The data is placed in a string in the same
// format used by the WM_COPYDATA data buffer used by MAPISendMail().
// Returns TRUE if successful.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::GetMessageById(
	CString& messageData, 		//(o) returned message data
	const CString& messageId, 	//(i) message id
	BOOL bodyAsFile, 			//(i) return body text as attachment
	BOOL envelopeOnly,			//(i) just return header info, minus body text and attachments
	BOOL markAsRead,			//(i) mark message as being read
	BOOL wantAttachments)		//(i) want the attachments returned
{
	//
	// Check some preconditions.
	//
	if (messageId.IsEmpty())
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

	//
	// Initiate a conversation for the "GetMessageById" topic.  Open
	// a new, inline block here to guarantee that the DDE String
	// objects destroy themselves before we do a DDE logoff.
	//
	HCONV h_conv = NULL;
	{
		CDDEString service(m_InstId, "EUDORA");
		CDDEString topic(m_InstId, "GetMessageById");
	
		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);
	}

	if (! h_conv)
	{
		DoLogoff_();
		return FALSE;		// Eudora is probably not running
	}

	//
	// Send the get message request.  Eudora returns the message data
	// in a string and we are responsible for deleting the memory.
	//
	BOOL status = TRUE;		// innocent until proven guilty
	{
		//
		// Format the request into a string of the form:
		//
		//   dwMessageId,dwBodyAsFile,dwEnvelopeOnly,dwMarkAsRead,dwWantAttachments
		//
		// where all arguments are formatted as 32-bit hex numbers.
		//
		// The dwMessageId value must be a valid, non-zero Eudora
		// message id returned by GetNextMessageId().
		//
		// A non-zero dwBodyAsFile value indicates that the message
		// body text is to be returned as the first file-based
		// attachment.
		//
		// A non-zero dwEnvelopeOnly value indicates that only the
		// message header is returned, minus the body text and the
		// attachments.
		//
		// A non-zero dwMarkAsRead value indicates that unread
		// messages, if successfully read, is to be marked as read in
		// the Eudora Inbox.
		//
		// A non-zero dwWantAttachments value indicates that the
		// attachments for the message are to be returned as temporary
		// files.
		//
		CString params(messageId);		// start with messageId

		if (bodyAsFile)
			params += ",0x1";
		else
			params += ",0x0";
		if (envelopeOnly)
			params += ",0x1";
		else
			params += ",0x0";
		if (markAsRead)
			params += ",0x1";
		else
			params += ",0x0";
		if (wantAttachments)
			params += ",0x1";
		else
			params += ",0x0";

		CDDEString item(m_InstId, params);

		HDDEDATA h_ddedata = DdeClientTransaction(NULL,
												  (DWORD)-1,
												  h_conv,
												  item.GetHandle(),
												  CF_TEXT,
												  XTYP_REQUEST,
												  10000, // ms timeout
												  NULL);

		if (FALSE == h_ddedata)
			status = FALSE;
		else
		{
			//
			// Determine the length of the returned string, copy
			// the returned string to the caller's string object,
			// then blow away the returned string.
			//
			DWORD len = DdeGetData(h_ddedata, NULL, 0, 0);
			if (len > 0)
			{
#ifndef WIN32
				//
				// For 16-bit code, forcibly truncate messages at
				// 32Kb.  If we don't do this, then the CString
				// 'messageData' will overflow and bad things will
				// happen.  Note that since attachment lines are at
				// the end, there could be a loss of attachment files
				// if the truncation occurs.
				//
				if (len >= 0x7FFF)
				{
					ASSERT(0);
					len = 0x7FF0;		// a little less than 32Kb
				}
#endif // !WIN32

				DdeGetData(h_ddedata, LPBYTE(messageData.GetBuffer(int(len))), len, 0);
				messageData.ReleaseBuffer();

#ifndef WIN32
				if (0x7FF0 == len)
				{
					//
					// Since we brutally truncated the data buffer
					// from the Eudora DDE server, we need to make
					// sure we end up with something that it is
					// properly terminated with a blank line.
					//
					int last_newline_idx = messageData.ReverseFind('\n');
					if (-1 == last_newline_idx)
					{
						messageData.Empty();
						status = FALSE;			// corrupted buffer (missing newlines)
					}
					else if (last_newline_idx < (messageData.GetLength() - 1))
					{
						messageData = messageData.Left(last_newline_idx + 1);		// throw away unterminated last line
						messageData += '\n';
					}
				}
#endif // !WIN32

				ASSERT('\n' == messageData[messageData.GetLength() - 1]);
			}
			DdeFreeDataHandle(h_ddedata);
		}
	}

	//
	// Done with the conversation now.
	//
	DdeDisconnect(h_conv);

	//
	// Done with DDEML.
	//
	DoLogoff_();
	return status;
}


////////////////////////////////////////////////////////////////////////
// PutMessageById [public]
//
// Given a valid Eudora message id, overwrite the existing message in
// the Eudora Inbox.  
//
// Otherwise, if we're not given a message id, then append a new
// message to the Eudora Inbox.  In this case, Eudora returns the
// message id for the newly added message.
//
// The message data sent to Eudora is in the same format used by the
// WM_COPYDATA data buffer used by MAPISendMail().
//
// Returns TRUE if successful.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::PutMessageById(
	const CString& messageData, 	//(i) message data to be sent
	CString& messageId) 			//(io) message id
{
	if (messageData.IsEmpty())
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

	//
	// Initiate a conversation for the "PutMessageById" topic.  Open
	// a new, inline block here to guarantee that the DDE String
	// objects destroy themselves before we do a DDE logoff.
	//
	HCONV h_conv = NULL;
	{
		CDDEString service(m_InstId, "EUDORA");
		CDDEString topic(m_InstId, "PutMessageById");

		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);
	}

	if (! h_conv)
	{
		DoLogoff_();
		return FALSE;		// Eudora is probably not running
	}

	//
	// Send the "put message" request.  Eudora returns the message id
	// data in a string and we are responsible for deleting the memory.
	//
	BOOL status = TRUE;		// innocent until proven guilty
	{
		//
		// Format the request into a string of the form:
		//
		//   szMessageData
		//
		// where szMessageData is the encoded message data (which can be
		// an arbitrarily large pile of data).
		//
		CDDEString item(m_InstId, messageData);

		HDDEDATA h_ddedata = DdeClientTransaction(NULL,
												  (DWORD)-1,
												  h_conv,
												  item.GetHandle(),
												  CF_TEXT,
												  XTYP_REQUEST,
												  10000, // ms timeout
												  NULL);

		if (FALSE == h_ddedata)
			status = FALSE;
		else
		{
			//
			// Determine the length of the returned string, copy
			// the returned string to the caller's string object,
			// then blow away the returned string.
			//
			DWORD len = DdeGetData(h_ddedata, NULL, 0, 0);
			if (len > 0)
			{
				DdeGetData(h_ddedata, LPBYTE(messageId.GetBuffer(int(len))), len, 0);
				messageId.ReleaseBuffer();
			}
			DdeFreeDataHandle(h_ddedata);
		}
	}

	//
	// Done with the conversation now.
	//
	DdeDisconnect(h_conv);

	//
	// Done with DDEML.
	//
	DoLogoff_();
	return status;
}


////////////////////////////////////////////////////////////////////////
// DeleteMessageById [public]
//
// Given a Eudora message id, tell Eudora to delete the message from
// the Eudora Inbox.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::DeleteMessageById(const CString& messageId)
{
	//
	// Check some preconditions.
	//
	if (messageId.IsEmpty())
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

	//
	// Initiate a conversation for the "DeleteMessageById" topic.  Open
	// a new, inline block here to guarantee that the DDE String
	// objects destroy themselves before we do a DDE logoff.
	//
	HCONV h_conv = NULL;
	{
		CDDEString service(m_InstId, "EUDORA");
		CDDEString topic(m_InstId, "DeleteMessageById");
	
		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);
	}

	if (! h_conv)
	{
		DoLogoff_();
		return FALSE;		// Eudora is probably not running
	}

	//
	// Send the delete message request.
	//
	BOOL status = TRUE;		// innocent until proven guilty
	{
		CDDEString item(m_InstId, messageId);

		HDDEDATA h_ddedata = DdeClientTransaction(NULL,
												  (DWORD)-1,
												  h_conv,
												  item.GetHandle(),
												  CF_TEXT,
												  XTYP_REQUEST,
												  5000, // ms timeout
												  NULL);

		if (FALSE == h_ddedata)
			status = FALSE;
		else
			DdeFreeDataHandle(h_ddedata);
	}

	//
	// Done with the conversation now.
	//
	DdeDisconnect(h_conv);

	//
	// Done with DDEML.
	//
	DoLogoff_();
	return status;
}


////////////////////////////////////////////////////////////////////////
// GetEudoraMAPIServerVersion [public]
//
// Fetch the MAPI DDE server version info, returning TRUE if the given
// data string was successfully populated.
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::GetEudoraMAPIServerVersion(CString& versionInfo)
{
	//
	// Check some preconditions.
	//
	ASSERT(versionInfo.IsEmpty());

	//
	// Initialize DDEML.
	//
	if (! DoLogon_())
		return FALSE;

#ifdef WIN32
	//
	// Wait a little bit between the initialization of DDE and the
	// first connection.  This turns out to be necessary when you are
	// starting up a copy of Eudora.exe as a result of a MAPILogon
	// call.
	//
	Sleep(1000);
#endif

	//
	// Initiate a conversation for the "GetMAPIServerVersion" topic.  Open
	// a new, inline block here to guarantee that the DDE String
	// objects destroy themselves before we do a DDE logoff.
	//
	HCONV h_conv = NULL;
	{
		CDDEString service(m_InstId, "EUDORA");
		CDDEString topic(m_InstId, "MAPI");
	
		h_conv = DdeConnect(m_InstId,
						    service.GetHandle(),
							topic.GetHandle(),
							NULL);
	}

	if (! h_conv)
	{
		DoLogoff_();
		return FALSE;		// Eudora is probably not running
	}

	//
	// Send the version info request.
	//
	BOOL status = TRUE;		// innocent until proven guilty
	{
		//
		// We want the ServerVersion info on the MAPI topic.
		//
		CDDEString item(m_InstId, "ServerVersion");

		HDDEDATA h_ddedata = DdeClientTransaction(NULL,
												  (DWORD)-1,
												  h_conv,
												  item.GetHandle(),
												  CF_TEXT,
												  XTYP_REQUEST,
												  30000, // ms timeout
												  NULL);

		if (FALSE == h_ddedata)
			status = FALSE;
		else
		{
			//
			// Determine the length of the returned string, copy
			// the returned string to the caller's string object,
			// then blow away the returned string.
			//
			DWORD len = DdeGetData(h_ddedata, NULL, 0, 0);
			if (len > 0)
			{
				DdeGetData(h_ddedata, LPBYTE(versionInfo.GetBuffer(int(len))), len, 0);
				versionInfo.ReleaseBuffer();
			}
			DdeFreeDataHandle(h_ddedata);
		}
	}

	//
	// Done with the conversation now.
	//
	DdeDisconnect(h_conv);

	//
	// Done with DDEML.
	//
	DoLogoff_();
	return status;
}


////////////////////////////////////////////////////////////////////////
// DoLogon_ [private]
//
// Initialize access to the DDEML services, saving the session instance
// handle in m_InstId.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEClient::DoLogon_(void)
{
	if (m_InstId != NULL)
	{
		ASSERT(0);
		return FALSE;
	}

	if (DMLERR_NO_ERROR == DdeInitialize(&m_InstId,
					   					 CDDEClient::DDECallback,
										 APPCMD_CLIENTONLY,
										 0L))
	{
		ASSERT(m_InstId != NULL);
		return TRUE;
	}
	else
	{
		m_InstId = NULL;
		return FALSE;
	}
}


////////////////////////////////////////////////////////////////////////
// DoLogoff_ [private]
//
// Shutdown access to the DDEML services.
//
////////////////////////////////////////////////////////////////////////
void CDDEClient::DoLogoff_(void)
{
	if (m_InstId != NULL)
	{
		DdeUninitialize(m_InstId);
		m_InstId = NULL;
	}
}
