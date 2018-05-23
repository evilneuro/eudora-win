/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "SymbolEngine.h"
#include "SymEngInternal.h"

BOOL CSymbolEngine :: DecodeTypeSymTagBaseType ( CVariableInfo & cVar )
{
    // Get the base type for this puppy.
    if ( FALSE == SymGetTypeInfo ( m_dwCurrLocalModBase   ,
                                   cVar.ulDecodeTypeIndex ,
                                   TI_GET_BASETYPE        ,
                                   &cVar.eBT               ) )
    {
        ASSERT ( !"Unable to get basic type!!" ) ;
        cVar.eBT = btNoType ;
        return ( FALSE ) ;
    }
    
    // Get the length of this basic type.  Sometimes the length returned
    // is zero so I need to account for that.
    if ( FALSE == SymGetTypeInfo ( m_dwCurrLocalModBase   ,
                                   cVar.ulDecodeTypeIndex ,
                                   TI_GET_LENGTH          ,
                                   &cVar.ulBTLen           ) )
    {
        cVar.ulBTLen = 0 ;
    }
    switch ( cVar.eBT )
    {
        case btNoType :
            cVar.cTypeName = _T ( "*** NO TYPE ***" ) ;
            ASSERT ( FALSE ) ;
            break ;
        case btVoid :
            cVar.cTypeName = _T ( "void" ) ;
            break ;
        case btChar :
            cVar.cTypeName = _T ( "char" ) ;
            break ;
        case btWChar :
            cVar.cTypeName = _T ( "wchar_t" ) ;
            break ;
        case btInt :
            {
                if ( 8 == cVar.ulBTLen )
                {
                    cVar.cTypeName = _T ( "_int64" ) ;
                }
                else if ( 2 == cVar.ulBTLen )
                {
                    cVar.cTypeName = _T ( "short" ) ;
                }
                else
                {
                    cVar.cTypeName = _T ( "int" ) ;
                }
            }
            break ;
        case btUInt :
            {
                cVar.cTypeName = _T ( "unsigned " ) ;
                if ( 8 == cVar.ulBTLen )
                {
                    cVar.cTypeName += _T ( "_int64" ) ;
                }
                else if ( 2 == cVar.ulBTLen )
                {
                    cVar.cTypeName += _T ( "short" ) ;
                }
                else
                {
                    cVar.cTypeName += _T ( "int" ) ;
                }
            }
            break ;
        case btFloat :
            cVar.cTypeName = _T ( "float" ) ;
            break ;
        case btBCD :
            cVar.cTypeName = _T ( "BCD" ) ;
            break ;
        case btBool :
            cVar.cTypeName = _T ( "bool" ) ;
            break ;
        case btLong :
            cVar.cTypeName = _T ( "long" ) ;
            break ;
        case btULong :
            cVar.cTypeName = _T ( "unsigned long" ) ;
            break ;
        case btCurrency :
            cVar.cTypeName = _T ( "CURRENCY" ) ;
            break ;
        case btDate :
            cVar.cTypeName = _T ( "DATE" ) ;
            break ;
        case btVariant :
            cVar.cTypeName = _T ( "VARIANT" ) ;
            break ;
        case btComplex :
            cVar.cTypeName = _T ( "COMPLEX" ) ;
            break ;
        case btBit :
            cVar.cTypeName = _T ( "bit" ) ;
            break ;
        case btBSTR :
            cVar.cTypeName = _T ( "BSTR" ) ;
            break ;
        case btHresult :
            cVar.cTypeName = _T ( "HRESULT" ) ;
            break ;
        default :
            cVar.cTypeName = _T ( "**UNKNOWN BasicType**" ) ;
            ASSERT ( FALSE ) ;
            break ;
    }
    return ( TRUE ) ;
}

