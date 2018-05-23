/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "SymbolEngine.h"
#include "SymEngInternal.h"


BOOL CSymbolEngine :: DecodeValue ( CVariableInfo & cVar )
{
    BOOL bRet = TRUE ;
    
    switch ( cVar.eTag )
    {
        case SymTagBaseType :
            bRet = DecodeValueSymTagBaseType ( cVar ) ;
            break ;
        case SymTagPointerType :
            bRet = DecodeValueSymTagPointerType ( cVar ) ;
            break ;
        case SymTagEnum :
            bRet = DecodeValueSymTagEnum ( cVar ) ;
            break ;
        case SymTagArrayType :
            bRet = DecodeValueSymTagArrayType ( cVar ) ;
            break ;
        default :
            bRet = TRUE ;
            break ;
    }
    return ( bRet ) ;
}

BOOL CSymbolEngine :: DecodeValueSymTagArrayType(CVariableInfo & cVar)
{
    // I only want special processing here if it's "char [*]" or
    // "wchar_t [*]" where * is nothing but numbers.
    
    BOOL bRet = TRUE ;
    
    // Let me grab the type, tag, and length for the elements in this
    // array.
    ULONG ulElemType = 0 ;
    enum SymTagEnum eElemTag = SymTagNull ;
    ULONG64 ulElemLen = 0 ;
    
    if ( ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                    cVar.ulTypeIndex     ,
                                    TI_GET_TYPEID        ,
                                    &ulElemType           ) ) &&
         ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                    ulElemType           ,
                                    TI_GET_SYMTAG        ,
                                    &eElemTag             ) ) &&
         ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                    ulElemType           ,
                                    TI_GET_LENGTH        ,
                                    &ulElemLen            ) )   )
    {
        // Grab the basic type.
        BasicType eBT ;
        if ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                      ulElemType           ,
                                      TI_GET_BASETYPE      ,
                                      &eBT                  ) )
        {
            // This is a basic type so check to see if it's a valid
            // string variable.
            int iCharSize = 0 ;
            
            if ( btChar == eBT )
            {
                iCharSize = sizeof ( char ) ;
            }
            else if ( ( btWChar == eBT                          ) ||
                    ( ( btUInt == eBT ) && ( 2 == ulElemLen ) )   )
            {
                iCharSize = sizeof ( wchar_t ) ;
            }
            if ( iCharSize > 0 )
            {
                // We got us a string.

                LPVOID lpCurrAddr = (LPVOID)cVar.dwAddress ;
                DWORD dwBytesRead = 0 ;
                BOOL bMemRead = FALSE ;
                TCHAR szBuff[ MAX_PATH ] ;
                
                // As I could easily read off the end of a page here,
                // which would fail the whole read, I'll bounce through
                // until I get a good read or drop down to zero bytes.
                int iReadLen = MAX_PATH * iCharSize ;
                do
                {
                    bMemRead = MemRead ( lpCurrAddr           ,
                                        (PBYTE)szBuff        ,
                                        iReadLen             ,
                                        &dwBytesRead          ) ;
                    iReadLen = iReadLen / 2 ;
                    if ( 0 == iReadLen )
                    {
                        break ;
                    }
                }
                while ( FALSE == bMemRead ) ;
                
                if ( TRUE == bMemRead )
                {
                    cVar.bIsStringArray = TRUE ;
                    
                    szBuff[ MAX_PATH - 1 ] = _T ( '\0' ) ;
                    
                    _tcscat ( cVar.szValue , _T ( " \"" ) ) ;
                    size_t iLen = _tcslen ( cVar.szValue ) ;
                    
                    if ( sizeof ( char ) == iCharSize )
                    {
                        MultiByteToWideChar ( CP_THREAD_ACP       ,
                                              0                   ,
                                              (char*)szBuff       ,
                                              -1                  ,
                                              cVar.szValue + iLen ,
                                              MAX_PATH - 3         ) ;
                    }
                    else
                    {
                        _tcscat ( cVar.szValue , szBuff ) ;
                    }
                    _tcscat ( cVar.szValue , _T ( "\"" ) ) ;
                }
            }
        }
    }
    else
    {
        ASSERT ( !"Unable to get elem type and tag!" ) ;
        bRet = FALSE ;
    }
    return ( bRet ) ;
}


