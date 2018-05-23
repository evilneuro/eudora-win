// EuGraphCtl.cpp : Implementation of the CEuGraphCtrl ActiveX Control class.

#include "stdafx.h"
#include "EuGraph.h"
#include "EuGraphCtl.h"
#include "EuGraphPpg.h"
#include <math.h>

#include "DebugNewHelpers.h"


IMPLEMENT_DYNCREATE(CEuGraphCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CEuGraphCtrl, COleControl)
	//{{AFX_MSG_MAP(CEuGraphCtrl)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CEuGraphCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CEuGraphCtrl)
	DISP_PROPERTY_EX(CEuGraphCtrl, "DefaultGraphType", GetDefaultGraphType, SetDefaultGraphType, VT_I2)
	DISP_PROPERTY_EX(CEuGraphCtrl, "SeriesCount", GetSeriesCount, SetSeriesCount, VT_I2)
	DISP_PROPERTY_EX(CEuGraphCtrl, "XAxisNumberOfIntervals", GetXAxisNumberOfIntervals, SetXAxisNumberOfIntervals, VT_I2)
	DISP_FUNCTION(CEuGraphCtrl, "RedrawGraph", RedrawGraph, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CEuGraphCtrl, "SetSeriesData", SetSeriesData, VT_I4, VTS_I2 VTS_PI4 VTS_I2 VTS_COLOR VTS_BSTR)
	DISP_FUNCTION(CEuGraphCtrl, "PieGraph", PieGraph, VT_EMPTY, VTS_PI4 VTS_I2 VTS_PCOLOR VTS_BSTR)
	DISP_FUNCTION(CEuGraphCtrl, "SetXAxisLabelInformation", SetXAxisLabelInformation, VT_EMPTY, VTS_BSTR VTS_I2 VTS_I2)
	DISP_FUNCTION(CEuGraphCtrl, "SetYAxisScalingFactor", SetYAxisScalingFactor, VT_EMPTY, VTS_I2)
	DISP_STOCKPROP_BACKCOLOR()
	DISP_STOCKPROP_FORECOLOR()
	DISP_STOCKPROP_FONT()
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CEuGraphCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CEuGraphCtrl, COleControl)
	//{{AFX_EVENT_MAP(CEuGraphCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CEuGraphCtrl, 1)
	PROPPAGEID(CEuGraphPropPage::guid)
END_PROPPAGEIDS(CEuGraphCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CEuGraphCtrl, "EUGRAPH.EuGraphCtrl.1",
	0x51e2c867, 0x367b, 0x11d4, 0x85, 0x54, 0, 0x8, 0xc7, 0xd3, 0xb6, 0xf8)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CEuGraphCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DEuGraph =
		{ 0x51e2c865, 0x367b, 0x11d4, { 0x85, 0x54, 0, 0x8, 0xc7, 0xd3, 0xb6, 0xf8 } };
