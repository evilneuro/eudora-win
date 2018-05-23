// StatMng.cpp
//
// Copyright (c) 2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

// This file has many functions similar to the MAC version. Especially the logic for calculating statistics.

#include "stdafx.h"
#include "statmng.h"

#include "UsgStatsDoc.h"
#include "UsgStatsView.h"
#include "utils.h"

#include "guiutils.h"

#include "tocframe.h"
#include "tocdoc.h"
#include "ReadMessageFrame.h"
#include "CompMessageFrame.h"
#include "tocdoc.h"

#include "Facetime.h"
#include "qcutils.h"

#include "DebugNewHelpers.h"

#include "xml_parser_base.h"

// Force everything to warning level 3.
#pragma warning ( push , 3 ) 

// Do not warn abt the conversion from 'type1' to 'type2', possible loss of data. 
// The instances of converting double to long are intentional
#pragma warning (disable: 4244)

#define STATCOUNT BEGINSHORTSTATS
#define SHORTSTATCOUNT (ENDSTATS - BEGINSHORTSTATS)

#define US_SUCCESS				0
#define	US_GENERIC ERROR		4000
#define US_MEMORY_ALLOC_ERROR	4001
#define US_FILE_IO_ERROR		4002

// Class for the XML Parser ...
enum element_ids {  EudoraStats,
					version,	
					startTime,	
					currentTime,
					
					current,	
					previous,	
					averageSum,	
					total,

					day,		
					week,		
					month,		
					year,							
					
					// For the following IDs, make sure that the order is same as the StatType Standard Numeric stats.
					receivedMail,					
					sentMail,	
					usageSeconds,

					// Junk stuff
					scoredJunk,
					scoredNotJunk,
					whiteList,
					falsePositives,
					falseNegatives,
					falseWhiteList,

					// calced junk stuff
					calc_junkTotal,
					calc_total,
					calc_junkPercent,

					receivedAttachments,
					sentAttachments,
					readMessages,
					forwardMessages,
					replyMessages,
					redirectMessages,
					usageRead,	
					usageCompose,
					usageOther,	};

#define CALCED_STAT(i) (calc_junkTotal<=i && i<=calc_junkPercent)

static inline int CalcPercentage(int n, int d) {if (n<=0 || d<=0) return 0;  int p = (n*100)/d; if (p>100) p = 100; return p;}

// XML Map - make sure that the Ids defined above are in sync with this.
_ElementMap MyElementMap[] = {	EudoraStats,			"EudoraStats",
								version,				"version",
								startTime,				"startTime",
								currentTime,			"currentTime",
								current,				"current",
								previous,				"previous",
								averageSum,				"averageSum",
								total,					"total",

								day,					"day",
								week,					"week",
								month,					"month",
								year,					"year",								
								
								receivedMail,			"receivedMail",
								sentMail,				"sentMail",	
								usageSeconds,			"usageSeconds",

								// Junk stuff
								scoredJunk,	"scoredJunk",
								scoredNotJunk,	"scoredNotJunk",
								whiteList,	"whiteList",
								falsePositives,	"falsePositives",
								falseNegatives,	"falseNegatives",
								falseWhiteList, "falseWhiteList",

								// calced junk stuff omitted on purpose

								receivedAttachments,	"receivedAttachments",
								sentAttachments,		"sentAttachments",
								readMessages,			"readMessages",
								forwardMessages,		"forwardMessages",
								replyMessages,			"replyMessages",
								redirectMessages,		"redirectMessages",
								usageRead,				"usageRead",
								usageCompose,			"usageCompose",
								usageOther,				"usageOther" ,

								id_none,				"Always the last one"};

#include <deque>
using namespace std;

typedef deque<int > IdsQueue;

class StatsXmlParser : public XmlParser
{
    char	m_outBuffer[1152];
	short	msLevel;

	IdsQueue  m_IdsQueue;

	FILE	*mfpXML;
	TCHAR	*mProcessingBuffer;

public:
    StatsXmlParser()
	{	
		*m_outBuffer = 0;
		msLevel = 0;
		mfpXML = NULL;
		mProcessingBuffer = NULL;
	}
    ~StatsXmlParser(){};

private:
    bool initElementMap( _ElementMap** map ) 
	{
        *map = MyElementMap;		
		return 1;
    }

    void handleData( int id, const char* data, int len ) 
	{
        strcat( m_outBuffer, "=" );
        strncat( m_outBuffer, data, len );
    }

    int startElement( int id, const char* name, const char** atts );        
	int endElement( int id, const char* name );	

public:
	void WriteStartElement( int id, char** attrs = NULL, bool linebreak = false);
	void WriteEndElement( int id, bool linebreak = false);
	void WriteElementData( int id, const char *szDataString );
	void WriteDataField( int id, long lValue);
	void WriteDataField( int id, const char *szDataString );
	void WriteDataField( int id, long *plValue, short sCount);

	bool OpenFileForWriting(char *szFileName)
	{
		mfpXML = fopen( szFileName, "wb");
		
		return (mfpXML != NULL);
	}

	void CloseFile()
	{
		fclose(mfpXML);
	}
	
	void AddLineBreak() { if (mfpXML) fprintf(mfpXML,"\n"); }

};
// XML Parser class ends here

// Globals
enum {	THISSTAT = 0,
		LASTSTAT,
		AVESTAT,
		TOTALSTAT };

const short gsSaveAfterXOperations = 25;
const int nSaveStatsFacetimeSecs = 10 * 60;
int gnSaveStatsAbsoluteSecs = 15 * 60;

static tm					gCurrentTime;
static BOOL					gStatsDirty;
static FMBHANDLE			gFaceMeasure;
static short				dataCountTab[] = { 24,7,31,12 };
static BOOL					gbStatsDirty;
static TCHAR gszStatsFile [] = "Eudora61Stats.xml";
static TCHAR gszOldStatsFile [] = "EudoraStats.xml";
static CUsageStatisticsDoc	*gpUsgStatsDoc = NULL;
static CUsageStatisticsView *gpUsgStatsView = NULL;
static QCFaceTime*			m_FaceTime;
static FacetimeMode			gFacetimeMode;

static BOOL					gbForceSave = FALSE;

StatDataPtr					gStatData;
CString						gStrUsgStatHTML = "";
StatTimePeriod				gsPeriodSelected = STATDAY;
BOOL						gbMoreStatistics = FALSE;
long						glFirstColPercentWidth = 18;
long						glSecondColPercentWidth = 70;


double						gdRcvdAttachValues[4];
double						gdReadMessageValues[4];
double						gdSentAttachValues[4];
double						gdFValues[3], gdRyValues[3], gdRdtValues[3];
short						gsFacetimeValues[3];

char*						m_XMLFilebuf = NULL;

extern CString EudoraDir;

HANDLE gStatUpdateReqEvent;

// Make sure that the order is same as the StatType Standard Numeric stats. These should not be moved to string table for localization
static TCHAR csUsgStatsHTMLTags[23][4][20] = { {"<!--RMT-->",		"<!--RML-->",		"<!--RA-->",		"<!--RT-->"		},// US_STATRECEIVEDMAIL
											{"<!--SMT-->",		"<!--SML-->",		"<!--SA-->",		"<!--ST-->"		},// US_STATSENTMAIL
											{"<!--UT-->",		"<!--UY-->",		"<!--UA-->",		"<!--UTo-->"	},// US_STATFACETIME
											{"<!--SJT-->",		"<!--SJL-->",		"<!--SJA-->",		"<!--SJTo-->"	},// US_STATSCOREDJUNK
											{"<!--SNJT-->",		"<!--SNJL-->",		"<!--SNJA-->",		"<!--SNJTo-->"	},// US_STATSCOREDNOTJUNK
											{"<!--WLT-->",		"<!--WLL-->",		"<!--WLA-->",		"<!--WLTo-->"	},// US_STATWHITELIST
											{"<!--FPT-->",		"<!--FPL-->",		"<!--FPA-->",		"<!--FPTo-->"	},// US_STATFALSEPOSITIVES
											{"<!--FNT-->",		"<!--FNL-->",		"<!--FNA-->",		"<!--FNTo-->"	},// US_STATFALSENEGATIVES
											{"<!--FWT-->",		"<!--FWL-->",		"<!--FWA-->",		"<!--FWTo-->"	},// US_STATFALSEWHITELIST
											{"<!--CJTT-->",		"<!--CJTL-->",		"<!--CJTA-->",		"<!--CJTo-->"	},// US_CALC_JUNKTOTAL
											{"<!--CTT-->",		"<!--CTL-->",		"<!--CTA-->",		"<!--CTTo-->"	},// US_CALC_TOTAL
											{"<!--JPT-->",		"<!--JPL-->",		"<!--JPA-->",		"<!--JPTo-->"	},// US_CALC_JUNKPERCENT
											{"<!--NULL-->",		"<!--NULL-->",		"<!--NULL-->",		"<!--NULL-->"	},// Filler for BEGINSHORTS
											{"<!--RAtT-->",		"<!--RAtL-->",		"<!--RAtA-->",		"<!--RAt-->"	},// US_STATRECEIVEDATTACH
											{"<!--SAtT-->",		"<!--SAtL-->",		"<!--SAtA-->",		"<!--SAt-->"	},// US_STATSENTATTACH
											{"<!--RMRdT-->",	"<!--RMRdL-->",		"<!--RMRdA-->",		"<!--RRdT-->"	},// US_STATREADMSG
											{"<!--NULL-->",		"<!--NULL-->",		"<!--NULL-->",		"<!--NULL-->"	},// US_UNUSED
											{"<!--FRR:FT-->",	"<!--FRR:FL-->",	"<!--FRR:FA-->",	"<!--NULL-->"	},// US_STATFORWARDMSG
											{"<!--FRR:RyT-->",	"<!--FRR:RyL-->",	"<!--FRR:RyA-->",	"<!--NULL-->"	},// US_STATREPLYMSG
											{"<!--FRR:RdtT-->", "<!--FRR:RdtL-->",	"<!--FRR:RdtA-->",	"<!--NULL-->"	},// US_STATREDIRECTMSG
											{"<!--UA:RM-->",	"<!--NULL-->",		"<!--NULL-->",		"<!--NULL-->"	},// US_STATFACETIMEREAD,
											{"<!--UA:CM-->",	"<!--NULL-->",		"<!--NULL-->",		"<!--NULL-->"	},// US_STATFACETIMECOMPOSE,
											{"<!--UA:O-->",		"<!--NULL-->",		"<!--NULL-->",		"<!--NULL-->"	},// US_STATFACETIMEOTHER,
										};

// These are been loaded from String Table (for localization purposes), but are initialized here anyway.
TCHAR csPeriodStrings [4][3][32] = { {"today" , "yesterday", "day" },
									{"this week" , "last week", "week"},
									{"this month" , "last month", "month"},
									{"this year" , "last year", "year"} };

TCHAR csProjectedString[32] = " (projected)";
TCHAR csAverageString[32] = "average";

typedef deque <UpdateParam>		StatReqQ;
static StatReqQ					gStatReqQ;
static CCriticalSection			gStatReqQCriticalSection;

static BOOL						gStatsInitialized = FALSE;
static BOOL						gbShutDown = FALSE;

static DWORD LoadStats(void);
static DWORD SaveStats(BOOL force);
static DWORD SaveStatsAsXML(char *pFileName = NULL);

static void UpdateNumStatLo(StatType which, long value);
static void UpdateNumStatWithTime(StatType which, long value, uLong seconds);

static void CheckTimeChange(void);
static void UpdateFacetimeLo(FacetimeMode facetimeMode = FACETIMEMODENULL);

static void GetStatTotals(NumericStats *stats,StatTimePeriod period,long values[]);

static long SumArray(long *values,short len);
static void AddToAve(ULONG *to,ULONG *from,short length);

static void UpdateHTMLContent(StatType which, short value);

