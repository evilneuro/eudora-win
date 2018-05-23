/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "PCH.h"
#include "BugslayerUtil.h"
#include "Resource.h"
#include "AssertDlg.h"
#include "AssertOptionsDlg.h"
#include "InputBox.h"

extern int g_iGlobalIgnoreCount ;

// The prototype for the MAPISendMail function.
typedef ULONG (WINAPI *PMAPISENDMAIL) ( ULONG         ,
                                        ULONG_PTR     ,
                                        MapiMessage * ,
                                        FLAGS         ,
                                        ULONG          ) ;

// The size of the grippy.
static const SIZE GRIP_SIZE = { 15, 15 } ;
// The font for the dialog.
static const LPCTSTR k_TEXTFACE = _T ( "Lucida Console" ) ;
static const int k_TEXTSIZE = 9 ;

// The menu items appended to the system menu.
static const int k_IDM_OPTIONS = 0x990 ;
static const int k_IDM_CENTERWINDOW = 0x9A00 ;

// The registry key where the assertion dialog saves it's settings.
static const LPCTSTR k_ASSERT_REG_KEY  =
    _T ( "Software\\Bugslayer\\SUPERASSERT" ) ;

// The folded value.
static const LPCTSTR k_ASSERT_REG_FOLDED_VAL = _T ( "Folded" ) ;
// The play sound value.
static const LPCTSTR k_ASSERT_REG_PLAYSOUND_VAL = _T ( "PlaySound" ) ;
// The screen position value.
static const LPCTSTR k_ASSERT_REG_LASTSCREEN_VAL =
                                                _T ( "LastScreenPos" ) ;
// The stack trace value.
static const LPCTSTR k_ASSERT_REG_DOSTACK_VAL =
                                           _T ( "StackTraceType" ) ;
// The mini dump value.
static const LPCTSTR k_ASSERT_REG_DUMPINFO_VAL = _T ( "MiniDumpInfo" ) ;

// The always on top value.
static const LPCTSTR k_ASSERT_REG_ALWAYONTOP_VAL =
                                                  _T ( "AlwaysOnTop" ) ;
                                                  
// The default locals expand depth.
static const LPCTSTR k_ASSERT_REG_LOCALSDEPTH_VAL =
                                                  _T ( "LocalsDepth" ) ;

// The show locals array value.
static const LPCTSTR k_ASSERT_REG_SHOWARRAYS_VAL = _T ( "ShowArrays" ) ;

// Minidump file open filters.
static const LPCTSTR k_DUMPFILTERS =
               _T ( "Dump Files (*.dmp)|*.dmp|All Files (*.*)|*.*||" ) ;
static const LPCTSTR k_FILEDIALOGTITLE = _T ( "Create Dump File" ) ;
static const LPCTSTR k_DEFEXTENSION = _T ( "dmp" ) ;

static TCHAR g_szMoreText[ 10 ] = _T ( "\0" ) ;
static TCHAR g_szLessText[ 10 ] = _T ( "\0" ) ;

static TCHAR * g_szThreadIDFmt =
#ifdef _WIN64
                               _T ( "0x%016X" ) ;
#else
                               _T ( "0x%08X" ) ;
#endif  // _WIN64

// This is really dumb!  Why doesn't Windows have a SM_* value that just
// returns the real caption height?!
static int g_iTitleBarSize = 0 ;

// The mapi DLL.
static LPCTSTR k_MAPIDLL = _T ( "MAPI32.DLL" ) ;
// The pointer to the MAPISendMail function.  The IsMAPIAvailible method
// sets this.
static PMAPISENDMAIL g_pMAPISendMail = NULL ;

// The Ansi strings necessary for the MAPI code.
static char * k_SMTPPREFIX = "SMTP:" ;
static char * k_EMAILSUBJECT = "SUPERASSERT Assertion Failure" ;
// The unicode error message.
static TCHAR * k_MAPIFAILEDFMT = _T ( "Sending mail failed : %08X" ) ;


/*//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////*/
class JBSUApp : public JWinApp
{
public  :
    JBSUApp ( void )
    {
    }
} ;

// The app instance for the DLL.
JBSUApp cTheApp ;

/*//////////////////////////////////////////////////////////////////////
The WindowProc (built by the macros for JAssertionDlg)
//////////////////////////////////////////////////////////////////////*/
BEGIN_MSG_MAP ( JAssertionDlg , JModalDlg )

    WM_INITDIALOG_HANDLER       (   )
    WM_GETMINMAXINFO_HANDLER    (   )
    WM_DESTROY_HANDLER          (   )
    WM_SIZE_HANDLER             (   )

    MESSAGE_HANDLER             ( WM_SETCURSOR , OnSetCursor )

    WM_PAINT_HANDLER            (   )
    WM_SYSCOMMAND_HANDLER       (   )

    COMMAND_ID_HANDLER ( IDC_LESSMORE     , OnLessMore       )
    COMMAND_ID_HANDLER ( IDC_DOSTACKTRACE , OnDoStackTrace   )
    COMMAND_ID_HANDLER ( IDC_IGNOREONCE   , OnIgnoreOnce     )
    COMMAND_ID_HANDLER ( IDC_IGNOREALWAYS , OnIgnoreAlways   )
    COMMAND_ID_HANDLER ( IDC_ABORTPROGRAM , OnAbortProgram   )
    COMMAND_ID_HANDLER ( IDC_BREAKINTODEBUGGER ,
                                         OnBreakIntoDebugger )
    COMMAND_ID_HANDLER ( IDC_IGNORE_THIS  , OnIgnoreThis     )
    COMMAND_ID_HANDLER ( IDC_IGNORE_ALL   , OnIgnoreAll      )
    COMMAND_ID_HANDLER ( IDC_COPYTOCLIPBOARD , OnCopyToClipboard )
    COMMAND_ID_HANDLER ( IDC_CREATEMINIDUMP , OnCreateMiniDump )
    COMMAND_HANDLER ( IDC_THREADCOMBO         ,
                      CBN_SELCHANGE           ,
                      OnThreadSelectionChange  )

    COMMAND_HANDLER ( IDC_DEPTHCOMBO      ,
                      CBN_SELCHANGE       ,
                      OnLocalsDepthChange  )
    COMMAND_ID_HANDLER ( IDC_SHOWARRAYSCHECK , OnShowArrays )
    COMMAND_ID_HANDLER ( IDC_EMAILASSERT , OnEmailAssert )
                      
END_MSG_MAP   ( JAssertionDlg , JModalDlg )

/*//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////*/

