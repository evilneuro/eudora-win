// SIGNATUR.CPP
//

#include "stdafx.h"

#include <QCUtils.h>

#include "doc.h"
#include "signatur.h"
#include "resource.h"
#include "rs.h"
#include "fileutil.h"
#include "mdichild.h"
#include "utils.h"
#include "guiutils.h"
#include "paige.h"
#include "PgStyleUtils.h"
#include "Paige_io.h"
#include "etf2html.h"
#include "pgosutl.h"
#include "pghtmdef.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSignatureDoc

IMPLEMENT_SERIAL(CSignatureDoc, CDoc, 0 /* schema number*/ )

BEGIN_MESSAGE_MAP(CSignatureDoc, CDocument)
	//{{AFX_MSG_MAP(CSignatureDoc)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSignatureDoc::CSignatureDoc()
{
	m_SaveAs = FALSE;
}

CSignatureDoc::~CSignatureDoc()
{
}

/*CPaigeEdtView* CSignatureDoc::GetView()
{
	POSITION pos = GetFirstViewPosition();

	if (pos)
		return ((CSignatureView*)GetNextView(pos));
		
	return (NULL);
} */


void CSignatureDoc::SetTitle(const char* pszTitle)
{
	CString title = pszTitle;

	if ( title.ReverseFind('.') > 0 )
		CDocument::SetTitle( title.Left(title.GetLength() - 4) );
	else
		CDocument::SetTitle(title);
}


void CSignatureDoc::OnFileSaveAs()
{
	CFileDialog theDlg(	FALSE, CRString(IDS_TEXT_EXTENSION), NULL, 
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
						CRString(IDS_TXT_HTML_FILE_FILTER), NULL );

	if ( theDlg.DoModal() == IDOK )
	{
		m_SaveAs = TRUE;
		
		//Use the filename only to set the document title
		CString pathName = theDlg.GetPathName();
		int pos = pathName.ReverseFind('\\');
		CString fileName = pathName.Right(pathName.GetLength() - pos - 1);
		CDocument::SetTitle(fileName);

		CDocument::OnSaveDocument(pathName);	
		
		m_SaveAs = FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSignatureDoc serialization

void CSignatureDoc::Serialize(CArchive& ar)
{
	POSITION pos = GetFirstViewPosition();
	
	if (pos)
	{
		if (m_SaveAs)
		{
			CSignatureView* pView = (CSignatureView*)GetNextView(pos);
			pView->CPaigeEdtView::Serialize(ar);
		}
		else
			GetNextView(pos)->Serialize(ar);
	}

	SetModifiedFlag(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CSignatureView

IMPLEMENT_DYNCREATE(CSignatureView, CPaigeEdtView)

CSignatureView::CSignatureView()
{
}

CSignatureView::~CSignatureView()
{
}

BEGIN_MESSAGE_MAP(CSignatureView, CPaigeEdtView)
	//{{AFX_MSG_MAP(CSignatureView)
	//}}AFX_MSG_MAP

	//ON_MESSAGE( WM_CONTEXTMENU, OnContextMenu )

END_MESSAGE_MAP()


void CSignatureView::Serialize(CArchive& ar)
{
	PgDataTranslator theTxr(m_paigeRef);
	PgDataTranslator::FormatType format = PgDataTranslator::kFmtText;

	if ( ar.IsStoring() ) 
	{
		// storing code
		
		if (PgHasStyledText(m_paigeRef, PgGlobalsPtr()) == TRUE)
			format = PgDataTranslator::kFmtHtml;

		MakeAutoURLSpaghetti(0, pgTextSize(m_paigeRef));

		theTxr.ExportData( ar.GetFile(), format);
		SaveChangeState();
	
	}
	else 
	{
		// loading code
		
		pgApplyNamedStyle(m_paigeRef,NULL,body_style,best_way);

		char *sigText = NULL;

		CFile *sigFile = ar.GetFile();

		int bytesToRead = sigFile->GetLength();

		sigText = new char[bytesToRead + 1];
		int bytesRead = sigFile->Read(sigText, bytesToRead);

		ASSERT(bytesRead > 0);
		sigText[bytesRead] = '\0';

		if ( IsFancy(sigText) == IS_RICH )
		{
			int NewSize = Etf2Html_Measure(sigText);
			char *NewSigText = new char[NewSize + 1];
			Etf2Html_Convert(NewSigText, sigText);

			// convert our c-string to a Paige Memory Manager reference
			memory_ref impRef = CString2Memory( PgMemGlobalsPtr(), NewSigText);
			format = PgDataTranslator::kFmtHtml;
			theTxr.ImportData( &impRef, format);
			if ( NewSigText )
			    delete[] NewSigText;

			UnuseAndDispose( impRef );
		}
		else if ( IsFancy(sigText) == IS_HTML)
		{
			format = PgDataTranslator::kFmtHtml;
			sigFile->Seek(0, CFile::begin);
			theTxr.ImportData( sigFile, format);
		}
		else
		{
			format = PgDataTranslator::kFmtText;
			sigFile->Seek(0, CFile::begin);
			theTxr.ImportData( sigFile, format);			
		}

		MakeAutoURLSpaghetti(0, pgTextSize(m_paigeRef));

		pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);

		delete [] sigText;
	}

}

/////////////////////////////////////////////////////////////////////////////
// CSignatureView diagnostics

#ifdef _DEBUG
void CSignatureView::AssertValid() const
{
	CPaigeEdtView::AssertValid();
}

void CSignatureView::Dump(CDumpContext& dc) const
{
	CPaigeEdtView::Dump(dc);
}
#endif //_DEBUG


