// TBarMenuButton.h: interface for the CTBarMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TBARMENUBUTTON_H__8506CDD1_223B_11D1_A766_0060972F7459__INCLUDED_)
#define AFX_TBARMENUBUTTON_H__8506CDD1_223B_11D1_A766_0060972F7459__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "tbtnstd.h"

class CTBarMenuButton : public SECStdBtn  
{
		HMENU	m_hMenu;

public:
	DECLARE_BUTTON(CTBarMenuButton);

	CTBarMenuButton();
	virtual ~CTBarMenuButton();

	virtual BOOL BtnPressDown(CPoint point);

	void SetHMenu( HMENU hMenu );

};

#define TBARMENU_BUTTON(id, style) \
	{BUTTON_CLASS(CTBarMenuButton), id, style|TBBS_BUTTON, 0, 0, 0, 0, 0},

#endif // !defined(AFX_TBARMENUBUTTON_H__8506CDD1_223B_11D1_A766_0060972F7459__INCLUDED_)
