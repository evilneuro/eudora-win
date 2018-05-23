// RecordDoc.cpp : implementation of the CQVoiceDoc class
//

#include "DebugNewHelpers.h"

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#include "QVoice.h"
#include "QVoiceView.h"
#include "LsStateEvent.h"
#include "LsStateMachine.h"
#include "LsStateAction.h"
//#include "SaveCompressDlg.h"

// LOCAL INCLUDES
#include "QVoiceDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CQVoiceDoc

IMPLEMENT_DYNCREATE(CQVoiceDoc, CDocument)

BEGIN_MESSAGE_MAP(CQVoiceDoc, CDocument)
	//{{AFX_MSG_MAP(CQVoiceDoc)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CQVoiceDoc::CQVoiceDoc() : mb_OneTime(TRUE)
{
	BIND_EVENT_STATE_ACTION( m_Event, m_State, m_Action, this );
	SetModifiedFlag( FALSE );	// set document as "clean"
}

CQVoiceDoc::~CQVoiceDoc()
{
}

BOOL CQVoiceDoc::OnNewDocument()
{
/*
	if ( mb_OneTime )
	 {
		mb_OneTime = FALSE;
        if ( !m_Event.Event( LsStateEvent::INITIAL ) )
			return FALSE; // quit app
	 }
*/
	if ( !CDocument::OnNewDocument() )
		return FALSE;

	if ( ::IsWindow(AfxGetMainWnd()->GetSafeHwnd()) )
		AfxGetMainWnd()->SetWindowText( AfxGetAppName() );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CQVoiceDoc serialization

void CQVoiceDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

/*inline*/ LsStateAction* CQVoiceDoc::StateAction()
{
	return &m_Action;
}

/*inline*/ LsStateEvent* CQVoiceDoc::StateEvent()
{
	return &m_Event;
}

//////////////////////////////////////////////////////////////////////////
// File Stuff

BOOL CQVoiceDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	// commandline?
	//if (!((CQVoiceApp *)AfxGetApp())->ms_CmdLineInFileName.IsEmpty())
	//	return TRUE;
/*
	if ( mb_OneTime )
	 {
		mb_OneTime = FALSE;
        if ( !m_Event.Event( LsStateEvent::INITIAL ) )
			FALSE; // quit app
	 }
*/
	// commandline?
	if (!((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.m_strFileName.IsEmpty())
	 {
		m_Action.SearchPlugins();
		return m_Action.CheckFile(CString(lpszPathName));
	 }

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	BOOL bRet = m_Event.Event( LsStateEvent::OPENFILE, (void *)lpszPathName );

	if ( bRet )
	 {
		CString fn = lpszPathName;
		// strip path name and extension
		if ( fn.ReverseFind( '\\' ) > -1 )
			fn = fn.Right( fn.GetLength() - fn.ReverseFind( '\\' )-1 );
		if ( fn.ReverseFind( '.' ) > -1 )
			fn = fn.Left(fn.ReverseFind( '.' ));
		AfxGetMainWnd()->SetWindowText( fn );
		((CMainFrame*)AfxGetMainWnd())->m_wndNameBar.Name( fn );
	 }

	return bRet;
}

void CQVoiceDoc::OnFileOpen() 
{
	CString FileName;

	if ( !SaveModified() )
		return; // cancel open since we don't want to lose current recording

	if (!DoPromptFileName(FileName, _T("Open"), OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE))
		return; // open cancelled

	OnOpenDocument(LPCTSTR(FileName));
}

BOOL CQVoiceDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	BOOL bRet = m_Event.Event( LsStateEvent::SAVEAS, (void *)lpszPathName );

	if ( bRet )
	 {
		CString fn = lpszPathName;
		// strip path name and extension
		if ( fn.ReverseFind( '\\' ) > -1 )
			fn = fn.Right( fn.GetLength() - fn.ReverseFind( '\\' )-1 );
		if ( fn.ReverseFind( '.' ) > -1 )
			fn = fn.Left(fn.ReverseFind( '.' ));
		AfxGetMainWnd()->SetWindowText( fn );
		((CMainFrame*)AfxGetMainWnd())->m_wndNameBar.Name( fn );		
	 }

	return bRet;
}

void CQVoiceDoc::OnFileSaveAs() 
{
	CString FileName = ((CMainFrame*)AfxGetMainWnd())->m_wndNameBar.Name();
	if ( FileName.IsEmpty() )
		FileName = _T("Message");
/*
	if ( m_Action.PlayMode() == LsStateAction::REALTIME )
		FileName += _T(".qcp");
	else
		FileName += _T(".wav");
	int ret = IDNO;
*/
//	while ( IDYES != ret )
//	 {
		if ( FileName.ReverseFind('.') == -1 )
		 {
			if ( m_Action.PlayMode() == LsStateAction::REALTIME )
				FileName += _T(".qcp");
			else
				FileName += _T(".wav");
		 }

//		CSaveCompressDlg save( &m_Action.m_PlugIn );
//		save.DoModal();

		if (!DoPromptFileName(FileName, _T("Save As"),
				OFN_HIDEREADONLY | OFN_PATHMUSTEXIST , FALSE))
			return; // 'save as' cancelled

		if ( FileName.ReverseFind('.') == -1 )
		 {
			if ( m_Action.PlayMode() == LsStateAction::REALTIME )
				FileName += _T(".qcp");
			else
				FileName += _T(".wav");
		 }

//		CCompressDialog convert;
//		convert.Init( pCodec, m_Action.WorkFileName(), );
//		if ( convert.DoModal() != IDOK )
//			return;
/*
		FILE* fp;
		if ( fp = fopen( LPCTSTR(FileName), "r" ) )
		 {
			fclose( fp );
			ret = AfxMessageBox( 
					"This file already exists. Do you want to replace it?",
					MB_YESNO );
		 }
		else
			ret = IDYES;
	 }		
*/	 
	OnSaveDocument(LPCTSTR(FileName));
}

//
BOOL CQVoiceDoc::DoPromptFileName(CString& fileName, LPCTSTR sTitle, DWORD lFlags, BOOL bOpenFileDialog )
{
	CFileDialog dlgFile(bOpenFileDialog);

	CString title = sTitle;

	dlgFile.m_ofn.Flags |= lFlags;

	CString strFilter;

	if ( (!bOpenFileDialog && 
		 (m_Action.PlayMode() == LsStateAction::REALTIME)) ||
		  bOpenFileDialog )
	{
		// "Qualcomm PureVoice Files (*.qcp)" 
		strFilter += _T("QUALCOMM PureVoice Files (*.qcp)");
		strFilter += (TCHAR)'\0';
		strFilter += _T("*.qcp");
		strFilter += (TCHAR)'\0';
	//	if ( !bOpenFileDialog && isEncodeDone())
	//		dlgFile.m_ofn.nFilterIndex = 2;
	}

	if ( (!bOpenFileDialog && 
		 (m_Action.PlayMode() != LsStateAction::REALTIME)) ||
		  bOpenFileDialog )
	{
		// "8 KHz (16 bits) PCM Files (*.wav)" 
		strFilter += _T("8 KHz (16 bits) PCM Files (*.wav)");
		strFilter += (TCHAR)'\0';
		strFilter += _T("*.wav");
		strFilter += (TCHAR)'\0';
	}

	// "All Files (*.*)"
	strFilter += _T("All Files (*.*)");
	strFilter += (TCHAR)'\0';
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0';

	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	BOOL bResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	fileName.ReleaseBuffer();

	return bResult;
}

//////////////////////////////////////////////////////////////////////////////

BOOL CQVoiceDoc::SaveModified()
{
#ifndef LS_PLAYER_ONLY
	if (!IsModified())
		return TRUE;        // ok to exit

	if ( m_State.State() != LsStateMachine::END )
	{
	switch ( AfxMessageBox( LS_STR_DISCARD,	MB_YESNO ) )
	 {
		case IDYES:
		 return TRUE;

		case IDNO:
		 return FALSE;       // don't continue

		default:
			ASSERT(FALSE);
		 break;
	 }
	}
	else
	{
	switch ( AfxMessageBox( LS_STR_ATTACH_QUIT, MB_YESNOCANCEL ) )
	 {
		case IDYES:			// exit after attach
			return m_Event.Event( LsStateEvent::ATTACH );
//		 return TRUE;
		
		case IDNO:			// exit without attach
		 return TRUE;

		case IDCANCEL:
			m_Event.Event( LsStateEvent::STOP );
		 return FALSE;       // cancel exit

		default:
			ASSERT(FALSE);
		 break;
	 }
	}
#endif
	return TRUE;
}

BOOL CQVoiceDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	m_Event.Event( LsStateEvent::END );
    return CDocument::CanCloseFrame( pFrame );
}

/////////////////////////////////////////////////////////////////////////////
// CQVoiceDoc diagnostics

#ifdef _DEBUG
void CQVoiceDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CQVoiceDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

void CQVoiceDoc::OnAppExit() 
{
	AfxGetMainWnd()->PostMessage( WM_CLOSE );
}
