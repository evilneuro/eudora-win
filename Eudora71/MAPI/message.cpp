////////////////////////////////////////////////////////////////////////
//
// CMapiMessage
//
// Intelligent C++ object wrapper for dumb C-based MapiMessage structure.
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include <afxwin.h>			// FORNOW, should probably be precompiled header

#ifdef _DEBUG
#include <ctype.h>
#endif // _DEBUG

#include "message.h"
#include "recip.h"
#include "filedesc.h"


////////////////////////////////////////////////////////////////////////
// StripControlChars [static]
//
// Given a string buffer, do an in-place conversion such that any
// "control characters" are stripped from the buffer.
// 
////////////////////////////////////////////////////////////////////////
static void StripControlChars(char* pszOriginal)
{
	ASSERT(pszOriginal != NULL);

	CString tempstr(pszOriginal);
	const int STRLEN = tempstr.GetLength();

	//
	// Access the raw data buffer for the original string.
	//
	int j = 0;
	for (int i = 0; i < STRLEN; i++)
	{
		BYTE ch = tempstr[i];

		if (ch < 32 && 
			ch != 0x0A &&	// linefeed (ctrl-J)
			ch != 0x0D &&	// carriage return (ctrl-M)
			ch != 0x0C &&	// form feed (ctrl-L)
			ch != 0x09)		// horizontal tab (ctrl-I)
		{
			continue;		// skip "control" character
		}
#ifdef _DEBUG
		if (ch != 0x0D && ch != 0x0a && (!isprint(ch) || iscntrl(ch)))
		{
			CString msg;
			msg.Format("StripControlChars: 0x%02X not stripped\n", ch);
			OutputDebugString(msg);
		}
#endif // _DEBUG
		pszOriginal[j++] = ch;
	}
	pszOriginal[j] = '\0';
}


////////////////////////////////////////////////////////////////////////
// CMapiMessage [private, constructor]
//
////////////////////////////////////////////////////////////////////////
CMapiMessage::CMapiMessage(void)
{
	ulReserved = 0L;
	lpszSubject = NULL;
	lpszNoteText = NULL;
	lpszMessageType = NULL;
	lpszDateReceived = NULL;
	lpszConversationID = NULL;
	flFlags = 0L;
	lpOriginator = NULL;
	nRecipCount = 0L;
	lpRecips = NULL;
	nFileCount = 0L;
	lpFiles = NULL;
}


////////////////////////////////////////////////////////////////////////
// ~CMapiMessage [public, destructor]
//
////////////////////////////////////////////////////////////////////////
CMapiMessage::~CMapiMessage(void)
{
	delete lpszSubject;
	delete lpszNoteText;
	delete lpszMessageType;
	delete lpszDateReceived;
	delete lpszConversationID;
	delete ((CMapiRecipDesc *) lpOriginator);
	delete [] ((CMapiRecipDesc *) lpRecips);
	delete [] ((CMapiFileDesc *) lpFiles);
}