BOOL CSymbolEngine :: DecodeTypeSymTagPointerType(CVariableInfo & cVar)
{
    BOOL bRet = TRUE ;
    // Get the next type down the chain and decode that one.
    if ( ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    cVar.ulDecodeTypeIndex  ,
                                    TI_GET_TYPEID           ,
                                    &cVar.ulDecodeTypeIndex  ) ) &&
         ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    cVar.ulDecodeTypeIndex  ,
                                    TI_GET_SYMTAG           ,
                                    &cVar.eDecodeTypeTag     ) )   )
    {
        // Decode it.
        bRet = DecodeType ( cVar ) ;
        cVar.cTypeName += _T ( " *" ) ;
    }
    else
    {
        ASSERT ( !"DecodeTypeSymTagPointerType failed!" ) ;
        bRet = FALSE ;
    }
    return ( bRet ) ;
}

BOOL CSymbolEngine :: DecodeTypeSymTagUDT ( CVariableInfo & cVar )
{
    WCHAR * szwTypeName = NULL ;
    BOOL bRet = FALSE ;
    
    bRet = SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                            cVar.ulDecodeTypeIndex  ,
                            TI_GET_SYMNAME          ,
                            &szwTypeName             ) ;
    ASSERT ( TRUE == bRet ) ;
    if ( TRUE == bRet )
    {
        cVar.cTypeName = szwTypeName ;
        LocalFree ( szwTypeName ) ;
        bRet = TRUE ;
    }
    return ( bRet ) ;
}

BOOL CSymbolEngine :: DecodeTypeTagArrayType ( CVariableInfo & cVar )
{
    // Save off the array index as I'll need that to get the count.
    ULONG ulArrayTypeIndex = cVar.ulDecodeTypeIndex ;
    
    BOOL bRet = FALSE ;
    
    // Get the type for the array.
    if ( ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    cVar.ulDecodeTypeIndex  ,
                                    TI_GET_TYPEID           ,
                                    &cVar.ulDecodeTypeIndex  ) ) &&
         ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    cVar.ulDecodeTypeIndex  ,
                                    TI_GET_SYMTAG           ,
                                    &cVar.eDecodeTypeTag     ) )   )
    {
        bRet = DecodeType ( cVar ) ;
        
        ASSERT ( TRUE == bRet ) ;
        if ( TRUE == bRet )
        {
            bRet = SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    ulArrayTypeIndex        ,
                                    TI_GET_COUNT            ,
                                    &cVar.dwArrayElemCount   ) ;
            
            ASSERT ( TRUE == bRet ) ;
            if ( TRUE == bRet )
            {
                TCHAR szBuff[ 20 ] ;
                wsprintf ( szBuff                ,
                           _T( "[%d]" )          ,
                           cVar.dwArrayElemCount  ) ;
                cVar.cTypeName += szBuff ;
            }
        }
    }
    
    return ( bRet ) ;
}

