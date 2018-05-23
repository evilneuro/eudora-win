/*////////////////////////////////////////////////////////////////////////////

NAME:
	CVolumeBar - 

FILE:		VolumeBar.CPP
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:

RESCRITICTIONS:

DEPENDENCIES:


NOTES:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/13/97   lss     -Initial
03/21/97   lss     -Ability to save and retrieve previous visible state of
					this volume bar
				   -Will adjust the volume sliders if volume was adjusted
				    outside of this program
05/09/97   lss     -Save volume settings
05/31/97   lss     -Speaker and Mike bmp now converted to transparent bmp
					via a new function convertBmp2Transparent
08/15/97   lss     -After a day of debugging, the "solid icon" problem
					seems to come down to this file. See comments in the
					destructor. Don't quite know why this fixes it but 
					hey, it works! Interestingly enough, before this fix,
					as long as Volume Control program was running, this
					problem did not occur. Go Figure..
11/20/97   lss	   -Moved stuff from destructor to OnClose. This seems to fix
					PureVoice hanging on exit on some NT machines.
					Looks like I needed to unsubclass before we destory this
					Wnd.
12/01/97   lss	   -Oops, OnClose never gets called and so we get the "solid
					icon" problem again on some Win95. So I moved the stuff 
					back to destructor and only unsubclass when running on
					Win95
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#include "QVoice.h"
#include "LsMixer.h"

// LOCAL INCLUDES
#include "VolumeBar.h"

// LOCAL DEFINES
#define VOLUMEBAR_WIDTH		65

#define VOLUME_OUT			(99 - m_VolOut.Volume())
#define VOLUME_IN			(99 - m_VolIn.Volume())

#define WRITE_PROFILE_VOLUMEBAR(bSet)	\
					AfxGetApp()->WriteProfileInt( "Settings", "VolumeBar", bSet )
#define GET_PROFILE_VOLUMEBAR()			\
					AfxGetApp()->GetProfileInt( "Settings", "VolumeBar", 0 )
#define WRITE_PROFILE_VOLUMEIN(volume)	\
					AfxGetApp()->WriteProfileInt( "Settings", "VolumeIn", volume )
#define GET_PROFILE_VOLUMEIN()			\
					AfxGetApp()->GetProfileInt( "Settings", "VolumeIn", -1 )
#define WRITE_PROFILE_VOLUMEOUT(volume)	\
					AfxGetApp()->WriteProfileInt( "Settings", "VolumeOut", volume )
#define GET_PROFILE_VOLUMEOUT()			\
					AfxGetApp()->GetProfileInt( "Settings", "VolumeOut", -1 )

#include "DebugNewHelpers.h"

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CVolumeBar::CVolumeBar() :
	mb_Show(FALSE), mp_MainFrame(NULL), m_OldVolOut(99), m_OldVolIn(99)
{
}

CVolumeBar::~CVolumeBar()
{
	if ( ((CQVoiceApp *)AfxGetApp())->m_OsPlatform 
		== VER_PLATFORM_WIN32_WINDOWS )
	{
	// ok, after a day of debugging, found that unsubclassing these
	// two slider ctrl seems to fix the solid icon problem on Win95 with
	// 'show icons with all possible colors' enabled.
		m_VolOutSld.UnsubclassWindow();
		m_VolInSld.UnsubclassWindow();
	}

	// save our state for next session
	WRITE_PROFILE_VOLUMEBAR( mb_Show ? 1 : 0 );
	if (m_VolIn.IsOpen()) WRITE_PROFILE_VOLUMEIN( m_VolIn.Volume() );
	if (m_VolOut.IsOpen()) WRITE_PROFILE_VOLUMEOUT( m_VolOut.Volume() );
	// restore original values
	m_VolOut.Volume( m_OldVolOut );
	m_VolIn.Volume( m_OldVolIn );
}

BOOL CVolumeBar::Create(
	CWnd* pParentWnd,	// must be frame window
	UINT nStyle,		//= CBRS_RIGHT|CBRS_TOOLTIPS
	UINT nID,			//= IDD_VOLUME
	UINT nIDTemplate	//= IDD_VOLUME
)
{
	BOOL ret = CDialogBar::Create( pParentWnd, nIDTemplate, nStyle, nID );
	mp_MainFrame = GetParentFrame();

	if ( ret )
	{
		m_VolOut.Open( LsMixerControl::waveOut, GetSafeHwnd() );
		m_VolIn.Open( LsMixerControl::waveIn, GetSafeHwnd() );

		if ( !m_VolOutSld.SubclassDlgItem(IDC_VOLUME_OUT, this) )
			return FALSE;
		if ( !m_VolInSld.SubclassDlgItem(IDC_VOLUME_IN, this) )
			return FALSE;
/*
		if ( !m_VolOutSld.Create(TBS_VERT | TBS_AUTOTICKS | TBS_RIGHT | WS_VISIBLE | WS_TABSTOP,
				CRect(0,0,5,5), this, IDC_VOLUME_OUT ) )
			return FALSE;
		if ( !m_VolInSld.Create(TBS_VERT | TBS_AUTOTICKS | TBS_RIGHT | WS_VISIBLE | WS_TABSTOP,
				CRect(5,0,10,5), this, IDC_VOLUME_IN ) )
			return FALSE;
*/
		// save old volumes
		m_OldVolOut = m_VolOut.Volume();
		m_OldVolIn  = m_VolIn.Volume();

		// get previous session's settings
		int vol;
		if ( (vol = GET_PROFILE_VOLUMEOUT()) > -1 )
		 {
			m_VolOut.Volume( vol );
		 }
		if ( (vol = GET_PROFILE_VOLUMEIN()) > -1 )
		 {
			m_VolIn.Volume( vol );
		 }

		m_VolOutSld.SetRange( 0, 99 );
		m_VolOutSld.SetTicFreq( 10 );
		m_VolInSld.SetRange( 0, 99 );
		m_VolInSld.SetTicFreq( 10 );
		m_VolOutSld.SetPos( VOLUME_OUT );
		m_VolInSld.SetPos( VOLUME_IN );

		// resize our main frame window so we can fit in
		CRect frmRect;
		pParentWnd->GetWindowRect( LPRECT(frmRect) );
		pParentWnd->SetWindowPos( NULL, 0, 0, 
						frmRect.Width()+VOLUMEBAR_WIDTH, frmRect.Height(),
						SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE );
		setup();
		// get state from previous session
		if ( !(mb_Show = GET_PROFILE_VOLUMEBAR()) )
		 {
			// Don't Show volume bar control
			ShowWindow( SW_HIDE );
		 }
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

