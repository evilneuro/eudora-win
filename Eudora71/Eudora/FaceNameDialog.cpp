// FaceNameDialog.cpp : implementation file
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
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

#include "dlgs.h"
#include "FaceNameDialog.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CFaceNameDialog

IMPLEMENT_DYNAMIC(CFaceNameDialog, CFontDialog)

CFaceNameDialog::CFaceNameDialog(LPLOGFONT lplfInitial, DWORD dwFlags, CDC* pdcPrinter, CWnd* pParentWnd) : 
	CFontDialog(lplfInitial, dwFlags, pdcPrinter, pParentWnd)
{
}


BEGIN_MESSAGE_MAP(CFaceNameDialog, CFontDialog)
	//{{AFX_MSG_MAP(CFaceNameDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CFaceNameDialog::OnInitDialog() 
{
	CFontDialog::OnInitDialog();
	
	CFontDialog::OnInitDialog();
	
	// hide the sizes static control
	ASSERT( GetDlgItem( stc3 ) );
	GetDlgItem( stc3 )->ShowWindow( SW_HIDE );

	// hide the sizes combo box
	ASSERT( GetDlgItem( cmb3 ) );
	GetDlgItem( cmb3 )->ShowWindow( SW_HIDE );

	// hide the script static control
	ASSERT( GetDlgItem( stc7 ) );
	GetDlgItem( stc7 )->ShowWindow( SW_HIDE );

	// hide the script combo box
	ASSERT( GetDlgItem( cmb5 ) );
	GetDlgItem( cmb5 )->ShowWindow( SW_HIDE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
