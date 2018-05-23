/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "JFX.h"

// Libraries necessary for JFX to work.
#pragma comment ( lib , "comctl32.lib" )

// The one global instance of the application.
JWinApp * JWinApp::m_pTheApp = NULL ;

// The defaults language internationalization.
const LPCTSTR k_DEFLANG = _T ( "ENU" ) ;


/*//////////////////////////////////////////////////////////////////////
                        Public Method Functions
                  Message Looping and Process Control
//////////////////////////////////////////////////////////////////////*/

// Takes care of loading internationalized resources.
BOOL JWinApp :: LoadInternationalResourcesDLL ( LPCTSTR szPrefix )
{
    // The rules are as follows:
    // 1.  Try and load the resource DLL for a whole language group.
    //     If the language is some sort of French, I will attempt to
    //     load *FR.DLL.
    // 2.  If I can't load *FR.DLL, I will try the whole sublanguage.
    //     For Canadian French, I will try to load *FRC.DLL
    // 3.  If both fail, I load the ENU (American English) resources.

    // Check that the application name has been set.


    // Get the user's locale.
    TCHAR szWholeLang[ 20 ] ;
    int iRet = GetLocaleInfo ( LOCALE_USER_DEFAULT    ,
                               LOCALE_SABBREVLANGNAME ,
                               szWholeLang            ,
                               20                      ) ;

    // The DLL name I am building up.
    JString sDLL ( szPrefix ) ;

    // The two character language name.
    TCHAR szMainLang[ 5 ] ;

    // If I was unable to get the language, default to ENU.
    if (  0 == iRet )
    {
        ASSERT ( !"GetLocaleInfo failed!" ) ;
        szMainLang[ 0 ] = k_DEFLANG[ 0 ] ;
        szMainLang[ 1 ] = k_DEFLANG[ 1 ] ;
        szMainLang[ 2 ] = k_DEFLANG[ 2 ] ;
        szMainLang[ 3 ] = _T( '\0' ) ;
    }
    else
    {
        szMainLang[ 0 ] = szWholeLang[ 0 ] ;
        szMainLang[ 1 ] = szWholeLang[ 1 ] ;
        szMainLang[ 2 ] = _T ( '\0' ) ;
    }

    // Slap on the main language extension.
    sDLL += szMainLang ;

    // Attempt to load the main language DLL.
    m_hInstResources = LoadLibrary ( sDLL ) ;
    if ( NULL == m_hInstResources )
    {
        // The main language does not exist.  Try and load the complete
        // language DLL.
        sDLL = szPrefix ;
        sDLL += szWholeLang ;

        m_hInstResources = LoadLibrary ( sDLL ) ;
        if ( NULL == m_hInstResources )
        {
            // The user's language resources were not found at all.
            // Attempt to load the ENU resources.
            sDLL = szPrefix ;
            sDLL += k_DEFLANG ;

            // Try and load *ENU.DLL.
            m_hInstResources = LoadLibrary ( sDLL ) ;
            ASSERT ( NULL != m_hInstResources ) ;
            if ( NULL == m_hInstResources )
            {
                m_hInstResources = m_hInstMain ;
                return ( FALSE ) ;
            }
        }

    }
    return ( TRUE ) ;
}

// The function to load a dialog resource out of the resource DLL.
LPCDLGTEMPLATE JWinApp :: LoadDialog ( LPCTSTR     szResource    ,
                                       HINSTANCE * phInstFoundIn  )
{
    ASSERT ( NULL != m_hInstResources ) ;

    HINSTANCE hInstToUse = NULL ;
    // Find the dialog resource.
    HRSRC hRSRC = FindResourceEx ( m_hInstResources ,
                                   RT_DIALOG        ,
                                   szResource       ,
                                   MAKELANGID ( LANG_NEUTRAL   ,
                                                SUBLANG_NEUTRAL ) ) ;
    if ( ( NULL == hRSRC ) && ( NULL != m_hInstMain ) )
    {
        hRSRC = FindResourceEx ( m_hInstMain      ,
                                 RT_DIALOG        ,
                                 szResource       ,
                                 MAKELANGID ( LANG_NEUTRAL   ,
                                              SUBLANG_NEUTRAL ) ) ;
        if ( NULL != hRSRC )
        {
            hInstToUse = m_hInstMain ;
        }
    }
    else
    {
        hInstToUse = m_hInstResources ;
    }

    ASSERT ( NULL != hRSRC ) ;
    if ( NULL == hRSRC )
    {
        return ( FALSE ) ;
    }

    // Now load it.
    HGLOBAL hGlob = LoadResource ( hInstToUse , hRSRC ) ;
    ASSERT ( NULL != hGlob ) ;
    if ( NULL == hGlob )
    {
        return ( FALSE ) ;
    }

    if ( NULL != phInstFoundIn )
    {
        ASSERT ( FALSE == IsBadWritePtr ( phInstFoundIn        ,
                                          sizeof ( HINSTANCE )  ) ) ;
        *phInstFoundIn = hInstToUse ;
    }

    LPCDLGTEMPLATE lpRet = (LPCDLGTEMPLATE)LockResource ( hGlob ) ;
    return ( lpRet ) ;
}

