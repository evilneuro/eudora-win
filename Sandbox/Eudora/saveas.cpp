// SAVEAS.CPP
//

#include "stdafx.h"

#include <dlgs.h>

#include "QCUtils.h"

#include "saveas.h"
#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "doc.h"
#include "nickdoc.h"
#include "3dformv.h"
#include "fileutil.h"
#include "font.h"
#include "utils.h"
#include "address.h"
#include "summary.h"
#include "tocdoc.h"
#include "eudora.h"
#include "SaveAsDialog.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif           
                                              

CSaveAs::CSaveAs()
{
	m_File = NULL;
	m_Stationery = FALSE;
}

CSaveAs::~CSaveAs()
{
	delete m_File;
}

BOOL CSaveAs::Start(const char* Filename /*= NULL*/, BOOL IsMessage /*= FALSE*/, CWnd* pParent /*= NULL*/, BOOL Stationery /*= FALSE*/)
{
	char Name[_MAX_PATH + 1];
	char	*theName = Name;
	CString	szPathName;
	CRString	szStatExt( IDS_STATIONERY_EXTENSION );

#ifndef COMMERCIAL
	Stationery = FALSE;		// Lite doesn't support saving (creating) Stationery
#endif //COMMERCIAL
			
	if (Filename)
	{
		strcpy(Name, Filename);
		char* Slash = strrchr(Name, SLASH);
		if (Slash)
			strcpy(Name,Slash+1);
		::StripIllegalMT(Name, EudoraDir);
		if (!::LongFileSupportMT(EudoraDir))
		{
			if (strlen(Name) >= 8)
				Name[8] = 0;
		}
	}
	else
		*Name = 0;
	
	//m_Stationery = Stationery;
	m_Stationery = FALSE;
	
	CRString Extension(IDS_TEXT_EXTENSION);
	//CRString Filter(Stationery? IDS_STATIONERY_FILE_FILTER32 : IDS_TEXT_FILE_FILTER);
	CRString Filter(IDS_TXT_HTML_FILE_FILTER);

	CSaveAsDialog theDlg(Name, IsMessage, Stationery, Extension, Filter, pParent);
		
	if ( theDlg.DoModal() == IDOK)
	{
		//
		// Hack alert!  Under the 32-bit Version 4 shell, the OnOK()
		// method of dialog doesn't get called!  Therefore, this is a
		// hack workaround to manually update these INI settings
		// outside of the dialog class.  Whatta hack.
		//
		if (IsVersion4())
		{
			SetIniInt(IDS_INI_INCLUDE_HEADERS, theDlg.m_Inc);
			SetIniInt(IDS_INI_GUESS_PARAGRAPHS, theDlg.m_Guess);
		}
		szPathName = theDlg.GetPathName();
		m_Stationery = theDlg.m_IsStat;
		// determine whether or not this is stationery by the file extension      
/*		if ( !m_Stationery && szPathName.GetLength() > szStatExt.GetLength() + 1 )
		{                             
			if ( ( szPathName.Right( szStatExt.GetLength() + 1 ) ).CompareNoCase( "." + szStatExt ) == 0 )
			{     
				m_Stationery = TRUE;			
			}
		}		
*/			
		return( CreateFile( szPathName ) );
	}

	DWORD	huh = 0;
	huh = CommDlgExtendedError();

	return (FALSE);
}

BOOL CSaveAs::CreateFile(const char* Filename)
{
	char realFileName[_MAX_PATH + 1];
	strcpy(realFileName,Filename);

	if ((m_File = new JJFile) == NULL)
		return FALSE;
	if (FAILED(m_File->Open(realFileName, O_CREAT | O_TRUNC | O_WRONLY)))
		return FALSE;

	m_PathName = realFileName;
	return (TRUE);
}


BOOL CSaveAs::PutText(const char* Text)
{
	if (!m_File)
	{
		ASSERT(FALSE);
		return (FALSE);
	}
	
	return (FAILED(m_File->Put(Text)) ? FALSE : TRUE);
}


