#ifndef __SECAUX_H__
#define __SECAUX_H__

#define CX_BORDER       1
#define CY_BORDER       1

/////////////////////////////////////////////////////////////////////////////
// OT Auxiliary System/Screen metrics

struct SEC_AUX_DATA
{
	// system metrics
	int cxBorder2, cyBorder2;

	// color values of system colors used for CToolBar
	COLORREF clrBtnFace, clrBtnShadow, clrBtnHilite;
	COLORREF clrWindowFrame, clr3DLight, clr3DDkShadow;
	COLORREF clrText, clrWindow, clrHighlight, clrHighlightText;

	// standard cursors
	HCURSOR hcurArrow;
	HCURSOR hcurHSplit, hcurVSplit;

	// other system information
	BOOL    bWin4;          // TRUE if Windows 4.0

// Implementation
	SEC_AUX_DATA();
	~SEC_AUX_DATA();
	void UpdateSysColors();
};

/////////////////////////////////////////////////////////////////////////////
// OT General purpose 

// Useful for parsing changed style bits to feed to ModifyStyle functions
#define SEC_BITS_ADDED(oldVal,newVal) ((oldVal|newVal)^oldVal)
#define SEC_BITS_REMOVED(oldVal,newVal) ((oldVal^newVal)&oldVal)


#endif // __SECAUX_H__
