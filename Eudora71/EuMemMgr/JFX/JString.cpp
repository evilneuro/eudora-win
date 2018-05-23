/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.

September, 25, 2003 - Fixed one off bugs in:
    operator = ( JString & )
    operator = ( TCHAR * )
    operator = ( char * )
    operator += ( const char * )
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "JString.h"

#ifndef _PORTABLE_JSTRING
#include "JFX.h"
#endif

const int k_EXTRAMEMLEN = 25 ;

// Internal allocation functions so I can play with different styles
// of allocations.
static TCHAR * Allocate ( size_t iLen )
{
    void * pRet = HeapAlloc ( GetProcessHeap ( )             ,
                              HEAP_GENERATE_EXCEPTIONS |
                                    HEAP_ZERO_MEMORY         ,
                              iLen                            ) ;
    return ( (TCHAR*)pRet ) ;
}

static BOOL Deallocate ( void * pMem )
{
    BOOL bRet = HeapFree ( GetProcessHeap ( ) , 0 , pMem ) ;
    return ( bRet ) ;
}

// Constructor that creates a NULL string.
JString :: JString ( )
{
    m_szData = NULL ;
    m_iBuffLen = 0 ;
    m_iAllocLen = k_EXTRAMEMLEN ;
    m_bDeleteBufferOnDestruct = TRUE ;
    m_pEndPos = m_szData ;
}

// Constructor that creates a string from a constant char array.
JString :: JString ( const TCHAR * szInString )
{
    // Calculate total amount to allocate.
    m_iAllocLen = k_EXTRAMEMLEN ;

    size_t iLen = _tcslen ( szInString ) ;
    
    m_iBuffLen = iLen + 1 + m_iAllocLen ;
    
    m_szData = Allocate ( m_iBuffLen * sizeof ( TCHAR ) ) ;

    _tcscpy ( m_szData , szInString ) ;
    
    m_pEndPos = m_szData + iLen ;
    
    m_bDeleteBufferOnDestruct = TRUE ;
}

// Constructor that creates a class that does not delete the
// buffer when the destructor is called.
JString :: JString ( bool bDeleteBufferOnDestruct )
{
    m_szData = NULL ;
    m_iBuffLen = 0 ;
    m_iAllocLen = k_EXTRAMEMLEN ;
    m_bDeleteBufferOnDestruct = bDeleteBufferOnDestruct ;
    m_pEndPos = m_szData ;
}

// Copy constructor.
JString :: JString ( const JString & cInString )
{
    m_iAllocLen = cInString.m_iAllocLen ;
    if ( NULL == cInString.m_szData )
    {
        m_szData = NULL ;
        m_pEndPos = m_szData ;
    }
    else
    {
        size_t iLen = _tcslen ( cInString.m_szData ) ;
        m_iBuffLen = iLen + 1 + m_iAllocLen ;
        m_szData = Allocate ( m_iBuffLen * sizeof ( TCHAR ) );
        _tcscpy ( m_szData , cInString.m_szData ) ;
        m_iAllocLen = cInString.m_iAllocLen ;
        m_bDeleteBufferOnDestruct = TRUE ;
        m_pEndPos = m_szData + iLen ;
    }
}

// Destructor.
JString :: ~JString ( )
{
    if ( TRUE == m_bDeleteBufferOnDestruct )
    {
        Deallocate ( m_szData ) ;
        m_szData = NULL ;
        m_pEndPos = NULL ;
        m_iBuffLen = 0 ;
    }
}

// Equality operators.
bool operator == ( const JString & s , const JString & t )
{
    return ( 0 == _tcsicmp ( s.m_szData , t.m_szData ) ) ;
}

bool operator == ( const JString & s , const TCHAR * szBuff )
{
    return ( 0 == _tcsicmp ( s , szBuff ) ) ;
}

// Inequality operator. All compares are case insensitive.
bool operator != ( const JString & s , const JString & t )
{
    return ( 0 != _tcsicmp ( s.m_szData , t.m_szData ) ) ;
}

bool operator != ( const JString & s , const TCHAR * szBuff)
{
    return ( 0 != _tcsicmp ( s , szBuff ) ) ;
}

// Assignement operator.
JString & JString :: operator = ( const JString & t )
{
    m_iAllocLen = t.m_iAllocLen ;
    // If t is empty, fix up this.
    if ( 0 == t.m_iBuffLen )
    {
        if ( 0 != m_iBuffLen )
        {
            m_szData[ 0 ] = _T ( '\0' ) ;
            m_pEndPos = m_szData ;
        }
    }
    else
    {

        size_t iTLen = t.Length ( ) ;

        // Is the new string larger than buffer already allocated?
        if ( iTLen >= m_iBuffLen )
        {
            if ( NULL != m_szData )
            {
                Deallocate ( m_szData ) ;
            }

            m_iBuffLen = iTLen + 1 + m_iAllocLen ;
            m_szData = Allocate ( m_iBuffLen * sizeof ( TCHAR ) ) ;
            _tcscpy ( m_szData , t.m_szData ) ;
        }
        else
        {
            // There's enough room in the existing buffer.
            _tcscpy ( m_szData , t.m_szData ) ;
        }
        m_pEndPos = m_szData + iTLen ;
    }
    return ( *this ) ;
}

