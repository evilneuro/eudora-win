// MsgLabel.cpp: implementation of the CMsgLabel class.
//
// --------------------------------------------------------------------------

#include "stdafx.h"
#include "resource.h"
#include "rs.h"

#include "MsgLabel.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

// --------------------------------------------------------------------------

// [PUBLIC] CMsgLabel (Default Constructor)
CMsgLabel::CMsgLabel()
	: m_Color(RGB(0,0,0)), m_Text("")
{ }

// --------------------------------------------------------------------------

// [PUBLIC] CMsgLabel (Copy Constructor)
CMsgLabel::CMsgLabel(const CMsgLabel& copy)
	: m_Color(copy.m_Color), m_Text(copy.m_Text)
{ }

// --------------------------------------------------------------------------

CMsgLabel::CMsgLabel(const COLORREF& clr, LPCSTR str)
	: m_Color(clr), m_Text(str)
{ }

// --------------------------------------------------------------------------

// [PUBLIC] CMsgLabel (Destructor)
CMsgLabel::~CMsgLabel()
{ }

// --------------------------------------------------------------------------

// [PUBLIC] GetColor
COLORREF CMsgLabel::GetColor() const
{
	return (m_Color);
}

// --------------------------------------------------------------------------

// [PUBLIC] GetText
LPCSTR CMsgLabel::GetText() const
{
	return (m_Text);
}

// --------------------------------------------------------------------------

// [PUBLIC] SetColor
bool CMsgLabel::SetColor(const COLORREF & clr)
{
	m_Color = clr;
	return (true);
}

// --------------------------------------------------------------------------

// [PUBLIC] SetText
bool CMsgLabel::SetText(const CString &txt)
{
	m_Text = txt;
	return (true);
}
