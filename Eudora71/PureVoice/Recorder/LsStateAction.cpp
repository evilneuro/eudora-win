/*////////////////////////////////////////////////////////////////////////////

NAME:
	LsStateAction - 

FILE:		LsStateAction.CPP
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.
	Must call Init() before any use.

DEPENDENCIES:


NOTES:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/??/96   lss     -Initial
01/08/97   lss	   -Fixed exit path when exiting from realtime recording
01/15/97   lss	   -Added WRITE_PROFILE_CODEC and GET_PROFILE_CODEC macros
				    for saving and retrieving last used codec
02/21/97   lss     -Added CODEC_PLUGIN_EXT define
03/20/97   lss     -Added LS_TOOLBAR_COMBOBOX to include the combo-box or not
03/21/97   lss     -Added WRITE_PROFILE_OVERWRITE and GET_PROFILE_OVERWRITE 
					macros to save and retrieve previous state of OverWrite
					mode
03/24/97   lss     -Added PlaySpeed() access so we can have playback speed
					info 
04/01/97   lss     -Added mi_hogCpu for encoding cpu usage "control" in
					CConvert
04/23/97   lss     -Instead doing delete/new CWaveOutDecode every time user
					clicks on the play button, do the delete/new when necessary
04/30/97   lss     -Remove No Compression option, instead give error message
					then quit
05/02/97   lss     -Changed *_PROFILE_OVERWRITE to *_PROFILE_INSERT and
					changed Overwrite stuff to InsertMode
05/19/97   lss     -Added GET/WRITE_PROFILE_PLAYSPEED
06/04/97   lss     -Fixed the problem of not recognizing any change of 
					playback speed during pause
06/07/97   lss     -Make sure we delete objects in correct order
08/20/97   lss     -Don't prematurely delete current recording
					so that if we have error in recording, we
					can revert back to previous one. (OnRecord)
					
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"
#include <mapi.h>
#include <memory.h>

// PROJECT INCLUDES
#include "lscommon.h"
#include "QVoice.h"
#include "QVoiceView.h"
#include "CAWave.h"
#include "CWavePCM.h"
#include "CWaveCon.h"
#include "CACodec.h"
#include "mmioFile.h"
#include "CConvert.h"
#include "CPlugIn.h"
#include "CodecMnu.h"
#include "ErrorHandler.h"
#include "LsStateMachine.h"
#ifdef LS_DEBUG0
	#include "LsDebug.h"
#endif

// LOCAL INCLUDES
#include "LsStateAction.h"

// LOCAL DEFINES
#define CHECK_MENU_RANGE_BEGIN	ID_CODEC_DEFAULT
#define CHECK_MENU_RANGE_END	ID_CODEC_INSERT_END

#define DEFAULT_RATE			128000
#define NO_CODEC_FOUND			-1
#define NO_CODEC				-1

#define	CODEC_PLUGIN_EXT		_T("qcl")

#define NO_COMPRESSION_REGISTRY		1				// for registery

//#define WRITE_PROFILE_CODEC(id)			\
//					AfxGetApp()->WriteProfileInt( "Codec", "CurrentID", id )
//#define GET_PROFILE_CODEC()				\
//					AfxGetApp()->GetProfileInt( "Codec", "CurrentID", 0 )

void GUID2String( REFGUID cid, CString& scid )
{
	unsigned long *d4a = (unsigned long*)&cid.Data4[0];
	unsigned long *d4b = (unsigned long*)&cid.Data4[4];
	scid.Format( "%X%X%X%X%X", cid.Data1, cid.Data2, cid.Data3, *d4a, *d4b );
}

void String2GUID( LPCTSTR scid, GUID& cid )
{
	char *endptr;
	CString s = scid, t;
	unsigned long *d4a = (unsigned long*)&cid.Data4[0];
	unsigned long *d4b = (unsigned long*)&cid.Data4[4];
	t = s.Left(8);
	cid.Data1 = strtoul( LPCTSTR(t), &endptr, 16 );
	s.Format("%s", s.Right(s.GetLength()-8)); t = s.Left(4);
	cid.Data2 = (WORD)strtoul( LPCTSTR(t), &endptr, 16 );
	s.Format("%s", s.Right(s.GetLength()-4)); t = s.Left(4);
	cid.Data3 = (WORD)strtoul( LPCTSTR(t), &endptr, 16 );
	s.Format("%s", s.Right(s.GetLength()-4)); t = s.Left(8);
	*d4a = strtoul( LPCTSTR(t), &endptr, 16 );
	s.Format("%s", s.Right(s.GetLength()-8));
	*d4b = strtoul( LPCTSTR(s), &endptr, 16 );
}

inline void WRITE_PROFILE_CODEC( REFGUID cid )
{
	CString tmp;
	GUID2String( cid, tmp );
	AfxGetApp()->WriteProfileString( "Codec", "CurrentID", tmp );
}

inline void GET_PROFILE_CODEC( GUID& cid )
{
	CString tmp = AfxGetApp()->GetProfileString( "Codec", "CurrentID" );
	if (tmp.IsEmpty()) 
		SetGUID( cid, GUID_NULL );
	else
		String2GUID( tmp, cid );
}

#define WRITE_PROFILE_INSERT(bSet)	\
					AfxGetApp()->WriteProfileInt( "Settings", "InsertMode", bSet )
#define GET_PROFILE_INSERT()			\
					AfxGetApp()->GetProfileInt( "Settings", "InsertMode", 0 )
#define WRITE_PROFILE_HOGCPU(i)			\
					AfxGetApp()->WriteProfileInt( "Settings", "CpuUsage", i )
#define GET_PROFILE_HOGCPU()			\
					AfxGetApp()->GetProfileInt( "Settings", "CpuUsage", 1 )
#define WRITE_PROFILE_AGC(i)			\
					AfxGetApp()->WriteProfileInt( "Settings", "AGC", i )
#define GET_PROFILE_AGC()			\
					AfxGetApp()->GetProfileInt( "Settings", "AGC", 0 )
#define WRITE_PROFILE_PLAYSPEED(speed)			\
					AfxGetApp()->WriteProfileInt( "Settings", "PlaybackSpeed", speed )
#define GET_PROFILE_PLAYSPEED()			\
					AfxGetApp()->GetProfileInt( "Settings", "PlaybackSpeed", 0 )
#define WRITE_PROFILE_ATTACH_DESC(bSet)	\
					AfxGetApp()->WriteProfileInt( "Settings", "AttachDesc", bSet )
#define GET_PROFILE_ATTACH_DESC()			\
					AfxGetApp()->GetProfileInt( "Settings", "AttachDesc", 0 )


//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

LsStateAction::LsStateAction() :
	mp_CodecMenu(0), mp_Codec(0), mp_WaveOut(0),
	mp_WaveIn(0), mp_Converter(0), mp_SndBuf(0), ml_rate(0), ml_PlayLenMs(0),
	ml_DispTotTime(0), ml_PlayIndex(0), ml_RecordIndex(0),
	ms_OpenFileName(_T("")), ms_SaveFileName(_T("")), ms_TempFileName(_T("")),
	ms_WorkFileName(_T("")), ms_WorkFileNameLast(_T("")),
	mb_FromMmWomClose(FALSE), mb_noCodec(FALSE),
	mi_codecMenuIndex(0),	// use first codec found
	mi_playCodecIndex(0),
	mi_recordCodecIndex(NO_CODEC),mi_recordCodecIndexOld(NO_CODEC),
	me_playMode(NONE), me_recordMode(NONE),
	mb_ConvDone(FALSE), mb_editInsertMode(FALSE), mi_hogCpu(1),
	mb_NewFlag(FALSE), ml_DispFinalSize(0),
	m_srchPluginFlag(-1), ml_volume(0), m_playSpeed(0),
	mb_agcEnable(FALSE), mb_attachDesc(FALSE),
	mi_validToConvert(FALSE)
{
	// flag for threaded vs non-threaded (yield events to the system)
	// - TRUE : for non-thread
	// - FALSE: for thread
	mb_UseYield	= FALSE; 
}

void LsStateAction::Init( LsStateMachine& State, CDocument* doc )
{
	mp_State = &State;
	mp_Doc   = doc;
}

LsStateAction::~LsStateAction()
{
	delete mp_CodecMenu;

	// order of deletion is very important, want to delete CConvert and
	// CWaveIn before CSndBuffer and CACodec
	delete mp_Converter; mp_Converter = NULL;
	delete mp_WaveIn;
	delete mp_Codec;
#ifndef LS_PLAYER_ONLY
	delete mp_SndBuf;
#endif

	delete mp_WaveOut; 
	mp_WaveOut = NULL; // since deleteTempFiles() will do delete also

	deleteTempFiles();
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL LsStateAction::UpdateView()
{
	POSITION pos = mp_Doc->GetFirstViewPosition();
	CQVoiceView* pView = (CQVoiceView*)	(mp_Doc->GetNextView( pos ));

	return pView->UpdateView();
}

BOOL LsStateAction::OnInitial()
{
	if ( LastState() == LsStateMachine::NONE )
	 { 
		BOOL bFound = SearchPlugins();
		if (!bFound)
		 {
			m_ErrorDisplay.ReportError( LS_ERROR_NO_CODEC );
			return FALSE;
		 }
#ifndef LS_PLAYER_ONLY
		// retrieve previous session's state of InsertMode mode
		mb_editInsertMode = GET_PROFILE_INSERT();
		
		// retrieve CPU Usage setting
		mi_hogCpu = GET_PROFILE_HOGCPU();

		// retrieve previous session's playspeed
		m_playSpeed = GET_PROFILE_PLAYSPEED();

		// retrieve previous session's attach description option
		mb_attachDesc = GET_PROFILE_ATTACH_DESC();

		// get the id of the codec used in the last session
//		int id = GET_PROFILE_CODEC();
		GUID id;
		GET_PROFILE_CODEC( id );
/*		
		if ( NO_COMPRESSION_REGISTRY == id )
		 {
			// no compression
			mi_recordCodecIndex = NO_CODEC;
		 }
*/
		if ( id == GUID_NULL )
		 {
			// ok, didn't find anything in registery, must be first
			// time or something. so use 13k by default
			if ( bFound )
				mi_recordCodecIndex = 0;// m_PlugIn.getIndex(LCP_ID_CELP13K);
		 }
		else
		 {
			// see if we really have it
			mi_recordCodecIndex = m_PlugIn.getIndex( id );
		 }

		if ( (NO_CODEC != mi_recordCodecIndex) &&
			 (NO_CODEC_FOUND != mi_recordCodecIndex) )
			m_PlugIn.setIndex( mi_recordCodecIndex );

		mi_recordCodecIndexOld = mi_recordCodecIndex;

		// put a name in the name box 
		CMainFrame *pMf = (CMainFrame*)AfxGetMainWnd();
		CQVoiceApp *pApp = (CQVoiceApp *)AfxGetApp(); 
		if ( !pApp->m_cmdLineInfo.m_strOutFileName.IsEmpty() )
		 {
			CString fn = pApp->m_cmdLineInfo.m_strOutFileName;
			// strip path name and extension
			if ( fn.ReverseFind( '\\' ) > -1 )
				fn = fn.Right( fn.GetLength() - fn.ReverseFind( '\\' )-1 );
			if ( fn.ReverseFind( '.' ) > -1 )
				fn = fn.Left(fn.ReverseFind( '.' ));
			pMf->m_wndNameBar.Name( fn );
		 }
		else
			pMf->m_wndNameBar.Name( getUniqueName() );
