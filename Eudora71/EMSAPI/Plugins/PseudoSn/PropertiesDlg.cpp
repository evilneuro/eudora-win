/*
 *  Filename: PropertiesDlg.cpp
 *  Original Author: Scott Manjourides
 *
 *  Copyright 1995-2003 QUALCOMM, Incorporated
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


 *
 *  Send comments and questions to <eudora-emsapi@qualcomm.com>
 */

#include "stdafx.h"
#include "resource.h"
#include "PropertiesDlg.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertiesDlg dialog


CPropertiesDlg::CPropertiesDlg(int te)
	: CDialog(CPropertiesDlg::IDD, NULL),
	m_te(te)
{
	//{{AFX_DATA_INIT(CPropertiesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertiesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(CPropertiesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertiesDlg message handlers

BOOL CPropertiesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetupRadioButtons();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPropertiesDlg::OnOK()
{
	SaveCheckRadioBtn();
	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CPropertiesDlg mmember functions

// PUBLIC
int CPropertiesDlg::GetChoice()
{
	return (m_te);
}

// PROTECTED
void CPropertiesDlg::SaveCheckRadioBtn()
{
	int nIDCheckedButton = 0;

	nIDCheckedButton = GetCheckedRadioButton(IDC_RADIO_BASE64, IDC_RADIO_NONE);

	switch (nIDCheckedButton)
	{
		case IDC_RADIO_BASE64:
			m_te = te_base64;
		break;
		
		case IDC_RADIO_QP:
			m_te = te_qp;
		break;
		
		case IDC_RADIO_NONE:
		default:
			m_te = te_none;
		break;
	}
}

// PROTECTED
void CPropertiesDlg::SetupRadioButtons()
{
	// Setup the radio buttons to reflect the m_te setting
	int nIDCheckButton = 0;

	switch (m_te)
	{
		case te_base64:
			nIDCheckButton = IDC_RADIO_BASE64;
		break;
		
		case te_qp:
			nIDCheckButton = IDC_RADIO_QP;
		break;
		
		case te_none:
		default:
		{
			m_te = te_none;
			nIDCheckButton = IDC_RADIO_NONE;
		}
		break;
	}

	CheckRadioButton(IDC_RADIO_BASE64, IDC_RADIO_NONE, nIDCheckButton);
}
