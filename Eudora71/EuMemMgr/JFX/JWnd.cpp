/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"

#include "JFX.h"

/*//////////////////////////////////////////////////////////////////////
                      Private Static Data Members
//////////////////////////////////////////////////////////////////////*/
STATICTLS HHOOK  JWnd::sm_hhook = NULL ;
STATICTLS JWnd * JWnd::sm_pThis = NULL ;

/*//////////////////////////////////////////////////////////////////////
                             Public Methods
                      Creating Windows and Dialogs
//////////////////////////////////////////////////////////////////////*/

INT_PTR JWnd :: Dialog ( LPCTSTR   szTemplate  ,
                         HWND      hWndParent  ,
                         HINSTANCE hInst       ,
                         LPARAM    lParam       )
{
    HINSTANCE hInstReal = hInst ;
    // Try and load the dialog resource.
    LPCDLGTEMPLATE pDlg = JfxGetApp()->LoadDialog ( szTemplate ,
                                                    &hInstReal  ) ;
    ASSERT ( NULL != pDlg ) ;
    if ( NULL == pDlg )
    {
        return ( FALSE ) ;
    }

    // Set the TLS static.
    sm_pThis = this ;
    // Crank the hook up.
    sm_hhook = SetWindowsHookEx ( WH_CALLWNDPROC        ,
                                  CallWndProc           ,
                                  NULL                  ,
                                  GetCurrentThreadId ( ) ) ;
    ASSERT ( NULL != sm_hhook ) ;
                                  
    // Do the dialog.
    INT_PTR iRet = ::DialogBoxIndirectParam ( hInstReal     ,
                                              pDlg          ,
                                              hWndParent    ,
                                              CommonDlgProc ,
                                              lParam         ) ;
    // Clear the hook if needed.
    if ( NULL != sm_hhook )
    {
        UnhookWindowsHookEx ( sm_hhook ) ;
        sm_hhook = NULL ;
    }
    return ( iRet ) ;
}

HWND JWnd :: ModelessDialog ( LPCTSTR   szTemplate ,
                              HWND      hWndParent ,
                              HINSTANCE hInst      ,
                              LPARAM    lParam      )
{
    HINSTANCE hInstReal = hInst ;
    // Try and load the dialog resource.
    LPCDLGTEMPLATE pDlg = JfxGetApp()->LoadDialog ( szTemplate ,
                                                    &hInstReal  ) ;
    ASSERT ( NULL != pDlg ) ;
    if ( NULL == pDlg )
    {
        return ( FALSE ) ;
    }

    // Set the TLS static.
    sm_pThis = this ;
    // Do the hook.
    sm_hhook = SetWindowsHookEx ( WH_CALLWNDPROC        ,
                                  CallWndProc           ,
                                  NULL                  ,
                                  GetCurrentThreadId ( ) ) ;

    HWND hWndRet = ::CreateDialogIndirectParam ( hInstReal      ,
                                                 pDlg           ,
                                                 hWndParent     ,
                                                 CommonDlgProc  ,
                                                 lParam          ) ;
    // Clear the hook if needed.
    if ( NULL != sm_hhook )
    {
        UnhookWindowsHookEx ( sm_hhook ) ;
        sm_hhook = NULL ;
    }
    return ( hWndRet ) ;
}

// For regular windows, allows derived classes to influence the
// window creation.
BOOL JWnd :: PreCreateWindow ( CREATESTRUCT & /*cs*/ )
{
    return ( TRUE ) ;
}