static void ComposeShortStats(StatType type, double dValues[]);
static void ComposeFRR(double dFValues[], double dRyValues[], double dRdtValues[]);
static void ComposeUsageActivities(short sValues[]);
static void ComposeDetailedFaceTime(FacetimeMode facetimeMode = FACETIMEMODENULL);
static void ComposeTimeStringInStatsFormat(long lTime, LPTSTR lpszDate);

static void DisplayUsage(ULONG ulValue, char *szBuf);
static void DisplayBasicNumStats(StatTimePeriod stTimePeriod, StatType which);
static void DisplayShortStats(StatType which, double* dValues);
static void DisplayJunkJunk(StatTimePeriod stTimePeriod);

static int ClassifyXMLString(int nRSID, int nCPAID, int nDWMYID, const char* szString);    
static int ParseXMLStringIntoNumericStats(const char* szString, NumericStats* pNum, int nCPAID);
static int ParseXMLStringIntoShortStats(const char* szString, ShortNumericStats* pShort, int nCPAID);
static int ParseStringInArry(const char *szString, long* plValue, short slValueArraySize);
static void SaveNumericStatsInXML(StatsXmlParser *pXMLParser, int id, PeriodicStats *pNumericStats);
static void SaveShortStatsInXML(StatsXmlParser *pXMLParser, int id, ShortPeriodicStats *pShortStats);

void CALLBACK SaveStatsTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
static UINT CreateStatsUpdateReqThread(LPVOID lpParameter);

typedef enum {opPlus, opMinus, opPercent} OpType;
static void UpdateCalculatedStats(void);
static void NumericStatOperation(StatType destination,StatType increment,OpType op);
static void PeriodicStatOperation(PeriodicStats *destP, PeriodicStats *incP, OpType op);
static void LongArrayOperation(long *array,long *sub,short n,OpType op);

// Thread for Updating the stats
static UINT USUpdateNumStatInternal(UpdateParam strUpdateParam);


static void ParsePeriodStringInArray()
{
	int nMatchLoc = 0, i = 0, nPeriod = 0;
	CString csExtractedString;

	try
	{
		for (nPeriod = 0; nPeriod < 4; nPeriod++)
		{
			CString csStr;
			csStr.LoadString(IDS_STATISTICS_DAY_PERIOD_STRINGS + nPeriod);
			i = 0;
			nMatchLoc = 0;

			if (!csStr.IsEmpty())
			{
				while(nMatchLoc != -1)
				{
					nMatchLoc = csStr.Find(';');
					if(nMatchLoc != -1)
					{
						csExtractedString = csStr.Left(nMatchLoc);
						csStr = csStr.Right(csStr.GetLength() - (nMatchLoc + 1));
						_tcscpy(csPeriodStrings[nPeriod][i],csExtractedString);
						i++;
					}
				}
			}
		}
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in ParsePeriodStringInArray" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in ParsePeriodStringInArray" );
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in ParsePeriodStringInArray" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception (not std::bad_alloc) in ParsePeriodStringInArray" );
	}
}

void InitStats(void)
{
	gStatsInitialized = FALSE;

	if(GetIniShort(IDS_INI_STATISTICS_DISABLE))
	{
		// Statistic collection is disabled
		TRACE("Statistics collection is disabled");
		return;	
	}
		
	gnSaveStatsAbsoluteSecs = GetIniShort(IDS_INI_STATISTICS_SAVE_INTERVAL) * 60;
	if (gnSaveStatsAbsoluteSecs == 0)
	{
		gnSaveStatsAbsoluteSecs = 15 * 60;
		SetIniShort(IDS_INI_STATISTICS_SAVE_INTERVAL,15);
	}

	gbForceSave = GetIniShort(IDS_INI_STATISTICS_FORCE_SAVE);

	ParsePeriodStringInArray();
	CRString csProjectedStr(IDS_STATISTICS_PROJECTED_STRING);
	_tcscpy(csProjectedString,csProjectedStr);

	CRString csAverageStr(IDS_STATISTICS_AVERAGE_STRING);
	_tcscpy(csAverageString,csAverageStr);

	// Create Event object for Text Change Request
    gStatUpdateReqEvent = CreateEvent(
                            NULL,
                            TRUE,
                            FALSE,
                            NULL);

    if(gStatUpdateReqEvent == NULL)
    {
        ShutdownStats();
        return;
    }

	gStatReqQ.clear();

	AfxBeginThread(CreateStatsUpdateReqThread, 
				   NULL,
				   0,
				   0,
				   0);	

	SetTimer(NULL,0,gnSaveStatsAbsoluteSecs * 1000, TIMERPROC(&SaveStatsTimerProc));

	LoadStats();

	m_FaceTime = NULL;
	m_FaceTime = QCFaceTime::Create();
	if (m_FaceTime)
		m_FaceTime->Start();

	gStatsInitialized = TRUE;
	gbShutDown = FALSE;

	if (!QCLoadTextData(IDT_USAGE_STATS_HTML, gStrUsgStatHTML))
		return;

}

void ShutdownStats(void)
{
	gbShutDown = TRUE;
	
	SetEvent(gStatUpdateReqEvent);

	if (!gStatData) return;
	UpdateFacetime();
	SaveStats(true);

	if (gStatData)
	{
		delete gStatData;
		gStatData = NULL;
	}

	if (m_XMLFilebuf)
	{
		delete []m_XMLFilebuf;
		m_XMLFilebuf = NULL;
	}

	// Close the Event Handle
	CloseHandle (gStatUpdateReqEvent);

	if (m_FaceTime)
	{
		m_FaceTime->Dispose();
		m_FaceTime = NULL;		
	}
	gStatsInitialized = FALSE;

}
 
void UpdateNumStat(StatType which, short sValue, long lTimeStamp)
{
	if(gStatsInitialized)
	{
		UpdateParam strUpdateParam;	
		
		strUpdateParam.which = which;
		strUpdateParam.sValue = sValue;
		strUpdateParam.lTimeStamp = lTimeStamp;

		CSingleLock singleLock(&gStatReqQCriticalSection,TRUE);
		try
		{
			gStatReqQ.push_front(strUpdateParam);
		}
		catch (CMemoryException * /* pMemoryException */)
		{
			// Catastrophic memory exception - rethrow
			ASSERT( !"Rethrowing CMemoryException in UpdateNumStat" );
			throw;
		}
		catch (CException * pException)
		{
			// Other MFC exception
			pException->Delete();
			ASSERT( !"Caught CException (not CMemoryException) in UpdateNumStat" );
		}
		catch (std::bad_alloc & /* exception */)
		{
			// Catastrophic memory exception - rethrow
			ASSERT( !"Rethrowing std::bad_alloc in UpdateNumStat" );
			throw;
		}
		catch (std::exception & /* exception */)
		{
			ASSERT( !"Caught std::exception (not std::bad_alloc) in UpdateNumStat" );
		}

		singleLock.Unlock();

		// We've got an update request ..fire the event.
		SetEvent(gStatUpdateReqEvent);	
	}
}

UINT CreateStatsUpdateReqThread(LPVOID lpParameter)
{
	UpdateParam    strUpdateParam = {StatType(0), 0, 0};

    while (TRUE)
    {        
        WaitForSingleObject(gStatUpdateReqEvent,INFINITE);

        // If shutdown in progress then just break from the loop
        if (gbShutDown)
            break;

        // Make sure we reset the event
        ResetEvent(gStatUpdateReqEvent);

		try
		{
			while ( !gStatReqQ.empty() )
			{
				// If shutdown in progress then just break from the loop
				if (gbShutDown)
					break;

				CSingleLock singleLock(&gStatReqQCriticalSection,TRUE);
				strUpdateParam = gStatReqQ.back();
				gStatReqQ.pop_back();
				singleLock.Unlock();

				// Call the function that updates the stats data            
				USUpdateNumStatInternal(strUpdateParam);
			}
		}
		catch (CMemoryException * /* pMemoryException */)
		{
			// Catastrophic memory exception - rethrow
			ASSERT( !"Rethrowing CMemoryException in CreateStatsUpdateReqThread" );
			throw;
		}
		catch (CException * pException)
		{
			// Other MFC exception
			pException->Delete();
			ASSERT( !"Caught CException (not CMemoryException) in CreateStatsUpdateReqThread" );
		}
		catch (std::bad_alloc & /* exception */)
		{
			// Catastrophic memory exception - rethrow
			ASSERT( !"Rethrowing std::bad_alloc in CreateStatsUpdateReqThread" );
			throw;
		}
		catch (std::exception & /* exception */)
		{
			ASSERT( !"Caught std::exception (not std::bad_alloc) in CreateStatsUpdateReqThread" );
		}
    } // End while loop

    return TRUE;

}

UINT USUpdateNumStatInternal(UpdateParam strUpdateParam)
{
	CheckTimeChange();

	if (strUpdateParam.sValue == -1)
		UpdateNumStatLo(strUpdateParam.which,strUpdateParam.sValue);	
	else
	{
		// Call the one with TimeStamp
		UpdateNumStatWithTime(strUpdateParam.which,strUpdateParam.sValue,strUpdateParam.lTimeStamp);
	}
	
	UpdateHTMLContent(strUpdateParam.which,strUpdateParam.sValue);	
	
	return 0;
}

void SetUsgStatsDocPtr(void* pDoc)
{
	gpUsgStatsDoc = (CUsageStatisticsDoc *)pDoc; 
}

void SetUsgStatsViewPtr(void* pView)
{
	gpUsgStatsView = (CUsageStatisticsView *)pView;
}

void UpdateValueInString(CString& strSrc, CString strTarget, CString strNewStr)
{
	int			iMatchLoc = -1;		// Location where strTarget occurs
	int			iMatchInternal = -1;
	CString		strTemp = _T("");	// Temporary string
	
	CString		csStartToken = "<!--VS-->";
	CString		csEndToken = "<!--VE-->";

	CString		csRetStr = strSrc;

	do
	{
		iMatchLoc = strSrc.Find(strTarget);
		if (iMatchLoc != -1)
		{
			// Take the part after the replace position...
			strTemp = strSrc.Right(strSrc.GetLength() - (iMatchLoc + strTarget.GetLength()));
			// ...and the part before the replace position...
			
			csRetStr = strSrc.Left(iMatchLoc);
			strSrc = strSrc.Left(iMatchLoc);

			csRetStr += strTarget;
			
			iMatchInternal = strTemp.Find(csStartToken);
			if (iMatchInternal != -1)
			{
				csRetStr += csStartToken;
				strSrc += csStartToken;
				
				strTemp = strTemp.Right(strTemp.GetLength() - (iMatchInternal + csStartToken.GetLength()));

				iMatchInternal = -1;
				iMatchInternal = strTemp.Find(csEndToken);

				if (iMatchInternal != -1)
				{
					// ...add the replace string...
					csRetStr += strNewStr;
					strSrc += strNewStr;
					// ...and concatenate the last part of the string.
					csRetStr += strTemp.Right(strTemp.GetLength() - (iMatchInternal));
					strSrc += strTemp.Right(strTemp.GetLength() - (iMatchInternal));
				}
			}
		}
	} while (iMatchLoc != -1);

	strSrc = csRetStr;
}

static void UpdateNumStatLo(StatType which, long value)
{
	StatDataPtr	pData;
	NumericStats	*pNum = NULL;
	ShortNumericStats	*pShort = NULL;
	
	if (LoadStats())
		return;
	
	if (value)
	{
		pData = gStatData;
		
		if (which < STATCOUNT)
		{
			// Granularity for these stats is smaller. (Hour, Day, Month, Year)
			pNum = &(pData->numStats[which]);
			pNum->current.day[gCurrentTime.tm_hour] += value;
			pNum->current.week[gCurrentTime.tm_wday] += value;
			pNum->current.month[gCurrentTime.tm_mday - 1] += value;
			pNum->current.year[gCurrentTime.tm_mon] += value;
			pNum->total += value;

			// This is not valid anymore SD 1/20/04
			// Some code to check for the data corruption 
			// If the total is less than the current & previous values, then something has gone wrong.
			//if (pNum->total < SumArray(pNum->current.day,24) + SumArray(pNum->last.day,24) )
			//	ASSERT(0);

		}
		else if (which < ENDSTATS)
		{
			// Granularity for these stats is bigger. (Day, Week, Month, Year)

			pShort = &(pData->shortStats[which-BEGINSHORTSTATS-1]);
			pShort->current.day += value;
			pShort->current.week += value;
			pShort->current.month += value;
			pShort->current.year += value;
			pShort->total += value;

			// This is not valid anymore SD 1/20/04
			// Some code to check for the data corruption 
			// If the total is less than the current & previous values, then something has gone wrong.
			//if (pShort ->total < pShort->current.day + pShort->last.day)
			//	ASSERT(0);
			
		}
		gStatsDirty = true;		
	}
}


