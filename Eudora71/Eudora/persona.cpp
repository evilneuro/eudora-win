// persona.cpp
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"		// precompiled header stuff

#include <QCUtils.h>

#include "resource.h"
#include "password.h"
#include "rs.h"
#include "utils.h"
#include "QCPersonalityDirector.h"
#include "QCMailboxDirector.h" // For IMAP account stuff.
#include "ImapAccountMgr.h"
#include "SpoolMgr.h"  //for CreateSpoolMgr when adding new persona
#include "PersParams.h"
#include "TaskStatusView.h"
#include "EudoraMsgs.h"

#include "persona.h"
#include "QCTaskManager.h"

#include "DebugNewHelpers.h"


extern QCPersonalityDirector g_thePersonalityDirector;

// For IMAP stuff.
extern QCMailboxDirector g_theMailboxDirector;

// maybe this should come out of the resource file...
const CString csPersonaPreface = "Persona-";

CPersonality::CPersonality( void )
{
	// initialize member variables
	m_bListStale = TRUE;
	m_ListBuf[ 0 ] = '\0';
	m_Current = "";
}

void CPersonality::Init( void )
{
	// these can't be called from a global object's constuctor...
	QCLoadString( IDS_PERSONALITIES, m_szPersonality, sizeof( m_szPersonality ) );

	m_Dominant = CRString( IDS_DOMINANT );
}

BOOL CPersonality::IsA( const char* Name ) const
{
	char Persona[ 30 ];
	char Entry[ MAX_PERSONA_LEN ];
	CString PersonaID = csPersonaPreface + Name;

	// check for <Dominant> (special case)
	if ( IsDominant(Name) )
		return TRUE;

	// make sure the personality exists
	for ( int i = 0; 1 /* forever */; i++ )
	{
		sprintf( Persona, "Persona%d", i );
		GetPrivateProfileString( m_szPersonality, Persona, "", Entry, sizeof( Entry ), INIPath );
		if ( strcmpi( PersonaID, Entry ) == 0 )
			break;
		if ( Entry[ 0 ] == '\0' )
			return FALSE;	// Name not found
	}

	return TRUE;			// must have found it
}


BOOL CPersonality::IsDominant( const char* Name ) const
{
	return ( m_Dominant == Name );
}

DWORD CPersonality::GetHash( const char* Name )
{
	// treat <Dominant> as a alias for ""
	if ( IsDominant(Name) )
		return ::HashMT( "" );
	else
		return ::HashMT( Name );
}

CString CPersonality::GetByHash( DWORD HashVal )
{
	LPSTR lpPersonalities = List();

	CString Persona = lpPersonalities;
	while ( ! Persona.IsEmpty() )
	{
		// does this one match
		DWORD dwTemp = ::HashMT( Persona.GetBuffer( 0 ) );
		if ( dwTemp == HashVal )
			return Persona;

		// advance to next personality
		lpPersonalities += strlen( lpPersonalities ) + 1;
		Persona = lpPersonalities;
	}

	Persona.Empty();
	return Persona;
}


//
// If HasVal corresponds to the dominant personality, return the string
//  <Dominant> instead of an empty string.
//
CString CPersonality::ImapGetByHash( DWORD HashVal )
{
	if ( HashVal == ::HashMT( "" ) )
	{
		return m_Dominant;
	}
	else
	{
		LPSTR lpPersonalities = List();

		CString Persona = lpPersonalities;

		while ( ! Persona.IsEmpty() )
		{
			// does this one match
			DWORD dwTemp = ::HashMT( Persona.GetBuffer( 0 ) );
			if ( dwTemp == HashVal )
				return Persona;

			// advance to next personality
			lpPersonalities += strlen( lpPersonalities ) + 1;
			Persona = lpPersonalities;
		}

		Persona.Empty();
		return Persona;
	}
}



CString CPersonality::GetCurrent( void ) const
{
	// strip the Persona perface
	CString	tmp;
	
	if ( m_Current.IsEmpty() )
		tmp.Empty();
	else
		tmp = m_Current.Mid( csPersonaPreface.GetLength() );

	return tmp;
}

BOOL CPersonality::SetCurrent( const char* Name )
{
	CString PersonaID;

	if ( Name && *Name && ! IsDominant(Name) )
		PersonaID = csPersonaPreface + Name;
	else
		PersonaID.Empty();

	if ( PersonaID != m_Current )
	{
		// save off the password info.
		SavePassInfo();

		// flush and reset the ini cache
		FlushINIFile();
		RemoveAllFromCache();

		// we're trusting souls
		m_Current = PersonaID;

		// restore any password info.
		RestorePassInfo();
	}

	return TRUE;
}

