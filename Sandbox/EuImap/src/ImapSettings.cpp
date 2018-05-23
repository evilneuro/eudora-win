// ImapSettinfs.cpp - Implementation of CImapSettings.
//

#include "stdafx.h"

#ifdef IMAP4

#include "resource.h"
#include "rs.h"
#include "persona.h"
#include "ImapSettings.h"
#include "imapactl.h"


// ================================ CImapSettings =================//

// set default values:
CImapSettings::CImapSettings ()
{

}

CImapSettings::~CImapSettings()
{


}



// GrabSettings [PUBLIC]
//
// Note: This calss only goes and gets IMAP account-specific or mailbox-specific
// settings. Use CPOPSettings for the rest.
//
void CImapSettings::GrabSettings(unsigned long AccountID)
{
	m_AccountID = AccountID;

	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount ( m_AccountID );

	if (pAccount)
	{
		// Get our personality name.
		CString strPersona = g_Personalities.ImapGetByHash( pAccount->GetAccountID() );

		// Switch current:
		CString	homie =	g_Personalities.GetCurrent();
		CRString Dominant( IDS_DOMINANT );

		if ( strPersona == Dominant )	// "<Dominant>" is just a placeholder
			strPersona = "";

		g_Personalities.SetCurrent( strPersona );

		// Set them for this personality:
		//

		// Doing minimal download.
		//
		m_nDoingMinimalDownload = ::GetIniShort (IDS_INI_IMAP_MINDNLOAD);

		// AutoExpunge.!!
		//
		m_bRemoveOnDelete = ::GetIniShort (IDS_INI_IMAP_REMOVE_ON_DELETE);

		// Reset!
		g_Personalities.SetCurrent( homie );
	}
}


// Get the value from the settings object, not from INI.
//
long CImapSettings::GetSettingsLong (UINT StringNum)
{
	long lRes = 0;

	switch (StringNum)
	{
		case IDS_INI_IMAP_REMOVE_ON_DELETE:
			lRes = m_bRemoveOnDelete;
			break;

		case IDS_INI_IMAP_MINDNLOAD:
			lRes = m_nDoingMinimalDownload;
			break;

		default:
			break;
	}


	return lRes;
}


// Get the shorts.
//
short CImapSettings::GetSettingsShort (UINT StringNum)
{
	return (short) this->GetSettingsLong (StringNum);
}


// Get pointers to strings.
//
LPCSTR CImapSettings::GetSettingsString (UINT StringNum, char *Buffer /* = NULL */, int len /* = -1 */)
{
	LPCSTR pStr = NULL;

	// Clear the buffer if any.
	//
	if (Buffer)
	{
		*Buffer = 0;
	}

	/*
	switch (StringNum)
	{

		default:
			break;


	}
	*/

	if (pStr)
	{
		int sLen = strlen (pStr);

		if ( Buffer && (len > sLen) )
		{
			strcpy (Buffer, pStr);
		}
	}

	return pStr;
}



#endif // IMAP4

