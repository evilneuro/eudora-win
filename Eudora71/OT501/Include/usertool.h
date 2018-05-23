// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:			John Williston
//  Description:	Declaration for SECUserTool
//  Created:		August 1996
//

#ifndef __USERTOOL_H__
#define __USERTOOL_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

/////////////////////////////////////////////////////////////////////////////
// SECUserTool

class SECUserTool : public CObject
{
	DECLARE_SERIAL(SECUserTool);

	BOOL m_bWait;
	CString m_strMenuText;
	CString m_strCommand;
	CString m_strArgs;
	CString m_strDirectory;
	UINT m_uiShowFlag;

	BOOL ParseReplacement(LPCTSTR p, CString& strKey, CString& strData) const;
	BOOL Run(LPCTSTR pcszDir, LPCTSTR pcszCmd, UINT uiShow, BOOL bWait) const;

public:
	SECUserTool();
	SECUserTool(const SECUserTool& rhs);
	virtual void Serialize(CArchive& archive);

	BOOL IsWaitEnabled() const
		{ return m_bWait; }
	BOOL SetWaitEnabled(BOOL b)
		{ return m_bWait = b; }
	LPCTSTR GetMenuText() const	{ return m_strMenuText; }
	LPCTSTR SetMenuText(LPCTSTR str)
		{ return m_strMenuText = str; }
	LPCTSTR GetCommand() const
		{ return m_strCommand; }
	LPCTSTR SetCommand(LPCTSTR str)
		{ return m_strCommand = str; }
	LPCTSTR GetArgs() const
		{ return m_strArgs; }
	LPCTSTR SetArgs(LPCTSTR str)
		{ return m_strArgs = str; }
	LPCTSTR GetDirectory() const
		{ return m_strDirectory; }
	LPCTSTR SetDirectory(LPCTSTR str)
		{ return m_strDirectory = str; }
	UINT GetShowFlag() const
		{ return m_uiShowFlag; }
	UINT SetShowFlag(UINT i)
		{ return m_uiShowFlag = i; }

	BOOL Execute(CStringList* pReplacements) const;

	SECUserTool& operator=(const SECUserTool& rhs);
	SECUserTool* Clone() const;
};

BOOL ReadUserToolFile(LPCTSTR pcszFilename, CObArray& a);
BOOL WriteUserToolFile(LPCTSTR pcszFilename, CObArray& a);
BOOL AppendUserTools(CMenu* pMenu, UINT uiFirstID, const CObArray& a);
void EmptyUserToolArray(CObArray& a);

/////////////////////////////////////////////////////////////////////////////

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif	// __USERTOOL_H__