JAssertionDlg :: JAssertionDlg ( LPCTSTR      szExpression  ,
                                 LPCSTR       szEmail       ,
                                 DWORD64      dwStack       ,
                                 DWORD64      dwStackFrame  ,
                                 DWORD64      dwIP          ,
                                 int *        piIgnoreCount ,
                                 LPTHREADINFO pThreadInfo   ,
                                 DWORD        dwThreadCount  ) :
                JModalDlg ( IDD_ASSERTION            ,
                            NULL                     ,
                            GetBSUInstanceHandle ( )  )
{
    memset ( & m_OriginalRect , NULL , sizeof ( RECT ) ) ;
    m_FoldedHeight = 0 ;
    m_UnfoldedHeight = 0 ;
    m_RightBorder = 0 ;
    m_BottomBorder = 0 ;
    m_pFoldStates = 0 ;
    m_iWinInFold = 0 ;

    m_szExpression = szExpression ;
    m_dwStack = dwStack ;
    m_dwStackFrame = dwStackFrame ;
    m_dwIP = dwIP ;
    
    ASSERT ( NULL != piIgnoreCount ) ;
    m_piIgnoreCount = piIgnoreCount ;

    ASSERT ( NULL != pThreadInfo ) ;
    m_pThreadInfo = pThreadInfo ;
    
    m_dwThreadCount = dwThreadCount ;

    m_LastRect.left   = -1 ;
    m_LastRect.top    = 0  ;
    m_LastRect.right  = 0  ;
    m_LastRect.bottom = 0  ;
    m_bIsFolded = TRUE ;
    m_iStackWalkType = k_WALKONLYASSERTIONTHREAD ;
    m_iMiniDumpInfo = (int)(MiniDumpNormal | MiniDumpWithHandleData) ;
    m_bDoSounds = TRUE ;
    m_bAlwaysOnTop = TRUE ;
    
    m_bShowArrays = FALSE ;
    m_iLocalsDepth = 2 ;
    
    m_szEmail = szEmail ;

    // Get the real values out of the registry.
    JRegistry jReg ;
    if ( TRUE == jReg.OpenKey ( HKEY_CURRENT_USER ,
                                k_ASSERT_REG_KEY   ) )
    {
        jReg.QueryValue ( k_ASSERT_REG_FOLDED_VAL ,
                          m_bIsFolded              ) ;
        jReg.QueryValue ( k_ASSERT_REG_PLAYSOUND_VAL ,
                          m_bDoSounds                 ) ;
        jReg.QueryValue ( k_ASSERT_REG_DOSTACK_VAL ,
                          m_iStackWalkType          ) ;
        jReg.QueryValue ( k_ASSERT_REG_DUMPINFO_VAL ,
                          m_iMiniDumpInfo           ) ;
        jReg.QueryValue ( k_ASSERT_REG_LASTSCREEN_VAL ,
                          (LPVOID)&m_LastRect         ,
                          sizeof ( RECT )              ) ;
        jReg.QueryValue ( k_ASSERT_REG_ALWAYONTOP_VAL ,
                          m_bAlwaysOnTop               ) ;

        jReg.QueryValue ( k_ASSERT_REG_SHOWARRAYS_VAL ,
                          m_bShowArrays                ) ;
        jReg.QueryValue ( k_ASSERT_REG_LOCALSDEPTH_VAL ,
                          m_iLocalsDepth                ) ;
    }
    // I used to initialize this globally, but I found with VC7 that
    // GetSystemMetrics was getting called before USER32 was ready.
    if ( 0 == g_iTitleBarSize )
    {
        g_iTitleBarSize =  ::GetSystemMetrics ( SM_CYCAPTION ) +
                           ::GetSystemMetrics ( SM_CYEDGE ) +
                           ::GetSystemMetrics ( SM_CYBORDER ) +
                           ::GetSystemMetrics ( SM_CYFRAME ) + 1 ;
    }
    
    // Grab the shared symbol engine lock.
    AcquireCommonSymEngLock ( ) ;
}

JAssertionDlg :: ~JAssertionDlg ( void )
{
    ReleaseCommonSymEngLock ( ) ;
    
    if ( NULL != m_pFoldStates )
    {
        VERIFY ( HeapFree ( GetProcessHeap ( ) , 0 , m_pFoldStates ) ) ;
    }
    m_cSysMenu.Detach ( ) ;

    PersistState ( ) ;
}

void JAssertionDlg :: Fold ( BOOL bFold )
{
    if ( m_bIsFolded == bFold )
    {
        return ;
    }

    if ( _T ( '\0' ) == g_szMoreText[ 0 ] )
    {
        if ( 0 == ::LoadString ( GetBSUInstanceHandle ( ) ,
                                 IDS_MORE                 ,
                                 g_szMoreText             ,
                                 sizeof(g_szMoreText)/sizeof ( TCHAR)))
        {
            _tcscpy ( g_szMoreText , _T ( "More&>>" ) ) ;
        }
        if ( 0 == ::LoadString ( GetBSUInstanceHandle ( ) ,
                                 IDS_LESS                 ,
                                 g_szLessText             ,
                                 sizeof(g_szLessText)/sizeof ( TCHAR)))
        {
            _tcscpy ( g_szMoreText , _T ( "&Less&<<" ) ) ;
        }
    }

    m_bIsFolded = bFold ;
    InvalidateGrip ( ) ;

    // Update button
    HWND hMore = GetDlgItem ( IDC_LESSMORE ) ;
    ASSERT ( ::IsWindow ( hMore ) ) ;
    ::SendMessage ( hMore      ,
                    WM_SETTEXT ,
                    0          ,
                    (LPARAM)(m_bIsFolded ? g_szMoreText
                                         : g_szLessText ) ) ;

    // if unfolding, show controls before the MoveWindow()
    if ( !m_bIsFolded )
    {
        SaveRestoreFoldedWindowStates ( TRUE ) ;
    }

    // resize
    RECT wndRect;
    GetWindowRect( &wndRect );
    wndRect.bottom = wndRect.top + g_iTitleBarSize +
                     (m_bIsFolded ? m_FoldedHeight : m_UnfoldedHeight) ;

    MoveWindow ( wndRect ) ;

    // if folding, hide controls after the MoveWindow()
    if ( m_bIsFolded )
    {
        SaveRestoreFoldedWindowStates ( FALSE ) ;
    }
    
    // Hide the ignore group box as depending how controls are layed out
    // it can peek up if the dialog is folded.
    HWND hIgnore = GetDlgItem ( IDC_IGNOREGROUP ) ;
    ASSERT ( ::IsWindow ( hIgnore ) ) ;
    if ( TRUE == m_bIsFolded )
    {
        ::ShowWindow ( hIgnore , SW_HIDE ) ;
    }
    else
    {
        ::ShowWindow ( hIgnore , SW_NORMAL ) ;
    }
    

    InvalidateGrip();
}

void JAssertionDlg :: SaveRestoreFoldedWindowStates ( BOOL bEnable )
{
    // Check the focus to make sure we aren't going to disable it
    if ( !bEnable )
    {
        HWND focusWnd = ::GetFocus ( ) ;
        if ( focusWnd != NULL )
        {
            int focusID = ::GetDlgCtrlID( focusWnd );
            if ( focusID >= IDC_DIVIDER )
            {
                ::SetFocus( GetDlgItem( IDC_LESSMORE ) );
            }
        }
    }

    int i = 0 ;
    for ( HWND hCurr = GetWindow ( GetDlgItem( IDC_DIVIDER ) ,
                                   GW_HWNDNEXT                 ) ;
          hCurr != NULL                                          ;
          hCurr = GetWindow ( hCurr , GW_HWNDNEXT )               )
    {
        if ( bEnable )
        {
            if (  TRUE == m_pFoldStates[ i ] )
            {
                ::EnableWindow ( hCurr , TRUE ) ;
            }
        }
        else
        {
            m_pFoldStates[ i ] = !::EnableWindow ( hCurr , bEnable ) ;
        }
        i++ ;
    }
}

BOOL JAssertionDlg :: GetClientSize ( SIZE & size )
{
    RECT rect;
    BOOL bRet = GetClientRect ( rect ) ;
    size.cx = rect.right - rect.left;
    size.cy = rect.bottom - rect.top;
    return ( bRet );
}

BOOL JAssertionDlg :: GetItemWindowRectLocal ( UINT id, RECT & rect )
{
    return ( GetItemWindowRect( id, rect )                    &&
             ::ScreenToClient( m_hWnd , (POINT*)&rect.left )  &&
             ::ScreenToClient( m_hWnd, (POINT*)&rect.right )     ) ;
}

