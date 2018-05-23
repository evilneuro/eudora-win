/////////////////////////////////////////////////////////////////////////////
// CSignatureManager

#include "stdafx.h"

#include <direct.h>

#include <ctype.h>

#ifdef WIN32
	#include <winver.h>
#else
	#include <ver.h>
#endif

#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "fileutil.h"
#include "doc.h"
#include "debug.h"
#include "utils.h"
#include "sigmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


CSignatureManager *SigMgr = NULL;


CSignatureManager::CSignatureManager()
{
	// Initialize Member variables
	m_SignatureList = NULL;
	m_SignatureCount = 0;
	m_SignatureArray = NULL;
	m_changed = 0;
	
	// build the folder name
	m_SignatureDir = EudoraDir + CRString( IDS_SIGNATURE_FOLDER ); 

	// see if it exists
	if ( _access( m_SignatureDir, 0 ) )
	{
		// nope -- create it
		if ( mkdir( m_SignatureDir ) )
		{
			return;
		}
	}

	// Move signatur.alt file
	MigrateSignatures();
   
	// Save the current dir so that it can be restored later.
	char oldDir[_MAX_PATH+1]; 
#ifdef WIN32
    GetCurrentDirectory(_MAX_PATH, oldDir); 
#else
	_getcwd(oldDir, _MAX_PATH);
#endif

	// Set the current dir to where we want to start looking.
#ifdef WIN32
	SetCurrentDirectory(m_SignatureDir);
#else
	_chdir(m_SignatureDir);
#endif

	// Collect name of all files and add to list
#ifdef COMMERCIAL

	CString flt = "*." + CRString(IDS_SIGNATURE_EXTENSION);
	WIN32_FIND_DATA	wfd;
	HANDLE findHandle = FindFirstFile(flt, &wfd); 

	while (findHandle != HANDLE(INVALID_HANDLE_VALUE))
	{
		CString tmp = wfd.cFileName;

		AddSignature(tmp);

		if (!FindNextFile( findHandle, &wfd )) 
            break; 
	}
	if (findHandle != INVALID_HANDLE_VALUE)
		FindClose(findHandle);

#else

	AddSignature( CRString( IDS_STANDARD_SIGNATURE ) );
	#ifdef WIN32
		AddSignature( CRString( IDS_ALTERNATE_SIGNATURE32 ) );
	#else
		AddSignature( CRString( IDS_ALTERNATE_SIGNATURE16 ) );
	#endif

#endif // COMMERCIAL

#ifdef WIN32
	SetCurrentDirectory(oldDir);
#else
	_chdir(oldDir);
#endif
}

CSignatureManager::~CSignatureManager()
{
	SigList *temp1, *temp2;
	SigStruct **temp3, *temp4;

	temp1 = m_SignatureList;

	while (temp1)
	{
		temp2 = temp1;
		temp1 = temp1->next;
		delete temp2;
	}

	if (temp3 = m_SignatureArray)
	{
		while (*temp3)
		{
			temp4 = *temp3++;
			delete temp4;
		}

		delete [] m_SignatureArray;
	}
}


int CSignatureManager::AddSignature(CString sigfilename)
{
	SigList *curr, *temp, *prev;
	CString signame = sigfilename, ext;
	const char *cp;
	int cmp;
	int idh = 0;
	int idl = 0;

	ext = "." + CRString(IDS_SIGNATURE_EXTENSION);
	if (!ext.CompareNoCase(signame.Right(4)))
		signame = signame.Left(signame.GetLength() - 4);
	else
		sigfilename += ext;

	cp = (const char *)signame;
	
	if (!*cp)
		return(1);	// Error, filename is empty

	while (*cp)		// Simple checksum to generate a unique ID
	{
		idh += toupper(*cp);
		idl ^= toupper(*cp);
		cp++;
	}
	idh = (idh << 8) + (idl & 0xff);

	if (m_SignatureList == NULL)
	{
		m_SignatureList = new SigList;
		m_SignatureList->next = NULL;
		m_SignatureList->SigName = signame;
		m_SignatureList->ID = idh;
	}
	else
	{
		curr = m_SignatureList;
		prev = m_SignatureList;
		while (curr)
		{
			if ((cmp = curr->SigName.CompareNoCase(signame)) > 0)
			{
				temp = new SigList;
				temp->next = curr;
				temp->SigName = signame;
				temp->ID = idh;
				if (curr == m_SignatureList)
					m_SignatureList = temp;
				else
					prev->next = temp;
				break;
			}
			else if (curr->next == NULL)
			{
				temp = new SigList;
				temp->next = NULL;
				temp->SigName = signame;
				temp->ID = idh;
				curr->next = temp;
				break;
			}
			else if (cmp == 0)
				return (1);		// Error, duplicate found

			prev = curr;
			curr = curr->next;
		}
	}

	m_changed = 1;
	m_SignatureCount++;
	return (0);
}