// --------------------------------------------------------------------------

// [PUBLIC] Modify
//
// Changes the attributes of the personality with the given name.
//
BOOL CPersonality::Modify(const CPersParams& Params)
{
	if ( !IsA(Params.PersName) )
		return FALSE;

	if ( !SavePersonaInfo(Params, false) )
		return FALSE;

	RemoveAllFromCache(); // Invalidate the INI cache

	return TRUE;
}

// --------------------------------------------------------------------------

BOOL CPersonality::Add( CPersParams & Params )
{
	if ( Params.PersName.IsEmpty() || IsDominant(Params.PersName) )
		return FALSE;		// alternate personalities must have names

	char Persona[ 30 ];
	char Entry[ MAX_PERSONA_LEN ];
	CString PersonaID = csPersonaPreface + Params.PersName;

	// find the first available entry
	for ( int i = 0; 1 /* forever */ ; i++ )
	{
		sprintf( Persona, "Persona%d", i );
		GetPrivateProfileString( m_szPersonality, Persona, "", Entry, sizeof( Entry ), INIPath );
		if ( ! Entry[ 0 ] )
			break;
		if ( strcmpi( Entry, PersonaID ) == 0 )
			return TRUE;	// already in [Personalities] list
	}

	if ( ! WritePrivateProfileString(m_szPersonality, Persona, PersonaID, INIPath) )
		return FALSE;

	if ( ! SavePersonaInfo( Params, true ) )
		return FALSE;

	//create a spool manager to mange the mail
	QCSpoolMgrMT::CreateSpoolMgr(Params.PersName);

	//
	// Notify the personality director that a new Persona has been added
	// so that it can update its Message:Change:Personality menu.
	//
	g_thePersonalityDirector.AddCommand(Params.PersName);

	m_bListStale = TRUE;

	if (QCGetTaskStatusView())
		QCGetTaskStatusView()->PostMessage(msgTaskViewUpdateCols);

	return TRUE;
}

