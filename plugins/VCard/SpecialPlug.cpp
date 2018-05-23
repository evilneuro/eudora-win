// SpecialPlug.cpp: implementation of the CSpecialPlug class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "testplugin.h"
#include "SpecialPlug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSpecialPlug::CSpecialPlug()
{
	m_PluginIcon = AfxGetApp()->LoadIcon(IDI_VCARD);
}

CSpecialPlug::~CSpecialPlug()
{

}
