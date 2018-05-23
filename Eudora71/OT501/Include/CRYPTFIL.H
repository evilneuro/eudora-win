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
//  Author:		    PCB
//  Description:	Declarations for Crypto-File (SECCryptoFile)
//  Created:		Apr 4, 1995
//
//  CHANGELOG:
//     PCB          12/1/95    SEC++ Accomodations
//
//


#ifndef __CRYPTOFIL_H__
#define __CRYPTOFIL_H__

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


#include "blackbox.h"

/////////////////////////////////////////////////////////////////////////////
// SECCryptoFile

class SECCryptoFile : public CFile 
{
public:
enum 	eDir { ENC, DEC };
enum 	eMode { ECB, OFB };


// Construction
		SECCryptoFile( const TCHAR 	*pszFileName, 
					const TCHAR 	*pszPassword, 
					UINT 		nOpenFlags,
					eMode		CipherMode );
		SECCryptoFile( HANDLE hFile, const TCHAR *pszPassword, eMode CipherMode );
	
// Operations
	BOOL Open( 	const TCHAR *pszFileName,
				const TCHAR *pszPassword, 
				UINT nOpenFlags,
	 			eMode CipherMode,
				CFileException *p = NULL );
	
	UINT Read( void FAR *pBuf, UINT nCount );
	void Write( const void FAR *pBuf, UINT nCount );
	
	ULONGLONG Seek( long lOff, UINT nFrom );
	void SeekToBegin( void );
	void SeekToEnd( void );

protected:
// Attributes
	SECCryptoBlackBox m_bb;
};	


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR


#endif // __CRYPTOFIL_H__