BOOL CSymbolEngine :: DecodeValueSymTagEnum ( CVariableInfo & cVar )
{
    // Get the length for the enum value.
    ULONG64 ulPtrSize = 0 ;
    if ( FALSE == SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                   cVar.ulTypeIndex     ,
                                   TI_GET_LENGTH        ,
                                   &ulPtrSize            ) )
    {
        _tcscpy ( cVar.szValue , _T ( "????" ) ) ;
    }
    else
    {
        // Armed with the size, I can read the memory pointed to.
        DWORD dwBytesRead = 0 ;
        DWORD dwData[ 2 ] ;
        PBYTE pDataBuff = (PBYTE)&dwData ;

        LPCVOID lpCurrAddr = (LPCVOID)cVar.dwAddress ;
        int iDataSize = (int)ulPtrSize ;
        
        TCHAR szBuff [ 50 ] ;

        BOOL bMemRead = MemRead ( lpCurrAddr    ,
                                  pDataBuff     ,
                                  iDataSize     ,
                                  &dwBytesRead   ) ;
        if ( TRUE == bMemRead )
        {
            // Now grind through the enum children to get the value
            // name.
            DWORD dwChildCount = 0 ;
            if ( ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                            cVar.ulTypeIndex     ,
                                            TI_GET_CHILDRENCOUNT ,
                                            &dwChildCount        ))&&
                    ( dwChildCount > 0 )                              )
            {
                BOOL bRet = FALSE ;
                
                FINDCHILDREN stFC ;
                stFC.Count = dwChildCount ;
                stFC.Start = 0 ;
            
                bRet = SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                        cVar.ulTypeIndex     ,
                                        TI_FINDCHILDREN      ,
                                        &stFC                 ) ;

                ASSERT ( TRUE == bRet ) ;
                if ( TRUE == bRet )
                {
                    for ( DWORD i = 0 ; i < dwChildCount ; i++ )
                    {
                        VARIANT vT ;
                        
                        bRet = SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                                stFC.ChildId[ i ]    ,
                                                TI_GET_VALUE         ,
                                                &vT                   );
                        ASSERT ( TRUE == bRet ) ;
                        if ( TRUE == bRet )
                        {
                            if ( vT.iVal == (int)*(int*)pDataBuff )
                            {
                                WCHAR * szwTypeName = NULL ;
                            
                                if ( TRUE == SymGetTypeInfo
                                            ( m_dwCurrLocalModBase  ,
                                              stFC.ChildId[ i ]     ,
                                              TI_GET_SYMNAME        ,
                                              &szwTypeName           ) )
                                {
                                    if ( m_iRadix == 16 )
                                    {
                                        wsprintf ( szBuff          ,
                                                   _T ( "0x%X (" ) ,
                                                   *pDataBuff       ) ;
                                    }
                                    else
                                    {
                                        wsprintf ( szBuff        ,
                                                   _T ( "%d (" ) ,
                                                   *pDataBuff     ) ;
                                    }
                                    _tcscpy ( cVar.szValue ,
                                              szBuff        ) ;
                                    _tcscat ( cVar.szValue ,
                                              szwTypeName   ) ;
                                    _tcscat ( cVar.szValue ,
                                              _T ( ")" )    ) ;
                                    
                                    LocalFree ( szwTypeName ) ;
                                    
                                    return ( TRUE ) ;
                                }
                            }
                        }
                    }
                }
            }
            // Ain't in the enum.
            if ( m_iRadix == 16 )
            {
                wsprintf ( szBuff         ,
                            _T ( "0x%X" ) ,
                            *pDataBuff     ) ;
            }
            else
            {
                wsprintf ( szBuff       ,
                            _T ( "%d" ) ,
                            *pDataBuff   ) ;
            }
            _tcscpy ( cVar.szValue , szBuff ) ;
        }
    }
    return ( TRUE ) ;
}