void UpdateNumStatWithTime(StatType which, long value, uLong seconds)
{
	ULONG			curSeconds;
	NumericStats	*pNum = NULL;
	ShortNumericStats	*pShort = NULL;
	time_t	theTime;
		
	
	if (!gStatData) return;
	
	CheckTimeChange();
	time(&theTime);
	curSeconds = (ULONG) theTime;
	if (seconds > curSeconds)
		//	If time is in future, just use current time
		UpdateNumStatLo(which,value);
	else if (value)
	{
		StatData*	pData;
		struct tm  dt;
		short		dYear,dMonth,dWeek,dDay;
		
		theTime = (time_t) seconds;		
		dt = *(localtime( &theTime )); /* Convert to local time. */	

		dDay = (short) (gCurrentTime.tm_yday - dt.tm_yday);

		// Code needed to compute Week of the year		
		TCHAR szTmpBuf[10];
		strftime( szTmpBuf, sizeof(szTmpBuf), "%U", &dt);
		int nDTWeekOfTheYear = _ttoi(szTmpBuf);
		strftime( szTmpBuf, sizeof(szTmpBuf), "%U", &gCurrentTime);
		int ngCurrentTimeWeekOfTheYear = _ttoi(szTmpBuf);
		
		dWeek = (short) (ngCurrentTimeWeekOfTheYear  - nDTWeekOfTheYear);
	
		dMonth = (short) (gCurrentTime.tm_mon - dt.tm_mon);
		dYear = (short) (gCurrentTime.tm_year - dt.tm_year);

		pData = gStatData;
		
		if (which < STATCOUNT)
		{
			pNum = &pData->numStats[which];
			if (dYear==0)
			{
				// current year
				pNum->current.year[dt.tm_mon] += value;
				if (dMonth==0)
					pNum->current.month[dt.tm_mday - 1] += value;	// current month
				else 
				{
					if (dMonth==1)
						pNum->last.month[dt.tm_mday - 1] += value;	// last month
					pNum->average.month[dt.tm_mday - 1] += value;	// add to average
				}
				if (dWeek==0)
					pNum->current.week[dt.tm_wday] += value;	// current week
				else
				{
					if (dWeek==1)
						pNum->last.week[dt.tm_wday] += value;	// last week
					pNum->average.week[dt.tm_wday] += value;	// add to average
				}
				if (dDay==0)
					pNum->current.day[dt.tm_hour] += value;	// today
				else 
				{
					if (dDay==1)
						pNum->last.day[dt.tm_hour] += value;	// yesterday
					pNum->average.day[dt.tm_hour] += value;	// add to average				
				}
			}
			else
			{
				if (dYear==1)
				{
					// last year
					pNum->last.year[dt.tm_mon] += value;
					if (dt.tm_mon ==12 && gCurrentTime.tm_mon==1)
						pNum->last.month[dt.tm_mday - 1] += value;	// last month
					if (nDTWeekOfTheYear == 52 && ngCurrentTimeWeekOfTheYear ==1)
						pNum->last.week[dt.tm_wday] += value;	// last week
				}
				pNum->average.year[dt.tm_mon] += value;	// add to average
			}
			pNum->total += value;


			// Some code for checking data corruption 
			// If the total is less than the current & previous values, then something has gone wrong.
			if (pNum->total < SumArray(pNum->current.day,24) + SumArray(pNum->last.day,24) )
				ASSERT(0);

		}
		else if (which < ENDSTATS)
		{
			pShort = &pData->shortStats[which-BEGINSHORTSTATS-1];
			if (dYear==0)
			{
				// current year
				pShort->current.year += value;
				if (dMonth==0)
					pShort->current.month += value;	// current month
				else 
				{
					if (dMonth==1)
						pShort->last.month += value;	// last month
					pShort->average.month += value;		//add to average
				}
				if (dWeek==0)
					pShort->current.week += value;	// current week
				else
				{
					if (dWeek==1)
						pShort->last.week += value;	// last week
					pShort->average.week += value;	// add to average
				}
				if (dDay==0)
					pShort->current.day += value;	// today
				else 
				{
					if (dDay==1)
						pShort->last.day += value;	// yesterday
					pShort->average.day += value;	//add to average
				}
			}
			else 
			{
				if (dYear==1)
				{
					// last year
					pShort->last.year += value;
					if (dt.tm_mon==12 && gCurrentTime.tm_mon==1)
						pShort->last.month += value;	// last month
					if (nDTWeekOfTheYear==52 && ngCurrentTimeWeekOfTheYear==1)
						pShort->last.week += value;	// last week
				}
				pShort->average.year += value;	//add to average
			}
			pShort->total += value;

			// Some code for checking data corruption 
			// If the total is less than the current & previous values, then something has gone wrong.
			if (pShort->total < pShort->current.day + pShort->last.day) 
				ASSERT(0);

		}
		gStatsDirty = true;		
	}
}

// Called whenever ON_TIMECHANGE notification is caused
void USRefreshStats(void)
{
	if(gStatsInitialized)
	{
		CheckTimeChange();
		UpdateHTMLContent(STATCOUNT,0);
	}
}

static void UpdateHTMLContent(StatType which, short value)
{
	static short sOperations = 0;
	BOOL  bForce = false;
 	
	sOperations += value;
	if ( (sOperations >= gsSaveAfterXOperations) && 
		 (which != US_STATFACETIME) &&
		 (which < US_STATFACETIMEREAD || which > US_STATFACETIMEOTHER) )
	{
		// Save Stats
		SaveStats(true);
		//Reset the counter
		sOperations = 0;
		bForce = true;
	}

	if (gpUsgStatsView != NULL && gpUsgStatsView->m_hWnd != NULL)
	{
		::SendMessage(gpUsgStatsView->m_hWnd,uRefreshUsageStatisticsWindow ,WPARAM(which),LPARAM(bForce));
	}

	
}

void ReloadDataInHTML(StatTimePeriod stTimePeriod, StatType which, BOOL bDetailed)
{
	TCHAR szBuf[64];
	long	values[4];	
	
	ULONG	secondsTab[] = { 24*60*60,7*24*60*60,31*24*60*60,365*24*60*60 };
	ULONG	ulElapsedUnits = 0;

	time_t	theTime;
	time(&theTime);	
	BOOL bSetProj = FALSE;

	if (stTimePeriod == -1)
	{
		CheckTimeChange();
		stTimePeriod = gsPeriodSelected;
	}

	if ( (unsigned) (theTime - gStatData->startTime) < secondsTab[stTimePeriod] ) 
		bSetProj = TRUE;

	UpdateCalculatedStats();
	
	switch(stTimePeriod)
	{
		case (short) STATDAY:
		case (short) STATWEEK:
		case (short) STATMONTH:
		case (short) STATYEAR:
			// <!--CurP-->
			UpdateValueInString(gStrUsgStatHTML , "<!--CurP-->", csPeriodStrings[stTimePeriod][0]);
			// <!--PrevP-->
			UpdateValueInString(gStrUsgStatHTML , "<!--LastP-->", csPeriodStrings[stTimePeriod][1]);
			if (bSetProj == TRUE)
			{
				// <!--P-->
				strcpy(szBuf,csPeriodStrings[stTimePeriod][2]);
				strcat(szBuf,csProjectedString);
				UpdateValueInString(gStrUsgStatHTML , "<!--P-->", szBuf);
			}
			else
				UpdateValueInString(gStrUsgStatHTML , "<!--P-->", csPeriodStrings[stTimePeriod][2]);

			break;

		default :
				break;

	}

	ulElapsedUnits = CalcElapsedUnits(stTimePeriod);
	
	if ( (which < STATCOUNT) && (which !=  US_STATFACETIME) )
	{
		DisplayBasicNumStats(stTimePeriod, which);	
	}
	else if (which == STATCOUNT)
	{
		
		for (int i = 0;i < STATCOUNT; i++)
		{
			DisplayBasicNumStats(stTimePeriod, (StatType) i);		
		}
	}

	sprintf(szBuf,"<td width=%d%%>",glFirstColPercentWidth);
	UpdateValueInString(gStrUsgStatHTML , "<!--FirstColWidth-->", szBuf);
	sprintf(szBuf,"<td width=%d%%>",glSecondColPercentWidth);
	UpdateValueInString(gStrUsgStatHTML , "<!--SecondColWidth-->", szBuf);

	if (glFirstColPercentWidth < 30)
		UpdateValueInString(gStrUsgStatHTML , "<!--Above800x600-->", "");
	else
		UpdateValueInString(gStrUsgStatHTML , "<!--Below800x600-->", "");


	// Do it for the facetime
	GetStatTotals(&(gStatData->numStats[US_STATFACETIME]),stTimePeriod,values);

	DisplayUsage((ULONG) values[THISSTAT], szBuf);
	UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATFACETIME][THISSTAT], szBuf);
			
	// <!--RML-->
	DisplayUsage((ULONG) values[LASTSTAT], szBuf);
	UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATFACETIME][LASTSTAT], szBuf);
	
	// <!--RA-->
	/*if (ulElapsedUnits != 0)
		values[AVESTAT] /= ulElapsedUnits;*/
	DisplayUsage((ULONG) values[AVESTAT], szBuf);
	UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATFACETIME][AVESTAT], szBuf);
	
	// <!--RT--> 
	DisplayUsage((ULONG) values[TOTALSTAT], szBuf);
	UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATFACETIME][TOTALSTAT], szBuf);		


	if (gbMoreStatistics)
	{
		// TODO:Replace this by a function when debugged so that we don't cut & paste it all over the place

		ComposeShortStats(US_STATRECEIVEDATTACH,gdRcvdAttachValues);	
		DisplayShortStats(US_STATRECEIVEDATTACH,gdRcvdAttachValues);

		/*_ltoa(gdRcvdAttachValues[THISSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATRECEIVEDATTACH][THISSTAT], szBuf);

		_ltoa(gdRcvdAttachValues[LASTSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATRECEIVEDATTACH][LASTSTAT], szBuf);

		//_ltoa(dValues[AVESTAT],szBuf,10);
		sprintf(szBuf,"%.2f",gdRcvdAttachValues[AVESTAT]);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATRECEIVEDATTACH][AVESTAT], szBuf);

		_ltoa(gdRcvdAttachValues[TOTALSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATRECEIVEDATTACH][TOTALSTAT], szBuf);*/

		ComposeShortStats(US_STATREADMSG,gdReadMessageValues);
		DisplayShortStats(US_STATREADMSG,gdReadMessageValues);

		/*_ltoa(gdReadMessageValues[THISSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATREADMSG][THISSTAT], szBuf);

		_ltoa(gdReadMessageValues[LASTSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATREADMSG][LASTSTAT], szBuf);

		//_ltoa(dValues[AVESTAT],szBuf,10);
		sprintf(szBuf,"%.2f",gdReadMessageValues[AVESTAT]);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATREADMSG][AVESTAT], szBuf);

		_ltoa(gdReadMessageValues[TOTALSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATREADMSG][TOTALSTAT], szBuf);*/

		ComposeShortStats(US_STATSENTATTACH,gdSentAttachValues);		
		DisplayShortStats(US_STATSENTATTACH,gdSentAttachValues);

		/*_ltoa(gdSentAttachValues[THISSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATSENTATTACH][THISSTAT], szBuf);

		_ltoa(gdSentAttachValues[LASTSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATSENTATTACH][LASTSTAT], szBuf);

		//_ltoa(dValues[AVESTAT],szBuf,10);
		sprintf(szBuf,"%.2f",gdSentAttachValues[AVESTAT]);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATSENTATTACH][AVESTAT], szBuf);

		_ltoa(gdSentAttachValues[TOTALSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATSENTATTACH][TOTALSTAT], szBuf);*/

		ComposeFRR(gdFValues, gdRyValues, gdRdtValues);
		_ltoa(gdFValues[THISSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATFORWARDMSG][THISSTAT], szBuf);

		_ltoa(gdFValues[LASTSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATFORWARDMSG][LASTSTAT], szBuf);

		//_ltoa(gdFValues[AVESTAT],szBuf,10);
		sprintf(szBuf,"%.2f",gdFValues[AVESTAT]);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATFORWARDMSG][AVESTAT], szBuf);
		
		_ltoa(gdRyValues[THISSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATREPLYMSG][THISSTAT], szBuf);

		_ltoa(gdRyValues[LASTSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATREPLYMSG][LASTSTAT], szBuf);
		
		//_ltoa(gdRyValues[AVESTAT],szBuf,10);
		sprintf(szBuf,"%.2f",gdRyValues[AVESTAT]);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATREPLYMSG][AVESTAT], szBuf);

		_ltoa(gdRdtValues[THISSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATREDIRECTMSG][THISSTAT], szBuf);

		_ltoa(gdRdtValues[LASTSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATREDIRECTMSG][LASTSTAT], szBuf);

		//_ltoa(gdRdtValues[AVESTAT],szBuf,10);
		sprintf(szBuf,"%.2f",gdRdtValues[AVESTAT]);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATREDIRECTMSG][AVESTAT], szBuf);

		ComposeUsageActivities(gsFacetimeValues);
		sprintf(szBuf,"%d",gsFacetimeValues[FACETIMEREAD]);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATFACETIMEREAD][THISSTAT], szBuf);

		sprintf(szBuf,"%d",gsFacetimeValues[FACETIMEWRITE]);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATFACETIMECOMPOSE][THISSTAT], szBuf);

		sprintf(szBuf,"%d",gsFacetimeValues[FACETIMEOTHER]);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[US_STATFACETIMEOTHER][THISSTAT], szBuf);
	}

	ComposeTimeStringInStatsFormat(gStatData->startTime,szBuf);
	UpdateValueInString(gStrUsgStatHTML , "<!--StartTime-->", szBuf);

	ComposeTimeStringInStatsFormat(theTime,szBuf);
	UpdateValueInString(gStrUsgStatHTML , "<!--CurrentTime-->", szBuf);

	// Junk junk
	DisplayJunkJunk(stTimePeriod);
}

