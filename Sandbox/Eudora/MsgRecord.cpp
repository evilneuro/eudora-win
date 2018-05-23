#include "stdafx.h"
#include "afxmt.h"
#include "resource.h"


#include "Callbacks.h"
#include "QCUtils.h"    //for HashMT
#include "MsgRecord.h"
#include "POPSession.h"         


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


static ULONG PersonaHash(const char* pszHashStr, CString strPersona)
{
	// cat the Persona to the end of the hash string
	CString csHashStr(pszHashStr);
	csHashStr += strPersona;
	return ::HashMT(csHashStr);
}


static int hhh=0;
//	===================================================================================
//	===================================================================================
//	===================================================================================
//	===================================================================================
CMsgRecord::CMsgRecord()
{
	SetUIDLID("udef");
	SetMesgID("udef");
	SetDate("udef");
	
	m_nMsgNum = 0;			//SetMsgNum(0);
	m_ulHashValue = 0;		//SetHashValue(0);
	m_ulEstimatedSize = 0;  //SetHashValue(0);

	SetDeleteFlag(LMOS_DONOT_DELETE);	// del  = 0  Ndel  = 1  delS  = -1
	SetSkippedFlag(LMOS_DONOT_SKIP);	// skip = 0  Nskip = 1  skipP = -l
	//SetSaveFlag(1);		// save = 0  Nsave = 1
	SetSaveFlag(LMOS_DONOT_SAVE);		// save = 0  Nsave = 1
	
	//SetReadFlag(1);		// read = 0  Nread = 1  readP = -1
	SetReadFlag(LMOS_NOT_READ);		// read = 0  Nread = 1  readP = -1

	//SetRetrieveFlag(0);	// get  = 0  Nget  = 1
	SetRetrieveFlag(LMOS_RETRIEVE_MESSAGE);	// get  = 0  Nget  = 1

}



//Constructor for Thread-safe data from LMOS fixed records
CMsgRecord::CMsgRecord(unsigned long nMsgNum,	
					   const char* uidlString,
					   const char* msgIdString, 
					   unsigned long ulDate,	//const char* dateString,  
					   unsigned long ulFlags, 
					   unsigned long ulHash,
					   unsigned long lEstSize)
{
	//TRACE("Creating new MsgRecord from file %d\n", ++hhh);
	
	//Nullify all the char buffers
	memset(m_szUIDLID, 0, LMOS_UIDL_SIZE +1);
	memset(m_szMesgID, 0, LMOS_MSGID_SIZE+1);
	
	//memset(m_szDate,   0, LMOS_DATE_SIZE +1);
	
	
	SetUIDLID(uidlString);
	SetMesgID(msgIdString);
	//SetDate(dateString);
	m_ulDate = ulDate;
	
	m_nMsgNum = nMsgNum;		//SetMsgNum(nMsgNum);
	m_ulHashValue = ulHash;		//SetHashValue(ulHash);
	m_ulEstimatedSize = lEstSize;	//SetEstimatedSize(lEstSize);

	//Delete Flag
	if(ulFlags & LMOS_DONOT_DELETE) 
		m_sDeleteFlag = LMOS_DONOT_DELETE;
	
	if(ulFlags & LMOS_DELETE_MESSAGE) 
		m_sDeleteFlag = LMOS_DELETE_MESSAGE;
	
	if(ulFlags & LMOS_DELETE_SENT) 
		m_sDeleteFlag = LMOS_DELETE_SENT;

	//Read Flag
	if(ulFlags & LMOS_HASBEEN_READ) 
		m_sReadFlag = LMOS_HASBEEN_READ;
	
	if(ulFlags & LMOS_NOT_READ) 
		m_sReadFlag = LMOS_NOT_READ;

	if(ulFlags & LMOS_READ_PROGRAMMATICALLY) 
		m_sReadFlag = LMOS_READ_PROGRAMMATICALLY;

	//Retrieve Flag
	if(ulFlags & LMOS_RETRIEVE_MESSAGE) 
		m_sRetrieveFlag = LMOS_RETRIEVE_MESSAGE;

	if(ulFlags & LMOS_DONOT_RETRIEVE) 
		m_sRetrieveFlag = LMOS_DONOT_RETRIEVE;

	//Skip Flag
	if(ulFlags & LMOS_SKIP_MESSAGE) 
		m_sSkippedFlag = LMOS_SKIP_MESSAGE;

	if(ulFlags & LMOS_DONOT_SKIP) 
		m_sSkippedFlag = LMOS_DONOT_SKIP;

	if(ulFlags & LMOS_SKIP_WHAT) 
		m_sSkippedFlag = LMOS_SKIP_WHAT;

	//Save Flag
	if(ulFlags & LMOS_SAVE_MESSAGE) 
		m_sSaveFlag = LMOS_SAVE_MESSAGE;

	if(ulFlags & LMOS_DONOT_SAVE) 
		m_sSaveFlag = LMOS_DONOT_SAVE;
	
}




