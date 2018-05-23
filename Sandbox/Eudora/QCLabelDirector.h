// QCLabelDirector.h: interface for the QCLabelDirector class.
//
// --------------------------------------------------------------------------

#if !defined(AFX_QCLABELDIRECTOR_H__BDB08419_E91F_11D1_94C7_00805F9BF4D7__INCLUDED_)
#define AFX_QCLABELDIRECTOR_H__BDB08419_E91F_11D1_94C7_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// --------------------------------------------------------------------------

#include "NewIfaceMT.h"

#include <vector.h>
#include "MsgLabel.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

// --------------------------------------------------------------------------

class QCLabelDirector
{
public:
	bool Read();
	bool Write();
	static bool Destroy();
	static bool Initialize();
	static QCLabelDirector *GetLabelDirector();

	bool SetCount(unsigned int nCount);
	bool SetLabel(unsigned int nIdx, LPCSTR txt);
	bool SetLabel(unsigned int nIdx, const COLORREF& clr);
	bool SetLabel(unsigned int nIdx, const COLORREF& clr, LPCSTR txt);

	unsigned int GetCount() const;
	bool GetLabel(unsigned int nIdx, CString *txt) const;
	bool GetLabel(unsigned int nIdx, COLORREF *clr) const;
	bool GetLabel(unsigned int nIdx, COLORREF *clr, CString *txt) const;

	LPCSTR GetLabelText(unsigned int nIdx) const;
	COLORREF GetLabelColor(unsigned int nIdx) const;

protected:
	unsigned int ReadLabelCount();
	bool ReadNewINILabel(unsigned int nIdx, CMsgLabel *lbl);
	bool ReadOldINILabel(unsigned int nIdx, CMsgLabel *lbl);

	bool GetDefaultLabel(unsigned int nIdx, CMsgLabel *lbl) const;
	CString GetDefaultText(unsigned int nIdx) const;
	COLORREF GetDefaultColor(unsigned int nIdx) const;

	CString m_INICountKey, m_INISection, m_INIKeyFormat;
	vector<CMsgLabel> m_vLabels;

	// Protected constructor
	QCLabelDirector();
	virtual ~QCLabelDirector();

	static QCLabelDirector *m_pTheLabelDirector;
};

#endif // !defined(AFX_QCLABELDIRECTOR_H__BDB08419_E91F_11D1_94C7_00805F9BF4D7__INCLUDED_)
