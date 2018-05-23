// QCCustomizePersPage.cpp : implementation file
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
#include "QCCustomizePersPage.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPersonalityCommand.h"
#include "QCPersonalityDirector.h"
#include "rs.h"

#include "DebugNewHelpers.h"


extern QCCommandStack			g_theCommandStack;
extern QCPersonalityDirector	g_thePersonalityDirector;

static UINT BASED_CODE s_PersonalityButtons[] =
{
	ID_CHANGE_PERSONA
};

/////////////////////////////////////////////////////////////////////////////
// QCCustomizePersonalitiesPage property page

IMPLEMENT_DYNCREATE(QCCustomizePersonalitiesPage, QCToolBarCmdPage)

QCCustomizePersonalitiesPage::QCCustomizePersonalitiesPage() : QCToolBarCmdPage(QCCustomizePersonalitiesPage::IDD, IDR_PERSONALITIES )
{
	//{{AFX_DATA_INIT(QCCustomizePersonalitiesPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

QCCustomizePersonalitiesPage::~QCCustomizePersonalitiesPage()
{
}

void QCCustomizePersonalitiesPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCCustomizePersonalitiesPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


UINT QCCustomizePersonalitiesPage::TranslateID(UINT uID)
{
	COMMAND_ACTION_TYPE theAction;

	switch (uID)
	{
	case ID_CHANGE_PERSONA:	theAction = CA_CHANGE_PERSONA;	break;
	default:
		return uID;
	}

	CTreeCtrl* pTree = (CTreeCtrl*)GetDlgItem(IDC_TREE_CTRL);
	if (!pTree)
	{
		ASSERT(0);
		return 0;
	}

	HTREEITEM hItem = pTree->GetSelectedItem();
	if (!hItem)
	{
		ASSERT( 0 );
		return 0;
	}

	QCPersonalityCommand* pCommand = (QCPersonalityCommand*)pTree->GetItemData(hItem);
	if (!pCommand)
	{
		ASSERT(0);
		return 0;
	}

	UINT uNewID = g_theCommandStack.FindOrAddCommandID(pCommand, theAction);

	return uNewID;
}



BEGIN_MESSAGE_MAP(QCCustomizePersonalitiesPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCCustomizePersonalitiesPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCCustomizePersonalitiesPage message handlers

BOOL QCCustomizePersonalitiesPage::OnInitDialog() 
{
	CStatic*				pLabel;
	CRString				szTitle( IDR_PERSONALITIES );
	CTreeCtrl*				pTree;
	POSITION				pos;
	QCPersonalityCommand*	pCommand;
	HTREEITEM				hItem;

	pLabel = ( CStatic* ) GetDlgItem( IDC_TREE_ITEMS_STATIC );
	
	if( pLabel != NULL )
	{
		pLabel->SetWindowText( 	szTitle );	
	}
	
	pos = g_thePersonalityDirector.GetFirstPersonalityPosition();

	if( pos == NULL )
	{
		// there are no personalities
		return QCToolBarCmdPage::OnInitDialog();
	}
		
	DefineBtnGroup(_T( szTitle), NUMELEMENTS(s_PersonalityButtons), s_PersonalityButtons);

	QCToolBarCmdPage::OnInitDialog();
	
	pTree = ( CTreeCtrl* ) GetDlgItem( IDC_TREE_CTRL );
	
	if( pTree == NULL )
	{
		ASSERT( 0 );
		return TRUE;
	}

	//
	// Add the bitmap images for tree control.
	//
	
	if (! m_ImageList.Load() )
	{
		return TRUE;		// mangled resources?
	}

	pTree->SetImageList( &m_ImageList, TVSIL_NORMAL );
	
	//
	// Set "thin" font for old Win 3.x GUI.
	//
	if (! IsVersion4() )
	{
		pTree->SetFont(CFont::FromHandle(HFONT(::GetStockObject(ANSI_VAR_FONT))));
	}

	pos = g_thePersonalityDirector.GetFirstPersonalityPosition();

	while( pos != NULL )
	{
		pCommand = g_thePersonalityDirector.GetNextPersonality( pos );
		hItem = pTree->InsertItem( pCommand->GetName() , QCMailboxImageList::IMAGE_PERSONALITY, QCMailboxImageList::IMAGE_PERSONALITY );
		pTree->SetItemData( hItem, ( DWORD ) pCommand );
	}
	
	pTree->SortChildren( NULL );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
