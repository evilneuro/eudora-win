
#ifndef STATMNG_H
#define STATMNG_H

typedef enum
{
	// Keep stats organized by type
	
	// Standard numeric stats
	US_STATRECEIVEDMAIL,
	US_STATSENTMAIL,
	US_STATFACETIME,

	// Junk stuff
	US_STATSCOREDJUNK,
	US_STATSCOREDNOTJUNK,
	US_STATWHITELIST,
	US_STATFALSEPOSITIVES,
	US_STATFALSENEGATIVES,
	US_STATFALSEWHITELIST,

	// calculated junk stuff
	US_CALC_JUNKTOTAL,
	US_CALC_TOTAL,
	US_CALC_JUNKPERCENT,
	
	// Beginning of the Status Types
	BEGINSHORTSTATS,	

	US_STATRECEIVEDATTACH,
	US_STATSENTATTACH,
	US_STATREADMSG,
	US_UNUSED,
	US_STATFORWARDMSG,
	US_STATREPLYMSG,
	US_STATREDIRECTMSG,
	US_STATFACETIMEREAD,
	US_STATFACETIMECOMPOSE,
	US_STATFACETIMEOTHER,
	
	// End of the Status Types
	ENDSTATS,

} StatType;

typedef enum
{
	STATDAY,
	STATWEEK,
	STATMONTH,
	STATYEAR
} StatTimePeriod;

enum FacetimeMode	{	FACETIMEMODENULL = -1,
						FACETIMEREAD = 0,
						FACETIMEWRITE,
						FACETIMEOTHER };

void InitStats(void);
void ShutdownStats(void);
void ResetStatistics(void);

void UpdateNumStat(StatType which, short sValue, long lTimeStamp = -1);
void UpdateFacetime(FacetimeMode facetimeMode = FACETIMEMODENULL);
FacetimeMode USGetFacetimeMode(CMDIChildWnd* pChild);
void USRefreshStats(void);
void USGetStartTime(LPTSTR lpszDate);

void SetUsgStatsDocPtr(void *pDoc);
void SetUsgStatsViewPtr(void* pView);
void ReloadDataInHTML(StatTimePeriod stTimePeriod = StatTimePeriod(-1), StatType which = BEGINSHORTSTATS, BOOL bDetailed = FALSE);
ULONG CalcElapsedUnits(StatTimePeriod period);

#define STATCOUNT BEGINSHORTSTATS
#define SHORTSTATCOUNT (ENDSTATS - BEGINSHORTSTATS)

#define SAVESTATESECS			15*60			//	Save playlist state every 15 minutes
#define STATFILEVERSION			2
#define AUTOUPDATETICKS			120			

#define	DAYSTATCOUNT	24
#define	WEEKSTATCOUNT	7
#define	MONTHSTATCOUNT	31
#define	YEARSTATCOUNT	12

#define US_SUCCESS				0
#define	US_GENERIC ERROR		4000
#define US_MEMORY_ALLOC_ERROR	4001
#define US_FILE_IO_ERROR		4002

// The declaration of the data types is same in MAC also. This is an attempt to keep the file format same
// which could ease the job of writing 3rd party conversion routines on either platforms

typedef struct
{
	long	day[DAYSTATCOUNT];
	long	week[WEEKSTATCOUNT];
	long	month[MONTHSTATCOUNT];
	long	year[YEARSTATCOUNT];
} PeriodicStats;

typedef struct
{
	PeriodicStats	current;
	PeriodicStats	last;
	PeriodicStats	average;
	long			total;
} NumericStats;

typedef struct
{
	long	day;
	long	week;
	long	month;
	long	year;
} ShortPeriodicStats;

typedef struct
{
	ShortPeriodicStats	current;
	ShortPeriodicStats	last;
	ShortPeriodicStats	average;
	long				total;
} ShortNumericStats;

typedef struct
{
	short	version;
	long	startTime;	//	When did we start collecting stats?
	long	currentTime;
	long	unused1	;	//	totalDays;
	long	unused2	;	//	totalWeeks;
	long	unused3	;	//	totalMonths;
	long	unused4	;	//	totalYears;
	NumericStats	numStats[STATCOUNT];
	ShortNumericStats	shortStats[SHORTSTATCOUNT];
} StatData, *StatDataPtr, **StatDataHandle;

typedef struct
{
	StatType which;
	short sValue;
	long lTimeStamp;
} UpdateParam;





#endif //STATMNG_H
