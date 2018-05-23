// status.cpp : implementation file
//

#include "stdafx.h"
#include "swmapi.h"
#include "status.h"
#include "mapiinst.h"

#ifndef WIN32
#include <sys/types.h>
#include <ver.h>
#endif // WIN32

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg dialog


CStatusDlg::CStatusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStatusDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStatusDlg)
	m_Description16 = "";
	m_Description32 = "";
	m_Company16 = "";
	m_Company32 = "";
	m_Directory = "";
	m_DllName16 = "";
	m_DllName32 = "";
	m_Version16 = "";
	m_Version32 = "";
	m_InUse16 = "";
	m_InUse32 = "";
	m_FileInfo16 = "";
	m_FileInfo32 = "";
	//}}AFX_DATA_INIT
}

void CStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStatusDlg)
	DDX_Text(pDX, IDST_DESCRIPTION16, m_Description16);
	DDX_Text(pDX, IDST_DESCRIPTION32, m_Description32);
	DDX_Text(pDX, IDST_COMPANY16, m_Company16);
	DDX_Text(pDX, IDST_COMPANY32, m_Company32);
	DDX_Text(pDX, IDST_DIRECTORY, m_Directory);
	DDX_Text(pDX, IDST_DLLNAME16, m_DllName16);
	DDX_Text(pDX, IDST_DLLNAME32, m_DllName32);
	DDX_Text(pDX, IDST_VERSION16, m_Version16);
	DDX_Text(pDX, IDST_VERSION32, m_Version32);
	DDX_Text(pDX, IDST_IN_USE_16, m_InUse16);
	DDX_Text(pDX, IDST_IN_USE_32, m_InUse32);
	DDX_Text(pDX, IDST_FILEINFO16, m_FileInfo16);
	DDX_Text(pDX, IDST_FILEINFO32, m_FileInfo32);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CStatusDlg, CDialog)
	//{{AFX_MSG_MAP(CStatusDlg)
	ON_BN_CLICKED(IDRB_EUDORA, OnRadioButtonClicked)
	ON_BN_CLICKED(IDRB_WINDOWS, OnRadioButtonClicked)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CStatusDlg message handlers

////////////////////////////////////////////////////////////////////////
// OnInitDialog
//
////////////////////////////////////////////////////////////////////////
BOOL CStatusDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CButton* p_button = (CButton *) GetDlgItem(IDRB_WINDOWS);
	ASSERT(p_button);

	p_button->SetCheck(1);
	OnRadioButtonClicked();		// FORNOW, need to call this explicitly?

	return TRUE;  // return TRUE  unless you set the focus to a control
}


////////////////////////////////////////////////////////////////////////
// OnRefresh
//
////////////////////////////////////////////////////////////////////////
void CStatusDlg::OnRefresh()
{
	OnRadioButtonClicked();
}