HWND JWnd :: CreateEx ( DWORD     dwExStyle       ,
                        LPCTSTR   szClassName     ,
                        LPCTSTR   szWindowName    ,
                        DWORD     dwStyle         ,
                        int       x               ,
                        int       y               ,
                        int       nWidth          ,
                        int       nHeight         ,
                        JWnd *    pParent         ,
                        HMENU     nIDorHMenu      ,
                        HINSTANCE hInstance       ,
                        LPVOID    lpParam          )
{
    // Note that I assume the class is already registered.
    CREATESTRUCT cs ;
    cs.dwExStyle = dwExStyle ;
    cs.lpszClass = szClassName ;
    cs.lpszName = szWindowName ;
    cs.style = dwStyle ;
    cs.x = x ;
    cs.y = y ;
    cs.cx = nWidth ;
    cs.cy = nHeight ;
    if ( NULL != pParent )
    {
        cs.hwndParent = pParent->m_hWnd ;
    }
    else
    {
        cs.hwndParent = NULL ;
    }
    cs.hMenu = nIDorHMenu;

    cs.lpCreateParams = lpParam;

    // If the hInstance is NULL and the class name is JWndClass, I
    // will use the same one I registered the class with.  I had a
    // bug here where I accidentally used the internationalization
    // resource handle! (Duh!)
    if ( 0 == hInstance )
    {
        hInstance = JfxGetApp()->GetInstanceForClassRegistration ( ) ;
    }
    cs.hInstance = hInstance ;


    if ( FALSE == PreCreateWindow ( cs ) )
    {
        return ( FALSE ) ;
    }

    // Set the TLS static.
    sm_pThis = this ;

    // Do the hook.
    sm_hhook = SetWindowsHookEx ( WH_CALLWNDPROC        ,
                                  CallWndProc           ,
                                  NULL                  ,
                                  GetCurrentThreadId ( ) ) ;
    HWND hWndRet = ::CreateWindowEx ( cs.dwExStyle      ,
                                      cs.lpszClass      ,
                                      cs.lpszName       ,
                                      cs.style          ,
                                      cs.x              ,
                                      cs.y              ,
                                      cs.cx             ,
                                      cs.cy             ,
                                      cs.hwndParent     ,
                                      cs.hMenu          ,
                                      cs.hInstance      ,
                                      cs.lpCreateParams  ) ;
    // Clear the hook if needed.
    if ( NULL != sm_hhook )
    {
        UnhookWindowsHookEx ( sm_hhook ) ;
        sm_hhook = NULL ;
    }

    return ( hWndRet ) ;
}

// The Create for simple top level windows.
HWND JWnd :: Create ( LPCTSTR szName , DWORD dwStyle )
{
    return ( CreateEx ( 0             ,
                        JWNDDEFCLASS  ,
                        szName        ,
                        dwStyle       ,
                        CW_USEDEFAULT ,
                        CW_USEDEFAULT ,
                        CW_USEDEFAULT ,
                        CW_USEDEFAULT ,
                        NULL          ,
                        NULL           ) ) ;
}



/*//////////////////////////////////////////////////////////////////////
                           Protected Methods
                       General Message Processing
//////////////////////////////////////////////////////////////////////*/

/*//////////////////////////////////////////////////////////////////////
                           Protected Methods
                            Helper Functions
//////////////////////////////////////////////////////////////////////*/
BOOL JWnd :: CenterWindow ( HWND hWndCenter , BOOL bCurrMonitor )
{
    ASSERT ( ::IsWindow ( m_hWnd ) ) ;

    // Lifted primarily from the ATL source code.

    // determine owner window to center against
    DWORD dwStyle = (DWORD)GetWindowLongPtr ( GWL_STYLE ) ;
    if ( NULL == hWndCenter )
    {
        if( dwStyle & WS_CHILD )
        {
            hWndCenter = ::GetParent ( m_hWnd ) ;
        }
        else
        {
            hWndCenter = ::GetWindow ( m_hWnd , GW_OWNER ) ;
        }
    }

    // Get coordinates of the window relative to its parent
    RECT rcDlg ;
    GetWindowRect ( &rcDlg ) ;
    RECT rcArea ;
    RECT rcCenter ;
    HWND hWndParent ;
    if ( !( dwStyle & WS_CHILD ) )
    {
        // Don't center against invisible or minimized windows
        if( NULL != hWndCenter )
        {
            DWORD dwStyle = ::GetWindowLongPtr ( hWndCenter ,
                                                 GWL_STYLE   ) ;
            if( !( dwStyle & WS_VISIBLE  ) ||
                 ( dwStyle & WS_MINIMIZE )   )
            {
                hWndCenter = NULL ;
            }
        }

        if ( FALSE == bCurrMonitor )
        {
            // Center within screen coordinates
            ::SystemParametersInfo ( SPI_GETWORKAREA ,
                                     NULL            ,
                                     &rcArea         ,
                                     NULL             ) ;
        }
        else
        {
            // Center based on the monitor containing the majority of
            // the window.
            HMONITOR hMon = MonitorFromWindow ( m_hWnd  ,
                                              MONITOR_DEFAULTTONEAREST);

            MONITORINFO stMI ;
            ZeroMemory ( &stMI , sizeof ( MONITORINFO ) ) ;
            stMI.cbSize = sizeof ( MONITORINFO ) ;

            GetMonitorInfo ( hMon , &stMI ) ;

            rcArea = stMI.rcMonitor ;
        }

        if ( NULL == hWndCenter )
        {
            rcCenter = rcArea;
        }
        else
        {
            ::GetWindowRect ( hWndCenter , &rcCenter ) ;
        }
    }
    else
    {
        // center within parent client coordinates
        hWndParent = GetParent ( ) ;

        ::GetClientRect ( hWndParent , &rcArea ) ;

        ::GetClientRect ( hWndCenter , &rcCenter ) ;
        ::MapWindowPoints ( hWndCenter        ,
                            hWndParent        ,
                            (POINT*)&rcCenter ,
                            2                  ) ;
    }

    int DlgWidth = rcDlg.right - rcDlg.left ;
    int DlgHeight = rcDlg.bottom - rcDlg.top ;

    // Find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2 ;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2 ;

    // If the dialog is outside the screen, move it inside
    if ( xLeft < rcArea.left )
    {
        xLeft = rcArea.left ;
    }
    else if ( xLeft + DlgWidth > rcArea.right )
    {
        xLeft = rcArea.right - DlgWidth ;
    }

    if ( yTop < rcArea.top )
    {
        yTop = rcArea.top ;
    }
    else if ( yTop + DlgHeight > rcArea.bottom )
    {
        yTop = rcArea.bottom - DlgHeight ;
    }

    // Map screen coordinates to child coordinates
    return ( ::SetWindowPos ( m_hWnd ,
                              NULL   ,
                              xLeft  ,
                              yTop   ,
                              -1     ,
                              -1     ,
                              SWP_NOSIZE |
                                    SWP_NOZORDER |
                                    SWP_NOACTIVATE  ) ) ;
}


