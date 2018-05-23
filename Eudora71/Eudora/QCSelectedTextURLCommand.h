// QCSelectedTextURLCommand.h: interface for the QCSelectedTextURLCommand class.
//
// Copyright (c) 1997-2002 by QUALCOMM, Incorporated
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
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCSELECTEDTEXTURLCOMMAND_H__3068C5A1_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
#define AFX_QCSELECTEDTEXTURLCOMMAND_H__3068C5A1_AB8E_11D0_97B6_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandObject.h"

extern enum COMMAND_ACTION_TYPE;

class QCSelectedTextURLCommand : public QCCommandObject
{
public:
	QCSelectedTextURLCommand(const char* URLFormat);
	virtual ~QCSelectedTextURLCommand();

public:
	DECLARE_DYNAMIC(QCSelectedTextURLCommand)

	virtual void	Execute(COMMAND_ACTION_TYPE theAction, void* pData = NULL);
	virtual UINT	GetFlyByID(COMMAND_ACTION_TYPE theAction);
	virtual CString	GetToolTip(COMMAND_ACTION_TYPE theAction);

protected:
	CString	m_URLFormat;
};

#endif // !defined(AFX_QCSELECTEDTEXTURLCOMMAND_H__3068C5A1_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
