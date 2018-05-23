/*////////////////////////////////////////////////////////////////////////////

NAME:
	CPlayDisplay - The CD type main display

FILE:		PlayDisplay.cpp
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:
	CPlayDisplay is used to control the CD type display for both
	player and recorder. CLEDBitmapDC is used encapsulate LED 'fonts'

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.

DEPENDENCIES:


NOTES:

FILES:


REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     -Initial
01/13/97   lss     -Added SetMeter for Wave Out/In Meter
03/24/97   lss     -Added SetPlaySpeed for fast/slow playback 
04/02/97   lss     -Fixed min display whenever it goes over 60
04/14/97   lss     -Added m_bmpPlaySpeeds - SetPlaySpeed is modified
				    to use settings of "FAST", "SLOW", and "STD" instead
					of numberical values
05/08/97   lss     -Change SetPlaySpeed to display bars instead of words

/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#include "qvoice.h"

//	LOCAL INCLUDES
#include "PlayDisplay.h"

//	LOCAL DEFINES
// postions and size for various parts in pixels
#define PLAY_TIME_X				3
#define PLAY_TIME_Y				4
#define PLAY_LENGTH_X			45
#define PLAY_LENGTH_Y			28
#define RATE_X                  120
#define RATE_Y                  6
#define SIZE_X                  RATE_X
#define SIZE_Y                  20
#define SIZE_W					46
#define SIZE_H					15
#define SPEED1_X                136 
#define SPEED1_Y                34 
#define SPEED2_X                SPEED1_X + 12  
#define SPEED2_Y                SPEED1_Y  
#define	VOLUME_X				23
#define VOLUME_Y				43
#define METER_X					4
#define METER_Y					43
#define INDICATOR_W				9
#define INDICATOR_H				11
#define INDICATOR_PLAY_X		87
#define INDICATOR_PLAY_Y		36
#define INDICATOR_RECORD_X		INDICATOR_PLAY_X
#define INDICATOR_RECORD_Y		INDICATOR_PLAY_Y
#define INDICATOR_PAUSE_X		(INDICATOR_PLAY_X+INDICATOR_W+1)
#define INDICATOR_PAUSE_Y		INDICATOR_PLAY_Y
#define INDICATOR_COMPRESS_X	(INDICATOR_PAUSE_X+INDICATOR_W+1)
#define INDICATOR_COMPRESS_Y	INDICATOR_PLAY_Y
#define INDICATOR_BLANK_ORIGX	0
#define INDICATOR_PLAY_ORIGX	INDICATOR_W
#define INDICATOR_PAUSE_ORIGX	(INDICATOR_W*2) 
#define INDICATOR_COMPRESS_ORIGX (INDICATOR_W*3) 
#define INDICATOR_RECORD_ORIGX	(INDICATOR_W*4) 
#define SPEED_W					22
#define SPEED_H					9
#define SPEED_NORM_X			121
#define SPEED_NORM_Y			35
#define SPEED_SLOW_X			SPEED_NORM_X
#define SPEED_SLOW_Y			SPEED_NORM_Y
#define SPEED_FAST_X			SPEED_NORM_X
#define SPEED_FAST_Y			SPEED_NORM_Y
#define SPEED_NORM_ORIGX		0
#define SPEED_SLOW_ORIGX		SPEED_W
#define SPEED_FAST_ORIGX		(SPEED_W*2)
#define	PLAY_ANIM_X				88
#define PLAY_ANIM_Y				5
#define	PLAY_ANIM_W				27
#define PLAY_ANIM_H				27
#define ANIM_FRAME_W			27

#include "DebugNewHelpers.h"

//////////////////////////////////////////////////////////////////////////////
// CPlayDisplay Implementation
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CPlayDisplay::CPlayDisplay() : 
m_LED13x20( IDB_LED13x20 ), m_LED6x11( IDB_LED6x11 ),
ml_playTimeUpdate(0), ml_playLengthUpdate(0), me_indicators(INDICATOR_NONE),
ml_volume(0), mi_rateUpdate(0), mi_sizeUpdate(0), mi_speedUpdate(0), 
mi_playFrame(0)
{
	// load background bmp and get width and height
	m_bmpBkg.LoadBitmap( IDB_PLAYFORM );
	BITMAP bitmap;
	m_bmpBkg.GetObject(sizeof (BITMAP), &bitmap);
	mi_bkgWidth = bitmap.bmWidth;
	mi_bkgHeight = bitmap.bmHeight;

	m_bmpIndicators.LoadBitmap( IDB_PLAY_INDICATORS );
	m_bmpPlaySpeeds.LoadBitmap( IDB_PLAY_SPEED );
	m_bmpAnim.LoadBitmap( IDB_ANIM );
	//new mp_LED13x20 = CLEDBitmapDC( IDB_LED13x20 );
	//new mp_LED6x11 = CLEDBitmapDC( IDB_LED6x11 );
}

CPlayDisplay::~CPlayDisplay()
{
	//delete mp_LED13x20;
	//delete mp_LED6x11;
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

void CPlayDisplay::SetPlayTime( unsigned long lTime /* = 0 */, CDC* pCDC /* = NULL */ ) 
{
	if (!pCDC)
	 {
		if ( ml_playTimeUpdate == lTime ) return;
		ml_playTimeUpdate = lTime;
	 }
	setTime( &m_LED13x20, lTime, PLAY_TIME_X, PLAY_TIME_Y, 4, pCDC );
}

