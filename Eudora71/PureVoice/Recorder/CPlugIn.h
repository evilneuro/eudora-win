/*////////////////////////////////////////////////////////////////////////////

NAME:
	CPlugIn - PlugIn Class for Codec PlugIn 

FILE:		CPlugIn.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
    CPlugIn is essentially a "wrapper" for Codec Plugin DLL.
    It will search for codec plugins in a specified directory and
    maintain a list of them. 

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.

FILES:
    CACODEC.H
    QPL.H

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/09/97   lss	   -Initial

/*////////////////////////////////////////////////////////////////////////////
#ifndef _CPLUGIN_H_
#define _CPLUGIN_H_

// PROJECT INCLUDES
#ifndef _CACodec_H_
#include "CACodec.h"
#endif
#ifndef _QPL_H_
#include "QPL.H"
#endif

typedef HRESULT (WINAPI *type_pfn_QPLCreateInstance)( REFIID, void**);

class CPlugIn
{
public:
//  LIFECYCLE
	CPlugIn( LPCTSTR dir = NULL, LPCTSTR ext = "dll" );
	~CPlugIn();
	// Copy ctor or assignment operator not allowed

//  OPERATIONS
	int			findCodecs();	// need to call this before calling rest
	
//  ACCESS
	// set search path for codec plugins and extension of plugins
    BOOL		setPath( LPCTSTR dir = NULL, LPCTSTR ext = "dll" );
	int			setIndex( int nIndex = 0 );
	int			getIndex();
	int			getIndex( REFGUID id );
	int			getIndex( const t_qpl_codec_info& codecInfo );
	int			getNumOfCodecs();
	CACodec*	getCodec( int nIndex );
	CACodec*	getCodec();
	CACodec*	getCodec( const t_qpl_codec_info& codecInfo );
	LPCTSTR		getCodecName( int nIndex );
	LPCTSTR		getCodecName();
//	LPCTSTR		getCodecMenuName( int nIndex );
//	LPCTSTR		getCodecDescription( int nIndex );

	const t_qpl_codec_info*       getCodecInfo( int nIndex );
	const t_qpl_codec_info*       getCodecInfo();

//  INQUIRY
	BOOL		isExistCodec( const t_qpl_codec_info& codecInfo, BOOL bSetIndex=FALSE );

//////////////////////////////////////////////////////////////////////////////
private:
//  METHODS
	int			findDllNames();
	CString&	_getCodecName( LPCTSTR dllname, int index );
	void		deleteCodecDllArray();
//	LPCTSTR		getDllName( int nIndex );

	BOOL		WITHIN(int a, int x, int y){return ((a>=x) && (a<=y));};

//  ATTRIBUTES
	CString			ms_codecName;
	CString			ms_searchStr;
	CStringArray	m_dllNameArray;
	CPtrArray		m_codecDllArray;

	int				mi_codecDllArrayIndex;
	
private:
	// Do not allow copy ctor or assignment
	CPlugIn( const CPlugIn& );
	CPlugIn& operator=( const CPlugIn& );

	struct cCodecDllName
	{
		cCodecDllName( HINSTANCE hDll,
					   int dup,
					   IQPLCodec* pObj,
					   t_qpl_codec_info& codecInfo,
					   t_qpl_codec_gui_info& codecGuiInfo
					   )
		{
			mh_Dll = hDll;
			m_dup = dup;
			mp_IQPLCodec = pObj;
			memcpy(&m_codecInfo, &codecInfo, sizeof(t_qpl_codec_info));
			memcpy(&m_codecGuiInfo, &codecGuiInfo, sizeof(t_qpl_codec_gui_info));
		}
		~cCodecDllName() {mp_IQPLCodec->Release(); 
							if(0 == m_dup) FreeLibrary(mh_Dll);}

		HINSTANCE					mh_Dll;
		int							m_dup;
		IQPLCodec*					mp_IQPLCodec;
		t_qpl_codec_info			m_codecInfo;
		t_qpl_codec_gui_info		m_codecGuiInfo;
	};
};

#endif