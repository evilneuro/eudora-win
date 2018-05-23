// SearchDoc.cpp : implementation file
//
 
#include "stdafx.h"
#include "resource.h"
#include "rs.h"
#include "summary.h" 
#include "doc.h"
#include "mdichild.h"
#include "tocdoc.h"
#include "msgframe.h"
#include "compmsgd.h"
#include "guiutils.h"
#include "progress.h"
#include "mainfrm.h"
#include "SearchView.h"
#include "cursor.h"
#include "ReadMessageDoc.h"
#include "QCProtocol.h"

#ifdef IMAP4
#include "imapfol.h"
#endif // IMAP4

#include "SearchDoc.h"

// --------------------------------------------------------------------------

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// --------------------------------------------------------------------------

// One global SearchDoc to store user info that will be carried over to next
//  invoke of window. View should populate from the doc.
CSearchDoc	*gSearchDoc = NULL;  

// --------------------------------------------------------------------------
// CSearchDoc

IMPLEMENT_DYNCREATE(CSearchDoc, CDoc)

// --------------------------------------------------------------------------
// CSearchDoc [protected, constructor]
//
////////////////////////////////////////////////////////////////////////
CSearchDoc::CSearchDoc()
{
	ASSERT(gSearchDoc == NULL);
	gSearchDoc = this;
}

////////////////////////////////////////////////////////////////////////
// ~CSearchDoc [protected, destructor]
//
////////////////////////////////////////////////////////////////////////
CSearchDoc::~CSearchDoc()
{
	gSearchDoc = NULL;
}

////////////////////////////////////////////////////////////////////////
// OnNewDocument [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CSearchDoc::OnNewDocument()
{		
	if (!CDocument::OnNewDocument())
		return FALSE;

	SetTitle(CRString(IDS_SEARCH_TITLE));
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnCloseDocument [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void
CSearchDoc::OnCloseDocument()
{   
	//
	// Setting CDocument::m_bAutoDelete to FALSE forces MFC to bypass
	// the destruction of this object.  This allows the same CSearchDoc
	// object to live across multiple invocations of the CSearchView 
	// dialog.  However, it does force us to "manually" delete the
	// global 'gSearchDoc' object (i.e. ourselves) when we shutdown the
	// program.
	//
	this->m_bAutoDelete = FALSE;
	CDoc::OnCloseDocument();
}


/////////////////////////////////////////////////////////////////////////////
// CSearchDoc commands    

BEGIN_MESSAGE_MAP(CSearchDoc, CDocument)
	//{{AFX_MSG_MAP(CSearchDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
