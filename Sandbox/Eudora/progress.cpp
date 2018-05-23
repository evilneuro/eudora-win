// PROGRESS.CPP
//
// Routines for the Progress window
//

#include "stdafx.h"

#include <afxpriv.h>
#include <string.h>
#include <QCUtils.h>

#include "progress.h"
#include "resource.h"
#include "guiutils.h"
#include "rs.h"
#include "font.h"
#include "doc.h"
#include "debug.h"
#include "persona.h"
#include "Automation.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// --------------------------------------------------------------------------

void AFXAPI AfxSetWindowText(HWND hWndCtrl, LPCTSTR lpszNew); 

// --------------------------------------------------------------------------

class CProgressObject : public CObject
{
public:
	CProgressObject();

	void SetMessage(const char* Message)
		{ m_Message = Message; }
	void SetSoFar(long SoFar)
		{ m_SoFar = min(SoFar, m_Total); }
	void AddSoFar(long Increment)
		{ SetSoFar(m_SoFar + Increment); }
	void SetTotal(long Total)
		{ m_Total = Total; }

	const char* GetMessage()
		{ return (m_Message); }
	long GetSoFar()
		{ return (m_SoFar); }
	long GetTotal()
		{ return (m_Total); }
	int GetPercent()
		{ return (m_Percent); }
	int CalcPercent()
		{ return (m_Percent = (int)(m_Total > 0L? m_SoFar * 100L / m_Total : (!m_Total? 100 : -1))); }

protected:
	CString	m_Message;
	long	m_SoFar;
	long	m_Total;
	int		m_Percent;
};

CProgressObject::CProgressObject()
{
	m_SoFar = m_Total = -1L;
	m_Percent = -1;
}

/////////////////////////////////////////////////////////////////////////////
// CProgressWindow dialog

class CProgressWindow : public CDialog
{
// Construction
public:
	CProgressWindow(CWnd* pParent = NULL);	// standard constructor
	~CProgressWindow();

// Dialog Data
	//{{AFX_DATA(CProgressWindow)
	enum { IDD = IDD_PROGRESS };
	CStatic	m_Bar;
	CString	m_Text1;
	CString	m_Text2;
	//}}AFX_DATA

// Operations
	BOOL OnInitDialog();

	CProgressObject* CurrentObject();
	void Push()
		{ m_ObjectStack.AddHead(new CProgressObject); }
	void Pop()
		{ delete m_ObjectStack.RemoveHead(); }
	
	void SetCountdown(const char* Message, int Value);
	void DecrementCountdown()
		{ ASSERT(m_CountdownValue > 0); SetCountdown(NULL, m_CountdownValue - 1); }

	BOOL CanShow()
		{ return m_bCanShow; }


// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void OnCancel();
	void PostNcDestroy();

	// Generated message map functions
	//{{AFX_MSG(CProgressWindow)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nTimerID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	CString		m_CountdownMessage;
	int			m_CountdownValue;
	CFontInfo	m_SecondaryMessageFont;
	CObList		m_ObjectStack;
	CRect		m_InitialWindowPos;
	BOOL		m_bCanShow;
};

/////////////////////////////////////////////////////////////////////////////
// CProgressWindow dialog


CProgressWindow::CProgressWindow(CWnd* pParent /*=NULL*/)
	: CDialog()
{
	ASSERT(::IsMainThreadMT());

	//{{AFX_DATA_INIT(CProgressWindow)
	m_Text1 = "";
	m_Text2 = "";
	//}}AFX_DATA_INIT
	
	m_CountdownValue = 0;
	m_bCanShow = FALSE;
	
	// Add initial progress object
	Push();
	
	Create(IDD, pParent);
}

CProgressWindow::~CProgressWindow()
{
	// There should be only one Progress object left, the one
	// that was created when the Progress windows was created.
	// If you get this ASSERT it means that you tried to close
	// the Progress window when there were other tasks still
	// using the Progress window.
	ASSERT(m_ObjectStack.GetCount() == 1);

	while (m_ObjectStack.IsEmpty() == FALSE)
		Pop();
}