void CPlayDisplay::SetLengthTime( unsigned long lLength /* = 0 */, CDC* pCDC /* = NULL */ )
{
	if (!pCDC)
	 {
		if ( ml_playLengthUpdate == lLength ) return;
		ml_playLengthUpdate = lLength;
	 }	
	setTime( &m_LED6x11, lLength, PLAY_LENGTH_X, PLAY_LENGTH_Y, 3, pCDC );
}

void CPlayDisplay::SetRate( unsigned int rate /* = 0 */, CDC* pCDC /* = NULL */ )
{
   	CClientDC dc(this);	

	if ( rate > 999)
		rate = 999;

    if ( !pCDC )
	 {
		if ( mi_rateUpdate == rate ) return;
		mi_rateUpdate = rate;
        pCDC = &dc;
	 }	

    char strNum[10];
		
	if ( rate == 0 )
		strcpy( strNum, "   ");
	else if ( rate < 10 )
		sprintf( strNum, "  %d", rate );
	else if ( rate < 100 )
		sprintf( strNum, " %d", rate );
    else
        sprintf( strNum, "%d", rate );

    m_LED6x11.TextOut( pCDC, RATE_X, RATE_Y, strNum, strlen(strNum) );
}

void CPlayDisplay::SetSize( unsigned int size /* = 0 */, CDC* pCDC /* = NULL */ )
{
   	CClientDC dc(this);	

	if ( size > 99999 )
		size = 99999;

    if ( !pCDC )
	 {
		if ( mi_sizeUpdate == size ) return;
		mi_sizeUpdate = size;
        pCDC = &dc;
	 }	

/*
	if ( size == 0 )
	 {
		// clear it
		pCDC->FillSolidRect( SIZE_X-2, SIZE_Y-2, SIZE_W, SIZE_H, RGB(0, 0, 0) );
		return;
	 }
*/
    char strNum[6]="    0";
	if ( size == 0 );
	else if ( size < 10 )
		sprintf( strNum, "    %d", size );
	else if ( size < 100 )
		sprintf( strNum, "   %d", size );
	else if ( size < 1000 )
		sprintf( strNum, "  %d", size );
	else if ( size < 10000 )
		sprintf( strNum, " %d", size );
    else
        sprintf( strNum, "%d", size );

    m_LED6x11.TextOut( pCDC, SIZE_X, SIZE_Y, strNum, strlen(strNum) );
}