BOOL CPersonality::GetParams(const char* PersName, CPersParams& params)
{
	if (!IsA(PersName))
	{
		ASSERT(0);			// Doesn't exist
		return FALSE;
	}

	params.PersName = PersName;

	char szValue[128];

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_POP_ACCOUNT), 
								GetIniDefaultValue(IDS_INI_PERSONA_POP_ACCOUNT), 
								szValue, sizeof(szValue));
	params.POPAccount = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_REAL_NAME), 
								GetIniDefaultValue(IDS_INI_PERSONA_REAL_NAME),
								szValue, sizeof(szValue));
	params.RealName = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_RETURN_ADDR), 
								GetIniDefaultValue(IDS_INI_PERSONA_RETURN_ADDR),
								szValue, sizeof(szValue));
	params.ReturnAddress = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_DEFAULT_DOMAIN), 
								GetIniDefaultValue(IDS_INI_PERSONA_DEFAULT_DOMAIN),
								szValue, sizeof(szValue));
	params.DefaultDomain = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_SMTP_SERVER), 
								GetIniDefaultValue(IDS_INI_PERSONA_SMTP_SERVER),
								szValue, sizeof(szValue));
	params.OutServer = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_POP_SERVER), 
								GetIniDefaultValue(IDS_INI_PERSONA_POP_SERVER),
								szValue, sizeof(szValue));
	params.InServer = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_LOGIN_NAME), 
								GetIniDefaultValue(IDS_INI_PERSONA_LOGIN_NAME),
								szValue, sizeof(szValue));
	params.LoginName = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_IMAP_PREFIX), 
								GetIniDefaultValue(IDS_INI_PERSONA_IMAP_PREFIX),
								szValue, sizeof(szValue));
	params.IMAPPrefix = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_STATIONERY), 
								GetIniDefaultValue(IDS_INI_PERSONA_STATIONERY),
								szValue, sizeof(szValue));
	params.Stationery = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_SIGNATURE), 
								GetIniDefaultValue(IDS_INI_PERSONA_SIGNATURE),
								szValue, sizeof(szValue));
	params.Signature = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_CHECK_MAIL), 
								GetIniDefaultValue(IDS_INI_PERSONA_CHECK_MAIL),
								szValue, sizeof(szValue));
	params.bCheckMail = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_LMOS), 
								GetIniDefaultValue(IDS_INI_PERSONA_LMOS),
								szValue, sizeof(szValue));
	params.bLMOS = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_AUTH_PASS), 
								GetIniDefaultValue(IDS_INI_AUTH_PASS),
								szValue, sizeof(szValue));
	params.bPassword = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_AUTH_KERB), 
								GetIniDefaultValue(IDS_INI_AUTH_KERB),
								szValue, sizeof(szValue));
	params.bKerberos = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_AUTH_APOP), 
								GetIniDefaultValue(IDS_INI_AUTH_APOP),
								szValue, sizeof(szValue));
	params.bAPop = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_CONNECT_WINSOCK), 
								GetIniDefaultValue(IDS_INI_CONNECT_WINSOCK),
								szValue, sizeof(szValue));
	params.bWinSock = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_AUTH_RPA), 
								GetIniDefaultValue(IDS_INI_AUTH_RPA),
								szValue, sizeof(szValue));
	params.bRPA = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_USES_IMAP), 
								GetIniDefaultValue(IDS_INI_USES_IMAP),
								szValue, sizeof(szValue));
	params.bIMAP = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_USES_POP), 
								GetIniDefaultValue(IDS_INI_USES_POP),
								szValue, sizeof(szValue));
	params.bPOP = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_LEAVE_ON_SERVER_DAYS), 
								GetIniDefaultValue(IDS_INI_LEAVE_ON_SERVER_DAYS),
								szValue, sizeof(szValue));
	params.LeaveOnServDays = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_BIG_MESSAGE_THRESHOLD), 
								GetIniDefaultValue(IDS_INI_BIG_MESSAGE_THRESHOLD),
								szValue, sizeof(szValue));
	params.BigMsgThreshold = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_IMAP_MAXSIZE), 
								GetIniDefaultValue(IDS_INI_IMAP_MAXSIZE),
								szValue, sizeof(szValue));
	params.IMAPMaxSize = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_DELETE_MAIL_FROM_SERVER), 
								GetIniDefaultValue(IDS_INI_DELETE_MAIL_FROM_SERVER),
								szValue, sizeof(szValue));
	params.bDelServerAfter = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_SERVER_DELETE), 
								GetIniDefaultValue(IDS_INI_SERVER_DELETE),
								szValue, sizeof(szValue));
	params.bDelWhenTrashed = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_SKIP_BIG_MESSAGES), 
								GetIniDefaultValue(IDS_INI_SKIP_BIG_MESSAGES),
								szValue, sizeof(szValue));
	params.bSkipBigMsgs = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_IMAP_MINDNLOAD), 
								GetIniDefaultValue(IDS_INI_IMAP_MINDNLOAD),
								szValue, sizeof(szValue));
	params.bIMAPMinDwnld = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_IMAP_OMITATTACH), 
								GetIniDefaultValue(IDS_INI_IMAP_OMITATTACH),
								szValue, sizeof(szValue));
	params.bIMAPFullDwnld = atoi(szValue);

	// IMAP - Initialize current method for how to handle deleting messages.
	GetProfileString(PersName, GetIniKeyName(IDS_INI_IMAP_XFERTOTRASH), 
								GetIniDefaultValue(IDS_INI_IMAP_XFERTOTRASH),
								szValue, sizeof(szValue));
	params.bIMAPXferToTrash = atoi(szValue);

	// This is always the opposite of bIMAPXferToTrash.
	params.bIMAPMarkDeleted = !params.bIMAPXferToTrash;

	// And current name of Trash mailbox, if any.
	GetProfileString(PersName, GetIniKeyName(IDS_INI_IMAP_TRASH_MBOXNAME), 
								GetIniDefaultValue(IDS_INI_IMAP_TRASH_MBOXNAME),
								szValue, sizeof(szValue));
	params.IMAPTrashMailbox = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_IMAP_AUTO_EXPUNGE_NEVER), 
								GetIniDefaultValue(IDS_INI_IMAP_AUTO_EXPUNGE_NEVER),
								szValue, sizeof(szValue));
	params.bIMAPAutoExpNever = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS), 
								GetIniDefaultValue(IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS),
								szValue, sizeof(szValue));
	params.bIMAPAutoExpAlways = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT), 
								GetIniDefaultValue(IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT),
								szValue, sizeof(szValue));
	params.bIMAPAutoExpOnPct = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_IMAP_AUTO_EXPUNGE_PCT), 
								GetIniDefaultValue(IDS_INI_IMAP_AUTO_EXPUNGE_PCT),
								szValue, sizeof(szValue));
	params.strIMAPAutoExpPct = szValue;

	GetProfileString(PersName, GetIniKeyName(IDS_INI_SMTP_AUTH_ALLOWED), 
								GetIniDefaultValue(IDS_INI_SMTP_AUTH_ALLOWED),
								szValue, sizeof(szValue));
	params.bSMTPAuthAllowed = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_PERSONA_USE_RELAY), 
								GetIniDefaultValue(IDS_INI_PERSONA_USE_RELAY),
								szValue, sizeof(szValue));
	params.bUseSMTPRelay = atoi(szValue);

	GetProfileString(PersName, GetIniKeyName(IDS_INI_USE_SUBMISSION_PORT), 
								GetIniDefaultValue(IDS_INI_USE_SUBMISSION_PORT),
								szValue, sizeof(szValue));
	params.bUseSubmissionPort = atoi(szValue);

	
	GetProfileString(PersName, GetIniKeyName(IDS_INI_SSL_SEND_USE), 
								GetIniDefaultValue(IDS_INI_SSL_SEND_USE),
								szValue, sizeof(szValue));
	params.m_SSLSendUsage  = atoi(szValue);


	GetProfileString(PersName, GetIniKeyName(IDS_INI_SSL_RECEIVE_USE), 
								GetIniDefaultValue(IDS_INI_SSL_RECEIVE_USE),
								szValue, sizeof(szValue));
	params.m_SSLReceiveUsage  =  atoi(szValue);


	return (TRUE);
}

