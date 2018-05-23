/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "SymbolEngine.h"
#include "SymEngInternal.h"

BOOL CSymbolEngine :: DecodeVariable ( CVariableInfo & cStartVar ,
                                       int             iIndent    )
{
    BOOL bRet = TRUE ;
    
    if ( iIndent > m_iMaxExpandLevel )
    {
        return ( TRUE ) ;
    }
    
    // SymTagFunction values cause DBGHELP.DLL to access violate so
    // I don't even touch them.
    if ( cStartVar.eTag == SymTagFunction )
    {
        return ( TRUE ) ;
    }

    // The first step is to decode this start variable's type.
    bRet = DecodeType ( cStartVar ) ;
    
    ASSERT ( TRUE == bRet ) ;
    if ( TRUE == bRet )
    {
        // Now get the address for this variable.
        bRet = DetermineAddress ( cStartVar ) ;
        ASSERT ( TRUE == bRet ) ;
        if ( FALSE == bRet )
        {
            return ( FALSE ) ;
        }
        
        // Get the value for this variable.
        bRet = DecodeValue ( cStartVar ) ;
        ASSERT ( TRUE == bRet ) ;
        if ( FALSE == bRet )
        {
            return ( FALSE ) ;
        }
        
        // Do the output through the callback.
        bRet = OutputVariable ( cStartVar , iIndent ) ;
        
        if ( TRUE == bRet )
        {
            
            // Am I supposed to expand past the first level?
            // Additionally, I don't want to process function types
            // and enums as the child items to them are parameters
            // and the enum values.
            if ( ( m_iMaxExpandLevel > 0                ) &&
                 ( SymTagFunctionType != cStartVar.eTag ) &&
                 ( SymTagEnum         != cStartVar.eTag )   )
            {
                // Does this variable have any children nodes?
                DWORD dwChildCount = 0 ;
                if ( ( TRUE == SymGetTypeInfo ( m_dwCurrLocalModBase,
                                              cStartVar.ulTypeIndex ,
                                              TI_GET_CHILDRENCOUNT  ,
                                              &dwChildCount        ))&&
                     ( dwChildCount > 0 )                              )
                {
                    // Yes it does, so get the kids.
                    FINDCHILDREN stFC ;
                    stFC.Count = dwChildCount ;
                    stFC.Start = 0 ;
                
                    bRet = SymGetTypeInfo ( m_dwCurrLocalModBase    ,
                                            cStartVar.ulTypeIndex   ,
                                            TI_FINDCHILDREN         ,
                                            &stFC                    ) ;

                    ASSERT ( TRUE == bRet ) ;
                    if ( TRUE == bRet )
                    {
                        CVariableInfo cChildVar ;
                        for ( DWORD i = 0 ; i < dwChildCount ; i++ )
                        {
                            // Clear out anything that might have
                            // been there before without getting rid of
                            // any allocated string memory.
                            cChildVar.ZeroData ( ) ;
                            
                            // Set up the type indexes.
                            cChildVar.ulDecodeTypeIndex =
                                                stFC.ChildId[ i ] ;
                            cChildVar.ulTypeIndex = stFC.ChildId[i];
                        
                            // Set up the symbol tags.
                            bRet = SymGetTypeInfo(m_dwCurrLocalModBase,
                                                  stFC.ChildId[ i ]   ,
                                                  TI_GET_SYMTAG       ,
                                            &cChildVar.eDecodeTypeTag );
                            cChildVar.eTag = cChildVar.eDecodeTypeTag;
                            
                            // For COM pointers, there's a weird
                            // SymTagNull floating along with the array
                            // of functions.
                            // Hence, I'll skip those here.
                            if ( SymTagNull == cChildVar.eTag )
                            {
                                bRet = TRUE ;
                                continue ;
                            }

                            // Check that getting the tag worked.
                            ASSERT ( TRUE == bRet ) ;
                            if ( FALSE == bRet )
                            {
                                break ;
                            }
                            
                            // Get the address offset.
                            DWORD dwOffset = 0 ;
                            bRet =
                                SymGetTypeInfo ( m_dwCurrLocalModBase  ,
                                                 cChildVar.ulTypeIndex ,
                                                 TI_GET_OFFSET         ,
                                                 &dwOffset            );

                            // The call can fail, but all it means is
                            // that the offset is zero.
                            if ( FALSE == bRet )
                            {
                                dwOffset = 0 ;
                            }
                            
                            // Calculate the child address.
                            cChildVar.dwAddress =
                                    cStartVar.dwAddress +
                                           dwOffset      ;
                            
                            // Decode the child.
                            bRet = DecodeVariable ( cChildVar ,
                                                    iIndent + 1  ) ;
                            if ( FALSE == bRet )
                            {
                                ASSERT ( !"Whoops!" ) ;
                                break ;
                            }
                        }
                    }
                }
                
                // If this is a pointer, I want to expand through
                // so I can display things like int ** into:
                // int * *
                //   int *
                //    int
                // Of course I don't look at empty pointers, bother
                // expanding if I don't need to, or this value is
                // a character string I have already expanded as
                // part of the value display.
                if ( ( SymTagPointerType == cStartVar.eTag ) &&
                     ( 0 != cStartVar.dwAddress            ) &&
                     ( m_iMaxExpandLevel > 0               ) &&
                     ( FALSE == cStartVar.bIsStringArray   )    )
                {
                    // Get the address this pointer points to.
                    ReadPointerAddress ( cStartVar ) ;
                    
                    // If it's not zero, I can grind on it.
                    if ( 0 != cStartVar.dwAddress )
                    {
                        // Get the next level down info.
                        if ((TRUE==SymGetTypeInfo(m_dwCurrLocalModBase ,
                                                 cStartVar.ulTypeIndex ,
                                                 TI_GET_TYPEID         ,
                                              &cStartVar.ulTypeIndex))&&
                            (TRUE==SymGetTypeInfo(m_dwCurrLocalModBase ,
                                                 cStartVar.ulTypeIndex ,
                                                TI_GET_SYMTAG          ,
                                                &cStartVar.eTag      )))
                        {
                            // Don't need the strings any more.
                            cStartVar.szVarName[ 0 ] = _T ( '\0' ) ;
                            cStartVar.cTypeName.NullString ( ) ;
                            cStartVar.szValue[ 0 ] = _T ( '\0' ) ;
                            
                            // Set the tag and type info.
                            cStartVar.eDecodeTypeTag = cStartVar.eTag ;
                            cStartVar.ulDecodeTypeIndex =
                                                 cStartVar.ulTypeIndex ;
                                                    
                            // Any flag info is unnecessary from here on
                            // out.
                            cStartVar.ulFlags = 0 ;
                            
                            // Decode this bad boy.
                            bRet = DecodeVariable ( cStartVar   ,
                                                    iIndent + 1  ) ;
                        }
                    }
                }
                // If it's an array type, the user wants them
                // expanded, the expand level is greater than
                // zero, and I haven't done the character array
                // expansion already, I'll dump it.
                else if ( ( SymTagArrayType == cStartVar.eTag ) &&
                          ( TRUE == m_bExpandArrays           ) &&
                          ( m_iMaxExpandLevel > 0             ) &&
                          ( FALSE == cStartVar.bIsStringArray )   )
                {
                    bRet = DumpArray ( cStartVar , iIndent ) ;
                }
            }
        }
    }
    return ( bRet ) ;
}

