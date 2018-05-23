#if !defined(AFX_POOTERBUTTON_H__A6D7AA23_7DC7_11D2_B887_00805F9BB281__INCLUDED_)
#define AFX_POOTERBUTTON_H__A6D7AA23_7DC7_11D2_B887_00805F9BB281__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PooterButton.h : header file
//

#ifdef _DEBUG
#ifdef new
#undef new
#endif
#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif
#endif

///////////////////////////////////////////////////////////////////////////////
// Ensure that memory for MFC CStatic object are routed throgh DMM. 
// smohanty: 12/16/98
///////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
class CStaticDMM : public CStatic {
public:
        void * __cdecl
	operator new(size_t size, const char *file, int line)
	{
	    return ::new(file, line) char[size];
	}
        void * __cdecl
	operator new(size_t size, char *file, int line)
	{
	    return ::new(file, line) char[size];
	}
        void * __cdecl
	operator new(size_t size, int, const char *file,
				    int line)
	{
	    return ::new(file, line) char[size];
	}
        void * __cdecl
	operator new(size_t size, int, char *file, int line)
	{
	    return ::new(file, line) char[size];
	}
        void operator delete(void *pv)
	{
	    delete [] ((char *) pv);
	}
};
#ifndef CStaticDMM
#define CStatic CStaticDMM
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// PooterButton window

class PooterButton : public CStatic
{
    enum { kFlat, kRaised, kPushed };
    int m_iState;
    bool m_bMouseCaptured;

// Construction
public:
    PooterButton();

// Attributes

// Operations
    void Draw( CDC* pdc );
    void Draw( CDC* pdc, int iState );

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(PooterButton)
        //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~PooterButton();

    // Generated message map functions
protected:
    //{{AFX_MSG(PooterButton)
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnMouseMove( UINT nFlags, CPoint point );
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POOTERBUTTON_H__A6D7AA23_7DC7_11D2_B887_00805F9BB281__INCLUDED_)