const IID BASED_CODE IID_DEuGraphEvents =
		{ 0x51e2c866, 0x367b, 0x11d4, { 0x85, 0x54, 0, 0x8, 0xc7, 0xd3, 0xb6, 0xf8 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwEuGraphOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CEuGraphCtrl, IDS_EUGRAPH, _dwEuGraphOleMisc)


const double pi = 3.1415926535;

/////////////////////////////////////////////////////////////////////////////
// CEuGraphCtrl::CEuGraphCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CEuGraphCtrl

BOOL CEuGraphCtrl::CEuGraphCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_EUGRAPH,
			IDB_EUGRAPH,
			afxRegApartmentThreading,
			_dwEuGraphOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphCtrl::CEuGraphCtrl - Constructor

CEuGraphCtrl::CEuGraphCtrl()
{
	InitializeIIDs(&IID_DEuGraph, &IID_DEuGraphEvents);

	m_sColCount = 31;
	m_sCurCol = 0;	

	m_csTitleFontName = "Times New Roman";
	m_csTitle = "";//"Title";
	m_sTitleFontRelSize = 13;
	m_csSubTitle = "";//"subtitle";
	m_csSubTitleFontName = "Times New Roman";
	m_sSubTitleFontRelSize = 12;
	m_TextColor = 0;
	m_csXAxisIntervalString = "";

	m_GraphType = EU_GT_2D_BAR_GRAPH;
	m_FrameType = EU_FT_FULL;
	m_pSeriesData = NULL;

	m_ShowRule = TRUE;
	m_sCurSeries = 0;

	m_pSeriesData = NULL;

	m_PieGraphData.pColorRef = NULL;
	m_sRadiusForPointGraphs = 0;
	m_sXAxisLabelSkipCount = 0;
	m_sXAxisLabelDisplayStartOffset = 0;	

	m_sYAxisScalingFactor = 1;	// Default Y Axis Scaling Factor
	m_sYAxisRuleFactor = 5;		// Default Y Axis Rule Factor

  
 	m_bPercent = 0;	// not doing percents unless told
}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphCtrl::~CEuGraphCtrl - Destructor

CEuGraphCtrl::~CEuGraphCtrl()
{
	// TODO: Cleanup your control's instance data here.
	if (m_pSeriesData)
	{
		for (int i = 0;i < m_sSeriesCount; i++)
		{		
			if (m_pSeriesData[i].data)
				delete m_pSeriesData[i].data;
		}

		delete m_pSeriesData;
	}
	
	if (m_PieGraphData.pColorRef != NULL)
	{
		
		delete []m_PieGraphData.pColorRef;
	}

}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphCtrl::OnDraw - Drawing function

void CEuGraphCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	CBrush bkBrush(TranslateColor(GetBackColor()));
    pdc->FillRect( rcBounds, &bkBrush );
	pdc->SetBkMode(TRANSPARENT);
	//pdc->SetTextColor( TranslateColor(GetSysColor(COLOR_WINDOWTEXT)) );
	pdc->SetTextColor( TranslateColor(GetForeColor()) );

	CFont* pLastFont;    
    
	pLastFont = SelectStockFont( pdc );
	
	CRect rcMyBounds( rcBounds );
	rcMyBounds.left += 15;
	rcMyBounds.top += 10;
	rcMyBounds.bottom -= 15;
	rcMyBounds.right -= abs(rcBounds.right - rcBounds.left)/3;

	CRect rcLgdBounds( rcMyBounds.right , rcMyBounds.top , rcBounds.right, rcMyBounds.bottom );

	int nActualTitleHeight=0, nActualSubTitleHeight=0;

	CFont Font, *pOldFont;

	/*pdc->Rectangle(rcBounds.left,
                   rcBounds.top,
                   rcBounds.right,
                   rcBounds.bottom);*/


	// Draw the Title
	if( m_csTitle.GetLength() > 0 && m_sTitleFontRelSize > 0 )
	{
		nActualTitleHeight = int(((rcBounds.bottom - rcBounds.top) / 100.0) * m_sTitleFontRelSize);
		Font.CreateFont(  nActualTitleHeight, 0, 0, 0, FW_NORMAL,FALSE, 
			FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, m_csTitleFontName );
		//pOldFont = pdc->SelectObject( &Font );
		pdc->DrawText( m_csTitle, rcMyBounds, DT_RIGHT );
		//pdc->SelectObject( pOldFont );
		Font.DeleteObject();
		rcMyBounds.top += nActualTitleHeight;
	}

	// Draw the SubTitle
	if( m_csSubTitle.GetLength() > 0 && m_sSubTitleFontRelSize > 0 )
	{
		nActualSubTitleHeight = int(((rcBounds.bottom - rcBounds.top) / 100.0) * m_sSubTitleFontRelSize);
		Font.CreateFont(  nActualSubTitleHeight, 0, 0, 0, FW_NORMAL,FALSE, 
			FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, m_csSubTitleFontName );
		pOldFont = pdc->SelectObject( &Font );
		
		pdc->DrawText( m_csSubTitle, rcMyBounds, DT_RIGHT );
		pdc->SelectObject( pOldFont );
		rcMyBounds.top += nActualSubTitleHeight;
	}

	if ( (m_pSeriesData != NULL) && m_pSeriesData[0].GraphType == EU_GT_PIE_GRAPH)
	{
		PlotPieGraph(pdc, rcMyBounds, rcInvalid );
		DrawLegends(pdc, rcLgdBounds, m_PieGraphData.pColorRef, CString(m_PieGraphData.szLegendString),  m_PieGraphData.sLegendCount);
	}
	else
	{
		DrawBarLine( pdc, rcMyBounds, rcInvalid );	
		//DrawFrame(pdc, rcMyBounds);
		DrawLegends(pdc, rcLgdBounds);
	}

	pdc->SelectObject(pLastFont);
}

void CEuGraphCtrl::DrawFrame(CDC *pdc, const CRect& rcBounds)
{
    if(pdc == NULL)
        return;
	
	if (EU_FT_NONE == m_FrameType)
		return;

    CPen Pen, *pOldPen;
	CBrush Brush;
    static COLORREF colorRef;

    // Get the background color to be used by the brush
    colorRef = pdc->GetBkColor();

    // Create the brush to erase the background
    Brush.CreateSolidBrush(colorRef);

    // Now erase the background
    //pdc->FillRect(&rcBounds, &Brush);

    // Create a pen to draw the frame X and Y axis
    Pen.CreatePen(PS_SOLID, 1, TranslateColor(GetForeColor()));

    pOldPen = pdc->SelectObject( &Pen );

    if (m_FrameType == EU_FT_AXIS || m_FrameType == EU_FT_FULL)
    {
        // Draw 'axis' lines on the bounding rect
        MoveToEx(pdc->m_hDC, rcBounds.left, rcBounds.top, NULL);
        LineTo(pdc->m_hDC, rcBounds.left, rcBounds.bottom);
        LineTo(pdc->m_hDC, rcBounds.right, rcBounds.bottom);
    

    	if (m_FrameType == EU_FT_FULL)
		{        
			LineTo(pdc->m_hDC, rcBounds.right, rcBounds.top);
			//LineTo(pdc->m_hDC, rcBounds.left, rcBounds.top);
		}
    
    }

	pOldPen = pdc->SelectObject( pOldPen );
	
	Pen.DeleteObject();

	Brush.DeleteObject();


} //DrawFrame()

