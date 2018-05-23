#ifndef TBARCOMBO_H
#define TBARCOMBO_H

#ifndef __TBTNCMBO_H__
#include "tbtncmbo.h"
#endif

class CTBarComboBtn : public SECComboBtn
{
	DECLARE_BUTTON(CTBarComboBtn);

	// Construction
public:
	CTBarComboBtn() {}

// Overrideables
protected:
//	virtual CWnd* GetWnd();
	virtual BOOL CreateWnd(CWnd* pParentWnd, DWORD dwStyle, CRect& rect, int nID);

//	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnFontCreateAndSet();

// Implementation
public:
	virtual ~CTBarComboBtn() {}

	// Called when toolbar bitmap has changed ... buttons should 
	// now adjust their size.
//	virtual void AdjustSize();
};

// Define a combo button
#define TBARCOMBO_BUTTON(id, comboId, style, comboStyle, comboDefWidth, \
					 comboMinWidth, comboHeight) \
	WND_BUTTON(BUTTON_CLASS(CTBarComboBtn), id, comboId, style, comboStyle, \
		       comboDefWidth, comboMinWidth, comboHeight)
#endif // WIN32