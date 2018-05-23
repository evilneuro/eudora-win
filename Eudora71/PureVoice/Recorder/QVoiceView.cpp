// QVoiceView.cpp : implementation of the CQVoiceView class
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "QVoice.h"
#include "lscommon.h"

// LOCAL INCLUDES
#include "QVoiceView.h"
#ifdef LS_DEBUG0
	#include "LsDebug.h"
#endif

// LOCAL DEFINES
#include "DebugNewHelpers.h"

#define STATE_INITIAL		LsStateMachine::INITIAL
#define STATE_PLAY			LsStateMachine::PLAY
#define STATE_RECORD		LsStateMachine::RECORD
#define STATE_PAUSE			LsStateMachine::PAUSE
#define STATE_IDLE			LsStateMachine::IDLE
#define STATE_OPENFILE		LsStateMachine::OPENFILE
#define STATE_REPOSITION	LsStateMachine::REPOSITION
#define STATE_ENCODE		LsStateMachine::ENCODE
#define STATE_RECORDENCODE	LsStateMachine::RECORDENCODE
#define STATE_ATTACH		LsStateMachine::ATTACH
#define STATE_NEWRECORD		LsStateMachine::NEWRECORD
#define STATE_END			LsStateMachine::END

#define EVENT_INITIAL		LsStateEvent::INITIAL
#define EVENT_PLAY			LsStateEvent::PLAY
#define EVENT_RECORD		LsStateEvent::RECORD
#define EVENT_STOP			LsStateEvent::STOP
#define EVENT_PAUSE			LsStateEvent::PAUSE
#define EVENT_REPOSITION	LsStateEvent::REPOSITION
#define EVENT_ENCODE		LsStateEvent::ENCODE
#define EVENT_ATTACH		LsStateEvent::ATTACH
#define EVENT_OPENFILE		LsStateEvent::OPENFILE
#define EVENT_PLAYEND		LsStateEvent::PLAYEND
#define EVENT_NEW			LsStateEvent::NEW

#define START_RECORD_TIMER	0xFF

#define MAP_SLIDER_POS_2_TIME_MS(p) \
			long( float(m_curTotTime)*p/m_slider.GetRangeMax() )
#define MAP_TIME_MS_2_SLIDER_POS(t) \
			int( float(t)*m_slider.GetRangeMax()/m_curTotTime )

#ifndef LS_PLAYER_ONLY
	#define FORMVIEW_W  240
	#define FORMVIEW_H  130
#else
	#define FORMVIEW_W  210
	#define FORMVIEW_H  130
#endif
/////////////////////////////////////////////////////////////////////////////
// CQVoiceView

IMPLEMENT_DYNCREATE(CQVoiceView, CFormView)

BEGIN_MESSAGE_MAP(CQVoiceView, CFormView)
	//{{AFX_MSG_MAP(CQVoiceView)
	ON_MESSAGE(MM_WIM_DATA, OnWaveInData)
	ON_MESSAGE(MM_WOM_DONE, OnWaveOutData)
	ON_MESSAGE(MM_WOM_CLOSE, OnWaveOutClose)
	ON_BN_CLICKED(IDC_BTN_PLAY, OnBtnPlay)
	ON_BN_CLICKED(IDC_BTN_RECORD, OnBtnRecord)
	ON_BN_CLICKED(IDC_BTN_PAUSE, OnBtnPause)
	ON_BN_CLICKED(IDC_BTN_STOP, OnBtnStop)
	ON_BN_CLICKED(IDC_BTN_FASTR, OnBtnFastR)
	ON_BN_CLICKED(IDC_BTN_FASTF, OnBtnFastF)
	ON_BN_CLICKED(IDC_BTN_SPDU, OnBtnVolU)
	ON_BN_CLICKED(IDC_BTN_SPDD, OnBtnVolD)
	ON_BN_CLICKED(IDC_BTN_ATTACH, OnBtnAttach)
	ON_UPDATE_COMMAND_UI(IDC_BTN_PLAY, OnUpdatePlay)
	ON_UPDATE_COMMAND_UI(IDC_BTN_RECORD, OnUpdateRecord)
	ON_UPDATE_COMMAND_UI(IDC_BTN_STOP, OnUpdateStop)
	ON_UPDATE_COMMAND_UI(IDC_BTN_PAUSE, OnUpdatePause)
	ON_UPDATE_COMMAND_UI(IDC_BTN_FASTR, OnUpdateFastR)
	ON_UPDATE_COMMAND_UI(IDC_BTN_FASTF, OnUpdateFastF)
	ON_UPDATE_COMMAND_UI(IDC_BTN_SPDU, OnUpdateVolU)
	ON_UPDATE_COMMAND_UI(IDC_BTN_SPDD, OnUpdateVolD)
	ON_UPDATE_COMMAND_UI(IDC_BTN_ATTACH, OnUpdateAttach)
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_COMMAND(IDT_NEW_RECORD, OnNewRecord)
	ON_COMMAND(IDT_EDIT_INSERT_MODE, OnEditInsertMode)
	ON_COMMAND(IDT_VOLUMEBAR, OnVolumebar)
	ON_UPDATE_COMMAND_UI(IDT_EDIT_INSERT_MODE, OnUpdateEditInsertMode)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_UPDATE_COMMAND_UI(IDT_NEW_RECORD, OnUpdateNewRecord)
	ON_UPDATE_COMMAND_UI(IDT_VOLUMEBAR, OnUpdateVolumebar)
	ON_CBN_SELCHANGE(IDC_CMB_CODEC, OnSelchangeCmbCodec)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify )
	ON_COMMAND(IDT_AGC, OnAgc)
	ON_UPDATE_COMMAND_UI(IDT_AGC, OnUpdateAgc)
	ON_COMMAND(IDA_ENCODE, OnEncode)
	//}}AFX_MSG_MAP
	// for CODEC dynamic menu
	ON_COMMAND_RANGE(ID_CODEC_DEFAULT, ID_CODEC_INSERT_END, OnCodecMenuRange)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CODEC_DEFAULT, ID_CODEC_INSERT_END, OnUpdateCodecMenuRange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQVoiceView construction/destruction

