/*////////////////////////////////////////////////////////////////////////////

NAME:
	CPlugIn - PlugIn Class Implemenation for Codec PlugIn 

FILE:		CPLUGIN.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc
TYPE:		C++-CLASS

DESCRIPTION:
    CPlugIn is essentially a "wrapper" for Codec Plugin DLL.
    It will search for codec plugins in a specified directory and
    maintain a list of them. 

RESTRICTIONS:

FILES:
	CPLUGIN.H

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     -Initial
01/03/97   lss     -Support for multiple codecs within one plugin (dll)
01/06/97   lss     -Added type_lcp_info and type_lcp_gui_info	
				    to cCodecDllName struct.
				   -Added _getCodecInfos()

01/16/97   lss     -Added getIndex() using codec ID
02/21/97   lss     -Added 2nd parameter LPCTSTR ext to CPlugin and setPath
04/09/97   lss     -
  
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#define INITGUID
#include "QPL.h"

// LOCAL INCLUDES
#include "CPlugIn.h"

#include "DebugNewHelpers.h"

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CPlugIn::CPlugIn( LPCTSTR dir /* = NULL */, LPCTSTR ext /* = "dll" */  ) :
	mi_codecDllArrayIndex(0)
{
	setPath( dir, ext );
}

CPlugIn::~CPlugIn()
{
	m_dllNameArray.RemoveAll();
	deleteCodecDllArray();
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

/*\===========================================================================
NAME:
	findCodecs - Search specified directory for codec plugins and
    put the names in a list for future use.

PARAMTERS:      None

DESCRIPTION:
    findCodecs finds codec plugins in a specified directory. This
    directory can be defined by calling setPath. If not, findCodecs
    will search in the directory where this executable is found.
    findCodecs must be called before using other functions.

RESTRICTIONS:   None

ERRORS:         None

RETURN:
    Number of codec plugins found.  
===========================================================================\*/
int CPlugIn::findCodecs()
{
	deleteCodecDllArray();
	int numOfDll = findDllNames();
	type_pfn_QPLCreateInstance pfn = NULL;
	IQPLGetCodec* pObjList = NULL;
	IQPLCodec* pObjCodec = NULL;
	t_qpl_codec_info	codecInfo;
	t_qpl_codec_gui_info codecGuiInfo;
	HINSTANCE hDll;

	for (int i = 0; i < numOfDll; i++)
	 {

		hDll = LoadLibrary( m_dllNameArray[i] ); if ( NULL == hDll ) continue;

		pfn = 
		(type_pfn_QPLCreateInstance) GetProcAddress(hDll, "QPLCreateInstance");

		if ( pfn ) 
		 {
			if ( S_OK != (*pfn)( IID_IQPLGetCodec, (void**)&pObjList ) )
				continue;
		 }
		else
		 {
			continue;
		 }
		int nCodecs = pObjList->NumOfCodecs()-1;
		for (int j = 0; j <= nCodecs; j++)
		 {
			if ( S_OK == pObjList->CreateCodec( j, (void**)&pObjCodec ) )
			 {
				pObjCodec->Info( &codecInfo, &codecGuiInfo );
				m_codecDllArray.Add(
				new cCodecDllName( hDll, nCodecs-j, pObjCodec, codecInfo, codecGuiInfo) );
			 }
		 }
		pObjList->Release();
	 }

	return m_codecDllArray.GetSize();
}

//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

/*\===========================================================================
NAME:
	setPath - Sets the path for codec plugin search

PARAMTERS:      
    dir         the path to search for plugins (NULL by default)
	ext			the extension of plugins ("dll" by default)

DESCRIPTION:
    setPath sets the path to 'dir' for codec plugin search by findCodecs.
    This is also called by ctor to set default search directory.

RESTRICTIONS:   None

ERRORS:         None

RETURN:
    TRUE
===========================================================================\*/
BOOL CPlugIn::setPath( LPCTSTR dir /* = NULL */, LPCTSTR ext /* = "dll" */ )
{
	CString dirName = dir;

	if (dir == NULL)
	 {
		TCHAR szBuff[_MAX_PATH];
		VERIFY(::GetModuleFileName(AfxGetInstanceHandle( ),
														szBuff, _MAX_PATH));
		dirName = szBuff;
		dirName = dirName.Left (dirName.ReverseFind( '\\' ));
	 }

	ms_searchStr = dirName + "\\*." + ext;	

	return TRUE;
}

// Sets the current index of codec plugin list to 'nIndex'.
// Returns the previous index.
int	CPlugIn::setIndex( int nIndex /*= 0*/ )
{
	int tmp = mi_codecDllArrayIndex;

	if (!WITHIN(nIndex, 0, getNumOfCodecs()))
		return -1;
	mi_codecDllArrayIndex = nIndex;	

	return tmp;
}

// Returns the index of codec plugin in the list with matching info 'codecInfo'.
int	CPlugIn::getIndex( const t_qpl_codec_info& codecInfo )
{
	for ( int i = 0; i < getNumOfCodecs(); ++i )
	 {
		if ( codecInfo.Id == getCodecInfo(i)->Id )
		 {
			return i;
		 }
	 }
	return -1;
}

// Returns the index of codec plugin in the list with matching codec ID.
int	CPlugIn::getIndex( REFGUID cid )
{
	for ( int i = 0; i < getNumOfCodecs(); ++i )
	 {
		if ( cid == getCodecInfo(i)->Id )
		 {
			return i;
		 }
	 }
	return -1;
}

// Returns the current index of our codec plugin list.
int	CPlugIn::getIndex()
{
	if (getNumOfCodecs())
		return mi_codecDllArrayIndex;
	return -1;
}

// Returns the number of codecs available (# entries in our list).
int CPlugIn::getNumOfCodecs()
{
	return m_codecDllArray.GetSize();
}

// Return nIndex CACodec* in the our list
CACodec* CPlugIn::getCodec( int nIndex )
{
	return new CACodec(
		((cCodecDllName*) m_codecDllArray[nIndex])->mp_IQPLCodec );
}

// Return current indexed CACodec* in the our list
CACodec* CPlugIn::getCodec()
{
	return getCodec( mi_codecDllArrayIndex );
}

// Search through available codecs in our list and return the matching one
// else return NULL.
CACodec* CPlugIn::getCodec( const t_qpl_codec_info& codecInfo )
{
	int nIndex = -1;
	//findCodecs();
	for ( int i = 0; i < getNumOfCodecs(); ++i )
	 {
		if ( codecInfo.Id == getCodecInfo(i)->Id )
			nIndex = i;
	 }
	if ( nIndex >= 0 )
		return new CACodec(
			((cCodecDllName*) m_codecDllArray[nIndex])->mp_IQPLCodec );
	else
		return NULL;
}

// Return the name of the codec with index of 'nIndex' in our codec
// plugin list.
LPCTSTR	CPlugIn::getCodecName( int nIndex )
{
	if (!WITHIN(nIndex, 0, getNumOfCodecs()))
		return NULL;

	return LPCTSTR(
		((cCodecDllName*)m_codecDllArray[nIndex])->m_codecGuiInfo.menuName);
}

// Return the name of the codec with the current index.
LPCTSTR	CPlugIn::getCodecName()
{
	return getCodecName( mi_codecDllArrayIndex );
}

// Return t_qpl_codec_info* of the codec with index of 'nIndex'.
const t_qpl_codec_info* CPlugIn::getCodecInfo( int nIndex ) 
{
	if (!WITHIN(nIndex, 0, getNumOfCodecs()))
		return NULL;
	
	return &((cCodecDllName*) m_codecDllArray[nIndex])->m_codecInfo;
}

// Return t_qpl_codec_info* of the codec with the current index.
const t_qpl_codec_info* CPlugIn::getCodecInfo() 
{
	return getCodecInfo( mi_codecDllArrayIndex );
}

//////////////////////////////////////////////////////////////////////////////
// INQUIRY
//////////

// Return TRUE if codec with codec info of 'codecInfo' exists in our list. If
// not, then return FALSE. If bSetIndex = TRUE then set the current Index
// to this codec.
BOOL CPlugIn::isExistCodec(
	const t_qpl_codec_info& codecInfo,
	BOOL bSetIndex /*= FALSE*/)
{
	//findCodecs();
	for ( int i = 0; i < getNumOfCodecs(); ++i )
	 {
		if ( codecInfo.Id == getCodecInfo(i)->Id )
		 {
			if (bSetIndex) setIndex( i );
			return TRUE;
		 }
	 }
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// FOR YOUR EYES ONLY ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CPlugIn::deleteCodecDllArray()
{
	for (int n = 0; n < m_codecDllArray.GetSize(); n++)
	 {
		delete (cCodecDllName*) m_codecDllArray[n];
	 }
	m_codecDllArray.RemoveAll();
}

int CPlugIn::findDllNames()
{
	int	numOfDll = 0;
	WIN32_FIND_DATA wfd;
	HANDLE hFile = FindFirstFile( LPCSTR(ms_searchStr), &wfd );

	m_dllNameArray.RemoveAll();
	hFile = FindFirstFile( LPCSTR(ms_searchStr), &wfd );
	if ( hFile == INVALID_HANDLE_VALUE )
		return 0;
	do {
		CString fileName(wfd.cFileName);
		fileName = ms_searchStr.Left (ms_searchStr.ReverseFind( '\\' )+1) + fileName;
		m_dllNameArray.Add( LPCTSTR(fileName) );
		numOfDll++;
	} while ( FindNextFile( hFile, &wfd ) );
	FindClose( hFile );

	return numOfDll;
}

/*
LPCTSTR CPlugIn::getDllName( int nIndex )
{
	if (!WITHIN(nIndex, 0, getNumOfCodecs()))
		return NULL;

	return LPCTSTR(((cCodecDllName*) m_codecDllArray[nIndex])->ms_dllName);
}
*/