void JAssertionDlg :: InvalidateGrip( void )
{
    SIZE sz ;
    GetClientSize ( sz ) ;

    RECT rect;
    rect.left   = sz.cx - GRIP_SIZE.cx;
    rect.right  = sz.cx;
    rect.top    = sz.cy - GRIP_SIZE.cy;
    rect.bottom = sz.cy;

    InvalidateRect ( &rect , TRUE ) ;
}

void JAssertionDlg :: PersistState ( void )
{
    // Save the state!
    JRegistry jReg ;
    if ( TRUE == jReg.CreateKey ( HKEY_CURRENT_USER ,
                                  k_ASSERT_REG_KEY   ) )
    {
        jReg.SetValue ( k_ASSERT_REG_FOLDED_VAL ,
                        m_bIsFolded              ) ;
        jReg.SetValue ( k_ASSERT_REG_PLAYSOUND_VAL ,
                        m_bDoSounds                 ) ;
        jReg.SetValue ( k_ASSERT_REG_DOSTACK_VAL ,
                        m_iStackWalkType          ) ;
        jReg.SetValue ( k_ASSERT_REG_DUMPINFO_VAL ,
                        m_iMiniDumpInfo            ) ;
        jReg.SetValue ( k_ASSERT_REG_LASTSCREEN_VAL ,
                        (LPVOID)&m_LastRect         ,
                        sizeof ( RECT )              ) ;
        jReg.SetValue ( k_ASSERT_REG_ALWAYONTOP_VAL ,
                        m_bAlwaysOnTop               ) ;

        jReg.SetValue ( k_ASSERT_REG_SHOWARRAYS_VAL ,
                        m_bShowArrays                ) ;
        jReg.SetValue ( k_ASSERT_REG_LOCALSDEPTH_VAL ,
                        m_iLocalsDepth                ) ;
                        
    }
}

// The following locals are dropped in as part of the ASSERT macro so
// I need to cull them out so I don't show them and confuse the user.
#define ASSERTLOCALSCOUNT 4
TCHAR * g_szAssertLocalVars [ ASSERTLOCALSCOUNT ] =
{
    _T ( "sIgnoreCount" ) ,
    _T ( "sFailCount" )   ,
    _T ( "dwStack" )      ,
    _T ( "dwStackFrame" ) ,
} ;

BOOL CALLBACK JAssertionDlg ::
            EnumerateLocalsIntoBuffer ( DWORD64  dwAddr        ,
                                        LPCTSTR  szType        ,
                                        LPCTSTR  szName        ,
                                        LPCTSTR  szValue       ,
                                        int      iIndentLevel  ,
                                        PVOID    pUserCtx       )
{
    for ( int j = 0 ; j < ASSERTLOCALSCOUNT ; j++ )
    {
        if ( 0 == _tcscmp ( szName , g_szAssertLocalVars[ j ] ) )
        {
            return ( TRUE ) ;
        }
    }

    JString * pcBuff = (JString*)pUserCtx ;
    
    pcBuff->operator+= ( _T ( "\t\t" ) ) ;
    // Plop on the indent.
    for ( int i = 0 ; i < iIndentLevel ; i++ )
    {
        pcBuff->operator+= ( _T ( "\t" ) ) ;
    }
    
    // Add the type.
    pcBuff->operator+= ( szType ) ;

    pcBuff->operator+= ( _T ( " " ) ) ;
    
    // Add the variable name.
    pcBuff->operator+= ( szName ) ;

    pcBuff->operator+= ( _T ( " " ) ) ;
    
    if ( ( NULL == szValue ) || ( _T ( '\0' ) == szValue[ 0 ] ) )
    {
        TCHAR szAddrString[ 25 ] ;
        wsprintf ( szAddrString         ,
                   g_szThreadIDFmt      ,
#ifdef _WIN64
                   (DWORD64)dwAddr         ) ;
#else
                   (DWORD)((DWORD)dwAddr ) ) ;
#endif

        pcBuff->operator+= ( szAddrString ) ;
    }
    else
    {
        pcBuff->operator+= ( "= " ) ;
        pcBuff->operator+= ( szValue ) ;
    }
    pcBuff->operator+= ( _T ( "\r\n" ) ) ;
    
    return ( TRUE ) ;
}

void JAssertionDlg :: WalkTheSpecifiedStack ( int iIndex , BOOL bReWalk)
{
    LPTHREADINFO pThread =
                     (LPTHREADINFO)m_cThreadCombo.GetItemData ( iIndex);
    ASSERT ( NULL != pThread ) ;
    
    if ( ( TRUE == bReWalk ) && ( NULL != pThread->szStackWalk ) )
    {
        JString::DeallocateBuffer ( pThread->szStackWalk ) ;
        pThread->szStackWalk = NULL ;
    }
    else if ( NULL != pThread->szStackWalk )
    {
        // This stack has already been walked, just show it and leave.
        m_cStackTraceEdit.SetWindowText ( pThread->szStackWalk ) ;
        // Can't walk this stack again!
        m_cDoStackButton.EnableWindow ( FALSE ) ;
        return ;
    }

    // This particular stack hasn't been walked, so do it.

    HANDLE hProcess = GetCurrentProcess ( ) ;
    HANDLE hThread = pThread->hThread ;

    // Once this button is pressed, disable it.
    m_cDoStackButton.EnableWindow ( FALSE ) ;

    // Do the stack trace into a buffer.

    // Initialize the symbol engine.
    InitializeCommonSymbolEngine ( ) ;

    STACKFRAME64 stFrame ;
    DWORD      dwMachine = IMAGE_FILE_MACHINE_I386 ;

    ZeroMemory ( &stFrame , sizeof ( STACKFRAME64 ) ) ;

    stFrame.AddrPC.Mode = AddrModeFlat ;
    
    CONTEXT stCtx ;

    stFrame.AddrStack.Mode   = AddrModeFlat ;
    stFrame.AddrFrame.Mode   = AddrModeFlat ;

    // I have to special case the first value as that's the current
    // thread.
    if ( 0 == iIndex )
    {
        stFrame.AddrPC.Offset    = m_dwIP         ;
        stFrame.AddrStack.Offset = m_dwStack      ;
        stFrame.AddrFrame.Offset = m_dwStackFrame ;
    }
    else
    {
        // Grovel the context for the thread.
        stCtx.ContextFlags = CONTEXT_FULL ;

        VERIFY ( GetThreadContext ( hThread , &stCtx ) ) ;

#if defined (_M_IX86)
        dwMachine                = IMAGE_FILE_MACHINE_I386 ;
        stFrame.AddrPC.Offset    = stCtx.Eip    ;
        stFrame.AddrStack.Offset = stCtx.Esp    ;
        stFrame.AddrFrame.Offset = stCtx.Ebp    ;
#elif defined (_M_AMD64)
        dwMachine                = IMAGE_FILE_MACHINE_AMD64 ;
        stFrame.AddrPC.Offset    = stCtx.RIP    ;
        stFrame.AddrStack.Offset = stCtx.RSP    ;
        stFrame.AddrFrame.Offset = stCtx.RBP    ;
#else
#error ( "Unknown machine!" )
#endif
    }


    // Allocate space for the symbol name and source file.  Since VC7
    // symbols are now no longer truncated at 255 characters, you have
    // to assume everything is gigantic.  (Think STL).
    TCHAR * szSym = (TCHAR*)HeapAlloc ( GetProcessHeap ( )       ,
                                        HEAP_GENERATE_EXCEPTIONS ,
                                        2048                      ) ;

    // Create a JString on the stack which does not delete the buffer
    // so I can put that buffer in the THREADINFO associated with the
    // list box data.
    JString cStackBuffer ( false ) ;
    
    // Set the minimum amount to allocate each time the string runs out
    // of room to something pretty big as we'll be pumping a lot into
    // here with all the locals.
    cStackBuffer.SetMinumumAllocationLength ( 16384 ) ;

    // Before doing the stack walk, tickle the symbol engine so
    // that it gets the initial module loaded.  The reason I need to do
    // this is that the first stack walk does not call GetModBase
    // part of the time.  Since the symbol engine only loads modules on
    // demand, I need to prime it for the first address.
    GetModBase ( hProcess , m_dwIP ) ;

    // Loop for the first 512 stack elements.
    for ( DWORD i = 0 ; i < 512 ; i++ )
    {
        if ( FALSE == StackWalk64 ( dwMachine                ,
                                    hProcess                 ,
                                    hThread                  ,
                                    &stFrame                 ,
                                    &stCtx                   ,
                                    NULL                     ,
                                    SymFunctionTableAccess64 ,
                                    GetModBase               ,
                                    NULL                      ) )
        {
            break ;
        }
        // Also check that the address is not zero.  Sometimes
        // StackWalk returns TRUE with a frame of zero.
        if ( 0 != stFrame.AddrPC.Offset )
        {
            // ConvertAddress limits symbol names to 1024 so my 2048
            // size buffer is cool.
            ConvertAddress ( stFrame.AddrPC.Offset ,
                             szSym                 ,
                             _T ( "\r\n" )          ) ;
            // Append this value onto the collected string.
            cStackBuffer += szSym ;
            
            JString * pStr = &cStackBuffer ;
            
            // Walk the happy locals.
            CommonSymbolEngineEnumLocalVariables
                                        ( EnumerateLocalsIntoBuffer ,
                                          m_iLocalsDepth            ,
                                          m_bShowArrays             ,
                                          NULL                      ,
                                          &stFrame                  ,
                                          &stCtx                    ,
                                          pStr                       ) ;
        }
    }
    
    // I can get rid of the memory allocated to hold symbol and line
    // information.
    VERIFY ( HeapFree ( GetProcessHeap ( ) , 0 , szSym ) ) ;
    
    // Set the allocated memory into the item data.
    pThread->szStackWalk = cStackBuffer.GetBuffer ( ) ;

    m_cStackTraceEdit.SetWindowText ( pThread->szStackWalk ) ;

    // Only do it to other places if this is not a rewalk.
    if ( FALSE == bReWalk )
    {
        // Is the trace also supposed to go other places?
        if ( DA_SHOWODS == ( DA_SHOWODS & GetDiagAssertOptions ( ) ) )
        {
            OutputDebugString ( pThread->szStackWalk ) ;
            OutputDebugString ( _T ( "\n" ) ) ;
        }

        if ( INVALID_HANDLE_VALUE != GetDiagAssertFile ( ) )
        {
            DWORD dwWritten ;
            int    iLen = lstrlen ( pThread->szStackWalk ) ;
            char * pToWrite = NULL ;

#ifdef UNICODE
            pToWrite = (char*)_alloca ( iLen + 1 ) ;

            BSUWide2Ansi ( pThread->szStackWalk , pToWrite , iLen + 1 );
#else
            pToWrite = szTrace ;
#endif
            WriteFile ( GetDiagAssertFile ( )   ,
                        pToWrite                ,
                        iLen                    ,
                        &dwWritten              ,
                        NULL                     ) ;
        }
    }
}