// I get bored typing this crap
#define SUM_ARRAY(x) SumArray(x,sizeof(x)/sizeof(x[0]))
#define CASE_BLOCK(i,tp) do {\
				num = SUM_ARRAY(gStatData->numStats[jt[i].numerator].current.##tp); \
				denom = SUM_ARRAY(gStatData->numStats[jt[i].denominator].current.##tp); \
				last_num = SUM_ARRAY(gStatData->numStats[jt[i].numerator].last.##tp); \
				last_denom = SUM_ARRAY(gStatData->numStats[jt[i].denominator].last.##tp); \
	} while(0)

static void DisplayJunkJunk(StatTimePeriod stTimePeriod)
{
	static struct {
		StatType numerator;
		StatType denominator;
		TCHAR *tag;
	} jt[] = {
		US_CALC_JUNKTOTAL, US_CALC_TOTAL, "<!-JkP%c->",
		US_CALC_JUNKTOTAL, US_CALC_TOTAL, "<!-JkP2%c->",
		US_STATFALSEPOSITIVES, US_CALC_TOTAL, "<!-JkFP%c->",
		US_STATFALSENEGATIVES, US_CALC_TOTAL, "<!-JkFN%c->",

		US_STATFALSEWHITELIST, US_CALC_TOTAL, "<!-JkFW%c->"
		};

	// calculations
	long num=0;
	long denom=0;
	long last_num=0;
	long last_denom=0;
	TCHAR szTagBuf[64];
	TCHAR szValBuf[64];
	long fp=0, last_fp=0;
	long fn=0, last_fn=0;
	long t=0, last_t=0;

	for (int i = 0;i<sizeof(jt)/sizeof(jt[0]);i++)
	{
		switch (stTimePeriod)
		{
			case STATDAY:
				CASE_BLOCK(i,day);
				break;
			case STATWEEK:
				CASE_BLOCK(i,week);
				break;
			case STATMONTH:
				CASE_BLOCK(i,month);
				break;
			case STATYEAR:
				CASE_BLOCK(i,year);
				break;
		}

		sprintf(szTagBuf,jt[i].tag,'T');	// compose tag, for This period
		_ltoa(CalcPercentage(num,denom),szValBuf,10);	// compose value as percent
		UpdateValueInString(gStrUsgStatHTML , szTagBuf, szValBuf);  // update in HTML

		sprintf(szTagBuf,jt[i].tag,'L');	// compose tag, for Last period
		_ltoa(CalcPercentage(last_num,last_denom),szValBuf,10);	// compose value as percent
		UpdateValueInString(gStrUsgStatHTML , szTagBuf, szValBuf);  // update in HTML

		// some values we remember
		if (jt[i].numerator==US_STATFALSEPOSITIVES) {fp = num; last_fp = last_num;}
		if (jt[i].numerator==US_STATFALSENEGATIVES) {fn = num; last_fn = last_num;}
		if (jt[i].denominator==US_CALC_TOTAL) {t = denom; last_t = last_denom;}
	}

	// Grand total
	denom = gStatData->numStats[US_CALC_TOTAL].total;
	_ltoa(CalcPercentage(gStatData->numStats[US_CALC_JUNKTOTAL].total,denom),szValBuf,10);	// compose value as percent
	UpdateValueInString(gStrUsgStatHTML , "<!-JkPG->", szValBuf);  // update in HTML

	// Accuracy.
	_ltoa(CalcPercentage(t-fp-fn,t),szValBuf,10);	// compose value as percent
	UpdateValueInString(gStrUsgStatHTML , "<!-JkAPT->", szValBuf);  // update in HTML
	_ltoa(CalcPercentage(last_t-last_fp-last_fn,last_t),szValBuf,10);	// compose value as percent
	UpdateValueInString(gStrUsgStatHTML , "<!-JkAPL->", szValBuf);  // update in HTML
}

static void DisplayShortStats(StatType which, double *dValues)
{
	// No checking is done for lValues pointer being passed. It is assumed that it will have what is required.		
	try
	{
		TCHAR szBuf[64];
		_ltoa(dValues[THISSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[which][THISSTAT], szBuf);

		_ltoa(dValues[LASTSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[which][LASTSTAT], szBuf);

		//_ltoa(dValues[AVESTAT],szBuf,10);
		sprintf(szBuf,"%.2f",dValues[AVESTAT]);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[which][AVESTAT], szBuf);

		_ltoa(dValues[TOTALSTAT],szBuf,10);
		UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[which][TOTALSTAT], szBuf);
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in DisplayShortStats" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in DisplayShortStats" );
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in DisplayShortStats" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception (not std::bad_alloc) in DisplayShortStats" );
	}
}


void CheckTimeChange(void)
{
	struct tm	dt;
	struct tm dtStart;
	time_t	theTime;
	
	enum 
	{
		kStart = 0,
		kNewYear=0x01,kSkipYear=0x02,kNewMonth=0x04,kSkipMonth=0x08,
		kNewWeek=0x10,kSkipWeek=0x20,kNewDay=0x40,kSkipDay=0x80,
		kStartYear=0x100,kStartMonth=0x200,kStartWeek=0x400,kStartDay=0x800
	};
	
	if (!gStatData) return;

	time(&theTime);
	dt = *(localtime( &theTime )); /* Convert to local time. */	
	
	//theTime = gStatData->startTime;
	dtStart = *(localtime( &gStatData->startTime )); /* Convert to local time. */	

	int timeFlags = kStart;

	if (dt.tm_hour != gCurrentTime.tm_hour)
		// facetime needs to be updated at least every hour
		UpdateFacetimeLo();

	if (dt.tm_year  > gCurrentTime.tm_year )
	{
		//	Happy New Year!
		timeFlags |= kNewYear+kNewMonth+kNewDay;
		if (dt.tm_year  - gCurrentTime.tm_year  > 1)
			// Skipped a year
			timeFlags |= kSkipYear+kSkipMonth+kSkipWeek+kSkipDay;
		if (dtStart.tm_year == gCurrentTime.tm_year )
			// don't include very first month in average
			timeFlags |= kStartYear;
	}
	
	if (dt.tm_mon > gCurrentTime.tm_mon || dt.tm_year  > gCurrentTime.tm_year )
	{
		// New month
		timeFlags |= kNewMonth+kNewDay;
		if (dt.tm_mon - gCurrentTime.tm_mon > 1)
			// Skipped a month
			timeFlags |= kSkipMonth+kSkipWeek+kSkipDay;
		if (dtStart.tm_year ==gCurrentTime.tm_year  && dtStart.tm_mon==gCurrentTime.tm_mon)
			// don't include very first day in average
			timeFlags |= kStartMonth;
	}

	// Code needed to compute Week of the year
	TCHAR szTmpBuf[10];
	strftime( szTmpBuf, sizeof(szTmpBuf), "%U", &dt);
	int nDTWeekOfTheYear = _ttoi(szTmpBuf);
	strftime( szTmpBuf, sizeof(szTmpBuf), "%U", &gCurrentTime);
	int ngCurrentTimeWeekOfTheYear = _ttoi(szTmpBuf);

	if (nDTWeekOfTheYear > ngCurrentTimeWeekOfTheYear || dt.tm_year  > gCurrentTime.tm_year )
	{
		// New week
		timeFlags |= kNewWeek+kNewDay;
		if (nDTWeekOfTheYear - ngCurrentTimeWeekOfTheYear > 1)
			// Skipped a week
			timeFlags |= kSkipWeek+kSkipDay;
		if (dtStart.tm_year ==gCurrentTime.tm_year  && nDTWeekOfTheYear==ngCurrentTimeWeekOfTheYear)
			// don't include very first day in average
			timeFlags |= kStartWeek;
	}

	if (dt.tm_yday > gCurrentTime.tm_yday || dt.tm_year  > gCurrentTime.tm_year )
	{
		// New day
		timeFlags |= kNewDay;
		if (dt.tm_yday - gCurrentTime.tm_yday > 1)
			// Skipped a day
			timeFlags |= kSkipDay;
		if (dtStart.tm_year ==gCurrentTime.tm_year  && dtStart.tm_yday==gCurrentTime.tm_yday)
			// don't include very first hour in average
			timeFlags |= kStartDay;
	}

	if (timeFlags)
	{
		short	i;
		NumericStats	*pStats;
		ShortNumericStats	*pShortStats;

		//	Do numeric stats
		for(i=0,pStats=gStatData->numStats;i<STATCOUNT;i++,pStats++)
		{
			//	Do year
			if (timeFlags&kNewYear)
			{
				memcpy(pStats->last.year,pStats->current.year,sizeof(pStats->last.year));

				if (timeFlags&kStartYear)
					// don't use first partial month
					pStats->current.year[dtStart.tm_mon] = 0;
				AddToAve((ULONG *)pStats->average.year,(ULONG *)pStats->current.year,12);
				memset(pStats->current.year,0,sizeof(pStats->current.year));
				if (timeFlags&kSkipYear)
					memset(pStats->last.year,0,sizeof(pStats->last.year));
					
			}
			//	Do month
			if (timeFlags&kNewMonth)
			{
				memcpy(pStats->last.month,pStats->current.month,sizeof(pStats->last.month));
				if (timeFlags&kStartMonth)					
					pStats->current.month[dtStart.tm_mday - 1] = 0;
				AddToAve((ULONG *)pStats->average.month,(ULONG *)pStats->current.month,31);				
				memset(pStats->current.month,0,sizeof(pStats->current.month));
				if (timeFlags&kSkipMonth)
					memset(pStats->last.month,0,sizeof(pStats->last.month));					
			}
			//	Do week
			if (timeFlags&kNewWeek)
			{
				memcpy(pStats->last.week,pStats->current.week,sizeof(pStats->last.week));
				if (timeFlags&kStartWeek)
					// don't use first partial day
					pStats->current.week[dtStart.tm_wday] = 0;
				AddToAve((ULONG *)pStats->average.week,(ULONG *)pStats->current.week,7);
				memset(pStats->current.week,0,sizeof(pStats->current.week));
				if (timeFlags&kSkipWeek)
					memset(pStats->last.week,0,sizeof(pStats->last.week));					
			}
			//	Do day
			if (timeFlags&kNewDay)
			{
				if(GetIniShort(IDS_INI_STATISTICS_BACKUP_DAILY))
				{
					// Backup the stats file if the INI setting is TRUE
					TCHAR szBackupFileName[_MAX_FNAME + _MAX_EXT + sizeof(TCHAR)];
					
					// The name of the stats file will be Stat[Month]-[Day].xml ...Aint's 365 enuf ??
					sprintf(szBackupFileName,"Stats61%d-%d.xml",dt.tm_mon + 1,dt.tm_mday);

					SaveStatsAsXML(szBackupFileName);
				}
					
				memcpy(pStats->last.day,pStats->current.day,sizeof(pStats->last.day));
				if (timeFlags&kStartDay)
					// don't use first partial hour
					pStats->current.day[dtStart.tm_hour] = 0;
				AddToAve((ULONG *)pStats->average.day,(ULONG *)pStats->current.day,24);
				memset(pStats->current.day,0,sizeof(pStats->current.day));
				if (timeFlags&kSkipDay)
					memset(pStats->last.day,0,sizeof(pStats->last.day));					
			}
		}
		
		//	Do short stats
		for(i=0,pShortStats=gStatData->shortStats;i<SHORTSTATCOUNT;i++,pShortStats++)
		{
			//	Do year
			if (timeFlags&kNewYear)
			{
				if (timeFlags&kSkipYear)
					pShortStats->current.year = 0;
				pShortStats->last.year = pShortStats->current.year;
				pShortStats->current.year = 0;
			}
			//	Do month
			if (timeFlags&kNewMonth)
			{
				if (timeFlags&kSkipMonth)
					pShortStats->current.month = 0;
				pShortStats->last.month = pShortStats->current.month;
				pShortStats->current.month = 0;
			}
			//	Do week
			if (timeFlags&kNewWeek)
			{
				if (timeFlags&kSkipWeek)
					pShortStats->current.week = 0;
				pShortStats->last.week = pShortStats->current.week;
				pShortStats->current.week = 0;
			}
			//	Do day
			if (timeFlags&kNewDay)
			{
				if (timeFlags&kSkipDay)
					pShortStats->current.day = 0;
				pShortStats->last.day = pShortStats->current.day;
				pShortStats->current.day = 0;
			}			
		}		
	}
	
	gCurrentTime = dt;
}

static void AddToAve(ULONG *to,ULONG *from,short length)
{
	while(length--)
		*to++ += *from++;
}


DWORD LoadStats(void)
{

	if (gStatData) 
		return US_SUCCESS;;	// We have already loaded it once

	time_t	theTime;
	// Get the current time
	time(&theTime);

	// Allocate memory for the Stat Data
	gStatData = DEBUG_NEW (StatData);	

	// Initialize it 
	memset(gStatData,0,sizeof(StatData));

	// I am not using _MAX_PATH 'coz it's just 256 chars.
	TCHAR szStatsFile[1024 + _MAX_FNAME + _MAX_EXT + sizeof(TCHAR)];
	TCHAR szStatsFileBinary[1024 + _MAX_FNAME + _MAX_EXT + sizeof(TCHAR)];

	_tcscpy(szStatsFile,EudoraDir);
	_tcscat(szStatsFile,gszStatsFile);

	// Because no provision was made for adding new stats prior to 6.1,
	// we have to save to a different filename.  However, if the new file
	// doesn't exist yet, we will read stats from the old file.  So if the new file
	// doesn't exist, try the old file
	if (!FileExistsMT(szStatsFile))
	{
		_tcscpy(szStatsFile,EudoraDir);
		_tcscat(szStatsFile,gszOldStatsFile);
	}

	FILE *fsStatsFile = NULL;
	_tcscpy(szStatsFileBinary,EudoraDir);
	_tcscat(szStatsFileBinary,"EudoraStats");

	if (FileExistsMT(szStatsFile))
	{
		int h = open( szStatsFile, O_BINARY | O_RDONLY );
		long lSize = lseek( h, 0, SEEK_END );
		m_XMLFilebuf = DEBUG_NEW_NOTHROW char[lSize + 1];

		if (m_XMLFilebuf)
		{
			lseek( h, 0, SEEK_SET );
			read( h, m_XMLFilebuf, lSize );
			m_XMLFilebuf[lSize] = 0;

			close(h);
			
			StatsXmlParser xpt;
			xpt.Parse( m_XMLFilebuf, lSize, 1);

			delete []m_XMLFilebuf;
			m_XMLFilebuf = NULL;
		}
		else
			return US_MEMORY_ALLOC_ERROR;
		
	}	
	else if (FileExistsMT(szStatsFileBinary))	// // This will be only required for 5.0 beta ..
	{
		// This means that the binary version of file exists. So lets read it, convert it into XML format
		// and delete the binary format

		// Open the file
		if( NULL != (fsStatsFile = fopen( szStatsFileBinary, "rt" )) )
		{
			if (gStatData != NULL)
			{
				// Read the file
				fread(gStatData, sizeof(StatData), 1, fsStatsFile);
				if( ferror( fsStatsFile ) || gStatData->startTime == 0)
				{
					// There was error reading
					gStatData->startTime = gStatData->currentTime = theTime;
				}
				
				// Take versioning into account

				// If the version is 0, then just flush the data. This is 'coz 
				// the data that was neing stored didn't have currentTime stored 
				// correctly in it & hence all the calculations were wrong
				if (gStatData->version == 0)
				{								
					// Re-initialize the data to be safe
					memset(gStatData,0,sizeof(StatData));

					// Set it to the newer version
					gStatData->version = STATFILEVERSION;

					// Set the start & current time 
					gStatData->startTime = gStatData->currentTime = theTime;
				}
			}
			else 
			{
				// Close the file & return error
				fclose(fsStatsFile);
				return US_MEMORY_ALLOC_ERROR;
			}

		}
		
		// Finally Close the File
		if (fsStatsFile)
			fclose(fsStatsFile);

		// Lets wait for removing this file for the next beta ..
		FileRemoveMT(szStatsFileBinary);
	
	}	// This will be only required for 5.0 beta ..ends
	else
	{
		// Set it to the current time
		if (gStatData)
			gStatData->startTime =  gStatData->currentTime = theTime;

		gStatData->version = STATFILEVERSION;
	}
	
	if (gStatData)
		gCurrentTime = *localtime(&gStatData->currentTime);	// set the local time as the current time
	
	return US_SUCCESS;
	//return 0;
}

void CALLBACK SaveStatsTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	SaveStats(false);
}