void CEuGraphCtrl::DrawBarLine(CDC * pdc, CRect & rcBounds, const CRect & rcInvalid)
{
	int C, BarWidth = 0, Gap, nMin = 0, nMax = 0, V;
	int nDispHeight, nDispWidth, nHeight;
	float fAspect;
	RECT Bar;
	CBrush Br, *pOldBrush;
	CPen Pen, *pOldPen;
	CString Val;
	CSize FontDim;
	CFont RuleFont; //, *pOldFont;

	int lSeriesCount;

	CRgn FillRgn;
	POINT *PointList;
	int i;

	if (m_bPercent)
	{
		nMin = 0;
		nMax = 100;
	}
	else
	{
		if (m_sSeriesCount >0)
		{
			nMin= 200000000 ;
			nMax = -200000000;
		}

		for (int i = 0; i < m_sSeriesCount; i++)
		{
			if (m_pSeriesData != NULL)
			{
				for( C = 0; C < m_pSeriesData[i].sDataValueCount; C++ )
				{
					nMin = m_pSeriesData[i].data[C] < nMin ? m_pSeriesData[i].data[C] : nMin;
					nMax = m_pSeriesData[i].data[C] > nMax ? m_pSeriesData[i].data[C] : nMax;		
				}
			}
			
		}

		if (nMax < 10)
			nMax  = 10;
		else
		{
			nMax = nMax + (5 - nMax%5);
		}
	}	

	if( m_ShowRule )
	{
		Val = m_csTitleFontName;
		if( Val=="" )
			Val = "Times New Roman";
		RuleFont.CreateFont( (rcBounds.bottom - rcBounds.top) / 10, 0, 0, 0, 
			FW_NORMAL,FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, Val );
		//pOldFont = pdc->SelectObject( &RuleFont );
		pdc->SetTextColor( TranslateColor(GetForeColor()) );
		Val.Format("%d-", nMax);
		FontDim = pdc->GetTextExtent(Val);
		rcBounds.left += FontDim.cx;
		rcBounds.top+=FontDim.cy/2;
		rcBounds.bottom-=FontDim.cy/2;
	}

	DrawFrame(pdc, rcBounds);

	nDispHeight = rcBounds.bottom - rcBounds.top;
	nDispWidth = rcBounds.right - rcBounds.left;
	nHeight = nMin > 0 ? nMax : nMax-nMin;
	if( nHeight == nDispHeight  )
		fAspect = 1.0;
	else 
		fAspect = float(nDispHeight) / nHeight;
	
	// Determine bar width, accordng to display size
	if (m_sXAxisNumberOfIntervals > 0)
		BarWidth = nDispWidth / m_sXAxisNumberOfIntervals;
	Gap = BarWidth / 6;

	m_sRadiusForPointGraphs = (rcBounds.right - rcBounds.left)/48;	

	int nActualBarWidth = -1;
	int nBarGraphSeries = 0;
	int nCurrentBarGraphSeries = -1;

	UINT nTextAlign;

	if( m_ShowRule )
	{
		Pen.CreatePen(PS_SOLID, 1, TranslateColor(GetForeColor()));
		pOldPen = pdc->SelectObject( &Pen );

		nTextAlign = pdc->SetTextAlign(TA_RIGHT);

		Val.Format( "%d", 0);
		pdc->TextOut( rcBounds.left - FontDim.cx/2, 
			rcBounds.bottom - FontDim.cy/2,
			Val);

		for( int i=0; i < 5; i++ )
		{
			/*Val.Format( "%d", (i+1) * nMax / m_sYAxisRuleFactor);
			pdc->TextOut( rcBounds.left  - FontDim.cx/2, 
				rcBounds.bottom-(nDispHeight/m_sYAxisRuleFactor)*i - FontDim.cy/2 - (nDispHeight/m_sYAxisRuleFactor), 
				Val);

			pdc->SetTextColor( TranslateColor(GetForeColor()) );
			pdc->MoveTo(rcBounds.left - 3,  rcBounds.top + nDispHeight - (nDispHeight/m_sYAxisRuleFactor)*(i+1));
			pdc->LineTo(rcBounds.right, rcBounds.top + nDispHeight - (nDispHeight/m_sYAxisRuleFactor)*(i+1));*/

			int nValue = (i+1) * nMax / 5;
			int nQ = nValue/m_sYAxisScalingFactor;
			int nD = nValue%m_sYAxisScalingFactor;

			if (m_sYAxisScalingFactor != 1 /* i.e default*/ )
				Val.Format( "%d:%.2d",nQ,nD );
			else
				Val.Format( "%d",nQ);
			pdc->TextOut( rcBounds.left  - FontDim.cx/2, 
				rcBounds.bottom-(nDispHeight/5)*i - FontDim.cy/2 - (nDispHeight/5), 
				Val);

			pdc->SetTextColor( TranslateColor(GetForeColor()) );
			pdc->MoveTo(rcBounds.left - 3,  rcBounds.top + nDispHeight - (nDispHeight/5)*(i+1));
			pdc->LineTo(rcBounds.right, rcBounds.top + nDispHeight - (nDispHeight/5)*(i+1));

		}		
		pdc->SetTextAlign(nTextAlign);

		pOldPen = pdc->SelectObject( pOldPen );
	
		Pen.DeleteObject();

	}

	if (m_pSeriesData != NULL)
	{
		for (i = 0;i < m_sSeriesCount; i++)
		{
			if (m_pSeriesData[i].GraphType == EU_GT_2D_BAR_GRAPH) 
				nCurrentBarGraphSeries++;

			if ( m_pSeriesData[i].GraphType != EU_GT_PIE_GRAPH)			
			{
				for( C=0; C < m_pSeriesData[i].sDataValueCount; C++ )
				{
					V = m_pSeriesData[i].data[C];
					if (V >= 0) // || (m_pSeriesData[i].GraphType == EU_GT_FLOOD_FILLED_GRAPH) )
					{
						if (C==0)
							Bar.left = rcBounds.left +  + Gap/2;
						else
							Bar.left = rcBounds.left + C*BarWidth + Gap/2;

						if ( (nMax <= 10) || (V == 0) )
							Bar.top = rcBounds.top + nDispHeight - int((V)*fAspect);
						else
							Bar.top = rcBounds.top + nDispHeight - int((V-1)*fAspect);

						Bar.right = Bar.left+BarWidth - Gap;
						Bar.bottom = rcBounds.bottom;
						if( m_pSeriesData[i].GraphType == EU_GT_2D_BAR_GRAPH )
						{
							if (nActualBarWidth == -1)
							{
								// Graph has support for multiple bars per series, but each series bar is drawn
								// in it's appropriate place in the section. One series forms a section & there
								// could be multiple sections per graph.							

								for (int k = 0; k < m_sSeriesCount; k++)							
									if(m_pSeriesData[k].GraphType == EU_GT_2D_BAR_GRAPH)
										nBarGraphSeries++;
								
								nActualBarWidth = nBarGraphSeries == 1 ? BarWidth : (BarWidth - Gap) / nBarGraphSeries;
							}
							if (nActualBarWidth > 1 && nBarGraphSeries > 1)
									Bar.left += nCurrentBarGraphSeries * nActualBarWidth;
							
							Bar.right = Bar.left + nActualBarWidth;
							if (nBarGraphSeries == 1)
								Bar.right -= Gap;
							if( Bar.bottom == Bar.top )
								Bar.top--;								
							pdc->FillSolidRect( &Bar, m_pSeriesData[i].GraphColor);
							
						}
						else if (m_pSeriesData[i].GraphType == EU_GT_POINT_AS_CIRCLE_GRAPH || 
								 m_pSeriesData[i].GraphType == EU_GT_POINT_AS_RECTANGLE_GRAPH || 
								 m_pSeriesData[i].GraphType == EU_GT_POINT_AS_DIAMOND_GRAPH)
						{
							Br.CreateSolidBrush( m_pSeriesData[i].GraphColor);
							pOldBrush = pdc->SelectObject( &Br);

							DrawPointGraph(pdc, m_pSeriesData[i].GraphType, Bar, Br, BarWidth);

							Br.DeleteObject();
							Pen.DeleteObject();
						}
						else if (m_pSeriesData[i].GraphType == EU_GT_LINE_GRAPH || 
								 m_pSeriesData[i].GraphType == EU_GT_FLOOD_FILLED_GRAPH)
						{
							Pen.CreatePen(  PS_SOLID, 3, m_pSeriesData[i].GraphColor);
							pOldPen = pdc->SelectObject( &Pen );

							Br.CreateSolidBrush( m_pSeriesData[i].GraphColor );
							pOldBrush = pdc->SelectObject( &Br);							

							/*if( Bar.bottom == Bar.top )
								Bar.top--;*/
							if( C )
							{
								pdc->LineTo( Bar.left+(Bar.right-Bar.left)/2, Bar.top );
								
								if (m_pSeriesData[i].GraphType == EU_GT_FLOOD_FILLED_GRAPH)
								{					
									PointList[C + 1].x=Bar.left+(Bar.right-Bar.left)/2;
									PointList[C + 1].y=Bar.top;					

									PointList[C + 2].x=Bar.left+(Bar.right-Bar.left)/2;
									PointList[C + 2].y=Bar.bottom;					
								}
							}
							else
							{
								lSeriesCount = 0;
								for (int k = m_sXAxisNumberOfIntervals - 1; k >= 0 && m_pSeriesData[i].data[k] == -1; k--)
								{
									lSeriesCount++;
								}

								pdc->MoveTo( Bar.left+(Bar.right-Bar.left)/2, Bar.top );

								if (m_pSeriesData[i].GraphType == EU_GT_FLOOD_FILLED_GRAPH)
								{
									PointList = DEBUG_NEW POINT [m_pSeriesData[i].sDataValueCount + 3 - lSeriesCount];

									PointList[0].x=Bar.left+(Bar.right-Bar.left)/2;
									PointList[0].y=Bar.bottom;

									PointList[1].x=Bar.left+(Bar.right-Bar.left)/2;
									PointList[1].y=Bar.top;

									if (lSeriesCount == m_pSeriesData[i].sDataValueCount - 1)
									{
										PointList[0].x=Bar.left;
										PointList[0].y=Bar.bottom;

										PointList[2].x = Bar.left+(Bar.right-Bar.left)/2;
										PointList[2].y = Bar.bottom;
									}										

								}					
							}

							if ((C == m_pSeriesData[i].sDataValueCount - lSeriesCount - 1) && (m_pSeriesData[i].GraphType == EU_GT_FLOOD_FILLED_GRAPH))
							{
								PointList[C + 3] = PointList[0];

								FillRgn.CreatePolygonRgn( PointList, m_pSeriesData[i].sDataValueCount + 3 - lSeriesCount, WINDING );					
								pdc->FillRgn( &FillRgn, &Br );

								FillRgn.DeleteObject();

								delete []PointList;
							}
							
							pOldBrush = pdc->SelectObject( pOldBrush);				
							Br.DeleteObject();

							pOldPen = pdc->SelectObject( pOldPen );
							Pen.DeleteObject();
						}
					}
				}
			}			
		}
	}

	BOOL bLongTick = FALSE;
	CString csTempStr = m_csXAxisIntervalString; 
	CString	csLeftStr = "";
	int nMatch = 0;

	Pen.CreatePen(PS_SOLID, 1, TranslateColor(GetForeColor()));
	pOldPen = pdc->SelectObject( &Pen );

	nTextAlign = pdc->SetTextAlign(TA_CENTER);
	pdc->SetTextColor( TranslateColor(GetForeColor()) );
	CRect rectToDisplayLabel;

	short sTickLength = (rcBounds.bottom - rcBounds.top)/20;

	for( C=0; C < m_sXAxisNumberOfIntervals; C++ )
	{
		if (C == 0)
			Bar.left = rcBounds.left;
		else
			Bar.left = rcBounds.left + C*BarWidth;

		Bar.right = Bar.left+BarWidth;// - Gap;
		Bar.bottom = rcBounds.bottom;

		nMatch = csTempStr.Find(',');
		if (nMatch != -1)
		{
			csLeftStr = csTempStr.Left(nMatch);
			csTempStr = csTempStr.Right(csTempStr.GetLength() - (nMatch + 1));

			Val.Format("%2d",C + 1000);
			FontDim = pdc->GetTextExtent(Val);
			
			if (m_sXAxisLabelDisplayStartOffset != 0 && m_sXAxisLabelSkipCount != 0)
			{
				if (bLongTick)
				{
					pdc->MoveTo( Bar.left, Bar.bottom);
					pdc->LineTo( Bar.left, Bar.bottom + (int)(1.6 * sTickLength));
				}
				else
				{
					pdc->MoveTo( Bar.left, Bar.bottom);
					pdc->LineTo( Bar.left, Bar.bottom + sTickLength);
				}			
			}
			
			if ( (m_sXAxisLabelSkipCount == 0) || (C == m_sXAxisLabelDisplayStartOffset) || ( (C % (m_sXAxisLabelSkipCount+1)) == m_sXAxisLabelDisplayStartOffset))
			{
				if( (m_sXAxisLabelSkipCount == 0) || (m_sXAxisLabelDisplayStartOffset == 0) )
				{
					pdc->MoveTo( Bar.left, Bar.bottom);
					pdc->LineTo( Bar.left, Bar.bottom + (int)(1.6 * sTickLength));		
				}
				bLongTick = TRUE;

				rectToDisplayLabel = Bar;
				rectToDisplayLabel.right = Bar.right + Gap;

				pdc->TextOut(rectToDisplayLabel.right - (rectToDisplayLabel.right - rectToDisplayLabel.left)/2, rectToDisplayLabel.bottom + (int)(1.8 * sTickLength), csLeftStr);
			}
			else
			{
				if(m_sXAxisLabelDisplayStartOffset == 0)
				{
					pdc->MoveTo( Bar.left, Bar.bottom);
					pdc->LineTo( Bar.left, Bar.bottom + sTickLength);
				}
				bLongTick = FALSE;
			}			
		}		
	}

	pdc->SetTextAlign(nTextAlign);

	pdc->MoveTo( Bar.right, Bar.bottom);
	pdc->LineTo( Bar.right, Bar.bottom + (int)(1.6 * sTickLength));


	/*if( m_ShowRule )
	{
		nTextAlign = pdc->SetTextAlign(TA_RIGHT);

		Val.Format( "%d", 0);
		pdc->TextOut( rcBounds.left - FontDim.cx/2, 
			rcBounds.bottom - FontDim.cy/2,
			Val);

		for( int i=0; i < 5; i++ )
		{
			Val.Format( "%d", (i+1) * nMax / 5);
			pdc->TextOut( rcBounds.left  - FontDim.cx/2, 
				rcBounds.bottom-(nDispHeight/5)*i - FontDim.cy/2 - (nDispHeight/5), 
				Val);

			pdc->SetTextColor( TranslateColor(GetForeColor()) );
			pdc->MoveTo(rcBounds.left - 3,  rcBounds.top + nDispHeight - (nDispHeight/5)*(i+1));
			pdc->LineTo(rcBounds.right, rcBounds.top + nDispHeight - (nDispHeight/5)*(i+1));

		}		
		pdc->SetTextAlign(nTextAlign);
	}*/
	
	pOldPen = pdc->SelectObject( pOldPen );
	
	Pen.DeleteObject();

}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphCtrl::DoPropExchange - Persistence support

void CEuGraphCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphCtrl::GetControlFlags -
// Flags to customize MFC's implementation of ActiveX controls.
//
// For information on using these flags, please see MFC technical note
// #nnn, "Optimizing an ActiveX Control".
DWORD CEuGraphCtrl::GetControlFlags()
{
	DWORD dwFlags = COleControl::GetControlFlags();


	// The control will not be redrawn when making the transition
	// between the active and inactivate state.
	dwFlags |= noFlickerActivate;
	return dwFlags;
}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphCtrl::OnResetState - Reset control to default state

void CEuGraphCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphCtrl::AboutBox - Display an "About" box to the user

void CEuGraphCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_EUGRAPH);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphCtrl message handlers

void CEuGraphCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// This would have Menu to change the graph display type. 
	
}

short CEuGraphCtrl::GetDefaultGraphType() 
{
	// TODO: Add your property handler here
	
	return (short)m_GraphType;
}

void CEuGraphCtrl::SetDefaultGraphType(short sNewValue) 
{
	// TODO: Add your property handler here
	m_GraphType = (EU_GRAPH_TYPES)sNewValue;

	SetModifiedFlag();
}

short CEuGraphCtrl::GetSeriesCount() 
{
	// TODO: Add your property handler here
	return m_sSeriesCount;
}

