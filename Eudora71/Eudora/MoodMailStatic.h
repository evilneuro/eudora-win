#if !defined(AFX_MOODMAILSTATIC_H__3A69ECBE_3BFB_11D4_AD74_0008C7D3A427__INCLUDED_)
#define AFX_MOODMAILSTATIC_H__3A69ECBE_3BFB_11D4_AD74_0008C7D3A427__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MoodMailStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMoodMailStatic window

class CMoodMailStatic : public SECStdBtn
{
DECLARE_BUTTON(CMoodMailStatic)
// Construction
public:
	CMoodMailStatic();

// Attributes
public:
	//CImageList m_listMoodImages;

// Operations
public:
	BOOL AddImages();
	int m_nScore;

// Implementation
public:
	virtual ~CMoodMailStatic();

	virtual void AdjustSize();
	void SetScore(int nScore);
	int GetScore();

protected:
	virtual void DrawFace(SECBtnDrawData& data, BOOL bForce, int& x, int& y, 
						  int& nWidth, int& nHeight, int nImgWidth = -1);

	
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOODMAILSTATIC_H__3A69ECBE_3BFB_11D4_AD74_0008C7D3A427__INCLUDED_)