CMsgRecord::CMsgRecord
(
	const char* msgNumString,
	const char* uidlString,
	const char* msgIdString,
	const char* dateString,
	const char* deleteString,
	const char* skippedString,
	const char* saveString,
	const char* readString,
	const char* retrieveString,
	unsigned long hashVal)
{
	int		cmp = 0;
	BOOL	oldFileVersion = FALSE;

	SetUIDLID(uidlString);
	SetMesgID(msgIdString);
	SetDate(dateString);
	SetMsgNum(msgNumString);
	
	m_ulHashValue = hashVal;	//SetHashValue(hashVal);
	//TRACE("NewRec Serv %d Num %d Hash %d uidl %s\n", ++hhh, m_nMsgNum, m_ulHashValue, uidlString);
	
	
	// We need to determine if this is the old style or new style lmos data..
	// so lets check the delete string to figure it out...
	cmp = strcmp("delete", deleteString);
	if (cmp == 0)
		oldFileVersion = TRUE;
	else 
	{
		// Actually, we still need to check for the noDelete string..just to be sure..
		cmp = strcmp("noDelete", deleteString);
		if (cmp == 0)
			oldFileVersion = TRUE;
	}
 		
	if (oldFileVersion)
	{
		cmp = strcmp("delete",deleteString);
		if (cmp == 0)
			SetDeleteFlag(LMOS_DONOT_DELETE);  //SetDeleteFlag(1);
		else
			SetDeleteFlag(LMOS_DELETE_MESSAGE); //SetDeleteFlag(0);
	 		
		cmp = strcmp("skipped",skippedString);
		if (cmp == 0)
			SetSkippedFlag(LMOS_DONOT_SKIP);	//SetSkippedFlag(1);
		else
			SetSkippedFlag(LMOS_SKIP_MESSAGE);  //SetSkippedFlag(0);

		cmp = strcmp("save",saveString);
		if (cmp == 0)
			SetSaveFlag(LMOS_DONOT_SAVE);	//SetSaveFlag(1);
		else
			SetSaveFlag(LMOS_SAVE_MESSAGE);	//SetSaveFlag(0);

		cmp = strcmp("read",readString);
		if (cmp == 0)
			SetReadFlag(LMOS_NOT_READ);	//SetReadFlag(1);
		else
			SetReadFlag(LMOS_HASBEEN_READ); //SetReadFlag(0); 

		cmp = strcmp("get",retrieveString);
		if (cmp == 0)
			SetRetrieveFlag(LMOS_DONOT_RETRIEVE);	//SetRetrieveFlag(1);
		else
			SetRetrieveFlag(LMOS_RETRIEVE_MESSAGE); //SetRetrieveFlag(0); 		 
	}
	else
	{
		//short delFlag = 0;
		//short skipFlag = 0;
		//short readFlag = 0;

		LMOS_DELETE_FLAG delFlag =	LMOS_DELETE_MESSAGE;
		LMOS_SKIP_FLAG	 skipFlag = LMOS_SKIP_MESSAGE;
		LMOS_READ_FLAG	 readFlag = LMOS_HASBEEN_READ;


		// del=0  Ndel  = 1  delS = -1
		if (strlen(deleteString) == 4)
		{
			if (deleteString[0] == 'N')
				delFlag = LMOS_DONOT_DELETE;  //delFlag = 1;
			else
				delFlag = LMOS_DELETE_SENT; //delFlag = -1;
		}

		// skip=0  Nskip = 1  skipP = -1
		if (strlen(skippedString) == 5)
		{
			if (skippedString[0] == 'N')
				skipFlag = LMOS_DONOT_SKIP;	//skipFlag = 1;
			else
				skipFlag = LMOS_SKIP_WHAT;	//skipFlag = -1;
		}

		// read=0  Nread=1  readP =-1
		if (strlen(readString) == 5)
		{
			if (readString[0] == 'N')
				readFlag = LMOS_NOT_READ;	//readFlag = 1;
			else
				readFlag = LMOS_READ_PROGRAMMATICALLY;	//readFlag = -1;
		}
	
		LMOS_SAVE_FLAG saveFlag = (saveString[0] == 'N')? LMOS_DONOT_SAVE : LMOS_SAVE_MESSAGE;
		LMOS_RETRIEVE_FLAG retrFlag = (retrieveString[0] == 'N')? LMOS_DONOT_RETRIEVE : LMOS_RETRIEVE_MESSAGE;
		
		/*
		SetDeleteFlag(delFlag);
		SetSkippedFlag(skipFlag);
		SetSaveFlag(saveFlag);			//save=0  Nsave=1
		SetReadFlag(readFlag);
		SetRetrieveFlag(retrFlag);	//get=0   Nget=1
		*/
		m_sDeleteFlag = delFlag;
		m_sSkippedFlag = skipFlag;
		m_sSaveFlag = saveFlag;
		m_sReadFlag = readFlag;
		m_sRetrieveFlag = retrFlag;
	}
}


