// UsgStatsView.cpp: implementation of the CUsageStatisticsView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UsgStatsView.h"
#include "resource.h"
#include "utils.h"

#include "pgembeddedObject.h"
#include "pgcntritem.h"

#include "mainfrm.h"
#include "usgstatsdoc.h"

#include "QCSharewareManager.h"

#include "EuLang.h"
#include "locale.h"

#include "eudora.h"
#include "Trnslate.h"

#include "DebugNewHelpers.h"

UINT uRefreshUsageStatisticsWindow = RegisterWindowMessage("RefreshUsageStatisticsWindow");

IMPLEMENT_DYNCREATE(CUsageStatisticsView, CPaigeEdtView)

BEGIN_MESSAGE_MAP(CUsageStatisticsView, CPaigeEdtView)
	//{{AFX_MSG_MAP(CUsageStatisticsView)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_REGISTERED_MESSAGE( uRefreshUsageStatisticsWindow, OnRefreshUsageStatisticsWindow)
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
END_MESSAGE_MAP()


extern CString			gStrUsgStatHTML;
extern StatTimePeriod	gsPeriodSelected;
extern StatDataPtr		gStatData;
extern BOOL				gbMoreStatistics;

extern double gdRcvdAttachValues[4];
extern double gdReadMessageValues[4];
extern double gdSentAttachValues[4];
extern double gdFValues[3], gdRyValues[3], gdRdtValues[3];
extern short gsFacetimeValues[3];

extern long	glFirstColPercentWidth;
extern long	glSecondColPercentWidth;

extern TCHAR csPeriodStrings [4][3][32];
extern TCHAR csAverageString[32];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CUsageStatisticsView diagnostics

#ifdef _DEBUG
void CUsageStatisticsView::AssertValid() const
{
	CView::AssertValid();
}

void CUsageStatisticsView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

CUsageStatisticsDoc* CUsageStatisticsView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CUsageStatisticsDoc)));
	return (CUsageStatisticsDoc*)m_pDocument;
}

