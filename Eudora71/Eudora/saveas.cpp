// SAVEAS.CPP
//
// Copyright (c) 1994-2000 by QUALCOMM, Incorporated
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


#include "DebugNewHelpers.h"
                                              

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

	if ((m_File = DEBUG_NEW_NOTHROW JJFile) == NULL)
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


