#if !defined(AFX_EUGRAPHCTL_H__51E2C875_367B_11D4_8554_0008C7D3B6F8__INCLUDED_)
#define AFX_EUGRAPHCTL_H__51E2C875_367B_11D4_8554_0008C7D3B6F8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// EuGraphCtl.h : Declaration of the CEuGraphCtrl ActiveX Control class.

/////////////////////////////////////////////////////////////////////////////
// CEuGraphCtrl : See EuGraphCtl.cpp for implementation.

class CEuGraphCtrl : public COleControl
{
	DECLARE_DYNCREATE(CEuGraphCtrl)

// Constructor
public:
	CEuGraphCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEuGraphCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual DWORD GetControlFlags();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CEuGraphCtrl();
	void DrawFrame(CDC *pdc, const CRect& rcBounds);
	void DrawBarLine(CDC * pdc, CRect & rcBounds, const CRect & rcInvalid);
	void PlotPieGraph(CDC * pdc, CRect & rcBounds, const CRect & rcInvalid);

	void DrawLegends( CDC * pdc, CRect & rcBounds, COLORREF *pColor = NULL, LPCTSTR lpLgdString = NULL, short sLegendCount = 0);
	
	DECLARE_OLECREATE_EX(CEuGraphCtrl)
	DECLARE_OLETYPELIB(CEuGraphCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CEuGraphCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CEuGraphCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CEuGraphCtrl)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CEuGraphCtrl)
	afx_msg short GetDefaultGraphType();
	afx_msg void SetDefaultGraphType(short nNewValue);
	afx_msg short GetSeriesCount();
	afx_msg void SetSeriesCount(short nNewValue);
	afx_msg short GetXAxisNumberOfIntervals();
	afx_msg void SetXAxisNumberOfIntervals(short nNewValue);
	afx_msg void RedrawGraph();
	afx_msg long SetSeriesData(short sGraphType, long FAR* plDataValue, short sDataValueCount, OLE_COLOR GraphColor, LPCTSTR csLegendStrings);
	afx_msg void PieGraph(long FAR* plData, short sCount, OLE_COLOR* pColors, LPCTSTR csLegendStrings);
	afx_msg void SetXAxisLabelInformation(LPCTSTR szXAxisLabelString, short sLabelDisplayStartOffset, short sLabelDisplaySkipCount);
	afx_msg void SetYAxisScalingFactor(short sYAxisScaleFactor);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CEuGraphCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CEuGraphCtrl)
	dispidDefaultGraphType = 1L,
	dispidSeriesCount = 2L,
	dispidXAxisNumberOfIntervals = 3L,
	dispidRedrawGraph = 4L,
	dispidSetSeriesData = 5L,
	dispidPieGraph = 6L,
	dispidSetXAxisLabelInformation = 7L,
	dispidSetYAxisScalingFactor = 8L,
	//}}AFX_DISP_ID
	};


protected:
	enum EU_GRAPH_TYPES { EU_GT_2D_BAR_GRAPH = 0,
						  EU_GT_3D_BAR_GRAPH,
						  EU_GT_POINT_AS_CIRCLE_GRAPH,
						  EU_GT_POINT_AS_RECTANGLE_GRAPH,
						  EU_GT_POINT_AS_DIAMOND_GRAPH,
						  EU_GT_LINE_GRAPH,
						  EU_GT_FLOOD_FILLED_GRAPH,
						  EU_GT_PIE_GRAPH,
						};

	EU_GRAPH_TYPES m_GraphType;

	// Enum for  on bar and line types
    enum EU_FRAME_TYPES { EU_FT_NONE, 
						  EU_FT_AXIS, 
						  EU_FT_FULL };

	enum EU_POINT_GRAPH_ALIGNMENT { EU_POINT_GRAPH_TOP = 0,
									EU_POINT_GRAPH_CENTER,
									EU_POINT_GRAPH_BOTTOM };

	#define MAX_COLUMNS 32

    EU_FRAME_TYPES m_FrameType;

	CString		m_csTitle;
	CString		m_csTitleFontName;
	short		m_sTitleFontRelSize;

	CString		m_csSubTitle;
	CString		m_csSubTitleFontName;
	short		m_sSubTitleFontRelSize;

	COLORREF	m_TextColor; 

	long		m_lColValues[MAX_COLUMNS ];
	CString		m_csLegends[MAX_COLUMNS ];
	COLORREF	m_Colors[MAX_COLUMNS ];

	short		m_sCurCol;

	short		m_sColCount;	

	BOOL		m_ShowRule; 
	BOOL		m_ShowLegend; 

	short		m_sSeriesCount;
	short		m_sXAxisNumberOfIntervals;

	CString		m_csXAxisIntervalString;

	short		m_sYAxisScalingFactor;

	short		m_sYAxisRuleFactor;

	typedef struct
	{
		EU_GRAPH_TYPES	GraphType;	
		short			sDataValueCount;
		long			*data;			
		COLORREF		GraphColor;	
		TCHAR			szLegendString[128];
	} SeriesData;

	typedef struct
	{
		COLORREF	*pColorRef;
		TCHAR		szLegendString[1024];
		short		sLegendCount;
	} PieGraphData;

	SeriesData*	m_pSeriesData;

	PieGraphData	m_PieGraphData;

	short	m_sCurSeries;

	short	m_sRadiusForPointGraphs;
	short	m_sXAxisLabelSkipCount;
	short	m_sXAxisLabelDisplayStartOffset;
	BOOL	m_bPercent;

	void DrawPointGraph(CDC * pdc, EU_GRAPH_TYPES GraphType, const CRect &rcRect, CBrush &Br, short sBarWidth = -1, EU_POINT_GRAPH_ALIGNMENT AlignType = EU_POINT_GRAPH_TOP);

					  
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EUGRAPHCTL_H__51E2C875_367B_11D4_8554_0008C7D3B6F8__INCLUDED)