CUsageStatisticsView::CUsageStatisticsView()
{
	int i = 0,nVal = 0;
	// Shareware: Register that we want to know of feature changes
	QCSharewareManager *pSWM = GetSharewareManager();
	if (pSWM)
	{
		pSWM->Register((QICommandClient*)this);
	}

	for (i = 0; i < STATCOUNT; i++)
		m_Graph[i] = NULL;

	// Read from the INI file 
	short sGraphType;
	for (i = IDS_INI_STATISTICS_GRAPH_CURRENT;i <= IDS_INI_STATISTICS_GRAPH_AVERAGE; i++)
	{
		 sGraphType = GetIniShort(i);
		 switch(sGraphType)
		 {
			case 0:	// Line Graph
				m_GraphStyles[i - IDS_INI_STATISTICS_GRAPH_CURRENT] = 5 /*EuGraph's appropriate graph type value*/ ;	
				break;
			case 1:	// Area Graph
				m_GraphStyles[i - IDS_INI_STATISTICS_GRAPH_CURRENT] = 6;
				break;
			case 2:	// Bar Graph
				m_GraphStyles[i - IDS_INI_STATISTICS_GRAPH_CURRENT] = 0;
				break;
			case 3:	// Circle Graph
				m_GraphStyles[i - IDS_INI_STATISTICS_GRAPH_CURRENT] = 2;
				break;
			case 4:	// Diamond Graph
				m_GraphStyles[i - IDS_INI_STATISTICS_GRAPH_CURRENT] = 4;
				break;
			case 5:	// Rectangle Graph
				m_GraphStyles[i - IDS_INI_STATISTICS_GRAPH_CURRENT] = 3;
				break;
			default:
				ASSERT(0);	// It should not come here
				if (i == IDS_INI_STATISTICS_GRAPH_CURRENT) 
					m_GraphStyles[i - IDS_INI_STATISTICS_GRAPH_CURRENT] = 6;
				else
					m_GraphStyles[i - IDS_INI_STATISTICS_GRAPH_CURRENT] = 5;					
				break;
		 }
	}

	long lColorVal = GetIniLong(IDS_INI_STATISTICS_COLOR_CURRENT);
	m_GraphColors[0] = lColorVal == 0 ? RGB(255, 0, 0) : lColorVal;

	lColorVal = GetIniLong(IDS_INI_STATISTICS_COLOR_PREVIOUS);
	m_GraphColors[1] = lColorVal == 0 ? RGB(0, 128, 0) : lColorVal;

	lColorVal = GetIniLong(IDS_INI_STATISTICS_COLOR_AVERAGE);
	m_GraphColors[2] = lColorVal == 0 ? RGB(0, 0, 255) : lColorVal; 

	m_nTimerEventID = 1001;

	HDC hdc = ::GetDC(NULL);  // Screen DC used to get current display settings	
	if (GetDeviceCaps(hdc,HORZRES) > 800)
		glFirstColPercentWidth = 30;
	else
		glFirstColPercentWidth = 18;

	glSecondColPercentWidth = 70;
	::ReleaseDC(NULL, hdc);

	// Lets assume that the Graph OCX has not been registered
	m_bGraphOCXRegistered = FALSE;

	// Code for getting date in localized forms
	char szStrfTimeBuf[128], szDateBuf[128], szTempBuf[32];	
	time_t ltime;
	struct tm *today;
	
	char szRFC1766langID[6];
	LNG_GetLanguageInfo( LANG_INFO_RFC1766,szRFC1766langID);

	char szISO3166A3langID[6];
	
	time( &ltime );
	today = localtime( &ltime );
	CString csStr = "";
	_tcscpy(szDateBuf,"");

	char *pszCurrentLocale = NULL;

	if ( 0 != _tcsnicmp(szRFC1766langID,_T("EN"),2) )
	{
		LNG_GetLanguageInfo( LANG_INFO_ISO3166A3,szISO3166A3langID);
		pszCurrentLocale = setlocale(LC_ALL, NULL);
		setlocale(LC_ALL, szISO3166A3langID);
	}
	
	// Hours in a day
	for(i = 0;i < DAYSTATCOUNT;i++)
	{
		if ( !_tcsnicmp(szRFC1766langID,_T("EN"),2) && (i == 12) )
		{
			_tcscat(szDateBuf,_T("noon,"));
		}
		else
		{
			today->tm_hour = i;
			strftime( szStrfTimeBuf, 128, "%I", today );
			nVal = atoi(szStrfTimeBuf);
			sprintf(szTempBuf,"%d,",nVal);
			_tcscat(szDateBuf,szTempBuf);
		}
	}	
	m_csHourString = szDateBuf;

	// Week Days
	_tcscpy(szDateBuf,"");
	nVal = 0;
	for(i = 0;i < WEEKSTATCOUNT;i++)
	{
		today->tm_wday = i;
		strftime( szStrfTimeBuf, 128, "%a,", today );				
		_tcscat(szDateBuf,szStrfTimeBuf);
	}	
	m_csWeekString = szDateBuf;

	// Days in a month
	_tcscpy(szDateBuf,"");
	nVal = 0;
	for(i = 0;i < MONTHSTATCOUNT;i++)
	{
		today->tm_mday = i+1;
		strftime( szStrfTimeBuf, 128, "%d", today );
		nVal = atoi(szStrfTimeBuf);
		sprintf(szTempBuf,"%d,",nVal);
		_tcscat(szDateBuf,szTempBuf);
	}
	m_csMonthString = szDateBuf;

	// Months in an year
	_tcscpy(szDateBuf,"");
	nVal = 0;
	for(i = 0;i < YEARSTATCOUNT;i++)
	{
		today->tm_mon = i;
		strftime( szStrfTimeBuf, 128, "%b,", today );				
		_tcscat(szDateBuf,szStrfTimeBuf);
	}
	m_csYearString = szDateBuf;

	// Reset the locale back to the original one.
	if (pszCurrentLocale)
		setlocale(LC_ALL, pszCurrentLocale);	
}

CUsageStatisticsView::~CUsageStatisticsView()
{
	SetUsgStatsViewPtr(NULL);

	QCSharewareManager *pSWM = GetSharewareManager();
	if (pSWM)
	{
		pSWM->UnRegister(this);
	}	
}

void CUsageStatisticsView::Clear()
{
	pgSetSelection(m_paigeRef, 0, pgTextSize(m_paigeRef), 0, TRUE);
	pgDelete(m_paigeRef, NULL, best_way);
}

void CUsageStatisticsView::ResetCaret()
{
	pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);
}

void CUsageStatisticsView::HideCaret()
{
	pgSetHiliteStates(m_paigeRef, deactivate_verb, deactivate_verb | no_change_verb, TRUE);	
}



