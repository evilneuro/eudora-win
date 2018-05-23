// PooterButton.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"
#include "PooterButton.h"


#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// PooterButton

PooterButton::PooterButton()
{
    m_bMouseCaptured = false;
    m_iState = kFlat;
}

PooterButton::~PooterButton()
{
}


BEGIN_MESSAGE_MAP(PooterButton, CStatic)
    //{{AFX_MSG_MAP(PooterButton)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_SETCURSOR()
    ON_WM_MOUSEMOVE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PooterButton message handlers


void PooterButton::OnPaint() 
{
    CPaintDC dc(this);
    Draw( &dc );

    CRect rect;
    GetClientRect( &rect );
    rect.right += 2;
    rect.InflateRect( 1, 1 );
    CBrush brshActBorder;
    brshActBorder.CreateSolidBrush( ::GetSysColor(COLOR_BTNTEXT) );
    dc.FrameRect( &rect, &brshActBorder );
}

void PooterButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
    Draw( GetDC(), kPushed );
}

void PooterButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
    Draw( GetDC(), kRaised );
    WPARAM wParam = MAKEWPARAM( GetDlgCtrlID(), BN_CLICKED );
    GetParent()->SendMessage( WM_COMMAND, wParam, (LPARAM) GetSafeHwnd() );
    ReleaseCapture();
    m_bMouseCaptured = false;
    Invalidate();
}

void PooterButton::Draw( CDC* pdc )
{
    Draw( pdc, m_iState );
}


// BOG -- Note that there is a little bit of magic going on in the following routine.
// +1 this; -1 that; what gives? Well, in particular, the button itself is a bit
// of a hack -- drawing outside it's own window rect and such. Took some fiddling to
// get things "just so". Hmmm! Just something to be aware of when Copy/Pasting
// this code!
//
// The 3d sunken down-arrow adornment should work anywhere. I copied it from netscape.
// Not much to these things -- look at 'em too long, and they lose the 3d effect.

void PooterButton::Draw( CDC* pdc, int iState )
{
    int iSavedDC = pdc->SaveDC();

    CRect rect;
    GetClientRect( &rect );
    CRect textRect = rect;
    rect.right += 2;

    CBrush theBrush;
    theBrush.CreateSolidBrush( ::GetSysColor(COLOR_BTNFACE) );
    CPen hilitePen;
    hilitePen.CreatePen( PS_SOLID, 1, ::GetSysColor(COLOR_BTNHIGHLIGHT) );
    CPen shadowPen;
    shadowPen.CreatePen( PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW) );

    // draw the button -- swap the shadow/hilite based on state
    pdc->SelectObject( &theBrush );
    pdc->PatBlt( rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY );

    if ( iState != kFlat ) {
        if ( iState == kRaised )
            pdc->SelectObject( hilitePen );
        else
            pdc->SelectObject( shadowPen );

        pdc->MoveTo( rect.left, rect.bottom - 1 );
        pdc->LineTo( rect.TopLeft() );
        pdc->LineTo( rect.right - 1, rect.top );

        if ( iState == kRaised )
            pdc->SelectObject( shadowPen );
        else
            pdc->SelectObject( hilitePen );

        pdc->LineTo( rect.right - 1, rect.bottom - 1 );
        pdc->LineTo( rect.left, rect.bottom - 1 );
    }

    enum {
        top_left,
        top_right,
        bottom
    };

    // draw the funky 3d down-arrow
    CRect arrowRect( rect.TopLeft(), CPoint(rect.bottom, rect.left + rect.Height()) );
    int deflateBy = (arrowRect.Width() - (arrowRect.Width() % 4)) / 4;
    arrowRect.DeflateRect( deflateBy, deflateBy );

    POINT points[3];
    points[top_left]    = arrowRect.TopLeft();
    points[top_right].x = arrowRect.right;
    points[top_right].y = arrowRect.top;
    points[bottom].x    = arrowRect.left + (arrowRect.Width() / 2);
    points[bottom].y    = arrowRect.bottom;

    pdc->SelectObject( hilitePen );
    pdc->MoveTo( points[top_right] );
    pdc->LineTo( points[bottom] );
    pdc->SelectObject( shadowPen );
    pdc->LineTo( points[top_left] );
    pdc->LineTo( points[top_right].x + 1, points[top_right].y );

    // draw the caption
    CString theCaption;
    GetWindowText( theCaption );
    pdc->SetBkMode( TRANSPARENT );
    pdc->SelectObject( GetFont() );
    pdc->DrawText( theCaption, -1, &textRect, DT_RIGHT | DT_VCENTER );

    // this is better than lots of calls to SelectObject
    pdc->RestoreDC( iSavedDC );
}


BOOL PooterButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    ::SetCursor( AfxGetApp()->LoadCursor(IDC_APP_LAUNCH) );
    return TRUE;
}


void PooterButton::OnMouseMove( UINT nFlags, CPoint point )
{
    if ( m_bMouseCaptured ) {
        CRect theRect;
        GetClientRect( &theRect );

        if ( !theRect.PtInRect( point ) ) {
            ReleaseCapture();
            m_bMouseCaptured = false;
            Draw( GetDC(), kFlat );
        }
    }
    else {
        SetCapture();
        m_bMouseCaptured = true;
        int iState = (nFlags & MK_LBUTTON) ? kPushed : kRaised;
        Draw( GetDC(), iState );
    }
}

