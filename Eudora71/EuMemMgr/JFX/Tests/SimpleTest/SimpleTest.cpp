/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
//#include "JFX.h"
#include "resource.h"

class SimpleWnd : public JModalDlg
{
public      :
    SimpleWnd ( int iID ) : JModalDlg ( iID )
    {
    }
    virtual ~SimpleWnd ( void )
    {
    }
protected   :

    // Declare the messages I will handle here.
    BEGIN_MSG_DECLARES()
        DECL_WM_INITDIALOG()
    END_MSG_DECLARES()

    virtual void OnOk ( void )
    {
        MessageBox ( m_hWnd            ,
                    _T ( "OnOk!" )     ,
                    _T ( "SimpleWnd" ) ,
                    MB_OK               ) ;
    }

} ;

BEGIN_MSG_MAP ( SimpleWnd , JModalDlg )
    WM_INITDIALOG_HANDLER()
END_MSG_MAP ( SimpleWnd , JModalDlg )

BOOL SimpleWnd :: OnInitDialog ( HWND , LPARAM )
{
    SetWindowText ( _T ( "SimpleWnd Dialog Class!" ) ) ;
    CenterWindow ( ) ;
    return ( TRUE ) ;
}


class HappyModeless : public JWnd
{
public      :
    HappyModeless ( void )
    {

    }

    ~HappyModeless ( void )
    {
        //DestroyIcon ( m_hIcon ) ;
    }

protected   :

    // Declare the messages I will handle here.
    BEGIN_MSG_DECLARES()
        DECL_WM_INITDIALOG()
        DECL_WM_DESTROY()
        DECL_WM_CLOSE()
    END_MSG_DECLARES()

    // The OK handler.
    void TestOnOK ( void ) ;
    // The Cancel handler.
    void TestCancelHandler ( void ) ;

    // The WM_TIMER handler.
    LRESULT OnTimer ( WPARAM wParam , LPARAM lParam ) ;

    HICON m_hIcon ;

    JWnd m_cButton ;
} ;

BEGIN_MSG_MAP ( HappyModeless , JWnd )
    MESSAGE_HANDLER ( WM_TIMER , OnTimer )
    COMMAND_ID_HANDLER ( IDOK , TestOnOK )
    COMMAND_ID_HANDLER ( IDCANCEL , TestCancelHandler )
    WM_INITDIALOG_HANDLER ( )
    WM_DESTROY_HANDLER ( )
    WM_CLOSE_HANDLER ( )
END_MSG_MAP ( HappyModeless , JWnd )




void HappyModeless :: OnDestroy ( void )
{
    PostQuitMessage ( 0 ) ;
}

void HappyModeless :: OnClose ( void )
{
    DestroyWindow ( ) ;
}

BOOL HappyModeless :: OnInitDialog ( HWND , LPARAM )
{
    m_hIcon = JfxGetApp()->LoadIcon ( IDI_MYICON ) ;
    m_hIcon = JfxGetApp()->LoadSmallIcon ( IDI_MYICON ) ;

    SetIcon ( m_hIcon , TRUE ) ;
    SetIcon ( m_hIcon , FALSE ) ;
    m_cButton.Attach ( *this , IDOK ) ;
    CenterWindow ( ) ;

    SetTimer ( 1 , 500 ) ;
    return ( TRUE ) ;
}

void HappyModeless :: TestOnOK ( void )
{
    m_cButton.EnableWindow ( FALSE ) ;
}

void HappyModeless :: TestCancelHandler ( void )
{
    OnClose ( ) ;
}

LRESULT HappyModeless :: OnTimer ( WPARAM /*wParam*/ , LPARAM /*lParam*/ )
{
    static int iTurn = 0 ;
    if ( 0 == iTurn )
    {
        SetIcon ( m_hIcon , FALSE ) ;
        iTurn++ ;
    }
    else
    {
        iTurn = 0 ;
        SetIcon ( LoadIcon ( NULL , IDI_ERROR ) , FALSE ) ;
    }
    return ( 0 ) ;
}


class MyApp : public JWinApp
{
public :
    BOOL InitInstance ( void )
    {
        // Do a modal dialog with the raw class.
        JModalDlg MyModalDialog ( IDD_MODALDIALOG ) ;
        MyModalDialog.DoModal ( ) ;

        // Do a modal dialog with a simple class.
        SimpleWnd MySimpleDialog ( IDD_MODALDIALOG ) ;
        MySimpleDialog.DoModal ( ) ;

        m_pMainWnd = &MyModeless ;

        MyModeless.ModelessDialog ( IDD_MODELESSDLG ) ;

        return ( TRUE ) ;
    }

    HappyModeless MyModeless ;
} ;


MyApp theApp ;