#endif
		srchCodecUpdateMenu();
	 }

	mp_Doc->SetModifiedFlag( FALSE );
	NewFlag( FALSE );

	ml_RecordIndex = 0;
	ml_PlayIndex   = 0;
	ml_DispTotTime = 0;

	return TRUE;
}

BOOL LsStateAction::PostOnInitial()
{
#if !defined( LS_PLAYER_ONLY ) && defined( LS_TOOLBAR_COMBOBOX )
	((CMainFrame*)AfxGetMainWnd())->m_wndToolBar.m_cmbCodec.EnableWindow(
						FALSE );
#endif
	return TRUE;
}

BOOL LsStateAction::OnReposition()
{
	switch( LastState() )
	 {
		case LsStateMachine::PLAY:
            OnIdle();
         break;
	 }

	if ( mp_State->Param() == 0 )
		ml_PlayIndex = 0;
	else
		ml_PlayIndex = *((int *)mp_State->Param());

	ml_RecordIndex = ml_PlayIndex;

	return TRUE;
}

BOOL LsStateAction::OnPlay()
{
	long ret;

	if ( PlayMode() == NONE )
	 {
		return FALSE;
	 }

	if ( (LastState() != LsStateMachine::REPOSITION) &&
		 (LastState() != LsStateMachine::PAUSE) )
	{
	HWND hWnd = HWND(mp_State->Param());
	if ( hWnd == 0 ) return FALSE;
//	delete mp_WaveOut;
	
	switch( PlayMode() )
	 {
		case FILE:
			if (!mp_WaveOut)
				mp_WaveOut = new CWaveOut( WorkFileName(), hWnd );
		 break;
		case MEM:
			if (mp_SndBuf==NULL)
			 {
				//m_ErrorDisplay.ReportError( "Nothing to Play." );
				return FALSE;// can't play without something to play
			 }
			if (!mp_WaveOut)
				mp_WaveOut = new CWaveOut( mp_SndBuf, hWnd );
		 break;
		case REALTIME:// else play the real-time decoded samples
			if ( NO_CODEC_FOUND == mi_playCodecIndex ) 
			 {
				m_ErrorDisplay.ReportError( LS_ERROR_NO_CODEC );
				return FALSE;
			 }
			if (!mp_WaveOut)
			 {
				delete mp_Codec; mp_Codec = NULL;
				mp_Codec = m_PlugIn.getCodec( mi_playCodecIndex );
				mp_WaveOut = new CWaveOutDecode( WorkFileName(), hWnd, 
										mp_Codec, mb_UseYield );
			 }
			((CWaveOutDecode *)mp_WaveOut)->PlaySpeed( m_playSpeed );
		break;
	 }
	}
	if ( ret = mp_WaveOut->Error() )
	 {
		m_ErrorDisplay.ReportError( ret );
		return FALSE;
	 }
	mp_WaveOut->setPosition( ml_PlayIndex, UNIT_TIME_MS );

	if ( REALTIME == PlayMode() )
		((CWaveOutDecode *)mp_WaveOut)->PlaySpeed( m_playSpeed );
    TRACE("About to PLAY\n");  
	ret = mp_WaveOut->Play();
	if ( ret < 1 )	// uh-oh, error or nothing to play
	 {
		if ( ret == -1 ) m_ErrorDisplay.ReportError( mp_WaveOut->Error() );
		return FALSE;
	 }

	return TRUE;
}