CMsgRecord::~CMsgRecord()
{
}

void CMsgRecord::SetDeleteFlag(LMOS_DELETE_FLAG deleteThis, bool bIgnoreDeleteSent /*=false*/)
{ 
	if( deleteThis == m_sDeleteFlag) //if the same flag. no need to change
		return;

	if( !bIgnoreDeleteSent && (deleteThis != LMOS_DELETE_SENT) )
	{
		//ASSERT(m_sDeleteFlag != LMOS_DELETE_SENT);
		//if delete is already sent, can't be changed to any other state
		if(m_sDeleteFlag == LMOS_DELETE_SENT)
			return;
	}

	InterlockedExchange((long*)&m_sDeleteFlag, deleteThis); 
}


void CMsgRecord::DebugDump(char *buf, int len)
{
	char tbuf[256];
	if( m_sDeleteFlag == LMOS_DELETE_SENT)
		strcpy(tbuf, "DelSent");
	else if( m_sDeleteFlag == LMOS_DELETE_MESSAGE)
		strcpy(tbuf, "DeleteNextTime");
	else
		strcpy(tbuf, "DontDelete");

	_snprintf(buf, len, "%3d %s %s %ld %ld %s", m_nMsgNum, m_szUIDLID, m_szMesgID, m_ulDate, m_ulHashValue, tbuf);
	
	//TRACE("MsgRec: Num %d Hash %ld UIDL %s\n", m_nMsgNum, m_ulHashValue, m_szUIDLID);
}

void CMsgRecord::GetUIDLID(char *theString)
{
	CSingleLock lock(&m_Guard, TRUE);
	m_szUIDLID[0] ? strcpy(theString,m_szUIDLID) : strcpy(theString,"udef");
}

void CMsgRecord::GetMesgID(char *theString)
{
	CSingleLock lock(&m_Guard, TRUE);

	m_szMesgID[0] ? strcpy(theString,m_szMesgID) : strcpy(theString,"udef");
}
	