void CPlayDisplay::SetPlaySpeed( int speed, CDC* pCDC )
{
   	CClientDC dc(this);	
	LS_RANGE( speed, -3, 3 );

    if ( !pCDC )
	 {
		if ( mi_speedUpdate == speed ) return;
		mi_speedUpdate = speed;
        pCDC = &dc;
	 }	
	{
	int x = 124, y =36;
	CRect rect( x, y, x+3, y+7 );
	int offX = rect.Width() + 1, offY = 0;
	DWORD color = RGB(0, 128, 0);
	// clear left 3 "boxes"
	for (int i = 0; i < 3; i++)
	 {
		pCDC->FillSolidRect( LPCRECT(rect), color );
		rect.OffsetRect( offX, offY );
	 }
	// clear middle one
	rect.OffsetRect( 2, offY );
	pCDC->FillSolidRect( LPCRECT(rect), color );
	rect.OffsetRect( rect.Width(), offY );
	pCDC->FillSolidRect( LPCRECT(rect), color );
	rect.OffsetRect( offX+2, offY );
	// now clear 3 right ones
	for (i = 0; i < 3; i++)
	 {
		pCDC->FillSolidRect( LPCRECT(rect), color );
		rect.OffsetRect( offX, offY );
	 }
	// now highlight the appropriate one
	color = RGB(0, 255, 0);
	rect = CRect( x, y, x+3, y+7 );
	if ( speed < 0 ) // slow 
	 {
		rect.OffsetRect( offX*(speed+3), offY );
		pCDC->FillSolidRect( LPCRECT(rect), color );
	 }
	else if ( speed > 0 ) // fast
	 {
		rect.OffsetRect( (offX*5+3)+(offX*(speed-1)), offY );
		pCDC->FillSolidRect( LPCRECT(rect), color );
	 }
	else // normal
	 {
		rect.OffsetRect( offX*3+2, offY );
		pCDC->FillSolidRect( LPCRECT(rect), color );
		rect.OffsetRect( rect.Width(), offY );
		pCDC->FillSolidRect( LPCRECT(rect), color );
	 }
	}
/*
	LS_RANGE( speed, -1, 1 );

    if ( !pCDC )
	 {
		if ( mi_speedUpdate == speed ) return;
		mi_speedUpdate = speed;
        pCDC = &dc;
	 }	

	if ( 1 == speed )		// fast
		setIndicator( m_bmpPlaySpeeds, SPEED_FAST_X, SPEED_FAST_Y, SPEED_W,
				SPEED_H, SPEED_FAST_ORIGX, pCDC );
	else if ( -1 == speed )// slow
		setIndicator( m_bmpPlaySpeeds, SPEED_SLOW_X, SPEED_SLOW_Y, SPEED_W,
				SPEED_H, SPEED_SLOW_ORIGX, pCDC );
	else							// norm
		setIndicator( m_bmpPlaySpeeds, SPEED_NORM_X, SPEED_NORM_Y, SPEED_W,
				SPEED_H, SPEED_NORM_ORIGX, pCDC );
*/
}

void CPlayDisplay::SetVolume( unsigned int level , CDC *pCDC /* = NULL */ )
{/*
	if ( level > 99 ) level = 99;
	if ( !pCDC )
	 {
		ml_volume = level;
	 }
	// scale level range of 0 - 99 to 0 - 16
	level = level * 16 / 99;
	setBar( VOLUME_X, VOLUME_Y, level, 16, HORZ, pCDC );
*/}

void CPlayDisplay::SetMeter( unsigned int level , CDC *pCDC /* = NULL */ )
{
	if ( level > 99 ) level = 99;
	if ( !pCDC )
	 {
		ml_meter = level;
	 }
	// scale level range of 0 - 99 to 0 - 19
	level = level * 19 / 99;
	setBar( METER_X, METER_Y, level, 19, HORZ, pCDC );
}