void CEuGraphCtrl::SetSeriesCount(short sNewValue) 
{
	// TODO: Add your property handler here
	m_sSeriesCount = sNewValue;

	// Allocate the Series Array
	if (m_pSeriesData)
		delete []m_pSeriesData;

	if (m_sSeriesCount > 0)
	{
		m_pSeriesData = DEBUG_NEW SeriesData[m_sSeriesCount];		
		for (int i = 0;i < m_sSeriesCount; i++)
			m_pSeriesData[i].data = NULL;
		m_sCurSeries = 0;
	}

	SetModifiedFlag();
}

short CEuGraphCtrl::GetXAxisNumberOfIntervals() 
{
	// TODO: Add your property handler here

	return m_sXAxisNumberOfIntervals;
}

void CEuGraphCtrl::SetXAxisNumberOfIntervals(short sNewValue) 
{
	// TODO: Add your property handler here
	m_sXAxisNumberOfIntervals = sNewValue;

	SetModifiedFlag();
}

void CEuGraphCtrl::RedrawGraph() 
{
	// TODO: Add your dispatch handler code here
	SetModifiedFlag();
	InvalidateControl();
}

long CEuGraphCtrl::SetSeriesData(short sGraphType, long FAR* plDataValue, short sDataValueCount, OLE_COLOR GraphColor, LPCTSTR csLegendStrings) 
{
	// TODO: Add your dispatch handler code here
	m_pSeriesData[m_sCurSeries].GraphType = (EU_GRAPH_TYPES)sGraphType;
	m_pSeriesData[m_sCurSeries].sDataValueCount = sDataValueCount;

	if (m_pSeriesData[m_sCurSeries].data == NULL)
		delete []m_pSeriesData[m_sCurSeries].data;
	
	m_pSeriesData[m_sCurSeries].data = DEBUG_NEW long[sDataValueCount];
	for (int i = 0;i < sDataValueCount; i++)
		m_pSeriesData[m_sCurSeries].data[i] = plDataValue[i];

	m_pSeriesData[m_sCurSeries].GraphColor = GraphColor;	
	_tcscpy(m_pSeriesData[m_sCurSeries].szLegendString,csLegendStrings);

	m_sCurSeries ++;

	return m_sCurSeries;
}


