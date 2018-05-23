// imapopt.cpp: routines to handle access to options database.
//
// Functions to add IMAP mailboxes to Mailboxes and Transfer menus.
// For WIN32, mailboxes are also added to the tree control.


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.


#include <fcntl.h>

#include "cursor.h"
#include "fileutil.h"
#include "rs.h"
#include "resource.h"

#include "imapopt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// Statics
static CImapOption	*pImapOption = NULL;


// === Static Function =====
static void InitializeOptions (void);

// ====================================================================== //

// class CImapOption
CImapOption::CImapOption ()
{
	m_Path.Empty();
}


CImapOption::CImapOption (const char *pPath)
{
	if (pPath)
	{
		m_Path = pPath;
	}
	else
		m_Path.Empty();
}


CImapOption::~CImapOption ()
{
}


// FUNCTION
// Delete all entries in the database.
// END FUNCTION

// NOTES
// In this particular implementation, truncate the file to zero length.
// END NOTES

BOOL  CImapOption::ClearDatabase ()
{
	JJFile	jFile;
	BOOL	bResult = FALSE;

	if (!m_Path.IsEmpty())
	{
		if (FileExistsMT (m_Path))
		{
			if (jFile.Open (m_Path, O_RDWR) >= 0)
			{
				bResult = jFile.ChangeSize ( 0 ) >= 0;
				jFile.Close();
			}
		}
	}

	return bResult;
}



BOOL CImapOption::ReadString (LPCTSTR section, LPCTSTR optionName, CString &value)
{
	char buf[1024];

	if (m_Path.IsEmpty())
		return FALSE;

	if (!(section && optionName))
		return FALSE;

	if (GetPrivateProfileString (section, optionName, "", buf, sizeof(buf), (const char *)m_Path) == 0)
		return FALSE;
	else
	{
		value = buf;
		value.TrimLeft();
		value.TrimRight();
		return TRUE;
	}
}


BOOL  CImapOption::WriteString (LPCTSTR section, LPCTSTR OptionName, LPCTSTR value)
{
	// Make sure we're OK.	
	if (!(section && OptionName && value))
		return FALSE;

	if (m_Path.IsEmpty())
		return FALSE;

	return WritePrivateProfileString (section, OptionName, value, (LPCTSTR)m_Path);
}

#endif // IMAP4