////////////////////////////////////////////////////////////////////////
// SetUIDLID [public]
//
////////////////////////////////////////////////////////////////////////
void CMsgRecord::SetUIDLID(const char* pszUIDLID)
{
	CSingleLock lock(&m_Guard, TRUE);

	// hack to fix bad LMOS.DAT files - strlen( "X-UIDL: " )== 8
	if (strncmp(pszUIDLID, "X-UIDL: ", 8) == 0)
		pszUIDLID += 8;

	//lstrcpyn(m_szUIDLID, pszUIDLID, sizeof(m_szUIDLID));
	strncpy(m_szUIDLID, pszUIDLID, LMOS_UIDL_SIZE);

	// replace spaces with '-' chars
	//for (int i = 0; ( i < strlen(m_szUIDLID) && m_szUIDLID[i] ); i++ )
	for (int i=0; i < (int)strlen(m_szUIDLID); i++ )
	{
		if (m_szUIDLID[i] == ' ')
			m_szUIDLID[i] = '-';
	}
}


////////////////////////////////////////////////////////////////////////
// SetMesgID [public]
//
////////////////////////////////////////////////////////////////////////
void CMsgRecord::SetMesgID(const char* pszMesgID)
{
	CSingleLock lock(&m_Guard, TRUE);

	char *pszChar;

	//lstrcpyn(m_szMesgID, pszMesgID, sizeof(m_szMesgID));
	strncpy(m_szMesgID, pszMesgID, LMOS_MSGID_SIZE);
	if (pszChar = strchr(m_szMesgID,'\r'))
		*pszChar = '_';
	if (pszChar = strchr(m_szMesgID,'\n'))
		*pszChar = '_';
}


////////////////////////////////////////////////////////////////////////
// SetHashString [public]
//
////////////////////////////////////////////////////////////////////////
void CMsgRecord::SetHashString(const char* pszHashString, CString strPersona)
{
	SetHashValue(::PersonaHash(pszHashString, strPersona));
}


////////////////////////////////////////////////////////////////////////
// GetHashString [public]
//
////////////////////////////////////////////////////////////////////////
void CMsgRecord::GetHashString(char* pszHashString) const
{
	sprintf(pszHashString, "%lu", GetHashValue());
}

const unsigned long JAN_1_2038 = ( 24836UL * 60 * 60 * 24 );
////////////////////////////////////////////////////////////////////////
// SetDate [public]
//
// This function will determine if an unitialized string is passed in
// and get the current local time, convert that time to GMT and then
// convert the time to a string...and save that in the member
// variable...m_szDate....  so all times are relative to GMT !!!
////////////////////////////////////////////////////////////////////////
void CMsgRecord::SetDate(const char* pszDate)
{
	CSingleLock lock(&m_Guard, TRUE);

	//
	// Make sure the given date is in range.
	//
	BOOL bDateUndefined = strncmp(pszDate, "udef", 4) == 0;

	char* pszEndChar = NULL;		// not used
	//const unsigned long JAN_1_2038 = ( 24836UL * 60 * 60 * 24 );
	unsigned long ulDate = strtoul( pszDate, &pszEndChar, 10 );
	if ( ulDate == 0 || ulDate > JAN_1_2038 )
		bDateUndefined = TRUE;

	if (bDateUndefined)
	{
		long lCurrentTime = 0;
		time(&lCurrentTime);
		lCurrentTime -= _timezone;		// apply GMT offset
		//sprintf(m_szDate, "%ld", lCurrentTime); 
		m_ulDate = lCurrentTime;
	}
	else
	{
		//lstrcpyn(m_szDate, pszDate, sizeof(m_szDate));
		//strncpy(m_szDate, pszDate, LMOS_DATE_SIZE);
		m_ulDate = ulDate;
	}
}

void CMsgRecord::GetDate(char *theString) const
{
	if(m_ulDate)
	{
		//strcpy(theString,m_szDate);
		sprintf(theString, "%ld", m_ulDate);
	}
	else
		strcpy(theString,"udef");
		
}

////////////////////////////////////////////////////////////////////////
// GetDateLong [public]
//
////////////////////////////////////////////////////////////////////////
unsigned long CMsgRecord::GetDateLong()
{
	CSingleLock lock(&m_Guard, TRUE);

	//ASSERT(m_szDate[0]);
	//ASSERT(strcmp(m_szDate, "udef") != 0);
	//return atol(m_szDate);
	ASSERT(m_ulDate > 365*27*60*60*24);
	return m_ulDate;
}