// The helper function for DecodeVariable to display array fields.
BOOL CSymbolEngine :: DumpArray ( CVariableInfo & cVar    ,
                                  int             iIndent  )
{
    ASSERT ( SymTagArrayType == cVar.eTag ) ;
    ASSERT ( TRUE == m_bExpandArrays ) ;
    
    // First step is to get the size of the whole array block of memory.
    ULONG64 ulArrayByteLen = 0 ;
    BOOL bRet = SymGetTypeInfo ( m_dwCurrLocalModBase  ,
                                 cVar.ulTypeIndex      ,
                                 TI_GET_LENGTH         ,
                                 &ulArrayByteLen        ) ;
    ASSERT ( TRUE == bRet ) ;
    if ( TRUE == bRet )
    {
        CVariableInfo cElemVar ;

        // Get the element type and tag.
        bRet = SymGetTypeInfo ( m_dwCurrLocalModBase  ,
                                cVar.ulTypeIndex      ,
                                TI_GET_TYPEID         ,
                                &cElemVar.ulTypeIndex  ) ;
        cElemVar.ulDecodeTypeIndex = cElemVar.ulTypeIndex ;
        
        ASSERT ( TRUE == bRet ) ;
        if ( TRUE == bRet )
        {
            // Get the symbol tag for the element.
            bRet = SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                    cElemVar.ulTypeIndex ,
                                    TI_GET_SYMTAG        ,
                                    &cElemVar.eTag        ) ;
            cElemVar.eDecodeTypeTag = cElemVar.eTag ;
            
            ASSERT ( TRUE == bRet ) ;
            if ( TRUE == bRet )
            {
                // Get the size of each elemnt.
                ULONG64 ulElemSize = 0 ;
                bRet = SymGetTypeInfo ( m_dwCurrLocalModBase ,
                                        cElemVar.ulTypeIndex ,
                                        TI_GET_LENGTH        ,
                                        &ulElemSize           ) ;
                ASSERT ( TRUE == bRet ) ;
                if ( TRUE == bRet )
                {
                    // Now I can loop through the elements.
                    for ( DWORD i = 0               ;
                          i < cVar.dwArrayElemCount ;
                          i++                        )
                    {
                        // Clear any strings on this element.
                        cElemVar.cTypeName.NullString ( ) ;
                        cElemVar.szValue[ 0 ] = _T ( '\0' ) ;
                        cElemVar.szVarName[ 0 ] = _T ( '\0' ) ;
                        
                        // Reset the decode type index and tag so I can
                        // start at the top of the element for
                        // decoding.
                        cElemVar.ulDecodeTypeIndex =
                                            cElemVar.ulTypeIndex ;
                        cElemVar.eDecodeTypeTag = cElemVar.eTag ;
                        
                        // Calculate the address for this element.
                        cElemVar.dwAddress = cVar.dwAddress +
                                              ( i * ulElemSize ) ;
                                                
                        // The array position is the variable
                        // name.
                        TCHAR * szFmt = _T ( "[0x%X]" ) ;
                        if ( 10 == m_iRadix )
                        {
                            szFmt = _T ( "[%d]" ) ;
                        }
                        wsprintf ( cElemVar.szVarName , szFmt , i ) ;
                        
                        // Decode it.
                        bRet = DecodeVariable ( cElemVar    ,
                                                iIndent + 1  ) ;
                        ASSERT ( TRUE == bRet ) ;
                        if ( FALSE == bRet )
                        {
                            break ;
                        }
                    }
                }
            }
        }
    }
    return ( bRet ) ;
}