DWORD SaveStats(BOOL bForce)
{
	if (!gStatData) 
		return US_SUCCESS;	// Nothing to save

	if (gStatsDirty || bForce)
	{
		CheckTimeChange();		
		SaveStatsAsXML();
		gStatsDirty = false;
	}

	return US_SUCCESS;
}

static void GetStatTotals(NumericStats *stats,StatTimePeriod period,long values[])
{
	float	dayAve,secs;
	time_t	theTime;

	time(&theTime);
	secs = (long)theTime - (gStatData)->startTime;
	dayAve = secs ? 24.0*60.0*60.0*(float)stats->total/secs : 0 ;
	switch ((short) period)
	{
		case (short) STATDAY:
			values[THISSTAT] = SumArray(stats->current.day,24);
			values[LASTSTAT] = SumArray(stats->last.day,24);
			values[AVESTAT] = dayAve;
			break;
			
		case (short) STATWEEK:
			values[THISSTAT] = SumArray(stats->current.week,7);
			values[LASTSTAT] = SumArray(stats->last.week,7);
			values[AVESTAT] = 7.0*dayAve;
			break;
			
		case (short) STATMONTH:
			values[THISSTAT] = SumArray(stats->current.month,31);
			values[LASTSTAT] = SumArray(stats->last.month,31);
			values[AVESTAT] = 30.4375*dayAve;
			break;
			
		case (short) STATYEAR:
			values[THISSTAT] = SumArray(stats->current.year,12);
			values[LASTSTAT] = SumArray(stats->last.year,12);
			values[AVESTAT] = 365.25*dayAve;
			break;			
	}
	values[TOTALSTAT] = stats->total;

	// This assertion isn't valid when doing some of the calculated stats
	// if(values[TOTALSTAT] < values[THISSTAT] + values[LASTSTAT])
	//	ASSERT(0);
}

static long SumArray(long *values,short len)
{
	short	i;
	long	sum=0;
	
	for(i=0;i<len;i++)
		sum += *values++;
	return sum;
}

static void UpdateCalculatedStats(void)
{
	// Calculate the total junk
	// start with those scored junk
	gStatData->numStats[US_CALC_JUNKTOTAL] = gStatData->numStats[US_STATSCOREDJUNK];
	
	// subtract the false positives
	NumericStatOperation(US_CALC_JUNKTOTAL,US_STATFALSEPOSITIVES,opMinus);
	
	// add the false negatives
	NumericStatOperation(US_CALC_JUNKTOTAL,US_STATFALSENEGATIVES,opPlus);
	
	// add the false whitelist
	NumericStatOperation(US_CALC_JUNKTOTAL,US_STATFALSEWHITELIST,opPlus);
	
	// Calculate the overall total
	// Start with those scored junk
	gStatData->numStats[US_CALC_TOTAL] = gStatData->numStats[US_STATSCOREDJUNK];
	
	// add the those scored not junk
	NumericStatOperation(US_CALC_TOTAL,US_STATSCOREDNOTJUNK,opPlus);
	
	// add the whitelist
	NumericStatOperation(US_CALC_TOTAL,US_STATWHITELIST,opPlus);
	
	// Compute percentages
	gStatData->numStats[US_CALC_JUNKPERCENT] = gStatData->numStats[US_CALC_JUNKTOTAL];
	NumericStatOperation(US_CALC_JUNKPERCENT,US_CALC_TOTAL,opPercent);
}

/************************************************************************
 * NumericStatOperation - add or subtract one numeric stat from another
 ************************************************************************/
static void NumericStatOperation(StatType destination,StatType increment,OpType op)
{
	PeriodicStatOperation(&gStatData->numStats[destination].current,&gStatData->numStats[increment].current,op);
	PeriodicStatOperation(&gStatData->numStats[destination].last,&gStatData->numStats[increment].last,op);
	PeriodicStatOperation(&gStatData->numStats[destination].average,&gStatData->numStats[increment].average,op);

	switch (op)
	{
		case opPlus:
			gStatData->numStats[destination].total += gStatData->numStats[increment].total;
			break;
		case opMinus:
			gStatData->numStats[destination].total -= gStatData->numStats[increment].total;
			break;
		case opPercent:
			gStatData->numStats[destination].total = gStatData->numStats[increment].total ?
				CalcPercentage(gStatData->numStats[destination].total,gStatData->numStats[increment].total)
				: 0;
			break;
	}
}

