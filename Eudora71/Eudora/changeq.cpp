// CHANGEQ.CPP
//
// Change Queueing dialog
//

#include "stdafx.h"

#ifdef WIN32
#include <afxcmn.h>
#include <afxrich.h>
#endif
#include <ctype.h>

#include <QCUtils.h>

#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "summary.h"
#include "changeq.h"
#include "doc.h"
#include "msgdoc.h"
#include "compmsgd.h"
#include "guiutils.h"
#include "utils.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CChangeQueueing dialog


CChangeQueueing::CChangeQueueing(long GMTSeconds, CWnd* pParent /*=NULL*/)
	: CHelpxDlg(CChangeQueueing::IDD, pParent? pParent : AfxGetMainWnd())
{
	long Seconds = GMTSeconds;
	if (Seconds)
		Seconds -= GetGMTOffset() * 60;
	m_StartSeconds = time(NULL);
	m_Seconds = Seconds? Seconds : m_StartSeconds;

	// this allows the user to set anytime in the future or after
	// last setting, whichever is smaller
	if (Seconds)
	{
		if (m_StartSeconds > Seconds)
			m_StartSeconds = Seconds;
	}
	m_StartSeconds -= 60;	// rounds down, because user can only set to the minute

	//{{AFX_DATA_INIT(CChangeQueueing)
	m_Date = "";
	m_Time = "";
	//}}AFX_DATA_INIT

	// restore radio button setting if we're coming around again
	if (Seconds)
		m_Type = ON_OR_AFTER;
	else
		m_Type = GetIniShort(IDS_INI_IMMEDIATE_SEND)? NEXT_TIME : RIGHT_NOW;
}

void CChangeQueueing::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);

	// If we're filling in the fields, then grab the date and time from m_Seconds
	if (pDX->m_bSaveAndValidate == FALSE)
	{
		char TimeDate[32];
		char* Date;
		
		::TimeDateStringMT(TimeDate, m_Seconds, TRUE);
		Date = strrchr(TimeDate, ' ');
		*Date++ = 0;
		m_Time = TimeDate;
		m_Date = Date;
	}

	//{{AFX_DATA_MAP(CChangeQueueing)
	DDX_Text(pDX, IDC_EDIT_DATE, m_Date);
	DDX_Text(pDX, IDC_EDIT_TIME, m_Time);
	DDX_Radio(pDX, IDC_RIGHT_NOW, m_Type);
	//}}AFX_DATA_MAP

	// If we're getting the values from the controls, do some validation	
	if (pDX->m_bSaveAndValidate && m_Type == ON_OR_AFTER)
	{
		// Get some info to help parse fields
		CRString IntlSection(IDS_WININI_INTL_SECTION);
		int iTime;
		char s2359[8];
		char sShortDate[32];
		
		iTime = ::GetProfileInt(IntlSection, CRString(IDS_WININI_ITIME), 0);
		::GetProfileString(IntlSection, CRString(IDS_WININI_S2359), "PM", s2359, sizeof(s2359));
		::GetProfileString(IntlSection, CRString(IDS_WININI_SSHORTDATE), "m/d/yy",
			sShortDate, sizeof(sShortDate));	

		int i, Month = 0, Day = 1, Year = 2;
		const char* d = sShortDate;
		for (i = 0; i < 3 && *d; i++)
		{
			int Type = tolower(*d++);
			switch (Type)
			{
			case 'm': Month = i;	break;
			case 'd': Day = i;		break;
			case 'y': Year = i;		break;
			}
			
			// Skip to next type
			while (tolower(*d) == Type)
				d++;
			while (!isalpha((int)(unsigned char)*d))
				d++;
		}

		// Parse date field
		int Date[3];
		d = m_Date;
		for (i = 0; i < 3 && *d; i++)
		{
			Date[i] = atoi(d);
			
			// Skip to next type
			while (isdigit((int)(unsigned char)*d))
				d++;
			while (!isdigit((int)(unsigned char)*d) && *d)
				d++;
		}
		
		// Parse time field
		int Hour, Minute;
		const char* Time = m_Time;
		
		Hour = atoi(Time);
			
		// Skip hour-minute separator
		while (isdigit((int)(unsigned char)*Time))
			Time++;
		while (!isdigit((int)(unsigned char)*Time) && *Time)
			Time++;
			
		Minute = atoi(Time);
		
		// Look for AM/PM designation only if international time isn't specified
		if (!iTime)
		{
			while (isdigit((int)(unsigned char)*Time))
				Time++;
			while (isspace((int)(unsigned char)*Time))
				Time++;
			if (*Time)
			{
				if (Hour == 12)
					Hour = 0;
				if (!strnicmp(Time, s2359, strlen(s2359)))
					Hour += 12;
			}
		}
		
		// Whew! Now that we have the data, get it in seconds since epoch format
		if (Date[Year] < 70)
			Date[Year] += 2000;
		else if (Date[Year] < 100)
			Date[Year] += 1900;
		CTime TheTime(Date[Year], Date[Month], Date[Day], Hour, Minute, 0);
		m_Seconds = static_cast<time_t>( TheTime.GetTime() );
		
		// Flag a time that's before the current time
		if (m_Seconds < m_StartSeconds)
		{
			ErrorDialog(IDS_ERR_OLD_DATE);
			pDX->PrepareEditCtrl(IDC_EDIT_TIME);
			pDX->Fail();
		}
	}
}