void CUsageStatisticsView::OnInitialUpdate()
{
	CPaigeEdtView::OnInitialUpdate();
	
	FARPROC lpDllEntryPoint;
	TCHAR	szOcxPath[_MAX_PATH + _MAX_FNAME + _MAX_EXT];
	_tcscpy(szOcxPath,ExecutableDir);
	_tcscat(szOcxPath,"EuGraph.ocx");

	HINSTANCE hLib = NULL;
	hLib = LoadLibrary(szOcxPath);

	try
	{
		if (hLib != NULL)
		{		
			// Find the entry point.
			(FARPROC&)lpDllEntryPoint = GetProcAddress(hLib, _T("DllRegisterServer"));
			if (lpDllEntryPoint != NULL)
			 if (S_OK != (*lpDllEntryPoint)())
			 {
				 // Write to log file
				 m_bGraphOCXRegistered = FALSE;
				 PutDebugLog(DEBUG_MASK_MISC, "Stats : EuGraph not registered successfully");
			 }
			 else
				 m_bGraphOCXRegistered = TRUE;
		}
		else
		{
			TCHAR szBuf[128];
			sprintf(szBuf,"Stats : EuGraph LoadLibrary failed with error - %d",GetLastError());
			PutDebugLog(DEBUG_MASK_MISC, szBuf);
		}

		SetUsgStatsViewPtr(this);

		ReloadData((short)gsPeriodSelected,(BOOL)gbMoreStatistics);
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in CUsageStatisticsView::OnInitialUpdate" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in CUsageStatisticsView::OnInitialUpdate" );
		// Write to log file
		PutDebugLog(DEBUG_MASK_MISC, "Stats : Exception while registering EuGraph or loading data");
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in CUsageStatisticsView::OnInitialUpdate" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception (not std::bad_alloc) in CUsageStatisticsView::OnInitialUpdate" );
		// Write to log file
		PutDebugLog(DEBUG_MASK_MISC, "Stats : Exception while registering EuGraph or loading data");
	}

	if (hLib) 
		FreeLibrary(hLib);

}


void CUsageStatisticsView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	
}

long CUsageStatisticsView::OnRefreshUsageStatisticsWindow( WPARAM	wParam, LPARAM	lParam )
{
	static UINT uiElapseSec = 0;
	uiElapseSec = lParam == 0 ? 2000 : 0;
		
	m_nTimerEventID = SetTimer(m_nTimerEventID , uiElapseSec, (TIMERPROC) NULL);
	return 1;
}

void CUsageStatisticsView::RefreshUsageStatisticsWindow(long wParam, long lParam)
{
	// Set the Paige view to readonly.
	SetReadOnly();

	// Clear the Paige view.
	Clear();

	// Allow Tables
	m_bAllowTables = TRUE;

	// Import HTML from the file.
	SetAllHTML(gStrUsgStatHTML, FALSE);

	// Set caret to the beginning.
	ResetCaret();
	HideCaret();

	// Update the scrollbars and scroll to the beginning.
	UpdateScrollBars(true);
	ScrollToCursor(0);	
}

void CUsageStatisticsView::ReloadData(short sSel,BOOL bMoreStatistics)
{
	if (!QCLoadTextData(IDT_USAGE_STATS_HTML, gStrUsgStatHTML))
		return;

	gbMoreStatistics = bMoreStatistics;
	CString	csHTML = gStrUsgStatHTML;
	
	if (!bMoreStatistics) FilterString(csHTML,"<!--Detailed Begin-->","<!--Detailed End-->");

	if (!g_pApp->m_TransManager->CanScoreJunk()) FilterString(csHTML,"<!--Junk Begin-->","<!--Junk End-->");

	gStrUsgStatHTML = csHTML;

	switch(sSel)
	{
	case 0:
		ReloadDataInHTML(STATDAY);
		break;
	case 1:
		ReloadDataInHTML(STATWEEK);
		break;
	case 2:
		ReloadDataInHTML(STATMONTH);
		break;
	case 3:
		ReloadDataInHTML(STATYEAR);
		break;
	default:
		break;
	}

	try
	{
		SetRedraw( FALSE );
		RefreshUsageStatisticsWindow();	
		if (m_bGraphOCXRegistered)
		{
			PgLoadAllObjects((pg_ref ) m_paigeRef);
			RefreshGraphicalStatistics(STATCOUNT);
		}
		SetRedraw( TRUE );
		Invalidate();
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in CUsageStatisticsView::ReloadData" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in CUsageStatisticsView::ReloadData" );
		// Do nothing, but just making sure that if we don't find the right OCX or it's not registered, we do not crash.
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in CUsageStatisticsView::ReloadData" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception (not std::bad_alloc) in CUsageStatisticsView::ReloadData" );
		// Do nothing, but just making sure that if we don't find the right OCX or it's not registered, we do not crash.
	}
}

