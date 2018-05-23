// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:		    AAB
//  Description:	Declarations for Compress-File (SECCompress)
//  Created:		Jan 15, 1996
//
//  CHANGELOG:
//     AAB	1/15/96		Original creation based on CCryptoFile architecture
//
//

#ifndef __SECCOMPRESS_FILE_H__
#define __SECCOMPRESS_FILE_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL


#include "compeng.h"

/////////////////////////////////////////////////////////////////////////////
// SECCompressFile

class SECCompressFile : public CFile 
{
public: 
// enum 	eDir { ENC, DEC };
// enum 	eMode { ECB, OFB };

	SECComp m_Comp;
	gzFile	m_gzFile; 
#ifndef WIN32
	BOOL	m_bReadHuge;	// flag when reading huge vals (WIN3.1 only)
#endif
// Construction
		SECCompressFile(  LPCTSTR lpszFileName, 
					UINT 		nOpenFlags );
		SECCompressFile( HANDLE hFile );
	
// Operations
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);

	UINT Read( void FAR *pBuf, UINT nCount );
	void Write( const void FAR *pBuf, UINT nCount );
	
	void Close( void );
	void SetCompressMode(BOOL bState);
	BOOL GetCompressMode();

protected:
// Attributes
	BOOL m_bCompressMode;

};	

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

#endif // __SECCOMPRESS_FILE_H__
