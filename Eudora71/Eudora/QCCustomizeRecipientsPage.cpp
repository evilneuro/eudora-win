// QCCustomizeRecipientsPage.cpp : implementation file
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
#include "QCCustomizeRecipientsPage.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"
#include "rs.h"

#include "DebugNewHelpers.h"


extern QCCommandStack		g_theCommandStack;
extern QCRecipientDirector	g_theRecipientDirector;

static UINT BASED_CODE theRecipientButtons[] =
{
	ID_RECIPIENT_NEW_MESSAGE,
	ID_RECIPIENT_FORWARD,
	ID_RECIPIENT_REDIRECT,
	ID_RECIPIENT_INSERT
};

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeRecipientsPage property page

IMPLEMENT_DYNCREATE(QCCustomizeRecipientsPage, QCToolBarCmdPage)

QCCustomizeRecipientsPage::QCCustomizeRecipientsPage() : QCToolBarCmdPage(QCCustomizeRecipientsPage::IDD, IDS_RECIPIENTS_TITLE )
{
	//{{AFX_DATA_INIT(QCCustomizeRecipientsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

QCCustomizeRecipientsPage::~QCCustomizeRecipientsPage()
{
}

void QCCustomizeRecipientsPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCCustomizeRecipientsPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


UINT QCCustomizeRecipientsPage::TranslateID(UINT uID)
{
	COMMAND_ACTION_TYPE theAction;

	switch (uID)
	{
	case ID_RECIPIENT_NEW_MESSAGE:	theAction = CA_NEW_MESSAGE_TO;	break;
	case ID_RECIPIENT_FORWARD:		theAction = CA_FORWARD_TO;		break;
	case ID_RECIPIENT_REDIRECT:		theAction = CA_REDIRECT_TO;		break;
	case ID_RECIPIENT_INSERT:		theAction = CA_INSERT_RECIPIENT;break;
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

	QCRecipientCommand* pCommand = (QCRecipientCommand*)pTree ->GetItemData(hItem);
	if (!pCommand)
	{
		ASSERT(0);
		return 0;
	}

	UINT uNewID = g_theCommandStack.FindOrAddCommandID(pCommand, theAction);

	return uNewID;
}



BEGIN_MESSAGE_MAP(QCCustomizeRecipientsPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCCustomizeRecipientsPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeRecipientsPage message handlers

BOOL QCCustomizeRecipientsPage::OnInitDialog() 
{
	CStatic*			pLabel;
	CRString			szTitle( IDS_RECIPIENTS_TITLE );
	CTreeCtrl*			pTree;
	POSITION			pos;
	QCRecipientCommand*	pCommand;
	HTREEITEM			hItem;

	pLabel = ( CStatic* ) GetDlgItem( IDC_TREE_ITEMS_STATIC );
	
	if( pLabel != NULL )
	{
		pLabel->SetWindowText( 	szTitle );	
	}
	
	pos = g_theRecipientDirector.GetFirstRecipientPosition();

	if( pos == NULL )
	{
		// there are no recipients
		return QCToolBarCmdPage::OnInitDialog();
	}
		
	DefineBtnGroup(_T( szTitle), NUMELEMENTS(theRecipientButtons), theRecipientButtons);

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

	pos = g_theRecipientDirector.GetFirstRecipientPosition();

	while( pos != NULL )
	{
		pCommand = g_theRecipientDirector.GetNextRecipient( pos );
		hItem = pTree->InsertItem( pCommand->GetName() , QCMailboxImageList::IMAGE_RECIPIENT, QCMailboxImageList::IMAGE_RECIPIENT );
		pTree->SetItemData( hItem, ( DWORD ) pCommand );
	}
	
	
	pTree->SortChildren( NULL );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