BOOL CSymbolEngine :: DecodeValueSymTagPointerType(CVariableInfo & cVar)
{
    // Get the size of the pointer.
    ULONG64 ulPtrSize = 0 ;
    if ( FALSE == SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                   cVar.ulTypeIndex     ,
                                   TI_GET_LENGTH        ,
                                   &ulPtrSize            ) )
    {
        _tcscpy ( cVar.szValue , _T ( "????" ) ) ;
    }
    else
    {
        // Armed with the size, I can read the memory pointed to.
        DWORD dwBytesRead = 0 ;
        DWORD dwData[ 2 ] ;
        PBYTE pDataBuff = (PBYTE)&dwData ;

        LPCVOID lpCurrAddr = (LPCVOID)cVar.dwAddress ;
        int iDataSize = (int)ulPtrSize ;

        BOOL bMemRead = MemRead ( lpCurrAddr    ,
                                  pDataBuff     ,
                                  iDataSize     ,
                                  &dwBytesRead   ) ;
        if ( TRUE == bMemRead )
        {
            if ( 8 == ulPtrSize )
            {
                _stprintf ( cVar.szValue             ,
                            _T ( "0x016I64X" )       ,
                            (_int64)*(_int64*)dwData  ) ;
            }
            else
            {
                _stprintf ( cVar.szValue    ,
                            _T ( "0x%08X" ) ,
                            *dwData          ) ;
            }
            
            // Check to see if this is a "char *" or "wchar_t *"
            TCHAR szBuff[ MAX_PATH ] ;
            int iCharSize = 0 ;
            
            if ( cVar.cTypeName == _T ( "char *" ) )
            {
                iCharSize = sizeof ( char ) ;
            }
            else if ( ( cVar.cTypeName == _T ( "wchar_t *" )       ) ||
                      ( cVar.cTypeName == _T ( "unsigned short *" ) ) )
            {
                iCharSize = sizeof ( wchar_t ) ;
            }
            if ( iCharSize > 0 )
            {
                lpCurrAddr = (LPCVOID)*(LPCVOID*)(dwData) ;

                // As I could easily read off the end of a page here,
                // which would fail the whole read, I'll bounce through
                // until I get a good read or drop down to zero bytes.
                int iReadLen = MAX_PATH * iCharSize ;
                do
                {
                    bMemRead = MemRead ( lpCurrAddr           ,
                                         (PBYTE)szBuff        ,
                                         iReadLen             ,
                                         &dwBytesRead          ) ;
                    iReadLen = iReadLen / 2 ;
                    if ( 0 == iReadLen )
                    {
                        break ;
                    }
                }
                while ( FALSE == bMemRead ) ;
                
                
                if ( TRUE == bMemRead )
                {
                    cVar.bIsStringArray = TRUE ;
                    
                    szBuff[ MAX_PATH - 1 ] = _T ( '\0' ) ;

                    _tcscat ( cVar.szValue , _T ( " \"" ) ) ;
                    size_t iLen = _tcslen ( cVar.szValue ) ;
                    
                    if ( sizeof ( char ) == iCharSize )
                    {
                        MultiByteToWideChar ( CP_THREAD_ACP       ,
                                              0                   ,
                                              (char*)szBuff       ,
                                              -1                  ,
                                              cVar.szValue + iLen ,
                                              MAX_PATH - 3         ) ;

                    }
                    else
                    {
                        _tcscat ( cVar.szValue , szBuff ) ;
                    }
                    _tcscat ( cVar.szValue , _T ( "\"" ) ) ;
                }
            }
        }
        else
        {
            _tcscpy ( cVar.szValue , _T ( "????" ) ) ;
        }
    }
    
    return ( TRUE ) ;
}