////////////////////////////////////////////////////////////////////////
// IsEqualMesgID [public]
//
// Returns TRUE if the given value matches m_szMesgID.
////////////////////////////////////////////////////////////////////////
BOOL CMsgRecord::IsEqualMesgID(const char* pszMesgID) 
{
	CSingleLock lock(&m_Guard, TRUE);

	return strncmp(m_szMesgID, pszMesgID, strlen(m_szMesgID)) == 0;
}


////////////////////////////////////////////////////////////////////////
// IsEqualUIDLID [public]
//
// Returns TRUE if the given value matches m_szUIDLID.
////////////////////////////////////////////////////////////////////////
BOOL CMsgRecord::IsEqualUIDLID(const char* pszUIDLID)
{
	CSingleLock lock(&m_Guard, TRUE);

	return strncmp(m_szUIDLID, pszUIDLID, strlen(m_szUIDLID)) == 0;
}


////////////////////////////////////////////////////////////////////////
// PurgeThisMessage [public]
//
// Each message has the ability to delete itself from the POP
// server...the only assumption that is made is that there is a valid
// POP connection at the time this function is called...
////////////////////////////////////////////////////////////////////////
BOOL CMsgRecord::PurgeThisMessage(CPOPSession *popSession, unsigned long purgeTime)
{
	if (!popSession || GetDeleteFlag() == LMOS_DELETE_SENT /*-1*/)
	{
		//
		// No POP connection or the message has already been deleted.
		//
		return FALSE;
	}

	if (GetRetrieveFlag() == LMOS_RETRIEVE_MESSAGE && GetSkippedFlag() != LMOS_DONOT_SKIP)
	{
		//
		// If the message is marked to be retrieved but has not been
		// fully downloaded, then don't remove it from the server.
		//
		return FALSE;
	}

	//
	// Now decide whether or not the message should be deleted.
	//
	BOOL bDeleteMsg = FALSE;
	if (GetDeleteFlag() == LMOS_DELETE_MESSAGE /*0*/)
		bDeleteMsg = TRUE;
	else if( GetDateLong() < purgeTime)
	{
		//
		// This msg has expired.  But, we only delete it if it was 
		// NOT skipped.
		//
		if (GetSkippedFlag() != LMOS_SKIP_MESSAGE /*0*/)
			bDeleteMsg = TRUE;
	}
	
	if (bDeleteMsg)
	{
		//
		// Send the POP server a DELE command.
		//
		char szArgs[20];
		sprintf(szArgs, "%d", GetMsgNum());
		
		char szBuffer[QC_NETWORK_BUF_SIZE];		// to hold (unused) results from POP server
		if (popSession->POPCmdGetReply(IDS_POP_DELETE, szArgs, szBuffer, sizeof(szBuffer)) >= 0)
		{
			TRACE("Purgeing: Delete sent for msg %d\n", GetMsgNum());
			SetDeleteFlag(LMOS_DELETE_SENT);	//SetDeleteFlag(-1);
			//s_bDeletedMail = TRUE;
		}
		else
		{
			TRACE("Purgeing: Failed to send Delete for msg %d\n", GetMsgNum());
			ASSERT(0);
			return FALSE;
		}
		
		
	}
	
	//return TRUE;
	//return TRUE only when a message is deleted
	return bDeleteMsg;
}