void CUsageStatisticsView::FilterString(CString &csHTML,const char *szBegin,const char *szEnd)
{
	CString	csTempBuf;

	int nStart = 1, nStop = -1;

	do
	{
		nStart = csHTML.Find(szBegin);
		nStop = csHTML.Find(szEnd );
		if ((nStart != -1) && (nStop != -1))
		{
			// Take the part after the stop point...
			csTempBuf = csHTML.Right(csHTML.GetLength() - (nStop + strlen(szEnd)));
			// ...and the part before the start point...
			csHTML = csHTML.Left(nStart);
			// ...and concatenate them.
			csHTML += csTempBuf;
		}
	} while ((nStart != -1) && (nStop != -1));
}

void CUsageStatisticsView::SetPeriodSelection(short sSel)
{
	gsPeriodSelected = (StatTimePeriod)sSel;	
}

void CUsageStatisticsView::ShowMoreStatistics(BOOL bMoreStatistics)
{
	gbMoreStatistics = bMoreStatistics;
}

LPDISPATCH CUsageStatisticsView::GetIDispatch(COleClientItem *pItem)
{
    IUnknown* lpUnk = pItem->m_lpObject;

    LPDISPATCH lpDispatch = NULL;
    if (lpUnk->QueryInterface(IID_IDispatch, (void **)&lpDispatch) 
        != NOERROR)
    {
        TRACE0("Warning: does not support IDispatch!\n");
        return NULL;
    }

    ASSERT(lpDispatch != NULL);
    return lpDispatch;
}

