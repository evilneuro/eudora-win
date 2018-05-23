// Bstr.cpp: implementation of the CBstr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Bstr.h"
#include "rs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBstr::CBstr()
{
	m_bstr = NULL;
}

CBstr::CBstr( 
LPCSTR	szText)
{
	USES_CONVERSION;
	m_bstr = A2BSTR( szText );
}

CBstr::CBstr(
UINT	uStringID )
{
	CRString szText( uStringID );
	
	USES_CONVERSION;
	
	m_bstr = A2BSTR( szText );
}

CBstr::~CBstr()
{
	if( m_bstr != NULL )
	{
		::SysFreeString( m_bstr );
	}
}

BSTR CBstr::operator = ( BSTR b )
{
	if( m_bstr )
	{
		::SysFreeString( m_bstr );
	}
	m_bstr = b; 
	return m_bstr; 
}


BSTR CBstr::operator = ( UINT uStringID )
{
	USES_CONVERSION;

	if( m_bstr )
	{
		::SysFreeString( m_bstr );
	}

	if( uStringID == NULL )
	{
		m_bstr = NULL; 
	}
	else
	{
		CRString szText( uStringID );
		m_bstr = A2BSTR( szText );
	}
	return m_bstr; 
}


BSTR CBstr::operator = ( 
LPCSTR szText )
{
	USES_CONVERSION;

	if( m_bstr )
	{
		::SysFreeString( m_bstr );
	}

	if( szText == NULL )
	{
		m_bstr = NULL; 
	}
	else
	{
		m_bstr = A2BSTR( szText );
	}
	return m_bstr; 
}

BSTR* CBstr::BSTRArg()
{
	if( m_bstr )
	{
		::SysFreeString( m_bstr );
		m_bstr = NULL;
	}

	return &m_bstr; 
}


void CBstr::Release()
{
	if( m_bstr )
	{
		::SysFreeString( m_bstr );
		m_bstr = NULL;
	}
}