////////////////////////////////////////////////////////////////////////
// WriteLMOSData [public]
//
// Writes out one line worth of LMOS data for this message record.
////////////////////////////////////////////////////////////////////////
bool CMsgRecord::WriteLMOSData(CFile* pLMOSFile)
{
	char szBuffer[LMOS_RECORD_SIZE];
	char* pszBuffer = szBuffer;
	  
	memset(szBuffer, 0, sizeof(szBuffer));

	//
	// Write the fields out in order:
	//
	//    UIDLID MesgID Date [DeleteFlag SkippedFlag SaveFlag ReadFlag RetrieveFlag] Hash EstimatedSize
	//
	GetUIDLID(pszBuffer);
	GetMesgID(pszBuffer + LMOS_MSGID_OFFSET);
	
	//GetDate(pszBuffer+LMOS_DATE_OFFSET);
	//Hash as a long
	memcpy(pszBuffer+LMOS_DATE_OFFSET, &m_ulDate, LMOS_DATE_SIZE);

	//Flags as a long
	unsigned long ulFlags = GetFlags();
	memcpy(pszBuffer+LMOS_FLAGS_OFFSET, &ulFlags, LMOS_FLAGS_SIZE);
	
	//Hash as a long
	memcpy(pszBuffer+LMOS_HASH_OFFSET, &m_ulHashValue, LMOS_HASH_SIZE);

	//Estimated Message size as a long
	memcpy(pszBuffer+LMOS_ESTSIZE_OFFSET, &m_ulEstimatedSize, LMOS_ESTSIZE_SIZE);

	pLMOSFile->Write(szBuffer, LMOS_RECORD_SIZE);
	
	return true;
}