int CSignatureManager::DeleteSignature(CString sigfilename)
{
	SigList	*pCurrent;
	SigList	*pPrev;
	CString ext;
	CString	signame = sigfilename;

	ext = "." + CRString(IDS_SIGNATURE_EXTENSION);
	if (!ext.CompareNoCase(signame.Right(4)))
		signame = signame.Left(signame.GetLength() - 4);
	else
		sigfilename += ext;

	pPrev = NULL;
	pCurrent = m_SignatureList;

	while( ( pCurrent != NULL ) && stricmp( pCurrent->SigName, signame ) )
	{
		pPrev = pCurrent;
		pCurrent = pCurrent->next;
	}

	if ( pCurrent != NULL )
	{
		// we found it

		// first, remove it from the list

		if ( pPrev != NULL )
		{
			pPrev->next = pCurrent->next;
		}
		else
		{
			// it's the first thing in the list -- set the head node
			m_SignatureList = pCurrent->next;
		}

		// delete the object 
		delete pCurrent;	
		
		// decrement the count	
		m_SignatureCount--;
		m_changed = 1;
			
		// Save the current dir so that it can be restored later.
		char oldDir[_MAX_PATH+1]; 
#ifdef WIN32
		GetCurrentDirectory(_MAX_PATH, oldDir); 
#else
		_getcwd(oldDir, _MAX_PATH);
#endif
		// Set the current dir to where we want to start looking.
#ifdef WIN32
		SetCurrentDirectory(m_SignatureDir);
#else
		_chdir(m_SignatureDir);
#endif
		_unlink(sigfilename);
#ifdef WIN32
		SetCurrentDirectory(oldDir);
#else
		_chdir(oldDir);
#endif
		return (0);
	}

	return (1);	// Not found
}


const SigStruct **
CSignatureManager::GetSignatureList()
{
	SigList *listptr;
	SigStruct **arrayptr;

	if (m_changed)
	{
		if (m_SignatureArray)
		{
			SigStruct **temp = m_SignatureArray, *temp2;

			while (*temp)
			{
				temp2 = *temp++;
				delete temp2;
			}

			delete m_SignatureArray;
		}

		int index = 0;

		listptr = m_SignatureList;
		arrayptr = m_SignatureArray = new SigStruct *[m_SignatureCount + 1];
		while (listptr)
		{
			*arrayptr = new SigStruct;
			(*arrayptr)->SigName = listptr->SigName;
			(*arrayptr)->ID = listptr->ID;
			listptr = listptr->next;
			arrayptr++;
		}
		(*arrayptr) = NULL;

		m_changed = 0;
	}
		return((const SigStruct **)m_SignatureArray);
}

void CSignatureManager::MigrateSignatures( void )
{
	CString		szSrcFileName;
	CString		szDestFileName;
	BOOL		bCopied;
	CRString	szExt(IDS_SIGNATURE_EXTENSION);
	JJFile		fileNew;

	bCopied = FALSE;

	if ( GetIniShort( IDS_INI_USER_SIGNATURES ) == 0 )
	{
		// check for signatur.pce
		szSrcFileName = EudoraDir + CRString( IDS_SIG_FILENAME_FIRST );

		// build the standard signature filename
		szDestFileName = m_SignatureDir + "\\" + CRString( IDS_STANDARD_SIGNATURE );
		szDestFileName += "." + szExt;

		if ( ::FileExistsMT( szSrcFileName ) )
		{
			// copy the default file
			QCCopyFile( szSrcFileName, szDestFileName, TRUE );
		}
		else if ( !::FileExistsMT( szDestFileName ) )
		{
			// create a blank standard signature
			fileNew.Open( szDestFileName, O_CREAT );
			fileNew.Close();
		}

		// check for signatur.alt
		szSrcFileName = EudoraDir + CRString( IDS_SIG_FILENAME_FIRST + 1 );
		if ( ::FileExistsMT( szSrcFileName ) )
		{
			// copy the default file
#ifdef WIN32
			szDestFileName = m_SignatureDir + "\\" + CRString( IDS_ALTERNATE_SIGNATURE32 );
			
#else
			szDestFileName = m_SignatureDir + "\\" + CRString( IDS_ALTERNATE_SIGNATURE16 );
#endif
			szDestFileName += "." + szExt;
			QCCopyFile( szSrcFileName, szDestFileName, TRUE );
		}

		SetIniShort( IDS_INI_USER_SIGNATURES, 1);
	}
}

// NewSignature.cpp : implementation file
//

/////////////////////////////////////////////////////////////////////////////
// CNewSignature dialog


CNewSignature::CNewSignature(CWnd* pParent /*=NULL*/)
	: CDialog(CNewSignature::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewSignature)
	m_NewSigName = _T("");
	//}}AFX_DATA_INIT
}


void CNewSignature::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewSignature)
	DDX_Text(pDX, IDC_TOOL_SIGNATURE_NEW, m_NewSigName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewSignature, CDialog)
	//{{AFX_MSG_MAP(CNewSignature)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewSignature message handlers