BOOL CSymbolEngine ::
            DecodeTypeSymTagFunctionType ( CVariableInfo & cVar )
{
    BOOL bRet = FALSE ;
    
    // With function types, I need to scoot through all the children
    // so I can build up the whole function pointer.
    CVariableInfo cTempVar ;
        
    // I only care about getting the type index and the tag into the
    // temporary as that's all I need to decode the children.
    if ( ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase        ,
                                    cVar.ulDecodeTypeIndex      ,
                                    TI_GET_TYPEID               ,
                                    &cTempVar.ulDecodeTypeIndex  ) ) &&
         ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase        ,
                                    cTempVar.ulDecodeTypeIndex  ,
                                    TI_GET_SYMTAG               ,
                                    &cTempVar.eDecodeTypeTag     ) )   )
    {
        // Decode the initial type.
        bRet = DecodeType ( cTempVar ) ;
        
        ASSERT ( TRUE == bRet ) ;
        if ( TRUE == bRet )
        {
            // Append this type onto the entry type.
            cVar.cTypeName += cTempVar.cTypeName ;
            cVar.cTypeName += _T ( " ( " ) ;
            
            // Get the children for the function type.
            DWORD dwChildCount = 0 ;
            if ( ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                            cVar.ulDecodeTypeIndex  ,
                                            TI_GET_CHILDRENCOUNT    ,
                                            &dwChildCount           ))&&
                 ( dwChildCount > 0 )                                  )
            {
                // Do each child of the SymTagFunctionType so I can get
                // all the parameters.
                FINDCHILDREN stFC ;
                stFC.Count = dwChildCount ;
                stFC.Start = 0 ;
                
                bRet = SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                        cVar.ulDecodeTypeIndex  ,
                                        TI_FINDCHILDREN         ,
                                        &stFC                    ) ;

                ASSERT ( TRUE == bRet ) ;
                if ( TRUE == bRet )
                {
                    for ( DWORD i = 0 ; i < dwChildCount ; i++ )
                    {
                        cTempVar.cTypeName[0] = _T ( '\0' ) ;
                        cTempVar.ulDecodeTypeIndex = stFC.ChildId[ i ] ;
                        
                        bRet = SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                                stFC.ChildId[ i ]    ,
                                                TI_GET_SYMTAG        ,
                                             &cTempVar.eDecodeTypeTag );
                        ASSERT ( TRUE == bRet ) ;
                        if ( TRUE == bRet )
                        {
                            bRet = DecodeType ( cTempVar ) ;
                            
                            ASSERT ( TRUE == bRet ) ;
                            if ( TRUE == bRet )
                            {
                                cVar.cTypeName += cTempVar.cTypeName ;
                                if ( i != ( dwChildCount - 1 ) )
                                {
                                    cVar.cTypeName += _T ( " , " ) ;
                                }
                            }
                        }
                        if ( FALSE == bRet )
                        {
                            break ;
                        }
                    }
                }
            }
        }
        cVar.cTypeName += _T ( " )" ) ;
    }
    return ( bRet ) ;
}
            
BOOL CSymbolEngine ::
            DecodeTypeSymTagFunctionArgType ( CVariableInfo & cVar )
{
    BOOL bRet = FALSE ;
    // Get the next type down the chain and decode that one.
    if ( ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    cVar.ulDecodeTypeIndex  ,
                                    TI_GET_TYPEID           ,
                                    &cVar.ulDecodeTypeIndex  ) ) &&
         ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    cVar.ulDecodeTypeIndex  ,
                                    TI_GET_SYMTAG           ,
                                    &cVar.eDecodeTypeTag     ) )   )
    {
        // Decode it.
        bRet = DecodeType ( cVar ) ;
        bRet = TRUE ;
    }
    return ( bRet ) ;
}

BOOL CSymbolEngine :: DecodeTypeSymTagEnum ( CVariableInfo & cVar )
{
    BOOL bRet = FALSE ;
    WCHAR * szwTypeName = NULL ;
    
    if ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase      ,
                                  cVar.ulDecodeTypeIndex    ,
                                  TI_GET_SYMNAME            ,
                                  &szwTypeName               ) )
    {
        cVar.cTypeName += szwTypeName ;
        LocalFree ( szwTypeName ) ;
        bRet = TRUE ;
    }
    return ( bRet ) ;
}

BOOL CSymbolEngine :: DecodeTypeSymTagData ( CVariableInfo & cVar )
{
    // Get the next time down the chain.
    BOOL bRet = TRUE ;
    
    ULONG ulTagDataIndex = cVar.ulDecodeTypeIndex ;
    
    if ( ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    cVar.ulDecodeTypeIndex  ,
                                    TI_GET_TYPEID           ,
                                    &cVar.ulDecodeTypeIndex  ) ) &&
         ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    cVar.ulDecodeTypeIndex  ,
                                    TI_GET_SYMTAG           ,
                                    &cVar.eDecodeTypeTag     ) )   )
    {
        // Data tags generally have some name associated with them.
        WCHAR * szwTypeName = NULL ;
    
        if ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase  ,
                                      ulTagDataIndex        ,
                                      TI_GET_SYMNAME        ,
                                      &szwTypeName           ) )
        {
            _tcsncpy ( cVar.szVarName , szwTypeName , MAX_VARNAME ) ;
            cVar.szVarName[ MAX_VARNAME - 1 ] = _T ( '\0' ) ;
        }
        
        // The data tag is special.  When a data tag is the starting
        // tag, the only time time this will ever get called, I will
        // bump the starting tag down to the actual datatype.
        cVar.eTag = cVar.eDecodeTypeTag ;
        cVar.ulTypeIndex = cVar.ulDecodeTypeIndex ;

        bRet = DecodeType ( cVar ) ;
    }
    else
    {
        ASSERT ( !"DecodeTypeSymTagData failed!" ) ;
        bRet = FALSE ;
    }
    return ( bRet ) ;
}