////////////////////////////////////////////////////////////////////////
// CreateMsgRecord [public, static]
//
////////////////////////////////////////////////////////////////////////
CMsgRecord* CMsgRecord::CreateMsgRecord(CString strPersona, int nType, char* pszBuffer, int nBufferLen, int *pnMessageNum /*=NULL*/)
{
	if (nBufferLen <= 0)
	{
		ASSERT(0);
		return NULL;
	}

	
	//
	// Default values for LMOS arguments.
	//
	char szMsgNum[40]		= { '\0' };		// temp storage for globally-assigned message number string
	char szUndefined[]		= "udef";		// "undefined"
	char szNotSkipped[]		= "Nskip";		// "notSkipped" // or "skip"  "skipP"
	char szNoDelete[]		= "Ndel";		// "noDelete"	// or "del" "delS"
	char szNoSave[] 		= "Nsave";		// "noSave"
	char szNotRead[] 		= "Nread";		// "notRead" 	// or "read" "readP"
	char szGet[] 			= "get";		// "getIt" 		// or "Nget"
	unsigned long ulHash	= 0;

	// hack to fix bad LMOS.DAT files - strlen( "X-UIDL: " )== 8
	if (strncmp(pszBuffer, "X-UIDL: ", 8) == 0)
	{
		//
		// Skip over leading "X-UIDL: " tags.
		//
		pszBuffer += 8;
		nBufferLen -= 8;
	}

	//
	// The following will point to various argument strings either in
	// the caller-provided 'pszBuffer' or to the default values in the
	// buffers above.
	//
	char* pszMsgNum			= pszBuffer;	// Message Number
	char* pszUIDL			= NULL;			// UIDL ID
	char* pszMessageID		= NULL;			// Message ID
	char* pszDownloadDate	= NULL;			// Date message read from server
	char* pszDelete			= NULL;			// Delete flag
	char* pszSkipped		= NULL;			// Skipped flag
	char* pszSave			= NULL;			// Save flag
	char* pszHasBeenRead	= NULL;			// Has Been Read flag
	char* pszRetrieve		= NULL;			// Retrieve flag
	char* pszHash			= NULL;			// Hash value

	{
		//
		// Find the end-of-line marker and strip it off.
		//
		char* pszEndOfLine = strchr(pszBuffer, '\r');
		RETURN_NULL_IF_NULL(pszEndOfLine);

		*pszEndOfLine = 0;	// Strip off the CRLF
		pszEndOfLine += 2;  // skip over the old CRLF pair.
	}

	switch (nType)
	{
	case kUIDL:
		//
		// Buffer contains:
		//
		// 1: MsgNum
		// 2: UIDL
		//

		//
		// Seek to UIDL and terminate MsgNum.
		//
		pszUIDL = strchr(pszMsgNum, ' ');
		RETURN_NULL_IF_NULL(pszUIDL);
		*pszUIDL = 0;
		pszUIDL++;

		// 
		// Default the remaining arguments, and compute the Hash value
		// from the UIDL.
		//
		pszMessageID 	= szUndefined;
		pszDownloadDate = szUndefined;
		pszDelete 		= szNoDelete;
		pszSkipped 		= szNotSkipped;
		pszSave 		= szNoSave;
		pszHasBeenRead 	= szNotRead;
		pszRetrieve		= szGet;
		ulHash			= ::PersonaHash(pszUIDL, strPersona);
		break;

	case kFromFile:

		if (strstr(pszMsgNum, "NoMessages") != NULL)
		{
			//
			// Buffer contains "NoMessages", indicating that there is
			// no LMOS message data for this host.
			//
			return NULL;
		}

		//
		// Otherwise, buffer contains:
		//
		// 1: UIDL
		// 2: MessageID
		// 3: DownloadDate
		// 4: Delete
		// 5: Skipped
		// 6: Save
		// 7: HasBeenRead
		// 8: Retrieve
		// 9: Hash
		//

		pszUIDL = pszMsgNum;

		//
		// Create a message number, based on the global current message
		// number.
		//
		//sprintf(szMsgNum, "%d", s_ulMessageIndex);
		//s_ulMessageIndex++;
		sprintf(szMsgNum, "%d", *pnMessageNum);
		(*pnMessageNum)++;
		pszMsgNum = szMsgNum;

		
		//
		// Seek to MessageID and terminate UIDL.
		//
		pszMessageID = strchr(pszUIDL, ' ');
		RETURN_NULL_IF_NULL(pszMessageID);
		*pszMessageID = 0;
		pszMessageID++;

		//
		// Seek to DownloadDate and terminate MessageID.
		//
		pszDownloadDate = strchr(pszMessageID, ' ');
		RETURN_NULL_IF_NULL(pszDownloadDate);
		*pszDownloadDate = 0;
		pszDownloadDate++;

		//
		// Seek to Delete and terminate DownloadDate.
		//
		pszDelete = strchr(pszDownloadDate, ' ');
		RETURN_NULL_IF_NULL(pszDelete);
		*pszDelete = 0;
		pszDelete++;

		//
		// Seek to Skipped and terminate Delete.
		//
		pszSkipped = strchr(pszDelete, ' ');
		RETURN_NULL_IF_NULL(pszSkipped);
		*pszSkipped = 0;
		pszSkipped++;

		//
		// Seek to Save and terminate Skipped.
		//
		pszSave = strchr(pszSkipped, ' ');
		RETURN_NULL_IF_NULL(pszSave);
		*pszSave = 0;
		pszSave++;

		//
		// Seek to HasBeenRead and terminate Save.
		//
		pszHasBeenRead = strchr(pszSave, ' ');
		RETURN_NULL_IF_NULL(pszHasBeenRead);
		*pszHasBeenRead = 0;
		pszHasBeenRead++;

		//
		// Seek to Retrieve and terminate HasBeenRead.
		//
		pszRetrieve = strchr(pszHasBeenRead, ' ');
		RETURN_NULL_IF_NULL(pszRetrieve);
		*pszRetrieve = 0;
		pszRetrieve++;

		//
		// Seek to Hash and terminate Retrieve.
		//
		pszHash = strchr(pszRetrieve, ' ');
		RETURN_NULL_IF_NULL(pszHash);
		*pszHash = 0;
		pszHash++;

		if (strcmp(pszHash, "noHash") == 0)
		{
			//
			// The Hash has no value in it, so we need to compute a new
			// Hash value.
			//
			ulHash	= ::PersonaHash(pszUIDL, strPersona);
	    }
		else
		{
			//
			// Use the existing Hash value.
			//
			ulHash = (unsigned long) atol(pszHash);
		}
		break;
	
		

	case kNonUIDL:
		//
		// Buffer contains:
		//
		// 1: MsgNum
		// 2: MessageID
		// 3: Hash
		//

		//
		// Seek to MessageID and terminate MsgNum.
		//
		pszMessageID = strchr(pszMsgNum, ' ');
		RETURN_NULL_IF_NULL(pszMessageID);
		*pszMessageID = 0;
		pszMessageID++;
		
		//
		// Seek to Hash and terminate MessageID.
		//
		pszHash = strchr(pszMessageID, ' ');
		RETURN_NULL_IF_NULL(pszHash);
		*pszHash = 0;
		pszHash++;

		// 
		// Default the remaining arguments, and compute the hash
		// value from the Hash.  FORNOW, computing a hash from a hash
		// sounds kind of fishy doesn't it?
		//
		pszUIDL			= szUndefined;
		pszDownloadDate = szUndefined;                
		pszDelete 		= szNoDelete;          
		pszSkipped 		= szNotSkipped;
		pszSave 		= szNoSave;
		pszHasBeenRead 	= szNotRead;
		pszRetrieve 	= szGet;
		//ulHash			= ::PersonaHash(pszHash);	//DONT HASH Really fishy code doesn't work!! whats going on!! SK
		ulHash = (unsigned long) atol(pszHash);        
		break;

	default:
		ASSERT(0);			// unknown type
		return NULL;
	}

	//
	// If we get this far, we should have everything we need to create
	// and return new message record object to the caller.
	//
	ASSERT(pszMsgNum);
	ASSERT(pszUIDL);
	ASSERT(pszMessageID);
	ASSERT(pszDownloadDate);
	ASSERT(pszDelete);
	ASSERT(pszSkipped);
	ASSERT(pszSave);
	ASSERT(pszHasBeenRead);
	ASSERT(pszRetrieve);
	ASSERT(ulHash);
	return new CMsgRecord(pszMsgNum,
						  pszUIDL,
						  pszMessageID,
						  pszDownloadDate,
						  pszDelete,
						  pszSkipped,
						  pszSave,
						  pszHasBeenRead,
						  pszRetrieve,
						  ulHash);
}



