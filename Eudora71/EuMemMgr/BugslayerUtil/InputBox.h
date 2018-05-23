/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#ifndef _INPUTBOX_H
#define _INPUTBOX_H

#include "JFX.h"
#include "Resource.h"
#include "Internal.h"

class JInputBox : public JModalDlg
{
/*//////////////////////////////////////////////////////////////////////
                      Constructors and Destructors
//////////////////////////////////////////////////////////////////////*/
public      :
    JInputBox ( LPCTSTR szCaption   ,
                LPCTSTR szPrompt    ,
                LPTSTR  szInputText ,
                int     iInputLen   ,
                JWnd * pParent       )
                    : JModalDlg ( IDD_INPUTBOX             ,
                                  pParent                  ,
                                  GetBSUInstanceHandle ( )  )

    {
        m_szCaption     = szCaption   ;
        m_szPrompt      = szPrompt    ;
        m_szInputText   = szInputText ;
        m_iInputLen     = iInputLen   ;
    }

    virtual ~JInputBox ( void )
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
    // The label control.
    JWnd m_cPromptLabel ;
    // The edit box.
    JWnd m_cEditBox ;

    // The dialog caption.
    LPCTSTR m_szCaption ;
    // The prompt label text.
    LPCTSTR m_szPrompt ;
    // The text the user entered.
    LPTSTR m_szInputText ;
    // The max length of the input text.
    int    m_iInputLen ;
} ;


#endif // _INPUTBOX_H