void CPlayDisplay::SetIndicator( int eIndicator, 
	BOOL bSet /* = TRUE */, CDC *pCDC /* = NULL */ )
{
	if ( !pCDC )
	 {
		// if ALL then erase all no matter what bSet is
		if ( eIndicator == ALL )
		 {
			me_indicators = 0;
			eIndicator = INDICATOR_PLAY | INDICATOR_PAUSE |
						 INDICATOR_RECORD | INDICATOR_COMPRESS;
			bSet = FALSE;
		 }
			
		if ( bSet )
			me_indicators |= eIndicator;
		else
			me_indicators &= ~eIndicator;
	 }
	if ( bSet )
	 {
		if ( eIndicator & INDICATOR_PLAY )
			setIndicator( m_bmpIndicators, INDICATOR_PLAY_X, 
				INDICATOR_PLAY_Y, INDICATOR_W,
				INDICATOR_H, INDICATOR_PLAY_ORIGX, pCDC );
		if ( eIndicator & INDICATOR_PAUSE )
			setIndicator( m_bmpIndicators, INDICATOR_PAUSE_X, 
				INDICATOR_PAUSE_Y, INDICATOR_W,
				INDICATOR_H, INDICATOR_PAUSE_ORIGX, pCDC );
		if ( eIndicator & INDICATOR_RECORD )
			setIndicator( m_bmpIndicators, INDICATOR_RECORD_X,
				INDICATOR_RECORD_Y, INDICATOR_W,
				INDICATOR_H, INDICATOR_RECORD_ORIGX, pCDC );
		if ( eIndicator & INDICATOR_COMPRESS )
			setIndicator( m_bmpIndicators, INDICATOR_COMPRESS_X,
				INDICATOR_COMPRESS_Y, INDICATOR_W, 
				INDICATOR_H, INDICATOR_COMPRESS_ORIGX, pCDC );
	 }
	else
	 {
		if (eIndicator & INDICATOR_PLAY)
			setIndicator( m_bmpIndicators, INDICATOR_PLAY_X,
				INDICATOR_PLAY_Y, INDICATOR_W,
				INDICATOR_H, INDICATOR_BLANK_ORIGX, pCDC );
		if ( eIndicator & INDICATOR_PAUSE )
			setIndicator( m_bmpIndicators, INDICATOR_PAUSE_X,
				INDICATOR_PAUSE_Y, INDICATOR_W,
				INDICATOR_H, INDICATOR_BLANK_ORIGX, pCDC );
		if ( eIndicator & INDICATOR_RECORD )
			setIndicator( m_bmpIndicators, INDICATOR_RECORD_X,
				INDICATOR_RECORD_Y, INDICATOR_W,
				INDICATOR_H, INDICATOR_BLANK_ORIGX, pCDC );
		if ( eIndicator & INDICATOR_COMPRESS )
			setIndicator( m_bmpIndicators, INDICATOR_COMPRESS_X,
				INDICATOR_COMPRESS_Y, INDICATOR_W, 
				INDICATOR_H, INDICATOR_BLANK_ORIGX, pCDC );
	 }
}

