/*////////////////////////////////////////////////////////////////////////////

NAME:
	LsStateAction - 

FILE:		LsStateAction.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/??/96   lss     -Initial
03/24/97   lss     -Added PlaySpeed()  
04/01/97   lss     -Added mi_hogCpu

/*////////////////////////////////////////////////////////////////////////////

#ifndef _LSSTATEACTION_H_
#define _LSSTATEACTION_H_

#include "CPlugIn.h"
#include "ErrorHandler.h"

// FORWARD DECLARATION
class CmmioFile;
class CPlugIn;
class CCodecMenu;
class CACodec;
class CAWave;
class CConvert;
class CSndBuffer;
class CErrorHandler;
class LsStateMachine;


class LsStateAction
{
public:
//	LIFECYCLE
	LsStateAction();
	void Init( LsStateMachine& State, CDocument* doc );
	~LsStateAction();
	// Copy ctor or assignment operator not allowed

//	OPERATIONS
	BOOL UpdateView();

	BOOL OnInitial();
	BOOL PostOnInitial();
	BOOL OnIdle();
	BOOL OnPlay();
	BOOL OnRecord();
	BOOL OnPause();
	BOOL OnReposition();
	BOOL OnOpenFile();
	BOOL OnAttach();
	BOOL OnEncode();
	BOOL OnRecordEncode();
	BOOL OnNewRecord();
	BOOL OnSaveAs();

	BOOL SearchPlugins();
	void srchCodecUpdateMenu();

//	ACCESS
	LsStateType NextState();
	LsStateType State();
	LsStateType LastState( int last  = 0 );

	BOOL ToggleEditInsertMode();
	BOOL EditInsertMode();
	BOOL NewFlag();
	BOOL NewFlag( BOOL flag );

	long Volume();
	long Volume( long volume );
	long PlaySpeed();
	long PlaySpeed( long speed );
	long FinalSize();
	long LenCurMs();
	long LenTotMs();
	long LenPlayCurMs();
	long LenPlayTotMs();
	long LenRecordCurMs(); 
	long LenRecordTotMs();
    int  Rate();
	void FromMmWomClose( BOOL val );
	BOOL FromMmWomClose();
	int  PlayMode();
	void PlayMode( int mode );
	int  RecordMode();
	void RecordMode( int mode );

	BOOL IsValidToConvert(){ return mi_validToConvert; }

	BOOL mb_agcEnable;

	// for PlayMode, RecordMode
	enum{
		NONE,
		MEM,
		FILE,
		REALTIME
	};

//	INQUIRY
	int  UpdateProgress();
	BOOL CheckFile( CString& fileName );

//	SYSTEM PURPOSES
	LRESULT _record(WPARAM wParam, LPARAM lParam);
	LRESULT _play(WPARAM wParam, LPARAM lParam);

	void	OnCodecMenuRange(UINT nID);
	void	OnUpdateCodecMenuRange(CCmdUI* pCmdUI);
	void	OnSelchangeCmbCodec();

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
	BOOL		RecordEdit( CmmioFile& file );
	void		WorkFileName( CString& fn );
	CString&	WorkFileName();
	CString&	getTempFileName();
	void		deleteTempFiles( int mode = ALL );
	CString		getUniqueName();

//	ENUMS
	// for deleteTempFiles()
	enum{
		ALL,
		CUR,
		LAST
	};

	// for mi_validToConvert
	enum{
		ENCODE = 1,
		DECODE
	};

//	ATTRIBUTES
	CDocument*		mp_Doc;

	LsStateMachine* mp_State;
public:
	CPlugIn			m_PlugIn;
private:
	CCodecMenu		*mp_CodecMenu;
	CACodec			*mp_Codec;
	CAWave			*mp_WaveOut;
	CAWave			*mp_WaveIn;
	CConvert		*mp_Converter;
	CSndBuffer		*mp_SndBuf;

    CString			ms_OpenFileName;			// file name for open
	CString			ms_SaveFileName;			// file name for save
	CString			ms_WorkFileName;
	CString			ms_WorkFileNameLast;

	CString			ms_TempFileName;
	
	int		mi_codecMenuIndex;	// indexes into CPlugIn for appropriate codec 
	int		mi_playCodecIndex;	// same as above, but for Play
	int		mi_recordCodecIndex;// same as above, but for Record
	int		mi_recordCodecIndexOld;
	long	ml_PlayLenMs;		// total length (ms) for play
	long	ml_DispTotTime;		// total time display during "insert" record
    long    ml_rate;			// bits/s of current "sound"
	long	ml_DispFinalSize;	// final size in kbytes to be displayed
	long	ml_volume;			// volume setting
	long	m_playSpeed;		// 0 = norm, -1 = slow, 1 = fast

    long    ml_PlayIndex;
	long	ml_RecordIndex;

	int		me_playMode;	// see enum above
	int		me_recordMode;

	BOOL	mb_ConvDone;
    BOOL    mb_UseYield;

	BOOL	mb_noCodec;
    BOOL    mb_FromMmWomClose;

	int		mi_hogCpu;
	BOOL	mb_editInsertMode;
	BOOL	mb_NewFlag;
	BOOL	mb_attachDesc;

	BOOL	mi_validToConvert;

	int		m_srchPluginFlag;

	CErrorHandler	m_ErrorDisplay;

// Do not allow copy ctor or assignment
	LsStateAction( const LsStateAction& );
	LsStateAction& operator=( const LsStateAction& );

};

#endif