CProgressObject* CProgressWindow::CurrentObject()
{
	if (m_ObjectStack.IsEmpty())
	{
		// Uh oh.  Someone popped off too many progress objects!
		ASSERT(0);
		return NULL;
	}
		
	return ((CProgressObject*)m_ObjectStack.GetHead());
}

BOOL CProgressWindow::OnInitDialog()
{
	CRect rect;
	
	CDialog::OnInitDialog();
	
	// Set the font of the main progress message
	CWnd* Wnd = GetDlgItem(IDC_PROGRESS2);
	if (Wnd && m_SecondaryMessageFont.CreateFont("MS Sans Serif", 8));
	{
//		m_SecondaryMessageFont.m_Weight == FW_BOLD;
		Wnd->SetFont(&m_SecondaryMessageFont, FALSE);
	}
	
	// Only position the window if it's been moved
	GetIniWindowPos(IDS_INI_PROGRESS_WIN_POS, rect);
	if (rect.left || rect.top)
	{
		rect.left = max(rect.left, 0);
		rect.top = max(rect.top, 0);
		SetWindowPos(NULL, rect.left, rect.top, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
	}
	GetWindowRect(&m_InitialWindowPos);

	// Move Progress window to the right place in the Z-order (just above the main window)
	CWnd* WinAbove = AfxGetMainWnd();
	if (WinAbove && (WinAbove == WinAbove->GetNextWindow(GW_HWNDPREV)))
		SetWindowPos(WinAbove, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

	// Set the timer causes the Progress window to delay being shown, so that if a task
	// takes a very short amount of time the Progress window won't be shown.
	// If creating the timer fails or the user has set a zero timeout,
	// then just show the window right away
	int ProgressIdle = GetIniShort(IDS_INI_PROGRESS_IDLE);
	if (!ProgressIdle || SetTimer(1, ProgressIdle * 1000, NULL) == 0)
		m_bCanShow = TRUE;

	return (FALSE);
}

void CProgressWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressWindow)
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_Bar);
	DDX_Text(pDX, IDC_PROGRESS1, m_Text1);
	DDX_Text(pDX, IDC_PROGRESS2, m_Text2);
	//}}AFX_DATA_MAP
}

void CProgressWindow::SetCountdown(const char* Message, int Value)
{
	char buf[12];	// Have to handle the maximum sized long which is like  -2147483648  + \0
	
	if (Message)
		m_CountdownMessage = Message;
	if (Value >= 0)
		m_CountdownValue = Value;
		
	m_Text1 = m_CountdownMessage;
	wsprintf(buf, "%d", m_CountdownValue);
	m_Text1 += buf;
	
	// Call MainProgress() so that the info gets written to the log file
	MainProgress(m_Text1);
}

void CProgressWindow::OnCancel()
{
	// Simulate the press of the Esc key
	PostMessage(WM_CHAR, VK_ESCAPE);
}

void CProgressWindow::PostNcDestroy()
{
	ASSERT( ProgWin == this );
	ProgWin = NULL;	 // ProgWin should be equal to this
	delete this;
}

BEGIN_MESSAGE_MAP(CProgressWindow, CDialog)
	//{{AFX_MSG_MAP(CProgressWindow)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProgressWindow message handlers

void CProgressWindow::OnPaint()
{
	// Can't use a CPaintDC because it will call BeginPaint and EndPaint,
	// thus validating the entire window
	//CPaintDC dc(this); // device context for painting
	
	CProgressObject* Obj = CurrentObject();
	
	if (Obj && Obj->GetTotal() >= 0L)
	{
		CDC* dc = m_Bar.GetDC();
		CRect rect;
		CRect NewRect;
		CBrush BackBrush(::GetSysColor(COLOR_HIGHLIGHT));
		
		ASSERT_VALID(dc);
		m_Bar.GetClientRect(&rect);
		rect.InflateRect(-2, -2);
		NewRect = rect;
		dc->SelectStockObject(NULL_PEN);
		
		// Calculate rectangle for progress left to go and draw
		NewRect.left += (int)((NewRect.Width() * (long)Obj->CalcPercent()) / 100L );
		dc->Rectangle(&NewRect);

		// Calculate rectangle for progress so far and draw
		NewRect.right = NewRect.left;
		NewRect.left = rect.left;
		dc->SelectObject(&BackBrush);
		dc->Rectangle(&NewRect);
	
		m_Bar.ValidateRect(&rect);

		dc->SelectStockObject(BLACK_BRUSH);
		m_Bar.ReleaseDC(dc);
	}
	
	// Do not call CDialog::OnPaint() for painting messages
	Default();
}