// The friend function that takes care of all the local enumeration.
BOOL CALLBACK LocalsEnumSymbols ( PSYMBOL_INFO pSymInfo        ,
                                  ULONG        /*SymbolSize*/  ,
                                  PVOID        UserContext      )
{
    // Get the symbol engine calling this function.
    CSymbolEngine * pSym = (CSymbolEngine *)UserContext ;
    // Double check my own stupidity.
    ASSERT ( 0 != pSym->m_hProcess ) ;
    
    // Set the module base for this local.  This way I don't have to
    // haul the value through the recursion.
    pSym->m_dwCurrLocalModBase = pSymInfo->ModBase ;
    
    // The return value.
    BOOL bRet = TRUE ;

    // Enumerating symbols always get SymTagData.  To get to the
    // types, I'll get the next level down before passing it on.
    enum SymTagEnum eStartTag = SymTagNull ;
    BOOL bTagID = pSym->SymGetTypeInfo ( pSymInfo->ModBase      ,
                                         pSymInfo->TypeIndex    ,
                                         TI_GET_SYMTAG          ,
                                         &eStartTag              ) ;
    ASSERT ( TRUE == bTagID ) ;
    if ( TRUE == bTagID )
    {
        pSym->m_cCurrentLocal.InitializeFromSymbolInfo ( pSymInfo  ,
                                                         eStartTag  ) ;
        
        bRet = pSym->DecodeVariable ( pSym->m_cCurrentLocal , 0 ) ;
    }
                                         
    return ( bRet ) ;
}


