// QCCustomizePluginsPage.cpp : implementation file
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include "eudora.h"
#include "QCCustomizePluginsPage.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "rs.h"

#include "DebugNewHelpers.h"

extern QCCommandStack	g_theCommandStack;
extern QCPluginDirector	g_thePluginDirector;


/////////////////////////////////////////////////////////////////////////////
// QCCustomizePluginsPage property page

IMPLEMENT_DYNCREATE(QCCustomizePluginsPage, QCToolBarCmdPage)

QCCustomizePluginsPage::QCCustomizePluginsPage() : QCToolBarCmdPage(QCCustomizePluginsPage::IDD, IDS_PLUGINS_TITLE )
{
	//{{AFX_DATA_INIT(QCCustomizePluginsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_IDs = NULL;
}

QCCustomizePluginsPage::~QCCustomizePluginsPage()
{
	delete [] m_IDs;
}


void QCCustomizePluginsPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCCustomizePluginsPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(QCCustomizePluginsPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCCustomizePluginsPage)
	//}}AFX_MSG_MAP
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREE_CTRL, OnSelChanged )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCCustomizePluginsPage message handlers

BOOL QCCustomizePluginsPage::OnInitDialog() 
{
	CStatic* pLabel = (CStatic*)GetDlgItem(IDC_TREE_ITEMS_STATIC);
	
	if (pLabel)
		pLabel->SetWindowText(CRString(IDS_PLUGINS_TITLE));

	m_nCurSel = LB_ERR;

	POSITION pos = g_thePluginDirector.GetFirstPluginPosition();
	if (!pos) 
	{
		// there are no plugins
		return QCToolBarCmdPage::OnInitDialog();
	}

	m_IDs = DEBUG_NEW_NOTHROW UINT[g_thePluginDirector.GetCount()];
	
	if (!m_IDs)
		return FALSE;
	
	CTreeCtrl* pTree = (CTreeCtrl*)GetDlgItem(IDC_TREE_CTRL);
	
	if (!pTree)
	{
		ASSERT(0);
		return TRUE;
	}

	pTree->SetImageList(&g_thePluginDirector.m_ImageList, TVSIL_NORMAL);
	
	//
	// Set "thin" font for old Win 3.x GUI.
	//
	if (!IsVersion4())
		pTree->SetFont(CFont::FromHandle(HFONT(::GetStockObject(ANSI_VAR_FONT))));

	INT i = 0;

	while (pos)
	{
		QCPluginCommand* pCommand = g_thePluginDirector.GetNextPlugin(pos);

		if (pCommand->GetType() != CA_TRANSMISSION_PLUGIN)
		{
			m_IDs[i] = g_theCommandStack.FindOrAddCommandID(pCommand, pCommand->GetType());
			if (m_IDs[i])
			{
				DefineBtnGroup(_T(pCommand->GetName()), 1, &(m_IDs[i]));
				i++;
				HTREEITEM hItem = pTree->InsertItem(pCommand->GetName() , pCommand->m_ImageOffset, pCommand->m_ImageOffset);
				pTree->SetItemData(hItem, (DWORD)pCommand);
			}
		}
	}

	QCToolBarCmdPage::OnInitDialog();	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void QCCustomizePluginsPage::SetDescription(int nID)
{
	QCCommandObject*	pObject;
	COMMAND_ACTION_TYPE	theAction;
				
	if( g_theCommandStack.Lookup( (unsigned short)nID, &pObject, &theAction ) == TRUE )
	{
		switch (theAction)
		{
			case CA_ATTACH_PLUGIN:
				nID = ID_MESSAGE_ATTACH_PLUGIN;
				break;
			case CA_SPECIAL_PLUGIN:
				nID = ID_SPECIAL_PLUGIN;
				break;
			case CA_TRANSLATE_PLUGIN:
				nID = ID_TRANSLATOR;
				break;
			default:
				break;
		}
	}
	
	SECToolBarCmdPage::SetDescription(nID);
}

void QCCustomizePluginsPage::OnSelChanged(
NMHDR*		pNMHDR, 
LRESULT*	pResult) 
{
	QCPluginCommand*	pCommand;
	NM_TREEVIEW*		pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	INT					iIndex;
	HTREEITEM			hItem;
	CTreeCtrl*			pTree;
	CString				szName;

	*pResult = 0;

	hItem = pNMTreeView->itemNew.hItem;

	pTree = ( CTreeCtrl* ) GetDlgItem( IDC_TREE_CTRL );
	
	if( pTree == NULL )
	{
		ASSERT( 0 );
		return;
	}

	pCommand = ( QCPluginCommand* ) ( pTree -> GetItemData( hItem ) );

	if( pCommand == NULL )
	{
		ASSERT( 0 );
		return;
	}
		
	
	// The current selection has changed. I need to show the newly selected
	// toolbar
	for( iIndex = 0; iIndex < m_btnGroups.GetSize(); iIndex ++)
	{
		if( pCommand->GetName() == ((SECBtnGroup*) m_btnGroups[iIndex])->lpszTitle )
		{
			break;
		}
	}

	if( iIndex == m_btnGroups.GetSize() )
	{
		ASSERT( 0 );
		return;
	}


	if(m_nCurSel != LB_ERR)
	{
		((SECBtnGroup*) 
			m_btnGroups[m_nCurSel])->pToolBar->EnableConfigMode(FALSE);
		m_nCurSel = LB_ERR;
	}

	for(int i=0; i<m_btnGroups.GetSize(); i++)
	{
		SECCustomizeToolBar* pBar = ((SECBtnGroup*) m_btnGroups[i])->pToolBar;
		if(i == iIndex)
		{
			pBar->EnableConfigMode(TRUE);
			pBar->ShowWindow(SW_SHOW);
			m_nCurSel = i;
		}
		else
			pBar->ShowWindow(SW_HIDE);
	}
}

