// Machine generated IDispatch wrapper class(es) created with ClassWizard

#include "stdafx.h"
#include "eugraph.h"

#include "DebugNewHelpers.h"



/////////////////////////////////////////////////////////////////////////////
// _DEuGraph properties

OLE_COLOR _DEuGraph::GetBackColor()
{
	OLE_COLOR result;
	GetProperty(DISPID_BACKCOLOR, VT_I4, (void*)&result);
	return result;
}

void _DEuGraph::SetBackColor(OLE_COLOR propVal)
{
	SetProperty(DISPID_BACKCOLOR, VT_I4, propVal);
}

short _DEuGraph::GetDefaultGraphType()
{
	short result;
	GetProperty(0x1, VT_I2, (void*)&result);
	return result;
}

void _DEuGraph::SetDefaultGraphType(short propVal)
{
	SetProperty(0x1, VT_I2, propVal);
}

short _DEuGraph::GetSeriesCount()
{
	short result;
	GetProperty(0x2, VT_I2, (void*)&result);
	return result;
}

void _DEuGraph::SetSeriesCount(short propVal)
{
	SetProperty(0x2, VT_I2, propVal);
}

short _DEuGraph::GetXAxisNumberOfIntervals()
{
	short result;
	GetProperty(0x3, VT_I2, (void*)&result);
	return result;
}

void _DEuGraph::SetXAxisNumberOfIntervals(short propVal)
{
	SetProperty(0x3, VT_I2, propVal);
}

OLE_COLOR _DEuGraph::GetForeColor()
{
	OLE_COLOR result;
	GetProperty(DISPID_FORECOLOR, VT_I4, (void*)&result);
	return result;
}

void _DEuGraph::SetForeColor(OLE_COLOR propVal)
{
	SetProperty(DISPID_FORECOLOR, VT_I4, propVal);
}

LPFONTDISP _DEuGraph::GetFont()
{
	LPFONTDISP result;
	GetProperty(DISPID_FONT, VT_DISPATCH, (void*)&result);
	return result;
}

void _DEuGraph::SetFont(LPFONTDISP propVal)
{
	SetProperty(DISPID_FONT, VT_DISPATCH, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// _DEuGraph operations

void _DEuGraph::RedrawGraph()
{
	InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long _DEuGraph::SetSeriesData(short sGraphType, long* plDataValue, short sDataValueCount, unsigned long GraphColor, LPCTSTR csLegendStrings)
{
	long result;
	static BYTE parms[] =
		VTS_I2 VTS_PI4 VTS_I2 VTS_I4 VTS_BSTR;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		sGraphType, plDataValue, sDataValueCount, GraphColor, csLegendStrings);
	return result;
}

void _DEuGraph::PieGraph(long* plData, short sCount, unsigned long* pColors, LPCTSTR csLegendStrings)
{
	static BYTE parms[] =
		VTS_PI4 VTS_I2 VTS_PI4 VTS_BSTR;
	InvokeHelper(0x6, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 plData, sCount, pColors, csLegendStrings);
}

void _DEuGraph::SetXAxisLabelInformation(LPCTSTR szXAxisLabelString, short sLabelDisplayStartOffset, short sLabelDisplaySkipCount)
{
	static BYTE parms[] =
		VTS_BSTR VTS_I2 VTS_I2;
	InvokeHelper(0x7, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 szXAxisLabelString, sLabelDisplayStartOffset, sLabelDisplaySkipCount);
}

void _DEuGraph::SetYAxisScalingFactor(short sYAxisScaleFactor)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x8, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 sYAxisScaleFactor);
}

void _DEuGraph::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// _DEuGraphEvents properties

/////////////////////////////////////////////////////////////////////////////
// _DEuGraphEvents operations
