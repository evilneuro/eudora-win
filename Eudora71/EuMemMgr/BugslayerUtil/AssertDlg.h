/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#ifndef _ASSERTDLG_H
#define _ASSERTDLG_H

#include "JFX.h"
#include "Resource.h"
#include "Internal.h"

// The structure used to hold information about threads.
typedef struct tag_THREADINFO
{
    // The thread ID.
    DWORD dwTID ;
    // The thread handle.
    HANDLE hThread ;
    // Any associated stack walk text.
    LPTSTR szStackWalk ;
} THREADINFO , * LPTHREADINFO ;

class JAssertionDlg : public JModalDlg
{
/*//////////////////////////////////////////////////////////////////////
                      Constructors and Destructors
//////////////////////////////////////////////////////////////////////*/
public      :
    JAssertionDlg ( LPCTSTR      szExpression  ,
                    LPCSTR       szEmail       ,
                    DWORD64      dwStack       ,
                    DWORD64      dwStackFrame  ,
                    DWORD64      dwIP          ,
                    int *        piIgnoreCount ,
                    LPTHREADINFO pThreadInfo   ,
                    DWORD        dwThreadCount  ) ;

    virtual ~JAssertionDlg ( void ) ;

/*//////////////////////////////////////////////////////////////////////
                             Public Methods
                        Dialog Folding Functions
//////////////////////////////////////////////////////////////////////*/
public      :
    void Fold ( BOOL bFold = TRUE ) ;
    void Unfold ( void )
    {
        Fold ( FALSE ) ;
    }
    void ToggleFold( void )
    {
        Fold ( !m_bIsFolded ) ;
    }

/*//////////////////////////////////////////////////////////////////////
                           Protected Methods
                            Helper Functions
//////////////////////////////////////////////////////////////////////*/
protected   :

    void SaveRestoreFoldedWindowStates ( BOOL bEnable ) ;
    BOOL GetClientSize ( SIZE & size ) ;

    BOOL GetItemWindowRectLocal ( UINT id, RECT & rect ) ;

    void InvalidateGrip( void ) ;

    void PersistState ( void ) ;

    // Walks the specific stack.
    void WalkTheSpecifiedStack ( int iIndex , BOOL bReWalk = FALSE ) ;
    // Pumps locals into the buffer.
    static BOOL CALLBACK
            EnumerateLocalsIntoBuffer ( DWORD64  dwAddr        ,
                                        LPCTSTR  szType        ,
                                        LPCTSTR  szName        ,
                                        LPCTSTR  szValue       ,
                                        int      iIndentLevel  ,
                                        PVOID    pUserCtx       ) ;

    // Helper function that allocate a memory buffer big enough to
    // hold all the text from the assertion and any stack walks and
    // copy the text data into that buffer.
    LPCTSTR AllocAndCopyAssertionData ( size_t * pChrCount = NULL ) ;
    // Gotta call this to free it!
    void FreeAssertionData ( LPCTSTR szData  ) ;
    
    // Returns TRUE if MAPI support exists.
    BOOL IsMAPIAvailable ( void ) ;
                                     

/*//////////////////////////////////////////////////////////////////////
                           Protected Methods
                        Windows Message Handling
//////////////////////////////////////////////////////////////////////*/
protected   :

    BEGIN_MSG_DECLARES ( )
        DECL_WM_INITDIALOG      ( )
        DECL_WM_GETMINMAXINFO   ( )
        DECL_WM_DESTROY         ( )
        DECL_WM_SIZE            ( )
        DECL_MESSAGE_HANDLER    ( WM_SETCURSOR , OnSetCursor )
        DECL_WM_PAINT           ( )
        DECL_WM_SYSCOMMAND      ( )