CSize CVolumeBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize size = CDialogBar::CalcFixedLayout(bStretch, bHorz);
	if (!bHorz) size.cx = VOLUMEBAR_WIDTH;
	return size;
}

void CVolumeBar::Show( BOOL bShow ) //=TRUE 
{
	int state = SW_SHOWNA;
	if ( !bShow ) state = SW_HIDE;
	BOOL prev = ShowWindow( state );
	if ( state == prev ) return;
	mb_Show = bShow;
	if ( bShow && (prev == SW_HIDE) )
	 {
		CRect frmRect;
		mp_MainFrame->GetWindowRect( LPRECT(frmRect) );
		mp_MainFrame->SetWindowPos( NULL, 0, 0,
						frmRect.Width()+VOLUMEBAR_WIDTH, frmRect.Height(),
						SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE );
		m_VolOutSld.SetPos( VOLUME_OUT );
		m_VolInSld.SetPos( VOLUME_IN );
	 }
	mp_MainFrame->RecalcLayout();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// properly align our sliders and pics
void CVolumeBar::setup()
{
	CRect rect;
	CRect vOutRect;

	GetWindowRect( LPRECT(rect) );
	m_VolOutSld.GetWindowRect( LPRECT(vOutRect) );

	int wSlider = 20;
	// adjust slider depending on the thumb size so that ticks will
	// show up with different display types. crude way but better than
	// no adjustment at all
	vOutRect.SetRect(0, 0, 20, rect.Height() - 15 - 3*6 - 4);
	m_VolOutSld.MoveWindow( LPCRECT(vOutRect) );
	CRect thumbRect; m_VolOutSld.GetThumbRect( LPRECT(thumbRect) );
	if (thumbRect.Width() >= 20) wSlider = 19;
	else if (thumbRect.Width() >= 17) wSlider = 24;
	else if (thumbRect.Width() == 16) wSlider = 22;

	int xSpacing = (rect.Width() - 2*wSlider)/3;
	int hSlider  = rect.Height() - 15 - 3*6 - 4;

	if ( (xSpacing < 1) || (hSlider < 1) ) return;

	CPoint topleft(xSpacing, 6);
	vOutRect.SetRect(topleft.x, topleft.y, 
					topleft.x + wSlider, topleft.y + hSlider);
	m_VolOutSld.MoveWindow( LPCRECT(vOutRect) );
	vOutRect.OffsetRect( vOutRect.Width() + xSpacing, 0 );
	m_VolInSld.MoveWindow( LPCRECT(vOutRect) );

	CStatic* pSpeaker = (CStatic*)GetDlgItem(IDP_SPEAKER);
	CStatic* pMike  = (CStatic*)GetDlgItem(IDP_MIKE);

	if ( convertBmp2Transparent( IDB_SPEAKER, m_bmpSpeaker ) )
		pSpeaker->SetBitmap( m_bmpSpeaker );
	if ( convertBmp2Transparent( IDB_MIKE, m_bmpMike ) )
		pMike->SetBitmap( m_bmpMike );

	// align our speaker bmp
	m_VolOutSld.GetWindowRect( LPRECT(vOutRect) );
	int xMid = vOutRect.Width()/2 + vOutRect.left - rect.left;
	pSpeaker->GetWindowRect( LPRECT(vOutRect) );
	topleft.x = xMid - vOutRect.Width()/2; topleft.y = topleft.y + hSlider + 3;
	vOutRect.SetRect( topleft.x, topleft.y,
				topleft.x + vOutRect.Width(), topleft.y + vOutRect.Height() );
	pSpeaker->MoveWindow( LPRECT(vOutRect) );
	// align our mike bmp
	m_VolInSld.GetWindowRect( LPRECT(vOutRect) );
	xMid = vOutRect.Width()/2 + vOutRect.left - rect.left;
	pMike->GetWindowRect( LPRECT(vOutRect) );
	topleft.x = xMid - vOutRect.Width()/2; //topleft.y = topleft.y + hSlider + 3;
	vOutRect.SetRect( topleft.x, topleft.y,
				topleft.x + vOutRect.Width(), topleft.y + vOutRect.Height() );
	pMike->MoveWindow( LPRECT(vOutRect) );
}

// Yes, this code is pretty much the same as the one found in CLsBmpButton.
// I'm going to consolidate this into a new CBitmapTransparent class or
// something similar
BOOL CVolumeBar::convertBmp2Transparent(
	UINT		nIDResource, 
	CBitmap&	destBitmap
)
{
	CClientDC dc(this);

    // Create memory DCs
	CDC	srcMemDC;
	srcMemDC.CreateCompatibleDC( &dc );

	CBitmap srcBitmap;
	if ( !srcBitmap.LoadBitmap(nIDResource) )
	 {
		TRACE0("Failed to load bitmap!\n");
		return FALSE;
	 }

	// get src bitmap size
	BITMAP bmInfo;
	VERIFY(srcBitmap.GetObject(sizeof(bmInfo), &bmInfo) == sizeof(bmInfo));
	CSize bmpSize( bmInfo.bmWidth, bmInfo.bmHeight );

	CBitmap* pSrcOldBmp = srcMemDC.SelectObject( &srcBitmap );

	CBitmap& memBitmap = destBitmap;

	CBitmap imageBitmap;
	CBitmap maskBitmap;

	CDC maskDC;
	CDC imageDC;

	// Create compatible dc's
	maskDC.CreateCompatibleDC( &dc );
	imageDC.CreateCompatibleDC( &dc );

	CPalette* pImagePalette = imageDC.SelectPalette(
			dc.GetCurrentPalette(), TRUE );

	// Create the memory bitmap.
	memBitmap.CreateCompatibleBitmap( &dc, bmpSize.cx, bmpSize.cy );

	// Create a compatible dc for the source image.
	imageBitmap.CreateCompatibleBitmap( &dc, bmpSize.cx, bmpSize.cy );

	// Create a mask bitmap.
	maskBitmap.CreateBitmap( bmpSize.cx, bmpSize.cy, 1, 1, NULL );

	// Select them into their dc's
	CBitmap* oldImage = imageDC.SelectObject( &imageBitmap );
	
	// Copy the source image into the image dc.
	imageDC.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy, &srcMemDC, 0, 0, 
			SRCCOPY );

	// Select the memory bitmap in now.
	srcMemDC.SelectObject( &memBitmap );

	CBitmap* oldMask = maskDC.SelectObject( &maskBitmap );

	// fill in our background with the btn face color
	CRect rc(0, 0, bmpSize.cx, bmpSize.cy);
	srcMemDC.FillSolidRect( rc, COLORREF(GetSysColor(COLOR_3DFACE)) );

	// Create the mask.
	// All bits matching the transparent color are set to white (1)
	// in the mask bitmap. All other bits are set to black (0).
	//m_transparentColor = ::GetPixel( imageDC, 0, 0 );
	imageDC.SetBkColor( ::GetPixel( imageDC, 0, 0 ) );
	maskDC.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy, &imageDC, 0, 0,
			SRCCOPY );

	// Use the mask to copy the image to the dc.

	// XOR the screen with the bitmap.
	srcMemDC.SetBkColor( RGB(0xFF, 0xFF, 0xFF) );
	srcMemDC.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy,
			&imageDC, 0, 0,	SRCINVERT );

	// AND the screen and the mask.
	srcMemDC.BitBlt ( 0, 0, bmpSize.cx, bmpSize.cy,
			&maskDC, 0, 0, SRCAND );

	// XOR the screen and the image again.
	srcMemDC.BitBlt ( 0, 0, bmpSize.cx, bmpSize.cy,
			&imageDC, 0, 0,	SRCINVERT );

	// Do a little clean up.
	maskDC.SelectObject( oldMask );
	imageDC.SelectObject( oldImage );

	imageDC.SelectPalette( pImagePalette, TRUE );

	maskDC.DeleteDC();
	imageDC.DeleteDC();

	srcMemDC.SelectObject( pSrcOldBmp );
	srcMemDC.DeleteDC();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CVolumeBar message handlers
