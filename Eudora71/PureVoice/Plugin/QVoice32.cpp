/*
 *  Filename: QVoice32.cpp
 *
 *  Last Edited: Thursday, December 12, 1996
 *
 *  Author: lsong (original sample code, Lower-Upper, from Scott Manjourides)
 *
 *  Copyright 1995, 1996 QUALCOMM Inc.
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
 */

#include "stdafx.h"
#include "QVoice32.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// The initialization routines for the DLL.

BOOL CQVoice32DLL::InitInstance()
{
	// any DLL initialization goes here
	TRACE("Qualcomm PureVoice Plug-In for Eudora DLL initializing\n");
	SetDialogBkColor( COLORREF(GetSysColor(COLOR_3DFACE)),
			COLORREF(GetSysColor(COLOR_BTNTEXT)) ); // lsong

	return TRUE;
}

int CQVoice32DLL::ExitInstance()
{
	// any DLL termination goes here (WEP-like code)
	return CWinApp::ExitInstance();
}

// lsong
void CQVoice32DLL::YieldEvents()
{
  MSG msg;

  if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) // check for messages
    { 
    if ( !PumpMessage() ) // if FALSE app has terminated
      { 
      //m_bProcessActive = FALSE;
      ::PostQuitMessage(0); 
      } 
    } 
  // let MFC do its idle processing
  LONG lIdle = 0;
  while ( OnIdle(lIdle++ ));
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CQVoice32DLL object

CQVoice32DLL theDLL( APP_TITLE );