void CEuGraphCtrl::PlotPieGraph(CDC * pdc, CRect & rcBounds, const CRect & rcInvalid)
{
	if (m_pSeriesData[0].GraphType != EU_GT_PIE_GRAPH)
		return;

	ULONG	sum = 0,total = 0,startAngle,arcAngle;
	int		j;
	int i = 0;
	CBrush Br, *pOldBrush;
	CPoint	PrevPoint,CurPoint;
	double dStartAngle = 0, dEndAngle;
	double dOffX = 0, dOffY = 0, dX = 0 , dY = 0;

	for(j=0;j<m_pSeriesData[0].sDataValueCount;j++)
		total += m_pSeriesData[0].data[j];
	
	int radius = (rcBounds.bottom - rcBounds.top)/2 - 5;

	if (total >0)
	{
		for(j=0;j<m_pSeriesData[0].sDataValueCount;j++)
		{
			if (m_pSeriesData[0].data[j])
			{
				Br.CreateSolidBrush( m_PieGraphData.pColorRef[j] );
				pOldBrush = pdc->SelectObject( &Br);

				pdc->MoveTo((rcBounds.right- rcBounds.left)/2, (rcBounds.bottom - rcBounds.top)/2);
				startAngle = sum*360L/total;
				if (j == m_pSeriesData[0].sDataValueCount-1)
					arcAngle = 360-startAngle;
				else
					arcAngle = m_pSeriesData[0].data[j]*360L/total;
				
				dStartAngle = ((double)startAngle)*pi/(double)180;

				dOffX = (radius*sin(dStartAngle ));
				dOffY = 0.0 - (radius*cos(dStartAngle ));

				dX = ((double)(rcBounds.right+rcBounds.left))/2.0;
				dY = ((double)(rcBounds.top+rcBounds.bottom))/2.0;
				
				PrevPoint.x = (int)(dX + dOffX);
				PrevPoint.y = (int)(dY + dOffY);

				dEndAngle = ((double)arcAngle + startAngle)*pi/(double)180;

				dOffX = (radius*sin(dEndAngle));
				dOffY = 0.0 - (radius*cos(dEndAngle));

				dX = ((double)(rcBounds.right+rcBounds.left))/2.0;
				dY = ((double)(rcBounds.top+rcBounds.bottom))/2.0;
				
				CurPoint.x = (int)(dX + dOffX);
				CurPoint.y = (int)(dY + dOffY);

				pdc->Pie(rcBounds,CurPoint, PrevPoint);

				sum += m_pSeriesData[0].data[j];
			
				pOldBrush = pdc->SelectObject( pOldBrush);
				Br.DeleteObject();
			}
		}		
	}
	else
		pdc->Ellipse(rcBounds);
	
	
	return;
}