JString & JString :: operator = ( const TCHAR * szStr )
{
    // If szStr is empty, fix up this.
    if ( ( NULL == szStr ) || ( _T ( '\0' ) == szStr[ 0 ] ) )
    {
        if ( 0 != m_iBuffLen )
        {
            m_szData[ 0 ] = _T ( '\0' ) ;
            m_pEndPos = m_szData ;
        }
    }
    else
    {

        size_t iTLen = _tcslen ( szStr ) ;

        // Is the new string larger than buffer already allocated?
        if ( iTLen >= m_iBuffLen )
        {
            if ( NULL != m_szData )
            {
                Deallocate ( m_szData ) ;
            }

            m_iBuffLen = iTLen + 1 + m_iAllocLen ;
            m_szData = Allocate(m_iBuffLen * sizeof ( TCHAR ) ) ;
            _tcscpy ( m_szData , szStr ) ;
        }
        else
        {
            // There's enough room in the existing buffer.
            _tcscpy ( m_szData , szStr ) ;
        }
        m_pEndPos = m_szData + iTLen ;
    }
    return ( *this ) ;
}

JString & JString :: operator = ( const TCHAR cChar )
{
    if ( 0 == m_iBuffLen )
    {
        m_iBuffLen = m_iAllocLen + 1 ;
        m_szData = Allocate ( m_iBuffLen * sizeof ( TCHAR ) ) ;
    }

    // Replace the data.
    m_szData[ 0 ] = cChar ;
    m_szData[ 1 ] = _T ( '\0' ) ;
    m_pEndPos = m_szData + 1 ;
    return ( *this ) ;
}

#ifdef UNICODE
JString & JString :: operator = ( const char * szAnsiStr )
{
    // If szStr is empty, fix up this.
    if ( ( NULL == szAnsiStr ) || ( '\0' == szAnsiStr[ 0 ] ) )
    {
        if ( 0 != m_iBuffLen )
        {
            m_szData[ 0 ] = _T ( '\0' ) ;
            m_pEndPos = m_szData ;
        }
    }
    else
    {
        size_t iAnsiLen = strlen ( szAnsiStr ) ;

        // Is the new string larger than buffer already allocated?
        if ( iAnsiLen >= m_iBuffLen )
        {
            if ( NULL != m_szData )
            {
                Deallocate ( m_szData ) ;
            }

            m_iBuffLen = iAnsiLen + 1 + m_iAllocLen ;
            m_szData = Allocate ( m_iBuffLen * sizeof ( TCHAR ) ) ;
        }
        if ( 0 == MultiByteToWideChar ( CP_THREAD_ACP     ,
                                        0                 ,
                                        szAnsiStr         ,
                                        -1                ,
                                        m_szData          ,
                                        (int)iAnsiLen + 1  ) )
        {
        }
        m_pEndPos = m_szData + iAnsiLen ;
    }
    return ( *this ) ;
}
#endif  // UNICODE


// Length function.
size_t JString :: Length ( ) const
{
    if ( NULL == m_szData )
    {
        return ( 0 ) ;
    }
    return ( m_pEndPos - m_szData ) ;
}

// cast operator.
JString :: operator const TCHAR * ( ) const
{
    return ( (const TCHAR *) m_szData ) ;
}

// Is empty function.
bool JString :: IsEmpty ( )
{
    return ( m_pEndPos == m_szData ) ;
}

JString operator + ( const JString & s , const JString & t  )
{
    if ( 0 == s.Length ( ) )
    {
        return ( t ) ;
    }
    else if ( 0 == t.Length ( ) )
    {
        return ( s ) ;
    }
    
    JString szNew ;

    if ( s.m_iAllocLen > t.m_iAllocLen )
    {
        szNew.m_iAllocLen = s.m_iAllocLen ;
    }
    else
    {
        szNew.m_iAllocLen = t.m_iAllocLen ;
    }
    
    size_t iSLen = s.Length ( ) ;
    size_t iTLen = t.Length ( ) ;
    
    szNew.m_iBuffLen = iSLen + iTLen + 1 + szNew.m_iAllocLen ;
    szNew.m_szData = Allocate ( szNew.m_iBuffLen * sizeof ( TCHAR ) ) ;
    
    _tcscpy ( szNew.m_szData , s.m_szData ) ;
    szNew.m_pEndPos = szNew.m_szData + iSLen ;
    
    _tcscpy ( szNew.m_pEndPos , t.m_szData ) ;
    szNew.m_pEndPos = szNew.m_pEndPos + iTLen ;
    
    return ( szNew ) ;
}