BOOL JAssertionDlg :: OnInitDialog ( HWND , LPARAM )
{

    // Start with the simple stuff....
    // Set the X icon.
    HWND hIconWnd = GetDlgItem ( IDC_XICON ) ;
    ASSERT ( ::IsWindow ( hIconWnd ) ) ;
    HICON hIcon = JfxGetApp()->LoadStandardIcon ( IDI_HAND ) ;
    ::SendMessage ( hIconWnd      ,
                    STM_SETICON   ,
                    (WPARAM)hIcon ,
                    0              ) ;

    // Set the dialog icon while I'm at it.
    SetIcon ( hIcon , FALSE ) ;
    SetIcon ( hIcon , TRUE ) ;

    // Set the ignore count edit control to zero.
    HWND hWndEdit = GetDlgItem ( IDC_IGNORECOUNT ) ;
    ASSERT ( ::IsWindow ( hWndEdit ) ) ;
    ::SendMessage ( hWndEdit           ,
                    WM_SETTEXT         ,
                    0                  ,
                    (LPARAM)_T ( "0" )  ) ;

    // Get the starting dialog dimensions.
    GetWindowRect ( m_OriginalRect ) ;
    m_UnfoldedHeight = m_OriginalRect.bottom - m_OriginalRect.top ;

    // Adjustments for folded size
    RECT childRect = { 0 , 0 , 0 , 0 } ;
    GetItemWindowRect ( IDC_DIVIDER, childRect ) ;
    m_FoldedHeight = childRect.top - m_OriginalRect.top ;

    // borders
    GetItemWindowRect( IDC_STACKTRACE, childRect );
    m_RightBorder  = m_OriginalRect.right  - childRect.right;
    m_BottomBorder = m_OriginalRect.bottom - childRect.bottom;


    // Put me in the right spot (if centering, use unfolded dialog)
    if ( -1 != m_LastRect.left )
    {
            ::SetWindowPos( m_hWnd                              ,
                            NULL                                ,
                            m_LastRect.left                     ,
                            m_LastRect.top                      ,
                            m_LastRect.right - m_LastRect.left  ,
                            m_LastRect.bottom - m_LastRect.top  ,
                            SWP_NOZORDER |
                                SWP_NOACTIVATE | SWP_NOREDRAW    ) ;
            m_UnfoldedHeight = m_LastRect.bottom - m_LastRect.top ;
    }
    else
    {
            CenterWindow ( ) ;
    }

    // Get the controls on the dialog I need.
    VERIFY ( TRUE == m_cAssertEdit.Attach ( *this , IDC_EXPRESSIONS ) );
    VERIFY ( TRUE == m_cStackTraceEdit.Attach( *this , IDC_STACKTRACE));
    VERIFY ( TRUE == m_cDoStackButton.Attach ( *this            ,
                                               IDC_DOSTACKTRACE  ) ) ;
    VERIFY ( TRUE == m_cThreadCombo.Attach ( *this , IDC_THREADCOMBO ));
    VERIFY ( TRUE == m_cThreadStatic.Attach ( *this            ,
                                              IDC_THREADSTATIC  ) ) ;
    VERIFY ( TRUE == m_cMiniDumpButton.Attach ( *this ,
                                                IDC_CREATEMINIDUMP ) ) ;
    VERIFY ( TRUE == m_cLocalsDepthCombo.Attach ( *this ,
                                                  IDC_DEPTHCOMBO ) ) ;
    VERIFY ( TRUE == m_cShowArraysCheck.Attach ( *this ,
                                                 IDC_SHOWARRAYSCHECK ));
    VERIFY ( TRUE == m_cEmailButton.Attach ( *this , IDC_EMAILASSERT ));
                                                 
    // Set the locals depth and if I'm supposed to expand locals arrays.
    m_cShowArraysCheck.SetCheck ( m_bShowArrays ) ;

    // As this the values are hard coded, why calculate them?
    m_cLocalsDepthCombo.AddString ( _T ( "0" ) ) ;
    m_cLocalsDepthCombo.AddString ( _T ( "1" ) ) ;
    m_cLocalsDepthCombo.AddString ( _T ( "2" ) ) ;
    m_cLocalsDepthCombo.AddString ( _T ( "3" ) ) ;
    m_cLocalsDepthCombo.AddString ( _T ( "4" ) ) ;
    m_cLocalsDepthCombo.AddString ( _T ( "5" ) ) ;
    m_cLocalsDepthCombo.AddString ( _T ( "6" ) ) ;
    m_cLocalsDepthCombo.AddString ( _T ( "7" ) ) ;
    m_cLocalsDepthCombo.AddString ( _T ( "8" ) ) ;
    m_cLocalsDepthCombo.AddString ( _T ( "9" ) ) ;
    
    m_cLocalsDepthCombo.SetCurSel ( m_iLocalsDepth ) ;

    // Set the font.
    if ( TRUE == m_cFont.CreatePointFont ( k_TEXTSIZE ,
                                           k_TEXTFACE  ) )
    {
        m_cAssertEdit.SetFont ( m_cFont , FALSE ) ;
        m_cStackTraceEdit.SetFont ( m_cFont , FALSE ) ;
    }
    else
    {
        ASSERT ( !"Unable to create the font!!" ) ;
    }

    // Set the expression text.
    m_cAssertEdit.SetWindowText ( m_szExpression ) ;

    // If the version of DBGHELP.DLL in memory can't handle minidumps,
    // disable the button.  That way the user see's that it's there,
    // but not availible yet.
    if ( FALSE == IsMiniDumpFunctionAvailable ( ) )
    {
        m_cMiniDumpButton.EnableWindow ( FALSE ) ;
    }
    
    // If there's no email address specified and I can't find the MAPI
    // stuff, disable the email button.
    if ( ( NULL == m_szEmail ) || ( FALSE == IsMAPIAvailable ( ) ) )
    {
        m_cEmailButton.EnableWindow ( FALSE ) ;
    }

    // The initial thread is always plopped into the thread combo box
    // as it's used in the stack walking code.
    TCHAR szThreadId [ 20 ] ;
    wsprintf ( szThreadId , g_szThreadIDFmt , m_pThreadInfo[0].dwTID ) ;

    int iCurr =
        m_cThreadCombo.AddStringAndData ( szThreadId              ,
                                         (void*)&m_pThreadInfo[0]  ) ;
    m_cThreadCombo.SetCurSel ( iCurr ) ;

    // If there's more than one thread, show the thread combo box
    // and set it up.
    if ( m_dwThreadCount > 1 )
    {
        // Show them.
        m_cThreadCombo.ShowWindow ( SW_SHOWNORMAL ) ;
        m_cThreadStatic.ShowWindow ( SW_SHOWNORMAL ) ;

        for ( DWORD i = 1 ; i < m_dwThreadCount ; i++ )
        {
            wsprintf ( szThreadId             ,
                       g_szThreadIDFmt        ,
                       m_pThreadInfo[i].dwTID  ) ;
            m_cThreadCombo.AddStringAndData ( szThreadId              ,
                                              (void*)&m_pThreadInfo[i]);
        }
    }

    // Set the tab stops in the stack window.
    UINT uiTabs = 22 ;
    m_cStackTraceEdit.SendMessage ( EM_SETTABSTOPS ,
                                    1              ,
                                    (LPARAM)&uiTabs ) ;

    // Fiddle the check state.
    if ( k_WALKONLYASSERTIONTHREAD == m_iStackWalkType )
    {
        // Do the stack trace immediately.
        OnDoStackTrace ( ) ;
        m_cDoStackButton.EnableWindow ( FALSE ) ;
    }
    else if ( k_WALKALLTHREADS == m_iStackWalkType )
    {
        for ( DWORD i = 0 ; i < m_dwThreadCount ; i++ )
        {
            WalkTheSpecifiedStack ( i ) ;
        }
        // Set the focus to the first one.
        WalkTheSpecifiedStack ( 0 ) ;
    }

    // Count the number of windows after the IDC_DIVIDER line.
    m_iWinInFold = 0 ;
    for ( HWND hCurr = GetWindow ( GetDlgItem( IDC_DIVIDER ) ,
                                   GW_HWNDNEXT                 )   ;
          hCurr != NULL                                            ;
          hCurr = GetWindow ( hCurr , GW_HWNDNEXT )                 )
    {
        m_iWinInFold++ ;
    }

    // Allocate the array to hold the folded area's window states.
    m_pFoldStates = (BYTE*)HeapAlloc ( GetProcessHeap ( )             ,
                                       HEAP_GENERATE_EXCEPTIONS       ,
                                       sizeof ( BYTE ) * m_iWinInFold  ) ;

    // Loop through the windows again and record their states.
    int i = 0 ;
    for ( hCurr = GetWindow ( GetDlgItem( IDC_DIVIDER ) ,
                              GW_HWNDNEXT                 ) ;
          hCurr != NULL                                     ;
          hCurr = GetWindow ( hCurr , GW_HWNDNEXT )          )
    {
        m_pFoldStates [ i ] = (BYTE)::IsWindowEnabled ( hCurr ) ;
        i++ ;
    }

    // Do the folded state.
    BOOL bOldState = m_bIsFolded ;
    m_bIsFolded = FALSE ;
    Fold ( bOldState ) ;

    // Get the system menu and append the options option and the
    // center in window command.!
    JString cOptString ;
    JString cCenterWndString ;
    //JStringSetResInst ( GetBSUInstanceHandle ( ) ) ;
    if ( FALSE == cOptString.LoadString ( IDS_OPTIONS ) )
    {
        cOptString = _T ( "&Options" ) ;
    }
    if ( FALSE == cCenterWndString.LoadString ( IDS_CENTERWINDOW ) )
    {
        cCenterWndString = _T ( "C&enter on Monitor" ) ;
    }

    HMENU hSysMenu = GetSystemMenu ( ) ;
    if ( NULL != hSysMenu )
    {
        m_cSysMenu.Attach ( hSysMenu , FALSE ) ;
        m_cSysMenu.AppendMenu ( MF_SEPARATOR ) ;
        m_cSysMenu.AppendMenu ( MF_STRING     ,
                                k_IDM_OPTIONS ,
                                cOptString     ) ;
        m_cSysMenu.AppendMenu ( MF_SEPARATOR ) ;
        m_cSysMenu.AppendMenu ( MF_STRING          ,
                                k_IDM_CENTERWINDOW ,
                                cCenterWndString    ) ;

    }

    // If sounds are supposed to be on, play it again, Sam.
    if ( TRUE == m_bDoSounds )
    {
        ::MessageBeep ( MB_ICONHAND ) ;
    }

    if ( ( FALSE == m_bAlwaysOnTop        ) ||
         ( TRUE  == IsDebuggerPresent ( ) )    )
    {
        ::SetWindowPos ( m_hWnd                 ,
                         HWND_NOTOPMOST         ,
                         0                      ,
                         0                      ,
                         0                      ,
                         0                      ,
                         SWP_NOMOVE           |
                            SWP_NOOWNERZORDER |
                            SWP_NOREPOSITION  |
                            SWP_NOSIZE          ) ;
    }

    // Set focus to the ignore once button.
    ::SetFocus ( GetDlgItem ( IDC_IGNOREONCE ) ) ;

    return ( FALSE ) ;
}