BEGIN_MESSAGE_MAP(CChangeQueueing, CHelpxDlg)
	//{{AFX_MSG_MAP(CChangeQueueing)
	ON_EN_SETFOCUS(IDC_EDIT_DATE, OnSetFocusEdit)
	ON_EN_SETFOCUS(IDC_EDIT_TIME, OnSetFocusEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CChangeQueueing::ChangeSummary(CSummary* Sum)
{
	BOOL Success = FALSE;
	
	if (m_Type == DONT_SEND)
	{
		if (Sum->m_State != MS_UNSENDABLE)
			Sum->SetState(MS_SENDABLE);
		Success = TRUE;
	}
	else if (Sum->m_FrameWnd)
	{
		if (((CCompMessageDoc*)Sum->FindMessageDoc())->Queue())
			Success = TRUE;
    }
	else
	{
		if (Sum->m_State != MS_UNSENDABLE)
        {
			Sum->SetState(MS_QUEUED);
			Success = TRUE;
		}
	}

	if (Success)
	{
		time_t Now = time(NULL);
		int TimeZoneMinutes = -GetGMTOffset();
		time_t GMTNow = Now - TimeZoneMinutes * 60;

		Sum->m_TimeZoneMinutes = TimeZoneMinutes;
		switch (m_Type)
		{
		case RIGHT_NOW:
			Sum->m_Seconds = GMTNow;
			FlushQueue = TRUE;
			break;
		case NEXT_TIME:
			if (Sum->m_Seconds > GMTNow)
				Sum->m_Seconds = GMTNow;
			FlushQueue = FALSE;
			break;
		case ON_OR_AFTER:
			Sum->SetState(MS_TIME_QUEUED);
			Sum->m_Seconds = m_Seconds - TimeZoneMinutes * 60;
			break;
		default:
			ASSERT(0);
			Success = FALSE;
		}

		Sum->FormatDate();
		SetQueueStatus();
	}

	return (Success);
}

/////////////////////////////////////////////////////////////////////////////
// CChangeQueueing message handlers

BOOL CChangeQueueing::OnInitDialog()
{
	CHelpxDlg::OnInitDialog();
	
	CenterWindow();
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}

// OnSetFocusEdit
// One of the edit boxes under the On or after radio button is getting the focus,
// so select the On or after option
//
void CChangeQueueing::OnSetFocusEdit()
{
	if (IsDlgButtonChecked(IDC_RIGHT_NOW))		CheckDlgButton(IDC_RIGHT_NOW, FALSE);
	else if (IsDlgButtonChecked(IDC_NEXT_TIME))	CheckDlgButton(IDC_NEXT_TIME, FALSE);
	else if (IsDlgButtonChecked(IDC_DONT_SEND))	CheckDlgButton(IDC_DONT_SEND, FALSE);
	else return;
	
	CheckDlgButton(IDC_ON_OR_AFTER, TRUE);
}