JString & JString::operator += ( const JString & s )
{
    if ( 0 == s.Length ( ) )
    {
        return ( *this ) ;
    }

    if ( NULL == m_szData )
    {
        *this = s ;
        return ( *this ) ;
    }

    size_t iSLen = s.Length ( ) ;
    size_t iThisLen = Length ( ) ;

    if ( m_iBuffLen > ( iSLen + iThisLen ) )
    {
        _tcscpy ( m_pEndPos , s.m_szData ) ;
        m_pEndPos += iSLen ;
    }
    else
    {
        m_iBuffLen = iThisLen + iSLen + 1 + m_iAllocLen ;

        TCHAR * szBuff = Allocate ( m_iBuffLen * sizeof ( TCHAR ) ) ;
        
        _tcscpy ( szBuff , m_szData ) ;
        m_pEndPos = szBuff + iThisLen ;
        _tcscpy ( m_pEndPos , s.m_szData ) ;
        m_pEndPos += iSLen ;
        
        Deallocate ( m_szData ) ;
        
        m_szData = szBuff ;
    }
    return ( *this ) ;
}

JString & JString::operator += ( const TCHAR * s )
{
    if ( NULL == s )
    {
        return ( *this ) ;
    }

    if ( NULL == m_szData )
    {
        *this = s ;
        return ( *this ) ;
    }

    size_t iSLen = _tcslen ( s )  ;
    size_t iThisLen = Length ( ) ;

    if ( m_iBuffLen > ( iSLen + iThisLen ) )
    {
        _tcscpy ( m_pEndPos , s ) ;
        m_pEndPos += iSLen ;
    }
    else
    {
        m_iBuffLen = iThisLen + iSLen + 1 + m_iAllocLen  ;

        TCHAR * szBuff = Allocate ( m_iBuffLen * sizeof ( TCHAR ) ) ;

        
        _tcscpy ( szBuff , m_szData ) ;
        m_pEndPos = szBuff + iThisLen ;
        _tcscpy ( m_pEndPos , s ) ;
        m_pEndPos += iSLen ;
        
        Deallocate ( m_szData ) ;
        m_szData = szBuff ;
        
        m_szData = szBuff ;
    }
    return ( *this ) ;
}

JString & JString::operator += ( const TCHAR c )
{
    if ( NULL == m_szData )
    {
        m_iBuffLen = m_iAllocLen ;
        m_szData = Allocate ( m_iBuffLen * sizeof ( TCHAR ) ) ;
        m_szData[ 0 ] = c ;
        m_szData[ 1 ] = _T ( '\0' ) ;
        m_pEndPos = m_szData + 1 ;
    }
    else
    {
        size_t iLen = Length ( ) ;

        if ( ( m_iBuffLen - 1 ) > iLen )
        {
            m_szData[ iLen     ] = c ;
            m_szData[ iLen + 1 ] = _T ( '\0' ) ;
            m_pEndPos++ ;
        }
        else
        {
            m_iBuffLen = iLen + 2 + m_iAllocLen ;

            TCHAR * szBuff = Allocate ( m_iBuffLen * sizeof ( TCHAR ));

            _tcscpy ( szBuff , m_szData ) ;
            szBuff[ iLen ] = c ;
            szBuff[ iLen + 1 ] = _T ( '\0' ) ;
            Deallocate ( m_szData ) ;
            m_szData = szBuff ;
            m_pEndPos = m_szData + 1 ;
        }
    }
    return ( *this ) ;
}

#ifdef UNICODE
JString & JString :: operator += ( const char * szAnsiStr )
{
    if ( NULL == szAnsiStr )
    {
        return ( *this ) ;
    }

    if ( NULL == m_szData )
    {
        *this = szAnsiStr ;
        return ( *this ) ;
    }

    size_t iASLen = strlen ( szAnsiStr ) ;
    size_t iThisLen = Length ( ) ;
    
    if ( m_iBuffLen <= ( iASLen + iThisLen ) )
    {
        m_iBuffLen = iThisLen + iASLen + 1 + m_iAllocLen  ;

        TCHAR * szBuff = Allocate ( m_iBuffLen * sizeof ( TCHAR ) ) ;

        _tcscpy ( szBuff , m_szData ) ;
        Deallocate ( m_szData ) ;
        m_szData = szBuff ;
        m_pEndPos = m_szData + iThisLen ;
    }
    
    if ( 0 == MultiByteToWideChar ( CP_THREAD_ACP   ,
                                    0               ,
                                    szAnsiStr       ,
                                    -1              ,
                                    m_pEndPos       ,
                                    (int)iASLen + 1   ) )
    {
    }
    m_pEndPos += iASLen ;
    
    return ( *this ) ;
}
#endif