// TODO : Code needs more streamlining here ... will be done shortly (when debugged completely).
void CUsageStatisticsView::RefreshGraphicalStatistics(long wParam, long lParam)
{	

	int i = 0;
	CString	csStr = "";

	// Update the graphs
	COleDocument* poleDoc = (COleDocument*) GetDocument();
	POSITION	pos;
	
	IDispatch	*pDispatch = NULL;

	if (poleDoc)
	{
		ASSERT_KINDOF( COleDocument, poleDoc );
		pos =  poleDoc->GetStartPosition();

		COleClientItem *pItem;
		int i = 0;
		while( pos != NULL)
		{
			pItem = (COleClientItem *)poleDoc->GetNextItem(pos);
			pDispatch = GetIDispatch(pItem);

			if (pDispatch)
			{
				csStr = ((PgCntrItem *)pItem)->GetIDString();
				
				if (! (csStr.CompareNoCase("Received")))
					m_Graph[US_STATRECEIVEDMAIL].AttachDispatch(pDispatch, TRUE);
				else if (! (csStr.CompareNoCase("Sent")))
					m_Graph[US_STATSENTMAIL].AttachDispatch(pDispatch, TRUE);
				else if (! (csStr.CompareNoCase("Junk")))
				{
					m_Graph[US_CALC_JUNKPERCENT].AttachDispatch(pDispatch, TRUE);
					m_Graph[US_CALC_JUNKPERCENT].SetYAxisScalingFactor(/*Magic value that means 0..100*/42);
				}
				else if (! (csStr.CompareNoCase("Usage")))
					m_Graph[US_STATFACETIME].AttachDispatch(pDispatch, TRUE);
				else if (! (csStr.CompareNoCase("FRR")))
					m_Graph[3].AttachDispatch(pDispatch, TRUE);
				else if (! (csStr.CompareNoCase("UA")))
					m_Graph[4].AttachDispatch(pDispatch, TRUE);
				
				i++;
			}
		}
	}

	long	lCurData[31];
	long	lLastData[31];
	long	lAvData[31];

	long	*plGraphData[3];	// Max for now
	short	sSeriesCount = 3;

	ULONG	ulElapsedUnits = 0;
	CString	csLegendString;

	CString csFRRStr = "Forward, Reply, Redirect,";	
	csFRRStr.LoadString(IDS_STATISTICS_FRR_STRING);
	CString csUAStr = "Reading, Composing, Other,";
	csUAStr.LoadString(IDS_STATISTICS_UA_STRING);	

	struct tm	tmCurTime;
	time_t	theTime;	
	time(&theTime);
	tmCurTime = *(localtime( &theTime )); /* Convert to local time. */	

	csLegendString = csPeriodStrings[gsPeriodSelected][0];
	csLegendString += ',';
	csLegendString += csPeriodStrings[gsPeriodSelected][1];
	csLegendString +=  ',';
	csLegendString += csAverageString;
	csLegendString += ',';

	if (wParam == STATCOUNT)
	{	
		switch(gsPeriodSelected)
		{
			case (short) STATDAY:
				for (i = 0;i < STATCOUNT; i++)
				{// Set the Graph Parameters
					if (m_Graph[i] != NULL)
					{
						ulElapsedUnits =  i == US_CALC_JUNKPERCENT ? 0 : CalcElapsedUnits(STATDAY);
						if (ulElapsedUnits == 0)
							sSeriesCount = 2;

						if (i == US_STATFACETIME)
						{
							m_Graph[i].SetYAxisScalingFactor(60);

							for (int j = 0; j < DAYSTATCOUNT; j++)
							{
								lCurData[j] = gStatData->numStats[i].current.day[j]/60;
								lLastData[j] = gStatData->numStats[i].last.day[j]/60;
							}
							
							plGraphData[0] = lCurData;
							plGraphData[1] = lLastData;
						}
						else
						{
							plGraphData[0] = gStatData->numStats[i].current.day;
							plGraphData[1] = gStatData->numStats[i].last.day;
						}

						if (ulElapsedUnits > 0)
						{
							if (i == US_STATFACETIME)
							{
								for (int j = 0; j < DAYSTATCOUNT; j++)
									lAvData[j] = (gStatData->numStats[i].average.day[j])/ (60 * ulElapsedUnits);
							}
							else
							{
								for (int j = 0; j < DAYSTATCOUNT; j++)
									lAvData[j] = (gStatData->numStats[i].average.day[j])/ ulElapsedUnits;
							}
						}

						plGraphData[2] = lAvData;

						memcpy((void *)lCurData, plGraphData[0], DAYSTATCOUNT * sizeof(long));
						for (int j = tmCurTime.tm_hour; j < DAYSTATCOUNT; j++)
							lCurData[j] = lCurData[j] == 0 ? -1 : lCurData[j];

						plGraphData[0] = lCurData;
						
						DrawGraph(&m_Graph[i], sSeriesCount, m_csHourString, DAYSTATCOUNT, plGraphData, csLegendString, 1, 3);
					}
				}

				break;

			case (short) STATWEEK:				
				for (i = 0;i < STATCOUNT; i++)
				{// Set the Graph Parameters
					if (m_Graph[i] != NULL)
					{
						// Set the Graph Parameters
						ulElapsedUnits = i == US_CALC_JUNKPERCENT ? 0 : CalcElapsedUnits(STATWEEK);
						if (ulElapsedUnits == 0)
							sSeriesCount = 2;

						if (i == US_STATFACETIME)
						{
							m_Graph[i].SetYAxisScalingFactor(60);

							for (int j = 0; j < WEEKSTATCOUNT; j++)
							{
								lCurData[j] = gStatData->numStats[i].current.week[j]/60;
								lLastData[j] = gStatData->numStats[i].last.week[j]/60;
							}

							plGraphData[0] = lCurData;
							plGraphData[1] = lLastData;						
							
						}
						else
						{
							plGraphData[0] = gStatData->numStats[i].current.week;
							plGraphData[1] = gStatData->numStats[i].last.week;							
						}

						if (ulElapsedUnits > 0)
						{
							if (i == US_STATFACETIME)
							{
								for (int j = 0; j < WEEKSTATCOUNT; j++)								
									lAvData[j] = (gStatData->numStats[i].average.week[j])/ (60 * ulElapsedUnits);
							}
							else
							{
								for (int j = 0; j < WEEKSTATCOUNT; j++)
									lAvData[j] = (gStatData->numStats[i].average.week[j])/ ulElapsedUnits;
							}
						}

						plGraphData[2] = lAvData;

						memcpy((void *)lCurData, plGraphData[0], WEEKSTATCOUNT * sizeof(long));
						for (int j = tmCurTime.tm_wday; j < WEEKSTATCOUNT; j++)
							lCurData[j] = lCurData[j] == 0 ? -1 : lCurData[j];

						plGraphData[0] = lCurData;						

						DrawGraph(&m_Graph[i], sSeriesCount, m_csWeekString, WEEKSTATCOUNT, plGraphData, csLegendString, 0, 0);
						
					}
				}
				break;

			case (short) STATMONTH:				
				for (i = 0;i < STATCOUNT; i++)
				{
					if (m_Graph[i] != NULL)
					{
						ulElapsedUnits = i == US_CALC_JUNKPERCENT ? 0 : CalcElapsedUnits(STATMONTH);
						if (ulElapsedUnits == 0)
							sSeriesCount = 2;						

						if (i == US_STATFACETIME)
						{
							m_Graph[i].SetYAxisScalingFactor(60);
							for (int j = 0; j < MONTHSTATCOUNT; j++)
							{
								lCurData[j] = gStatData->numStats[i].current.month[j]/60;
								lLastData[j] = gStatData->numStats[i].last.month[j]/60;								
							}
							plGraphData[0] = lCurData;
							plGraphData[1] = lLastData;							
							
						}
						else
						{
							plGraphData[0] = gStatData->numStats[i].current.month;
							plGraphData[1] = gStatData->numStats[i].last.month;							
						}

						if (ulElapsedUnits > 0)
						{
							if (i == US_STATFACETIME)
							{
								for (int j = 0; j < MONTHSTATCOUNT; j++)								
									lAvData[j] = (gStatData->numStats[i].average.month[j])/ (60 * ulElapsedUnits);
							}
							else
							{
								for (int j = 0; j < MONTHSTATCOUNT; j++)
									lAvData[j] = (gStatData->numStats[i].average.month[j])/ ulElapsedUnits;
							}
						}

						plGraphData[2] = lAvData;

						memcpy((void *)lCurData, plGraphData[0], MONTHSTATCOUNT * sizeof(long));
						for (int j = tmCurTime.tm_mday - 1; j < MONTHSTATCOUNT; j++)
							lCurData[j] = lCurData[j] == 0 ? -1 : lCurData[j];

						plGraphData[0] = lCurData;						

						DrawGraph(&m_Graph[i], sSeriesCount, m_csMonthString, MONTHSTATCOUNT, plGraphData, csLegendString, 5, 4);
					}
				}
				
				break;

			case (short) STATYEAR:
				for (i = 0;i < STATCOUNT; i++)
				{
					if (m_Graph[i] != NULL)
					{	
						
						ulElapsedUnits = i == US_CALC_JUNKPERCENT ? 0 : CalcElapsedUnits(STATYEAR);
						if (ulElapsedUnits == 0)
							sSeriesCount = 2;						

						if (i == US_STATFACETIME)
						{
							m_Graph[i].SetYAxisScalingFactor(60);
							for (int j = 0; j < YEARSTATCOUNT; j++)
							{
								lCurData[j] = gStatData->numStats[i].current.year[j]/60;
								lLastData[j] = gStatData->numStats[i].last.year[j]/60;								
							}

							plGraphData[0] = lCurData;
							plGraphData[1] = lLastData;
							
							
						}
						else
						{
							plGraphData[0] = gStatData->numStats[i].current.year;
							plGraphData[1] = gStatData->numStats[i].last.year;							
						}

						if (ulElapsedUnits > 0)
						{
							if (i == US_STATFACETIME)
							{
								for (int j = 0; j < YEARSTATCOUNT; j++)								
									lAvData[j] = (gStatData->numStats[i].average.year[j])/ (60 * ulElapsedUnits);
							}
							else
							{
								for (int j = 0; j < YEARSTATCOUNT; j++)
									lAvData[j] = (gStatData->numStats[i].average.year[j])/ ulElapsedUnits;
							}
						}

						plGraphData[2] = lAvData;
						
						memcpy((void *)lCurData, plGraphData[0], YEARSTATCOUNT * sizeof(long));
						for (int j = tmCurTime.tm_mon; j < YEARSTATCOUNT; j++)
							lCurData[j] = lCurData[j] == 0 ? -1 : lCurData[j];

						plGraphData[0] = lCurData;
						

						DrawGraph(&m_Graph[i], sSeriesCount, m_csYearString,YEARSTATCOUNT, plGraphData, csLegendString, 1, 1);
						
					}
				}
				break;
			default:
				break;
				
		}

		if (gbMoreStatistics)
		{
			static long lFTValues[3];
			static long lCurrent[3];
			static long lPrevious[3];
			static long lAverage[3];

			if (ulElapsedUnits > 0)
				m_Graph[3].SetSeriesCount(3);
			else
				m_Graph[3].SetSeriesCount(2);

			m_Graph[3].SetXAxisNumberOfIntervals(3);
			m_Graph[3].SetXAxisLabelInformation(csFRRStr,0,0);

			// This is a little tricky. We need to pass three series of data to the graph
			// Graph has support for multiple bars per series, but each series bar is drawn
			// in it's appropriate place in the section. One series forms a section & there
			// could be multiple sections per graph.
			// So in all we have 3 series & 3 sections

			lCurrent[0]	= (long)gdFValues[0];
			lCurrent[1] = (long)gdRyValues[0];
			lCurrent[2] = (long)gdRdtValues[0];

			lPrevious[0]	= (long)gdFValues[1];
			lPrevious[1] = (long)gdRyValues[1];
			lPrevious[2] = (long)gdRdtValues[1];
			
			m_Graph[3].SetSeriesData(0, lCurrent, 3, m_GraphColors[0],csPeriodStrings[gsPeriodSelected][0]);
			m_Graph[3].SetSeriesData(0, lPrevious, 3, m_GraphColors[1],csPeriodStrings[gsPeriodSelected][1]);			

			if (ulElapsedUnits > 0)
			{
				lAverage[0]	= (long)gdFValues[2];
				lAverage[1] = (long)gdRyValues[2];
				lAverage[2] = (long)gdRdtValues[2];

				m_Graph[3].SetSeriesData(0, lAverage, 3, m_GraphColors[2],csAverageString);
			}

			m_Graph[3].RedrawGraph();

			for (i = 0; i < 3; i++)
				lFTValues[i] = gsFacetimeValues[i];
			
			m_Graph[4].PieGraph(lFTValues, 3, m_GraphColors, csUAStr);
					
		}
	}	
}