BOOL CPersonality::Clone(const char* strNewName, const char* strOldName)
{
	if (!strNewName || !*strNewName)
	{
		ASSERT(0);
		return FALSE;
	}
	else if (IsA(strNewName))
	{
		ASSERT(0);			// huh? target already exists
		return FALSE;
	}
	else if (!IsA(strOldName))
	{
		ASSERT(0);			// huh? source doesn't exist
		return FALSE;
	}

	//
	// Grab all values from old personality.
	//
	CPersParams params;
	GetParams(strOldName, params);

	//
	// Change the name
	//
	params.PersName = strNewName;

	//
	// The moment of truth ... do the cloning.
	//
	return Add(params);
}


BOOL CPersonality::Remove( const char* Name )
{
	char Persona[ 30 ];
	char Entry[ MAX_PERSONA_LEN ];
	CString PersonaID = csPersonaPreface + Name;

	if( QCGetTaskManager()->GetTaskCount(Name) != 0)
	{
		AfxMessageBox( CRString(IDS_CANT_REMOVE_PERSONA));
		return FALSE;
	}
	// make sure the personality exists
	for ( int i = 0; 1 /* forever */; i++ )
	{
		sprintf( Persona, "Persona%d", i );
		GetPrivateProfileString( m_szPersonality, Persona, "", Entry, sizeof( Entry ), INIPath );
		if ( strcmpi( PersonaID, Entry ) == 0 )
			break;
		if ( Entry[ 0 ] == '\0' )
			return FALSE;	// Name not found
	}

#ifdef IMAP4
	//
	// Notify mailbox director that the Persona is about to be removed.
	g_theMailboxDirector.WillDeletePersona (Name);
#endif // IMAP4

	// pack the Personalities index
	for ( /* i is set above */ ; 1 /*forever*/; i++ )
	{
		sprintf( Persona, "Persona%d", i + 1 );
		GetPrivateProfileString( m_szPersonality, Persona, "", Entry, sizeof( Entry ), INIPath );
		if ( Entry[ 0 ] == '\0' )
			break;			// all done with personality list

		// replace Persona[ i ] with Persona[ j ]
		sprintf( Persona, "Persona%d", i );
		WritePrivateProfileString(m_szPersonality, Persona, Entry, INIPath);
	}

	//clean up the last (duplicated) Persona
	sprintf( Persona, "Persona%d", i );
	WritePrivateProfileString(m_szPersonality, Persona, NULL, INIPath);

	// delete the section associated with the personality
	WritePrivateProfileString(PersonaID, NULL, NULL, INIPath);

	//
	// Notify the personality director that the Persona has been nuked
	// so that it can update its Message:Change:Personality menu.
	//
	g_thePersonalityDirector.DeleteCommand(Name);

	m_bListStale = TRUE;

	if (QCGetTaskStatusView())
		QCGetTaskStatusView()->PostMessage(msgTaskViewUpdateCols);

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// GetIniKeyName [public, static]
//
////////////////////////////////////////////////////////////////////////
CString CPersonality::GetIniKeyName(UINT uIniID)
{
	//
	// Load the INI key into 'szKey', stripping off the default value.
	//
	char szKey[64];
	::QCLoadString(uIniID, szKey, sizeof(szKey));
	char * pszKey = ::strchr(szKey, '\n');		
	if (pszKey) 
		*pszKey = '\0';

	return CString(szKey);
}


////////////////////////////////////////////////////////////////////////
// GetIniDefaultValue [public, static]
//
////////////////////////////////////////////////////////////////////////
CString CPersonality::GetIniDefaultValue(UINT uIniID)
{
	//
	// Load the INI key into 'szKey', looking for the default value,
	// if any.
	//
	char szKey[256]={0};

	::QCLoadString(uIniID, szKey, sizeof(szKey));
	char * pszKey = ::strchr(szKey, '\n');		
	if (pszKey)
		return CString(pszKey + 1); 
	else
		return CString("");
}


// returns ptr to null delimited list of null delimited strings
// treat as read-only limited-life (like a getenv() call)
// THIS LIST IS ALPHABETICAL EXCEPT <DOMINANT> COMES FIRST
LPSTR CPersonality::List( void )
{
	char Persona[ 30 ];
	char Entry[ MAX_PERSONA_LEN ];

	if ( m_bListStale )
	{
		LPSTR lpBuf = m_ListBuf;
		CSortedStringListMT theSortedList;

		for ( int i = 0; i < MAX_PERSONAS-1; i++ )
		{
			sprintf( Persona, "Persona%d", i );
			GetPrivateProfileString( m_szPersonality, Persona, "", Entry, sizeof( Entry ), INIPath );
			if ( Entry[ 0 ] )
			{
				// strip the Persona perface
				strcpy( m_ListBuf, &Entry[ csPersonaPreface.GetLength() ] );
				theSortedList.Add( m_ListBuf );
			}
			else
				break;	// all done
		}
		// add the Dominant personality
		CRString Dominant( IDS_DOMINANT );
		strcpy( lpBuf, Dominant );
		lpBuf += strlen( lpBuf ) + 1;

		// build the rest of the list based on theSortedList
	  	POSITION pos = theSortedList.GetHeadPosition();
		while( pos )
		{
			CString listItem = theSortedList.GetNext(pos);
			strcpy( lpBuf, listItem );
			lpBuf += strlen( lpBuf ) + 1;
		}

		// null-delimit the list
		*lpBuf++ = '\0';
		*lpBuf   = '\0';	// in case there are no members

		m_bListStale = FALSE;
	}

	return m_ListBuf;
}

// THIS LIST IS ORDERED BASED ON THE [PERSONALITIES] SECTION OF EUDORA.INI
// EXCEPT <DOMINANT> COMES FIRST
LPSTR CPersonality::CheckMailList( void )
{
	char szKey[ 64 ];
	char szDefault[ 64 ];
	char Persona[ 30 ];
	char Entry[ MAX_PERSONA_LEN ];

	// load the key - strip the default value
	QCLoadString( IDS_INI_PERSONA_CHECK_MAIL, szKey, sizeof( szKey ) );
	char * pc = strchr( szKey, '\n' );
	if ( pc ) 
	{
		*pc = '\0';
		strncpy( szDefault, pc + 1, sizeof( szDefault - 1 ) );
	}

	LPSTR lpBuf = m_CheckMailBuf;

	// add the Dominant personality
	BOOL bCheckMail = (BOOL)GetPrivateProfileInt( "Settings", szKey, atoi( szDefault ), INIPath );
	if ( bCheckMail )
	{
		strcpy( lpBuf, m_Dominant );
		lpBuf += strlen( lpBuf ) + 1;
	}

	for ( int i = 0; i < MAX_PERSONAS-1; i++ )
	{
		sprintf( Persona, "Persona%d", i );
		GetPrivateProfileString( m_szPersonality, Persona, "", Entry, sizeof( Entry ), INIPath );
		if ( Entry[ 0 ] )
		{
			bCheckMail = (BOOL)GetPrivateProfileInt( Entry, szKey, atoi( szDefault ), INIPath );

			if ( bCheckMail )
			{
				// strip the Persona preface
				strcpy( lpBuf, &Entry[ csPersonaPreface.GetLength() ] );
				lpBuf += strlen( lpBuf ) + 1;
			}
		}
		else
			break;	// all done
	}

	// null-delimit the list
	*lpBuf++ = '\0';
	*lpBuf   = '\0';	// in case there are no members

	return m_CheckMailBuf;
}

void CPersonality::SaveAllPasswords( void )
{
	CRString key( IDS_INI_SAVE_PASSWORD_TEXT );

	// walk all alternate personalities 
	LPSTR lpPersonalities = List();
	while ( *lpPersonalities )
	{
		CString tmpDude = lpPersonalities;

		//
		// Do differently for IMAP personalities.
		//
		if ( IsImapPersona (tmpDude) )	// Uses IMAP?
		{
#ifdef IMAP4
			//
			// Uses IMAP. Get the password from the cached account object.
			//
			CString ImapPassword;
			g_ImapAccountMgr.GetPassword (tmpDude, ImapPassword);

			WriteProfileString( tmpDude, key, EncodePassword(ImapPassword) );
#endif // IMAP4
		}
		else
		{
			//
			// Do the yucky POP global thing. (JOK)
			//
			WriteProfileString( tmpDude, key, EncodePassword(POPPassword) );
		}

		// advance to next personality
		lpPersonalities += strlen( lpPersonalities ) + 1;
	}
}

void CPersonality::EraseAllPasswords( void )
{
	CRString key( IDS_INI_SAVE_PASSWORD_TEXT );

/*#ifdef IMAP4
	// If the dominant personality is IMAP, erase it's password as well.
	//
	CRString tmpDominant = CRString( IDS_DOMINANT );

	if ( IsImapPersona (tmpDominant) )	// Uses IMAP?
	{
		WriteProfileString( tmpDominant, key, "" );
	}
#endif // IMAP4
*/
	// walk all alternate personalities
	LPSTR lpPersonalities = List();
	while ( *lpPersonalities )
	{
		CString tmpDude = lpPersonalities;

		WriteProfileString( tmpDude, key, "" );

		// advance to next personality
		lpPersonalities += strlen( lpPersonalities ) + 1;
	}

	// flush and reset the caches
	FlushINIFile();
	RemoveAllFromCache();
	
	//m_Passwords.RemoveAll();

	// IMAP NOTE:: This just says to erase passwords from disk and not to 
	// save cached ones. It doesn't say to clear the currently cached passwords,
	// so don't call g_ImapAccountMgr.ClearAllPasswords() here.
}

bool CPersonality::AccountsHavePasswords()
{
	ASSERT(::IsMainThreadMT());

	CPersonaStateMgr reset;
	
	LPSTR lpPersonalities = List();
	while ( *lpPersonalities )
	{
		CString tmpDude = lpPersonalities;
		
		if (!IsImapPersona (tmpDude) )
		{
			g_Personalities.SetCurrent(tmpDude);
		
			if( !POPPassword.IsEmpty())
				return true;
			
			CString pass = GetIniString(IDS_INI_SAVE_PASSWORD_TEXT);
			if( !pass.IsEmpty())
				return true;
		}
		// advance to next personality
		lpPersonalities += strlen( lpPersonalities ) + 1;
	}
	
	return false;
}


void CPersonality::ForgetPasswords(LPCSTR lpPersonalities)
{

	CString strCurrent = GetCurrent();
	// walk all alternate personalities
	while ( *lpPersonalities )
	{
		CString tmpDude = lpPersonalities;
		g_Personalities.SetCurrent(tmpDude);

		if ( IsImapPersona (tmpDude) )
		{
			GetImapAccountMgr()->ForgetPassword(tmpDude);
		}
		else
		{
			SetIniString(IDS_INI_SAVE_PASSWORD_TEXT, NULL);
			POPPassword.Empty();
		}

		// advance to next personality
		lpPersonalities += strlen( lpPersonalities ) + 1;
	}
	g_Personalities.SetCurrent(strCurrent);

	// flush and reset the caches
	//FlushINIFile();
	//RemoveAllFromCache();
	//m_Passwords.RemoveAll();

	// IMAP NOTE:: This just says to erase passwords from disk and not to 
	// save cached ones. It doesn't say to clear the currently cached passwords,
	// so don't call g_ImapAccountMgr.ClearAllPasswords() here.
}


UINT CPersonality::GetProfileInt
( 
	LPCTSTR	lpName,
	LPCTSTR	lpKeyName,
	int		nDefault 
)
{
	CString PersonaID;

	if ( *lpName == '\0' || ! strcmp( lpName, m_Dominant ) )
		PersonaID = "Settings";
	else
		PersonaID = csPersonaPreface + lpName;

	return ::GetPrivateProfileInt( PersonaID, lpKeyName, nDefault, INIPath );
}

	
DWORD CPersonality::GetProfileString
( 
	LPCTSTR	lpName, 
    LPCTSTR	lpKeyName, 
	LPCTSTR	lpDefault, 
	LPTSTR	lpReturnString, 
	DWORD	nSize )
{
	CString PersonaID;

	if ( *lpName == '\0' || ! strcmp( lpName, m_Dominant ) )
		PersonaID = "Settings";
	else
		PersonaID = csPersonaPreface + lpName;

	return ::GetPrivateProfileString( PersonaID, lpKeyName, lpDefault, 
									  lpReturnString, nSize, INIPath );
}

BOOL CPersonality::WriteProfileString
( 
	LPCTSTR	lpName,
	LPCTSTR	lpKeyName,
	LPCTSTR	lpString 
)
{
	CString PersonaID;

	if ( *lpName == '\0' || ! strcmp( lpName, m_Dominant ) )
		PersonaID = "Settings";
	else
		PersonaID = csPersonaPreface + lpName;

	return ::WritePrivateProfileString( PersonaID, lpKeyName, lpString, INIPath );
}

// helper methods
BOOL CPersonality::SavePersonaInfo(const CPersParams& Params, bool bAddPersona)
{
//	DoParamsCheck(Params); // Check params against bad behavior

	CString PersonaID;
	
	if ( Params.PersName.IsEmpty() || IsDominant(Params.PersName) )
		PersonaID = "Settings";
	else
		PersonaID = csPersonaPreface + Params.PersName;

	// Strings

	if ( ! WriteStrHelper( PersonaID, IDS_INI_PERSONA_POP_ACCOUNT, Params.POPAccount ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_PERSONA_REAL_NAME, Params.RealName ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_PERSONA_RETURN_ADDR, Params.ReturnAddress ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_PERSONA_DEFAULT_DOMAIN, Params.DefaultDomain ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_PERSONA_SMTP_SERVER, Params.OutServer ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_PERSONA_POP_SERVER, Params.InServer ) )
		return FALSE;
	
	if ( ! WriteStrHelper( PersonaID, IDS_INI_PERSONA_LOGIN_NAME, Params.LoginName ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_PERSONA_IMAP_PREFIX, Params.IMAPPrefix ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_PERSONA_STATIONERY, Params.Stationery ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_PERSONA_SIGNATURE, Params.Signature ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_LEAVE_ON_SERVER_DAYS, Params.LeaveOnServDays ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_BIG_MESSAGE_THRESHOLD, Params.BigMsgThreshold ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_IMAP_MAXSIZE, Params.IMAPMaxSize ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_IMAP_TRASH_MBOXNAME, Params.IMAPTrashMailbox ) )
		return FALSE;

	if ( ! WriteStrHelper( PersonaID, IDS_INI_IMAP_AUTO_EXPUNGE_PCT, Params.strIMAPAutoExpPct ) )
		return FALSE;


	// Booleans

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_PERSONA_CHECK_MAIL, Params.bCheckMail ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_PERSONA_LMOS, Params.bLMOS ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_AUTH_KERB, Params.bKerberos ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_AUTH_APOP, Params.bAPop ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_CONNECT_WINSOCK, Params.bWinSock ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_AUTH_RPA, Params.bRPA ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_USES_IMAP, Params.bIMAP ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_USES_POP, Params.bPOP ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_AUTH_PASS, Params.bPassword ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_DELETE_MAIL_FROM_SERVER, Params.bDelServerAfter ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_SERVER_DELETE, Params.bDelWhenTrashed ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_SKIP_BIG_MESSAGES, Params.bSkipBigMsgs ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_IMAP_MINDNLOAD, Params.bIMAPMinDwnld ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_IMAP_OMITATTACH, Params.bIMAPFullDwnld ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_IMAP_XFERTOTRASH, Params.bIMAPXferToTrash ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_IMAP_MARK_DELETED, Params.bIMAPMarkDeleted ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_IMAP_AUTO_EXPUNGE_NEVER, Params.bIMAPAutoExpNever ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS, Params.bIMAPAutoExpAlways ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT, Params.bIMAPAutoExpOnPct ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_SMTP_AUTH_ALLOWED, Params.bSMTPAuthAllowed ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_PERSONA_USE_RELAY, Params.bUseSMTPRelay ) )
		return FALSE;

	if ( ! WriteBoolHelper( PersonaID, IDS_INI_USE_SUBMISSION_PORT, Params.bUseSubmissionPort ) )
		return FALSE;

	if ( ! WriteIntHelper( PersonaID, IDS_INI_SSL_RECEIVE_USE, Params.m_SSLReceiveUsage) )
		return FALSE;

	if ( ! WriteIntHelper( PersonaID, IDS_INI_SSL_SEND_USE, Params.m_SSLSendUsage ) )
		return FALSE;

