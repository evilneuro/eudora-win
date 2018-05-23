/*////////////////////////////////////////////////////////////////////////////

NAME:
	CVolumeBar - 

FILE:		VolumeBar.h
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	Derived from CDialogBar.

RESCRITICTIONS:
	Needs Dialog Resource, here it is IDD_VOLUME

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/13/97   lss     -Initial

/*////////////////////////////////////////////////////////////////////////////
#ifndef _VOLUMEBAR_H_
#define _VOLUMEBAR_H_

#include "LsMixer.h"

class CFrameWnd;

/////////////////////////////////////////////////////////////////////////////
// CVolumeBar Dialog Bar

class CVolumeBar : public CDialogBar
{
public:
//	LIFECYCLE
	CVolumeBar();
	virtual ~CVolumeBar();

	BOOL Create( 
			CWnd* pParentWnd,
			UINT nStyle = CBRS_RIGHT|CBRS_TOOLTIPS,
			UINT nID = IDD_VOLUME,
			UINT nIDTemplate = IDD_VOLUME );

//	OPERATIONS
	void Show( BOOL bShow = TRUE );

//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
	void setup();
	BOOL convertBmp2Transparent( UINT nIDResource, CBitmap& destBitmap );

//	ATTRIBUTES
	CFrameWnd*			mp_MainFrame;

	BOOL				mb_Show;

	CSliderCtrl		    m_VolOutSld;	// playback volume slider
	CSliderCtrl			m_VolInSld;		// record gain slider
	LsMixerVolume		m_VolOut;
	LsMixerVolume		m_VolIn;

	int					m_OldVolOut;
	int					m_OldVolIn;

	CBitmap				m_bmpSpeaker;
	CBitmap				m_bmpMike;

//////////////////////////////////////////////////////////////////////////////
// ClassWizard stuff
//////////////////////////////////////////////////////////////////////////////
public:
	CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVolumeBar)
	//}}AFX_VIRTUAL
	// Generated message map functions
protected:
	//{{AFX_MSG(CVolumeBar)
	afx_msg LRESULT OnMixerControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
