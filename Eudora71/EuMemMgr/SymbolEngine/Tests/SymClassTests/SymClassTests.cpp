/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "SymbolEngine.h"

CSymbolEngine g_cSym ;

// These are just here for jollys and to see if they show up in the
// symbol dump.
int FuncADelic ( int i )
{
    return ( i++ ) ;
}

int FuncORama ( int i )
{
    return ( i-- ) ;
}

int FuncORific ( int i )
{
    return ( (i--) + 1 ) ;
}

#define TAGCASESTRING(tag)      \
    case tag    :               \
        szBuff = _T (#tag) ;    \
        break ;

TCHAR * TraceSymTagEnum ( enum SymTagEnum eTag )
{
    TCHAR * szBuff = NULL ;
    switch ( eTag )
    {
        TAGCASESTRING ( SymTagNull )
        TAGCASESTRING ( SymTagExe )
        TAGCASESTRING ( SymTagCompiland )
        TAGCASESTRING ( SymTagCompilandDetails )
        TAGCASESTRING ( SymTagCompilandEnv )
        TAGCASESTRING ( SymTagFunction )
        TAGCASESTRING ( SymTagBlock )
        TAGCASESTRING ( SymTagData )
        TAGCASESTRING ( SymTagAnnotation )
        TAGCASESTRING ( SymTagLabel )
        TAGCASESTRING ( SymTagPublicSymbol )
        TAGCASESTRING ( SymTagUDT )
        TAGCASESTRING ( SymTagEnum )
        TAGCASESTRING ( SymTagFunctionType )
        TAGCASESTRING ( SymTagPointerType )
        TAGCASESTRING ( SymTagArrayType )
        TAGCASESTRING ( SymTagBaseType )
        TAGCASESTRING ( SymTagTypedef )
        TAGCASESTRING ( SymTagBaseClass )
        TAGCASESTRING ( SymTagFriend )
        TAGCASESTRING ( SymTagFunctionArgType )
        TAGCASESTRING ( SymTagFuncDebugStart )
        TAGCASESTRING ( SymTagFuncDebugEnd )
        TAGCASESTRING ( SymTagUsingNamespace )
        TAGCASESTRING ( SymTagVTableShape )
        TAGCASESTRING ( SymTagVTable )
        TAGCASESTRING ( SymTagCustom )
        TAGCASESTRING ( SymTagThunk )
        TAGCASESTRING ( SymTagCustomType )
        TAGCASESTRING ( SymTagManagedType )
        TAGCASESTRING ( SymTagDimension )
        default :
            szBuff = _T ( "**UNKNOWN SymTagEnum**" ) ;
            break ;
    }
    return ( szBuff ) ;
}
#define BASECASESTRING(tag)      \
    case tag    :               \
        szBuff = _T (#tag) ;    \
        break ;

TCHAR * TraceBasicType ( enum BasicType eBT )
{
    TCHAR * szBuff = NULL ;
    switch ( eBT )
    {
        BASECASESTRING ( btNoType )
        BASECASESTRING ( btVoid )
        BASECASESTRING ( btChar )
        BASECASESTRING ( btWChar )
        BASECASESTRING ( btInt )
        BASECASESTRING ( btUInt )
        BASECASESTRING ( btFloat )
        BASECASESTRING ( btBCD )
        BASECASESTRING ( btBool )
        BASECASESTRING ( btLong )
        BASECASESTRING ( btULong )
        BASECASESTRING ( btCurrency ) 
        BASECASESTRING ( btDate )
        BASECASESTRING ( btVariant ) 
        BASECASESTRING ( btComplex )
        BASECASESTRING ( btBit )
        BASECASESTRING ( btBSTR )
        BASECASESTRING ( btHresult )
        default :
            szBuff = _T ( "**UNKNOWN BasicType**" ) ;
            break ;
    }
    return ( szBuff ) ;
}


BOOL CALLBACK EnumeratorDude ( LPSTR   SymbolName      ,
                               DWORD64 SymbolAddress   ,
                               ULONG   SymbolSize      ,
                               PVOID   /*UserContext*/  )
{
    _tprintf ( _T ( "0x%016I64X %04d %-30S\n" ) ,
               SymbolAddress        ,
               SymbolSize           ,
               SymbolName            ) ;
             
    return ( TRUE ) ;
}

struct FINDCHILDREN : TI_FINDCHILDREN_PARAMS
{
    ULONG   MoreChildIds[1024];
    FINDCHILDREN ( )
    {
        Count = sizeof(MoreChildIds) / sizeof(MoreChildIds[0]);
    }
} ;

void DumpORama ( DWORD64         dwModBase   , 
                 DWORD           dwTypeIndex , 
                 enum SymTagEnum eTag        , 
                 int             iIndent     ,
                 BOOL            iChild       )
{
    BOOL bTypeID = FALSE ;
    BOOL bTagID = FALSE ;
    
    do 
    {
        // Do any indenting as appropriate.
        for ( int i = 0 ; i < iIndent ; i++ )
        {
            _tprintf ( _T ( "  " ) ) ;
        }
        // If the function was called as a result of the child looping,
        // indicate that so I can see how those fall in the scheme of
        // things.
        if ( iChild > 0)
        {
            _tprintf ( _T ( "(c %02d) " ) , iChild ) ;
        }
        // Output the SymTagEnum and the type index.
        _tprintf ( _T ( "%s %02d" ) , 
                    TraceSymTagEnum ( eTag )      , 
                    dwTypeIndex                    ) ;
        
        // See if I can get the name.  One really stupid inconsistency
        // is that the TI_GET_SYMNAME returns the actual variable name
        // for some symbols and the TYPE (!!) name for other symbols.
        // Nothing like having a little consistency.  Sheez.        
        WCHAR * szwTypeName = NULL ;
        if ( TRUE == g_cSym.SymGetTypeInfo ( dwModBase  ,
                                             dwTypeIndex        ,
                                             TI_GET_SYMNAME     ,
                                             &szwTypeName        ) )
        {
            _tprintf ( _T ( " : %ls" ) , szwTypeName ) ;
            LocalFree ( szwTypeName ) ;
        }
        _tprintf ( _T ( "\n" ) ) ;
        
        // If I try to access SymTagFunction types it causes an access
        // violation in DBGHELP.DLL, which terminates the enumeration.
        if ( SymTagFunction == eTag )
        {
            return ;
        }
        
        // See if I can find any children for this type.  I noticed that
        // SymGetTypeInfo will succeed but return a count of zero.
        DWORD dwChildCount = 0 ;
        if ( ( TRUE == g_cSym.SymGetTypeInfo ( dwModBase            ,
                                               dwTypeIndex          ,
                                               TI_GET_CHILDRENCOUNT ,
                                               &dwChildCount         ) ) &&
            ( dwChildCount > 0 )                                           )
        {
            FINDCHILDREN stFC ;
            stFC.Count = dwChildCount ;
            stFC.Start = 0 ;
            
            // Get all the type indexes for the children.
            if ( TRUE == g_cSym.SymGetTypeInfo ( dwModBase       ,
                                                 dwTypeIndex     ,
                                                 TI_FINDCHILDREN ,
                                                 &stFC            ) )
            {
                // Loop through them all.
                for ( DWORD i = 0 ; i < dwChildCount ; i++ )
                {
                    enum SymTagEnum eChildTag ;
                    if ( TRUE == g_cSym.SymGetTypeInfo ( dwModBase         ,
                                                         stFC.ChildId[ i ] ,
                                                         TI_GET_SYMTAG     ,
                                                         &eChildTag         ) )
                    {
                        DumpORama ( dwModBase           , 
                                    stFC.ChildId[ i ]   , 
                                    eChildTag           , 
                                    iIndent + 1         ,
                                    iChild + 1           ) ;
                    }                                                         
                }
            }
        }
        
        // The SymTagData has a child tag I need to grab before I can 
        // get the next type index.
        if ( SymTagData == eTag ) 
        {
            enum SymTagEnum ePossibleDataSubTag ;
            
            bTagID = g_cSym.SymGetTypeInfo ( dwModBase              ,
                                             dwTypeIndex            ,
                                             TI_GET_SYMTAG          ,
                                             &ePossibleDataSubTag    ) ;
            if ( ( TRUE == bTagID ) && ( ePossibleDataSubTag != eTag ) )
            {
                DumpORama ( dwModBase           , 
                            dwTypeIndex         , 
                            ePossibleDataSubTag ,
                            iIndent + 1         ,
                            0                    ) ;
            }
        }
        
        // Get the next level data.  Keep in mind this is not what 
        // DBGHELP and DIA call child data, though it looks just like
        // it.  If I want to expand out something like char ** argv,
        // I need to do this.
        bTypeID = g_cSym.SymGetTypeInfo ( dwModBase     ,
                                          dwTypeIndex   ,
                                          TI_GET_TYPEID ,
                                          &dwTypeIndex   ) ;
        bTagID = g_cSym.SymGetTypeInfo ( dwModBase      ,
                                         dwTypeIndex    ,
                                         TI_GET_SYMTAG  ,
                                         &eTag           ) ;
        iIndent++ ;
    }
    while ( ( TRUE == bTagID ) && ( TRUE == bTypeID ) ) ;    
}

BOOL CALLBACK EnumFuncSymbolsCallback ( PSYMBOL_INFO  pSymInfo,
                                        ULONG         SymbolSize,
                                        PVOID         UserContext )
{
    SymbolSize = SymbolSize ;
    UserContext = UserContext ;
       
    printf ( "\n!!!%s!!!\n" , pSymInfo->Name ) ;
    
    // Enumerating symbols always get SymTagData.  To get to the 
    // types, I'll get the next level down before passing it on.
    enum SymTagEnum eTag ;
    BOOL bTagID = g_cSym.SymGetTypeInfo ( pSymInfo->ModBase     ,
                                          pSymInfo->TypeIndex   ,
                                          TI_GET_SYMTAG         ,
                                          &eTag                  ) ;
    if ( TRUE == bTagID )
    {
        DumpORama ( pSymInfo->ModBase   , 
                    pSymInfo->TypeIndex , 
                    eTag                , 
                    0                   ,
                    0                    ) ;
    }    
    return ( TRUE ) ;
}                                         


enum eCars
{
    eMercedes = 1 ,
    ePorsche  = 2 ,
    eBMW      = 3 ,
    
} ;

class HappyGoLuckyClass
{
public  :
    HappyGoLuckyClass ( void )
    {
        m_Data = 0 ;
    }
    
private :
    int m_Data ;    
} ;

class DerivedFuncClass : public HappyGoLuckyClass 
{
public  :
    int m_DerivedData ;
} ;

typedef struct tag_NESTEDSTRUCT
{
    API_VERSION stAPI ;
    int iDoNothing ;
} NESTEDSTRUCT ;

void _tmain ( int /*argc*/ , TCHAR * argv[] )
{
    DerivedFuncClass cDerived ;
    cDerived.m_DerivedData = 0 ;
    
    int aIntArray[ 13 ] ;
    aIntArray[ 0 ] = 0x666 ;
    
    int *** pppInt = NULL ;
    pppInt = pppInt ;
    
    HappyGoLuckyClass aClassArray[ 20 ] ;
    
    HappyGoLuckyClass cHappyClass ;
    
    NESTEDSTRUCT stNested ;
    stNested.iDoNothing = 0 ;
    
    eCars eEnumTest ;
    
    eEnumTest = eMercedes ;

    LPAPI_VERSION pAPIV = ImagehlpApiVersion ( ) ;

    printf ( "IMAGEHLP version %d.%d.%d\n\n" ,
              pAPIV->MajorVersion            ,
              pAPIV->MinorVersion            ,
              pAPIV->Revision                 ) ;

    // Initialize the symbol engine.
    BOOL bRet = g_cSym.SymInitialize ( (HANDLE)1 , NULL , FALSE ) ;

    if ( FALSE == bRet )
    {
        printf ( "SymInitialize failed!!!\n" ) ;
        return ;
    }
    
    g_cSym.SymSetOptions ( 0 ) ;

    DWORD dwMS = 0 ;
    DWORD dwLS = 0 ;
    if ( FALSE == g_cSym.GetImageHlpVersion ( dwMS , dwLS ) )
    {
        printf ( "g_cSym.GetImageHlpVersion FAILED!!!\n" ) ;
        DebugBreak ( ) ;
    }
    else
    {
        _tprintf ( _T ( "DBGHELP.DLL File Version : " ) ) ;
        _tprintf ( _T ( "%d.%02d.%d.%d" ) ,
                   HIWORD ( dwMS )        ,
                   LOWORD ( dwMS )        ,
                   HIWORD ( dwLS )        ,
                   LOWORD ( dwLS )         ) ;
        _tprintf ( _T ( "\n\n" ) ) ;
    }

    DWORD dwOpts = SymGetOptions ( ) ;
    dwOpts = dwOpts ;
    g_cSym.SymSetOptions ( SYMOPT_LOAD_LINES        |
                           SYMOPT_OMAP_FIND_NEAREST &
                           ~SYMOPT_UNDNAME           ) ;

    // Load this module.
    HANDLE hFile = CreateFile ( argv[ 0 ]               ,
                                GENERIC_READ            ,
                                FILE_SHARE_READ         ,
                                NULL                    ,
                                OPEN_EXISTING           ,
                                FILE_ATTRIBUTE_NORMAL   ,
                                NULL                     ) ;
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        _tprintf ( _T ( "CreateFile failed\n" ) ) ;
    }

    

    bRet = (BOOL)g_cSym.SymLoadModule64 ( hFile            ,
                                          argv[ 0 ]        ,
                                          NULL             ,
                                          0                ,
                                          0                 ) ;
    if ( FALSE == bRet )
    {
        _tprintf ( _T ( "SymLoadModule failed\n" ) ) ;
        return ;
    }

    // Get information about the module just loaded.
    IMAGEHLP_MODULE64 stIM ;
    memset ( &stIM , NULL , sizeof ( IMAGEHLP_MODULE64 ) ) ;
    stIM.SizeOfStruct = sizeof ( IMAGEHLP_MODULE64 ) ;

    bRet = g_cSym.SymGetModuleInfo64 ( 0x400000     ,
                                       &stIM         ) ;

    if ( FALSE == bRet )
    {
        _tprintf ( _T ( "SymGetModuleInfo failed\n" ) ) ;
        return ;
    }

    _tprintf ( _T ( "Module info    :\n" ) ) ;
    _tprintf ( _T ( "  Image base   : 0x%016X\n" ) , stIM.BaseOfImage ) ;
    _tprintf ( _T ( "  Image size   : %d\n"      ) , stIM.ImageSize ) ;
    _tprintf ( _T ( "  Time stamp   : 0x%08X\n"  ) , stIM.TimeDateStamp ) ;
    _tprintf ( _T ( "  Checksum     : 0x%08X\n"  ) , stIM.CheckSum ) ;
    _tprintf ( _T ( "  Symbol count : %d\n"      ) , stIM.NumSyms  ) ;
    _tprintf ( _T ( "  Symbol type  : " ) ) ;
    switch ( stIM.SymType )
    {
        case SymNone            :
            _tprintf ( _T ( "SymNone\n" ) ) ;
            break ;
        case SymCoff            :
            _tprintf ( _T ( "SymCoff\n" ) ) ;
            break ;
        case SymCv              :
            _tprintf ( _T ( "SymCv\n" ) ) ;
            break ;
        case SymPdb             :
            _tprintf ( _T ( "SymPdb\n" ) ) ;
            break ;
        case SymExport          :
            _tprintf ( _T ( "SymExport\n" ) ) ;
            break ;
        case SymDeferred        :
            _tprintf ( _T ( "SymDeferred\n" ) ) ;
            break ;
        case SymSym             :
            _tprintf ( _T ( "SymSym\n" ) ) ;
            break ;
        case SymDia             :
            _tprintf ( _T ( "SymDIA\n" ) ) ;
            break ;
        default :
            _tprintf ( _T ( "**UNKNOWN!!!!!\n" ) ) ;
    }
    _tprintf ( _T ( "  Module Name  : %s\n" ) , stIM.ModuleName  ) ;
    _tprintf ( _T ( "  Image  Name  : %s\n" ) , stIM.ImageName  ) ;
    _tprintf ( _T ( "  Loaded Name  : %s\n" ) , stIM.LoadedImageName ) ;

    bRet = g_cSym.SymEnumerateSymbols64 ( 0x400000       ,
                                          EnumeratorDude ,
                                          0               ) ;

    if ( FALSE == bRet )
    {
        _tprintf ( _T ( "SymEnumerateSymbols64 failed\n" ) ) ;
        return ;
    }

    PIMAGEHLP_SYMBOL64 pSym = (PIMAGEHLP_SYMBOL64)new char[ 256 ] ;
    memset ( pSym , NULL , 256 ) ;
    pSym->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL64 ) ;
    pSym->MaxNameLength = 200 ;

    bRet = g_cSym.SymGetSymFromName64 ( "wmain" , pSym ) ;
    if ( FALSE == bRet )
    {
        _tprintf ( _T ( "GetSymFromName64 failed looking up wmain!\n" ) ) ;
        delete [] pSym ;
        return ;
    }

    _tprintf ( _T ( "main is at 0x%08X\n" ) , pSym->Address ) ;

    IMAGEHLP_LINE64 stLine ;
    memset ( &stLine , NULL , sizeof ( IMAGEHLP_LINE64 ) ) ;
    stLine.SizeOfStruct = sizeof ( IMAGEHLP_LINE64 ) ;

    DWORD dwDis ;
    bRet = g_cSym.SymGetLineFromAddr64 ( pSym->Address ,
                                         &dwDis     ,
                                         &stLine     ) ;
    if ( FALSE ==bRet )
    {
        _tprintf ( _T ( "SymGetLineFromAddr failed\n" ) ) ;
        delete [] pSym ;
        return ;
    }
    _tprintf ( _T ( "Line information :\n" ) ) ;
    _tprintf ( _T ( "  File           : %s\n"      ) , stLine.FileName ) ;
    _tprintf ( _T ( "  Line Number    : %d\n"      ) , stLine.LineNumber ) ;
    _tprintf ( _T ( "  Address        : 0x%016X\n" ) , stLine.Address ) ;
    _tprintf ( _T ( "  Displacement   : %d\n"      ) , dwDis ) ;

    IMAGEHLP_STACK_FRAME stIHSF ;
    ZeroMemory ( &stIHSF , sizeof ( IMAGEHLP_STACK_FRAME ) ) ;
    stIHSF.InstructionOffset = stLine.Address ;
    
    if ( FALSE == g_cSym.SymSetContext ( &stIHSF , 0 ) )
    {
        _tprintf ( _T ( "SymSetContext failed!!\n" ) ) ;
        delete [] pSym ;
        return ;
    }
    
    PSYM_ENUMERATESYMBOLS_CALLBACK  pFn = EnumFuncSymbolsCallback ;

    // Enumerate the locals for main.
    bRet = g_cSym.SymEnumSymbols ( 0        ,
                                   0        ,
                                   pFn      ,
                                   &stIHSF   ) ;

    
    // Iterate backwards from main to the beginning of the line info.
    IMAGEHLP_LINE64 stTempLine ;
    stTempLine = stLine ;

    while ( TRUE == g_cSym.SymGetLinePrev64 ( &stLine ) )
    {
        // SymGetLinePrev64 NEVER ends!  Keep track of previous
        // lines seen so when SymGetLinePrev64 gets into it's infinite
        // loop I can break out!
        if ( stTempLine.LineNumber == stLine.LineNumber )
        {
            break ;
        }
        stTempLine = stLine ;
    }

    stLine = stTempLine ;

    // Now iterate through forwards printing out each line record.
    _tprintf ( _T ( "Line information  :\n" ) ) ;
    do
    {
        _tprintf ( _T ( "  File : %-15S Line : %d Address : 0x%016X\n" ) ,
                   stLine.FileName    ,
                   stLine.LineNumber  ,
                   stLine.Address      ) ;
    } while ( g_cSym.SymGetLineNext64 ( &stLine ) ) ;

    bRet = g_cSym.SymCleanup ( ) ;
    if ( FALSE == bRet )
    {
        _tprintf ( _T ( "SymCleanup failed!!!\n" ) ) ;
    }
    delete [] pSym ;
}