//	// make sure that auth_passwords is 0 if this is an IMAP account
//	if( Params.bIMAP)
//		WriteBoolHelper( PersonaID, IDS_INI_AUTH_PASS, 0) ;

	if (bAddPersona)
	{
		// We're adding this persona - init password text
		if ( ! WriteStrHelper(PersonaID, IDS_INI_SAVE_PASSWORD_TEXT, "") )
			return FALSE;
	}

	return TRUE;
}

// Note: (JOK) 4/21/98 - shouldn't get called for IMAP personas.
//
void CPersonality::SavePassInfo( void )
{
	
	if ( m_Current.IsEmpty() || IsDominant(m_Current) )
		m_Passwords[m_Dominant] = POPPassword;
	else
		m_Passwords[m_Current] = POPPassword;

		/*
	POSITION pos, cur_pos;
	CString	 tmp;

	// no personalities
	if ( m_Current.IsEmpty() || IsDominant(m_Current) )
	{
		m_DefaultPassword = POPPassword;
	}
	else
	{
		for( pos = m_Passwords.GetHeadPosition(); pos != NULL; )
		{
			cur_pos = pos;

			tmp = m_Passwords.GetNext( pos );

			// is this for the current personality
			if ( tmp.Find( m_Current ) == 0 )
			{
				m_Passwords.RemoveAt( cur_pos );
				break;
			}
		}

		// add an entry composed of <name><password>
		tmp = m_Current;
		tmp += POPPassword;
		m_Passwords.AddTail( tmp );
	}*/
}