void CPlayDisplay::SetPlayPic( int frame, CDC *pCDC /* = NULL */ )
{
   	CClientDC dc(this);

	// -1 == next frame
	if ( -1 == frame )
	 {
		if ( mi_playFrame == 3 )
			frame = 0;
		else
			frame = mi_playFrame + 1;
	 }
	else
		LS_RANGE( frame, 0, 3 );

    if ( !pCDC )
	 {
		if ( mi_playFrame == frame ) return;
		mi_playFrame = frame;
        pCDC = &dc;
	 }	

	setIndicator( m_bmpAnim, PLAY_ANIM_X, PLAY_ANIM_Y,
		PLAY_ANIM_W, PLAY_ANIM_H, frame*ANIM_FRAME_W, pCDC );
}
//void CPlayDisplay::SetIndicatorPlay( BOOL bSet /* = TRUE */, CDC *pCDC /* = NULL */ )
/*{
	if ( !pCDC )
	 {
		if ( bSet )
			me_indicators |= INDICATOR_PLAY;
		else
			me_indicators &= ~INDICATOR_PLAY;
	 }
	if ( bSet )
		setIndicator( INDICATOR_PLAY_X, INDICATOR_PLAY_Y, INDICATOR_W,
				INDICATOR_H, INDICATOR_PLAY_ORIGX, pCDC );
	else
		setIndicator( INDICATOR_PLAY_X, INDICATOR_PLAY_Y, INDICATOR_W,
				INDICATOR_H, INDICATOR_BLANK_ORIGX, pCDC );
}
*/
//void CPlayDisplay::SetIndicatorPause( BOOL bSet /* = TRUE */, CDC *pCDC /* = NULL */ )
/*{
	if ( !pCDC )
	 {
		if ( bSet )
			me_indicators |= INDICATOR_PAUSE;
		else
			me_indicators &= ~INDICATOR_PAUSE;
	 }
	if ( bSet )
		setIndicator( INDICATOR_PAUSE_X, INDICATOR_PAUSE_Y, INDICATOR_W,
				INDICATOR_H, INDICATOR_PAUSE_ORIGX, pCDC );
	else
		setIndicator( INDICATOR_PAUSE_X, INDICATOR_PAUSE_Y, INDICATOR_W,
				INDICATOR_H, INDICATOR_BLANK_ORIGX, pCDC );
}
*/
//void CPlayDisplay::SetIndicatorCompress( BOOL bSet /* = TRUE */, CDC *pCDC /* = NULL */ )
/*{
	if ( !pCDC )
	 {
		if ( bSet )
			me_indicators |= INDICATOR_COMPRESS;
		else
			me_indicators &= ~INDICATOR_COMPRESS;
	 }
	if ( bSet )
		setIndicator( INDICATOR_COMPRESS_X, INDICATOR_COMPRESS_Y, INDICATOR_W, 
				INDICATOR_H, INDICATOR_COMPRESS_ORIGX, pCDC );
	else
		setIndicator( INDICATOR_COMPRESS_X, INDICATOR_COMPRESS_Y, INDICATOR_W, 
				INDICATOR_H, INDICATOR_BLANK_ORIGX, pCDC );
}
*/

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CPlayDisplay, CStatic)
	//{{AFX_MSG_MAP(CPlayDisplay)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPlayDisplay::setBar( int x, int y, int level, int max, int orient, CDC *pCDC )
{
	CClientDC dc( this );
	if ( !pCDC )
	 {
		pCDC = &dc;
	 }

    CRect rect( x, y, x+3, y+3 );
	int offX = 0, offY = 0;
	DWORD color = RGB(0, 255, 0);

	if ( orient == HORZ )
	 {
		offX = rect.Width() + 1;
	 }
	else
	 {
		offY = -rect.Height() - 1;
	 }
	if ( level > max ) level = max;
	int l2 = 2 * max / 3, l3 = 10 * max / 11 ;
	for (int i = 0; i < level; i++)
	 {
		if ( i == l2 )
			color = RGB(255, 255, 0);
		else if ( i == l3 )
			color = RGB(255, 0, 0);

		pCDC->FillSolidRect( LPCRECT(rect), color );
		rect.OffsetRect( offX, offY );
	 }
	for (i = level; i < max; i++)
	 {
		pCDC->FillSolidRect( LPCRECT(rect), RGB(0, 0, 0) );
		rect.OffsetRect( offX, offY );
	 }	
}

void CPlayDisplay::setIndicator(
	CBitmap& bitmap,
	int destx, int desty, 
	int indicatorw, int indicatorh, int origx, CDC *pCDC )
{
	CClientDC dc( this );
	if ( !pCDC )
	 {
		pCDC = &dc;
	 }

	CDC memDC;
	CBitmap *bmpOld;
	memDC.CreateCompatibleDC( &dc );
	bmpOld = memDC.SelectObject( &bitmap );
	pCDC->BitBlt( destx, desty, indicatorw, indicatorh, &memDC, origx, 0, SRCCOPY );
	memDC.SelectObject( bmpOld );
	memDC.DeleteDC();
}

void CPlayDisplay::setTime( CLEDBitmapDC *pLed, long lTime,
                           int destx, int desty, int colonWidth, CDC* pCDC )
{
	CClientDC dc(this);
	if (!pCDC)
	 {
		pCDC = &dc;
	 }
	char strNum[3];
	int sec = lTime % 60;
	int min = (lTime % 3600) / 60;
	int hour = lTime / 3600;
	
	sprintf(strNum, "%d", hour);
	pLed->TextOut( pCDC, destx, desty, strNum, strlen(strNum) );
		
	if ( min == 0 )
		strcpy(strNum, "00");
	else if ( min < 10 )
		sprintf(strNum, "0%d", min);
	else
		sprintf(strNum, "%d", min);
	destx += pLed->GetWidth() + colonWidth;
	pLed->TextOut( pCDC, destx, desty, strNum, strlen(strNum) );
	
	if ( sec == 0 )
		strcpy(strNum, "00");
	else if ( sec < 10 )
		sprintf(strNum, "0%d", sec);
	else
		sprintf(strNum, "%d", sec);
	destx += pLed->GetWidth()*2 + colonWidth;
	pLed->TextOut( pCDC, destx, desty, strNum, strlen(strNum) );
}