////////////////////////////////////////////////////////////////////////
// SetOrignatorName [public]
//
// Allocates and initializes a new MapiRecipDesc record for the
// 'lpOriginator' field.
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiMessage::SetOriginatorName(const char* pName)
{
	ASSERT(pName != NULL);
	ASSERT(NULL == lpOriginator);

	CMapiRecipDesc* p_new = new CMapiRecipDesc;
	if (NULL == p_new)
		return FALSE;

	if (! p_new->SetName(pName))
	{
		delete p_new;
		return FALSE;
	}

	lpOriginator = p_new;
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// ReadMessageData [public]
//
// Parses a string buffer containing encoded message data and populates
// this object with the equivalent data.
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiMessage::ReadMessageData(const CString& messageData)
{
	ASSERT(NULL == lpOriginator);
	ASSERT(NULL == lpszSubject);
	ASSERT(NULL == lpszNoteText);
	ASSERT(NULL == lpszMessageType);
	ASSERT(NULL == lpszDateReceived);
	ASSERT(NULL == lpszConversationID);
	ASSERT(NULL == lpOriginator);
	ASSERT(0    == nRecipCount);
	ASSERT(NULL == lpRecips);
	ASSERT(0    == nFileCount);
	ASSERT(NULL == lpFiles);
	ASSERT(0    == flFlags);
	ASSERT(! messageData.IsEmpty());

	//
	// Humphf.  Since lpRecips and lpFiles are old-fashioned,
	// dynamically-allocated "C" arrays, we have to know how many
	// items to allocate in each array *before* we populate the
	// arrays.  That means we have to preprocess the data buffer
	// before we can do the real work.  Pity, isn't it?
	//
	const char* pszMessageData = messageData;		// make working copy
	while (*pszMessageData)
	{
		if ('\n' == *pszMessageData)
			break;				// empty line indicates end of data
			
		if ((strncmp(pszMessageData, "TO__: ", 6) == 0) ||
			(strncmp(pszMessageData, "CC__: ", 6) == 0) ||
			(strncmp(pszMessageData, "BCC_: ", 6) == 0))
		{
			nRecipCount++;
		}
		else if (strncmp(pszMessageData, "PATH: ", 6) == 0)
			nFileCount++;

		// skip to start of next line
		char* pszNewline = strchr(pszMessageData, '\n');
		if (pszNewline)
			pszMessageData = pszNewline + 1;
		else
		{
			// should never get here
			ASSERT(0);
			return FALSE;
		}
	}

	//
	// Okay, allocate recipient and file attachment arrays.
	//
	if (nRecipCount)
	{
		//
		// Don't use CMapiSessionMgr::NewMapiRecipDescArray() here
		// since we don't want this array registered with the session
		// manager's list of allocated items.
		//
		lpRecips = new CMapiRecipDesc[nRecipCount];
		if (NULL == lpRecips)
			return FALSE;		// out of memory
	}
	if (nFileCount)
	{
		lpFiles = new CMapiFileDesc[nFileCount];
		if (NULL == lpFiles)
			return FALSE;
	}

	//
	// Okay, let's cruise through the data buffer again ... this time
	// for real.
	//
	pszMessageData = messageData;
	unsigned recip_array_idx = 0;
	unsigned file_array_idx = 0;
	CString body_text;			// temp storage for message body text, if any
	body_text.GetBufferSetLength(messageData.GetLength());		// pre-allocate for performance
	body_text.ReleaseBuffer();
	CString optional_filename;	// temp storage for FILE/PATH pairs
	CString current_line;
	while (*pszMessageData)
	{
		if ('\n' == *pszMessageData)
			break;				// empty line indicates end of data
			
		//
		// Extract "keyword" portion of line, then remove the keyword
		// portion from the current line, leaving only the (possibly
		// empty) data.
		//
		char szKeyword[7];
		strncpy(szKeyword, pszMessageData, 6);
		szKeyword[6] = '\0';

		// grab the data for the current line
		char* pszNewline = strchr(pszMessageData, '\n');
		if (pszNewline)
		{
			const char* pszStart = pszMessageData + 6;	// strip off leading keyword
			if (pszStart <= pszNewline)
			{
				char* pszCurrentLine = current_line.GetBuffer(pszNewline - pszStart + 1);
				if (pszCurrentLine)
				{
					strncpy(pszCurrentLine, pszStart, pszNewline - pszStart);
					pszCurrentLine[pszNewline - pszStart] = '\0';
					current_line.ReleaseBuffer();
				}
				else
				{
					ASSERT(0);
					return FALSE;
				}
				
			}
			else
			{
				ASSERT(0);
				return FALSE;
			}

			pszMessageData = pszNewline + 1;
		}
		else
		{
			// should never get here
			ASSERT(0);
			return FALSE;
		}

		if (strcmp("BODY: ", szKeyword) == 0)
		{
			//
			// Handle multiple BODY lines by inserting newlines
			// between the lines.
			//
			if (body_text.IsEmpty())
			{
				//
				// Even though this is the first assignment, use the
				// concatenation operator (+=) instead of the
				// assignment operator (=) since the assignment
				// operator simply overwrites the internal CString
				// buffer.  For performance reasons, we want to reuse 
				// the existing, pre-allocated buffer.
				//
				body_text += current_line;
			}
			else
				body_text += "\n" + current_line;
		}
		else if (strcmp("SUBJ: ", szKeyword) == 0)
		{
			ASSERT(NULL == lpszSubject);		// there should only be one subject line
			lpszSubject = new char[current_line.GetLength() + 1];
			if (lpszSubject)
				strcpy(lpszSubject, current_line);
			else
				return FALSE;
		}
		else if (strcmp("TO__: ", szKeyword) == 0 || 
				 strcmp("CC__: ", szKeyword) == 0 ||
				 strcmp("BCC_: ", szKeyword) == 0)
		{
			if (recip_array_idx < nRecipCount)
			{
				ASSERT(lpRecips != NULL);
				CMapiRecipDesc* p_recip = ((CMapiRecipDesc *) lpRecips) + recip_array_idx;		// type cast
				if (strcmp("TO__: ", szKeyword) == 0)
					p_recip->SetClass(MAPI_TO);
				else if (strcmp("CC__: ", szKeyword) == 0)
					p_recip->SetClass(MAPI_CC);
				else if (strcmp("BCC__: ", szKeyword) == 0)
					p_recip->SetClass(MAPI_BCC);
				else
				{
					ASSERT(0);
					p_recip->SetClass(MAPI_TO);
				}
				p_recip->SetNameAndAddress(current_line, current_line);
				recip_array_idx++;
			}
			else
			{
				ASSERT(0);		// array overflow
				return FALSE;
			}
		}
		else if (strcmp("FROM: ", szKeyword) == 0)
		{
			ASSERT(NULL == lpOriginator);		// should only be one FROM line
			SetOriginatorName(current_line);
		}
		else if (strcmp("DATE: ", szKeyword) == 0)
		{
			ASSERT(NULL == lpszDateReceived);	// should only be one DATE line
			lpszDateReceived = new char[current_line.GetLength() + 1];
			if (lpszDateReceived)
				strcpy(lpszDateReceived, current_line);
			else
				return FALSE;
		}
		else if (strcmp("UNRD: ", szKeyword) == 0)
		{
			flFlags |= MAPI_UNREAD;
		}
		else if (strcmp("RCPT: ", szKeyword) == 0)
		{
			flFlags |= MAPI_RECEIPT_REQUESTED;
		}
		else if (strcmp("FILE: ", szKeyword) == 0)
		{
			//
			// This optional FILE value is associated with the
			// next required PATH value in the data stream.
			// Therefore, we just save a copy of this value
			// off to the side to be used when the PATH value
			// is processed.
			//
			optional_filename = current_line;
		}
		else if (strcmp("PATH: ", szKeyword) == 0)
		{
			if (file_array_idx < nFileCount)
			{
				ASSERT(lpFiles != NULL);
				CMapiFileDesc* p_filedesc =  ((CMapiFileDesc *) lpFiles) + file_array_idx;		// type cast
				if (! optional_filename.IsEmpty())
					p_filedesc->SetFileName(optional_filename);
				p_filedesc->SetPathName(current_line);
				file_array_idx++;
			}
			else
			{
				ASSERT(0);		// array overflow
				return FALSE;
			}

			//
			// Clear the filename.
			//
			optional_filename.Empty();
		}
		else
		{
			ASSERT(0);
			return FALSE;
		}
	}

	//
	// If we get this far, all that's left to do is to set the
	// body text field.
	//
	if (! body_text.IsEmpty())
	{
		lpszNoteText = new char[body_text.GetLength() + 1];
		if (lpszNoteText)
			strcpy(lpszNoteText, body_text);
		else
			return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// WriteMessageData [public]
//
// Writes the message data from this object into a simple, line-oriented
// textual format that can be easily transmitted via DDE, WM_COPYDATA,
// etc.
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiMessage::WriteMessageData(CString& messageData, BOOL wantAutoSend, BOOL useShortFilenames)
{
	if (lpszSubject)
	{
		CString subj(lpszSubject);

		//
		// Convert any newline characters in the subject string to
		// spaces in order to convert a multi-line subject string
		// into the single-line subject string assumed by Eudora.
		//
		// FORNOW, not implemented.

		//
		// Add subject line to the messageData buffer.
		//
		messageData += "SUBJ: " + subj + "\n";
	}

	//
	// It looks like we need to use lpszName rather than lpszAddress
	// here.  According the MAPI docs, the lpszName parameter is
	// required, while the lpszAddress parameter is optional.
	//
	for (ULONG idx = 0; idx < nRecipCount; idx++)
	{
		ASSERT(lpRecips);

		switch (lpRecips[idx].ulRecipClass)
		{
		case MAPI_TO:
			messageData += "TO__: ";
			break;
		case MAPI_CC:
		case MAPI_ORIG:
			messageData += "CC__: ";
			break;
		case MAPI_BCC:
			messageData += "BCC_: ";
			break;
		}

		CString FullAddress(MakeFullAddress(&lpRecips[idx]));
		ASSERT(FullAddress.Find('\n') == -1);
		messageData += FullAddress;
		messageData += "\n";
	}

	//
	// Give Eudora a clue whether or not to attempt automatically
	// sending the message without user intervention.
	//
	if (wantAutoSend)
	{
		messageData += "AUTO: ";	// tell Eudora to attempt auto-send

extern DWORD g_dwAuthentication;
		if (g_dwAuthentication)
		{
			char buf[16];
			sprintf(buf, "%lu", g_dwAuthentication);
			messageData += buf;
		}
		
		messageData += "\n";
	}

	//
	// Okay, now the important part ...  the list of attachment
	// files.  We also pass the filename part to Eudora since
	// some MAPI clients use a nonsense temp file name in the
	// pathname part, then specify the meaningful, user-oriented
	// file name in the filename part (can you say Excel 5.0c??).
	//
	for (idx = 0; idx < nFileCount; idx++)
	{
		ASSERT(lpFiles);

		//
		// Pass the optional filename part first so that Eudora
		// can associate it with the required pathname part to
		// follow.
		//
		if (lpFiles[idx].lpszFileName &&
		    strlen(lpFiles[idx].lpszFileName))
		{
			ASSERT(strchr(lpFiles[idx].lpszFileName, '\n') == NULL);
			ASSERT(strchr(lpFiles[idx].lpszFileName, ';') == NULL);
			messageData += "FILE: ";
			messageData += lpFiles[idx].lpszFileName;
			messageData += "\n";
		}

		ASSERT(lpFiles[idx].lpszPathName);
		ASSERT(strchr(lpFiles[idx].lpszPathName, '\n') == NULL);
		ASSERT(strchr(lpFiles[idx].lpszPathName, ';') == NULL);
		messageData += "PATH: ";
		if (useShortFilenames)
		{
#ifdef WIN32
			CString shortpath;
			const int PATHLEN = strlen(lpFiles[idx].lpszPathName) + 32;		// slop factor
			char* p_path = shortpath.GetBuffer(PATHLEN);
			if (p_path)
			{
				DWORD dwStatus = ::GetShortPathName(lpFiles[idx].lpszPathName, p_path, PATHLEN);
				shortpath.ReleaseBuffer();
				if (dwStatus && (int(dwStatus) < PATHLEN))
					messageData += shortpath;
				else
				{
					ASSERT(0);
					messageData += lpFiles[idx].lpszPathName;	// oh, well ... we tried
				}
			}
			else
				messageData += lpFiles[idx].lpszPathName;		// oh, well ... we tried
#else
			messageData += lpFiles[idx].lpszPathName;
#endif
		}
		else
		{
#ifndef WIN32
			ASSERT(0);			// under Win 16, all filenames are short, by definition
#endif // !WIN32
			messageData += lpFiles[idx].lpszPathName;
		}
		messageData += "\n";
	}

	if (lpszNoteText)
	{
#ifdef WIN32
		messageData.GetBufferSetLength(messageData.GetLength() + strlen(lpszNoteText) + (8 * (strlen(lpszNoteText) / 60)));
		messageData.ReleaseBuffer();
#else
		if (strlen(lpszNoteText) > 0x7F00)
		{
			ASSERT(0);
			return FALSE;			// message text too big for 16-bit CString
		}
#endif // !WIN32

		//
		// Strip control characters from the Note text.
		//
		CString NoteCopy(lpszNoteText);
		StripControlChars(NoteCopy.GetBuffer(NoteCopy.GetLength()));
		NoteCopy.ReleaseBuffer();

		//
		// Generate one BODY line for each line in the note text.
		//
		char* pszBody = NoteCopy.GetBuffer(NoteCopy.GetLength());
		CString bodyline;
		while (*pszBody)
		{
			char* pszEoln = pszBody + strcspn(pszBody, "\r\n");
			if ('\0' == *pszEoln)
			{
				//
				// No more end of line characters, so this is the
				// last line.
				//
				break;
			}

			//
			// Check for a normal "\r\n" EOLN marker.  If found,
			// then take the "\n" into account.
			//
			if (('\r' == *pszEoln) && 
			    ('\0' != *(pszEoln + 1)) &&
			    ('\n' == *(pszEoln + 1)))
				pszEoln++;

			//
			// If we get here, then we found another newline
			// character in the body text.  Extract the next
			// "line" from the body (without the terminating
			// newline), then add the line (with the terminating
			// newline) to the outgoing buffer.
			//
			{
				char chSaveEoln = *pszEoln;
				*pszEoln = '\0';		// temporarily NULL terminate
				bodyline.Empty();
				bodyline.GetBuffer(6 + strlen(pszBody) + 3);	// "BODY: " + str + SlopForCRLF
				bodyline.ReleaseBuffer();
				bodyline += "BODY: ";	// use concatenation instead of assignment to avoid realloc of internal buffer
				bodyline += pszBody;
				*pszEoln = chSaveEoln;	// undo NULL termination
			}

			//
			// Check for "\n" only termination and convert it to
			// "\r\n" termination required by Eudora edit control.
			//
			if (bodyline[bodyline.GetLength() - 1] != '\r')
				bodyline += "\r";
			bodyline += "\n";
#ifdef WIN32
			// output body line
			messageData += bodyline;
#else
			//
			// Check for 32Kb CString overflow on 16-bit Windows.  Note
			// that we fudge the overflow length a bit to make sure that
			// we have room for the final empty line below.
			//
			if (long(messageData.GetLength()) + long(bodyline.GetLength()) + 1L >= 0x00007FF0)
			{
				ASSERT(0);
				return FALSE;
			}
			else
				messageData += bodyline;
#endif // WIN32

			//
			// Remove the current line from the body text.
			//
			pszBody = pszEoln + 1;
		}

		//
		// Append the last line of the body text to the outgoing buffer.
		//
		CString lastline;
		lastline.Format("BODY: %s\n", pszBody);
#ifdef WIN32
		// output last line
		messageData += lastline;
#else
		//
		// Check for 32Kb CString overflow on 16-bit Windows.  Note that
		// we fudge the overflow length a bit to make sure we have room
		// for the final empty line below.
		//
		if (long(messageData.GetLength()) + long(lastline.GetLength()) + 1L >= 0x00007FF0)
		{
			ASSERT(0);
			return FALSE;
		}
		else
			messageData += lastline;
#endif // WIN32
	}

	//
	// Add a final empty line to signify the end of the data buffer.
	//
	messageData += "\n";
	return TRUE;
}
