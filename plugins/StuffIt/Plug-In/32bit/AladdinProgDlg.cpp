// AladdinProgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Stuff32.h"
#include "AladdinProgDlg.h"

/* The keyword 'bool' is used throughout the Stuffit engine, so let's ignore the warning. */
#pragma warning(disable:4237) /* warning C4237: nonstandard extension used : 'bool' keyword is reserved for future use */

#include "Engine.h"
using namespace StuffItEngine;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAladdinProgDlg dialog


CAladdinProgDlg::CAladdinProgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAladdinProgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAladdinProgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

//	m_filelist = fileList;
//	m_writer = writer;
}


void CAladdinProgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAladdinProgDlg)
	DDX_Control(pDX, IDC_MAIN_PROGRESS, m_progress_ctl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAladdinProgDlg, CDialog)
	//{{AFX_MSG_MAP(CAladdinProgDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAladdinProgDlg message handlers

CProgressCtrl *g_progress_ctl = NULL;
uint32 g_max_size = 0;

/* ----- Call the plug-ins configuration Interface ------------------------- */

extern "C" bool __cdecl ProgressChangeWriter(uint32 size, archiveWriter writer)
{
	if ((g_progress_ctl) && (g_max_size > 0))
		g_progress_ctl->SetPos((int) (((double)size/g_max_size) * 100) );

	return (TRUE);
}

extern "C" void __cdecl ProgressBeginWriter(uint32 size, archiveWriter writer)
{
	g_max_size = size;
}

extern "C" void __cdecl ProgressEndWriter(archiveWriter writer)
{
	if (g_progress_ctl)
		g_progress_ctl->SetPos((int) 100);
}

// ----------

extern "C" bool __cdecl ProgressChangeReader(uint32 size, archiveReader reader)
{
	if ((g_progress_ctl) && (g_max_size > 0))
		g_progress_ctl->SetPos((int) (((double)size/g_max_size) * 100) );

	return (TRUE);
}

extern "C" void __cdecl ProgressBeginReader(uint32 size, archiveReader reader)
{
	g_max_size = size;
}

extern "C" void __cdecl ProgressEndReader(archiveReader reader)
{
	if (g_progress_ctl)
		g_progress_ctl->SetPos((int) 100);
}

// ----------

void CAladdinProgDlg::DoCreateArchive(const char** fileList, archiveWriter writer)
{
	CWaitCursor waiting;

	setWriterProgressSizeMove(ProgressChangeWriter, writer);
	setWriterProgressSizeBegin(ProgressBeginWriter, writer);
	setWriterProgressSizeEnd(ProgressEndWriter, writer);
	
	m_progress_ctl.SetRange( 0, 100 );
	m_progress_ctl.SetStep( 2 );

	g_progress_ctl = &m_progress_ctl;
	bool bSuccess = createArchive(fileList, writer); // Do the work
    ASSERT(bSuccess);
}

bool CAladdinProgDlg::DoExpandArchive(archiveReader reader)
{
	CWaitCursor waiting;

	setReaderProgressSizeMove(ProgressChangeReader, reader);
	setReaderProgressSizeBegin(ProgressBeginReader, reader);
	setReaderProgressSizeEnd(ProgressEndReader, reader);
	
	m_progress_ctl.SetRange( 0, 100 );
	m_progress_ctl.SetStep( 2 );

	g_progress_ctl = &m_progress_ctl;

	bool bSuccess = decodeArchive(reader); // Do the work
    return (bSuccess);
}