CQVoiceView::CQVoiceView()
	: CFormView(CQVoiceView::IDD)
{
	//{{AFX_DATA_INIT(CQVoiceView)
	//}}AFX_DATA_INIT

	mp_ProgressCtrl = 0;
	mp_MainFrame	= 0;

	m_timeIndex		= 0;
	m_animIndex		= 0;

	m_curTotTime	= 10000; // some arbit for now, say 10,000 ms

	m_startRecordTimerId = START_RECORD_TIMER;

	mb_OpenEnable	= TRUE;
	mb_SaveEnable	= FALSE;
	mb_NewEnable	= FALSE;
    mb_Pause        = FALSE;

	mb_OneTime		= TRUE;
}

CQVoiceView::~CQVoiceView()
{
}

void CQVoiceView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQVoiceView)
	DDX_Control(pDX, IDC_PLAY_PIC, m_playPic);
	DDX_Control(pDX, IDC_SLIDER, m_slider);
	DDX_Control(pDX, IDC_BTN_PLAY, m_btnPlay);
//#ifndef LS_PLAYER_ONLY
	DDX_Control(pDX, IDC_BTN_RECORD, m_btnRecord);
	DDX_Control(pDX, IDC_BTN_ATTACH, m_btnAttach);
//#endif
	DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BTN_FASTR, m_btnFastR);
	DDX_Control(pDX, IDC_BTN_FASTF, m_btnFastF);
	DDX_Control(pDX, IDC_BTN_SPDU, m_btnVolU);
	DDX_Control(pDX, IDC_BTN_SPDD, m_btnVolD);
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CQVoiceView

LRESULT CQVoiceView::OnWaveInData(WPARAM wParam, LPARAM lParam)
{
//	UpdateProgress();
	return mp_Action->_record(wParam, lParam);
}

LRESULT CQVoiceView::OnWaveOutData(WPARAM wParam, LPARAM lParam)
{
//	UpdateProgress();
	return mp_Action->_play(wParam, lParam);
}

LRESULT CQVoiceView::OnWaveOutClose(WPARAM wParam, LPARAM lParam)
{
	TRACE("From MM_WOM_CLOSE\n");
    mp_Action->FromMmWomClose(TRUE);
	if ( State() == STATE_PLAY )
	 {
		TRACE("Calling stop from MM_WOM_CLOSE\n");
		Event( EVENT_STOP );
	 }
	else
 	 {
//		Event( EVENT_STOP, );
	 }

	return 0L;
}

void CQVoiceView::OnCodecMenuRange(UINT nID) 
{
	mp_Action->OnCodecMenuRange(nID);
}

void CQVoiceView::OnUpdateCodecMenuRange(CCmdUI* pCmdUI) 
{
	mp_Action->OnUpdateCodecMenuRange(pCmdUI);
}

//////////////////////////////////////////////////////////////////////////////