/************************************************************************
 * PeriodicStatOperation - add or subtract one periodic stat from another
 ************************************************************************/
static void PeriodicStatOperation(PeriodicStats *destP, PeriodicStats *incP, OpType op)
{
	LongArrayOperation(destP->day,incP->day,sizeof(incP->day)/sizeof(incP->day[0]),op);
	LongArrayOperation(destP->week,incP->week,sizeof(incP->week)/sizeof(incP->week[0]),op);
	LongArrayOperation(destP->month,incP->month,sizeof(incP->month)/sizeof(incP->month[0]),op);
	LongArrayOperation(destP->year,incP->year,sizeof(incP->year)/sizeof(incP->year[0]),op);
}

/************************************************************************
 * LongArrayOperation - add or subtract one array of longs from another
 ************************************************************************/
static void LongArrayOperation(long *array,long *sub,short n,OpType op)
{
	switch (op)
	{
		case opPlus:
			while (n-->0)
				*array++ += *sub++;
			break;
		case opMinus:
			while (n-->0)
				*array++ -= *sub++;
			break;
		case opPercent:
			for (;n-->0;array++,sub++)
				*array = CalcPercentage(*array,*sub);
			break;
	}
}

void UpdateFacetime(FacetimeMode facetimeMode)
{
	if(gStatsInitialized)
	{	
		CheckTimeChange();
		UpdateFacetimeLo(facetimeMode);
	}
}

static void UpdateFacetimeLo(FacetimeMode facetimeMode)
{
	static ULONG ulNextSaveStatus = 0;	
	time_t	faceTime;

	faceTime = m_FaceTime->FaceTime();

	if (faceTime > 0)
	{
		//TRACE("UsageStats: Facetime : %d\n", faceTime);
		UpdateNumStatLo(US_STATFACETIME,faceTime);
		m_FaceTime->Reset();
		m_FaceTime->Start();
		
		ComposeDetailedFaceTime(facetimeMode);
		switch (gFacetimeMode)
		{
			case FACETIMEOTHER : 
				UpdateNumStatLo(US_STATFACETIMEOTHER, faceTime);
				TRACE ("UsageStats -> Facetime updated for other details\n");				
				break;
			case FACETIMEREAD : 
				UpdateNumStatLo(US_STATFACETIMEREAD,faceTime);
				TRACE ("US -> Facetime updated for Read Message\n");
				break;
			case FACETIMEWRITE :
				UpdateNumStatLo(US_STATFACETIMECOMPOSE,faceTime);
				TRACE ("US -> Facetime updated for Composition Message\n");				
				break;
			default:
				break;
		}
		
	}
}

static void DisplayUsage(ULONG ulValue, char *szBuf)
{
	
	long lMin = 0,lHour = 0,lSec = 0;
	//	Value is in seconds. Convert to hours or minutes
	
	lMin = ulValue / 60;
	lSec = ulValue % 60;

	if (lMin > 60)
	{
		lHour = lMin / 60;
		lMin = lMin % 60;
	}
	static CRString csHours(IDS_STATISTICS_HOUR_STRING);
	wsprintf(szBuf,"%d:%.2d %s",lHour,lMin,(LPCTSTR)csHours);
}	

ULONG CalcElapsedUnits(StatTimePeriod period)
{
	ULONG		startSecs,nowSecs,elapsedSecs;
	struct tm	dtStart,dtNow;
	ULONG		elapsedUnits = 0;
	time_t		theTime;

	time(&theTime);
	dtNow = *(localtime( &theTime )); 

	dtStart = *(localtime( &gStatData->startTime));
	
	dtNow.tm_min = dtNow.tm_sec = 0;
	dtStart.tm_min = dtStart.tm_sec = 0;

	switch (period)
	{
		case (short) STATDAY:
			dtStart.tm_hour++;
			startSecs = (long) mktime(&dtStart);
			nowSecs = (long) mktime(&dtNow);
			elapsedSecs = nowSecs > startSecs ? nowSecs - startSecs : 0;
			elapsedUnits = elapsedSecs/(24*60*60);
			break;
		case (short) STATWEEK:
			dtNow.tm_hour = dtStart.tm_hour = 0;
			dtStart.tm_wday++;
			startSecs = (long) mktime(&dtStart);
			nowSecs = (long) mktime(&dtNow);
			elapsedSecs = nowSecs > startSecs ? nowSecs - startSecs : 0;
			elapsedUnits = elapsedSecs/(7*24*60*60);
			break;
		case (short) STATMONTH:
			elapsedUnits = 12*(dtNow.tm_year-dtStart.tm_year);
			dtStart.tm_year = dtNow.tm_year;
			if (mktime(&dtNow) < mktime(&dtStart))
			{
				elapsedUnits -= 12;
				dtNow.tm_mon += 12;
			}
			if (dtNow.tm_mon != dtStart.tm_mon)
				elapsedUnits += dtNow.tm_mon - dtStart.tm_mon - 1;
			break;
		case (short) STATYEAR:
			elapsedUnits = dtNow.tm_year-dtStart.tm_year;
			dtStart.tm_year = dtNow.tm_year;
			if (mktime(&dtNow) < mktime(&dtStart))
				elapsedUnits--;
			break;
	}
		
	return elapsedUnits;
}


static void ComposeShortStats(StatType type, double dValues[])
{
	ULONG	thisVal = 0,lastVal = 0,aveVal = 0;
	float	dayAve,secs,recvMailAve = 0;
	ShortNumericStats	*stats;
	NumericStats	*recvMailStats = NULL;
	BOOL percent = false;

	long	lSum = 0;

	time_t	theTime;
	time(&theTime);
	
	secs = (long)theTime - (gStatData)->startTime;
	stats = &((gStatData)->shortStats[type-BEGINSHORTSTATS-1]);
	dayAve = secs ? 24.0*60.0*60.0*(float)stats->total/secs : 0 ;
	if (type == US_STATREADMSG)
	{
		percent = true;
		recvMailStats =  &((gStatData)->numStats[US_STATRECEIVEDMAIL]);
		recvMailAve = secs ? 24.0*60.0*60.0*(float)recvMailStats->total/secs : 0 ;
		aveVal = recvMailAve > 0 ? dayAve * 100 / recvMailAve : 0;
	}
	switch (gsPeriodSelected)
	{
		case STATDAY:
			thisVal = stats->current.day;
			lastVal = stats->last.day;
			if (percent)
			{

				lSum = SumArray(recvMailStats->current.day,24);
				thisVal = lSum > 0 ? thisVal * 100 / lSum : 0;

				lSum = SumArray(recvMailStats->last.day,24);
				lastVal = lSum > 0 ? lastVal * 100 / lSum : 0;
			}
			else
				aveVal = dayAve;

			break;
			
		case STATWEEK:
			thisVal = stats->current.week;
			lastVal = stats->last.week;
			if (percent)
			{
				lSum = SumArray(recvMailStats->current.week,7);
				thisVal = lSum > 0 ? thisVal * 100 / lSum : 0;				

				lSum = SumArray(recvMailStats->last.week,7);
				lastVal = lSum > 0 ? lastVal * 100 / lSum : 0;
			}
			else
				aveVal = 7.0*dayAve;
			break;
			
		case STATMONTH:
			thisVal = stats->current.month;
			lastVal = stats->last.month;			
			if (percent)
			{
				lSum = SumArray(recvMailStats->current.month,31);
				thisVal = lSum > 0 ? thisVal * 100 / lSum : 0;

				lSum = SumArray(recvMailStats->last.month,31);
				lastVal = lSum > 0 ? lastVal * 100 / lSum : 0;
			}
			else
				aveVal = 30.4375*dayAve;
			break;
			
		case STATYEAR:
			thisVal = stats->current.year;
			lastVal = stats->last.year;
			if (percent)
			{
				lSum = SumArray(recvMailStats->current.year,12);
				thisVal = lSum > 0 ? thisVal * 100 / lSum : 0;

				lSum = SumArray(recvMailStats->last.year,12);
				lastVal = lSum > 0 ? lastVal * 100 / lSum: 0;
			}
			else
				aveVal = 365.25*dayAve;			
			break;	
		default:
			break;
	}

	if (percent)
	{
		// There are cases wherein the average value goes above 100%. Well, technically 
		// that is wrong. E.g It can happen when a user reads messages which was received 
		// before Eudora started collecting stats.
		// In cases like those & others, if percentage is more than 100%, then just make it 100%.
		
		dValues[THISSTAT] = thisVal <= 100 ? thisVal : 100;
		dValues[LASTSTAT] = lastVal <= 100 ? lastVal : 100;
		dValues[AVESTAT] = aveVal <= 100 ? aveVal : 100;
	}
	else
	{
		dValues[THISSTAT] = thisVal;
		dValues[LASTSTAT] = lastVal;
		dValues[AVESTAT] = aveVal;

	}

	dValues[TOTALSTAT] = stats->total;
}

static void ComposeFRR(double dFValues[], double dRyValues[], double dRdtValues[])
{
	ULONG	secs;
	ShortNumericStats	*sForwardStats,*sReplyStats,*sRedirectStats;
	float	dayForwardAve,dayReplyAve,dayRedirectAve;
	
	sForwardStats = &((gStatData)->shortStats[US_STATFORWARDMSG-BEGINSHORTSTATS-1]);
	sReplyStats = &((gStatData)->shortStats[US_STATREPLYMSG-BEGINSHORTSTATS-1]);
	sRedirectStats = &((gStatData)->shortStats[US_STATREDIRECTMSG-BEGINSHORTSTATS-1]);

	time_t	theTime;
	time(&theTime);	
	secs = (long)theTime - gStatData->startTime;

	dayForwardAve = secs ? 24.0*60.0*60.0*(float)sForwardStats->total/secs : 0 ;
	dayReplyAve = secs ? 24.0*60.0*60.0*(float)sReplyStats->total/secs : 0 ;
	dayRedirectAve = secs ? 24.0*60.0*60.0*(float)sRedirectStats->total/secs : 0 ;
	switch (gsPeriodSelected)
	{
		case STATDAY:
			dFValues[0] = sForwardStats->current.day;
			dFValues[1] = sForwardStats->last.day;
			dFValues[2] = dayForwardAve;

			dRyValues[0] = sReplyStats->current.day;
			dRyValues[1] = sReplyStats->last.day;
			dRyValues[2] = dayReplyAve;

			dRdtValues[0] = sRedirectStats->current.day;			
			dRdtValues[1] = sRedirectStats->last.day;
			dRdtValues[2] = dayRedirectAve;

			break;
			
		case STATWEEK:
			dFValues[0] = sForwardStats->current.week;
			dFValues[1] = sForwardStats->last.week;
			dFValues[2] = dayForwardAve*7.0;

			dRyValues[0] = sReplyStats->current.week;
			dRyValues[1] = sReplyStats->last.week;
			dRyValues[2] = dayReplyAve*7.0;

			dRdtValues[0] = sRedirectStats->current.week;			
			dRdtValues[1] = sRedirectStats->last.week;
			dRdtValues[2] = dayRedirectAve*7.0;

			break;
			
		case STATMONTH:			
			dFValues[0] = sForwardStats->current.month;
			dFValues[1] = sForwardStats->last.month;
			dFValues[2] = dayForwardAve*30.4375;

			dRyValues[0] = sReplyStats->current.month;
			dRyValues[1] = sReplyStats->last.month;
			dRyValues[2] = dayReplyAve*30.4375;

			dRdtValues[0] = sRedirectStats->current.month;			
			dRdtValues[1] = sRedirectStats->last.month;
			dRdtValues[2] = dayRedirectAve*30.4375;

			break;
			
		case STATYEAR:			
			dFValues[0] = sForwardStats->current.year;
			dFValues[1] = sForwardStats->last.year;
			dFValues[2] = dayForwardAve*365.25;

			dRyValues[0] = sReplyStats->current.year;
			dRyValues[1] = sReplyStats->last.year;
			dRyValues[2] = dayReplyAve*365.25;

			dRdtValues[0] = sRedirectStats->current.year;
			dRdtValues[1] = sRedirectStats->last.year;
			dRdtValues[2] = dayRedirectAve*365.25;

			break;			
	}	
}


