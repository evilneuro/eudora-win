// EmoticonMenu.h : header file
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

/////////////////////////////////////////////////////////////////////////////
// CColorMenu window

class EmoticonMenu : public CMenu
{

public:
	EmoticonMenu();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);

	static void DoDrawItem( LPDRAWITEMSTRUCT lpDIS );
	static void DoMeasureItem( LPMEASUREITEMSTRUCT lpMIS );

private:
	static const short kTextPadding;
	static unsigned int m_maxImageWidth;
	static unsigned int m_maxImageHeight;
	static unsigned int m_xPosMeaning;

	static void CalculateMaxMenuDimensions(CDC &dc);
};

/////////////////////////////////////////////////////////////////////////////