BOOL CSymbolEngine :: DecodeValueSymTagBaseType ( CVariableInfo & cVar )
{
    // I should have already gotten the basic type when doing the type
    // decoding.
    ASSERT ( btNoType != cVar.eBT ) ;
    if ( btNoType == cVar.eBT )
    {
        return ( FALSE ) ;
    }
    
    BOOL bMemRead = FALSE ;
    DWORD dwBytesRead = 0 ;
    DWORD dwData[ 8 ] ;
    LPDWORD pDataBuff = (LPDWORD)&dwData ;
    
    LPCVOID lpCurrAddr = (LPCVOID)cVar.dwAddress ;
    int iDataSize = (int)cVar.ulBTLen ;

    bMemRead = MemRead ( lpCurrAddr       ,
                         (PBYTE)pDataBuff ,
                         iDataSize        ,
                         &dwBytesRead      ) ;
    if ( FALSE == bMemRead )
    {
        _tcscpy ( cVar.szValue , _T ( "????" ) ) ;
        return ( TRUE ) ;
    }
                                

    // Handle the special values.
    if ( btBool == cVar.eBT )
    {
        bool bVal = (bool)*(bool*)pDataBuff ;
        if ( true == bVal )
        {
            _tcscpy ( cVar.szValue , _T ( "true" ) ) ;
        }
        else
        {
            _tcscpy ( cVar.szValue , _T ( "false" ) ) ;
        }
    }
    else if ( btHresult == cVar.eBT )
    {
        HRESULT hr = (HRESULT)*(HRESULT*)pDataBuff ;
        TCHAR szBuff[ 20 ] ;

        LPTSTR szFmtMsg ;
        if ( FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER    |
                               FORMAT_MESSAGE_IGNORE_INSERTS   |
                               FORMAT_MESSAGE_FROM_SYSTEM        ,
                             NULL                                ,
                             hr                                  ,
                             0                                   ,
                             (LPTSTR)&szFmtMsg                   ,
                             0                                   ,
                             NULL                                 ) )
        {
            // Sometimes FormatMessage returns a CR/LF on the message.
            TCHAR * pCurr = szFmtMsg ;
            while ( NULL != *pCurr )
            {
                if ( ( _T ( '\n' ) == *pCurr ) ||
                    ( _T ( '\r' ) == *pCurr )    )
                {
                    *pCurr = _T( '\0' ) ;
                }
                pCurr++ ;
            }

            if ( 4 == iDataSize )
            {
                _stprintf ( szBuff , _T ( "0x%08X (" ) , hr ) ;
            }
            else
            {
                _stprintf ( szBuff , _T ( "0x%016X (" ) , hr ) ;
            }

            _tcscpy ( cVar.szValue , szBuff ) ;
            _tcscat ( cVar.szValue , szFmtMsg ) ;
            _tcscat ( cVar.szValue , _T ( ")" ) ) ;

            LocalFree ( szFmtMsg ) ;
        }
        else
        {
            if ( 4 == iDataSize )
            {
                _stprintf ( szBuff , _T ( "0x%08X" ) , hr ) ;
            }
            else
            {
                _stprintf ( szBuff , _T ( "0x%016X" ) , hr ) ;
            }
            _tcscat ( cVar.szValue , szBuff ) ;
        }
    }
    else if ( btVariant == cVar.eBT )
    {
        ASSERT ( FALSE ) ;
    }
    else if ( btBSTR == cVar.eBT )
    {
        ASSERT ( FALSE ) ;
    }
    else if ( btFloat == cVar.eBT )
    {
        if ( 8 == cVar.ulBTLen )
        {
            _stprintf ( cVar.szValue             ,
                        _T ( "%E" )              ,
                        (double)*(double*)dwData  ) ;
        }
        else
        {
            _stprintf ( cVar.szValue           ,
                        _T ( "%f" )            ,
                        (float)*(float*)dwData  ) ;
        }
    }
    else
    {
        TCHAR * szFmt = NULL ;
        
        switch ( cVar.eBT )
        {
            case btVoid :
                szFmt = _T ( "0x%p" ) ;
                break ;
                
            case btChar :
#ifdef UNICODE
                szFmt = _T ( "'%C'" ) ;
#else
                szFmt = _T ( "'%c'" ) ;
#endif      // UNICODE ;
                // I need to special case NULLs so this doesn't end the
                // string prematurley.
                if ( 0 == (char)*(char*)dwData )
                {
                    szFmt = _T ( "'\\0'" ) ;
                }
                break ;
            case btWChar :
#ifdef UNICODE
                szFmt = _T ( "'%c'" ) ;
#else
                szFmt = _T ( "'%C'" ) ;
#endif      // UNICODE ;
                // I need to special case NULLs so this doesn't end the
                // string prematurley.
                if ( 0 == (wchar_t)*(wchar_t*)dwData )
                {
                    szFmt = _T ( "'\\0'" ) ;
                }
                break ;
            case btInt :
                {
                    if ( 8 == cVar.ulBTLen )
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%I64X" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%I64d" ) ;
                        }
                        _stprintf ( cVar.szValue             ,
                                    szFmt                    ,
                                    (_int64)*(_int64*)dwData  ) ;
                        return ( TRUE ) ;
                    }
                    else if ( 2 == cVar.ulBTLen )
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%hX" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%hd" ) ;
                        }
                    }
                    else
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%X" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%d" ) ;
                        }
                    }
                }
                break ;
            case btUInt :
                {
                    if ( 8 == cVar.ulBTLen )
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%I64X" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%I64u" ) ;
                        }
                        _stprintf ( cVar.szValue             ,
                                    szFmt                    ,
                                    (_int64)*(_int64*)dwData  ) ;
                                    
                        return ( TRUE ) ;
                        
                    }
                    else if ( 2 == cVar.ulBTLen )
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%hX" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%hu" ) ;
                        }
                    }
                    else
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%X" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%u" ) ;
                        }
                    }
                }
                break ;
            case btBCD :
                ASSERT ( FALSE ) ;
                szFmt = _T ( "BCD : %X" ) ;
                break ;
            case btLong :
                {
                    if ( 8 == cVar.ulBTLen )
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%I64X" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%I64d" ) ;
                        }
                        _stprintf ( cVar.szValue             ,
                                    szFmt                    ,
                                    (_int64)*(_int64*)dwData  ) ;
                        return ( TRUE ) ;
                        
                    }
                    else if ( 2 == cVar.ulBTLen )
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%hX" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%hd" ) ;
                        }
                    }
                    else
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%X" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%d" ) ;
                        }
                    }
                }
                break ;
            case btULong :
                {
                    if ( 8 == cVar.ulBTLen )
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%I64X" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%I64u" ) ;
                        }
                        _stprintf ( cVar.szValue             ,
                                    szFmt                    ,
                                    (_int64)*(_int64*)dwData  ) ;
                        return ( TRUE ) ;
                    }
                    else if ( 2 == cVar.ulBTLen )
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%02X" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%hu" ) ;
                        }
                    }
                    else
                    {
                        if ( 16 == m_iRadix )
                        {
                            szFmt = _T ( "0x%lX" ) ;
                        }
                        else
                        {
                            szFmt = _T ( "%lu" ) ;
                        }
                    }
                }
                break ;
            case btCurrency :
                ASSERT ( FALSE ) ;
                szFmt = _T ( "CURRENCY %X" ) ;
                break ;
            case btDate :
                ASSERT ( FALSE ) ;
                szFmt = _T ( "DATE %X" ) ;
                break ;
            case btComplex :
                ASSERT ( FALSE ) ;
                szFmt = _T ( "COMPLEX %X" ) ;
                break ;
            case btBit :
                ASSERT ( FALSE ) ;
                szFmt = _T ( "bit" ) ;
                break ;
            default :
                ASSERT ( FALSE ) ;
                szFmt = _T ( "**UNKNOWN BasicType**" ) ;
                break ;
        }
        _stprintf ( cVar.szValue ,szFmt , *dwData ) ;
    }
    
    return ( TRUE ) ;
}