int JAssertionDlg :: OnGetMinMaxInfo ( MINMAXINFO * lpMinMaxInfo )
{
    lpMinMaxInfo->ptMinTrackSize.x =
                            m_OriginalRect.right - m_OriginalRect.left ;

    if ( m_bIsFolded )
    {
        lpMinMaxInfo->ptMinTrackSize.y = m_FoldedHeight;
        lpMinMaxInfo->ptMaxTrackSize.y = m_FoldedHeight;
    }
    else
    {
        lpMinMaxInfo->ptMinTrackSize.y =
                            m_OriginalRect.bottom - m_OriginalRect.top ;
    }

    InvalidateGrip ( ) ;

    return ( FALSE );
}

void JAssertionDlg :: OnDestroy ( void )
{
    // Save off the last position.
    GetWindowRect ( m_LastRect ) ;

    // Whip through the thread information array and free any strings
    // allocated.
    for ( DWORD i = 0 ; i < m_dwThreadCount ; i++ )
    {
        if ( NULL != m_pThreadInfo[i].szStackWalk )
        {
            JString::DeallocateBuffer ( m_pThreadInfo[i].szStackWalk ) ;
        }
    }
}

void JAssertionDlg :: OnSize ( int iSizeReq   ,
                               int /*iWidth*/ ,
                               int iHeight     )
{
    // Don't do anything if this anything other than SIZE_RESTORED.
    if ( SIZE_RESTORED != iSizeReq )
    {
        return ;
    }

    // Update saved size for next time we fold then unfold
    if ( FALSE == m_bIsFolded )
    {
        m_UnfoldedHeight = iHeight ;
    }

    // get dialog size
    SIZE clientSize;
    GetClientSize( clientSize );

    // Resize assert text box
    RECT childRect ;
    GetItemWindowRectLocal ( IDC_EXPRESSIONS , childRect ) ;
    childRect.right = clientSize.cx - m_RightBorder ;
    ::SetWindowPos ( GetDlgItem( IDC_EXPRESSIONS )       ,
                     NULL                                ,
                     -1                                  ,
                     -1                                  ,
                     childRect.right - childRect.left    ,
                     childRect.bottom - childRect.top    ,
                     SWP_NOMOVE |
                         SWP_NOZORDER |
                         SWP_NOACTIVATE                   ) ;

    // Move more/less button
    GetItemWindowRectLocal ( IDC_LESSMORE, childRect ) ;
    childRect.left = ( clientSize.cx   - m_RightBorder  ) -
                     ( childRect.right - childRect.left )   ;
    ::SetWindowPos ( GetDlgItem( IDC_LESSMORE )     ,
                     NULL                           ,
                     childRect.left                 ,
                     childRect.top                  ,
                     -1                             ,
                     -1                             ,
                     SWP_NOSIZE |
                        SWP_NOZORDER |
                        SWP_NOACTIVATE               ) ;

    // Resize trace text box
    GetItemWindowRectLocal ( IDC_STACKTRACE, childRect ) ;
    childRect.right  = clientSize.cx - m_RightBorder ;
    childRect.bottom = clientSize.cy - m_BottomBorder ;
    ::SetWindowPos( GetDlgItem ( IDC_STACKTRACE )       ,
                    NULL                                ,
                    -1                                  ,
                    -1                                  ,
                    childRect.right - childRect.left    ,
                    childRect.bottom - childRect.top    ,
                    SWP_NOMOVE |
                        SWP_NOZORDER |
                        SWP_NOACTIVATE |
                        (m_bIsFolded ? SWP_NOREDRAW
                                     : 0           )    ) ;

    // Resize Ignore group box.
    GetItemWindowRectLocal ( IDC_IGNOREGROUP, childRect ) ;
    childRect.right  = clientSize.cx - m_RightBorder ;
    ::SetWindowPos( GetDlgItem ( IDC_IGNOREGROUP )       ,
                    NULL                                 ,
                    -1                                   ,
                    -1                                   ,
                    childRect.right - childRect.left     ,
                    childRect.bottom - childRect.top     ,
                    SWP_NOMOVE |
                        SWP_NOZORDER |
                        SWP_NOACTIVATE |
                        (m_bIsFolded ? SWP_NOREDRAW
                                     : 0           )    ) ;

    // Resize stack group box.
    GetItemWindowRectLocal ( IDC_STACKGROUP , childRect ) ;
    childRect.right  = clientSize.cx - m_RightBorder ;
    ::SetWindowPos( GetDlgItem ( IDC_STACKGROUP )        ,
                    NULL                                 ,
                    -1                                   ,
                    -1                                   ,
                    childRect.right - childRect.left     ,
                    childRect.bottom - childRect.top     ,
                    SWP_NOMOVE |
                        SWP_NOZORDER |
                        SWP_NOACTIVATE |
                        (m_bIsFolded ? SWP_NOREDRAW
                                     : 0           )    ) ;

    // Redo the grip.
    InvalidateGrip ( ) ;

}

