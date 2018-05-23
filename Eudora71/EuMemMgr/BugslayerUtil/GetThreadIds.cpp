/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#include "pch.h"
#include "BugslayerUtil.h"

// The project internal header file.
#include "Internal.h"

BOOL BUGSUTIL_DLLINTERFACE __stdcall
                           GetProcessThreadIds ( DWORD   dwPID         ,
                                                 UINT    uiCount       ,
                                                 LPDWORD paThreadArray ,
                                                 LPDWORD pdwRealCount  )
{
    // Do the debug checking.
    ASSERT ( NULL != pdwRealCount ) ;
    ASSERT ( FALSE == IsBadWritePtr ( pdwRealCount , sizeof ( DWORD ) ));
#ifdef _DEBUG
    if ( 0 != uiCount )
    {
        ASSERT ( NULL != paThreadArray ) ;
        ASSERT ( FALSE == IsBadWritePtr ( paThreadArray          ,
                                          uiCount *
                                                sizeof ( DWORD )   ));
    }
#endif

    // Do the parameter checking for real.  Note that I only check the
    // memory in paModArray if uiCount is > 0.  The user can pass zero
    // in uiCount if they are just interested in the total to be
    // returned so they could dynamically allocate a buffer.
    if ( ( TRUE == IsBadWritePtr ( pdwRealCount , sizeof(DWORD) ) )  ||
         ( ( uiCount > 0 ) &&
           ( TRUE == IsBadWritePtr ( paThreadArray ,
                                     uiCount * sizeof(DWORD) ) ) )     )
    {
        SetLastErrorEx ( ERROR_INVALID_PARAMETER , SLE_ERROR ) ;
        return ( FALSE ) ;
    }

    // Check the version and call the appropriate thing.
    if ( TRUE == IsNT4 ( ) )
    {
        // I don't have the NT4 version written!!
        //ASSERT ( !"Yikes!  I've forgotten to write NT4GetThreadIds!" ) ;
        return ( FALSE ) ;
    }
    else
    {
        // Win9x and Win2K go through tool help.
        return ( TLHELPGetProcessThreadIds ( dwPID         ,
                                             uiCount       ,
                                             paThreadArray ,
                                             pdwRealCount   ) ) ;
    }
}