BOOL CSymbolEngine :: EnumLocalVariables
                    ( PENUM_LOCAL_VARS_CALLBACK      pCallback     ,
                      int                            iExpandLevel  ,
                      BOOL                           bExpandArrays ,
                      PREAD_PROCESS_MEMORY_ROUTINE64 pReadMem      ,
                      LPSTACKFRAME64                 pFrame        ,
                      CONTEXT *                      pContext      ,
                      PVOID                          pUserContext   )
{
    ASSERT ( FALSE == IsBadCodePtr ( (FARPROC)pCallback ) ) ;
    ASSERT ( FALSE == IsBadReadPtr ( pFrame ,
                                     sizeof ( LPSTACKFRAME64 ) ) ) ;
    if ( ( TRUE == IsBadCodePtr ( (FARPROC)pCallback ) ) ||
         ( TRUE == IsBadReadPtr ( pFrame                    ,
                                  sizeof ( LPSTACKFRAME64 )  ) )   )
    {
        return ( FALSE ) ;
    }
    // Save off the important stuff so I don't have to pass them
    // all through this massively recursive code.
    m_pUserCallback   = pCallback ;
    m_pFrame          = pFrame ;
    m_pContext        = pContext ;
    m_pReadMem        = pReadMem ;
    m_pUserContext    = pUserContext ;
    m_bExpandArrays   = bExpandArrays ;
    m_iMaxExpandLevel = iExpandLevel ;
    

    // Set up the locals context based on the data in the frame.
    IMAGEHLP_STACK_FRAME stIHSF ;
    ZeroMemory ( &stIHSF , sizeof ( IMAGEHLP_STACK_FRAME ) ) ;
    
    stIHSF.InstructionOffset = pFrame->AddrPC.Offset ;
    stIHSF.StackOffset       = pFrame->AddrStack.Offset ;
    stIHSF.FrameOffset       = pFrame->AddrFrame.Offset ;
    

    BOOL bRet = SymSetContext ( &stIHSF , 0 ) ;
    if ( FALSE == bRet )
    {
        return ( FALSE ) ;
    }
    
    // Enumeratero.
    bRet = SymEnumSymbols ( 0 , 0 , LocalsEnumSymbols , this ) ;
    
    return ( TRUE ) ;

}

BOOL CSymbolEngine :: OutputVariable ( CVariableInfo & cVar     ,
                                       int             iIndent   )
{
    BOOL bRet = m_pUserCallback ( cVar.dwAddress   ,
                                  cVar.cTypeName   ,
                                  cVar.szVarName   ,
                                  cVar.szValue     ,
                                  iIndent          ,
                                  m_pUserContext    ) ;
    return ( bRet ) ;
}

BOOL CSymbolEngine :: MemRead ( LPCVOID lpAddr    ,
                                PBYTE   pData     ,
                                int     iDataSize ,
                                LPDWORD lpdwRead  )
{
    BOOL bMemRead = FALSE ;
    if ( NULL == m_pReadMem )
    {
        bMemRead = ReadProcessMemory ( m_hProcess   ,
                                       lpAddr       ,
                                       pData        ,
                                       iDataSize    ,
                                       lpdwRead      ) ;
    }
    else
    {
        bMemRead = m_pReadMem ( m_hProcess        ,
                                (DWORD64)lpAddr   ,
                                pData             ,
                                iDataSize         ,
                                lpdwRead           ) ;
    }
    return ( bMemRead ) ;
}

