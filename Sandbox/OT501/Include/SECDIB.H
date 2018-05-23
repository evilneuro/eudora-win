// Stringray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved.
//
// SEC DIB file format header
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG: 
//
//    AAB	7/22/95	Started
//    MSW       9/30/95 Code review
//
//

#ifndef __SECDIB_H__
#define __SECDIB_H__

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


class SECDib: public SECImage
{
    DECLARE_SERIAL(SECDib)
	
// Construction
public:
    SECDib();

// Implementation
protected:
    LPBITMAPFILEHEADER  m_lpbmfHdr;

    virtual BOOL DoSaveImage(CFile* pFile);
    virtual BOOL DoLoadImage(CFile* pFile);

public:    
    void Serialize(CArchive& ar);
    virtual  ~SECDib();
#ifdef _DEBUG
    void AssertValid() const;
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

#endif // __SECDIB_H__