LRESULT CQVoiceView::OnUserFileName(WPARAM wParam, LPARAM lParam)
{
	// A second instance of this program has sent us the handle of
	// read end of the anonymous pipe containing our file name
//	HANDLE hReadPipe = HANDLE(lParam);
//	DWORD bytesRead;
	char fileName[_MAX_PATH+50];

	// ok, now read the file name and close our anonymous pipe
//	::ReadFile( hReadPipe, fileName, _MAX_PATH, &bytesRead, NULL );
//	::CloseHandle( hReadPipe );
	strcpy( fileName, APP_ID_STR );
	strcat( fileName, "filename" );
	HANDLE hmmf = OpenFileMapping( FILE_MAP_READ, FALSE, fileName );
	if ( hmmf == NULL )
	 {
#ifdef LS_DEBUG0
		LsDebug err;
		strcat( fileName, "/ RX: OpenFileMapping Failed" );
		err.SystemErrorMsgBox(fileName);
#endif
		return 0L;
	 }
	LPCTSTR sharedbuf = (LPCTSTR)MapViewOfFile( hmmf, FILE_MAP_READ, 0, 0, 0 );
	if ( sharedbuf == NULL )
	 {
#ifdef LS_DEBUG0
		LsDebug err;
		err.SystemErrorMsgBox("RX: MapViewOfFile Failed");
#endif
		CloseHandle( hmmf );
		return 0L;

	 }
	lstrcpy( fileName, sharedbuf );
	BOOL err = UnmapViewOfFile( sharedbuf );
#ifdef LS_DEBUG0
	if ( !err )
	 {
		LsDebug err;
		err.SystemErrorMsgBox("RX: UnmapViewOfFile Failed");
	 }
#endif
	CloseHandle( hmmf );

	TRACE1("WM_USER_FILENAME received. File Name = %s\n", fileName );

	// stop play or record
	OnBtnStop();

	// first check with user if they wish to discard current recording
	// if appropriate and then proceed
	if ( GetDocument()->SaveModified() )
		GetDocument()->OnOpenDocument( fileName );

	return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// CQVoiceView diagnostics

#ifdef _DEBUG
void CQVoiceView::AssertValid() const
{
	CFormView::AssertValid();
}

void CQVoiceView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CQVoiceDoc* CQVoiceView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CQVoiceDoc)));
	return (CQVoiceDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
inline LsStateType CQVoiceView::State()
{
	return mp_Action->State();
}

inline LsStateType CQVoiceView::LastState()
{
	return mp_Action->LastState();
}

inline BOOL CQVoiceView::Event( int event, void* param )
{
	return mp_Event->Event( event, param );
}

BOOL CQVoiceView::UpdateView()
{
	switch( State() )
	 {
		case STATE_INITIAL:
		case STATE_NEWRECORD:
            disableCtrls( STATE_INITIAL );
            return TRUE;
        case STATE_REPOSITION:
            return TRUE;
		case STATE_IDLE:
            return OnIdleUpdate();
		case STATE_OPENFILE:
            return OnOpenFileUpdate();
		case STATE_PLAY:
            return OnPlayUpdate();
		case STATE_RECORD:
			return OnRecordUpdate();
        case STATE_PAUSE:
			return OnPauseUpdate();
		case STATE_ENCODE:
			return OnEncodeUpdate();
		case STATE_RECORDENCODE:
			return OnRecordEncodeUpdate();
	 }

	return FALSE;
}

BOOL CQVoiceView::OnPauseUpdate()
{
	switch( LastState() )
	 {
		case STATE_PLAY:
			m_btnPlay.EnableWindow();
		case STATE_RECORD:
#ifndef LS_PLAYER_ONLY
			m_btnRecord.EnableWindow();
#endif
            mb_Pause = TRUE;
            m_playPic.SetIndicator( CPlayDisplay::INDICATOR_PAUSE, mb_Pause );
			if ( STATE_RECORD == LastState() )
			 {
				m_slider.SetPos(
					MAP_TIME_MS_2_SLIDER_POS(mp_Action->LenCurMs()) );
			 }
         break;
//		case STATE_REPOSITION:
//		 break;
	 }
	EnableToolTips(TRUE);
    disableCtrls( State() );

	return TRUE;
}

BOOL CQVoiceView::OnPlayUpdate()
{
	switch( LastState() )
	 {
        case STATE_IDLE:
		case STATE_PAUSE:
		case STATE_OPENFILE:
            mb_Pause = FALSE;
			m_animIndex	= 0;
            m_playPic.SetIndicator( CPlayDisplay::INDICATOR_PAUSE, mb_Pause );
            m_playPic.SetIndicator( CPlayDisplay::INDICATOR_PLAY, TRUE );
         break;
//		case STATE_REPOSITION:
//		 break;
	 }

	// change control button states
	m_btnPlay.EnableWindow(FALSE);
#ifndef LS_PLAYER_ONLY
	m_btnRecord.EnableWindow(FALSE);
#endif
    m_btnPause.EnableWindow();
	m_slider.EnableWindow();
	m_fastTimerId = SetTimer( 2, 170, NULL );

    disableCtrls( State() );

	return TRUE;
}

BOOL CQVoiceView::OnRecordUpdate()
{
#ifndef LS_PLAYER_ONLY
	switch( LastState() )
	 {
        case STATE_INITIAL:
        case STATE_IDLE:
		case STATE_PAUSE:
            mb_Pause = FALSE;
            m_playPic.SetIndicator( CPlayDisplay::INDICATOR_PAUSE, mb_Pause );
            m_playPic.SetIndicator( CPlayDisplay::INDICATOR_RECORD, TRUE );
			m_playPic.SetRate( mp_Action->Rate() );
            m_playPic.SetSize( 0 );
			if ( mp_Action->RecordMode() == LsStateAction::REALTIME )
				m_playPic.SetIndicator( CPlayDisplay::INDICATOR_COMPRESS,
																	TRUE );
         break;
	 }

	// change control button states
	m_btnRecord.EnableWindow(FALSE);
	m_btnPause.EnableWindow();
	m_slider.EnableWindow(FALSE);
	m_btnPlay.EnableWindow(FALSE);
	m_btnFastR.EnableWindow(FALSE);
    m_btnFastF.EnableWindow(FALSE);
	m_btnVolU.EnableWindow(FALSE);
	m_btnVolD.EnableWindow(FALSE);
	m_fastTimerId = SetTimer( 2, 200, NULL );
	EnableToolTips(FALSE);
    disableCtrls( State() );
#endif
	return TRUE;
}

BOOL CQVoiceView::OnRecordEncodeUpdate()
{
	// LastState() == STATE_RECORD
#ifndef LS_PLAYER_ONLY
//	mp_ProgressCtrl = mp_MainFrame->m_wndStatusBar.SetUpProgress();
	mp_MainFrame->m_wndStatusBar.SetPaneText(0, "Converting: 0% Done");
//	mp_ProgressCtrl->SetRange( 0, 100 );
//	mp_ProgressCtrl->SetPos( 0 );
//	mp_ProgressCtrl->SetStep( 5 );
	m_slowTimerId = SetTimer( 1, 1000, NULL );
	m_playPic.SetIndicator( CPlayDisplay::INDICATOR_RECORD, FALSE );
	m_playPic.SetMeter( 0 );

	m_btnPause.EnableWindow(FALSE);
    disableCtrls( State() );
#endif
	return TRUE;
}

BOOL CQVoiceView::OnEncodeUpdate()
{
#ifndef LS_PLAYER_ONLY
	switch( LastState() )
	 {
        case STATE_IDLE:
//			mp_ProgressCtrl = mp_MainFrame->m_wndStatusBar.SetUpProgress();
			mp_MainFrame->m_wndStatusBar.SetPaneText(0, "Converting: 0% Done");
//			mp_ProgressCtrl->SetRange( 0, 100 );
//			mp_ProgressCtrl->SetPos( 0 );
//			mp_ProgressCtrl->SetStep( 5 );
			m_playPic.SetIndicator( CPlayDisplay::INDICATOR_COMPRESS, TRUE );
            m_playPic.SetSize( 0 );			
			m_slowTimerId = SetTimer( 1, 1000, NULL );
         break;
	 }
	// change control button states
	m_btnRecord.EnableWindow(FALSE);
	m_btnPause.EnableWindow(FALSE);
	m_slider.EnableWindow(FALSE);
	m_btnPlay.EnableWindow(FALSE);
	m_btnFastR.EnableWindow(FALSE);
    m_btnFastF.EnableWindow(FALSE);
	m_btnVolU.EnableWindow(FALSE);
	m_btnVolD.EnableWindow(FALSE);

    disableCtrls( State() );
#endif
	return TRUE;
}

BOOL CQVoiceView::OnIdleUpdate()
{
	LsStateType relevantState = LastState();

	if ( relevantState == STATE_END )
	 {
		relevantState = mp_Action->LastState(1);
		if ( relevantState == STATE_PAUSE )
			relevantState = mp_Action->LastState(2);
	 }
	else if ( relevantState == STATE_PAUSE )
	 {
        mb_Pause = FALSE;
        m_playPic.SetIndicator( CPlayDisplay::INDICATOR_PAUSE, mb_Pause );
		relevantState = mp_Action->LastState(1);
	 }

	switch( relevantState )
	 {
        case STATE_RECORD:
			KillTimer( m_fastTimerId );
			m_curTotTime = mp_Action->LenPlayTotMs();
			m_slider.SetTicFreq( m_slider.GetRangeMax()/10 );
			m_slider.SetPos( 0 );
			m_playPic.SetLengthTime( m_curTotTime/1000 );
			m_playPic.SetPlayTime( 0 );
			m_playPic.SetRate( mp_Action->Rate() );
            m_playPic.SetSize( mp_Action->FinalSize() );
			m_playPic.SetIndicator( CPlayDisplay::INDICATOR_RECORD, FALSE );
			m_playPic.SetMeter( 0 );
			EnableToolTips( TRUE );
		 break;
		case STATE_PLAY:
			KillTimer( m_fastTimerId );
			m_slider.SetPos( 0 );
			m_playPic.SetIndicator( CPlayDisplay::INDICATOR_PLAY, FALSE );
            m_playPic.SetPlayTime( 0 );
			m_playPic.SetMeter( 0 );
			m_animIndex	= 0;
			m_playPic.SetPlayPic( m_animIndex );
#ifndef LS_PLAYER_ONLY
			mp_MainFrame->m_wndStatusBar.SetPaneText( 0,
									m_strDefaultStatusMsg, TRUE );
#endif
         break;
        case STATE_OPENFILE:
         break;
        case STATE_REPOSITION:
         break;
#ifndef LS_PLAYER_ONLY
		case STATE_RECORDENCODE:
		case STATE_ENCODE:
		 {
//			if ( LastState() == STATE_ENCODE )
				m_curTotTime = mp_Action->LenPlayTotMs();
//			else
//				len = mp_Action->LenRecordTotMs();
			// get rid of progress ctrl in the status bar
			if (mp_ProgressCtrl)
			 {
				mp_MainFrame->m_wndStatusBar.RemoveProgress();
				mp_ProgressCtrl = NULL;
			 }
			mp_MainFrame->m_wndStatusBar.SetPaneText(0,
									m_strDefaultStatusMsg, TRUE);
			m_slider.SetTicFreq( m_slider.GetRangeMax()/10 );
			m_slider.SetPos( 0 );
			m_playPic.SetLengthTime( m_curTotTime/1000 );
			m_playPic.SetPlayTime( 0 );
			m_playPic.SetRate( mp_Action->Rate() );
            m_playPic.SetSize( mp_Action->FinalSize() );
			m_playPic.SetIndicator( CPlayDisplay::INDICATOR_COMPRESS, FALSE );
			EnableToolTips(TRUE);
		 } break;
#endif
	 }
    enableCtrls( State() );
	m_playPic.SetIndicator( CPlayDisplay::ALL, FALSE );

	return TRUE;
}

BOOL CQVoiceView::OnOpenFileUpdate()
{
	// LastState() == STATE_IDLE

	m_curTotTime = mp_Action->LenPlayTotMs();
	m_slider.SetTicFreq( m_slider.GetRangeMax() / 10 );
    m_slider.SetPos( 0 );
	m_playPic.SetLengthTime( m_curTotTime/1000 );
    m_playPic.SetPlayTime( 0 );
    m_playPic.SetRate( mp_Action->Rate() );
	m_playPic.SetSize( mp_Action->FinalSize() );

    enableCtrls( State() );

	return TRUE;
}

void CQVoiceView::disableCtrls( int mode )
{
	switch (mode)
	{
		case STATE_INITIAL:
			m_slider.SetRange( 0, 10000 );
			m_slider.ClearTics();
			m_slider.SetPos( 0 );
			m_playPic.SetIndicator( CPlayDisplay::ALL, FALSE );
            m_playPic.SetPlayTime( 0 );
            m_playPic.SetLengthTime( 0 );
			m_playPic.SetRate( mp_Action->Rate() );
            m_playPic.SetSize( 0 );
#ifndef LS_PLAYER_ONLY
			mp_MainFrame->m_wndStatusBar.SetPaneText(0,
								m_strDefaultStatusMsg, TRUE);
#ifdef LS_TOOLBAR_COMBOBOX 
			mp_MainFrame->m_wndToolBar.m_cmbCodec.EnableWindow();
#endif
#endif
			m_slider.EnableWindow(FALSE);
			m_btnPlay.EnableWindow(FALSE);
			m_btnStop.EnableWindow(FALSE);
			m_btnPause.EnableWindow(FALSE);
			m_btnFastR.EnableWindow(FALSE);
            m_btnFastF.EnableWindow(FALSE);
			m_btnVolU.EnableWindow(FALSE);
			m_btnVolD.EnableWindow(FALSE);
			m_playPic.SetPlaySpeed( mp_Action->PlaySpeed() );
#ifndef LS_PLAYER_ONLY
			m_btnAttach.EnableWindow( FALSE );
            m_btnRecord.SetFocus();
#endif
			mb_NewEnable = TRUE;
			mb_SaveEnable = FALSE;
		 return;
	}

	mb_OpenEnable = FALSE;
	mb_SaveEnable = FALSE;
	mb_NewEnable = FALSE;
#ifndef LS_PLAYER_ONLY
	m_btnAttach.EnableWindow( FALSE );
#endif
	AfxGetApp()->m_pMainWnd->DragAcceptFiles(FALSE);
	
	m_btnStop.EnableWindow();
	m_btnStop.SetFocus();
}

void CQVoiceView::enableCtrls( int mode )
{
	mb_OpenEnable	= TRUE;
	mb_SaveEnable	= mp_Action->NewFlag();
	m_slider.EnableWindow();
	m_slider.SetPageSize( m_slider.GetRangeMax()/10 );
	m_btnPlay.EnableWindow();
#ifndef LS_PLAYER_ONLY
	m_btnRecord.EnableWindow();
#endif
    m_btnPlay.SetFocus();
	if ( STATE_IDLE == mode )
     {
        m_btnPause.EnableWindow( FALSE );
       	m_btnStop.EnableWindow( FALSE );
#ifndef LS_PLAYER_ONLY
		m_btnAttach.EnableWindow( TRUE );
#endif
		mb_NewEnable = TRUE; //mp_Action->NewFlag();
     }
    else 
     {
        m_btnPause.EnableWindow();
	    m_btnStop.EnableWindow();
     }
	m_btnFastR.EnableWindow();
    m_btnFastF.EnableWindow();

	// disable fast/slow button for plain wave file since we don't
	// support it yet
	if ( (LsStateAction::REALTIME == mp_Action->PlayMode()) ||
		 (LsStateAction::NONE == mp_Action->PlayMode()) )
	 {
		if ( mp_Action->PlaySpeed() >= 3 )
		 {
			m_btnVolU.EnableWindow( FALSE );
			m_btnVolD.EnableWindow();
		 }
		else if ( mp_Action->PlaySpeed() <= -3 )
		 {
			m_btnVolU.EnableWindow();
			m_btnVolD.EnableWindow( FALSE );
		 }
		else
		 {
			m_btnVolU.EnableWindow();
			m_btnVolD.EnableWindow();
		 }
	 }
	else
	 {
		m_btnVolU.EnableWindow( FALSE );
		m_btnVolD.EnableWindow( FALSE );
	 }
	AfxGetApp()->m_pMainWnd->DragAcceptFiles();
}

void CQVoiceView::UpdateProgress()
{
	switch ( State() )
	 {
	    case STATE_PLAY:
	     {
		    long t = mp_Action->LenPlayCurMs();
		    m_playPic.SetPlayTime( t/1000 );
			m_slider.SetPos( MAP_TIME_MS_2_SLIDER_POS(t) );
			m_animIndex++;
			int mod = mp_Action->PlaySpeed();
			if ( !mod ) mod = 3;			// normal speed
			else if ( mod > 0 ) mod = 1;	// fast
			else mod = 5;					// slow
			if ( !(m_animIndex % mod) )
				m_playPic.SetPlayPic( -1 ); // -1 = auto next frame
			if (m_VolOutMeter.IsOpen())
				m_playPic.SetMeter( m_VolOutMeter.Meter() );
	     } break;
#ifndef LS_PLAYER_ONLY
		case STATE_RECORDENCODE:
		case STATE_ENCODE:
		{
			int percent = mp_Action->UpdateProgress();
			TRACE("percent = %d\n", percent);
			if (percent > 0)
			 {
				m_playPic.SetSize( mp_Action->FinalSize() );
				char str[30];
				if (mp_ProgressCtrl)
					mp_ProgressCtrl->SetPos(percent);
				sprintf(str, "Converting: %d%% Done", percent);
				mp_MainFrame->m_wndStatusBar.SetPaneText(0, str);
			 }
			else if (percent == -1) // -1 means we're done
			 {
				KillTimer( m_slowTimerId );
				Event( EVENT_STOP, &percent );
			 }
		 } break;

		case STATE_RECORD:
		{
			m_timeIndex++;
			if ( !(m_timeIndex % 5) )
			 {
				long t = mp_Action->LenCurMs()/1000;
				m_curTotTime = mp_Action->LenTotMs();
				m_playPic.SetPlayTime( t );
				m_playPic.SetLengthTime( m_curTotTime/1000 );
				m_playPic.SetSize( mp_Action->FinalSize() );
				m_playPic.SetIndicator( CPlayDisplay::INDICATOR_RECORD, !(t % 2) );
			 }

			if ( m_VolInMeter.IsOpen() )
				m_playPic.SetMeter( m_VolInMeter.Meter() );
		} break;
#endif
	 } // switch()
}

/////////////////////////////////////////////////////////////////////////////
// CQVoiceView message handlers

void CQVoiceView::OnBtnPlay() 
{
	Event( EVENT_PLAY, m_hWnd );
}

void CQVoiceView::OnBtnRecord() 
{
	Event( EVENT_RECORD, m_hWnd );
}

void CQVoiceView::OnBtnStop() 
{
	Event( EVENT_STOP );
}

void CQVoiceView::OnBtnPause() 
{
	Event( EVENT_PAUSE );
}

void CQVoiceView::OnBtnAttach() 
{
	Event( EVENT_ATTACH );
}

void CQVoiceView::OnBtnFastR() 
{
    int t = m_slider.GetPos() - m_slider.GetPageSize();
    if ( t < 1 ) t = 0;
	
	m_slider.SetPos( t );

	int tt = MAP_SLIDER_POS_2_TIME_MS(t);
	Event( EVENT_REPOSITION, &tt );

    m_playPic.SetPlayTime( long(tt/1000) );
}

void CQVoiceView::OnBtnFastF() 
{
    int t = m_slider.GetPos() + m_slider.GetPageSize();
    if ( t > m_slider.GetRangeMax() )
        t = m_slider.GetRangeMax();

	m_slider.SetPos( t );

	int tt = MAP_SLIDER_POS_2_TIME_MS(t);
	Event( EVENT_REPOSITION, &tt );

    m_playPic.SetPlayTime( long(tt/1000) );
}

void CQVoiceView::OnBtnVolU() 
{
	mp_Action->PlaySpeed( mp_Action->PlaySpeed() + 1 );
	if ( mp_Action->PlaySpeed() >= 3 )
	 {
		mp_Action->PlaySpeed( 3 );
		m_btnVolU.EnableWindow( FALSE );
		m_btnVolD.EnableWindow();
	 }		
	else
	 {
		m_btnVolU.EnableWindow();
		m_btnVolD.EnableWindow();
	 }
	m_playPic.SetPlaySpeed( mp_Action->PlaySpeed() );
}

void CQVoiceView::OnBtnVolD() 
{
	mp_Action->PlaySpeed( mp_Action->PlaySpeed() - 1 );
	if ( mp_Action->PlaySpeed() <= -3 )
	 {
		//mi_volume = -3;
		mp_Action->PlaySpeed( -3 );
		m_btnVolU.EnableWindow();
		m_btnVolD.EnableWindow( FALSE );
	 }	
	else
	 {
		m_btnVolU.EnableWindow();
		m_btnVolD.EnableWindow();
	 }
	m_playPic.SetPlaySpeed( mp_Action->PlaySpeed() );
}

void CQVoiceView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( pScrollBar )
	 {
 		long t = MAP_SLIDER_POS_2_TIME_MS( ((CSliderCtrl *) pScrollBar)->GetPos() );
 		m_playPic.SetPlayTime( t/1000 );
		if ( nSBCode == SB_ENDSCROLL )
         {
			Event( EVENT_REPOSITION, &t );
         }
	 }
	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CQVoiceView::OnNewRecord() 
{
	if ( ((CQVoiceDoc*)GetDocument())->SaveModified() )
		Event( EVENT_NEW );
}

void CQVoiceView::OnEditInsertMode() 
{
	mp_Action->ToggleEditInsertMode();
}

void CQVoiceView::OnSelchangeCmbCodec()
{
	mp_Action->OnSelchangeCmbCodec();
}

void CQVoiceView::OnEncode() 
{
	Event( EVENT_ENCODE );	
}

#define CONTROL_X 5  
#define CONTROL_Y 96
void CQVoiceView::setUpControlButtons()
{
	CRect   viewRect, origRect, newRect;
	CPoint  xy, origXY( CONTROL_X, CONTROL_Y );
//  UINT    flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE;

    // find total width of control buttons
    m_btnPlay.GetWindowRect( LPRECT(origRect) );
    int totalWidth = origRect.Width() + 6;
#ifndef LS_PLAYER_ONLY
    m_btnRecord.GetWindowRect( LPRECT(origRect) );
    totalWidth += origRect.Width();
#endif
    m_btnStop.GetWindowRect( LPRECT(origRect) );
    totalWidth += origRect.Width();
    m_btnPause.GetWindowRect( LPRECT(origRect) );
    totalWidth += origRect.Width();
    m_btnFastR.GetWindowRect( LPRECT(origRect) );
    totalWidth += origRect.Width();
    m_btnFastF.GetWindowRect( LPRECT(origRect) );
    totalWidth += origRect.Width();
    m_btnVolU.GetWindowRect( LPRECT(origRect) );
    totalWidth += origRect.Width();
 //   m_btnVolD.GetWindowRect( LPRECT(origRect) );
 //   totalWidth += origRect.Width();

    // align buttons
    GetClientRect( LPRECT(viewRect) );
    xy = origXY;
    xy.x = (viewRect.Width() - totalWidth) / 2 + 1;
    m_btnPlay.GetWindowRect( LPRECT(origRect) );
	newRect.SetRect( xy.x, xy.y,
                     xy.x + origRect.Width(), xy.y + origRect.Height() );

    // Play Button
//  m_btnPlay.SetWindowPos( NULL, newRect.left, newRect.top, 0, 0, flags );
	m_btnPlay.MoveWindow( LPCRECT(newRect), FALSE );
    newRect.OffsetRect( newRect.Width(), 0 );

#ifndef LS_PLAYER_ONLY
    // Record Button		
    m_btnRecord.GetWindowRect( LPRECT(origRect) );
	xy = newRect.TopLeft() + origRect.Size();
	newRect.SetRect( newRect.left, newRect.top, xy.x, xy.y );
	m_btnRecord.MoveWindow( LPCRECT(newRect), FALSE );
	newRect.OffsetRect( newRect.Width(), 0 );
#endif

    // Stop Button		
    m_btnStop.GetWindowRect( LPRECT(origRect) );
	xy = newRect.TopLeft() + origRect.Size();
	newRect.SetRect( newRect.left, newRect.top, xy.x, xy.y );
	m_btnStop.MoveWindow( LPCRECT(newRect), FALSE );
	newRect.OffsetRect( newRect.Width(), 0 );

    // Pause Button
    m_btnPause.GetWindowRect( LPRECT(origRect) );
	xy = newRect.TopLeft() + origRect.Size();
	newRect.SetRect( newRect.left, newRect.top, xy.x, xy.y );
	m_btnPause.MoveWindow( LPCRECT(newRect), FALSE );
	newRect.OffsetRect( newRect.Width(), 0 );

    // FastR Button
    m_btnFastR.GetWindowRect( LPRECT(origRect) );
	xy = newRect.TopLeft() + origRect.Size();
	newRect.SetRect( newRect.left, newRect.top, xy.x, xy.y );
	m_btnFastR.MoveWindow( LPCRECT(newRect), FALSE );
	newRect.OffsetRect( newRect.Width(), 0 );

    // FastF Button
	m_btnFastF.GetWindowRect( LPRECT(origRect) );
	xy = newRect.TopLeft() + origRect.Size();
	newRect.SetRect( newRect.left, newRect.top, xy.x, xy.y );
	m_btnFastF.MoveWindow( LPCRECT(newRect) );
	newRect.OffsetRect( newRect.Width() + 6, 0 );
/*
    // Speed Down Button
	m_btnVolD.GetWindowRect( LPRECT(origRect) );
	xy = newRect.TopLeft() + origRect.Size();
	newRect.SetRect( newRect.left, newRect.top, xy.x, xy.y );
	m_btnVolD.MoveWindow( LPCRECT(newRect) );
	newRect.OffsetRect( newRect.Width(), 0 );
	//newRect.OffsetRect( 0, newRect.Height() );

    // Speed Up Button
	m_btnVolD.GetWindowRect( LPRECT(origRect) );
	xy = newRect.TopLeft() + origRect.Size();
	newRect.SetRect( newRect.left, newRect.top, xy.x, xy.y );
	m_btnVolU.MoveWindow( LPCRECT(newRect) );
*/	
    // Speed Up Button
	m_btnVolU.GetWindowRect( LPRECT(origRect) );
	xy = newRect.TopLeft() + origRect.Size();
	newRect.SetRect( newRect.left, newRect.top, xy.x, xy.y );
	m_btnVolU.MoveWindow( LPCRECT(newRect) );
	newRect.OffsetRect( 0, newRect.Height() );

    // Speed Down Button
	m_btnVolU.GetWindowRect( LPRECT(origRect) );
	xy = newRect.TopLeft() + origRect.Size();
	newRect.SetRect( newRect.left, newRect.top, xy.x, xy.y );
	m_btnVolD.MoveWindow( LPCRECT(newRect) );

}
/*
void moveControl( CWnd *control, x, y, cx, cy, UINT nFlag )
{
	CRect   viewRect, newRect;
	CPoint  xy;

    GetClientRect( LPRECT(viewRect) );
    switch( nFlag )
    {
    case CENTER_VER:

    case CENTER_HOR:

    }
    int w = viewRect.Width() - (SLIDER_X * 2);
    int h = 20;
	xy = origXY;
	newRect.SetRect( xy.x, xy.y, xy.x + w, xy.y + h );
    control->MoveWindow( LPCRECT(newRect) );
}
*/
void CQVoiceView::OnInitialUpdate() 
{
	mp_Action = ((CQVoiceDoc*)GetDocument())->StateAction();
	mp_Event  = ((CQVoiceDoc*)GetDocument())->StateEvent();

	// assign our main frame
	mp_MainFrame = (CMainFrame*)GetParentFrame();

	if ( mb_OneTime )
	 {
		TRACE("CQVoiceView::OnInitialUpdate() - One Shot\n"); 
		// load bitmaps for each button
		m_btnPlay.AutoLoad(IDC_BTN_PLAY, this);
#ifndef LS_PLAYER_ONLY
		m_btnRecord.AutoLoad(IDC_BTN_RECORD, this);
		m_btnAttach.AutoLoad(IDC_BTN_ATTACH, this);
#endif
		m_btnStop.AutoLoad(IDC_BTN_STOP, this);
		m_btnPause.AutoLoad(IDC_BTN_PAUSE, this);
		m_btnFastR.AutoLoad(IDC_BTN_FASTR, this);
		m_btnFastF.AutoLoad(IDC_BTN_FASTF, this);
		m_btnVolU.AutoLoad(IDC_BTN_SPDU, this);
		m_btnVolD.AutoLoad(IDC_BTN_SPDD, this);

		// better method of determining if menu string needs to be
		// shortened or not
		int diff;
		{
			NONCLIENTMETRICS ncm;
			ncm.cbSize = sizeof(ncm);
			SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &ncm, 0 );
			CFont menuFont;
			menuFont.CreateFontIndirect( &ncm.lfMenuFont );
			CClientDC fontDC(this);
			CFont* oldFont = fontDC.SelectObject( &menuFont );
			CString menuStr; menuStr.LoadString( IDS_MAIN_MENU );
			CSize fSize;
			GetTextExtentPoint32(
				fontDC.GetSafeHdc(), menuStr, menuStr.GetLength(), &fSize );
			fontDC.SelectObject( oldFont );
			diff = fSize.cx*100 / FORMVIEW_W;
			TRACE2("Menu String Width = %d, %d\n", fSize.cx, diff);
		}
		if ( diff > 70 )
		 {
			CMenu menu, submenu;
			menu.Attach((mp_MainFrame->GetMenu())->GetSafeHmenu());
			CString name[4];
			name[0] = "F..";name[1] = "C..";
			name[2] = "O..";name[3] = "H..";
			if ( diff > 85 )
			 {
				name[0] = "F";name[1] = "C";
				name[2] = "O";name[3] = "H";
			 }
			for ( int i=0; i<4; i++)
			{
				submenu.Attach((menu.GetSubMenu(i))->GetSafeHmenu());
				menu.ModifyMenu( i, MF_POPUP | MF_BYPOSITION | MF_STRING,
					UINT(submenu.GetSafeHmenu()), name[i] );
				mp_MainFrame->SetMenu( &menu );
				submenu.Detach();
			}
			menu.Detach();
		 }
/*
		// clip menu name so it will fit on 
		// those system with large fonts enabled
		// Need to make this better but works for now
		if ( GetSystemMetrics(SM_CYMENU) > 19 )
		 {
			CMenu menu, submenu;
			menu.Attach((mp_MainFrame->GetMenu())->GetSafeHmenu());
			CString name[4];
			if ( GetSystemMetrics(SM_CYMENU) > 35 )
			 {
				name[0] = "F";name[1] = "C";
				name[2] = "O";name[3] = "H";
			 }
			else
			 {
				name[0] = "F..";name[1] = "C..";
				name[2] = "O..";name[3] = "H..";
			 }
			for ( int i=0; i<4; i++)
			{
				submenu.Attach((menu.GetSubMenu(i))->GetSafeHmenu());
				menu.ModifyMenu( i, MF_POPUP | MF_BYPOSITION | MF_STRING,
					UINT(submenu.GetSafeHmenu()), name[i] );
				mp_MainFrame->SetMenu( &menu );
				submenu.Detach();
			}
			menu.Detach();
		 }
*/
        // resize our view to correct size
        CSize viewSize( FORMVIEW_W, FORMVIEW_H );
        SetScrollSizes( MM_TEXT, viewSize );  
	    GetParentFrame()->RecalcLayout();
        ResizeParentToFit();
        UpdateData(FALSE);

        // position our LED display
        #define LED_Y 8  
	    CRect   viewRect, newRect, origRect;
	    CPoint  xy;
        GetClientRect( LPRECT(viewRect) );
#ifndef LS_PLAYER_ONLY
        m_btnAttach.GetWindowRect( LPRECT(origRect) );
        xy.x = origRect.Width();
#endif
        m_playPic.GetWindowRect( LPRECT(origRect) ); 
#ifndef LS_PLAYER_ONLY
        xy.x = (viewRect.Width() - (origRect.Width() + xy.x + 8))/2;
#else
        xy.x = (viewRect.Width() - origRect.Width())/2;
#endif
	    xy.y = LED_Y;
	    newRect.SetRect( xy.x, xy.y,
            xy.x + origRect.Width(), xy.y + origRect.Height() );
        m_playPic.MoveWindow( LPCRECT(newRect) );

#ifndef LS_PLAYER_ONLY    
        // position Attach button
	    newRect.OffsetRect( newRect.Width() + 8, 0 );
        m_btnAttach.SetWindowPos( NULL, newRect.left, newRect.top, 0, 0,
                            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE );
#endif
        
        // position the slider
        #define SLIDER_X 2  
        #define SLIDER_Y 63
        int w = viewRect.Width() - (SLIDER_X * 2);
        int h = 20;
		// adjust slider depending on the thumb size so that ticks will
		// show up with different display types. crude way but better than
		// no adjustment at all
	    xy.x = SLIDER_X; xy.y = SLIDER_Y;
	    newRect.SetRect( xy.x, xy.y, xy.x + w, xy.y + h );
        m_slider.MoveWindow( LPCRECT(newRect) );
		CRect thumbRect; m_slider.GetThumbRect( LPRECT(thumbRect) );
		if (thumbRect.Height() >= 20) h = 19;
		else if (thumbRect.Height() >= 17) h = 24;
		else if (thumbRect.Height() == 16) h = 22;
	    newRect.SetRect( xy.x, xy.y, xy.x + w, xy.y + h );
        m_slider.MoveWindow( LPCRECT(newRect) );

		// setup and position control buttons
        setUpControlButtons();

		// setup volume dialog bar
		if ( !m_VolumeBar.Create( mp_MainFrame ) )
		 {
			TRACE0("Failed to create VolumeBar\n");
		 }
//		m_VolumeBar.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
//		mp_MainFrame->DockControlBar(&m_VolumeBar);
	 }

	CFormView::OnInitialUpdate();
	if ( mb_OneTime )
	 { 
		// These calls are used to fit our Frame to our View
		mp_MainFrame->RecalcLayout();
		ResizeParentToFit();

		// position at the current mouse position
		CRect newRect;
		mp_MainFrame->GetWindowRect( LPRECT(newRect) );
		POINT pos;
		::GetCursorPos( &pos );
		pos.x -= newRect.Width()/2; pos.y -= newRect.Height()/2;
		int sx = GetSystemMetrics( SM_CXFULLSCREEN );
		int sy = GetSystemMetrics( SM_CYFULLSCREEN );
		if ( pos.x < 0 ) pos.x = 0; if ( pos.y < 0 ) pos.y = 0;
		if ( (pos.x + newRect.Width()) > sx )
			pos.x = sx - newRect.Width();
		if ( (pos.y + newRect.Height()) > sy )
			pos.y = sy - newRect.Height();
		mp_MainFrame->SetWindowPos( NULL, pos.x, pos.y, 0, 0,
						SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE );

		EnableToolTips(TRUE);
	 }

	// hey, did we have a filename when we started the program
	if ( mb_OneTime )
	 { 
        if ( !Event( EVENT_INITIAL ) )
			AfxGetMainWnd()->PostMessage( WM_CLOSE ); // quit app
		if ( ((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.m_nShellCommand 
			== CCommandLineInfo::FileOpen ) 
		 {
			CString fn = ((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.m_strFileName;
			// clear it - this is used as a flag by Document
			// to tell if we're being called from ProcessShellCommand() or not
			((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.m_strFileName.Empty();
			if ( !((CQVoiceDoc*)GetDocument())->OnOpenDocument(LPCTSTR(fn)) )
				AfxGetMainWnd()->PostMessage( WM_CLOSE ); // quit app
		 }

		m_VolOutMeter.Open( LsMixerControl::waveOut );
		m_VolInMeter.Open( LsMixerControl::waveIn );

		m_strDefaultStatusMsg.LoadString( AFX_IDS_IDLEMESSAGE );

		if ( ((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.mb_Record )
		 {
			m_startRecordTimerId = SetTimer(m_startRecordTimerId, 1000, NULL);
		 }

		TRACE("Exiting CQVoiceView::OnInitialUpdate() - One Shot\n"); 
	 }
	 
	mb_OneTime = FALSE;
}

void CQVoiceView::OnUpdatePlay(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_btnPlay.IsWindowEnabled() );
}

void CQVoiceView::OnUpdateRecord(CCmdUI* pCmdUI) 
{
#ifndef LS_PLAYER_ONLY
	pCmdUI->Enable( m_btnRecord.IsWindowEnabled() );
#endif
}

void CQVoiceView::OnUpdateStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_btnStop.IsWindowEnabled() );	
}

void CQVoiceView::OnUpdatePause(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_btnPause.IsWindowEnabled() );	
    m_playPic.SetIndicator( CPlayDisplay::INDICATOR_PAUSE, mb_Pause );
}

void CQVoiceView::OnUpdateFastR(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_btnFastR.IsWindowEnabled() );	
}

void CQVoiceView::OnUpdateFastF(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_btnFastF.IsWindowEnabled() );	
}

void CQVoiceView::OnUpdateVolU(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_btnVolU.IsWindowEnabled() );	
}

void CQVoiceView::OnUpdateVolD(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_btnVolD.IsWindowEnabled() );	
}