void CPlayDisplay::OnPaint()
{
	CPaintDC dc(this);
	
	// update background
	CDC bkgDC;
	CBitmap *bmpOld;
	bkgDC.CreateCompatibleDC( &dc );
	bmpOld = bkgDC.SelectObject( &m_bmpBkg );
	dc.BitBlt( 0, 0, mi_bkgWidth, mi_bkgHeight, &bkgDC, 0, 0, SRCCOPY );
	bkgDC.SelectObject( bmpOld );
	bkgDC.DeleteDC();

	// draw border
	CRect rc( 0, 0, mi_bkgWidth, mi_bkgHeight );
	//CBrush br1( COLORREF(GetSysColor(COLOR_BTNHILIGHT)) );
	//CBrush br2( COLORREF(GetSysColor(COLOR_BTNSHADOW)) );
	//dc.FrameRect( rc, &br2 );
	dc.DrawEdge( rc, EDGE_SUNKEN, BF_RECT );

	// update all else
//	SetVolume( ml_volume, &dc );
	SetPlayTime( ml_playTimeUpdate, &dc );
	SetLengthTime( ml_playLengthUpdate, &dc );
	if ( INDICATOR_NONE != me_indicators )
		SetIndicator( me_indicators, TRUE, &dc );
	if ( mi_speedUpdate != 0 )
		SetPlaySpeed( mi_speedUpdate, &dc );
	if ( mi_rateUpdate )
		SetRate( mi_rateUpdate, &dc );
	if ( mi_sizeUpdate )
		SetSize( mi_sizeUpdate, &dc );
	if ( mi_playFrame )
		SetPlayPic( mi_playFrame, &dc );
}

/////////////////////////////////////////////////////////////////////////////
// CLEDBitmapDC Implementation
/////////////////////////////////////////////////////////////////////////////

#define NUM_OF_LED_CHAR 11

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CLEDBitmapDC::CLEDBitmapDC( LPCTSTR lpszResourceName, CDC *pCDC /* = NULL */ )
{
	commonCtor( lpszResourceName, 0, pCDC );
}
  
CLEDBitmapDC::CLEDBitmapDC( UINT nIDResource, CDC *pCDC /* = NULL */ )
{
	commonCtor( NULL, nIDResource, pCDC );
}

CLEDBitmapDC::~CLEDBitmapDC()
{
//	ReleaseDestDC();
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

void CLEDBitmapDC::TextOut( CDC *pCDC, int x, int y, LPCTSTR lpszString, int nCount )
{
	int srcx, num;

	SetDestDC( pCDC );
	for (int i = 0; i < nCount; i++ )
	 {
		num = lpszString[i];
		if ( isdigit(num) )
			srcx = (num - '0') * mi_width;
		else if ( isspace(num) )
			srcx = (NUM_OF_LED_CHAR - 1) * mi_width;
		else
			continue;
		mp_DC->BitBlt( x, y, mi_width, mi_height, this, srcx, 0, SRCCOPY );
		x += mi_width;
	 }
	ReleaseDestDC();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CLEDBitmapDC::commonCtor( LPCTSTR lpszResourceName, UINT nIDResource, CDC *pCDC )
{
	mp_bmpOld = NULL;
	mp_DC = pCDC;
	//CreateCompatibleDC( pCDC );
	if ( lpszResourceName )
		m_bmpLED.LoadBitmap( lpszResourceName );
	else
		m_bmpLED.LoadBitmap( nIDResource);
	//SelectObject( m_bmpLED );
	BITMAP bitmap;
	m_bmpLED.GetObject(sizeof (BITMAP), &bitmap);
	mi_width = bitmap.bmWidth / NUM_OF_LED_CHAR;
	mi_height = bitmap.bmHeight;
}

void CLEDBitmapDC::SetDestDC( CDC *pCDC )
{
	mp_DC = pCDC;
	CreateCompatibleDC( mp_DC );
	mp_bmpOld = SelectObject( &m_bmpLED );
}

void CLEDBitmapDC::ReleaseDestDC()
{
	SelectObject( mp_bmpOld );
	DeleteDC();
}