void CEuGraphCtrl::DrawLegends( CDC * pdc, CRect & rcBounds, COLORREF *pColor, LPCTSTR lpLgdString , short sLegendCount)
{
	CString Val;
	CSize FontDim;
	CFont RuleFont; // *pOldFont;
	CBrush Br, *pOldBrush;

	CPen ColorPen, *pOldColorPen;

	CRgn Rgn;
	POINT PtList[7];

	short	sLgdRectWidth = (rcBounds.right - rcBounds.left)/5;//25;
	short	sLgdRectHeight = (rcBounds.bottom - rcBounds.top)/9; //12;
	short	sLgdBoxInterSpace = sLgdRectHeight;

	CString	csLgdString = lpLgdString;
	short sCenter = 0,sRadius = 0;

	RECT	rcLegend;

	int i = 0, k = 0;

	Val = m_csTitleFontName;
	if( Val=="" )
		Val = "Times New Roman";
	RuleFont.CreateFont( (rcBounds.bottom - rcBounds.top) / 10, 0, 0, 0, 
		FW_NORMAL,FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, Val );
	//pOldFont = pdc->SelectObject( &RuleFont );
	pdc->SetTextColor( TranslateColor(GetForeColor()) );

	FontDim = pdc->GetTextExtent("W");

	CPen Pen, *pOldPen;
	Pen.CreatePen(PS_SOLID, 1, TranslateColor(GetForeColor()));

    pOldPen = pdc->SelectObject( &Pen );
	
	if (pColor == NULL || lpLgdString == NULL || sLegendCount == 0)
	{
		for (i = 0; i < m_sSeriesCount; i++)
		{
			if (m_pSeriesData != NULL)
			{
				rcLegend.left = rcBounds.left + sLgdRectWidth/3;
				rcLegend.top = rcBounds.top + i * FontDim.cy + i * sLgdBoxInterSpace;
				rcLegend.right = rcLegend.left + sLgdRectWidth + FontDim.cx/2;
				rcLegend.bottom = rcLegend.top + FontDim.cy;		

				Br.CreateSolidBrush( m_pSeriesData[i].GraphColor);
				pOldBrush = pdc->SelectObject( &Br);

				// Depending upon the type of graph, draw the legend.

				switch(m_pSeriesData[i].GraphType)
				{
					case EU_GT_FLOOD_FILLED_GRAPH:
						
						PtList[0].x = rcLegend.left;
						PtList[0].y = rcLegend.bottom;	

						PtList[1].x = rcLegend.left;
						PtList[1].y = rcLegend.top + abs(rcLegend.bottom - rcLegend.top)/2;
						
						PtList[2].x = rcLegend.left + abs(rcLegend.right - rcLegend.left)/4;
						PtList[2].y = rcLegend.top;
						
						PtList[3].x = rcLegend.left + abs(rcLegend.right - rcLegend.left)/2;
						PtList[3].y = rcLegend.top + abs(rcLegend.bottom - rcLegend.top)/2;				

						PtList[4].x = rcLegend.left + abs(rcLegend.right - rcLegend.left)/2 + abs(rcLegend.right - rcLegend.left)/4;
						PtList[4].y = rcLegend.top;				

						PtList[5].x = rcLegend.right;
						PtList[5].y = rcLegend.top + abs(rcLegend.bottom - rcLegend.top)/2;				

						PtList[6].x = rcLegend.right;
						PtList[6].y = rcLegend.bottom;				

						Rgn.CreatePolygonRgn( PtList, 7, WINDING );
						
						pdc->FillRgn( &Rgn, &Br );

						pdc->MoveTo(PtList[0]);
						for (k = 1; k < 7; k++)
							pdc->LineTo(PtList[k]);
						pdc->LineTo(PtList[0]);

						Rgn.DeleteObject();
					break;

					case EU_GT_2D_BAR_GRAPH:
						
						PtList[0].x = rcLegend.left;
						PtList[0].y = rcLegend.bottom;	

						PtList[1].x = rcLegend.left;
						PtList[1].y = rcLegend.top;
						
						PtList[2].x = rcLegend.right;
						PtList[2].y = rcLegend.top;
						
						PtList[3].x = rcLegend.right;
						PtList[3].y = rcLegend.bottom;				

						Rgn.CreatePolygonRgn( PtList, 4, WINDING );
						
						pdc->FillRgn( &Rgn, &Br );

						pdc->MoveTo(PtList[0]);
						for (k = 1; k < 4; k++)
							pdc->LineTo(PtList[k]);
						pdc->LineTo(PtList[0]);

						Rgn.DeleteObject();
						break;

					case EU_GT_LINE_GRAPH:
						ColorPen.CreatePen(PS_SOLID, 3, m_pSeriesData[i].GraphColor);
						pOldColorPen = pdc->SelectObject( &ColorPen );

						pdc->MoveTo(rcLegend.left,rcLegend.top + abs(rcLegend.bottom - rcLegend.top)/2);
						pdc->LineTo(rcLegend.right,rcLegend.top + abs(rcLegend.bottom - rcLegend.top)/2);

						pOldColorPen = pdc->SelectObject( pOldColorPen);
						ColorPen.DeleteObject();
						break;

					case EU_GT_POINT_AS_CIRCLE_GRAPH:						
					case EU_GT_POINT_AS_RECTANGLE_GRAPH:						
					case EU_GT_POINT_AS_DIAMOND_GRAPH:
						DrawPointGraph(pdc, m_pSeriesData[i].GraphType, rcLegend, Br, -1, EU_POINT_GRAPH_CENTER);
						break;						
					default:
						break;
				}

				pOldBrush = pdc->SelectObject( pOldBrush);
				Br.DeleteObject();

				pdc->TextOut(rcLegend.right + 5, rcLegend.top, m_pSeriesData[i].szLegendString);			
			}			
		}		
	}
	else
	{	
		int nMatchLoc = -1;
		nMatchLoc = csLgdString.Find(',');
		CString csStringToDisplay = csLgdString.Left(nMatchLoc);
		for (i = 0;i < sLegendCount && pColor[i] && !csStringToDisplay.IsEmpty(); i++)
		{
			Br.CreateSolidBrush( pColor[i]);
			pOldBrush = pdc->SelectObject( &Br);

			rcLegend.left = rcBounds.left + 10;
			rcLegend.top = rcBounds.top + i * FontDim.cy + i * sLgdBoxInterSpace;
			rcLegend.right = rcLegend.left + sLgdRectWidth + FontDim.cx/2;
			rcLegend.bottom = rcLegend.top + FontDim.cy;		

			/*pdc->Rectangle( &rcLegend);
			pdc->FillSolidRect( &rcLegend, pColor[i]);*/
			PtList[0].x = rcLegend.left;
			PtList[0].y = rcLegend.bottom;	

			PtList[1].x = rcLegend.left;
			PtList[1].y = rcLegend.top;
			
			PtList[2].x = rcLegend.right;
			PtList[2].y = rcLegend.top;
			
			PtList[3].x = rcLegend.right;
			PtList[3].y = rcLegend.bottom;				

			Rgn.CreatePolygonRgn( PtList, 4, WINDING );
			
			pdc->FillRgn( &Rgn, &Br );

			pdc->MoveTo(PtList[0]);
			for (k = 1; k < 4; k++)
				pdc->LineTo(PtList[k]);
			pdc->LineTo(PtList[0]);

			Rgn.DeleteObject();
			pdc->TextOut(rcLegend.right + 5, rcLegend.top, csStringToDisplay);

			csLgdString = csLgdString.Right(csLgdString.GetLength() - (nMatchLoc + 1));
			nMatchLoc = csLgdString.Find(',');
			csStringToDisplay = csLgdString.Left(nMatchLoc);

			pOldBrush = pdc->SelectObject( pOldBrush);
			Br.DeleteObject();
		}		

	}

	pOldPen = pdc->SelectObject( pOldPen );	
	Pen.DeleteObject();

}