        DECL_COMMAND_ID_HANDLER ( IDC_LESSMORE     , OnLessMore     )
        DECL_COMMAND_ID_HANDLER ( IDC_DOSTACKTRACE , OnDoStackTrace )
        DECL_COMMAND_ID_HANDLER ( IDC_IGNOREONCE   , OnIgnoreOnce   )
        DECL_COMMAND_ID_HANDLER ( IDC_IGNOREALWAYS , OnIgnoreAlways )
        DECL_COMMAND_ID_HANDLER ( IDC_ABORTPROGRAM , OnAbortProgram )
        DECL_COMMAND_ID_HANDLER ( IDC_BREAKINTODEBUGGER ,
                                                    OnBreakIntoDebugger)
        DECL_COMMAND_ID_HANDLER ( IDC_IGNORE_THIS  , OnIgnoreThis   )
        DECL_COMMAND_ID_HANDLER ( IDC_IGNORE_ALL   , OnIgnoreAll    )
        DECL_COMMAND_ID_HANDLER ( IDC_COPYTOCLIPBOARD ,
                                                      OnCopyToClipboard)
        DECL_COMMAND_ID_HANDLER ( IDC_CREATEMINIDUMP , OnCreateMiniDump)
        DECL_COMMAND_ID_HANDLER ( IDC_THREADCOMBO  ,
                                                OnThreadSelectionChange)
        DECL_COMMAND_ID_HANDLER ( IDC_DEPTHCOMBO , OnLocalsDepthChange )
        DECL_COMMAND_ID_HANDLER ( IDC_SHOWARRAYSCHECK , OnShowArrays )
        DECL_COMMAND_ID_HANDLER ( IDC_EMAILASSERT , OnEmailAssert ) ;

    END_MSG_DECLARES ( )

    virtual void OnCancel ( void ) ;

/*//////////////////////////////////////////////////////////////////////
                          Private Data Members
//////////////////////////////////////////////////////////////////////*/
private     :
    // The ignore assert count edit control.
    JWnd m_cAssertEdit ;
    // The stack walk edit control.
    JWnd m_cStackTraceEdit ;

    // The Do Stack Walk Button.
    JButton m_cDoStackButton ;

    // The Mini Dump Button.
    JButton m_cMiniDumpButton ;
    
    // The Email Button.
    JButton m_cEmailButton ;

    // The font for the two edit controls.
    JFont m_cFont ;

    // The window system menu.
    JMenu m_cSysMenu ;

    // The thread ID combo box.  This is only visible on Windows 2000/XP
    // when there are multiple threads.
    JComboBox m_cThreadCombo ;
    // The thread comb static.
    JWnd m_cThreadStatic ;
    
    // The locals depth combo box.
    JComboBox m_cLocalsDepthCombo ;
    // The show arrays check box.
    JButton   m_cShowArraysCheck ;

    // The original size of the rect.
    RECT m_OriginalRect ;
    // The folded height.
    int m_FoldedHeight ;
    // The height, unfolded.
    int m_UnfoldedHeight ;
    // The right border width.
    int m_RightBorder ;
    // Bottom border width.
    int m_BottomBorder ;

    // The number of windows from the fold divider on.
    int m_iWinInFold ;
    // The array of enable/disable states for the windows in the fold
    // area.
    LPBYTE m_pFoldStates ;

    // The expression to show for the failure.
    LPCTSTR m_szExpression  ;
    // The stack and frame at the assertion location.
    DWORD64   m_dwStack      ;
    DWORD64   m_dwStackFrame ;
    DWORD64   m_dwIP         ;

    // The thread information for the process.
    LPTHREADINFO m_pThreadInfo ;
    // The number of threads in the array.
    DWORD m_dwThreadCount ;

    // The ignore count for this assertion.
    int *   m_piIgnoreCount ;

/*//////////////////////////////////////////////////////////////////////
                          Private Data Members
//////////////////////////////////////////////////////////////////////*/
    // The last position on the screen.
    RECT m_LastRect ;
    // Is the dialog folded?
    BOOL m_bIsFolded ;
    // What type of stack walk do I do (see consts in the options
    // dialog header.)
    int m_iStackWalkType ;
    // The mini dump data to include.
    int m_iMiniDumpInfo ;
    // Am I supposed to play annoying sounds?
    BOOL m_bDoSounds ;
    // Is the dialog always supposed to force it's way to the top?
    BOOL m_bAlwaysOnTop ;
    
    // True if I'm supposed to show arrays expanded.
    BOOL m_bShowArrays ;
    // The default depth to show locals.
    int m_iLocalsDepth ;
    
    // The email address passed in.
    LPCSTR m_szEmail ;
} ;

#endif  // _ASSERTDLG_H


