/*////////////////////////////////////////////////////////////////////////////

NAME:
	CLsBmpButton - Transparent Bitmap Button Class

FILE:		LsBmpButton.h
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	Derived from CButton.

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/30/97   lss     -Initial

/*////////////////////////////////////////////////////////////////////////////

#ifndef _LSBMPBUTTON_H_
#define _LSBMPBUTTON_H_

/////////////////////////////////////////////////////////////////////////////
// CLsBmpButton window

class CLsBmpButton : public CButton
{
public:
//	LIFECYCLE
	CLsBmpButton();
	virtual ~CLsBmpButton();

//	OPERATIONS
	BOOL AutoLoad( UINT nID, CWnd* pParent, int borderStyle = BORDER_NONE );
	void SizeToContent();

//	ENUM
	enum {
		BORDER_NONE,
		BORDER_UP,
		BORDER_DOWN
	};

//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
	void drawBorder( CDC& dc, CRect& rc, int borderStyle );

	BOOL makeOtherBitmap(
		LPCTSTR		  bitmapResource,
		CBitmap&      destBitmap,
		BOOL          bTransParent,
		int			  borderStyle
	);

//	ATTRIBUTES
	int			m_borderStyle;
	COLORREF	m_transparentColor;

	CSize		m_sizeBtn;
	CBitmap		m_bmpBtnUp;
	CBitmap		m_bmpBtnDn;
	CBitmap		m_bmpBtnFc;
	CBitmap		m_bmpBtnDs;

//////////////////////////////////////////////////////////////////////////////
// ClassWizard and other windows stuff
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLsBmpButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CLsBmpButton)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