void CQVoiceView::OnUpdateAttach(CCmdUI* pCmdUI) 
{
#ifndef LS_PLAYER_ONLY
	pCmdUI->Enable( m_btnAttach.IsWindowEnabled() );
#endif
}

void CQVoiceView::OnUpdateNewRecord(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( mb_NewEnable );	
}

void CQVoiceView::OnUpdateFileOpen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( mb_OpenEnable );	
}

void CQVoiceView::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( mb_SaveEnable );	
}

void CQVoiceView::OnUpdateEditInsertMode(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( mp_Action->EditInsertMode() ? 1 : 0 );		
}

void CQVoiceView::OnVolumebar() 
{
	m_VolumeBar.Show( !m_VolumeBar.IsWindowVisible() );
	ResizeParentToFit();
}

void CQVoiceView::OnUpdateVolumebar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_VolumeBar.IsWindowVisible() ? 1 : 0 );	
}

void CQVoiceView::OnAgc() 
{
	mp_Action->mb_agcEnable = !mp_Action->mb_agcEnable;
}

void CQVoiceView::OnUpdateAgc(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( mp_Action->mb_agcEnable ? 1 : 0 );			
}

void CQVoiceView::OnTimer(UINT nIDEvent) 
{
	if ( m_startRecordTimerId == nIDEvent )
	 {
		KillTimer( m_startRecordTimerId );
		TRACE0("START_RECORD_TIMER killed\n");
		Event( EVENT_RECORD, m_hWnd );
	 }
	else
		UpdateProgress();
	CFormView::OnTimer(nIDEvent);
}

void CQVoiceView::OnDraw(CDC* pDC) 
{
	CRect viewRect;
    
    // draw raised border around our view
	GetClientRect( viewRect );
	pDC->DrawEdge( viewRect, EDGE_RAISED, BF_RECT );

    // draw separator between display and control buttons
    viewRect.top = CONTROL_Y - 7;
    pDC->DrawEdge( viewRect, EDGE_ETCHED, BF_TOP );

	//CFormView::OnDraw( pDC );
}

BOOL CQVoiceView::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
    UINT nID =pNMHDR->idFrom;
    if (pTTT->uFlags & TTF_IDISHWND)
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);
        if(nID)
        {
            pTTT->lpszText = MAKEINTRESOURCE(nID);
            pTTT->hinst = AfxGetResourceHandle();
            return(TRUE);
        }
    }
    return(FALSE);
}