int JWinApp :: Run ( void )
{
    ASSERT ( NULL != m_pMainWnd ) ;
    ASSERT ( ::IsWindow ( m_pMainWnd->m_hWnd ) ) ;

    if ( NULL == m_pMainWnd )
    {
        return ( FALSE ) ;
    }

    MSG msg ;
    while ( TRUE == GetMessage ( &msg , NULL , 0 , 0 ) )
    {
        // Do IsDialogMessage for the keyboard stuff.
        if ( FALSE == IsDialogMessage ( m_pMainWnd->m_hWnd , &msg ) )
        {
            TranslateMessage ( &msg ) ;
            DispatchMessage ( &msg ) ;
        }
    }
    // On WM_QUIT, wParam is the exit code.
    return ( (int)msg.wParam ) ;
}

int JfxMessageBox ( UINT uiText , UINT nType )
{
    TCHAR szText[ MAX_PATH ] ;
    if ( 0 == JfxGetApp()->LoadString ( uiText , szText , MAX_PATH ) )
    {
        ASSERT ( !"Unable to load string!!" ) ;
        _tcscpy ( szText , _T ( "UNABLE TO LOAD STRING!!" ) ) ;
    }
    return ( JfxMessageBox ( szText , nType ) ) ;
}

int JfxMessageBox ( LPCTSTR szText , UINT nType )
{
    // Be a little smart about the HWND to ensure the message box always
    // gets up.
    HWND hWnd = (HWND)(JfxGetApp()->m_pMainWnd) ;
    if ( FALSE == ::IsWindowVisible ( hWnd ) )
    {
        hWnd = ::GetActiveWindow ( ) ;
    }
    return ( MessageBox ( hWnd                      ,
                          szText                    ,
                          JfxGetApp()->m_sAppName   ,
                          nType                      ) ) ;
}

int JfxMessageBox ( HWND hWnd , UINT uiText , UINT nType )
{
    TCHAR szText[ MAX_PATH ] ;
    if ( 0 == JfxGetApp()->LoadString ( uiText , szText , MAX_PATH ) )
    {
        ASSERT ( !"Unable to load string!!" ) ;
        _tcscpy ( szText , _T ( "UNABLE TO LOAD STRING!!" ) ) ;
    }
    return ( JfxMessageBox ( hWnd , szText , nType ) ) ;
}

int JfxMessageBox ( HWND hWnd , LPCTSTR szText , UINT nType )
{
    return ( MessageBox ( hWnd                      ,
                          szText                    ,
                          JfxGetApp()->m_sAppName   ,
                          nType                      ) ) ;
}


// Ye ol' WinMain.
int WINAPI WinMain ( HINSTANCE hInst , HINSTANCE , PSTR , int )
{
    // Always call InitCommonControls so XP's styles are happy.
    InitCommonControls ( ) ;
    
    // Grab the app instance.
    JWinApp * pApp = JfxGetApp ( ) ;
    pApp->m_hInstMain = hInst ;
    pApp->m_hInstResources = hInst ;

    HINSTANCE hRegInstance = pApp->GetInstanceForClassRegistration ( ) ;

    // Register the default window class provided by the class.
    VERIFY ( JfxRegisterClass ( CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW ,
                                JWNDDEFCLASS                         ,
                                (UINT)_tcslen ( JWNDDEFCLASS )       ,
                                ::LoadCursor ( NULL , IDC_ARROW )    ,
                                (HBRUSH)::GetStockObject(WHITE_BRUSH),
                                NULL                                 ,
                                NULL                                 ,
                                hRegInstance                          ));


    // If the initialization returns FALSE, don't do the message loop.
    if ( FALSE == pApp->InitInstance ( ) )
    {
        return ( pApp->ExitInstance ( ) ) ;
    }

    // Run the message loop.
    BOOL bRet = pApp->Run ( ) ;

    // Give any derived classes a chance to clean up.
    pApp->ExitInstance ( ) ;

    return ( bRet ) ;
}