BOOL LsStateAction::OnRecord()
{
#ifndef LS_PLAYER_ONLY
	if ( (LastState() == LsStateMachine::PAUSE) &&
		 (LastState(1) == LsStateMachine::RECORD) )
	 {
		if ( mp_WaveIn->Record() == -1) //uh-oh, error
		 {
			m_ErrorDisplay.ReportError( mp_WaveIn->Error() );
			return FALSE;
		 }
		return TRUE;
	 }

	HWND hWnd = HWND(mp_State->Param());
	if ( hWnd == 0 ) return FALSE;

	/*
	// if we're overwriting the whole file, just
	// delete the old one and create a new one
	if ( (ml_RecordIndex == 0) && !EditInsertMode() ) 
		deleteTempFiles();
	*/
	WorkFileName( getTempFileName() );

	if ( !EditInsertMode() ) ml_DispFinalSize = 0;
	if ( !EditInsertMode() && (ml_RecordIndex > 0) && 
		 (REALTIME == PlayMode()) )
	 {
		// since we're overwriting, we need the size info of ml_RecordIndex
		// worth of file being overwritten so we can display correct
		// dynamic file size info
		// it's really ugly at the moment but will restructure later
		ml_DispFinalSize = ml_RecordIndex/1000 * 1000; // trunc to full sec
		if ( mp_WaveOut )
		 {
		CQcelpFile *pQcelpFile = 
			(CQcelpFile*)(((CWaveOutDecode*)mp_WaveOut)->mp_converter->mp_mmioInFile);
		pQcelpFile->SeekInTime( (ULONG*)&ml_DispFinalSize );
		 }
		else
		 {
			if ( !ms_WorkFileNameLast.IsEmpty() )
			 {
				CQcelpFile size;
				if ( size.Open( ms_WorkFileNameLast, CmmioFile::READ ) )
				 {
					size.SeekInTime( (ULONG*)&ml_DispFinalSize );
					size.Close();
				 }
			 }
		 }
	 }

	delete mp_WaveOut; mp_WaveOut = NULL;
	delete mp_WaveIn; mp_WaveIn = NULL;

	if ( !mb_noCodec )
	 {
		mi_recordCodecIndex = m_PlugIn.getIndex();
		if ( NO_CODEC_FOUND == mi_recordCodecIndex )
		 {
			m_ErrorDisplay.ReportError( LS_ERROR_NO_CODEC );
			return FALSE;
		 }

		// the order of these object deletion is very important
		delete mp_Converter; mp_Converter = NULL;
		mb_ConvDone = FALSE;
		delete mp_Codec; mp_Codec = NULL;
		mp_Codec = m_PlugIn.getCodec( mi_recordCodecIndex );
		delete mp_SndBuf; mp_SndBuf = NULL;

		// make the CSndBuf page size to be 2 times the codec's frame size
		int frameSize = mp_Codec->Info()->samplesPerBlock *
						(mp_Codec->Info()->bitsPerSample >> 3) * 2;
		mp_SndBuf = new CSndBuffer( TRUE, frameSize );

		mp_WaveIn = new CWaveIn( mp_SndBuf, hWnd, mb_agcEnable );

		if ( ((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.mb_Record )
		 {
			//::MessageBeep( MB_OK );
			::Beep( 1000, 250 );
			((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.mb_Record = FALSE;
		 }

		if ( mp_WaveIn->Record() == -1) //uh-oh, error
		 {
			m_ErrorDisplay.ReportError( mp_WaveIn->Error() );
			// restore last file name
			ms_WorkFileName = ms_WorkFileNameLast;
			ms_WorkFileNameLast.Empty();
			return FALSE;
		 }

		mp_Converter = new CConvert( mp_SndBuf, LPCTSTR(WorkFileName()),
								mp_Codec, &mb_ConvDone, mb_UseYield );
		
		if ( !mp_Converter->Encode( mi_hogCpu ) )
		 {
			// delete current temp file
			deleteTempFiles( CUR );
			// restore last file name
			ms_WorkFileName = ms_WorkFileNameLast;
			ms_WorkFileNameLast.Empty();
			return FALSE;
		 }
		RecordMode( REALTIME );
		ml_rate  = m_PlugIn.getCodecInfo(mi_recordCodecIndex)->avgBitsPerSec;
	 }
	else
	 {
		mp_WaveIn = new CWaveIn( WorkFileName(), hWnd, mb_agcEnable );

		if ( mp_WaveIn->Record() == -1) //uh-oh, error
		 {
			m_ErrorDisplay.ReportError( mp_WaveIn->Error() );
			// delete current temp file
			deleteTempFiles( CUR );
			// restore last file name
			ms_WorkFileName = ms_WorkFileNameLast;
			ms_WorkFileNameLast.Empty();			
			return FALSE;
		 }
		RecordMode( FILE );
		ml_rate = DEFAULT_RATE;
	 }
#endif
	return TRUE;
}

BOOL LsStateAction::OnEncode()
{
#ifndef LS_PLAYER_ONLY
	if ( mi_validToConvert )  // check just in case
	 {
		mi_recordCodecIndex = m_PlugIn.getIndex();
		if ( NO_CODEC_FOUND == mi_recordCodecIndex )
		 {
			m_ErrorDisplay.ReportError( LS_ERROR_NO_CODEC );
			return FALSE;
		 }

		mb_ConvDone = FALSE;
		// need to delete CConvert before CACodec
		delete mp_Converter; mp_Converter = NULL;
		delete mp_Codec; mp_Codec = NULL;
		delete mp_WaveOut; mp_WaveOut = NULL;
		WorkFileName( getTempFileName() );
		mp_Codec = m_PlugIn.getCodec( mi_recordCodecIndex );
		mp_Converter = new CConvert( ms_WorkFileNameLast, WorkFileName(),
								mp_Codec, &mb_ConvDone, mb_UseYield );
		if ( ENCODE == mi_validToConvert )
		 {
			mp_Converter->Encode( mi_hogCpu );
			ml_rate = m_PlugIn.getCodecInfo(mi_recordCodecIndex)->avgBitsPerSec;
		 }
		else
		 {
			mp_Converter->Decode( TRUE );
			ml_rate = DEFAULT_RATE;
		 }
		ml_DispFinalSize = 0;
	 }
	else return FALSE;
#endif
	return TRUE;
}

BOOL LsStateAction::OnRecordEncode()
{
#ifndef LS_PLAYER_ONLY
	mp_WaveIn->Stop();
#endif
	return TRUE;
}

BOOL LsStateAction::OnPause()
{
	switch( LastState() )
	 {
		case LsStateMachine::PLAY:
		    OnIdle();
            ml_PlayIndex = mp_WaveOut->getPosition( UNIT_TIME_MS );
            ml_RecordIndex = ml_PlayIndex;
         break;
		case LsStateMachine::RECORD:
			mp_WaveIn->Pause();
		 break;
	 }

	return TRUE;
}

BOOL LsStateAction::OnIdle()
{
	LsStateType relevantState = LastState();

	if ( relevantState == LsStateMachine::PAUSE )
	 {
		relevantState = LastState(1);
	 }

#ifndef LS_PLAYER_ONLY
	if ( State() == LsStateMachine::END )
	 {
		// make sure we stop codec conversion if realtime
		if ( (relevantState == LsStateMachine::RECORD) &&
			 (RecordMode() == REALTIME) )
		 {
			mp_WaveIn->Stop();
			relevantState = LsStateMachine::RECORDENCODE;
		 }

		// save this session's state of InsertMode 
		WRITE_PROFILE_INSERT( EditInsertMode() );

		// save this session's playback speed
		WRITE_PROFILE_PLAYSPEED( PlaySpeed() );

		// save our codec preference for next session
		const t_qpl_codec_info* info = m_PlugIn.getCodecInfo(mi_recordCodecIndex);
		if ( info )
		 {
			WRITE_PROFILE_CODEC(info->Id);
		 }
//		else
//		 {
//			// must mean no compression
//			WRITE_PROFILE_CODEC(NO_COMPRESSION_REGISTRY);
//		 }
/*
		// if not using MAPI(i.e. called from Eudora) then always give
		// the user one last chance to attach when quitting
		if ( ( LsStateMachine::ATTACH != LastState() ) &&
			 NewFlag() &&
			!((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.mb_UseMapi )
		 {
			mp_Doc->SetModifiedFlag();
		 }
*/
	 }

	if ( (relevantState == LsStateMachine::RECORD)
		 && mp_WaveIn )
	 {
		mp_WaveIn->Stop();
		PlayMode( RecordMode() );

		CWaveFile file;
		RecordEdit( file );

		CheckFile( WorkFileName() );	// for disp update
		ml_DispTotTime = LenPlayTotMs();
		mp_Doc->SetModifiedFlag();
		NewFlag( TRUE );
   	 }

	if ( ((relevantState == LsStateMachine::ENCODE) ||
		  (relevantState == LsStateMachine::RECORDENCODE))
		 && mp_Converter )
	 {
		if (mp_State->Param() == 0)
		 {
			mp_Converter->Stop();
		 }
		PlayMode( REALTIME ); 
		if ( relevantState == LsStateMachine::RECORDENCODE )
		 {
			CQcelpFile file;
			RecordEdit( file );
		 }
		else
		 {
			deleteTempFiles( LAST );
			if ( ENCODE == mi_validToConvert )
				mi_codecMenuIndex = m_PlugIn.getIndex() + 1;
			else
				mi_codecMenuIndex = 0;
			OnCodecMenuRange( mi_codecMenuIndex + CHECK_MENU_RANGE_BEGIN );
		 }
		CheckFile( WorkFileName() );		// for disp update
		ml_DispTotTime = LenPlayTotMs();
		mp_Doc->SetModifiedFlag();
		NewFlag( TRUE );
	}
#endif
	if ( (relevantState == LsStateMachine::PLAY)
		 && mp_WaveOut )
	 {
        if ( !FromMmWomClose() )
         {
            // Kludge: Wait until MM_WOM_CLOSE message processing
            //         is complete before we play again
            //         The order of function calls is important
            //         to maintain syncronization.
            int ret = mp_WaveOut->Stop();
			if ( ret < 0 )
			 {
				return FALSE;
			 }
			else if ( ret > 0 )
			 {
				while( !FromMmWomClose() )
				 {
					((CQVoiceApp *)AfxGetApp())->YieldEvents();
					TRACE("I'm still waiting on MM_WOM_CLOSE\n");
				 }
			 }
         }
        FromMmWomClose( FALSE );
	 }

	if ( relevantState != LsStateMachine::REPOSITION )
	{
		ml_PlayIndex = 0;
		ml_RecordIndex = 0;
	}

	return TRUE;
}

BOOL LsStateAction::OnOpenFile()
{
	CString origFileName = LPCTSTR(mp_State->Param());
	CString FileName = origFileName;

	// check to see if the file is valid
	if (!CheckFile( FileName )) return FALSE;

	// get rid of any outstanding tmp files
	deleteTempFiles();

	if ( FileName != origFileName )
		WorkFileName( FileName );
	else	
		WorkFileName( origFileName );

	ms_OpenFileName = origFileName;

	//mp_Doc->SetModifiedFlag();
	NewFlag( TRUE );
	ml_DispTotTime = LenPlayTotMs();

	// update plugin display and set current codec
	if ( NO_CODEC == mi_playCodecIndex )
		mi_codecMenuIndex = 0;
	else 
		mi_codecMenuIndex = mi_playCodecIndex + 1;

	long save_rate = ml_rate;

	OnCodecMenuRange( mi_codecMenuIndex + CHECK_MENU_RANGE_BEGIN );

	// this is so that state Play will have valid HWND
	POSITION pos = mp_Doc->GetFirstViewPosition();
	CQVoiceView* pView = (CQVoiceView*)	(mp_Doc->GetNextView( pos ));
	mp_State->Param(pView->m_hWnd);

	delete mp_WaveOut; mp_WaveOut = NULL;

	if ( NO_CODEC == mi_playCodecIndex )
	 {
		// if playing wave file, restore ml_rate since the above call
		// will change it
		ml_rate = save_rate;
		// if this file is a wave file, make sure we save the
		// previous mi_recordCodecIndex so we can restore once we do New
		mi_recordCodecIndexOld = mi_recordCodecIndex;
	 }

	return TRUE;
}

BOOL LsStateAction::OnSaveAs()
{
	CString FileName = LPCTSTR(mp_State->Param());
	TRACE1("LsStateAction::OnSaveAs - %s\n", LPCTSTR(FileName));

	::CopyFile( LPCTSTR(ms_WorkFileName), LPCTSTR(FileName), FALSE );

	mp_Doc->SetModifiedFlag( FALSE );
	return TRUE;
}

BOOL LsStateAction::OnNewRecord()
{
#ifndef LS_PLAYER_ONLY
	mp_Doc->OnNewDocument();
	deleteTempFiles();
	ml_DispFinalSize = 0;
	// if previous file was a wave file, make sure we restore
	// mi_recordCodecIndex
	if ( mb_noCodec )
	 {
		mi_recordCodecIndex = mi_recordCodecIndexOld;
		m_PlugIn.setIndex( mi_recordCodecIndex );
		// update plugin display and set current codec
		mi_codecMenuIndex = mi_recordCodecIndex + 1;
		OnCodecMenuRange( mi_codecMenuIndex + CHECK_MENU_RANGE_BEGIN );
	 }
	((CMainFrame*)AfxGetMainWnd())->m_wndNameBar.Name( getUniqueName() );
#endif
	return TRUE;
}

BOOL LsStateAction::OnAttach() 
{
#ifndef LS_PLAYER_ONLY
	delete mp_WaveOut; mp_WaveOut = NULL;

	CMainFrame* pMf = (CMainFrame*)AfxGetMainWnd();
	CQVoiceApp* pApp = (CQVoiceApp *)AfxGetApp();
	BOOL bUseMapi = ((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.mb_UseMapi;

	CString finalFullName; finalFullName.GetBuffer(_MAX_PATH);
	finalFullName = pApp->m_cmdLineInfo.m_strOutFileName;
	if ( finalFullName.IsEmpty() || bUseMapi )
	 {
		CString fileName = pMf->m_wndNameBar.Name();
		fileName.TrimLeft();
		// Check if file exist or not
		if ( !fileName.IsEmpty() )
		 {
			OFSTRUCT junk;
			CString fullName =
				pApp->m_cmdLineInfo.m_strOutPathName;
			if ( fullName.IsEmpty() || bUseMapi )
				fullName = fileName;
			else
				fullName = fullName + fileName;

			// do we need to add ".qcp" extension?
			CString tmp = fullName.Right(4);
			tmp.MakeUpper();
			if ( CString(_T(".QCP")) != tmp )
				fullName += CString(_T(".qcp"));

			if ( !bUseMapi )
			if ( HFILE_ERROR != OpenFile( fullName, &junk, OF_EXIST) )
			 {
				CString errMsg;	errMsg.LoadString( LS_ERROR_FILE_EXIST );
				char err[256];
				sprintf( err, errMsg, fileName );
				
				if ( IDNO == AfxMessageBox( err, MB_YESNO ) )
				 {
					pMf->m_wndNameBar.Name( fileName );
					return FALSE;
				 }
			 }
			else
			 {
				DWORD err = GetLastError();
				if ( (err != NO_ERROR) && (err != ERROR_FILE_NOT_FOUND) )
				 {
					#ifdef LS_DEBUG0
						LsDebug err;
						err.SystemErrorMsgBox("OpenFile(OF_EXIST) Failed");
					#endif
					m_ErrorDisplay.ReportError( LS_ERROR_INVALID_NAME );
					return FALSE;
				 }
			 }
			finalFullName = fullName;
		 }
		else
		 {
			m_ErrorDisplay.ReportError( LS_ERROR_EMPTY_NAME );
			return FALSE;
		 }
	 }
	else
	 {
		int t = 0;
		while (t<100)
		 {
			OFSTRUCT junk;
			if ( HFILE_ERROR != OpenFile( finalFullName, &junk, OF_EXIST) )
			 {
				t++;
				finalFullName.Format( _T("%s%d"), finalFullName, t );
			 }
			else
				break;
		 }
	 }

	if ( !bUseMapi )
	 {	// no MAPI
		// if we are working with orig file then do a copy instead of move
		BOOL err;
		if ( ms_OpenFileName == WorkFileName() )
		 {
			err = ::CopyFile( LPCTSTR(WorkFileName()),
						LPCTSTR(finalFullName), FALSE );
		 }
		else
		 {
			err = ::MoveFileEx( LPCTSTR(WorkFileName()),
						LPCTSTR(finalFullName), 
						MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED );
			// ok, most likely we're using Win95 which don't support
			// MoveFileEx
			if ( !err )
			 {
				err = ::MoveFile( LPCTSTR(WorkFileName()),
											LPCTSTR(finalFullName) );
				// ok, most lilely we're doing cross volume move,
				// so do copy and delete
				if ( !err )
				 {
					err = ::CopyFile( LPCTSTR(WorkFileName()),
											LPCTSTR(finalFullName), FALSE );
					if ( err )
						err = ::DeleteFile( LPCTSTR(WorkFileName()) );
				 }
			 }
		 }
		if ( !err )
		 {
			DWORD err = GetLastError();
			if ( (ERROR_PATH_NOT_FOUND == err) || (ERROR_INVALID_NAME == err) )
			 {
				m_ErrorDisplay.ReportError( LS_ERROR_INVALID_NAME );
				return FALSE;
			 }
				
		 }
#ifdef LS_DEBUG0
		if ( !err )
		 {
			LsDebug err;
			err.SystemErrorMsgBox("CopyFile/MoveFile/DeleteFile Failed");
		 }
#endif
		pMf->SendFileName2Eudora( finalFullName, mb_attachDesc );
		ms_WorkFileName.Empty();
	 }
	else // use MAPI
	 {
		CString title = finalFullName;

/*		while (1)
		 {
			FILE* fp;
			if ( fp = fopen( LPCTSTR(title), "r" ) )
			 {
				time++;
				title.Format( "%X%d%d", t.GetMonth(), t.GetDay(), time );
			 }
			else
				break;
		 }
*/
		// prepare the file description (for the attachment)
		MapiFileDesc fileDesc;
		memset(&fileDesc, 0, sizeof(fileDesc));
		fileDesc.nPosition = (ULONG)-1;	
		fileDesc.lpszPathName = LPSTR(LPCTSTR(WorkFileName()));
		fileDesc.lpszFileName = LPSTR(LPCTSTR(title));
	/*
		if ( PlayMode() == REALTIME )
		 {
			fileDesc.lpszFileName = LPSTR(LPCTSTR(title + _T(".QCP")));
		 }
		else
		 {
			fileDesc.lpszFileName = LPSTR(LPCTSTR(title + _T(".WAV")));
		 }	
	*/
		// prepare the message (empty with 1 attachment)
		MapiMessage message;
		memset(&message, 0, sizeof(message));
		CString strDirection; 
		if ( mb_attachDesc )
		 {
			strDirection.LoadString( IDS_README );
			message.lpszNoteText = LPTSTR(LPCTSTR(strDirection));
		 }
		message.nFileCount = 1;
		message.lpFiles = &fileDesc;
		
		ULONG (PASCAL *lpfnSendMail)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);

		HINSTANCE hInstMAPIDLL;
		hInstMAPIDLL = ::LoadLibraryA(_T("MAPI32.DLL"));

		(FARPROC&)lpfnSendMail = GetProcAddress(hInstMAPIDLL, _T("MAPISendMail"));
		
		if (lpfnSendMail == NULL)
		 {
			::FreeLibrary( hInstMAPIDLL );
			//AfxMessageBox(AFX_IDP_INVALID_MAPI_DLL);
			m_ErrorDisplay.ReportError( LS_ERROR_INVALID_MAPI );
			return FALSE;
		 }

		int nError = lpfnSendMail(0, 0,
			&message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

		::FreeLibrary( hInstMAPIDLL );

		if ( nError != SUCCESS_SUCCESS )// &&
			//nError != MAPI_USER_ABORT && nError != MAPI_E_LOGIN_FAILURE)
		 {
			//AfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
			m_ErrorDisplay.ReportError( LS_ERROR_MAPI_SEND_FAIL );
			return FALSE;
		 }
	}

	mp_Doc->SetModifiedFlag( FALSE );
	NewFlag( FALSE );
#endif
	return TRUE;
}

BOOL LsStateAction::SearchPlugins()
{
	if ( m_srchPluginFlag > -1 )
	 {
		return m_srchPluginFlag;
	 }

	if (!((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.m_strDllPathName.IsEmpty()
		)
	 {
//		extName = dirName.Right(dirName.ReverseFind( '\\*.' ));
//		dirName = dirName.Left(dirName.ReverseFind( '\\*.' ));
		m_PlugIn.setPath( 
		 LPCTSTR(((CQVoiceApp *)AfxGetApp())->m_cmdLineInfo.m_strDllPathName),
		 CODEC_PLUGIN_EXT
		);
	 }
	else
	 {
		m_PlugIn.setPath( NULL, CODEC_PLUGIN_EXT );
	 }

	m_srchPluginFlag = m_PlugIn.findCodecs();

	return (m_srchPluginFlag > 0);
}

void LsStateAction::srchCodecUpdateMenu()
{
	SearchPlugins();
#ifndef LS_PLAYER_ONLY
	delete mp_CodecMenu;
	mp_CodecMenu = new CCodecMenu( 2, 0, CHECK_MENU_RANGE_BEGIN + 1 );

#ifdef LS_TOOLBAR_COMBOBOX
	((CMainFrame*)AfxGetMainWnd())->m_wndToolBar.m_cmbCodec.ResetContent();
	((CMainFrame*)AfxGetMainWnd())->m_wndToolBar.m_cmbCodec.AddString(
							_T("No Compression"));
#endif
	// put avail codec names in the menu
	for ( int i = 0; i < m_PlugIn.getNumOfCodecs(); i++ )
	 {
		mp_CodecMenu->appendItem(m_PlugIn.getCodecName(i));
#ifdef LS_TOOLBAR_COMBOBOX
		((CMainFrame*)AfxGetMainWnd())->m_wndToolBar.m_cmbCodec.AddString(
							m_PlugIn.getCodecName(i));
#endif
	 }
#endif
	if ( (NO_CODEC != mi_recordCodecIndex) &&
		 (NO_CODEC_FOUND != mi_recordCodecIndex) )
	 {
		mi_codecMenuIndex = 1 + mi_recordCodecIndex;
		ml_rate = m_PlugIn.getCodecInfo(mi_recordCodecIndex)->avgBitsPerSec;
	 }
	else
	 {
		mb_noCodec = TRUE;
		ml_rate	   = DEFAULT_RATE;
	 }
#if !defined( LS_PLAYER_ONLY ) && defined( LS_TOOLBAR_COMBOBOX )
	((CMainFrame*)AfxGetMainWnd())->m_wndToolBar.m_cmbCodec.SetCurSel(
						mi_codecMenuIndex);
#endif

}
 
//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

/*inline*/ LsStateType LsStateAction::LastState( int last /* = 0 */)
{
	return mp_State->LastState(last); 
}

/*inline*/ LsStateType LsStateAction::State()
{
	return mp_State->State(); 
}

inline LsStateType LsStateAction::NextState()
{ 
	return mp_State->NextState(); 
}

long LsStateAction::PlaySpeed( long speed )
{
	long s = m_playSpeed;
	m_playSpeed = speed;
	if ( (LsStateMachine::PLAY == State()) && (REALTIME == PlayMode())
		&& mp_WaveOut )
		((CWaveOutDecode *)mp_WaveOut)->PlaySpeed( m_playSpeed );
	return s;
}

long LsStateAction::PlaySpeed()
{
	return m_playSpeed;
}

long LsStateAction::Volume( long volume )
{
	long v = ml_volume;
	ml_volume = volume;
	if (mp_WaveOut) mp_WaveOut->Gain( volume );
	return v;
}

long LsStateAction::Volume()
{
	return ml_volume;
}

/*inline*/ BOOL LsStateAction::NewFlag()
{
	return mb_NewFlag;
}

inline BOOL LsStateAction::NewFlag( BOOL flag )
{
	BOOL old = mb_NewFlag;
	mb_NewFlag = flag;
	return old;
}

/*inline*/ long LsStateAction::FinalSize() 
{
	if ( mp_Converter && (State() != LsStateMachine::IDLE) )
		return (mp_Converter->ml_lengthVariable + ml_DispFinalSize)/1024;
	else
		return ml_DispFinalSize/1024;
}

/*inline*/ long LsStateAction::LenCurMs() 
{
	return mp_WaveIn->getPosition(UNIT_TIME_MS) + ml_RecordIndex;
}

/*inline*/ long LsStateAction::LenTotMs() 
{
	return !EditInsertMode() ?
/*
		if ( ml_DispTotTime >
				 (mp_WaveIn->getPosition(UNIT_TIME_MS) + ml_RecordIndex) )
		 {
			return ml_DispTotTime;
		 }
		 else
			return mp_WaveIn->getPosition(UNIT_TIME_MS) + ml_RecordIndex;
*/
		(mp_WaveIn->getPosition(UNIT_TIME_MS) + ml_RecordIndex) :
		(mp_WaveIn->getPosition(UNIT_TIME_MS) + ml_DispTotTime);
}

/*inline*/ long LsStateAction::LenPlayCurMs() 
{
	return mp_WaveOut->getPosition(UNIT_TIME_MS);
}

/*inline*/ long LsStateAction::LenPlayTotMs()
{
	return ml_PlayLenMs;
}

/*inline*/ long LsStateAction::LenRecordCurMs()
{
	return mp_WaveIn->getPosition(UNIT_TIME_MS) + ml_RecordIndex;
}

/*inline*/ long LsStateAction::LenRecordTotMs()
{
	return mp_WaveIn->getTotalLength(UNIT_TIME_MS) + ml_RecordIndex;
}

/*inline*/ int LsStateAction::Rate()
{
	return ml_rate/1000;
}

/*inline*/ void LsStateAction::FromMmWomClose( BOOL val )
{
	mb_FromMmWomClose = val;
}

inline BOOL LsStateAction::FromMmWomClose()
{
	return mb_FromMmWomClose;
}

/*inline*/ int LsStateAction::PlayMode()
{
	return me_playMode;
}

/*inline*/ void LsStateAction::PlayMode( int mode )
{
	me_playMode = mode;
}

/*inline*/ int LsStateAction::RecordMode()
{
	return me_recordMode;
}

/*inline*/ void LsStateAction::RecordMode( int mode )
{
	me_recordMode = mode;
}

/*inline*/ BOOL LsStateAction::ToggleEditInsertMode()
{
	mb_editInsertMode = !mb_editInsertMode;
	return !mb_editInsertMode;
}

/*inline*/ BOOL LsStateAction::EditInsertMode()
{
	return mb_editInsertMode;
}

//////////////////////////////////////////////////////////////////////////////
// INQUIRY
//////////

int LsStateAction::UpdateProgress()
{
#ifndef LS_PLAYER_ONLY
	// ok, we're done, so convey it to our view
	if ( mb_ConvDone )
		return -1;
	
	// in case UpdateProgress was called before mp_Converter became.
	if ( mp_Converter == NULL )
		return 0;

	return (mp_Converter->getLength()) ? 100*mp_Converter->getIndex()/
			mp_Converter->getLength() : 0;
#else
	return 0;
#endif
}

BOOL LsStateAction::CheckFile( CString& fileName )
{
	CQcelpFile checkFile;
	mi_validToConvert = FALSE;
	if ( checkFile.Open( LPCTSTR(fileName), CmmioFile::CHECK ) )
	 {
		// check File Format Version
		BOOL bNewer = FALSE;
		if ( checkFile.FileVersionMajor() > FILE_VERSION_MAJOR )
			bNewer = TRUE;
		else if ( (checkFile.FileVersionMajor() == FILE_VERSION_MAJOR) && 
				  (checkFile.FileVersionMinor() > FILE_VERSION_MINOR) )
			bNewer = TRUE;
		if ( bNewer )
		 {
/*
			CString Err, Err1;
			Err.Format( "This version of %s cannot read this newer file",
				AfxGetAppName() );
			Err1.Format( "%s format(v%d.%d). Please obtain a newer version",
				LPCTSTR(Err), checkFile.FileVersionMajor(), 
				checkFile.FileVersionMinor() );
			Err.Format( "%s of %s", LPCTSTR(Err1), AfxGetAppName() );
*/
			CString Err; Err.GetBuffer(256);
			Err.Format( LS_ERROR_FILE_NEWER, AfxGetAppName(),
				checkFile.FileVersionMajor(), checkFile.FileVersionMinor(),
				AfxGetAppName() );
			m_ErrorDisplay.ReportError( Err );
			return FALSE;
		 }

		if ( m_PlugIn.isExistCodec(
					*((t_qpl_codec_info*)checkFile.getInfo())) )
		 {
			mi_playCodecIndex = m_PlugIn.getIndex(
								*((t_qpl_codec_info*)checkFile.getInfo()) );
			PlayMode( REALTIME );

			if ( State() == LsStateMachine::OPENFILE )
			 {
				// check if this was an outdated file, if so, convert
				// to a temp file
				if ( (checkFile.FileVersionMajor() == 0) && 
					 (checkFile.FileVersionMinor() == 9) )	
				 {
					m_ErrorDisplay.ReportError( LS_ERROR_OLDER_FILE_FORMAT );
					return FALSE;
				 }
					 
/*				if ( checkFile.Convert2New(LPCTSTR(getTempFileName())) )
				 {
					fileName = ms_TempFileName;
				 }
*/
			 }

			ml_PlayLenMs  = checkFile.getDataLen( UNIT_TIME_MS );
			ml_DispFinalSize = checkFile.getDataLen();
            ml_rate       = ((t_qpl_codec_info*)
									checkFile.getInfo())->avgBitsPerSec;
			mi_validToConvert = DECODE;
		 }
		else
		 {
			CString Err; Err.GetBuffer(256);
			Err.Format( LS_ERROR_CODEC_NOT_FOUND,
				((t_qpl_codec_info*)checkFile.getInfo())->Name,
				((t_qpl_codec_info*)checkFile.getInfo())->Version );
			m_ErrorDisplay.ReportError( Err );
			return FALSE;
		 }
	 }
	else if ( checkFile.getError() == CmmioFile::FILE_NO_DATA )
	 {
		return FALSE; // nothing to play, no data
	 }
	else
	 {
		CWaveFile checkFile;
		if (!checkFile.Open( LPCTSTR(fileName), CmmioFile::CHECK ))
		{
			if (checkFile.getError() == CWaveFile::DEVICE_ERROR)
				m_ErrorDisplay.ReportError( ERROR_MM_CANT_OPEN_DEVICE );
			else
			 {
				m_ErrorDisplay.ReportError( LS_ERROR_INVALID_FILE );
			 }
			return FALSE;
		}
		else
		{
			PlayMode( FILE );

			mi_playCodecIndex = NO_CODEC; // no compression used
			ml_PlayLenMs = checkFile.getDataLen( UNIT_TIME_MS );
			ml_DispFinalSize = checkFile.getDataLen();
			WAVEFORMATEX* pWvfmt = (WAVEFORMATEX *)checkFile.getInfo();
			ml_rate = pWvfmt->nAvgBytesPerSec*8;
			if ( (pWvfmt->wFormatTag == WAVE_FORMAT_PCM) &&
				 (pWvfmt->nSamplesPerSec == 8000) &&
				 (pWvfmt->wBitsPerSample == 16) &&
				 (pWvfmt->nChannels = 1) )
				mi_validToConvert = ENCODE;
		}
	 }

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOL LsStateAction::RecordEdit( CmmioFile& file )
{
#ifndef LS_PLAYER_ONLY
	// check if we have something to edit first
	if ( ms_WorkFileNameLast.IsEmpty() ) return TRUE;

	// if we're overwriting the whole file, just
	// delete the old one
	if ( (ml_RecordIndex == 0) && !EditInsertMode() ) 
	 {
		deleteTempFiles( LAST );
		return TRUE;
	 }

	// also check if these are the same file
	if ( ms_WorkFileName == ms_WorkFileNameLast ) return TRUE;
	if ( ms_WorkFileNameLast == ms_OpenFileName )
	 {
		ms_WorkFileNameLast = getTempFileName();
		CopyFile( ms_OpenFileName, ms_WorkFileNameLast, FALSE );
	 }

	// if the orig file is invalid, get out
	if ( !file.Open( LPCTSTR(ms_WorkFileNameLast), CmmioFile::CHECK ) )
		return TRUE;

	if ( EditInsertMode() )
	 {
		file.Insert( LPCTSTR(ms_WorkFileName),
					ml_RecordIndex, UNIT_TIME_MS );
	 }
	else
	 {
		file.OverWrite( LPCTSTR(ms_WorkFileName),
					ml_RecordIndex, UNIT_TIME_MS );
	 }
	deleteTempFiles( CUR );
	ms_WorkFileName = ms_WorkFileNameLast;
	ms_WorkFileNameLast.Empty();

#endif
	return TRUE;
}

void LsStateAction::WorkFileName( CString& fn )
{
	ms_WorkFileNameLast = ms_WorkFileName;
	ms_WorkFileName = fn;
}

CString &LsStateAction::WorkFileName()
{
	return ms_WorkFileName;
}

CString& LsStateAction::getTempFileName()
{
	char tempName[MAX_PATH];
	char tempPath[MAX_PATH];

	::GetTempPath( sizeof(tempPath), tempPath );
	::GetTempFileName( tempPath, _T("~qc"), 0, tempName );
	::DeleteFile( tempName );

	ms_TempFileName = tempName;

	return ms_TempFileName;
}

void LsStateAction::deleteTempFiles( int mode /* = ALL */)
{
	// Make sure we delete CWaveOutDecode before deleting temp files.
	// Deleting CWaveOutDecode will delete CConvert which in turn
	// closes the file so we can delete it
	delete mp_WaveOut; mp_WaveOut = NULL;
	delete mp_Converter; mp_Converter = NULL;

	if ( (mode == LAST) || (mode == ALL) )
	 {
		if ( (ms_WorkFileNameLast != ms_OpenFileName) &&
			 (ms_WorkFileNameLast != ms_SaveFileName) )
		 {
			::DeleteFile( LPCTSTR(ms_WorkFileNameLast) );
		 }
		ms_WorkFileNameLast.Empty();
	 }

	if ( (mode == CUR) || (mode == ALL) )
	 {
		if ( (ms_WorkFileName != ms_OpenFileName) &&
			 (ms_WorkFileName != ms_SaveFileName) )
		 {
			::DeleteFile( LPCTSTR(ms_WorkFileName) );
		 }
		ms_WorkFileName.Empty();
	 }
}

CString LsStateAction::getUniqueName()
{
	CString title; title.GetBuffer(_MAX_PATH);
	CTime t = CTime::GetCurrentTime();
	long time = t.GetHour()*3600 + t.GetMinute()*60 + t.GetSecond();
	int  day  = t.GetDay();
	int  year = t.GetYear()-1990;

	if ( day < 10 )
		day = '0' + day;
	else
		day = 'A' + day - 10;

	if ( year < 0 )
		year = 0;
	else if ( year > 15 )
		year = 15;

	title.Format( _T("%X%X%c%d"), year, t.GetMonth(), day, time );
	// make sure this name does not exist
	int i = 10;
	while ( i-- )
	 {
		OFSTRUCT junk;
		if ( HFILE_ERROR != OpenFile( title, &junk, OF_EXIST) )
		 {
			time++;
			title.Format( _T("%X%d%d"), t.GetMonth(), t.GetDay(), time );
		 }
		else
			break;
	 }

	return title;
}

//////////////////////////////////////////////////////////////////////////
// Window Message Handlers for CAWave objects
/////////////////////////////////////////////

LRESULT LsStateAction::_record(WPARAM wParam, LPARAM lParam)
{
#ifndef LS_PLAYER_ONLY
	SSoundMsg msg;
	msg.wParam = wParam;
	msg.lParam = lParam;
	
	return mp_WaveIn->_record(&msg);
#else
	return 0L;
#endif
}

LRESULT LsStateAction::_play(WPARAM wParam, LPARAM lParam)
{
	SSoundMsg msg;
	msg.wParam = wParam;
	msg.lParam = lParam;
	
	return mp_WaveOut->_play(&msg);
}

//////////////////////////////////////////////////////////////////////////

void LsStateAction::OnCodecMenuRange(UINT nID) 
{
#ifndef LS_PLAYER_ONLY
	ASSERT( nID >= CHECK_MENU_RANGE_BEGIN && nID <= CHECK_MENU_RANGE_END );
	mi_codecMenuIndex = nID - CHECK_MENU_RANGE_BEGIN;
#ifdef LS_TOOLBAR_COMBOBOX
	((CMainFrame*)AfxGetMainWnd())->m_wndToolBar.m_cmbCodec.SetCurSel(
						mi_codecMenuIndex);
#endif
	if ( 0 == mi_codecMenuIndex )
	 {
		mb_noCodec = TRUE;
		ml_rate = DEFAULT_RATE;
	 }
	else
	 {
		m_PlugIn.setIndex( mi_codecMenuIndex - 1 );
		ml_rate = m_PlugIn.getCodecInfo()->avgBitsPerSec;
		mb_noCodec = FALSE;
	 }
	POSITION pos = mp_Doc->GetFirstViewPosition();
	((CQVoiceView*)mp_Doc->GetNextView(pos))->m_playPic.SetRate( Rate() );
#endif
}

void LsStateAction::OnUpdateCodecMenuRange(CCmdUI* pCmdUI) 
{
#ifndef LS_PLAYER_ONLY
	UINT nID = pCmdUI->m_nID;
	ASSERT( nID >= CHECK_MENU_RANGE_BEGIN && nID <= CHECK_MENU_RANGE_END );
	pCmdUI->SetCheck( (nID - CHECK_MENU_RANGE_BEGIN) == mi_codecMenuIndex );
	pCmdUI->Enable( State() == LsStateMachine::INITIAL );
#endif
}

void LsStateAction::OnSelchangeCmbCodec()
{
#if !defined( LS_PLAYER_ONLY ) && defined( LS_TOOLBAR_COMBOBOX )
	mi_codecMenuIndex 
		= ((CMainFrame*)AfxGetMainWnd())->m_wndToolBar.m_cmbCodec.GetCurSel();
	if ( 0 == mi_codecMenuIndex )
	 {
		mb_noCodec = TRUE;
		ml_rate = DEFAULT_RATE;
	 }
	else
	 {
		m_PlugIn.setIndex( mi_codecMenuIndex - 1 );
		ml_rate = m_PlugIn.getCodecInfo()->avgBitsPerSec;
		mb_noCodec = FALSE;
	 }
	POSITION pos = mp_Doc->GetFirstViewPosition();
	((CQVoiceView*)mp_Doc->GetNextView(pos))->m_playPic.SetRate( Rate() );
#endif
}
