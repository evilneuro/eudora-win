// eudordoc.cpp : implementation of the CEudoraDoc class
//

#include "stdafx.h"
#include "eudora.h"

#include "eudordoc.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CEudoraDoc

IMPLEMENT_DYNCREATE(CEudoraDoc, CDocument)

BEGIN_MESSAGE_MAP(CEudoraDoc, CDocument)
	//{{AFX_MSG_MAP(CEudoraDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEudoraDoc construction/destruction

CEudoraDoc::CEudoraDoc()
{
	// TODO: add one-time construction code here
}

CEudoraDoc::~CEudoraDoc()
{
}

BOOL CEudoraDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CEudoraDoc serialization

void CEudoraDoc::Serialize(CArchive& ar)
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
// CEudoraDoc diagnostics

#ifdef _DEBUG
void CEudoraDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEudoraDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEudoraDoc commands