BOOL CSymbolEngine :: ReadPointerAddress ( CVariableInfo & cVar )
{
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
                cVar.dwAddress = (DWORD64)*pDataBuff ;
            }
            else
            {
                cVar.dwAddress = (DWORD)dwData[0] ;
            }
        }
    }
    
    return ( TRUE ) ;
}

BOOL CSymbolEngine :: GetInMemoryFileVersion ( TCHAR * szFile ,
                                               DWORD & dwMS   ,
                                               DWORD & dwLS    )
{
    HMODULE hInstIH = GetModuleHandle ( szFile ) ;

    // Get the full filename of the loaded version.
    TCHAR szImageHlp[ MAX_PATH ] ;
    GetModuleFileName ( hInstIH , szImageHlp , MAX_PATH ) ;

    dwMS = 0 ;
    dwLS = 0 ;

    // Get the version information size.
    DWORD dwVerInfoHandle ;
    DWORD dwVerSize       ;

    dwVerSize = GetFileVersionInfoSize ( szImageHlp       ,
                                            &dwVerInfoHandle  ) ;
    if ( 0 == dwVerSize )
    {
        return ( FALSE ) ;
    }

    // Got the version size, now get the version information.
    LPVOID lpData = (LPVOID)new TCHAR [ dwVerSize ] ;
    if ( FALSE == GetFileVersionInfo ( szImageHlp       ,
                                        dwVerInfoHandle  ,
                                        dwVerSize        ,
                                        lpData            ) )
    {
        delete [] lpData ;
        return ( FALSE ) ;
    }

    VS_FIXEDFILEINFO * lpVerInfo ;
    UINT uiLen ;
    BOOL bRet = VerQueryValue ( lpData              ,
                                _T ( "\\" )         ,
                                (LPVOID*)&lpVerInfo ,
                                &uiLen               ) ;
    if ( TRUE == bRet )
    {
        dwMS = lpVerInfo->dwFileVersionMS ;
        dwLS = lpVerInfo->dwFileVersionLS ;
    }

    delete [] lpData ;

    return ( bRet ) ;
}

FARPROC CSymbolEngine :: GetDbgHelpAddress ( LPCSTR szFunc )
{
    FARPROC pRet = NULL ;
    HINSTANCE hInst = GetModuleHandle ( _T ( "DBGHELP.DLL" ) ) ;
    ASSERT ( NULL != hInst ) ;
    if ( NULL != hInst )
    {
        pRet = GetProcAddress ( hInst , szFunc ) ;
    }
    return ( pRet ) ;
}

BOOL CSymbolEngine :: SymEnumSymbols ( ULONG64   BaseOfDll             ,
                                       PCSTR     Mask                  ,
                                       PSYM_ENUMERATESYMBOLS_CALLBACK
                                               EnumSymbolsCallback     ,
                                       PVOID     UserContext           )
{
    if ( NULL == m_pSymEnumSymbols )
    {
        m_pSymEnumSymbols = (PSYMENUMSYMBOLS)
                                GetDbgHelpAddress ( "SymEnumSymbols" ) ;
    }
    BOOL bRet = FALSE ;
    if ( NULL != m_pSymEnumSymbols )
    {
        bRet = m_pSymEnumSymbols ( m_hProcess          ,
                                   BaseOfDll           ,
                                   Mask                ,
                                   EnumSymbolsCallback ,
                                   UserContext          ) ;
    }
    return ( bRet ) ;
}