static void ComposeDetailedFaceTime(FacetimeMode facetimeMode)
{	
	FacetimeMode	lfacetimeMode;
	
	//if (!gStatData || !IsForegroundApp()) return;
	if (!gStatData) return;

	lfacetimeMode = FACETIMEOTHER;
	CMDIChildWnd *pChild = NULL;

	if (FACETIMEMODENULL == facetimeMode)
	{
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

		// Get the active MDI child window.
		pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
	
		// Get the associated facetime mode
		lfacetimeMode = USGetFacetimeMode(pChild);

		gFacetimeMode = lfacetimeMode;
	}	
	else
		gFacetimeMode = facetimeMode;
	
}

FacetimeMode USGetFacetimeMode(CMDIChildWnd* pChild)
{	
	FacetimeMode	lfacetimeMode;
	
	lfacetimeMode = FACETIMEOTHER;

	try
	{
		if (pChild)
		{
			if (pChild->IsKindOf( RUNTIME_CLASS( CCompMessageFrame)) )
			{
				lfacetimeMode = FACETIMEWRITE;
				//TRACE ("US : Should update Facetime for Compostion Message Window\n");
			}
			else if (pChild->IsKindOf( RUNTIME_CLASS( CReadMessageFrame)) )
			{
				lfacetimeMode = FACETIMEREAD;
				//TRACE ("US : Should update Facetime for Read Message Window\n");
			}	
			else if ( (pChild->IsKindOf( RUNTIME_CLASS( CTocFrame))) )
			{
				if ( ((CTocDoc *)((CTocFrame *)pChild->GetActiveDocument())) ->UsingPreviewPane())
				{
					lfacetimeMode = FACETIMEREAD;
					//TRACE ("US : Should update Facetime for Read Message Window\n");
				}
				else
					lfacetimeMode = FACETIMEOTHER;		
			}
		}
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in USGetFacetimeMode" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in USGetFacetimeMode" );
		lfacetimeMode = FACETIMEOTHER;
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in USGetFacetimeMode" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception (not std::bad_alloc) in USGetFacetimeMode" );
		lfacetimeMode = FACETIMEOTHER;
	}
	
	return lfacetimeMode;
}


static void ComposeUsageActivities(short sValues[])
{
	ULONG	read = 0,compose = 0,other = 0,total = 0,readPercent = 0,composePercent = 0,otherPercent = 0;

	switch (gsPeriodSelected)
	{
		case STATDAY:
			read = gStatData->shortStats[US_STATFACETIMEREAD-BEGINSHORTSTATS-1].current.day;
			compose = gStatData->shortStats[US_STATFACETIMECOMPOSE-BEGINSHORTSTATS-1].current.day;
			other = gStatData->shortStats[US_STATFACETIMEOTHER-BEGINSHORTSTATS-1].current.day;
			break;
			
		case STATWEEK:
			read = gStatData->shortStats[US_STATFACETIMEREAD-BEGINSHORTSTATS-1].current.week;
			compose = gStatData->shortStats[US_STATFACETIMECOMPOSE-BEGINSHORTSTATS-1].current.week;
			other = gStatData->shortStats[US_STATFACETIMEOTHER-BEGINSHORTSTATS-1].current.week;
			break;
			
		case STATMONTH:
			read = gStatData->shortStats[US_STATFACETIMEREAD-BEGINSHORTSTATS-1].current.month;
			compose = gStatData->shortStats[US_STATFACETIMECOMPOSE-BEGINSHORTSTATS-1].current.month;
			other = gStatData->shortStats[US_STATFACETIMEOTHER-BEGINSHORTSTATS-1].current.month;
			break;
			
		case STATYEAR:
			read = gStatData->shortStats[US_STATFACETIMEREAD-BEGINSHORTSTATS-1].current.year;
			compose = gStatData->shortStats[US_STATFACETIMECOMPOSE-BEGINSHORTSTATS-1].current.year;
			other = gStatData->shortStats[US_STATFACETIMEOTHER-BEGINSHORTSTATS-1].current.year;
			break;		
		default:
			break;
	}
	
	total = read+compose+other;
	if (total > 0)
	{
		readPercent = read*100/total;
		composePercent = compose*100/total;
		otherPercent = 100 - readPercent - composePercent;	
	}

	sValues[FACETIMEREAD] = readPercent;
	sValues[FACETIMEWRITE] = composePercent;
	sValues[FACETIMEOTHER] = otherPercent;
}

int StatsXmlParser::startElement( int id, const char* name, const char** atts ) 
{
    if ( *m_outBuffer )
        puts( m_outBuffer );

    for (int i = 0; i < m_depth; i++)
        putchar('\t');

    strcpy( m_outBuffer, name );
	m_IdsQueue.push_back(id);
    return 0;    // continue default processing
}


int StatsXmlParser::endElement( int id, const char* name ) 
{

	IdsQueue::reverse_iterator pdeque;

	int nDWMYID = 0, nCPAID = 0, nRSID = 0;

    if ( *m_outBuffer ) 
	{
        puts( m_outBuffer );            

        switch(id)
		{
			case version:
			case startTime:
			case currentTime:
				nRSID = id;
				ClassifyXMLString(nRSID,nCPAID,nDWMYID,m_outBuffer);
				m_IdsQueue.pop_back();
				break;

			case day:
			case week:
			case month:
			case year:
				nDWMYID = id;
				pdeque = m_IdsQueue.rbegin();
				pdeque++;
				if (pdeque != m_IdsQueue.rend())
				{
					nCPAID = *pdeque;
					switch(nCPAID)
					{
						case current :
						case previous:
						case averageSum:						
							pdeque++;

							if (pdeque != m_IdsQueue.rend())
							{
								nRSID = *pdeque;
								ClassifyXMLString(nRSID,nCPAID,nDWMYID,m_outBuffer);
							}
							break;
					}

					m_IdsQueue.pop_back();
				}
				break;

			case total:
				nCPAID = id;
				pdeque = m_IdsQueue.rbegin();
				pdeque++;
				if (pdeque != m_IdsQueue.rend())
				{
					nRSID = *pdeque;
					ClassifyXMLString(nRSID,nCPAID,nDWMYID,m_outBuffer);					
				}
				m_IdsQueue.pop_back();
				break;
			default:
				m_IdsQueue.pop_back();
		}			
		
    }

    return 0;    // continue default processing
}

int ClassifyXMLString(int nRSID, int nCPAID, int nDWMYID, const char* szString)
{
	int nRDSIDMode = 0;
	StatDataPtr	pData;
	char *cToken = NULL;
	char cParsingToken[] = "=";
			

	long lSeconds = 0;
	int nTimeZoneMinutes = 0;

	NumericStats	*pNum = NULL;
	ShortNumericStats	*pShortNum = NULL;

	PeriodicStats	*pPeriodStats = NULL;
	ShortPeriodicStats	*pShortPeriodStats = NULL;

	long *plValue = NULL;

	short slValueArraySize = 0;
	
	switch(nRSID)
	{
		case version:
			ParseStringInArry(szString, (long *)&gStatData->version , 1);			
			break;
		case startTime:
			// Convert the RFC822 date format to time_t (long)			
			cToken = _tcstok( (char *)szString, cParsingToken);
			if (cToken) 
				cToken = _tcstok( NULL, cParsingToken);	  			
			lSeconds = GetTimeMT(cToken, FALSE);
			FormatDateMT(cToken,lSeconds,nTimeZoneMinutes);
			gStatData->startTime = lSeconds + ((nTimeZoneMinutes) * 60);
			break;
		case currentTime:
			// Convert the RFC822 date format to time_t (long)
			cToken = _tcstok( (char *)szString, cParsingToken);
			if (cToken) 
				cToken = _tcstok( NULL, cParsingToken);	  			
			lSeconds = GetTimeMT(cToken, FALSE);
			FormatDateMT(cToken,lSeconds,nTimeZoneMinutes);
			gStatData->currentTime = lSeconds + ((nTimeZoneMinutes) * 60);
			break;
		case receivedMail:
			nRDSIDMode = US_STATRECEIVEDMAIL;
			break;
		case sentMail:
			nRDSIDMode = US_STATSENTMAIL;
			break;
		case usageSeconds:
			nRDSIDMode = US_STATFACETIME;
			break;
		case scoredJunk:
			nRDSIDMode = US_STATSCOREDJUNK;
			break;
		case scoredNotJunk:
			nRDSIDMode = US_STATSCOREDNOTJUNK;
			break;
		case whiteList:
			nRDSIDMode = US_STATFALSEWHITELIST;
			break;
		case falsePositives:
			nRDSIDMode = US_STATFALSEPOSITIVES;
			break;
		case falseNegatives:
			nRDSIDMode = US_STATFALSENEGATIVES;
			break;
		case falseWhiteList:
			nRDSIDMode = US_STATFALSEWHITELIST;
			break;
		case receivedAttachments:
			nRDSIDMode = US_STATRECEIVEDATTACH;
			break;
		case sentAttachments:
			nRDSIDMode = US_STATSENTATTACH;
			break;
		case readMessages:
			nRDSIDMode = US_STATREADMSG;
			break;
		case forwardMessages:
			nRDSIDMode = US_STATFORWARDMSG;
			break;
		case replyMessages:
			nRDSIDMode = US_STATREPLYMSG;
			break;
		case redirectMessages:
			nRDSIDMode = US_STATREDIRECTMSG;
			break;
		case usageRead:
			nRDSIDMode = US_STATFACETIMEREAD;
			break;
		case usageCompose:
			nRDSIDMode = US_STATFACETIMECOMPOSE;
			break;
		case usageOther:
			nRDSIDMode = US_STATFACETIMEOTHER;
			break;
		default: // unknown tag; ignore this section!
			return 0;
			break;
	}

	pData = gStatData;
	if (nRDSIDMode < STATCOUNT)
		pNum = &(pData->numStats[nRDSIDMode]);		
	else
		pShortNum = &(pData->shortStats[nRDSIDMode-BEGINSHORTSTATS-1]);
	
	switch(nCPAID)
	{
		case current:
			if (pNum)
				pPeriodStats = &pNum->current;
			else if (pShortNum)
				pShortPeriodStats = &pShortNum->current;
			break;
		case previous:
			if (pNum)
				pPeriodStats = &pNum->last;
			else if (pShortNum)
				pShortPeriodStats = &pShortNum->last;
			break;
		case averageSum:
			if (pNum)
				pPeriodStats = &pNum->average;
			else if (pShortNum)
				pShortPeriodStats = &pShortNum->average;
			break;

		case total:
			if(pNum)
				ParseStringInArry(szString, &pNum->total, 1);
			else
				ParseStringInArry(szString, &pShortNum->total, 1);
			break;
	}

	switch(nDWMYID)
	{
		case day:
			if (pPeriodStats)
			{
				plValue = (long *)&pPeriodStats->day;			
				slValueArraySize = DAYSTATCOUNT;
			}
			if (pShortPeriodStats)
			{
				plValue = (long *)&pShortPeriodStats->day;
				slValueArraySize = 1;
			}
			
			break;
		case week:
			if (pPeriodStats)
			{
				plValue = (long *)&pPeriodStats->week;			
				slValueArraySize = WEEKSTATCOUNT;
			}
			if (pShortPeriodStats)
			{
				plValue = (long *)&pShortPeriodStats->week;
				slValueArraySize = 1;
			}			
			break;
		case month:
			if (pPeriodStats)
			{
				plValue = (long *)&pPeriodStats->month;			
				slValueArraySize = MONTHSTATCOUNT;
			}
			if (pShortPeriodStats)
			{
				plValue = (long *)&pShortPeriodStats->month;
				slValueArraySize = 1;
			}
			break;
		case year:
			if (pPeriodStats)
			{
				plValue = (long *)&pPeriodStats->year;			
				slValueArraySize = YEARSTATCOUNT;
			}
			if (pShortPeriodStats)
			{
				plValue = (long *)&pShortPeriodStats->year;
				slValueArraySize = 1;
			}			
			break;
		default: 
			break;
	}

	if (plValue)
		ParseStringInArry(szString, plValue, slValueArraySize);

	return 0;
}

