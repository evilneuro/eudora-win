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
//  Description:	Declaration of SECTNDocument - The ThumbNail document class
//  Created:		Wed Jul 26 13:15:44 1995
//

#ifndef __SECTNDOC_H__
#define __SECTNDOC_H__


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

class SECTNView;

class SECTNDocument : public CDocument
{
protected:
	SECTNDocument();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(SECTNDocument)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECTNDocument)
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL
    //Override to specify a certain view for thumbnailing
    virtual SECTNView * GetThumbNailView();
// Implementation
public:
	virtual ~SECTNDocument();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(SECTNDocument)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif //__SECTNDOC_H__
