// MessagePlug.cpp: implementation of the CMessagePlug class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "testplugin.h"
#include "MessagePlug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessagePlug::CMessagePlug(CTestpluginApp* TheApp)
{
	m_TheApp = TheApp;
	m_Icon = TheApp->LoadIcon(IDI_VCARD);
}

CMessagePlug::~CMessagePlug()
{

}