void CUsageStatisticsView::Notify( QCCommandObject*	pObject,COMMAND_ACTION_TYPE	theAction,void* pData)
{
	if (theAction == CA_SWM_CHANGE_FEATURE) 
	{
		if (!UsingFullFeatureSet())
		{
			// Moving from FULL FEATURED to REDUCED
			
			// Close Usage Statistics View
			::SendMessage((this->GetParent())->m_hWnd,WM_CLOSE,0,0);			
		}    
	}	
}

void CUsageStatisticsView::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	// Disable "Save"
	pCmdUI->Enable(FALSE);
	
}

void CUsageStatisticsView::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	// Disable "Save As"
	pCmdUI->Enable(FALSE);	
}


void CUsageStatisticsView::DrawGraph(_DEuGraph *pGraph, short sSeriesCount, CString csXString, short sXIntervals, long**ppData, CString csLegendString, short sLabelDisplayStartOffset, short sLabelDisplaySkipCount)
{	
	if (pGraph)	
	{
		pGraph->SetSeriesCount(sSeriesCount);	
		pGraph->SetXAxisNumberOfIntervals(sXIntervals);
		pGraph->SetXAxisLabelInformation(csXString, sLabelDisplayStartOffset, sLabelDisplaySkipCount);

		int nMatchLoc = -1;
		nMatchLoc = csLegendString.Find(',');
		CString csStringToDisplay = csLegendString.Left(nMatchLoc);		

		for (int i = 0;i < sSeriesCount && i < 3; i++)
		{
			if (ppData[i])
				pGraph->SetSeriesData(m_GraphStyles[i], ppData[i], sXIntervals, m_GraphColors[i],csStringToDisplay);

			csLegendString = csLegendString.Right(csLegendString.GetLength() - (nMatchLoc + 1));
			nMatchLoc = csLegendString.Find(',');
			csStringToDisplay = csLegendString.Left(nMatchLoc);
		}
		pGraph->RedrawGraph();
	}

}

