// Machine generated IDispatch wrapper class(es) created with ClassWizard
/////////////////////////////////////////////////////////////////////////////
// _DEuGraph wrapper class

class _DEuGraph : public COleDispatchDriver
{
public:
	_DEuGraph() {}		// Calls COleDispatchDriver default constructor
	_DEuGraph(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	_DEuGraph(const _DEuGraph& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:
	OLE_COLOR GetBackColor();
	void SetBackColor(OLE_COLOR);
	short GetDefaultGraphType();
	void SetDefaultGraphType(short);
	short GetSeriesCount();
	void SetSeriesCount(short);
	short GetXAxisNumberOfIntervals();
	void SetXAxisNumberOfIntervals(short);
	OLE_COLOR GetForeColor();
	void SetForeColor(OLE_COLOR);
	LPFONTDISP GetFont();
	void SetFont(LPFONTDISP);

// Operations
public:
	void RedrawGraph();
	long SetSeriesData(short sGraphType, long* plDataValue, short sDataValueCount, unsigned long GraphColor, LPCTSTR csLegendStrings);
	void PieGraph(long* plData, short sCount, unsigned long* pColors, LPCTSTR csLegendStrings);
	void SetXAxisLabelInformation(LPCTSTR szXAxisLabelString, short sLabelDisplayStartOffset, short sLabelDisplaySkipCount);
	void SetYAxisScalingFactor(short sYAxisScaleFactor);
	void AboutBox();
};
/////////////////////////////////////////////////////////////////////////////
// _DEuGraphEvents wrapper class

class _DEuGraphEvents : public COleDispatchDriver
{
public:
	_DEuGraphEvents() {}		// Calls COleDispatchDriver default constructor
	_DEuGraphEvents(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	_DEuGraphEvents(const _DEuGraphEvents& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
};
