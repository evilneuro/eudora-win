/////////////////////////////////////////////////////////////////////////////
// 	File: PrintEdit.h
//
//  Purpose:  This file provides the declarations for the huge
//			edit view class.  This class is subclassed from the
//			CTerView class...which provides these huge edit regions
//			This class also provides print and print preview 
//			functions, as well as standard headers/footers for the
//			document being printed
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __PRINT_EDIT_H_
#define __PRINT_EDIT_H_


#include "mdichild.h"
#include "font.h"

typedef void (*SetPrintingPreviewingFlagCB)(void *, bool);
typedef unsigned long	ULONG;

const	short	kMaxPages	= 1000;

class DSPrintEditPrintInfo 
{
public:
	CRect	m_theBodyRect;
	CFontInfo*	m_pPrinterFont;
	CFontInfo*	m_pHeaderFont;
	CPen*	m_pPen;
	CRect	m_HeaderTextRect;
	CRect	m_FooterTextRect;
	LONG	m_lPenWidth;
	UINT	m_uOldMappingMode;   
	ULONG	m_ulBufferSize;
};

/////////////////////////////////////////////////////////////////////////////
// DSPrintEditView view


class DSPrintEditView	: public CEditView
{
	DSPrintEditPrintInfo m_thePrintInfo;

	DECLARE_SERIAL(DSPrintEditView)

public:
	DSPrintEditView();			
 	~DSPrintEditView();
	
	void Serialize(CArchive& ar);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo);
                void SetCB(SetPrintingPreviewingFlagCB, void *);

//	Member Variables
protected: 
	//	================================================================================
	// Printer Specific member variables
	BOOL	m_includeHeaderFooters;     // Should a header/footer be done
	
	//	================================================================================

	//{{AFX_MSG(DSPrintEditView)
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintOne();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	//	================================================================================
	// Printer specific Functions
	// overridden -- or -- inherited functions
	virtual	void	OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual	void	OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual	void	OnPrint(CDC* pDC, CPrintInfo* pInfo);
			UINT	PrintInsideRect(CDC* pDC, RECT& rectLayout, UINT nIndexStart, UINT nIndexStop);

	
	// Our functions  
	virtual	void	DoPageHeader(CDC* pDC, CPrintInfo* pInfo);
	virtual	void	DoPageFooter(CDC* pDC, CPrintInfo* pInfo);
    
	
	// The header and footer text functions provide the actual text in the
	// header or footer...you should override these to provide different text..
    virtual void	GetHeaderText(char* theText);
    virtual	void	GetFooterText(char* theText);

private:
    void                        *pvUserData;
    SetPrintingPreviewingFlagCB  sppfCB;
};
 
/////////////////////////////////////////////////////////////////////////////


class DSPrintTextView : public DSPrintEditView
{
	DECLARE_DYNCREATE(DSPrintTextView)

//	Member Variables
protected: 
	// This object puts the path in the header...
   virtual void	GetHeaderText(char *theText);
};


/////////////////////////////////////////////////////////////////////////////

#endif
