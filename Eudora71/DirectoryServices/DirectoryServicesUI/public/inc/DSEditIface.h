///////////////////////////////////////////////////////////////////////////////
// DSEditIface.h
//
// Created: 09/13/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_EDIT_IFACE_H_
#define __DS_EDIT_IFACE_H_

typedef void (*KeyCB)(void *, WPARAM);
typedef void (*ActivateCB)(void *);

#include <xstddef>
#pragma warning (disable: 4663 4244)
#include "AutoCompleteSearcher.h"
#pragma warning (default: 4663 4244)
//class AutoCompList;
//class AutoCompleterListBox;
class DSQueryBtn;

class DSEdit : public AutoCompCEdit {

    DECLARE_DYNCREATE(DSEdit)

public:
                    DSEdit()
			: keyCB(NULL), activateCB(NULL), pvUserData(NULL)
                    {
//			fBaptizeBlockMT(this, "DSUI-DSEdit");
			m_hIBeam = AfxGetApp()->LoadStandardCursor(IDC_IBEAM);
			m_hArrowHour = AfxGetApp()->
			               LoadStandardCursor(IDC_APPSTARTING);
			m_DSQueryACList = NULL;
			m_ACListBox = NULL;
			m_TheDropDownBtn = NULL;
			m_PoppedUpState = false;
		    }

    virtual        ~DSEdit()
                    {}        

    virtual BOOL PreTranslateMessage(MSG* pMsg);

    void            SetKeyCB(void *pv, KeyCB cb, ActivateCB acb)
                    {pvUserData = pv; keyCB = cb; activateCB = acb;}

//	CDC* BeginPaint( LPPAINTSTRUCT lpPaint );
	void AssocDropDownBtn(DSQueryBtn* Thebutton){m_TheDropDownBtn = Thebutton;}
	BOOL ACListPoppedUp();
	void KillACList();
	void DoQueryAutoComplete(bool fullList = false);
	void InitListElements();
	void DestroyListElements();
	bool PoppedUpState();
	void GetACListWindowRect(LPRECT rct);
	void SetPoppedUpState(bool newState);
	AutoCompList* GetACList(){return m_DSQueryACList;}

private:

    HCURSOR     m_hIBeam;
    HCURSOR     m_hArrowHour;
    KeyCB       keyCB;
    ActivateCB  activateCB;
    void       *pvUserData;
	DSQueryBtn*					m_TheDropDownBtn;
	AutoCompList*				m_DSQueryACList;
	AutoCompleterListBox*		m_ACListBox;
	BOOL						m_DoAutoComp;
	bool						m_PoppedUpState;
  
protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    afx_msg BOOL    OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT msg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnKillFocus( CWnd* blah);
	afx_msg void OnSetFocus( CWnd* blah);
	void OnACListSelectionMade();
	void OnACListDisplayed();
//	afx_msg void OnPaint();

    DECLARE_MESSAGE_MAP()
};

class DSQueryBtn : public CButton {
   DECLARE_DYNCREATE(DSQueryBtn)

public:
                    DSQueryBtn();
    virtual			~DSQueryBtn(){}        
	void			SetAssocField(DSEdit* Editcon);
	void			DoQueryAutoComplete();
	  virtual BOOL PreTranslateMessage(MSG *pMsg);

private:
	DSEdit*		m_EditControl;
  
protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

    DECLARE_MESSAGE_MAP()
};


#endif // __DS_EDIT_IFACE_H_