//////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CVolumeBar, CDialogBar)
	//{{AFX_MSG_MAP(CVolumeBar)
	ON_MESSAGE(MM_MIXM_CONTROL_CHANGE, OnMixerControl)
	ON_WM_VSCROLL()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

LRESULT CVolumeBar::OnMixerControl(WPARAM wParam, LPARAM lParam)
{
	if ( mb_Show )
	 {
		m_VolOutSld.SetPos( VOLUME_OUT );
		m_VolInSld.SetPos( VOLUME_IN );
	 }
	return 0L;
}

void CVolumeBar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( pScrollBar )
	 {
		if ( IDC_VOLUME_OUT == pScrollBar->GetDlgCtrlID() )
		 {
			m_VolOut.Volume(99-((CSliderCtrl *) pScrollBar)->GetPos());
		 }
		else
		 {
			m_VolIn.Volume(99-((CSliderCtrl *) pScrollBar)->GetPos());
		 }
	 }
	
	CDialogBar::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CVolumeBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect viewRect;
    
    // draw raised border around our view
	GetClientRect( viewRect );
	dc.DrawEdge( viewRect, EDGE_SUNKEN, BF_RECT );
	viewRect.DeflateRect( 2, 2 );
	dc.DrawEdge( viewRect, EDGE_RAISED, BF_RECT );
	
	// Do not call CDialogBar::OnPaint() for painting messages
}
/*
void CVolumeBar::OnVolumebar() 
{
	Show( !IsWindowVisible() );
	((CFormView*)(GetParentFrame()->GetActiveView()))->ResizeParentToFit();
}

void CVolumeBar::OnUpdateVolumebar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( IsWindowVisible() );	
}
*/