// Note: (JOK) 4/21/98 - shouldn't get called for IMAP personas.
//
void CPersonality::RestorePassInfo( void )
{
	CString strPersona;

	if ( m_Current.IsEmpty() || IsDominant(m_Current) )
		strPersona = m_Dominant;
	else
		strPersona = m_Current;

	if(m_Passwords.Lookup(strPersona, POPPassword) == 0)
		::POPPassword = "";

	
	/*
	POSITION pos;
	CString	 tmp;

	if ( m_Current.IsEmpty() || IsDominant(m_Current) )
	{
		POPPassword = m_DefaultPassword;
	}
	else
	{
		POPPassword = "";

		for( pos = m_Passwords.GetHeadPosition(); pos != NULL; )
		{
			tmp = m_Passwords.GetNext( pos );

			// is this for the current personality
			if ( tmp.Find( m_Current ) == 0 )
			{
				// extract the password
				POPPassword = tmp.Mid( m_Current.GetLength() );
				break;
			}
		}
	}*/
}


BOOL CPersonality::WriteIntHelper(LPCTSTR lpSection, int idEntry, int iVal)
{
	char Entry[ MAX_PERSONA_LEN ];

	QCLoadString( idEntry, Entry, sizeof( Entry ) );

	// remove the default parameter
	char * pc = strchr( Entry, '\n' );		
	if ( pc ) 
		*pc = '\0';

	CString csVal;
	csVal.Format("%d",iVal);


	if (! ::WritePrivateProfileString( lpSection, Entry, csVal, INIPath) )
			return FALSE;

	return TRUE;



}


