// WazooBarMgr.cpp : implementation file
//
// CWazooBarMgr
// Owner for all of main frame's wazoo bars (containers)

#include "stdafx.h"


#include "WazooBar.h"
#include "AdWazooBar.h"
#include "WazooBarMgr.h"
#include "workbook.h"
#include "TaskStatusView.h"

#include "resource.h"
#include "rs.h"
#include "mainfrm.h"

#include "QCSharewareManager.h"

#include "MboxWazooWnd.h"
#include "SignatureWazooWnd.h"
#include "StationeryWazooWnd.h"
#include "FilterReportWazooWnd.h"
#include "FileBrowseWazooWnd.h"
#include "FiltersWazooWnd.h"
#include "NicknamesWazooWnd.h"
#include "PersonalityWazooWnd.h"
#include "DirectoryServicesWazooWndNewIface.h"
#include "TaskStatusWazooWnd.h"
#include "TaskErrorWazooWnd.h"
#include "AdWazooWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


//
// Static member data.
//
int CWazooBarMgr::s_nNextBarNumber = 1;

//
// Sane upper bound on the number of different Wazoo windows.
//
#define MAX_WAZOOS 13



CWazooBarMgr::CWazooBarMgr() :
	m_bUseDefaultConfig(FALSE)
{
	
}


CWazooBarMgr::~CWazooBarMgr()
{
	
}


