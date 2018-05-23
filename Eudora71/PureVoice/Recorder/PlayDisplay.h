/*////////////////////////////////////////////////////////////////////////////

NAME:
	CPlayDisplay - The CD type main display

FILE:		PlayDisplay.h
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	CPlayDisplay is used to control the CD type display for both
	player and recorder. CLEDBitmapDC is used encapsulate LED 'fonts'

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     -Initial
01/13/97   lss     -Added SetMeter for Wave Out/In Meter
03/24/97   lss     -Added SetPlaySpeed for fast/slow playback 
05/31/97   lss     -Added SetPlayPic for simple anim
  
/*////////////////////////////////////////////////////////////////////////////
#ifndef _PLAYDISPLAY_H_
#define _PLAYDISPLAY_H_

//////////////////////////////////////////////////////////////////////////////
// CLEDBitmapDC
//////////////////////////////////////////////////////////////////////////////

class CLEDBitmapDC : public CDC
{
public:
//	LIFECYCLE
	CLEDBitmapDC( LPCTSTR lpszResourceName, CDC *pCDC = NULL );
	CLEDBitmapDC( UINT nIDResource, CDC *pCDC = NULL );
	~CLEDBitmapDC();

//	OPERATIONS
	void TextOut( CDC *pCDC, int x, int y, LPCTSTR lpszString, int nCount );

//	ACCESS
	int GetHeight(){ return mi_height; }
	int GetWidth(){ return mi_width; }

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
	void commonCtor( LPCTSTR lpszResourceName, UINT nIDResource, CDC *pCDC );
	
	void SetDestDC( CDC *pCDC );
	void ReleaseDestDC();

//	ATTRIBUTES
	CBitmap m_bmpLED;
	CBitmap *mp_bmpOld;

	CDC		*mp_DC;
	int		mi_width;
	int		mi_height;

// Do not allow default, copy ctor or assignment
	CLEDBitmapDC();
	CLEDBitmapDC( const CLEDBitmapDC& );
	CLEDBitmapDC& operator=( const CLEDBitmapDC& );
};


//////////////////////////////////////////////////////////////////////////////
// CPlayDisplay
//////////////////////////////////////////////////////////////////////////////

class CPlayDisplay : public CStatic
{
public:
//	LIFECYCLE
	CPlayDisplay();
	~CPlayDisplay();

//	OPERATIONS
	void SetPlayTime( unsigned long lTime = 0, CDC* pCDC = NULL );
	//long GetPlayTime();
	void SetLengthTime( unsigned long lLength = 0, CDC* pCDC = NULL );
	//long GetLengthTime();
    void SetRate( unsigned int rate = 0, CDC* pCDC = NULL );
    void SetSize( unsigned int size = 0, CDC* pCDC = NULL );
	void SetPlaySpeed( int speed = 0, CDC* pCDC = NULL );
	void SetPlayPic( int frame = 0, CDC *pCDC  = NULL );
	
	void SetIndicator( int eIndicator, BOOL bSet = TRUE, CDC *pCDC = NULL );
	//void SetIndicatorPlay( BOOL bSet = TRUE, CDC *pCDC = NULL );
	//void SetIndicatorPause( BOOL bSet = TRUE, CDC *pCDC = NULL );
	//void SetIndicatorCompress( BOOL bSet = TRUE, CDC *pCDC = NULL );

	void SetVolume( unsigned int level , CDC *pCDC = NULL );
	void SetMeter( unsigned int level , CDC *pCDC = NULL );
	//void SetBar();

	enum{
		INDICATOR_NONE = 0,
		INDICATOR_PLAY = 1,
		INDICATOR_RECORD = 2,
		INDICATOR_PAUSE = 4,
		INDICATOR_COMPRESS = 8,
		ALL = 16
	};

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
	void setBar( int x, int y, int level, int max, int orient, CDC *pCDC  );
	void setTime( CLEDBitmapDC *pLed, long lTime, int destx, int desty,
							int colonWidth, CDC* pCDC );
	void setIndicator( CBitmap& bitmap, int destx, int desty, int indicatorw,
							 int indicatorh, int origx, CDC *pCDC );

//	ENUMS
	// for use with setBar's orient
	enum {
		HORZ,
		VERT
	};

//	ATTRIBUTES
	CLEDBitmapDC	m_LED13x20;
	CLEDBitmapDC	m_LED6x11;
	CBitmap			m_bmpIndicators;
	CBitmap			m_bmpPlaySpeeds;
	CBitmap			m_bmpAnim;
	CBitmap			m_bmpBkg;
	int				mi_bkgWidth;
	int				mi_bkgHeight;

	long			ml_playTimeUpdate;
	long			ml_playLengthUpdate;
	int				mi_speedUpdate;
    int             mi_rateUpdate;
    int             mi_sizeUpdate;
	int				mi_playFrame;

	long			ml_volume;
	long			ml_meter;

	int				me_indicators;
//	CString			ms_setting;
//	int				mi_playTimeX, mi_playTimeY;
//	int				mi_playLengthX, mi_playLengthY;

// Do not allow copy ctor or assignment
	CPlayDisplay( const CPlayDisplay& );
	CPlayDisplay& operator=( const CPlayDisplay& );

//////////////////////////////////////////////////////////////////////////////
//	ClassWizard Stuff
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayDisplay)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlayDisplay)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