BOOL CPersonality::WriteBoolHelper( LPCTSTR lpSection, int idEntry, BOOL bOn )
{
	char Entry[ MAX_PERSONA_LEN ];

	QCLoadString( idEntry, Entry, sizeof( Entry ) );

	// remove the default parameter
	char * pc = strchr( Entry, '\n' );		
	if ( pc ) 
		*pc = '\0';

	if ( bOn )
	{
		if ( ! ::WritePrivateProfileString( lpSection, Entry, "1", INIPath) )
			return FALSE;
	}
	else
	{
		if ( ! ::WritePrivateProfileString( lpSection, Entry, "0", INIPath) )
			return FALSE;
	}

	return TRUE;
}

BOOL CPersonality::WriteStrHelper( LPCTSTR lpSection, int idEntry, LPCSTR str )
{
//	if ((str) && (*str != '\0'))
//	{
		char Entry[ MAX_PERSONA_LEN ];

		QCLoadString( idEntry, Entry, sizeof( Entry ) );

		// remove the default parameter
		char * pc = strchr( Entry, '\n' );		
		if ( pc ) 
			*pc = '\0';

		if ( ! ::WritePrivateProfileString( lpSection, Entry, str, INIPath) )
			return FALSE;
//	}

	return TRUE;
}

long CPersonality::GetPersonaCount( )
{
	long i = 0;
	LPSTR lpPersonalities = List();
	CString Persona = lpPersonalities;

	while ( ! Persona.IsEmpty() )
	{
		lpPersonalities += strlen( lpPersonalities ) + 1;
		Persona = lpPersonalities;
		
		i++;
	}

	return i;
}


////////////////////////////////////////////////////////////
// IsImapPersona [PUBLIC]
// Is this an IMAP personality??
////////////////////////////////////////////////////////////
BOOL CPersonality::IsImapPersona (LPCSTR pszPersonaName)
{
	// Sanity.
	if (NULL == pszPersonaName)
	{
		ASSERT (0);
		return FALSE;
	}

	//
	// Read from disk. Don't trust the GetIniShort stuff!! May return Dominant's!!
	//

	CString key = GetIniKeyName( IDS_INI_USES_IMAP );

	return (GetProfileInt(pszPersonaName, key, 0) != 0);
}

void CPersonality::FillComboBox(CComboBox* pCB)
{
	LPSTR lpPersonalities = List();

	while (*lpPersonalities)
	{
		pCB->AddString(lpPersonalities);
		lpPersonalities += strlen( lpPersonalities ) + 1;
	}
}
