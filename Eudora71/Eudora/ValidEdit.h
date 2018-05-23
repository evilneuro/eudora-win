// ValidEdit.h : header file
//

#ifndef _VALIDEDIT_H_
#define _VALIDEDIT_H_

/////////////////////////////////////////////////////////////////////////////
// CValidEdit class

class CValidEdit : public CEdit
{
  DECLARE_DYNAMIC(CValidEdit)

public:
// Constructors
	CValidEdit();

// Operations
	void SetValid(LPCSTR lpValid);
	void SetInvalid(LPCSTR lpInvalid);

	bool SetSilent(bool bSilent = true);

protected:
// Attributes
	bool m_bUseValid, m_bUseInvalid;
	CString m_sValid, m_sInvalid;

	bool m_bSilent;

// Implementation
	bool CheckChar(UINT nChar);
	void SendChar(UINT nChar);

	// Generated message map functions
	//{{AFX_MSG(CValidEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	afx_msg LRESULT OnPaste(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

#endif // _VALIDEDIT_H_