////////////////////////////////////////////////////////////////////////
// OnRadioButtonClicked
//
////////////////////////////////////////////////////////////////////////
void CStatusDlg::OnRadioButtonClicked()
{
	m_Directory.Empty();
	m_Description16.Empty();
	m_Description32.Empty();
	m_DllName16.Empty();
	m_DllName32.Empty();
	m_Company16.Empty();
	m_Company32.Empty();
	m_FileInfo16.Empty();
	m_FileInfo32.Empty();
	m_Version16.Empty();
	m_Version32.Empty();
	m_InUse16.LoadString(IDS_FILE_NOT_FOUND);
	m_InUse32.LoadString(IDS_FILE_NOT_FOUND);

	CButton* p_windows_button = (CButton *) GetDlgItem(IDRB_WINDOWS);
	ASSERT(p_windows_button);
	CButton* p_eudora_button = (CButton *) GetDlgItem(IDRB_EUDORA);
	ASSERT(p_eudora_button);
	
	if (p_windows_button->GetCheck() == 1)
	{
		if (GetSystemDirectory(m_Directory.GetBuffer(_MAX_PATH), _MAX_PATH))
		{
			m_Directory.ReleaseBuffer();

#ifndef WIN32
			//
			// Okay, for 16-bit code running under WinNT 3.51, the
			// GetSystemDirectory() call for some reason returns
			// "c:\winnt\system" rather than "c:\winnt\system32" as
			// you might expect.  If left unchanged, this means that
			// we would be installing the Eudora DLL in the WRONG
			// directory since Win16 apps under NT actually look for
			// DLLs only in the "system32" directory and ignore any
			// DLLs in the "system" directory.  Therefore, let's do a
			// very crude hack to see if we can detect this situation.
			// BTW, the ::GetVersion() routine is no help since Win NT
			// 3.51 reports a Windows version of 3.10 when running
			// 16-bit code.
			//
			if (FileExists(m_Directory + "32"))
			{
				m_Directory += "32";		// brutal hack for Win NT
			}
#endif
			if (m_Directory[m_Directory.GetLength() - 1] != '\\')
				m_Directory += "\\";	// make sure there's a trailing slash
		}
	    else
	    	m_Directory.ReleaseBuffer();
		m_DllName16 = "MAPI.DLL";
		m_DllName32 = "MAPI32.DLL";
	}
	else if (p_eudora_button->GetCheck() == 1)
	{
		if (GetModuleFileName(AfxGetInstanceHandle(), m_Directory.GetBuffer(_MAX_PATH), _MAX_PATH))
		{
			m_Directory.ReleaseBuffer();
			int idx = m_Directory.ReverseFind('\\');
			if (idx != -1)
				m_Directory = m_Directory.Left(idx + 1);		// truncate the filename part
	    }
	    else
	    	m_Directory.ReleaseBuffer();
		m_DllName16 = "EUMAPI.DLL";
		m_DllName32 = "EUMAPI32.DLL";
	}
	else
	{
		ASSERT(0);
		return;
	}

	if (FileExists(m_Directory + m_DllName16))
	{
		m_FileInfo16 = GetFileInfo(m_Directory + m_DllName16);

		GetVersionInfo(m_Directory + m_DllName16, m_Company16, m_Version16, m_Description16);

		if (CMapiInstaller::IsDLLInUse(m_Directory + m_DllName16))
			m_InUse16.LoadString(IDS_IN_USE);
		else
			m_InUse16.LoadString(IDS_NOT_IN_USE);
	}

	if (FileExists(m_Directory + m_DllName32))
	{
		m_FileInfo32 = GetFileInfo(m_Directory + m_DllName32);

		GetVersionInfo(m_Directory + m_DllName32, m_Company32, m_Version32, m_Description32);

		if (CMapiInstaller::IsDLLInUse(m_Directory + m_DllName32))
			m_InUse32.LoadString(IDS_IN_USE);
		else
			m_InUse32.LoadString(IDS_NOT_IN_USE);
	}

	//
	// Display strings on dialog fields.
	//
	UpdateData(FALSE);
}



////////////////////////////////////////////////////////////////////////
// GetFileInfo
//
// Given a filename, return a string containing timestamp and file size
// info.
////////////////////////////////////////////////////////////////////////
CString CStatusDlg::GetFileInfo(
	const CString& fileName)	//(i) pathname to the file
{
	CFileStatus status;
	if (CFile::GetStatus(fileName, status))
	{
		//
		// Print file size (in bytes) and last modified time to string.
		//
		CString info;			// returned
		CString bytes;
		bytes.LoadString(IDS_BYTES);
		info.Format("%ld %s, %s", status.m_size, 
								  (const char *) bytes, 
								  (const char *) (status.m_mtime.Format("%c")));

		//
		// Check for read-only file.
		//
		if (status.m_attribute & CFile::readOnly)
		{
			CString read_only;
			read_only.LoadString(IDS_READ_ONLY);
			info += ", ";
			info += read_only;
		}
		return info;
	}
	else
	{
		return CString("");
	}
}