////////////////////////////////////////////////////////////////////////
// CreateInitialWazooBars [public]
//
// Create a pool of dockable Wazoo Bar "containers" owned by the given
// frame window.  These containers are initially empty, ready to be
// redocked to their last known positions by the
// CMainFrame::LoadBarState() method.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBarMgr::CreateInitialWazooBars(CMDIFrameWnd* pMainFrame)
{  
	ASSERT(FALSE == m_bUseDefaultConfig);
	ASSERT(pMainFrame);
	ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);

	//
	// First, figure out how many Wazoo bars to create based on the
	// saved config from the last time we ran the program.  If there
	// is no Wazoo info from last time, then assume a default
	// configuration.
	//
	CDWordArray dwArrayBarIds;
	LoadWazooBarIdsFromIni(dwArrayBarIds);
	if (dwArrayBarIds.GetUpperBound() == -1)
	{
		//
		// Wazoo Bar Id's from previous session not found, so assume
		// default configuration.
		//
		dwArrayBarIds.Add(IDC_WAZOOBAR_BASE);
		dwArrayBarIds.Add(IDC_WAZOOBAR_BASE + 1);
		dwArrayBarIds.Add(IDC_WAZOOBAR_BASE + 2);

		m_bUseDefaultConfig = TRUE;
	}

	//
	// Create a pool of dockable Wazoo control bars...
	//
	ASSERT(m_WazooBarList.GetCount() == 0);
	for (int nBar = 0; nBar <= dwArrayBarIds.GetUpperBound(); nBar++)
	{
		CWazooBar* pWazooBar = new CWazooBar(this);
		if (NULL == pWazooBar)
			return FALSE;
			
		CString strTitle;
		strTitle.Format("WazooBar%d", s_nNextBarNumber++);					// FORNOW, needs better name
		if (! pWazooBar->Create(pMainFrame, 
								strTitle,
								CBRS_LEFT | WS_VISIBLE | CBRS_SIZE_DYNAMIC,
								CBRS_EX_STDCONTEXTMENU | CBRS_EX_ALLOW_MDI_FLOAT | CBRS_EX_BORDERSPACE | CBRS_EX_COOL,
								dwArrayBarIds[nBar]))
		{
			delete pWazooBar;
			return FALSE;      
		} 

		//
		// Allow for Wazoo bars to be docked on any edge.
		//
		pWazooBar->EnableDocking(CBRS_ALIGN_ANY);

		//
		// Add the Wazoo bar to the collection.
		//
		m_WazooBarList.AddTail(pWazooBar);
	}

	// Create a special wazoo bar for holding the Ad window
	CAdWazooBar* pAWB = new CAdWazooBar(this);
	pAWB->Create(pMainFrame);
	pAWB->EnableDocking(CBRS_ALIGN_ANY);
	m_WazooBarList.AddTail(pAWB);

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// CreateNewWazooBar [public]
//
// Add a new, empty Wazoo Bar to the pool of dockable Wazoo Bar
// "containers" owned by the given frame window.
////////////////////////////////////////////////////////////////////////
CWazooBar* CWazooBarMgr::CreateNewWazooBar(CMDIFrameWnd* pFrameWnd)
{  
	ASSERT(pFrameWnd);
	ASSERT_KINDOF(CMDIFrameWnd, pFrameWnd);

	QCWorkbook* pMainFrame = DYNAMIC_DOWNCAST(QCWorkbook, pFrameWnd);
	if (NULL == pMainFrame)
	{
		ASSERT(0);
		return NULL;
	}

	//
	// First, figure out the next available Wazoo Bar ID.  The idea is
	// to use the smallest unused ID starting at IDC_WAZOOBAR_BASE.
	//
	for (int nNextBarId = IDC_WAZOOBAR_BASE; ; nNextBarId++)
	{
		BOOL bFoundMatch = FALSE;
		POSITION pos = m_WazooBarList.GetHeadPosition();
		while (pos)
		{
			CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
			ASSERT_KINDOF(CWazooBar, pWazooBar);

			if (pWazooBar->GetDlgCtrlID() == nNextBarId)
			{
				bFoundMatch = TRUE;
				break;
			}
		}

		if (! bFoundMatch)
		{
			ASSERT(nNextBarId < IDC_WAZOOBAR_BASE + MAX_WAZOOS);			// sanity check for runaway id's
			break;
		}
	}

	//
	// Create a new dockable Wazoo control bar...
	//
	ASSERT(m_WazooBarList.GetCount() > 0);
	CWazooBar* pWazooBar = new CWazooBar(this);
	if (NULL == pWazooBar)
		return NULL;
		
	CString strTitle;
	strTitle.Format("WazooBar%d", s_nNextBarNumber++);						// FORNOW, needs better name
	if (! pWazooBar->Create(pMainFrame, 
							strTitle,
							CBRS_LEFT | WS_VISIBLE | CBRS_SIZE_DYNAMIC,
							CBRS_EX_STDCONTEXTMENU | CBRS_EX_ALLOW_MDI_FLOAT | CBRS_EX_BORDERSPACE | CBRS_EX_COOL,
							nNextBarId))
	{
		delete pWazooBar;
		return NULL;      
	} 

	//
	// Allow for Wazoo bars to be docked on any edge.
	//
	pWazooBar->EnableDocking(CBRS_ALIGN_ANY);

	//
	// We have to tell newly-created dockable bars where their default
	// docking location is, otherwise, they won't be visible.  Also,
	// Let's configure all newly-created Wazoo Bars to be MDI child
	// windows by default.
	//
	ASSERT(NULL == pWazooBar->m_pDockBar);
	pMainFrame->DockControlBarEx(pWazooBar, AFX_IDW_DOCKBAR_BOTTOM, 0, 0, (float)1.00, 180);
	pWazooBar->SendMessage(WM_COMMAND, ID_SEC_MDIFLOAT, 0);

	//
	// Set the initial size of the window to be 80% of the
	// current MDI client area.
	//
	{
		QCWorkbook* pMainFrame = (QCWorkbook *) ::AfxGetMainWnd();
		ASSERT_KINDOF(QCWorkbook, pMainFrame);

		CRect rectMDIClient;
		ASSERT(pMainFrame->m_hWndMDIClient);
		CWnd* pMDIClient = CWnd::FromHandle(pMainFrame->m_hWndMDIClient);
		pMDIClient->GetWindowRect(rectMDIClient);

		CMDIChildWnd* pMDIFrame = (CMDIChildWnd *) pWazooBar->GetParentFrame();
		ASSERT_KINDOF(CMDIChildWnd, pMDIFrame);
		pMDIFrame->SetWindowPos(NULL, 0, 0, (rectMDIClient.Width() * 4) / 5, (rectMDIClient.Height() * 4) / 5, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	//
	// Add the Wazoo bar to the collection.
	//
	m_WazooBarList.AddTail(pWazooBar);

	return pWazooBar;
}


////////////////////////////////////////////////////////////////////////
// DestroyAllWazooBars [public]
//
// Destroy the internally managed pool of dockable Wazoo Bar "containers".
////////////////////////////////////////////////////////////////////////
void CWazooBarMgr::DestroyAllWazooBars()
{  
	while (! m_WazooBarList.IsEmpty())
	{
		CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.RemoveHead();
		ASSERT_KINDOF(CWazooBar, pWazooBar);
		if (::IsWindow(pWazooBar->GetSafeHwnd()))
			pWazooBar->DestroyWindow();
		delete pWazooBar;
	}
}


////////////////////////////////////////////////////////////////////////
// UnlinkWazooBar [public]
//
// Remove the reference to the given Wazoo Bar from our collection,
// returning TRUE if successful.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBarMgr::UnlinkWazooBar(CWazooBar* pWazooBar)
{  
	ASSERT_KINDOF(CWazooBar, pWazooBar);

	POSITION pos = m_WazooBarList.GetHeadPosition();
	ASSERT(pos);
	while (pos)
	{
		if (pWazooBar == (CWazooBar *) m_WazooBarList.GetAt(pos))
		{
			m_WazooBarList.RemoveAt(pos);
			return TRUE;
		}

		m_WazooBarList.GetNext(pos);
	}

	ASSERT(0);
	return FALSE;
}


//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// DestroyWazooBar [public]
//FORNOW//
//FORNOW// Destroy a specific Wazoo Bar, which is assumed to be empty.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWBOOL CWazooBarMgr::DestroyWazooBar(CWazooBar* pWazooBar)
//FORNOW{  
//FORNOW	ASSERT_KINDOF(CWazooBar, pWazooBar);
//FORNOW
//FORNOW	POSITION pos = m_WazooBarList.GetHeadPosition();
//FORNOW	ASSERT(pos);
//FORNOW	while (pos)
//FORNOW	{
//FORNOW		if (pWazooBar == (CWazooBar *) m_WazooBarList.GetAt(pos))
//FORNOW		{
//FORNOW			m_WazooBarList.RemoveAt(pos);
//FORNOW
//FORNOW			//
//FORNOW			// FORNOW, this may or may not work.
//FORNOW			//
//FORNOW			ASSERT(pWazooBar->IsEmpty());
//FORNOW			pWazooBar->OnBarFloat();
//FORNOW			ASSERT_KINDOF(SECDockBar, pWazooBar->m_pDockBar);
//FORNOW			((SECDockBar *) pWazooBar->m_pDockBar)->RemoveControlBar(pWazooBar, -1, FALSE);
//FORNOW//			pWazooBar->DestroyWindow();
//FORNOW//			delete pWazooBar;
//FORNOW			return TRUE;
//FORNOW		}
//FORNOW		else
//FORNOW			m_WazooBarList.GetNext(pos);
//FORNOW	}
//FORNOW
//FORNOW	ASSERT(0);
//FORNOW	return FALSE;
//FORNOW}

bool CWazooBarMgr::SetDefaultWazooBarState(CWazooBar *pWazooBar, int nIndex, DefaultStateType tWhichType /* = DST_ALL */)
{
	SECWorkbook* pMainFrame = (SECWorkbook *) ::AfxGetMainWnd();
	ASSERT(pMainFrame);
	ASSERT_KINDOF(SECWorkbook, pMainFrame);

	switch (nIndex)
	{
		case 0: // CMboxWazooWnd,CFileBrowseWazooWnd,CStationeryWazooWnd,CSignatureWazooWnd,CPersonalityWazooWnd
		{
			if ((DST_SIZE_FIXED == tWhichType) || (DST_ALL == tWhichType))
			{
				pMainFrame->DockControlBarEx(pWazooBar, AFX_IDW_DOCKBAR_LEFT, 0, 0, (float)1.00, 180);
			}
		}
		break;

		case 1: // DirectoryServicesWazooWndNew,CNicknamesWazooWnd,CFiltersWazooWnd,CFilterReportWazooWnd
		{
			if ((DST_SIZE_RELATIVE == tWhichType) || (DST_ALL == tWhichType))
			{
				pMainFrame->DockControlBarEx(pWazooBar, AFX_IDW_DOCKBAR_RIGHT, 0, 0, (float)1.00, 180);

				//
				// Now that it is docked ... convert it to an
				// MDI child window.
				//
				pWazooBar->SendMessage(WM_COMMAND, ID_SEC_MDIFLOAT, 0);
				pMainFrame->RecalcLayout(FALSE);	// make sure MDI client area size is up to date

				//
				// Set the initial size of the window to be 80% of the
				// current MDI client area.
				//
				{
					QCWorkbook* pMainFrame = (QCWorkbook *) ::AfxGetMainWnd();
					ASSERT_KINDOF(QCWorkbook, pMainFrame);

					CRect rectMDIClient;
					ASSERT(pMainFrame->m_hWndMDIClient);
					CWnd* pMDIClient = CWnd::FromHandle(pMainFrame->m_hWndMDIClient);
					pMDIClient->GetWindowRect(rectMDIClient);

					QCControlBarWorksheet* pMDIFrame = (QCControlBarWorksheet *) pWazooBar->GetParentFrame();
					ASSERT_KINDOF(QCControlBarWorksheet, pMDIFrame);
					pMDIFrame->SetWindowPos(NULL,
												0,
												0,
												(rectMDIClient.Width() * 4) / 5,
												(rectMDIClient.Height() * 4) / 5,
												SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

					//
					// Hack workaround for the problem where the
					// initial MDI wazoo doesn't restore properly
					// when MDI is in maximized window mode.
					//
					pMDIFrame->m_bFirstActivationAfterClose = TRUE;
				}
			}


			if ((DST_SHOWHIDE == tWhichType) || (DST_ALL == tWhichType))
			{
				// Hide the MDI DS/AB/F/FR child window.
				pWazooBar->SendMessage(WM_COMMAND, ID_SEC_HIDE, 0);
			}
		}
		break;

		case 2: // CTaskStatusWazooWnd,CTaskErrorWazooWnd
		{
			if ((DST_SIZE_FIXED == tWhichType) || (DST_ALL == tWhichType))
			{
				// Notice the '180' is the WIDTH when docked vertically
				pMainFrame->DockControlBarEx(pWazooBar, AFX_IDW_DOCKBAR_BOTTOM, 1, 0, (float)1.00, 180);
				
				// Move the tabs to the left
				pWazooBar->SendMessage(WM_COMMAND, ID_WAZOO_TAB_ON_LEFT, 0);

				// Set the default height (pixels)
				{
					SECControlBarInfo info;
					pWazooBar->GetBarInfo(&info);
					info.m_szDockHorz.cy = 80;
					pWazooBar->SetBarInfo(&info, pMainFrame);
				}
			}

			if ((DST_SHOWHIDE == tWhichType) || (DST_ALL == tWhichType))
			{
				// Hide the Task Status/Task Error window
				pWazooBar->SendMessage(WM_COMMAND, ID_SEC_HIDE, 0);
			}
		}
		break;

		case 3: // CAdWazooWnd
		{
			if ((DST_SIZE_FIXED == tWhichType) || (DST_ALL == tWhichType))
			{
				// Notice the '180' is the WIDTH when docked vertically
				pMainFrame->DockControlBarEx(pWazooBar, AFX_IDW_DOCKBAR_LEFT, 1, 0, (float)0.25, 180);

				// Set the default height (pixels)
				{
					SECControlBarInfo info;
					pWazooBar->GetBarInfo(&info);
					info.m_szDockHorz.cy = 180;
					pWazooBar->SetBarInfo(&info, (CFrameWnd*)AfxGetMainWnd());
				}
			}
		}
		break;

		default:
		{
			ASSERT(0);
			return (false);
		}
	}

	return (true);
}

////////////////////////////////////////////////////////////////////////
// LoadWazooBarConfigFromIni [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CWazooBarMgr::LoadWazooBarConfigFromIni()
{
	if (m_bUseDefaultConfig)
	{
		POSITION pos;
		int idx;

		// Fill the bars with appropriate Wazoos
		pos = m_WazooBarList.GetHeadPosition();
		for (idx = 0; pos; idx++)
		{
			CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
			ASSERT_KINDOF(CWazooBar, pWazooBar);
			ASSERT(pWazooBar->GetDlgCtrlID() == IDC_WAZOOBAR_BASE+idx || pWazooBar->GetDlgCtrlID() == IDC_AD_WAZOO_BAR);

			pWazooBar->SetDefaultWazooConfig(idx); // Fills the bar with Wazoo wnds
		}

		// First pass allows all default config which are fixed [FIXED]
		pos = m_WazooBarList.GetHeadPosition();
		for (idx = 0; pos; idx++)
		{
			CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
			ASSERT_KINDOF(CWazooBar, pWazooBar);

			VERIFY(SetDefaultWazooBarState(pWazooBar, idx, DST_SIZE_FIXED));
		}

		// Now that all fixed positions are done, do relative positions [RELATIVE]
		pos = m_WazooBarList.GetHeadPosition();
		for (idx = 0; pos; idx++)
		{
			CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
			ASSERT_KINDOF(CWazooBar, pWazooBar);

			VERIFY(SetDefaultWazooBarState(pWazooBar, idx, DST_SIZE_RELATIVE));
		}

		// Now go through and show/hide the bars
		pos = m_WazooBarList.GetHeadPosition();
		for (idx = 0; pos; idx++)
		{
			CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
			ASSERT_KINDOF(CWazooBar, pWazooBar);

			VERIFY(SetDefaultWazooBarState(pWazooBar, idx, DST_SHOWHIDE));
		}
	}
	else
	{
		ASSERT(! m_WazooBarList.IsEmpty());
		POSITION pos = m_WazooBarList.GetHeadPosition();
		while (pos)
		{
			CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
			ASSERT_KINDOF(CWazooBar, pWazooBar);

			if (! pWazooBar->LoadWazooConfigFromIni())
			{
				ASSERT(0);
				return FALSE;		// uh, oh...
			}
		}
	}
		
	// ------------------------------------------------------------------

	// We need to make sure all WazooWnds have been assigned to a WazooBar
	typedef CRuntimeClass * CRuntimeClassPtr;

	CRuntimeClassPtr BarArr1[] = { RUNTIME_CLASS(CMboxWazooWnd), RUNTIME_CLASS(CFileBrowseWazooWnd), RUNTIME_CLASS(CStationeryWazooWnd), RUNTIME_CLASS(CSignatureWazooWnd), RUNTIME_CLASS(CPersonalityWazooWnd), NULL };
	CRuntimeClassPtr BarArr2[] = { RUNTIME_CLASS(DirectoryServicesWazooWndNew), RUNTIME_CLASS(CNicknamesWazooWnd), RUNTIME_CLASS(CFiltersWazooWnd), RUNTIME_CLASS(CFilterReportWazooWnd), NULL };
	CRuntimeClassPtr BarArr3[] = { RUNTIME_CLASS(CTaskStatusWazooWnd), RUNTIME_CLASS(CTaskErrorWazooWnd), NULL };

	CWazooBar *pBar = NULL;
	if (pBar = EnsureGroupExists(BarArr1))
		VERIFY(SetDefaultWazooBarState(pBar, 0));

	if (pBar = EnsureGroupExists(BarArr2))
		VERIFY(SetDefaultWazooBarState(pBar, 1));

	if (pBar = EnsureGroupExists(BarArr3))
		VERIFY(SetDefaultWazooBarState(pBar, 2));

	return TRUE;
}

CWazooBar *CWazooBarMgr::FindWazooWnd(CRuntimeClass *pRuntimeClass) const
{
	//
	// Find the Wazoo Bar that contains the given Wazoo window.
	//
	POSITION pos = m_WazooBarList.GetHeadPosition();
	CWazooBar* pWazooBar = NULL;
	
	bool bFound = false;
	while ((!bFound) && (pos))
	{
		pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
		ASSERT_KINDOF(CWazooBar, pWazooBar);

		if (pWazooBar->ContainsWazooWindow(pRuntimeClass))
			bFound = true;
	}

	if (!bFound)
		return (NULL);

	return (pWazooBar);
}

CWazooBar *CWazooBarMgr::EnsureGroupExists(CRuntimeClass **pArr)
{
	ASSERT(pArr);

	if (!pArr)
		return (NULL);

	bool bAllExist = true; // Assume true until we find false

	CRuntimeClass **pGroup = pArr;
	CRuntimeClass *pRTC = NULL;

	while ((bAllExist) && (pRTC = (*pGroup++)))
	{
		if (!FindWazooWnd(pRTC))
			bAllExist = false;
	}
	
	if (bAllExist)
		return (NULL);

	QCWorkbook *pMainFrame = (QCWorkbook *) ::AfxGetMainWnd();
	CWazooBar *pNewWazooBar = CreateNewWazooBar(pMainFrame);
	
	ASSERT(pNewWazooBar);

	pNewWazooBar->SendMessage(WM_COMMAND, ID_SEC_HIDE, 0);

	pGroup = pArr;
	while (pRTC = (*pGroup++))
	{
		if (!FindWazooWnd(pRTC))
			pNewWazooBar->CreateWazooWindow(pRTC);
	}

	return (pNewWazooBar);
}


////////////////////////////////////////////////////////////////////////
// SaveWazooBarConfigToIni [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CWazooBarMgr::SaveWazooBarConfigToIni() const
{
	//
	// First things first.  Flush the entire [WazooBars] section.
	//
	::WritePrivateProfileString(CRString(IDS_INI_WAZOOBAR_SECTION_NAME), NULL, NULL, INIPath);

	//
	// Write the "WazooBarIds=200,201..." string to the [WazooBars] section.
	//
	SaveWazooBarIdsToIni();

	//
	// Write the "WazooBar200=CMboxWazooWnd,CStationeryWazooWnd,..."
	// strings to the [WazooBars] section.
	//
	ASSERT(! m_WazooBarList.IsEmpty());
	POSITION pos = m_WazooBarList.GetHeadPosition();
	while (pos)
	{
		CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
		ASSERT_KINDOF(CWazooBar, pWazooBar);

		pWazooBar->SaveWazooConfigToIni();
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// LoadWazooBarIdsFromIni [protected]
//
////////////////////////////////////////////////////////////////////////
void CWazooBarMgr::LoadWazooBarIdsFromIni(CDWordArray& dwArrayBarIds) const
{
	ASSERT(dwArrayBarIds.GetUpperBound() == -1);

	//
	// Fetch the INI entry...
	//
	CString strIds;
	::GetPrivateProfileString(CRString(IDS_INI_WAZOOBAR_SECTION_NAME), 
								CRString(IDS_INI_WAZOOBAR_IDS), 
								"",
								strIds.GetBuffer(255),
								256, 
								INIPath);
	strIds.ReleaseBuffer();
	if (strIds.IsEmpty())
		return;

	//
	// The INI string is of the form "XXX,XXX,XXX" where XXX is a 
	// decimal number.  So, let's walk the string and parse out the
	// numbers into the caller-provided array.
	//
	DWORD dwId;
	while (GetNextBarId(strIds, dwId))
	{
		ASSERT((dwId >= IDC_WAZOOBAR_BASE) && (dwId < IDC_WAZOOBAR_BASE + MAX_WAZOOS));
		dwArrayBarIds.Add(dwId);
	}
}


////////////////////////////////////////////////////////////////////////
// SaveWazooBarIdsToIni [protected]
//
////////////////////////////////////////////////////////////////////////
void CWazooBarMgr::SaveWazooBarIdsToIni() const
{
	//
	// The CMainFrame::SaveBarState() routine saves control bar IDs
	// to the INI file.  Therefore, we need to save the control
	// bar IDs for the Wazoo Bars for CMainFrame::LoadBarState() to
	// properly do its thing.
	//
	CString strIds;
	ASSERT(! m_WazooBarList.IsEmpty());
	POSITION pos = m_WazooBarList.GetHeadPosition();
	while (pos)
	{
		CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
		ASSERT_KINDOF(CWazooBar, pWazooBar);

		// Grab control bar ID
		const int nBarId = pWazooBar->GetDlgCtrlID();

		if (nBarId == IDC_AD_WAZOO_BAR)
			continue;

		ASSERT((nBarId >= IDC_WAZOOBAR_BASE) && (nBarId < IDC_WAZOOBAR_BASE + MAX_WAZOOS));

		// Convert it to a string
		CString strId;
		strId.Format("%d", nBarId);

		if (! strIds.IsEmpty())
			strIds += ",";
		strIds += strId;
	}

	//
	// Write the INI entry...
	//
	::WritePrivateProfileString(CRString(IDS_INI_WAZOOBAR_SECTION_NAME), 
								CRString(IDS_INI_WAZOOBAR_IDS), 
								strIds, 
								INIPath);
}


////////////////////////////////////////////////////////////////////////
// ActivateWazooWindow [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CWazooBarMgr::ActivateWazooWindow
(
	CMDIFrameWnd* pMainFrame, 
	CRuntimeClass* pRuntimeClass
)
{
	ASSERT(pMainFrame);
	ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);

	//
	// Find the Wazoo Bar that contains the given Wazoo window.
	//
	POSITION pos = m_WazooBarList.GetHeadPosition();
	CWazooBar* pWazooBar = FindWazooWnd(pRuntimeClass);
	
//	bool bFound = false;
//	while ((!bFound) && (pos))
//	{
//		pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
//		ASSERT_KINDOF(CWazooBar, pWazooBar);
//
//		if (pWazooBar->ContainsWazooWindow(pRuntimeClass))
//			bFound = true;
//	}

	if (NULL == pWazooBar)
	{
		ASSERT(0);
		return FALSE;
	}

	BOOL bShouldMax = FALSE;
	if (pWazooBar->IsMDIChild())
	{
		//
		// Activate the MDI child window containing the tree control.
		//
		SECWorksheet* p_childframe = (SECWorksheet *) pWazooBar->GetParentFrame();
		ASSERT(p_childframe);
		ASSERT_KINDOF(SECWorksheet, p_childframe);

		// If this window is being shown and it's the first MDI window to be opened up,
		// and the last MDI window that was open was maximized, then maximize this one.
		if (pWazooBar->IsWindowVisible() == FALSE && GetIniShort(IDS_INI_LAST_WINDOW_MAX))
		{
			BOOL bIsMax = FALSE;
			CMDIChildWnd* Win = pMainFrame->MDIGetActive(&bIsMax);
			if (!Win || Win->IsWindowVisible() == FALSE)
				bShouldMax = TRUE;
		}

		pMainFrame->MDIActivate(p_childframe);
	}

	if (! pWazooBar->IsVisible())
	{
		// Not visible, so make it visible.
		pMainFrame->ShowControlBar(pWazooBar, TRUE, FALSE);
		pMainFrame->RecalcLayout();
		ASSERT_KINDOF(QCWorkbook, pMainFrame);
		((QCWorkbook *) pMainFrame)->ResetTaskBar();
	}

	if (pWazooBar->IsMDIChild())
	{
		//
		// Make sure we "restore" the newly-activated window...
		// Curiously, we only need to do this if the MDI child
		// is not already maximized.
		//
		BOOL bIsMaximized = FALSE;
		SECWorksheet* p_childframe = (SECWorksheet *) pWazooBar->GetParentFrame();
		CMDIChildWnd* pActiveMDIChild = pMainFrame->MDIGetActive(&bIsMaximized);
		ASSERT(p_childframe == pActiveMDIChild);
		if (bShouldMax)
			pMainFrame->MDIMaximize(p_childframe);
		else if (pActiveMDIChild && !bIsMaximized)
			pMainFrame->MDIRestore(p_childframe);
	}

	// Activate the Wazoo window
	pWazooBar->ActivateWazooWindow(pRuntimeClass);

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// GetWazooBarWithFocus [public]
//
// Returns a pointer to the Wazoo bar, if any, that has the keyboard
// focus.
////////////////////////////////////////////////////////////////////////
CWazooBar* CWazooBarMgr::GetWazooBarWithFocus() const
{
	CWnd* pFocusWnd = CWnd::GetFocus();

	if (NULL == pFocusWnd)
		return NULL;			// nobody has focus

	ASSERT(! m_WazooBarList.IsEmpty());
	POSITION pos = m_WazooBarList.GetHeadPosition();
	while (pos)
	{
		CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
		ASSERT_KINDOF(CWazooBar, pWazooBar);

		if (pWazooBar->IsChild(pFocusWnd))
			return pWazooBar;
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// SetActiveWazooWindows [public]
//
// To support Just In Time wazoo display initialization, we must
// postpone the activation of the wazoo window in each wazoo container
// until after the main window is created.
////////////////////////////////////////////////////////////////////////
void CWazooBarMgr::SetActiveWazooWindows()
{
	ASSERT(! m_WazooBarList.IsEmpty());
	POSITION pos = m_WazooBarList.GetHeadPosition();
	while (pos)
	{
		CWazooBar* pWazooBar = (CWazooBar *) m_WazooBarList.GetNext(pos);
		ASSERT_KINDOF(CWazooBar, pWazooBar);

		pWazooBar->SetActiveWazooWindow();
	}
}


////////////////////////////////////////////////////////////////////////
// GetNextBarId [private]
//
// Private helper function for processing parameter strings containing
// DWORD values.  The idea is to remove the next parameter from the
// parameter list, then convert it to a DWORD, returning TRUE if
// everything went okay.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBarMgr::GetNextBarId(CString& strIds, DWORD& dwId) const
{
	if (strIds.IsEmpty())
		return FALSE;			// can't get juice from a turnip

	CString strId;

	int idx = -1;
	if ((idx = strIds.Find(',')) != -1)
	{
		//
		// Strip next parameter from the front of the paramList.
		//
		strId = strIds.Left(idx);
		strIds = strIds.Right(strIds.GetLength() - idx - 1);
	}
	else
	{
		strId = strIds;
		strIds.Empty();
	}

	if (strId.IsEmpty())
	{
		ASSERT(0);
		return FALSE;			// huh?  empty parameter?
	}

	char* p_stop_ch = NULL;
	dwId = ::strtoul(strId, &p_stop_ch, 0);
	if ((NULL == p_stop_ch) || (*p_stop_ch != '\0'))
		return FALSE;			// malformed integer

	return TRUE;
}
