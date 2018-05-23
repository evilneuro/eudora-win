// QCCustomizeSignaturesPage.cpp : implementation file
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
#include "QCCustomizeSignaturesPage.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCSignatureCommand.h"
#include "QCSignatureDirector.h"
#include "rs.h"

#include "DebugNewHelpers.h"

extern QCCommandStack		g_theCommandStack;
extern QCSignatureDirector	g_theSignatureDirector;

static UINT BASED_CODE theSignatureButtons[] =
{
	ID_WILDCARD
};

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeSignaturesPage property page

IMPLEMENT_DYNCREATE(QCCustomizeSignaturesPage, QCToolBarCmdPage)

QCCustomizeSignaturesPage::QCCustomizeSignaturesPage() : QCToolBarCmdPage(QCCustomizeSignaturesPage::IDD, IDS_SIGNATURES )
{
	//{{AFX_DATA_INIT(QCCustomizeSignaturesPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

QCCustomizeSignaturesPage::~QCCustomizeSignaturesPage()
{
}

void QCCustomizeSignaturesPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCCustomizeSignaturesPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


UINT QCCustomizeSignaturesPage::TranslateID(UINT uID)
{
	COMMAND_ACTION_TYPE theAction;

	switch (uID)
	{
	case ID_WILDCARD:	theAction = CA_OPEN;	break;
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

	QCSignatureCommand* pCommand = (QCSignatureCommand*)pTree->GetItemData(hItem);
	if (!pCommand)
	{
		ASSERT(0);
		return 0;
	}

	UINT uNewID = g_theCommandStack.FindOrAddCommandID(pCommand, theAction);

	return uNewID;
}


BEGIN_MESSAGE_MAP(QCCustomizeSignaturesPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCCustomizeSignaturesPage)
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(wmSECNotifyMsg, OnBarNotify)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeSignaturesPage message handlers

BOOL QCCustomizeSignaturesPage::OnInitDialog() 
{	
	CStatic*			pLabel;
	CRString			szTitle( IDS_SIGNATURES );
	CTreeCtrl*			pTree;
	POSITION			pos;
	QCSignatureCommand*	pCommand;
	HTREEITEM			hItem;

	pLabel = ( CStatic* ) GetDlgItem( IDC_TREE_ITEMS_STATIC );
	
	if( pLabel != NULL )
	{
		pLabel->SetWindowText( 	szTitle );	
	}
	
	pos = g_theSignatureDirector.GetFirstSignaturePosition();

	if( pos == NULL )
	{
		// there are no signatures
		return QCToolBarCmdPage::OnInitDialog();
	}
		
	DefineBtnGroup( _T( szTitle), NUMELEMENTS(theSignatureButtons), theSignatureButtons );

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

	while( pos != NULL )
	{
		pCommand = g_theSignatureDirector.GetNext( pos );
		hItem = pTree->InsertItem( pCommand->GetName() , QCMailboxImageList::IMAGE_SIGNATURE, QCMailboxImageList::IMAGE_SIGNATURE );
		pTree->SetItemData( hItem, ( DWORD ) pCommand );
	}
	
	
	pTree->SortChildren( NULL );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


LONG QCCustomizeSignaturesPage::OnBarNotify(UINT nID, LONG lParam)
{
	
	// A toolbar is notifying us of an event.
	if(m_pManager == NULL)
		return 0L;

	SECCustomToolBar* pBar = GetControlBar(nID);
	if(pBar == NULL)
		return 0L;

	switch(lParam)
	{
		case SECCustomToolBar::BarDesChange:
		{
			// The selected button has changed - so need to changed the
			// description text.
			if( pBar->GetCurBtn() != -1 )
			{
				CRString szDesc( IDS_EDIT_SIGNATURE_DESCRIPTION );
				GetDlgItem(IDC_TOOLBAR_DESCRIPTION)->SetWindowText( szDesc );
			}
			else
			{
				GetDlgItem(IDC_TOOLBAR_DESCRIPTION)->SetWindowText( "" );
			}
			
			return 0;
		}

		default:
			break;
	}

	return SECToolBarCmdPage::OnBarNotify( nID, lParam );
}