void CUsageStatisticsView::OnTimer(UINT nIDEvent) 
{
	CPaigeEdtView::OnTimer(nIDEvent);

	if (nIDEvent == m_nTimerEventID)
	{
		try
		{
			SetRedraw( FALSE );
			ReloadDataInHTML(gsPeriodSelected);
			RefreshUsageStatisticsWindow(0, 0);
			if (m_bGraphOCXRegistered)
			{
				PgLoadAllObjects((pg_ref ) m_paigeRef);
				RefreshGraphicalStatistics(STATCOUNT);
			}

			SetRedraw( TRUE );
			Invalidate();
		}
		catch (CMemoryException * /* pMemoryException */)
		{
			// Catastrophic memory exception - rethrow
			ASSERT( !"Rethrowing CMemoryException in CUsageStatisticsView::OnTimer" );
			throw;
		}
		catch (CException * pException)
		{
			// Other MFC exception
			pException->Delete();
			ASSERT( !"Caught CException (not CMemoryException) in CUsageStatisticsView::OnTimer" );
			// Do nothing, but just making sure that if we don't find the right OCX or it's not registered, we do not crash.
		}
		catch (std::bad_alloc & /* exception */)
		{
			// Catastrophic memory exception - rethrow
			ASSERT( !"Rethrowing std::bad_alloc in CUsageStatisticsView::OnTimer" );
			throw;
		}
		catch (std::exception & /* exception */)
		{
			ASSERT( !"Caught std::exception (not std::bad_alloc) in CUsageStatisticsView::OnTimer" );
			// Do nothing, but just making sure that if we don't find the right OCX or it's not registered, we do not crash.
		}


		KillTimer(m_nTimerEventID);			
	}
}

