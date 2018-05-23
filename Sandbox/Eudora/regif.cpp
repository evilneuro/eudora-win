#include "stdafx.h"

#include "regif.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// Interface logic for accessing the Registry

REGIF::REGIF( void )
{
	m_hKeyRoot = NULL;
	*m_cStrRoot = '\0';
}

REGIF::~REGIF( void )
{
	if ( m_hKeyRoot )
		RegCloseKey( m_hKeyRoot );
}

BOOL REGIF::Attach( LPCSTR pKey )
{
	LONG	status;
	HKEY	hKeyRoot;

	if ( m_hKeyRoot )
		RegCloseKey( m_hKeyRoot );

	// strip off the Microsoft pre-defined key
	if ( strncmp( pKey, "HKEY_CLASSES_ROOT\\", strlen("HKEY_CLASSES_ROOT\\") ) == 0 )
	{
		hKeyRoot = HKEY_CLASSES_ROOT;
		pKey += strlen("HKEY_CLASSES_ROOT\\");
	}
	else if ( strncmp( pKey, "HKEY_CURRENT_USER\\", strlen("HKEY_CURRENT_USER\\") ) == 0 )
	{
		hKeyRoot = HKEY_CURRENT_USER;
		pKey += strlen("HKEY_CURRENT_USER\\");
	}
	else if ( strncmp( pKey, "HKEY_LOCAL_MACHINE\\", strlen("HKEY_LOCAL_MACHINE\\") ) == 0 )
	{
		hKeyRoot = HKEY_LOCAL_MACHINE;
		pKey += strlen("HKEY_LOCAL_MACHINE\\");
	}
	else if ( strncmp( pKey, "HKEY_USERS\\", strlen("HKEY_USERS\\") ) == 0 )
	{
		hKeyRoot = HKEY_USERS;
		pKey += strlen("HKEY_USERS\\");
	}
	else
		hKeyRoot = HKEY_LOCAL_MACHINE;
		
#ifdef WIN32
	DWORD	dwDisposition;
	
	status = RegCreateKeyEx( hKeyRoot,
							 pKey,
							 0L,
							 "REG_SZ",
							 REG_OPTION_NON_VOLATILE,
							 KEY_ALL_ACCESS,
							 NULL,
							 &m_hKeyRoot,
							 &dwDisposition );
#else
	hKeyRoot = HKEY_CLASSES_ROOT;	// the only allowed root
	
	status = RegCreateKey( hKeyRoot,
						   pKey,
						   &m_hKeyRoot );
#endif

	if ( status != ERROR_SUCCESS )
	{
		m_hKeyRoot = NULL;
		*m_cStrRoot = '\0';
	}
	else
	{
		strcpy( m_cStrRoot, pKey );
	}

	return( m_hKeyRoot != NULL );
}

BOOL REGIF::Read( LPCSTR pName, LPSTR pVal, DWORD dwValLen )
{
	LONG	status;

	if ( ! m_hKeyRoot )
		return FALSE;

#ifdef WIN32
	DWORD	dwType;
	
    status = RegQueryValueEx( m_hKeyRoot, pName, 0L, &dwType,
					(LPBYTE)pVal, &dwValLen );
#else
    status = RegQueryValue( m_hKeyRoot, pName,
					pVal, (LONG FAR *)&dwValLen );
#endif

	return ( status == ERROR_SUCCESS );
}

// adds a value (name=data) to the current key
BOOL REGIF::Write( LPCSTR pName, LPCSTR pData )
{
	LONG	status;

	if ( ! m_hKeyRoot )
		return FALSE;

	// write the path/delay pairs out
#ifdef WIN32
	status = RegSetValueEx( m_hKeyRoot,
			   				pName,
			   				0L,
			   				REG_SZ,
			   				(const BYTE *)pData,
			   				strlen( pData ) + 1 );
#else
	status = RegSetValue( m_hKeyRoot,
			   			  pName,
			   			  REG_SZ,
			   			  (const char *)pData,
			   			  strlen( pData ) + 1 );
#endif

	return ( status == ERROR_SUCCESS );
}

