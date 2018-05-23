/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

#include "SafetyPal.h"

class CDirectoryStatic : public CStatic
{
public:
	CDirectoryStatic() { m_hCursor = NULL; }

protected:
	//{{AFX_MSG(CListCtrlEx)
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	HCURSOR m_hCursor;
};

class CAboutDlg : public CDialog
{
public:
	CAboutDlg(BOOL IsSplash, CWnd* pParentWnd = NULL);
	~CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_Version;
	CDirectoryStatic m_DataDirCtrl;
	CDirectoryStatic m_AppDirCtrl;
	//}}AFX_DATA

// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//{{AFX_MSG(CAboutDlg)
 	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnClickedCredits();
	afx_msg void OnClickedOK();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSysColorChange();
	afx_msg void OnHyperlinkBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void CenterControl(CWnd* pControl);
	void DrawCredits(CDC *pDC, char **pCreditsList);
	int CalcCreditsHeight(char **pCreditsList);

	char **LoadCreditsList(char **buffer, int nResourceID);

	UINT StartScrollTimer();
	void StopScrollTimer();

	CPaletteDC m_dcLogo;	// compatible Memory DC for storing bitmap bits
	CBitmap m_LogoBitmap; // Logo bitmap
	CBitmap* m_pLogoBitmap0; // Old bitmap

	UINT m_LogoBitmapWidth, m_LogoBitmapHeight;
	BOOL m_IsSplash;

	// "More Credits" stuff...
	BOOL m_InitializedCredits;

	CDC m_dcCredits, m_dcRealCredits;
	HBITMAP m_hCreditsBitmap, m_hOldCreditsBitmap;
	HBITMAP m_hRealCreditsBitmap, m_hOldRealCreditsBitmap;
	int m_nLeftPos, m_nTopPos, m_nTextLineHeight;
	CRect m_InvalidateRct;
	UINT m_TimerID,
		m_CurrentScrollPosition,
		m_MaxScroll,
		m_ScrollSpeed,
		m_CreditsHeight,
		m_CreditsWidth;
	CFont m_TitleFont, m_NameFont;
	CSafetyPalette m_Palette;
};

