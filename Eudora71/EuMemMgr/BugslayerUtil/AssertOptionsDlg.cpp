/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "PCH.h"
#include "BugslayerUtil.h"
#include "Resource.h"
#include "AssertOptionsDlg.h"

BEGIN_MSG_MAP ( JAssertionOptionsDlg , JModalDlg )
    WM_INITDIALOG_HANDLER       (   )
END_MSG_MAP   ( JAssertionOptionsDlg , JModalDlg )


void JAssertionOptionsDlg :: OnOk ( void )
{
    if ( TRUE == m_cWalkAlways.IsChecked ( ) )
    {
        *m_piStackWalkCode = k_WALKALLTHREADS ;
    }
    else if ( TRUE == m_cOnlyAssertions.IsChecked ( ) )
    {
        *m_piStackWalkCode = k_WALKONLYASSERTIONTHREAD ;
    }
    else
    {
        *m_piStackWalkCode = k_WALKMANUALLY ;
    }

    int iDumpInfo = MiniDumpNormal ;

    if ( TRUE == m_cDataDump.IsChecked ( ) )
    {
        iDumpInfo |= MiniDumpWithDataSegs ;
    }
    if ( TRUE == m_cFullMemDump.IsChecked ( ) )
    {
        iDumpInfo |= MiniDumpWithFullMemory ;
    }
    if ( TRUE == m_cHandleDump.IsChecked ( ) )
    {
        iDumpInfo |= MiniDumpWithHandleData ;
    }

    *m_piDumpInfo = iDumpInfo ;


    *m_pbPlaySounds = m_cSounds.IsChecked ( ) ;
    *m_pAlwaysOnTop = m_cForce.IsChecked ( ) ;

    JModalDlg::OnOk ( ) ;
}

BOOL JAssertionOptionsDlg :: OnInitDialog ( HWND /*hWndCtrl*/ , LPARAM /*lParam*/ )
{
    // Get the buttons.
    VERIFY ( TRUE == m_cWalkAlways.Attach( *this ,
                                           IDC_WALKALLTHREADSTACK));
    VERIFY ( TRUE == m_cOnlyAssertions.Attach( *this ,
                                               IDC_WALKONLYACTIVE));
    VERIFY ( TRUE == m_cWalkManually.Attach( *this             ,
                                             IDC_WALKNOTHREADS  ) ) ;
    VERIFY ( TRUE == m_cSounds.Attach( *this , IDC_PLAYSOUNDS ) ) ;
    VERIFY ( TRUE == m_cForce.Attach ( *this , IDC_FORCEONTOP ) ) ;

    VERIFY ( TRUE == m_cDataDump.Attach( *this , IDC_DUMPDATASEGS ) ) ;
    VERIFY ( TRUE == m_cFullMemDump.Attach( *this , IDC_DUMPFULL ) ) ;
    VERIFY ( TRUE == m_cHandleDump.Attach( *this , IDC_DUMPHANDLE ) ) ;


    // Set the checks.
    if ( k_WALKALLTHREADS == *m_piStackWalkCode )
    {
        m_cWalkAlways.SetChecked ( ) ;
    }
    else if ( k_WALKONLYASSERTIONTHREAD == *m_piStackWalkCode )
    {
        m_cOnlyAssertions.SetChecked ( ) ;
    }
    else
    {
        m_cWalkManually.SetChecked ( ) ;
    }

    int iDumpInfo = *m_piDumpInfo ;

    if ( MiniDumpWithDataSegs == ( MiniDumpWithDataSegs & iDumpInfo ) )
    {
        m_cDataDump.SetChecked ( ) ;
    }
    if ( MiniDumpWithFullMemory == ( MiniDumpWithFullMemory &iDumpInfo))
    {
        m_cFullMemDump.SetChecked ( ) ;
    }
    if ( MiniDumpWithHandleData == ( MiniDumpWithHandleData &iDumpInfo))
    {
        m_cHandleDump.SetChecked ( ) ;
    }

    if ( TRUE == *m_pbPlaySounds )
    {
        m_cSounds.SetChecked ( ) ;
    }
    if ( TRUE == *m_pAlwaysOnTop )
    {
        m_cForce.SetChecked ( ) ;
    }

    CenterWindow ( GetParent ( ) ) ;
    return ( TRUE ) ;
}