ULONG CSymbolEngine :: SymSetContext ( PIMAGEHLP_STACK_FRAME StackFrame,
                                       PIMAGEHLP_CONTEXT     Context   )
{
    if ( NULL == m_pSymSetContext )
    {
        m_pSymSetContext = (PSYMSETCONTEXT)
                                 GetDbgHelpAddress ( "SymSetContext" ) ;
    }
    ULONG ulRet = 0 ;
    if ( NULL != m_pSymSetContext )
    {
        ulRet = m_pSymSetContext ( m_hProcess ,
                                   StackFrame ,
                                   Context     ) ;
    }
    return ( ulRet ) ;
}

BOOL CSymbolEngine :: SymFromAddr ( DWORD64       Address        ,
                                    PDWORD64      Displacement   ,
                                    PSYMBOL_INFO  Symbol          )
{
    if ( NULL == m_pSymSetContext )
    {
        m_pSymFromAddr = (PSYMFROMADDR)GetDbgHelpAddress("SymFromAddr");
    }
    BOOL bRet = 0 ;
    if ( NULL != m_pSymFromAddr )
    {
        bRet = m_pSymFromAddr ( m_hProcess     ,
                                Address        ,
                                Displacement   ,
                                Symbol          ) ;
    }
    return ( bRet ) ;
}

BOOL CSymbolEngine :: SymFromName ( LPSTR        Name    ,
                                    PSYMBOL_INFO Symbol   )
{
    if ( NULL == m_pSymFromName )
    {
        m_pSymFromName = (PSYMFROMNAME)GetDbgHelpAddress("SymFromName");
    }
    BOOL bRet = 0 ;
    if ( NULL != m_pSymFromName )
    {
        bRet = m_pSymFromName ( m_hProcess , Name , Symbol ) ;
    }
    return ( bRet ) ;
}

BOOL CSymbolEngine :: SymGetTypeFromName ( ULONG64       BaseOfDll ,
                                           LPSTR         Name      ,
                                           PSYMBOL_INFO  Symbol     )
{
    if ( NULL == m_pSymGetTypeFromName )
    {
        m_pSymGetTypeFromName = (PSYMGETTYPEFROMNAME)
                            GetDbgHelpAddress ( "SymGetTypeFromName" ) ;
    }
    BOOL bRet = FALSE ;
    if ( NULL != m_pSymGetTypeFromName )
    {
        bRet = m_pSymGetTypeFromName ( m_hProcess  ,
                                       BaseOfDll   ,
                                       Name        ,
                                       Symbol       ) ;
    }
    return ( bRet ) ;
}

BOOL CSymbolEngine :: SymGetTypeInfo(DWORD64                    ModBase,
                                     ULONG                      TypeId ,
                                     IMAGEHLP_SYMBOL_TYPE_INFO  GetType,
                                     PVOID                      pInfo  )
{
    if ( NULL == m_pSymGetTypeInfo )
    {
        m_pSymGetTypeInfo = (PSYMGETTYPEINFO)
                                GetDbgHelpAddress ( "SymGetTypeInfo" ) ;
    }
    BOOL bRet = FALSE ;
    if ( NULL != m_pSymGetTypeInfo )
    {
        bRet = m_pSymGetTypeInfo ( m_hProcess  ,
                                   ModBase     ,
                                   TypeId      ,
                                   GetType     ,
                                   pInfo        ) ;
    }
    return ( bRet ) ;
}

BOOL CSymbolEngine :: SymEnumTypes ( ULONG64     BaseOfDll           ,
                                     PSYM_ENUMERATESYMBOLS_CALLBACK
                                                 EnumSymbolsCallback ,
                                     PVOID       UserContext          )
{
    if ( NULL == m_pSymEnumTypes )
    {
        m_pSymEnumTypes = (PSYMENUMTYPES)
                                  GetDbgHelpAddress ( "SymEnumTypes" ) ;
    }
    BOOL bRet = FALSE ;
    if ( NULL != m_pSymEnumTypes )
    {
        bRet = m_pSymEnumTypes ( m_hProcess            ,
                                 BaseOfDll             ,
                                 EnumSymbolsCallback   ,
                                 UserContext            ) ;
    }
    return ( bRet ) ;
}

    