////////////////////////////////////////////////////////////////////////
// GetVersionInfo
//
////////////////////////////////////////////////////////////////////////
void CStatusDlg::GetVersionInfo(
	const CString& fileName,	//(i) pathname to the file
	CString& companyName,		//(o) company name
	CString& fileVersion,		//(o) file version number
	CString& fileDescription)	//(o) file description
{
	ASSERT(FileExists(fileName));

	//
	// Start with a clean slate.
	//
	companyName.Empty();
	fileVersion.Empty();
	fileDescription.Empty();

	//
	// Do setup to prepare for the incoming version data.
	//
	DWORD handle = 0;
	DWORD size = ::GetFileVersionInfoSize((char *)(const char *) fileName, &handle);
	if (NULL == size)
		return;		// ERROR: file doesn't have VERSIONINFO data?

	//
	// Allocate data buffers of the proper sizes.
	//
	LPSTR p_data = DEBUG_NEW_NOTHROW char[size];
	if (NULL == p_data)
		return;		// ERROR: out of memory

	//
	// Now, fetch the actual VERSIONINFO data.
	//
	if (! ::GetFileVersionInfo((char *)(const char *) fileName, handle, size, p_data))
	{
		ASSERT(0);		// missing VERSIONINFO data?
		delete p_data;
		return;
	}

	//
	// Fetch CompanyName string.
	//
	LPSTR p_buf = NULL;
	UINT bufsize = 0;

	//
	// Okay, you would think that passing a raw string literal to an
	// argument that takes a string pointer would be okay.  Well, not
	// if you're running the 16-bit VerQueryValue() function under
	// Windows 3.1/32s.  It GPFs if you pass it a string literal.  So,
	// the workaround is to copy the string to a local buffer first,
	// then pass the string in the buffer.  This, for some inexplicable
	// reason, works under all Windows OSes.
	//
	const char* FILEINFOPATH = "\\StringFileInfo\\040904E4\\";
	char kludgestr[_MAX_PATH];
	strcpy(kludgestr, FILEINFOPATH);
	strcat(kludgestr, "CompanyName");
	if (::VerQueryValue(p_data,
						kludgestr, 
						(void **) &p_buf, 
						&bufsize))
	{
		if (p_buf && bufsize)
			companyName = p_buf;
	}
	else
	{
		ASSERT(0);			// required CompanyName string missing
	}

	//
	// Fetch FileVersion string.
	//
	p_buf = NULL;
	bufsize = 0;

	strcpy(kludgestr, FILEINFOPATH);
	strcat(kludgestr, "FileVersion");
	if (::VerQueryValue(p_data,
						kludgestr, 
						(void **) &p_buf, 
						&bufsize))
	{
		if (p_buf && bufsize)
			fileVersion = p_buf;
	}
	else
	{
		ASSERT(0);			// required FileVersion string missing
	}

	//
	// Fetch FileDescription string.
	//
	p_buf = NULL;
	bufsize = 0;

	strcpy(kludgestr, FILEINFOPATH);
	strcat(kludgestr, "FileDescription");
	if (::VerQueryValue(p_data,
						kludgestr, 
						(void **) &p_buf, 
						&bufsize))
	{
		if (p_buf && bufsize)
			fileDescription = p_buf;
	}
	else
	{
		ASSERT(0);			// required FileDescription string missing
	}

	//
	// All roads should lead here...
	//
	delete p_data;
}


////////////////////////////////////////////////////////////////////////
// FileExists [private]
//
////////////////////////////////////////////////////////////////////////
BOOL CStatusDlg::FileExists(const CString& fileName) const
{
	CFileStatus unused;

	if (CFile::GetStatus(fileName, unused) == 0)
		return FALSE;

	return TRUE;
}

