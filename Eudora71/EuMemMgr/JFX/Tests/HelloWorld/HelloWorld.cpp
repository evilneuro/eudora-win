/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#include "stdafx.h"

class HelloWorldWnd : public JWnd
{
public      :
    HelloWorldWnd ( void )
    {

    }

    ~HelloWorldWnd ( void )
    {
    }

protected   :

    // Declare the messages I will handle here.
    BEGIN_MSG_DECLARES()
        DECL_WM_DESTROY()
        DECL_WM_CLOSE()
        DECL_WM_PAINT()
    END_MSG_DECLARES()

} ;

BEGIN_MSG_MAP ( HelloWorldWnd , JWnd )
    WM_DESTROY_HANDLER ( )
    WM_CLOSE_HANDLER ( )
    WM_PAINT_HANDLER ( )
END_MSG_MAP ( HelloWorldWnd , JWnd )



void HelloWorldWnd :: OnDestroy ( void )
{
    PostQuitMessage ( 0 ) ;
}

void HelloWorldWnd :: OnClose ( void )
{
    DestroyWindow ( ) ;
}

void HelloWorldWnd :: OnPaint ( void )
{
    PAINTSTRUCT ps ;
    RECT rect ;
    HDC hDC ;

    GetClientRect ( rect ) ;

    hDC = BeginPaint ( &ps ) ;

    ::DrawText ( hDC                        ,
                 _T ( "Hello from JFX!!" )  ,
                 -1                         ,
                 &rect                      ,
                 DT_CENTER | DT_VCENTER |
                    DT_SINGLELINE            ) ;

    EndPaint ( &ps ) ;
}

class MyApp : public JWinApp
{
public :
    BOOL InitInstance ( void )
    {
        m_pWnd = new HelloWorldWnd ( ) ;

        m_pMainWnd = m_pWnd ;

        m_pMainWnd->Create ( _T ( "Hello from JFX!" ) ,
                             WS_OVERLAPPEDWINDOW   | WS_VISIBLE ) ;

        m_pMainWnd->ShowWindow ( SW_SHOWNORMAL ) ;
        m_pMainWnd->UpdateWindow ( ) ;

        return ( TRUE ) ;
    }

    ~MyApp ( void )
    {
        if ( NULL != m_pWnd )
        {
            delete m_pWnd ;
        }
    }

    HelloWorldWnd * m_pWnd ;

} ;


MyApp theApp ;