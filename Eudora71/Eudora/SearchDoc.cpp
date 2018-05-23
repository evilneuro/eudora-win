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
#include "QCSharewareManager.h"
#include "fileutil.h"


#ifdef IMAP4
#include "ImapMailbox.h"
#endif // IMAP4

#include "SearchDoc.h"

// --------------------------------------------------------------------------

#include "DebugNewHelpers.h"

// --------------------------------------------------------------------------

// One global SearchDoc to store user info that will be carried over to next
//  invoke of window. View should populate from the doc.
CSearchDoc	*gSearchDoc = NULL;  

// --------------------------------------------------------------------------
// CSearchDoc

IMPLEMENT_DYNCREATE(CSearchDoc, CDoc)


BEGIN_MESSAGE_MAP(CSearchDoc, CDocument)
	//{{AFX_MSG_MAP(CSearchDoc)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


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


CSearchView * CSearchDoc::GetSearchView()
{
	CSearchView *	pSearchView = NULL;
	CView *			pView = NULL;
	POSITION		pos = GetFirstViewPosition();   
	if (pos != NULL)  
		pView = GetNextView(pos);

	ASSERT_VALID(pView);
	ASSERT_KINDOF(CSearchView, pView);

	if ( pView && pView->IsKindOf(RUNTIME_CLASS(CSearchView)) )
		pSearchView = dynamic_cast<CSearchView *>(pView);

	return pSearchView;
}


void CSearchDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	//	Only allow saved search feature with full feature set
	pCmdUI->Enable( UsingFullFeatureSet() ? TRUE : FALSE );
}


void CSearchDoc::OnFileSave()
{
/*
	if ( m_strPathName.GetLength() )
		OnSaveDocument(m_strPathName);
	else
		OnFileSaveAs();
*/

	//	For now always make "Save" behave like "Save As...". Since we continue
	//	to be limited to one "Find Messages" window I'm not sure it makes sense
	//	to save over the last saved search when the user next saves. This may
	//	make it slightly more painful for a user to slightly tweak and re-save
	//	a search, but at least the user will know exactly what is being saved.
	OnFileSaveAs();
}


// If we get here looking for whether or not to enable the File->Save As...
// menu item, then we don't want to enable it.  This has to be done because
// CDocument has a default handler that we don't want called.
void CSearchDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}


void CSearchDoc::OnFileSaveAs()
{
	//	Only allow saved search feature with full feature set
	if ( !UsingFullFeatureSet() )
		return;
	
	CString		szSavedSearchDir = EudoraDir + CRString(IDS_IDX_SRCH_MGR_SAVED_SEARCHES_DIR);

	//	Check to see if it exists
	BOOL		bDirectoryExists = FileExistsMT(szSavedSearchDir);

	//	Create it if it doesn't exist
	if (!bDirectoryExists)
		bDirectoryExists = CreateDirectory(szSavedSearchDir, NULL);

	if (bDirectoryExists)
	{
		CString			strSuggestedFileName;
		const char *	szSuggestedFileName = NULL;

		CSearchView *	pSearchView = GetSearchView();

		if (pSearchView)
		{
			//	Get the suggested file name that represents the first line of the search
			pSearchView->GetSuggestedFileName(strSuggestedFileName);
			if ( !strSuggestedFileName.IsEmpty() )
				szSuggestedFileName = strSuggestedFileName;
		}
		
		CFileDialog theDlg(	FALSE, CRString(IDS_SAVED_SEARCH_EXTENSION),
							szSuggestedFileName, 
							OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
							CRString(IDS_XML_FILE_FILTER), NULL );

		theDlg.m_ofn.lpstrInitialDir = szSavedSearchDir;

		if ( theDlg.DoModal() == IDOK )
			OnSaveDocument( m_strPathName = theDlg.GetPathName() );	
	}
	else
	{
		ErrorDialog(IDS_ERR_DIR_CREATE, szSavedSearchDir);
	}
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


BOOL
CSearchDoc::OnSaveDocument(const char * pszPathName)
{
	CSearchView *	pSearchView = GetSearchView();

	if (pSearchView)
		pSearchView->WriteCurrentSearchCriteria(pszPathName);

	return TRUE;
}