LRESULT JAssertionDlg :: OnSetCursor ( WPARAM wParam , LPARAM lParam )
{
    HWND hCursorWnd = (HWND)wParam ;
    UINT uiHitCode = LOWORD ( lParam ) ;

    if ( ( TRUE == m_bIsFolded  ) &&
         ( m_hWnd == hCursorWnd )    )
    {
        switch ( uiHitCode )
        {
            case HTTOPLEFT :
            case HTBOTTOMLEFT :
            case HTTOPRIGHT :
            case HTBOTTOMRIGHT :
            {
                ::SetCursor ( LoadCursor ( NULL , IDC_SIZEWE ) ) ;
                return ( TRUE ) ;
            }   break ;

            case HTTOP :
            case HTBOTTOM :
            {
                ::SetCursor ( LoadCursor ( NULL , IDC_ARROW ) );
                return ( TRUE ) ;
            }   break ;

            default :
                break ;
        }
    }
    return ( FALSE ) ;
}

void JAssertionDlg :: OnPaint ( void )
{
    PAINTSTRUCT paint ;
    HDC hdc = BeginPaint ( &paint );

    RECT rect;
    GetClientRect( &rect ) ;
    rect.top = rect.bottom - GRIP_SIZE.cx ;
    rect.left = rect.right - GRIP_SIZE.cy ;
    DrawFrameControl ( hdc        ,
                       &rect      ,
                       DFC_SCROLL ,
                       DFCS_SCROLLSIZEGRIP ) ;

    EndPaint ( &paint );

}

BOOL JAssertionDlg :: OnSysCommand ( int iCmdType )
{
    if ( ( iCmdType & 0xFFF0 ) == k_IDM_OPTIONS )
    {
        JAssertionOptionsDlg cDlg ( m_iStackWalkType ,
                                    m_iMiniDumpInfo  ,
                                    m_bDoSounds      ,
                                    m_bAlwaysOnTop   ,
                                    this              ) ;

        if ( IDOK == cDlg.DoModal ( ) )
        {
            // The only one of import here is if the user wants to
            // rip through and do all stacks now.
            if ( k_WALKALLTHREADS == m_iStackWalkType )
            {
                int iSel = m_cThreadCombo.GetCurSel ( ) ;
                for ( DWORD i = 0 ; i < m_dwThreadCount ; i++ )
                {
                    WalkTheSpecifiedStack ( i ) ;
                }
                // Set the focus to the previously selected one.
                WalkTheSpecifiedStack ( iSel ) ;
            }
            if ( ( TRUE  == m_bAlwaysOnTop        ) &&
                 ( FALSE == IsDebuggerPresent ( ) )    )
            {
                ::SetWindowPos ( m_hWnd                 ,
                                 HWND_TOPMOST           ,
                                 0                      ,
                                 0                      ,
                                 0                      ,
                                 0                      ,
                                 SWP_NOMOVE           |
                                    SWP_NOOWNERZORDER |
                                    SWP_NOREPOSITION  |
                                    SWP_NOSIZE          ) ;
            }
            else
            {
                ::SetWindowPos ( m_hWnd                 ,
                                 HWND_NOTOPMOST         ,
                                 0                      ,
                                 0                      ,
                                 0                      ,
                                 0                      ,
                                 SWP_NOMOVE           |
                                    SWP_NOOWNERZORDER |
                                    SWP_NOREPOSITION  |
                                    SWP_NOSIZE          ) ;

            }
        }
        return ( TRUE ) ;
    }
    else if ( ( iCmdType & 0xFFF0 ) == k_IDM_CENTERWINDOW )
    {
        CenterWindow ( NULL , TRUE ) ;
        return ( TRUE ) ;
    }
    return ( (BOOL)DEF_WM_SYSCOMMAND_HANDLING ( iCmdType ) ) ;
}