int ParseStringInArry(const char *szString, long* plValue, short slValueArraySize)
{
	char *cToken = NULL;
	int i = 0;

	char cParsingToken[] = " ,=";

	// Altho' the string being passed in is const char *, either we should duplicate it
	// or cast it to char * & use it. I think it's better off casting it 'coz it's not being
	// used once pasrsing is done. 
	cToken = _tcstok( (char *)szString, cParsingToken);
	// We need to go past the first token ..e.g day=1 3 ....
	if (cToken != NULL)
		cToken = _tcstok( NULL, cParsingToken);	  
	while( cToken != NULL && i < slValueArraySize)
	{
	  // While there are tokens in "string"
	  plValue[i] = atol(cToken);
	  // Get next token
	  cToken = _tcstok( NULL, cParsingToken);	  
	  i++;
	}

	return 0;
}



void SaveNumericStatsInXML(StatsXmlParser *pXMLParser, int id, PeriodicStats *pNumericStats)
{
	pXMLParser->WriteStartElement(id);
	pXMLParser->AddLineBreak();
	pXMLParser->WriteDataField(day,pNumericStats->day,DAYSTATCOUNT);
	pXMLParser->WriteDataField(week,pNumericStats->week,WEEKSTATCOUNT);
	pXMLParser->WriteDataField(month,pNumericStats->month,MONTHSTATCOUNT);
	pXMLParser->WriteDataField(year,pNumericStats->year,YEARSTATCOUNT);
	pXMLParser->WriteEndElement(id);
}

void SaveShortStatsInXML(StatsXmlParser *pXMLParser, int id, ShortPeriodicStats *pShortStats)
{
	pXMLParser->WriteStartElement(id);
	pXMLParser->AddLineBreak();
	pXMLParser->WriteDataField(day,pShortStats->day);
	pXMLParser->WriteDataField(week,pShortStats->week);
	pXMLParser->WriteDataField(month,pShortStats->month);
	pXMLParser->WriteDataField(year,pShortStats->year);
	pXMLParser->WriteEndElement(id);
}


void DisplayBasicNumStats(StatTimePeriod stTimePeriod, StatType which)
{
	TCHAR szBuf[64];
	long	values[4];
	double	dValue = 0.0;	

	// HTML tag <!--*MT-->
	GetStatTotals(&(gStatData->numStats[which]),stTimePeriod,values);

	_itoa(values[THISSTAT],szBuf,10);
	UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[which][THISSTAT], szBuf);
			
	// HTML tag <!--*ML-->
	_itoa(values[LASTSTAT],szBuf,10);
	UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[which][LASTSTAT], szBuf);
	
	// HTML tag <!--*A--> 
	dValue = values[AVESTAT];
	
	sprintf(szBuf,"%.2f",dValue);
	UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[which][AVESTAT], szBuf);
	
	// HTML tag <!--*T--> 
	_itoa(values[TOTALSTAT],szBuf,10);
	UpdateValueInString(gStrUsgStatHTML , csUsgStatsHTMLTags[which][TOTALSTAT], szBuf);	
		
	// Sorry, no workee with calced stats SD 1/19/04
	// Some checking code for the data corruption ..starts
	// if (values[TOTALSTAT] < (values[THISSTAT] + values[LASTSTAT]))
	// 		ASSERT(0);
	// Some checking code for the data corruption ..ends

}

void ResetStatistics(void)
{
	if(gStatsInitialized)
	{
		// Shutdown the stats
		ShutdownStats();

		// I am not using _MAX_PATH 'coz it's just 256 chars.
		TCHAR szStatsFile[1024 + _MAX_FNAME + _MAX_EXT + sizeof(TCHAR)];
		TCHAR szStatsFileBinary[1024 + _MAX_FNAME + _MAX_EXT  + sizeof(TCHAR)];

		_tcscpy(szStatsFile,EudoraDir);
		_tcscat(szStatsFile,gszStatsFile);

		_tcscpy(szStatsFileBinary,EudoraDir);
		_tcscat(szStatsFileBinary,"EudoraStats");
		
		if (FileExistsMT(szStatsFile))
		{
			//Delete the XML file
			FileRemoveMT(szStatsFile);
		}
		else if (FileExistsMT(szStatsFileBinary))	// // This will be only required for 5.0 beta ..
		{
			//Delete the XML file
			FileRemoveMT(szStatsFileBinary);
		}

		// Initialize the stats
		InitStats();
	}
}

DWORD SaveStatsAsXML(char *pFileName)
{
	int i = 0, j = 0;

	char szDate[64];
	int nTimeZoneMinutes = 0;
	long lGMTTime = 0;

	if (!gStatData) 
		return US_SUCCESS;	// Nothing to save

	BOOL bSpunState = FALSE, bContinueWithFileWriting = FALSE;
	
	if ( (IsDiskSpun(bSpunState)) &&
		 (!gbForceSave) )
	{
		if (bSpunState)
			bContinueWithFileWriting = TRUE;
		else
			bContinueWithFileWriting = FALSE;
	}
	else
		bContinueWithFileWriting = TRUE;

	if (bContinueWithFileWriting)
	{
		static StatData StatDataCopy;

		CSingleLock singleLock(&gStatReqQCriticalSection,TRUE);
		memcpy(&StatDataCopy,gStatData,sizeof(StatData));
		singleLock.Unlock();

		// I am not using _MAX_PATH 'coz it's just 256 chars.
		TCHAR szStatsFile[1024 + _MAX_FNAME + _MAX_EXT + sizeof(TCHAR)];
		_tcscpy(szStatsFile,EudoraDir);
		
		if(pFileName)
			_tcscat(szStatsFile,pFileName);
		else
			_tcscat(szStatsFile,gszStatsFile);
		
		time_t	theTime;
		time(&theTime);		
			 
		StatDataCopy.currentTime = theTime;

		StatsXmlParser XMLFormat;
		
		if ( XMLFormat.OpenFileForWriting(szStatsFile) )
		{
			XMLFormat.WriteStartElement(EudoraStats);
			XMLFormat.AddLineBreak();
		
			XMLFormat.WriteDataField(version,StatDataCopy.version);

			char szTimeZone[64] = { '\0' };	
			GetGMTOffsetMT(szTimeZone, &nTimeZoneMinutes);

			nTimeZoneMinutes = -nTimeZoneMinutes;

			lGMTTime  = StatDataCopy.startTime - (nTimeZoneMinutes * 60);
			ComposeDateMT(szDate, lGMTTime, nTimeZoneMinutes, FALSE);
			::TrimWhitespaceMT(szDate);
			XMLFormat.WriteDataField(startTime,szDate);		

			lGMTTime  = StatDataCopy.currentTime - (nTimeZoneMinutes * 60);
			ComposeDateMT(szDate, lGMTTime, nTimeZoneMinutes, FALSE);
			::TrimWhitespaceMT(szDate);
			XMLFormat.WriteDataField(currentTime,szDate);

			for(i = 0, j = receivedMail ;i<ENDSTATS;i++)
			{
				if ( (i == BEGINSHORTSTATS) || (i == US_UNUSED) || CALCED_STAT(i) )
					continue;

				if (i < BEGINSHORTSTATS)
				{
					// Numeric stats
					XMLFormat.WriteStartElement(j);
					XMLFormat.AddLineBreak();
					SaveNumericStatsInXML(&XMLFormat, current, &StatDataCopy.numStats[i].current);
					SaveNumericStatsInXML(&XMLFormat, previous, &StatDataCopy.numStats[i].last);
					SaveNumericStatsInXML(&XMLFormat, averageSum, &StatDataCopy.numStats[i].average);
					XMLFormat.WriteDataField(total,StatDataCopy.numStats[i].total);
					XMLFormat.WriteEndElement(j);
				}
				else
				{
					// Short stats
					XMLFormat.WriteStartElement(j);
					XMLFormat.AddLineBreak();
					SaveShortStatsInXML(&XMLFormat, current, &StatDataCopy.shortStats[i - BEGINSHORTSTATS - 1].current);
					SaveShortStatsInXML(&XMLFormat, previous, &StatDataCopy.shortStats[i - BEGINSHORTSTATS - 1].last);
					SaveShortStatsInXML(&XMLFormat, averageSum, &StatDataCopy.shortStats[i - BEGINSHORTSTATS - 1].average);
					XMLFormat.WriteDataField(total,StatDataCopy.shortStats[i - BEGINSHORTSTATS - 1].total);
					XMLFormat.WriteEndElement(j);
				}			

				// Increment j here to point to the next element in the element_ids
				j++;
			}	

			XMLFormat.WriteEndElement(EudoraStats);

			XMLFormat.CloseFile();
		}
	}

	return US_SUCCESS;
}


// Start Time
void USGetStartTime(LPTSTR lpszDate)
{
	ComposeTimeStringInStatsFormat(gStatData->startTime ,lpszDate);	
}

void ComposeTimeStringInStatsFormat(long lTime, LPTSTR lpszDate)
{
	if (!lpszDate)
		return;

	char szTimeZone[64] = { '\0' };	
	int nTimeZoneMinutes = 0;
	GetGMTOffsetMT(szTimeZone, &nTimeZoneMinutes);

	nTimeZoneMinutes = -nTimeZoneMinutes;

	long lGMTTime  = lTime - (nTimeZoneMinutes * 60);
	ComposeDateMT(lpszDate, lGMTTime, nTimeZoneMinutes, FALSE);
	
	char *pChar;
	pChar = _tcsrchr(lpszDate,' ');
	if (pChar)
		lpszDate[pChar - lpszDate + 1] = '\0';

	::TrimWhitespaceMT(lpszDate);
}


// Functions for the XML Parser class

// write out an element start tag
void StatsXmlParser::WriteStartElement( int id, char** attrs, bool linebreak )
{
	for ( int j = msLevel; j > 0; j-- )
		fprintf( mfpXML, "\t" );

	fprintf( mfpXML, "<%s>", MyElementMap[id].name );

	if ( linebreak )
		fprintf(mfpXML,  "\n" );

	msLevel++;
}

// write out an element end tag
void StatsXmlParser::WriteEndElement( int id, bool linebreak )
{
	msLevel--;

	if ( linebreak ) {
		for ( int j = msLevel; j > 0; j-- )
			fprintf(mfpXML,  "\t" );
	}

	fprintf(mfpXML, "</%s>\n", MyElementMap[id].name );
}

void StatsXmlParser::WriteElementData( int id, const char *szDataString )
{
	if ( szDataString ) 
		fprintf(mfpXML, szDataString );
}

void StatsXmlParser::WriteDataField( int id, long lValue)
{
	char szBuffer[32];
	sprintf(szBuffer,"%ld\0",lValue);
	WriteDataField(id,szBuffer);
	
}

void StatsXmlParser::WriteDataField( int id, const char *szDataString )
{
	WriteStartElement( id, 0, false );
	WriteElementData( id, szDataString );
	WriteEndElement( id, false );
}

void StatsXmlParser::WriteDataField( int id, long *plValue, short sCount)
{
	char szBuf[32];
	CString	csStr = "";
	if (plValue)
	{
		for (int i = 0; i < sCount - 1; i++)
		{
			sprintf(szBuf,"%ld\0",plValue[i]);
			csStr += szBuf;
			csStr += ' ';

		}
		sprintf(szBuf,"%ld\0",plValue[i]);
		csStr += szBuf;			
		WriteDataField(id,csStr);
	}
}

// Get the warning level back to the original one.
#pragma warning ( pop ) 
