// SpecialPlug.h: interface for the CSpecialPlug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPECIALPLUG_H__D212512B_752B_11D2_8A0E_00805F9B7487__INCLUDED_)
#define AFX_SPECIALPLUG_H__D212512B_752B_11D2_8A0E_00805F9B7487__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSpecialPlug  
{
public:
	CSpecialPlug();
	virtual ~CSpecialPlug();
	HICON* GetIcon() {return &m_PluginIcon;}

	HICON m_PluginIcon;

};

#endif // !defined(AFX_SPECIALPLUG_H__D212512B_752B_11D2_8A0E_00805F9B7487__INCLUDED_)
