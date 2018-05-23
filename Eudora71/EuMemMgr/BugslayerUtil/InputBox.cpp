/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "PCH.h"
#include "BugslayerUtil.h"
#include "Resource.h"
#include "InputBox.h"

BEGIN_MSG_MAP ( JInputBox , JModalDlg )
    WM_INITDIALOG_HANDLER       (   )
END_MSG_MAP   ( JInputBox , JModalDlg )


void JInputBox :: OnOk ( void )
{
    m_cEditBox.GetWindowText ( m_szInputText , m_iInputLen ) ;

    JModalDlg::OnOk ( ) ;
}

BOOL JInputBox :: OnInitDialog ( HWND /*hWndCtrl*/ , LPARAM /*lParam*/ )
{
    // Get the controls.
    VERIFY ( TRUE == m_cPromptLabel.Attach ( *this , IDC_PROMPTTEXT ) );
    VERIFY ( TRUE == m_cEditBox.Attach ( *this , IDC_EDITBOX ) ) ;

    if ( NULL != m_szPrompt )
    {
        m_cPromptLabel.SetWindowText ( m_szPrompt ) ;
    }
    if ( ( NULL != m_szInputText ) && ( _T ( '\0' ) !=m_szInputText[0]))
    {
        m_cEditBox.SetWindowText ( m_szInputText ) ;
    }

    m_cEditBox.SendMessage ( EM_LIMITTEXT , m_iInputLen - 1 ) ;

    if ( NULL != m_szCaption )
    {
        SetWindowText ( m_szCaption ) ;
    }

    CenterWindow ( GetParent ( ) ) ;

    return ( TRUE ) ;
}