TCHAR & JString :: operator [] ( int iIndex )
{
    return ( m_szData[ iIndex ] ) ;
}

void JString :: ToLower ( )
{
    if ( NULL != m_szData )
    {
        _tcslwr ( m_szData ) ;
    }
}

void JString :: TrimTrailingWhiteSpace ( void )
{
    if ( NULL != m_szData )
    {
        TCHAR * pEnd = m_pEndPos - 1 ;
        while ( _istspace ( *pEnd ) )
        {
            pEnd-- ;
            if ( pEnd == m_szData )
            {
                pEnd-- ;
                break ;
            }
        }
        pEnd++ ;
        *pEnd = _T ( '\0' ) ;
        m_pEndPos = pEnd ;
    }
}

// Clears any data.
void JString :: Empty ( void )
{
    if ( NULL != m_szData )
    {
        Deallocate ( m_szData ) ;
    }
    m_szData = NULL ;
    m_iBuffLen = 0 ;
    m_pEndPos = m_szData ;
}

// Safely puts a NULL at the beginning of the string.
void JString :: NullString ( void )
{
    if ( NULL != m_szData )
    {
        m_szData[ 0 ] = _T ( '\0' ) ;
    }
    m_pEndPos = m_szData ;
}

// A printf style filler.
int JString :: Format ( const TCHAR * szFmt , ... )
{
    if ( m_iBuffLen < 1024 )
    {
        TCHAR * szTempData = Allocate ( 1024 * sizeof ( TCHAR ) ) ;

        if ( NULL != m_szData )
        {
            Deallocate ( m_szData ) ;
        }
        m_szData = szTempData ;
        m_iBuffLen = 1024 ;
    }

    va_list marker ;

    va_start ( marker, szFmt ) ;

    int iRet = _vstprintf ( m_szData , szFmt , marker ) ;

    va_end( marker );

    return( iRet );
}

#ifndef _PORTABLE_JSTRING
BOOL JString :: LoadString ( UINT uiResID )
{
    // If the buffer sitting around is less that 255 characters, get
    // rid of it.
    if ( ( ( m_iBuffLen < 255 ) && ( NULL != m_szData ) ) ||
         ( NULL == m_szData                             )    )
    {
        if ( NULL != m_szData )
        {
            Deallocate ( m_szData ) ;
        }
        m_iBuffLen = 255 ;
        m_szData = Allocate ( m_iBuffLen * sizeof ( TCHAR ) ) ;
    }

    // Do it!
    int iRet = ::LoadString ( JfxGetApp()->GetResourceInstance ( ) ,
                              uiResID                              ,
                              m_szData                             ,
                              (int)m_iBuffLen                       ) ;
    if ( ( 0 == iRet ) && ( NULL != JfxGetApp()->m_hInstMain ) )
    {
        iRet = ::LoadString ( JfxGetApp()->m_hInstMain  ,
                              uiResID                   ,
                              m_szData                  ,
                              (int)m_iBuffLen            ) ;
    }
    return ( iRet ) ;
}
#endif  // _PORTABLE_JSTRING

void JString :: SetBufferLen ( size_t iNewSize )
{
    if ( m_iBuffLen < iNewSize )
    {
        TCHAR * szBuff = Allocate ( iNewSize * sizeof ( TCHAR ) ) ;
        _tcsncpy ( szBuff , m_szData , m_iBuffLen * sizeof ( TCHAR ) ) ;
        m_iBuffLen = iNewSize ;
        if ( NULL != m_szData )
        {
            Deallocate ( m_szData ) ;
        }
        m_szData = szBuff ;
    }
}

TCHAR * JString :: GetBuffer ( void )
{
    return ( m_szData ) ;
}

void JString :: DeallocateBuffer ( TCHAR * szPreviousBuffer )
{
    Deallocate ( szPreviousBuffer ) ;
}

int JString :: SetMinumumAllocationLength ( int iNewLen )
{
    int iRet = m_iAllocLen ;
    m_iAllocLen = iNewLen ;
    return ( iRet ) ;
}

BOOL JString :: DoNotDeleteBuffer ( bool bDeleteBuffer )
{
    BOOL bRet = m_bDeleteBufferOnDestruct ;
    m_bDeleteBufferOnDestruct = bDeleteBuffer ;
    return ( bRet ) ;
}