BOOL CSymbolEngine :: DecodeTypeSymTagVTable ( CVariableInfo & cVar )
{
    cVar.cTypeName = _T ( "__vfptr" ) ;

    // Data tags generally have some name associated with them.
    WCHAR * szwTypeName = NULL ;

    if ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase   ,
                                  cVar.ulDecodeTypeIndex ,
                                  TI_GET_SYMNAME         ,
                                  &szwTypeName            ) )
    {
        _tcsncpy ( cVar.szVarName , szwTypeName , MAX_VARNAME ) ;
        cVar.szVarName[ MAX_VARNAME - 1 ] = _T ( '\0' ) ;
        LocalFree ( szwTypeName ) ;
    }
    return ( TRUE ) ;
}

BOOL CSymbolEngine :: DecodeTypeSymTagBaseClass ( CVariableInfo & cVar)
{
    BOOL bRet = TRUE ;
    
    if ( ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    cVar.ulDecodeTypeIndex  ,
                                    TI_GET_TYPEID           ,
                                    &cVar.ulDecodeTypeIndex  ) ) &&
         ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                    cVar.ulDecodeTypeIndex  ,
                                    TI_GET_SYMTAG           ,
                                    &cVar.eDecodeTypeTag     ) )   )
    {
        WCHAR * szwTypeName = NULL ;

        if ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase  ,
                                     cVar.ulDecodeTypeIndex ,
                                     TI_GET_SYMNAME        ,
                                     &szwTypeName           ) )
        {
            _tcsncpy ( cVar.szVarName , szwTypeName , MAX_VARNAME ) ;
            cVar.szVarName[ MAX_VARNAME - 1 ] = _T ( '\0' ) ;
            LocalFree ( szwTypeName ) ;
        }
        
        bRet = DecodeType ( cVar ) ;
    }
    else
    {
        bRet = FALSE ;
    }
    
    return ( bRet ) ;
}

BOOL CSymbolEngine :: DecodeType ( CVariableInfo & cVar )
{
    BOOL bRet = TRUE ;
    switch ( cVar.eDecodeTypeTag )
    {
        case SymTagBlock :
            break ;
        case SymTagData :
            bRet = DecodeTypeSymTagData ( cVar ) ;
            break ;
        case SymTagAnnotation :
            break ;
        case SymTagLabel :
            break ;
        case SymTagPublicSymbol :
            break ;
        case SymTagUDT :
            bRet = DecodeTypeSymTagUDT ( cVar ) ;
            break ;
        case SymTagEnum :
            DecodeTypeSymTagEnum ( cVar ) ;
            break ;
        case SymTagFunctionType :
            bRet = DecodeTypeSymTagFunctionType ( cVar ) ;
            break ;
        case SymTagPointerType :
            bRet = DecodeTypeSymTagPointerType ( cVar ) ;
            break ;
        case SymTagArrayType :
            bRet = DecodeTypeTagArrayType ( cVar ) ;
            break ;
        case SymTagBaseType :
            bRet = DecodeTypeSymTagBaseType ( cVar ) ;
            break ;
        case SymTagTypedef :
            break ;
        case SymTagBaseClass :
            bRet = DecodeTypeSymTagBaseClass ( cVar ) ;
            break ;
        case SymTagFriend :
            break ;
        case SymTagFunctionArgType :
            bRet = DecodeTypeSymTagFunctionArgType ( cVar ) ;
            break ;
        case SymTagVTable :
            bRet = DecodeTypeSymTagVTable ( cVar ) ;
            break ;
        default :
            break ;
    }
    return ( bRet ) ;
}
