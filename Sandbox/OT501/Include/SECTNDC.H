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
//  Description:	Declarations for SECTNDC the ThumbNail DC class
//  Created:		Tue Jul 25 23:13:22 1995
//

#ifndef __SECTNDC_H__
#define __SECTNDC_H__

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

class SECTNDC : public CDC
{
    DECLARE_DYNAMIC(SECTNDC);
    
// Constructors
public:
    SECTNDC(); 
    void Create(CSize * pSize); //The size of the bitmap
    
// Operations
    BOOL IsThumbNailing() { return TRUE;}; //This DC is a thumbnailing machine.
    
// Attributes
public:
    SECTNBitmap * m_pTNBitmap;   //A serializeable thumbnail bitmap, kicks ass!
    CBitmap     * m_pOldBitmap;  //Store any old bitmap from select
    
// Implementation
public:
    virtual ~SECTNDC();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR


#endif // __SECTNDC_H__
