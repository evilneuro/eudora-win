/*////////////////////////////////////////////////////////////////////////////

NAME:		CQVoiceView 

FILE:		QVoiceView.h
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	CQVoiceView is derived from CFormView

RESCRITICTIONS:
	Can only be instantiated by the MFC framework

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/??/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

#ifndef _QVOICEVIEW_H_
#define _QVOICEVIEW_H_

#define LS_BUTTON_TEST

// PROJECT INCLUDES
#include "QVoiceDoc.h"
#include "MainFrm.h"
#include "PlayDisplay.h"
#include "VolumeBar.h"
#include "LsBmpButton.h"

class CQVoiceView : public CFormView
{
protected: // create from serialization only
//	LIFECYCLE
	CQVoiceView();
	DECLARE_DYNCREATE(CQVoiceView)
public:
	virtual ~CQVoiceView();

//	OPERATIONS
	BOOL	UpdateView();

//	ACCESS
	CQVoiceDoc* GetDocument();

//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
	LsStateType		State();
	LsStateType		LastState();
	BOOL			Event( int event, void* param = 0 );
    void            setUpControlButtons();

    BOOL			OnPlayUpdate();
    BOOL			OnPauseUpdate();
	BOOL			OnRecordUpdate();
    BOOL			OnIdleUpdate();
    BOOL			OnOpenFileUpdate();
	BOOL			OnEncodeUpdate();
	BOOL			OnRecordEncodeUpdate();

	void			UpdateProgress();

	void			enableCtrls( int mode );		
	void			disableCtrls( int mode );	
	void			setupProgress( int mode );	// setup progress ctrl
	void			removeProgress( int mode );	// remove the progress ctrl
	
//	ATTRIBUTES
	CVolumeBar		m_VolumeBar;
	CProgressCtrl	*mp_ProgressCtrl;	// for encoding progress
	CMainFrame		*mp_MainFrame;		// pointer to main frame

	CString			m_strDefaultStatusMsg;

	UINT			m_slowTimerId;
	UINT			m_fastTimerId;
	UINT			m_startRecordTimerId;	// id of timer for /record

	UINT			m_timeIndex;			// time loop for updating display
	UINT			m_animIndex;

	long			m_curTotTime;

	BOOL			mb_OneTime;

	BOOL			mb_OpenEnable;
	BOOL			mb_SaveEnable;
	BOOL			mb_NewEnable;
    BOOL            mb_Pause;

	LsStateAction*	mp_Action;
	LsStateEvent*	mp_Event;

	LsMixerVolumeMeter	m_VolOutMeter;	// peak meter for speaker
	LsMixerVolumeMeter	m_VolInMeter;	// peak meter for mic
	
//////////////////////////////////////////////////////////////////////////
// Windows/Classwizard stuff
//////////////////////////////////////////////////////////////////////////

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
public:

	// Window Msg routed from MainFrame for One Instance thingy
	LRESULT OnUserFileName(WPARAM wParam, LPARAM lParam);

	//{{AFX_DATA(CQVoiceView)
	enum { IDD = IDD_QVOICE_FORM };
	CPlayDisplay	m_playPic;
	CSliderCtrl	    m_slider;

	CLsBmpButton m_btnPlay;
//#ifndef LS_PLAYER_ONLY
	CLsBmpButton m_btnRecord;
	CLsBmpButton m_btnAttach;
//#endif
	CLsBmpButton m_btnStop;
	CLsBmpButton m_btnPause;
	CLsBmpButton m_btnFastR;
	CLsBmpButton m_btnFastF;
	CLsBmpButton m_btnVolU;
	CLsBmpButton m_btnVolD;
/*
	CBitmapButton m_btnPlay;
//#ifndef LS_PLAYER_ONLY
	CBitmapButton m_btnRecord;
	CBitmapButton m_btnAttach;
//#endif
	CBitmapButton m_btnStop;
	CBitmapButton m_btnPause;
	CBitmapButton m_btnFastR;
	CBitmapButton m_btnFastF;
	CBitmapButton m_btnVolU;
	CBitmapButton m_btnVolD;
*/
	//}}AFX_DATA
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQVoiceView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CQVoiceView)
	afx_msg LRESULT OnWaveInData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWaveOutData(WPARAM wParam, LPARAM lParam); 
	afx_msg LRESULT OnWaveOutClose(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBtnPlay();
	afx_msg void OnBtnRecord();
	afx_msg void OnBtnPause();
	afx_msg void OnBtnStop();
	afx_msg void OnBtnFastR();
	afx_msg void OnBtnFastF();
	afx_msg void OnBtnVolU();
	afx_msg void OnBtnVolD();
	afx_msg void OnBtnAttach();
	afx_msg void OnUpdatePlay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecord(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStop(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePause(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFastR(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFastF(CCmdUI* pCmdUI);
	afx_msg void OnUpdateVolU(CCmdUI* pCmdUI);
	afx_msg void OnUpdateVolD(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAttach(CCmdUI* pCmdUI);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnNewRecord();
	afx_msg void OnEditInsertMode();
	afx_msg void OnVolumebar();
	afx_msg void OnUpdateEditInsertMode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNewRecord(CCmdUI* pCmdUI);
	afx_msg void OnUpdateVolumebar(CCmdUI* pCmdUI);
	afx_msg void OnSelchangeCmbCodec();
	afx_msg void OnCodecMenuRange(UINT nID);
	afx_msg void OnUpdateCodecMenuRange(CCmdUI* pCmdUI);
	afx_msg BOOL OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg void OnAgc();
	afx_msg void OnUpdateAgc(CCmdUI* pCmdUI);
	afx_msg void OnEncode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in QVoiceView.cpp
inline CQVoiceDoc* CQVoiceView::GetDocument()
   { return (CQVoiceDoc*)m_pDocument; }
#endif

#endif
