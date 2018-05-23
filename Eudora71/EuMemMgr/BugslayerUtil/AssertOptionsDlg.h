/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#ifndef _ASSERTOPTIONSDLG_H
#define _ASSERTOPTIONSDLG_H

#include "JFX.h"
#include "Resource.h"
#include "Internal.h"

const int k_WALKALLTHREADS          = 1 ;
const int k_WALKONLYASSERTIONTHREAD = 2 ;
const int k_WALKMANUALLY            = 3 ;

class JAssertionOptionsDlg : public JModalDlg
{
/*//////////////////////////////////////////////////////////////////////
                      Constructors and Destructors
//////////////////////////////////////////////////////////////////////*/
public      :
    JAssertionOptionsDlg ( int  & iStackWalkCode      ,
                           int  & iDumpInfo           ,
                           BOOL & bPlaySounds         ,
                           BOOL & bAlwaysOnTop        ,
                           JWnd * pParent               )
                    : JModalDlg ( IDD_ASSERTOPTIONS        ,
                                  pParent                  ,
                                  GetBSUInstanceHandle ( )  )

    {
        m_piStackWalkCode = &iStackWalkCode ;
        m_piDumpInfo      = &iDumpInfo      ;
        m_pbPlaySounds    = &bPlaySounds    ;
        m_pAlwaysOnTop    = &bAlwaysOnTop   ;
    }

    virtual ~JAssertionOptionsDlg ( void )
    {
    }

    BEGIN_MSG_DECLARES ( )
        DECL_WM_INITDIALOG      ( )
    END_MSG_DECLARES ( )

/*//////////////////////////////////////////////////////////////////////
                             Public Methods
//////////////////////////////////////////////////////////////////////*/
public      :
    virtual void OnOk ( void ) ;

/*//////////////////////////////////////////////////////////////////////
                          Private Data Members
//////////////////////////////////////////////////////////////////////*/
private     :
    int *  m_piStackWalkCode ;
    int * m_piDumpInfo ;
    BOOL * m_pbPlaySounds ;
    BOOL * m_pAlwaysOnTop ;

    JButton m_cWalkAlways ;
    JButton m_cOnlyAssertions ;
    JButton m_cWalkManually ;
    JButton m_cSounds ;
    JButton m_cForce ;

    JButton m_cDataDump ;
    JButton m_cFullMemDump ;
    JButton m_cHandleDump ;

} ;

#endif  // _ASSERTOPTIONSDLG_H