CMsgRecord* CMsgRecord::CreateMsgRecordFromFile(CString strPersona, char* pszBuffer, int nBufferLen, int *pnMsgNum /*=NULL*/)
{
	if (nBufferLen != LMOS_RECORD_SIZE)
	{
		ASSERT(0);
		return NULL;
	}

	int nMsgNum = 0;
	char* pszUIDL			= NULL;			// UIDL ID
	char* pszMessageID		= NULL;			// Message ID
	//char* pszDownloadDate	= NULL;			// Date message read from server
	
	unsigned long ulDate = 0;
	unsigned long ulFlags = 0;
	unsigned long ulHash = 0;
	unsigned long ulEstSize = 0;

	//Message Number
	nMsgNum = *pnMsgNum;
	(*pnMsgNum)++;

	// Seek to UIDL
	pszUIDL = pszBuffer + LMOS_UIDL_OFFSET;
	
	// Seek to MessageID
	pszMessageID = pszBuffer + LMOS_MSGID_OFFSET;
	
	// Seek to DownloadDate
	//pszDownloadDate = pszBuffer + LMOS_DATE_OFFSET;
	memcpy(&ulDate, pszBuffer + LMOS_DATE_OFFSET, LMOS_DATE_SIZE);

	memcpy(&ulFlags, pszBuffer + LMOS_FLAGS_OFFSET, LMOS_FLAGS_SIZE);
	
	//Get Hash vale as a unsigned long
	memcpy(&ulHash, pszBuffer + LMOS_HASH_OFFSET, LMOS_HASH_SIZE);
	
	//Get the estimated size
	memcpy(&ulEstSize, pszBuffer + LMOS_ESTSIZE_OFFSET, LMOS_ESTSIZE_SIZE);

	
	if(ulHash == 0)
	{
		// The Hash has no value in it, so we need to compute a new Hash value.
		ulHash	= ::PersonaHash(pszUIDL, strPersona);
	}
	
	return new CMsgRecord(nMsgNum,
						  pszUIDL,
						  pszMessageID,
						  ulDate,		//pszDownloadDate,
						  ulFlags,
						  ulHash,
						  ulEstSize);
}


