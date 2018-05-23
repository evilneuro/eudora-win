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
//  Description:	Implementation of SECTNView -ThumbNail View Class
//  Created:		Tue Jul 25 23:10:29 1995
//

#ifndef __SECTNVW_H__
#define __SECTNVW_H__


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

class SECTNBitmap;
class SECTNDC;

class SECTNView : public CView
{
    friend class SECTNDocument;
    
protected:
    SECTNView();           // protected constructor used by dynamic creation
    DECLARE_DYNAMIC(SECTNView);
    
// Attributes
public:
    BOOL m_bThumbNailing;
    BOOL IsThumbNailing() {return m_bThumbNailing;};
    
// Operations
public:
    
// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SECTNView)
protected:
    virtual void OnDraw(CDC* pDC) = 0;      // pass along pure virtual
    
    //}}AFX_VIRTUAL
    
    //Called by SECTNDialog to get thumbnail   
    virtual SECTNBitmap * GetThumbNailBitmap();
    
    //Override OnPrepareThumbNail to set size of thumbnail
    virtual void OnPrepareThumbNail(CSize * pSize);
    
    //Override to manipulate TNDC for special thumbnail DC objects.
    virtual void OnThumbNail(SECTNDC * pDC);
    
// Implementation
protected:
    virtual ~SECTNView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    
    // Generated message map functions
protected:
    //{{AFX_MSG(SECTNView)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP();
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif //__SECTNVW_H__