void CProgressWindow::OnDestroy()
{
	CRect rect;
	
	GetWindowRect(&rect);
	if (m_InitialWindowPos != rect)
		SetIniWindowPos(IDS_INI_PROGRESS_WIN_POS, rect);

	CDialog::OnDestroy();
}


// Pointer to a progress window
CProgressWindow* ProgWin = NULL;
BOOL InProgress = FALSE;
extern BOOL g_bShowProgress;	// from POP.cpp (yuk)
extern BOOL g_bProgressHasFocus;


// CanShowProgress
//
// Tells whether or not it is time for the Progress window to be shown
//
BOOL CanShowProgress()
{
	if (ProgWin)
		return ProgWin->CanShow();

	return FALSE;
}

// OpenProgress
//
// Create the progress window if it does not exist.
//
BOOL OpenProgress()
{
	ASSERT(::IsMainThreadMT());

	if (!ProgWin)
	{
		if (!(ProgWin = new CProgressWindow))
			return (FALSE);
	}

	static CString Persona;
	if (Persona != g_Personalities.GetCurrent())
	{
		// set progress title
		Persona = g_Personalities.GetCurrent();
		CRString Title(IDS_PROGRESS);
		if (Persona.IsEmpty() == FALSE)
		{
			Title += " - ";
			Title += Persona;
		}
		AfxSetWindowText(ProgWin->GetSafeHwnd(), Title);
	}

	if (ProgWin->IsWindowVisible())
		return (TRUE);
	
	if (!InProgress)
		AfxGetMainWnd()->EnableWindow(FALSE);

	InProgress = TRUE;

	if (!ProgWin->CanShow())
		return TRUE;

	// see if we are the active app
	CWnd* Active = CWnd::GetForegroundWindow();
	const BOOL bWeBeActive = (Active->GetParentFrame() == AfxGetMainWnd() || Active == AfxGetMainWnd() || Active == ProgWin);

	if (!gbAutomationCall)
	{
		if ( GetIniShort(IDS_INI_SHOW_PROGRESS) )
		{
			if (bWeBeActive || g_bShowProgress || GetIniShort(IDS_INI_SHOW_PROGRESS_INACTIVE))
			{
				if (bWeBeActive)
					ProgWin->ShowWindow(SW_SHOW);
				else
					ProgWin->ShowWindow(SW_SHOWNA);
			}
			else
				ProgWin->ShowWindow(SW_HIDE);
		}
	}

	ProgWin->EnableWindow(TRUE);

	// make whoever was active active again
	if (!bWeBeActive && Active)
	{
		if(g_bProgressHasFocus)
			ProgWin->SetForegroundWindow();
		else
			Active->SetForegroundWindow();
	}

	return (TRUE);
}

void CProgressWindow::OnTimer(UINT nTimerID)
{
	CWnd* pTopWindow = GetForegroundWindow();

	while (pTopWindow)
	{
		if (pTopWindow->GetStyle() & DS_MODALFRAME)
			return;

		pTopWindow = pTopWindow->GetParent();
	}

	KillTimer(nTimerID);
	m_bCanShow = TRUE;
	OpenProgress();
}


// CloseProgress
//
// Close the progress window.
//
void CloseProgress()
{
	ASSERT(::IsMainThreadMT());
	CWnd* MainFrame = AfxGetMainWnd();

	// InProgress variable needs to be reset first to prevent further
	// calls to OpenProgress() which disables the main window
	InProgress = FALSE;

	// Need to enable the main window before destroying the progress window so that it becomes
	// the on-top window
	MainFrame->EnableWindow(TRUE);

	if (ProgWin)
		ProgWin->DestroyWindow();

	// Post a WM_SETCURSOR message so that the cursor will get updated
	CPoint MouseLocation;
	GetCursorPos(&MouseLocation);
	HWND hWnd = WindowFromPoint(MouseLocation);
	if (IsChild(AfxGetMainWnd()->GetSafeHwnd(), hWnd))
		PostMessage(hWnd, WM_SETCURSOR, (WPARAM)hWnd, 0);
}

