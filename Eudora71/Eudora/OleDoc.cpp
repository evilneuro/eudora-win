// COleDoc.cpp : implementation file
//

#include "stdafx.h"
#include "assert.h"

#include "font.h"
#include "OleDoc.h"


#include "DebugNewHelpers.h"



/////////////////////////////////////////////////////////////////////////////
// COleDoc

IMPLEMENT_DYNCREATE(COleDoc, COleDocument)

COleDoc::COleDoc()
{
	m_pDefFont = 0;
}

COleDoc::~COleDoc()
{
}


BOOL COleDoc::OnNewDocument()
{
	if (!COleDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}


// caller must Release() this pointer
LPFONTDISP COleDoc::GetFontDispatch()
{
	if ( !m_pDefFont )
		init_default_font();
	else
		m_pDefFont->AddRef();

	assert( m_pDefFont );
	return m_pDefFont;
}

// derive font point size from logical units
int lu2pts( int logUnits )
{
	HDC hdc = ::GetDC( NULL );
	int pixPerInch = ::GetDeviceCaps( hdc, LOGPIXELSY );
	float pts = ((float)abs(logUnits)*72)/pixPerInch;

	// round up
	if ( pts > (int)pts )
		pts+=1;

	ReleaseDC( NULL, hdc );
	return (int)pts;
}

// initialize an ole font object from MessageFont
void COleDoc::init_default_font()
{
	// ole string conversion macro support
	USES_CONVERSION;

	LOGFONT lf;
	GetMessageFont().GetLogFont( &lf );

	FONTDESC fd;
	memset( &fd, '\0', sizeof(FONTDESC) );
	fd.cbSizeofstruct = sizeof(FONTDESC);
	fd.cySize.Hi      = 0;
	fd.cySize.Lo      = lu2pts(lf.lfHeight)*10000;
	fd.fItalic        = lf.lfItalic;
	fd.fStrikethrough = lf.lfStrikeOut;
	fd.fUnderline     = lf.lfUnderline;
	fd.lpstrName      = A2OLE(lf.lfFaceName);
	fd.sCharset       = lf.lfCharSet;
	fd.sWeight        = (short)lf.lfWeight;

	// QI happens here, don't ref count this
	HRESULT hr = OleCreateFontIndirect( &fd, IID_IFontDisp, (void**)&m_pDefFont );
	assert( SUCCEEDED(hr) );
}


BEGIN_MESSAGE_MAP(COleDoc, COleDocument)
	//{{AFX_MSG_MAP(COleDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COleDoc diagnostics

#ifdef _DEBUG
void COleDoc::AssertValid() const
{
	COleDocument::AssertValid();
}

void COleDoc::Dump(CDumpContext& dc) const
{
	COleDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// COleDoc serialization

void COleDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// COleDoc commands