void JAssertionDlg :: OnLessMore ( void )
{
    ToggleFold ( ) ;
}

void JAssertionDlg :: OnDoStackTrace ( void )
{
    // Get the information about the current selected item in the thread
    // combo box.
    int iSel = m_cThreadCombo.GetCurSel ( ) ;
    ASSERT ( CB_ERR != iSel ) ;

    WalkTheSpecifiedStack ( iSel ) ;
}

void JAssertionDlg :: OnIgnoreOnce ( void )
{
    EndDialog ( IDIGNORE ) ;
}

void JAssertionDlg :: OnAbortProgram ( void )
{
    // ExitProcess is going to get called so I need to save the dialog
    // state since the destructor won't have a chance.
    PersistState ( ) ;
    EndDialog ( IDABORT );
}

void JAssertionDlg :: OnBreakIntoDebugger ( void )
{
    // Just in case the user kills the process from the debugger, the
    // dialog settings need to be saved.
    PersistState ( ) ;

    EndDialog ( IDRETRY ) ;
}

void JAssertionDlg :: OnIgnoreAlways ( void )
{
    // Set the ignore flag for this particular assertion to -1 and
    // it will never come this way again!
    *m_piIgnoreCount = -1 ;
    EndDialog ( IDIGNORE ) ;
}

void JAssertionDlg :: OnIgnoreThis ( void )
{
    // Get the value out of the edit control and set the ignore count
    // for this local assertion.
    *m_piIgnoreCount = GetDlgItemInt ( IDC_IGNORECOUNT ) ;
    EndDialog ( IDIGNORE ) ;
}

void JAssertionDlg :: OnIgnoreAll ( void )
{
    // Get the value out of the edit control and set the global ignore
    // count.
    g_iGlobalIgnoreCount = GetDlgItemInt ( IDC_IGNORECOUNT ) ;
    EndDialog ( IDIGNORE ) ;
}

// Pressing the "X" is just like pressing Ignore Once.
void JAssertionDlg :: OnCancel ( void )
{
    EndDialog ( IDIGNORE ) ;
}

const TCHAR * k_CRLFSTR = _T ( "\r\n" ) ;
const int     k_CRLFSTRLEN = 2 * sizeof ( TCHAR ) ;
const TCHAR * k_THREADIDFMTSTR =
#ifdef _WIN64
_T ( "\r\nThread ID : 0x%016X\r\n" ) ;
#else
_T ( "\r\nThread ID : 0x%08X\r\n" ) ;
#endif
const int     k_THREADIDSTRLEN = 24 * sizeof ( TCHAR ) ;

LPCTSTR JAssertionDlg :: AllocAndCopyAssertionData ( size_t * pChrCount)
{
    // Get the size of the assertion buffer.
    int iAssertLen = m_cAssertEdit.GetWindowTextLength ( ) ;
    // Add on room for the NULL and CR/LF.
    iAssertLen += k_CRLFSTRLEN ;

    // Loop through and add up the sizes of all the stacks that have
    // been walked.
    size_t iTotalStackLen = 0 ;
    for ( DWORD i = 0 ; i < m_dwThreadCount ; i++ )
    {
        if ( NULL != m_pThreadInfo[ i ].szStackWalk )
        {
            iTotalStackLen +=
                    _tcslen ( m_pThreadInfo[ i ].szStackWalk ) ;
            iTotalStackLen += k_CRLFSTRLEN ;
            iTotalStackLen += k_THREADIDSTRLEN ;
        }
    }
    
    size_t iCharLen = ( iAssertLen + iTotalStackLen    ) *
                      ( k_CRLFSTRLEN * m_dwThreadCount )   ;
    if ( NULL != pChrCount )
    {
        *pChrCount = iCharLen ;
    }

    // Allocate a buffer big enough to hold everything.
    TCHAR * szClipBuff =
            (TCHAR*)HeapAlloc ( GetProcessHeap ( )          ,
                                HEAP_GENERATE_EXCEPTIONS    ,
                                iCharLen * sizeof ( TCHAR )  ) ;

    // Add the assertion text.
    m_cAssertEdit.GetWindowText ( szClipBuff , iAssertLen ) ;
    _tcscat ( szClipBuff , _T ( "\r\n" ) ) ;

    // Move up to the last position in the buffer.
    TCHAR * pCurrPos = (szClipBuff + iAssertLen) - sizeof ( TCHAR ) ;

    // Loop through the stacks again and plop any already walked into
    // the buffer.
    for ( i = 0 ; i < m_dwThreadCount ; i++ )
    {
        if ( NULL != m_pThreadInfo[ i ].szStackWalk )
        {
            pCurrPos += wsprintf ( pCurrPos                  ,
                                   k_THREADIDFMTSTR          ,
                                   m_pThreadInfo[ i ].dwTID   ) ;
            // The largest buffer wsprintf will handle is 1024
            // characters.  Since the stack walk with locals can get
            // huge, I have to do it manually.
            size_t iLen = _tcslen ( m_pThreadInfo[ i ].szStackWalk ) ;
            memcpy ( pCurrPos                       ,
                     m_pThreadInfo[ i ].szStackWalk ,
                     iLen * sizeof ( TCHAR )         ) ;
            pCurrPos += iLen ;
            _tcscpy ( pCurrPos , _T ( "\r\n" ) ) ;
        }
    }
   return ( szClipBuff ) ;
}

void JAssertionDlg :: FreeAssertionData ( LPCTSTR szData )
{
    VERIFY ( HeapFree ( GetProcessHeap ( ) , 0 , (LPVOID)szData ) ) ;
}

// Copy the important information to the clipboard.
void JAssertionDlg :: OnCopyToClipboard ( void )
{
    // Copy all the data.
    LPCTSTR szClipBuff = AllocAndCopyAssertionData ( ) ;

    // Do the actual copy.
    VERIFY ( CopyTextToClipboard ( szClipBuff , *this ) ) ;

    // Get rid of the buffer.
    FreeAssertionData ( szClipBuff ) ;
}