// MainProgress
//
// Modify the main progress message
//
void MainProgress(const char* Message)
{
	ASSERT(::IsMainThreadMT());

	if (!OpenProgress())
		return;

	if (Message)
	{
		PutDebugLog(DEBUG_MASK_PROG, Message);

		ProgWin->m_Text1 = Message;
		ProgWin->m_Text2.Empty();
		ProgWin->UpdateData(FALSE);
	}
}

// CountdownProgress
//
// Set parameters for the main progress countdown indicator
//
void CountdownProgress(const char* Message /*= NULL*/, int Number /*= -1*/)
{
	ASSERT(::IsMainThreadMT());

	if (!OpenProgress())
		return;

	ProgWin->SetCountdown(Message, Number);
}

// DecrementCountdownProgress
//
// Subtract off one from the main progress countdown indicator
//
void DecrementCountdownProgress()
{
	ASSERT(::IsMainThreadMT());

	if (!OpenProgress())
		return;

	ProgWin->DecrementCountdown();
}

// Progress
//
// Modify the secondary progress message.  Leaves the progress bar alone.
//
void Progress(const char* Message)
{
	ASSERT(::IsMainThreadMT());

	CProgressObject* Obj;
	
	if (!OpenProgress() || !(Obj = ProgWin->CurrentObject()))
		return;

	if (Message)
	{
		PutDebugLog(DEBUG_MASK_PROG, Message);

		Obj->SetMessage(Message);
		ProgWin->m_Text2 = Message;
		ProgWin->UpdateData(FALSE);
	}
}


// Progress
//
// Modify the secondary progress message and/or the progress bar
//
void Progress(long SoFar, const char* Message, long Total)
{
	ASSERT(::IsMainThreadMT());

	CProgressObject* Obj;
	
	if (!OpenProgress() || !(Obj = ProgWin->CurrentObject()))
		return;

	if (Total >= 0L)
	{
		Obj->SetTotal(Total);
		if (!ProgWin->m_Bar.IsWindowVisible())
		{
			HWND hFocus = ::GetFocus();	// will be null if we are inactive 
			if ( hFocus || g_bShowProgress || GetIniShort(IDS_INI_SHOW_PROGRESS_INACTIVE) )
				ProgWin->m_Bar.ShowWindow(SW_SHOWNA);
		}
	}
	else if (SoFar < 0L /* KCM && ProgWin->m_Bar.IsWindowVisible() */)
		ProgWin->m_Bar.ShowWindow(SW_HIDE);
	
	Progress(Message);

	if (SoFar >= 0L || Total >= 0L)
		ProgressAdd(SoFar - Obj->GetSoFar());
}

void ProgressAdd(long Increment)
{
	ASSERT(::IsMainThreadMT());

	CProgressObject* Obj;
	
	if (!ProgWin || !(Obj = ProgWin->CurrentObject()))
		return;
	
	int OldPercent = Obj->CalcPercent();
	long OldSoFar = Obj->GetSoFar();
	
	Obj->AddSoFar(Increment);
	
	if (OldPercent != Obj->CalcPercent())
	{
		CRect rect;
		ProgWin->m_Bar.GetWindowRect(&rect);
		ProgWin->ScreenToClient(&rect);
		ProgWin->InvalidateRect(&rect, FALSE);
//		ProgWin->UpdateWindow();
	}
}

// PushProgress
//
// Makes a copy of the data contained in the current progress and saves it off.
//
void PushProgress()
{
	ASSERT(::IsMainThreadMT());

	if (ProgWin)
		ProgWin->Push();
}


// PopProgress
//
// Pop off the old progress and redisplay the previous progress
// 
void PopProgress()
{
	ASSERT(::IsMainThreadMT());

	if (ProgWin)
	{
		ProgWin->Pop();

		CProgressObject* Obj = ProgWin->CurrentObject();
		if (Obj)
			Progress(Obj->GetSoFar(), Obj->GetMessage(), Obj->GetTotal());
	}
}