/*//////////////////////////////////////////////////////////////////////
                           Protected Methods
                       Default Message Processing
//////////////////////////////////////////////////////////////////////*/
LRESULT JWnd :: DefWindowProc ( void )
{
    ASSERT ( ::IsWindow ( m_hWnd ) ) ;
    return ( DefWindowProc ( m_uMsg , m_wParam , m_lParam ) ) ;
}

LRESULT JWnd :: DefWindowProc ( UINT   uMsg   ,
                                WPARAM wParam ,
                                LPARAM lParam  )
{
    // Check to see if this is a dialog window.  If it is, return FALSE
    // as the message was not processed.
    if ( (LPVOID)JWnd::CommonDlgProc ==
                             (LPVOID)GetWindowLongPtr ( DWLP_DLGPROC ) )
    {
        // Indicate the default window proc was called for this dialog.
        m_bDefWndProcCalled = TRUE ;
        return ( FALSE ) ;
    }
    // Was this an attached JWnd?
    if ( NULL != m_pOrigWndProc )
    {
        return( ::CallWindowProc ( (WNDPROC)m_pOrigWndProc ,
                                   m_hWnd                  ,
                                   uMsg                    ,
                                   wParam                  ,
                                   lParam                   ) ) ;
    }
    return ( ::DefWindowProc ( m_hWnd , uMsg , wParam , lParam ) ) ;
}

LRESULT JWnd :: WindowProc ( UINT uMsg , WPARAM wParam , LPARAM lParam )
{
    // Save the message values.
    m_uMsg = uMsg ;
    m_wParam = wParam ;
    m_lParam = lParam ;

    return ( DefWindowProc ( uMsg , wParam , lParam ) ) ;

}