void JAssertionDlg :: OnCreateMiniDump ( void )
{
    // Originally, I used the common file dialogs here.  Everything
    // worked great until the second time I went to write a dump
    // file.  The Save common dialog would hang after I selected the
    // filename to write to.  After some head scratching, I figured out
    // what was going on.  The common file dialogs create a bunch of
    // background threads.  Those threads are still active, though
    // suspended, after the common file dialogs go away.  That way the
    // next instance of the file dialog does not have to go through
    // the time consuming task of cranking threads again.  That's all
    // fine and good in normal applications.  However, in this assertion
    // code, the first thing it does is suspend all threads except for
    // the current one.  Since I was suspending the threads necessary
    // for the common file dialogs, it would hang.  While I could
    // probably hack it so that I kill any threads I didn't suspend
    // originally, that would be ugly.  Consequently, I have to resort
    // to my own dialog to get the filename.  If you are motivated,
    // feel free to write your own file dialog to drop in here.

    // Will hold the final file.
    TCHAR szFile[ MAX_PATH ] ;
    // Holds the caption.
    TCHAR szCaption [ MAX_PATH ] ;
    // Holds the prompt.
    TCHAR szPromptText[ MAX_PATH ] ;

    if ( 0 == LoadString ( GetBSUInstanceHandle ( ) ,
                           IDS_CREATEDUMPTITLE      ,
                           szCaption                ,
                           sizeof(szCaption)/sizeof ( TCHAR ) ) )
    {
        _tcscpy ( szCaption , _T ( "Create Mini Dump" ) ) ;
    }
    if ( 0 == LoadString ( GetBSUInstanceHandle ( ) ,
                           IDS_PROMPTTEXT           ,
                           szPromptText             ,
                           sizeof(szPromptText)/sizeof ( TCHAR ) ) )
    {
        _tcscpy ( szPromptText ,
         _T("Enter the complete path and name of the mini dump file:"));
    }
    ZeroMemory ( szFile , MAX_PATH * sizeof ( TCHAR ) ) ;

    JInputBox cInput ( szCaption    ,
                       szPromptText ,
                       szFile       ,
                       MAX_PATH     ,
                       this          ) ;

    if ( ( IDOK == cInput.DoModal ( ) ) &&
         ( _T ( '\0' ) != szFile[ 0 ] )    )
    {
        BSUMDRET eRet =
          SnapCurrentProcessMiniDump( (MINIDUMP_TYPE)m_iMiniDumpInfo ,
                                       szFile                         );

        if ( eDUMP_SUCCEEDED != eRet )
        {
            // Yikes!  Gotta error.
            // Reuse the buffers from above so I don't kill the stack.
            wsprintf ( szFile                                ,
                       _T ( "Minidump creation failed: %d" ) ,
                       eRet                                   ) ;
            ::MessageBox ( m_hWnd              ,
                          szFile               ,
                          _T ( "SUPERASSERT" ) ,
                          MB_OK                 ) ;
        }
    }
}

void JAssertionDlg :: OnThreadSelectionChange ( void )
{
    // Get the selected list box dude.
    int iIndex = m_cThreadCombo.GetCurSel ( ) ;

    LPTHREADINFO pThread =
                   (LPTHREADINFO)m_cThreadCombo.GetItemData(iIndex);
    ASSERT ( NULL != pThread ) ;

    // If the stack's already been walked for this thread, blast it
    // in and get out.
    if ( NULL != pThread->szStackWalk )
    {
        m_cStackTraceEdit.SetWindowText ( pThread->szStackWalk ) ;
        // Can't walk this stack again!
        m_cDoStackButton.EnableWindow ( FALSE ) ;
        return ;
    }

    // Zero out the text that's there.
    m_cStackTraceEdit.SetWindowText ( NULL ) ;
    m_cDoStackButton.EnableWindow ( ) ;
}

void JAssertionDlg :: OnLocalsDepthChange ( void )
{
    m_iLocalsDepth = m_cLocalsDepthCombo.GetCurSel ( ) ;
    WalkTheSpecifiedStack ( m_cThreadCombo.GetCurSel ( ) , TRUE ) ;
}

void JAssertionDlg :: OnShowArrays ( void )
{
    m_bShowArrays =  m_cShowArraysCheck.GetCheck ( ) ;
    WalkTheSpecifiedStack ( m_cThreadCombo.GetCurSel ( ) , TRUE ) ;
}

void JAssertionDlg :: OnEmailAssert ( void )
{
    ASSERT ( NULL != g_pMAPISendMail ) ;
    ASSERT ( NULL != m_szEmail ) ;
    
    // The message and recipient structures.
    MapiMessage stMessage ;
    MapiRecipDesc stRecipDesc ;
    // The ANSI buffer used to build the recipient address.
    char * szEmailAddr = NULL ;
    // The ANSI buffer used to hold the message text.
    //char * szMsgText = NULL ;

    // Zero out the structures.
    ZeroMemory ( &stMessage , sizeof ( MapiMessage ) ) ;
    ZeroMemory ( &stRecipDesc , sizeof ( MapiRecipDesc ) ) ;
    
    
    // Build up the recipient info.
    size_t iAddrLen = strlen ( m_szEmail ) + 1 + strlen ( k_SMTPPREFIX);
    szEmailAddr = (char*)HeapAlloc ( GetProcessHeap ( )       ,
                                     HEAP_GENERATE_EXCEPTIONS ,
                                     iAddrLen                  ) ;
    strcpy ( szEmailAddr , k_SMTPPREFIX ) ;
    strcat ( szEmailAddr , m_szEmail ) ;
    
    stRecipDesc.lpszAddress  = szEmailAddr ;
    stRecipDesc.lpszName     = (LPSTR)m_szEmail ;
    stRecipDesc.ulRecipClass = MAPI_TO ;
    
    stMessage.lpRecips = &stRecipDesc ;
    stMessage.nRecipCount = 1 ;
    stMessage.lpszSubject = k_EMAILSUBJECT ;
    
    // Get the UNICODE data.
    size_t iCharLen = 0 ;
    LPCTSTR szUniData = AllocAndCopyAssertionData ( &iCharLen ) ;
    
    // Allocate the ANSI data.
    char * szAnsiData = (char *)HeapAlloc ( GetProcessHeap ( )       ,
                                            HEAP_GENERATE_EXCEPTIONS ,
                                            iCharLen + 1              );
    // Convert UNICODE to ANSI.
    BSUWide2Ansi ( szUniData , szAnsiData , (int)iCharLen ) ;
    
    // Get rid of the UNICODE data.
    FreeAssertionData ( szUniData ) ;
    szUniData = NULL ;
    
    // Set the message body.
    stMessage.lpszNoteText = szAnsiData ;
    
    ULONG ulRet = g_pMAPISendMail ( 0                           ,
                                    (ULONG_PTR)m_hWnd           ,
                                    &stMessage                  ,
                                    MAPI_LOGON_UI | MAPI_DIALOG ,
                                    0                            ) ;
    if ( ( SUCCESS_SUCCESS   != ulRet ) &&
         ( MAPI_E_USER_ABORT != ulRet )    )
    {
        TCHAR szBuff[ MAX_PATH ] ;
        wsprintf ( szBuff , k_MAPIFAILEDFMT , ulRet ) ;
        JfxMessageBox ( m_hWnd , szBuff ) ;
    }

    // Free the email address.
    HeapFree ( GetProcessHeap ( ) , 0 , szEmailAddr ) ;
    // Get rid of the message body.
    HeapFree ( GetProcessHeap ( ) , 0 , szAnsiData ) ;
}

BOOL JAssertionDlg :: IsMAPIAvailable ( void )
{
    BOOL bRet = FALSE ;

    // If this has already been called, no need to wind through again.
    if ( NULL != g_pMAPISendMail )
    {
        return ( TRUE ) ;
    }

    // Look in WIN.INI (a blast from the past) to see if MAPI is
    // registered.
    if ( 0 != ::GetProfileInt ( _T ( "MAIL" ) , _T ( "MAPI" ) , 0 ) )
    {
        // Find MAPI32.DLL.
        if ( SearchPath ( NULL      ,
                          k_MAPIDLL ,
                          NULL      ,
                          0         ,
                          NULL      ,
                          NULL       ) > 0 )
        {
            HINSTANCE hInstMAPI = LoadLibrary ( k_MAPIDLL ) ;
            if ( NULL != hInstMAPI )
            {
                g_pMAPISendMail = (PMAPISENDMAIL)
                                   GetProcAddress ( hInstMAPI      ,
                                                    "MAPISendMail"  ) ;
                if ( NULL != g_pMAPISendMail )
                {
                    bRet = TRUE ;
                }
            }
        }
    }
    return ( bRet ) ;
}