void CEuGraphCtrl::PieGraph(long FAR* plData, short sCount, OLE_COLOR* pColors, LPCTSTR csLegendStrings) 
{
	m_sCurSeries = 1;
	m_sSeriesCount = 1;
	m_pSeriesData = DEBUG_NEW SeriesData[m_sSeriesCount];
	
	m_pSeriesData[0].GraphType = EU_GT_PIE_GRAPH;
	m_pSeriesData[0].sDataValueCount = sCount;

	if (m_PieGraphData.pColorRef != NULL)
		delete m_PieGraphData.pColorRef;

	m_pSeriesData[0].data = DEBUG_NEW long[sCount];
	m_PieGraphData.pColorRef = DEBUG_NEW COLORREF[sCount];
	for (int i = 0;i < sCount; i++)
	{
		m_pSeriesData[0].data[i] = plData[i];
		m_PieGraphData.pColorRef[i] = pColors[i];
	}
	
	_tcscpy(m_PieGraphData.szLegendString, csLegendStrings);
	m_PieGraphData.sLegendCount = sCount;

	InvalidateControl();	


}

void CEuGraphCtrl::DrawPointGraph(CDC * pdc, EU_GRAPH_TYPES GraphType, const CRect &rcRect, CBrush &Br, short sBarWidth, EU_POINT_GRAPH_ALIGNMENT AlignType)
{
	short sRadius = 0;
	short sYCenter = 0;

	if ( (sBarWidth != -1) && (sRadius >= sBarWidth) )
		sRadius = sBarWidth / 4 ;
	else
		 sRadius = m_sRadiusForPointGraphs; //4;

	short sSize = (rcRect.right - rcRect.left) / 2;

	switch(AlignType)
	{
		case EU_POINT_GRAPH_TOP:
			sYCenter = (short)rcRect.top;
			break;
		case EU_POINT_GRAPH_CENTER:
			sYCenter = rcRect.top + (rcRect.bottom - rcRect.top) / 2;
			break;
		default:
			break;
	}

	if (GraphType == EU_GT_POINT_AS_CIRCLE_GRAPH)
	{				
		pdc->Ellipse(rcRect.left + sSize - sRadius, sYCenter - sRadius, rcRect.left + sSize + sRadius, sYCenter + sRadius);
	}
	else if (GraphType == EU_GT_POINT_AS_RECTANGLE_GRAPH)
	{
		pdc->Rectangle(rcRect.left + sSize - sRadius, sYCenter - sRadius, rcRect.left + sSize + sRadius, sYCenter + sRadius);
	}
	else if (GraphType == EU_GT_POINT_AS_DIAMOND_GRAPH)
	{
		CRgn Rgn;
		POINT PtList[4];

		PtList[0].x=rcRect.left + sSize - sRadius;
		PtList[0].y=sYCenter;

		PtList[1].x=rcRect.left + sSize;
		PtList[1].y=sYCenter - sRadius;

		PtList[2].x=rcRect.left + sSize + sRadius;
		PtList[2].y=sYCenter;

		PtList[3].x=rcRect.left + sSize;
		PtList[3].y=sYCenter + sRadius;
		
		Rgn.CreatePolygonRgn( PtList, 4, WINDING );
		
		pdc->FillRgn( &Rgn, &Br );

		Rgn.DeleteObject();
	}				

}

void CEuGraphCtrl::SetXAxisLabelInformation(LPCTSTR szXAxisLabelString, short sLabelDisplayStartOffset, short sLabelDisplaySkipCount) 
{
	if(szXAxisLabelString)
		m_csXAxisIntervalString = szXAxisLabelString;
	m_sXAxisLabelDisplayStartOffset = sLabelDisplayStartOffset - 1;
	m_sXAxisLabelSkipCount = sLabelDisplaySkipCount;	

	SetModifiedFlag();
}

void CEuGraphCtrl::SetYAxisScalingFactor(short sYAxisScaleFactor) 
{
	if (sYAxisScaleFactor==42/*Magic value that means we're really a percentage*/)
		m_bPercent = 1;
	else
	{
		m_sYAxisScalingFactor = sYAxisScaleFactor;
		m_sYAxisRuleFactor = m_sYAxisScalingFactor / 10;
	}
}