/*//////////////////////////////////////////////////////////////////////
                    Private Static Member Functions
//////////////////////////////////////////////////////////////////////*/
LRESULT WINAPI JWnd :: CallWndProc ( int    nCode  ,
                                     WPARAM wParam ,
                                     LPARAM lParam  )
{
    LRESULT lRet = CallNextHookEx ( sm_hhook , nCode , wParam , lParam ) ;
    if ( HC_ACTION == nCode )
    {
        // Get the message parameters.
        CWPSTRUCT* pCWP = (CWPSTRUCT*) lParam;

        // ======================================================
        // QUALCOMM change - Determine whether or not it's
        // a JWnd by checking the window and dialog proc's.
        // If one of them matches a JWnd proc, it's a JWnd.
        // Note that this will break if someone else subclasses
        // the window after us. But for our purposes, for now,
        // it seems to do the trick.
        // ======================================================
        WNDPROC wndProc = (WNDPROC)(INT_PTR)
                          ::GetWindowLongPtr ( pCWP->hwnd    ,
                                               GWLP_WNDPROC  ) ;
        DLGPROC dlgProc = (DLGPROC)(INT_PTR)
                          ::GetWindowLongPtr ( pCWP->hwnd    ,
                                               DWLP_DLGPROC  ) ;
        BOOL bIsJWnd = ( wndProc == (WNDPROC)JWnd::CommonWndProc ) ||
                       ( dlgProc == (DLGPROC)JWnd::CommonDlgProc ) ;
        // ======================================================
        // End QUALCOMM change
        // ======================================================

        // Get the user data out of this window.
        JWnd* pThis = (JWnd*)(INT_PTR)
                      ::GetWindowLongPtr ( pCWP->hwnd    ,
                                           GWLP_USERDATA  ) ;

        // If the user data is not set, and the TLS this pointer
        // is set, I can hook up the this pointer!
        //
        // ======================================================
        // QUALCOMM change - check for whether or not it's a
        // JWnd. Allows us to parent SUPERASSERT dialog in the
        // main window.
        // ======================================================
        //if ( ( NULL == pThis ) && ( NULL != sm_pThis ) )
        if ( bIsJWnd && ( NULL == pThis ) && ( NULL != sm_pThis ) )
        // ======================================================
        // End QUALCOMM change
        // ======================================================
        {
            // On WindowsXP, I was crashing as creating a window
            // triggers other windows, primarily the IME editor, to be
            // created actually before mine.  At this point the only
            // thing I can see safe to do is look at the class name and
            // if it's "IME", I'll kick out.  I will fix this
            // permanently when I update the dialog resource loading
            // stuff to jam in a class name I create on the fly.  That
            // way I can check if the window class starts with "JWnd"
            // and if it does, I'll know it's mine.  Of course the dialog
            // template format in memory totally sucks for easily
            // changing.  Bummer...
            TCHAR szBuff[ 100 ] ;
            GetClassName ( pCWP->hwnd , szBuff , 100 ) ;
            if ( 0 != _tcsicmp ( szBuff , _T ( "IME" ) ) )
            {
            
                // Remember to always set the HWND value!
                sm_pThis->m_hWnd = pCWP->hwnd ;

// Gotta love it.  I've NEVER found a way to cast away the /Wp64
// warnings with the functions Microsoft tells you to use for
// Win64 readiness!
#pragma warning ( disable : 4244 )
                ::SetWindowLongPtr ( pCWP->hwnd           ,
                                    GWLP_USERDATA        ,
                                    (LONG_PTR)sm_pThis    ) ;
#pragma warning ( default : 4244 )
                                 
                sm_pThis = NULL ;
                // Unhook the hook.
                UnhookWindowsHookEx ( sm_hhook ) ;
                sm_hhook = NULL ;
            }
        }
    }
    return ( lRet ) ;
}

LRESULT WINAPI JWnd :: CommonWndProc ( HWND   hWnd   ,
                                       UINT   uMsg   ,
                                       WPARAM wParam ,
                                       LPARAM lParam  )
{
// Gotta love it.  I've NEVER found a way to cast away the /Wp64
// warnings with the functions Microsoft tells you to use for
// Win64 readiness!
#pragma warning ( disable : 4244 )
    // Get the JWnd for this message.
    JWnd * pThis = (JWnd*)(LONG_PTR)::GetWindowLongPtr ( hWnd         ,
                                                         GWLP_USERDATA);
#pragma warning ( default : 4244 )
    ASSERT ( NULL != pThis ) ;

    return ( pThis->WindowProc ( uMsg , wParam , lParam ) ) ;
}

BOOL WINAPI JWnd :: CommonDlgProc ( HWND   hWnd   ,
                                    UINT   uMsg   ,
                                    WPARAM wParam ,
                                    LPARAM lParam  )
{
// Gotta love it.  I've NEVER found a way to cast away the /Wp64
// warnings with the functions Microsoft tells you to use for
// Win64 readiness!
#pragma warning ( disable : 4244 )

    // Get the JWnd for this message.
    JWnd * pThis = (JWnd*)(LONG_PTR)::GetWindowLongPtr ( hWnd         ,
                                                         GWLP_USERDATA);

#pragma warning ( default : 4244 )

    ASSERT ( NULL != pThis ) ;

    // Set the flag necessary to see if this message bounces down to the
    // default WndProc for the class.
    pThis->m_bDefWndProcCalled = FALSE ;

    // Call the window procedure.
    LRESULT lResult = pThis->WindowProc ( uMsg , wParam , lParam ) ;

    // If the message bounced through to the JWnd::DefWindowProc, just
    // return FALSE.
    if ( TRUE == pThis->m_bDefWndProcCalled )
    {
        return ( FALSE ) ;
    }
    else
    {
// Gotta love it.  I've NEVER found a way to cast away the /Wp64
// warnings with the functions Microsoft tells you to use for
// Win64 readiness!
#pragma warning ( disable : 4244 )
        // Set the return value in the dialog, and return it directly.
        ::SetWindowLongPtr ( hWnd , DWL_MSGRESULT , (LONG_PTR)lResult ) ;
#pragma warning ( default : 4244 )
        return ( (BOOL)lResult ) ;
    }
}

