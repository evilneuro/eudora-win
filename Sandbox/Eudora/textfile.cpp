// TEXTFILE.CPP
//

#include "stdafx.h"

#include "doc.h"
#include "textfile.h"  
#include "mdichild.h"
#include "resource.h"  
#include "guiutils.h"
#include "rs.h"

#include "QCProtocol.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"

extern QCCommandStack		g_theCommandStack;
extern QCPluginDirector		g_thePluginDirector;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



/////////////////////////////////////////////////////////////////////////////
// CTextFileDoc

IMPLEMENT_SERIAL(CTextFileDoc, CDoc, 0 /* schema number*/ )

CTextFileDoc::CTextFileDoc()
{
	m_SaveAs = FALSE;
}

CTextFileDoc::~CTextFileDoc()
{
}

BOOL CTextFileDoc::OnSaveDocument(const char* pszPathName)
{
    CString winPath;
    CMDIChild* actWin	= (CMDIChild *)((CMDIChild *)AfxGetMainWnd())->GetActiveFrame();
    
    // m_SaveAs is here because we come through this function for Save, SaveAs, OnClose & Save 
    
    // if the *first* active window is the one its saving it as, behave like a save and skip this
    if (m_SaveAs && actWin->GetActiveView()->GetDocument()->GetPathName() != pszPathName)
    {
		while (actWin)
		{
			if (actWin->GetTemplateID() == IDR_NEWFILE) 
			{
 				winPath = actWin->GetActiveView()->GetDocument()->GetPathName();
               
 				if (winPath && winPath == pszPathName )			
				{
					// Found a match...get outta here
					ErrorDialog(IDS_ERR_TEXTFILE_OPEN);      
					return TRUE;
				}
			}
	    	actWin = (CMDIChild *)actWin->GetNextWindow(GW_HWNDNEXT);
		}
	}

	//Use the filename only to set the document title
	CString fullPathName = pszPathName;
	int pos = fullPathName.ReverseFind('\\');
	CString fileName = fullPathName.Right(fullPathName.GetLength() - pos - 1);
	CDocument::SetTitle(fileName);

	BOOL theReturn = CDocument::OnSaveDocument(pszPathName);
	
	return theReturn;
}


BEGIN_MESSAGE_MAP(CTextFileDoc, CDocument)
	//{{AFX_MSG_MAP(CTextFileDoc)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextFileDoc serialization

void CTextFileDoc::Serialize(CArchive& ar)
{
	POSITION pos = GetFirstViewPosition();
	
	if (pos)
		GetNextView(pos)->Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CTextFileDoc commands
void CTextFileDoc::OnFileSaveAs()
{
//	m_SaveAs = TRUE;
//    CDocument::OnFileSaveAs();
//	m_SaveAs = FALSE;

	CFileDialog theDlg(	FALSE, CRString(IDS_TEXT_EXTENSION), NULL, 
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
						CRString(IDS_TXT_HTML_FILE_FILTER), NULL );

	if ( theDlg.DoModal() == IDOK )
	{
		m_SaveAs = TRUE;
		OnSaveDocument( theDlg.GetPathName() );	
		m_SaveAs = FALSE;
	}
}

BOOL CTextFileDoc::SaveModified()
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
		//if (!DoSave(m_strPathName))
		OnFileSave();
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

void CTextFileDoc::OnFileSave()
{
	if ( m_strPathName.GetLength() )
		OnSaveDocument(m_strPathName);
	else
		OnFileSaveAs();
}


////////////////////////////////////////////////////////////////////////
// OnDynamicCommand [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL CTextFileDoc::OnDynamicCommand(
UINT uID )
{
	QCCommandObject*		pCommand;
	COMMAND_ACTION_TYPE		theAction;	
	CString					szTo;
	struct TRANSLATE_DATA	theData;

	if( ! g_theCommandStack.GetCommand( ( WORD ) uID, &pCommand, &theAction ) )
	{
		return FALSE;
	}

	if( ( pCommand == NULL ) || !theAction )
	{
		return FALSE;
	}

	if( theAction == CA_TRANSLATE_PLUGIN )
	{
		CView* pView = NULL;
		POSITION pos = GetFirstViewPosition();   
		if (pos != NULL)  
			pView = GetNextView(pos); 

		if( ( theData.m_pView = pView ) != NULL ) 
		{
			QCProtocol*	pProtocol = QCProtocol::QueryProtocol( QCP_TRANSLATE, ( CObject* )theData.m_pView);
			if( pProtocol == NULL ) 
				return FALSE;
			
			theData.m_pProtocol = pProtocol;
			theData.m_bBuildAddresses = FALSE;
			
			pCommand->Execute( theAction, &theData );

			return TRUE;
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////
// OnUpdateDynamicCommand [protected]
//
////////////////////////////////////////////////////////////////////////
void CTextFileDoc::OnUpdateDynamicCommand(
CCmdUI* pCmdUI)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	

	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) ( pCmdUI->m_nID ), &pCommand, &theAction ) )
		{
			CView* pView = NULL;
			POSITION pos = GetFirstViewPosition();   
			if (pos != NULL)  
				pView = GetNextView(pos); 
			
			if( pView && ( theAction == CA_TRANSLATE_PLUGIN ) )
			{
				pCmdUI->Enable( TRUE );
				return;
			}

		}
	}

	pCmdUI->ContinueRouting();
}

