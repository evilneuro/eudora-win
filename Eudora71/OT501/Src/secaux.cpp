
#include "stdafx.h"
 
#ifdef _SECDLL
#undef AFXAPI_DATA
#define AFXAPI_DATA __based(__segname("_DATA"))
#endif //_SECDLL

#ifndef __SECAUX_H__
#include "secaux.h"
#endif

#ifdef _SECDLL
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA
#endif //_SECDLL

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Cached system metrics, etc

SEC_AUX_DATA secData;

// Initialization code
// Initialization code
SEC_AUX_DATA::SEC_AUX_DATA()
{
	// Cache various target platform version information
	DWORD dwVersion = ::GetVersion();
	bWin4 = (BYTE)dwVersion >= 4;

	// Cached system values (updated in CWnd::OnSysColorChange)
	UpdateSysColors();

	// Standard cursors
	hcurHSplit = NULL;
	hcurVSplit = NULL;
	hcurArrow = ::LoadCursor(NULL, IDC_ARROW);
	ASSERT(hcurArrow != NULL);

	// cxBorder2 and cyBorder are 2x borders for Win4
	cxBorder2 = bWin4 ? CX_BORDER*2 : CX_BORDER;
	cyBorder2 = bWin4 ? CY_BORDER*2 : CY_BORDER;
}

// Termination code
SEC_AUX_DATA::~SEC_AUX_DATA()
{
}

void SEC_AUX_DATA::UpdateSysColors()
{
	clrBtnFace       = ::GetSysColor(COLOR_BTNFACE);
	clrBtnShadow     = ::GetSysColor(COLOR_BTNSHADOW);
	clrBtnHilite     = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	clrWindowFrame   = ::GetSysColor(COLOR_WINDOWFRAME);
	clrText          = ::GetSysColor(COLOR_WINDOWTEXT);
	clrWindow        = ::GetSysColor(COLOR_WINDOW);
	clrHighlight     = ::GetSysColor(COLOR_HIGHLIGHT);
	clrHighlightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
#ifdef WIN32
	clr3DLight       = ::GetSysColor(COLOR_3DLIGHT);
	clr3DDkShadow    = ::GetSysColor(COLOR_3DDKSHADOW);
#else
	clr3DLight       = ::GetSysColor(COLOR_BTNFACE);
	clr3DDkShadow    = ::GetSysColor(COLOR_WINDOWFRAME);
#endif
}

/////////////////////////////////////////////////////////////////////////////