// Registers a window class.  You can specify the class name in
// szClassName.  If the string is empty, the library will generate a
// name for you automagically.
BOOL JfxRegisterClass ( UINT      nClassStyle   ,
                        LPTSTR    szClassName   ,
                        UINT      nSizeOfName   ,
                        HCURSOR   hCursor       ,
                        HBRUSH    hbrBackground ,
                        HICON     hIconBig      ,
                        HICON     hIconSmall    ,
                        HINSTANCE hInst          )
{
    ASSERT ( FALSE == IsBadStringPtr ( szClassName , nSizeOfName ) ) ;
    if ( TRUE == IsBadStringPtr ( szClassName , nSizeOfName ) )
    {
        return ( FALSE ) ;
    }

    // If the name is empty, generate one.
    if ( _T ( '\0' ) == szClassName[ 0 ] )
    {
        if ( ( NULL == hCursor       ) &&
             ( NULL == hbrBackground ) &&
             ( NULL == hIconBig      )    )
        {
            wsprintf ( szClassName          ,
                       _T ( "JFX:%x:%x" )   ,
                       hInst                ,
                       nClassStyle           ) ;
        }
        else
        {
            wsprintf ( szClassName                 ,
                       _T ( "JFX:%x:%x:%x:%x:%x" ) ,
                       hInst                       ,
                       nClassStyle                 ,
                       hCursor                     ,
                       hbrBackground               ,
                       hIconBig                     ) ;
        }
    }
    // Does the class already exist?
    WNDCLASSEX wndClass ;
    wndClass.cbSize = sizeof ( WNDCLASSEX ) ;
    if ( TRUE == ::GetClassInfoEx ( hInst , szClassName , &wndClass ) )
    {
        ASSERT ( wndClass.style == nClassStyle ) ;
        return ( TRUE ) ;
    }

    wndClass.cbClsExtra = 0 ;
    wndClass.hbrBackground = hbrBackground ;
    wndClass.hCursor = hCursor ;
    wndClass.hIcon = hIconBig ;
    wndClass.hIconSm = hIconSmall ;
    wndClass.hInstance = hInst ;
    wndClass.lpfnWndProc = (WNDPROC)JWnd::CommonWndProc ;
    wndClass.lpszClassName = szClassName ;
    wndClass.lpszMenuName = NULL ;
    wndClass.style = nClassStyle ;
    wndClass.cbWndExtra = 0 ;

    return ( 0 != ::RegisterClassEx ( &wndClass ) ) ;
}


#ifdef UNICODE
#define CLIPBOARDFMT    CF_UNICODETEXT
#else
#define CLIPBOARDFMT    CF_TEXT
#endif
// Copies the text to the clipboard.
BOOL CopyTextToClipboard ( LPCTSTR szMem , HWND hWndOwner )
{
    ASSERT ( FALSE == IsBadStringPtr ( szMem , MAX_PATH ) ) ;
    if ( TRUE == IsBadStringPtr ( szMem , MAX_PATH ) )
    {
        return ( FALSE ) ;
    }

    BOOL bRet = TRUE ;
    BOOL bOpenedCB = FALSE ;
    if ( TRUE == ::OpenClipboard ( hWndOwner ) )
    {
        bOpenedCB = TRUE ;
        if ( TRUE == ::EmptyClipboard ( ) )
        {
            // Do the goofy memory stuff.
            HGLOBAL hGlob = ::GlobalAlloc ( GMEM_MOVEABLE         ,
                                            (_tcslen ( szMem ) + 1) *
                                               sizeof ( TCHAR )       );
            if ( NULL != hGlob )
            {
                TCHAR * szClipMem = (TCHAR*)GlobalLock ( hGlob ) ;
                ASSERT ( NULL != szMem ) ;

                _tcscpy ( szClipMem , szMem ) ;

                GlobalUnlock ( hGlob ) ;
                if ( NULL == ::SetClipboardData ( CLIPBOARDFMT , hGlob ) )
                {
                    ASSERT ( !"SetClipboardData failed!!" ) ;
                }
            }
            else
            {
                ASSERT ( !"Unable to GlobalAlloc memory!!" ) ;
                bRet = FALSE ;
            }
        }
        else
        {
            ASSERT ( !"Unable to empty the clipboard!!" ) ;
            bRet = FALSE ;
        }
    }
    else
    {
        ASSERT ( !"Unable to open the clipboard!!" ) ;
        bRet = FALSE ;
    }
    if ( TRUE == bOpenedCB )
    {
        CloseClipboard ( ) ;
    }

    return ( bRet ) ;
}
