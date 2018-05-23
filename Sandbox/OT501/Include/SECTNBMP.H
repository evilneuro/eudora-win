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
//  Author:		Scot Wingo
//  Description:	Declarations for SECTNBitmap
//  Created:		Thu Jul 20 21:08:46 1995
//

#ifndef __SECTNBITMAP_H__
#define __SECTNBITMAP_H__

// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL



//SECTMBitmap = SECBitmap + Serialization.

class SECTNBitmap : public CBitmap
{

protected: //Create from serialization only.
    DECLARE_DYNCREATE(SECTNBitmap) 
    SECTNBitmap();
    
// Implementation
public:
    virtual ~SECTNBitmap();
    virtual void Serialize(CArchive& ar);
#ifdef _DEBUG
    virtual void Dump(CDumpContext& dc) const;    
#endif

// DIB Helper functions
public:
    HANDLE CreateDIB(void);
    HBITMAP CreateDDBFromDIB(HANDLE);
    WORD FAR PaletteSize(LPTSTR lpDIB);
    WORD FAR DIBNumColors(LPTSTR lpDIB);
    LPTSTR FAR FindDIBBits(LPTSTR lpDIB);
    BOOL CreateDIBPalette(HANDLE,CPalette*);
    void PaintDIB(HANDLE);
    
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR


#endif // __SECTNBITMAP_H__