void CUsageStatisticsView::OnDestroy() 
{
	KillTimer(m_nTimerEventID);
	CPaigeEdtView::OnDestroy();	
}

void CUsageStatisticsView::OnSize(UINT nType, int cx, int cy) 
{
	CPaigeEdtView::OnSize(nType, cx, cy);
	m_nTimerEventID = SetTimer(m_nTimerEventID , 5, (TIMERPROC) NULL);	
}


LRESULT CUsageStatisticsView::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	long lPrevWidth = glFirstColPercentWidth ;

	if (LOWORD(lParam) > 800)
		glFirstColPercentWidth = 30;
	else
		glFirstColPercentWidth = 18;
		
	glSecondColPercentWidth = 70;	

	if (lPrevWidth != glFirstColPercentWidth)
		ReloadData((short)gsPeriodSelected,(BOOL)gbMoreStatistics);		
	
	return CPaigeEdtView::OnDisplayChange(wParam, lParam);
}

void CUsageStatisticsView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Post this message to the UsageStatisticsDlgBar so that it handles the acclerator keys (if any)
	// You might wanna read the Hack Alert in CUsageStatisticsDlgBar::PreTranslateMessage() for more information.

	// Make sure that we don't post the message when it's either ALT, Arrow Keys (Up, Down, Left, Right) or F10 Key ('coz Shift-F10 = right click).
	if ( (nChar != VK_MENU) && 
		 (nChar != VK_UP) && 
		 (nChar != VK_DOWN) && 
		 (nChar != VK_LEFT) && 
		 (nChar != VK_RIGHT) && 
		 (nChar != VK_F10) )
			::PostMessage(((CUsageStatisticsFrame *)GetParent())->m_secDialogBar.m_hWnd,WM_SYSKEYDOWN,WPARAM(nChar),MAKELPARAM(nRepCnt,nFlags));

	CPaigeEdtView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CUsageStatisticsView::PreTranslateMessage(MSG* pMsg) 
{
	if ((WM_KEYDOWN == pMsg->message) && (VK_TAB == pMsg->wParam))
	{
		if (::GetKeyState(VK_SHIFT) >= 0)  // i.e Shift Key not pressed
			::SetFocus(( ((CUsageStatisticsFrame *)GetParent())->m_secDialogBar.GetDlgItem(IDC_US_PERIOD_CB) )->m_hWnd);
		else
			::SetFocus(( ((CUsageStatisticsFrame *)GetParent())->m_secDialogBar.GetDlgItem(IDC_US_MORE_STATS_BTN) )->m_hWnd);
	}

	return CPaigeEdtView::PreTranslateMessage(pMsg);
}

void CUsageStatisticsView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar)
    {    
		case VK_UP : 
			SendMessage(WM_VSCROLL,SB_LINEUP,NULL);
			return;
		case VK_DOWN:
			SendMessage(WM_VSCROLL,SB_LINEDOWN,NULL);
			return;
		case VK_HOME:
			SendMessage(WM_VSCROLL,SB_TOP,NULL);
			return;
		case VK_END:
			SendMessage(WM_VSCROLL,SB_BOTTOM,NULL);
			return;
		case VK_LEFT:	// left key simulates page-up
			SendMessage(WM_VSCROLL,SB_PAGEUP,NULL);
			return;		
		case VK_RIGHT:	// right key simulates page-down
			SendMessage(WM_VSCROLL,SB_PAGEDOWN,NULL);		
			return;
		default:
			break;
	}
	
	CPaigeEdtView::OnKeyDown(nChar, nRepCnt, nFlags);
}