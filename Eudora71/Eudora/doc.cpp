// DOC.CPP
//
// Derived CDocument that does some handy functions
//

#include "stdafx.h"

#include "doc.h"
#include "resource.h"
#include "guiutils.h"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CDoc

IMPLEMENT_DYNCREATE(CDoc, COleDoc)

CDoc::CDoc()
{
}

CDoc::~CDoc()
{
}

BOOL CDoc::OnNewDocument()
{
	SetModifiedFlag(FALSE);     // make clean
	return (TRUE);
}

void CDoc::SetTitle(const char* pszTitle)
{
	// The title should only have to be set once.  This prevents screen flicker when saving.
	if (m_strTitle.IsEmpty())
		CDocument::SetTitle(pszTitle);
}

BOOL CDoc::OnSaveDocument(const char* /*pszPathName*/)
{
	// Always save to internal filename
	return (CDocument::OnSaveDocument(m_strPathName));
}

// Overridden so that the title gets used in the dialog that asks if changes need to be saved,
// rather than what MFC uses, the filename.
BOOL CDoc::SaveModified()
{
	if (!IsModified())
		return (TRUE);

	CString prompt;
	AfxFormatString1(prompt, IDS_SAVE_CHANGES, m_strTitle);
	switch (AfxMessageBox(prompt, MB_YESNOCANCEL))
	{
	case IDCANCEL:
		return (FALSE);       // don't continue

	case IDYES:
		// If so, either Save or Update, as appropriate
		if (!DoSave(m_strPathName))
			return (FALSE);       // don't continue
		break;
	
	case IDNO:
		break;

	default:
		ASSERT(FALSE);
		return (FALSE);       // don't continue
	}

	// If we get here, it may be the case that the user hit
	// No when asked to save the changes.  If this is so, then the modified
	// flag needs to be cleared so the save prompt doesn't come up again.
	SetModifiedFlag(FALSE);
	
	return (TRUE);    // keep going

}

BEGIN_MESSAGE_MAP(CDoc, CDocument)
	//{{AFX_MSG_MAP(CDoc)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDoc commands

void CDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsModified());
}

void CDoc::OnFileSave()
{
	static BOOL AlreadyIn = FALSE;
	if (AlreadyIn)
		return;
	AlreadyIn = TRUE;

	OnSaveDocument(NULL);
	if (ShiftDown())
		AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_SAVE_ALL);

	AlreadyIn = FALSE;
}

void CDoc::OnFileClose()
{
	if (ShiftDown())
		AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_CLOSE_ALL);
	else
	{
		// MFC bug.  Should just act like a normal window close.
		POSITION pos = GetFirstViewPosition();
		CView* View;
		CFrameWnd* Frame;
		
		if (pos && (View = GetNextView(pos)) && (Frame = View->GetParentFrame()))
			Frame->SendMessage(WM_CLOSE);
	}
}


// If we get here looking for whether or not to enable the File->Save As...
// menu item, then we don't want to enable it.  This has to be done because
// CDocument has a default handler that we don't want called.
void CDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CDoc::OnFileSaveAs()
{
	ASSERT(FALSE);
}
