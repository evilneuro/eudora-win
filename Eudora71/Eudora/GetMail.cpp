////////////////////////////////////////////////////////////////////////
// File: GetMail.cpp
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <mmsystem.h>
#include <vfw.h>

#include "resource.h"

#include "QCUtils.h"

#include "pop.h"
#include "compmsgd.h"

#include "POPSession.h"


#include "mainfrm.h"
#include "guiutils.h"
#include "rs.h"
#include "tocdoc.h"


#include "utils.h"    //QCFindResourcehandle
#include "persona.h"  //g_Personalities

#include "Automation.h"


#include "QCTaskManager.h"   //for QueueWorkerThread
#include "QCSharewareManager.h"


#ifdef IMAP4
#include "ImapMailMgr.h"		// Just for integration with IMAP's checking mail procedure.
#endif // IMAP4



//	Defines and other Constants	=============================
#include "DebugNewHelpers.h"


extern CString EudoraDir;

static void PlaySound( void );







//	================================================================================
// GetMail
// This is a stub entry routine for the Mail Service Object.
//
void 
GetMail(unsigned long bitflags, LPSTR lpCheckMailList /*=NULL*/, BOOL bFullMailCheck /*=TRUE*/)
{
	if (GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{
		int res = AlertDialog(IDD_ERR_OFFLINE);

		if (res == IDC_ONLINE_BUTTON)
			SetIniShort(IDS_INI_CONNECT_OFFLINE, 0);
		else if (res == IDCANCEL)
			return;
	}

	// Save open windows before we check mail
	((CMainFrame*)AfxGetMainWnd())->SaveOpenWindows(FALSE);

 
	BOOL bCheckedAPopAccount   = FALSE;

	// sanity check - nobody else creates a global pop object, right?
	if (gPOP != NULL)
		ASSERT(FALSE);

	CString	homie =	g_Personalities.GetCurrent();
	CRString Dominant( IDS_DOMINANT );

	// get all the alternate personalities
	LPSTR lpPersonalities;
	if ( lpCheckMailList )
		lpPersonalities = lpCheckMailList;
	else
		lpPersonalities = g_Personalities.CheckMailList();

	//count the # of ALL checkmail personas
	LPSTR lpPersona = lpPersonalities;
	CString tPersona = lpPersona;
	int nPersonas = 0;
	while ( !tPersona.IsEmpty() )
	{
		nPersonas++;

		// advance to next personality
		lpPersona += strlen( lpPersona ) + 1;
		tPersona = lpPersona;
	}

	bool bIgnoreIdle = false;
	bool bIgnoreIdleOnManulaCheck = GetIniLong(IDS_INI_IGNORE_IDLE_ON_MANUAL_CHECK)?true:false;
	
	if(bIgnoreIdleOnManulaCheck && bitflags == kManualMailCheckBits)
		bIgnoreIdle = true;
	
	bool bChecking = 0 != ( bitflags & kNeedMailCheckBits );
	
	//All the tasks started here will be taken as a group
	QCTaskGroup group(NULL, bIgnoreIdle);
	
	QCPOPNotifier *pPOPNotifier = NULL;

	CString Persona = lpPersonalities;

	if (!Persona.IsEmpty())
	{
		// Instead of defaulting to the number of personalities being checked (which was non-optimal
		// and was inadequate for IMAP multiple mailbox syncing) we default to 1 (which we decrement
		// after the below loop) and we allow the POP or IMAP connection to increment the notifier
		// appropriately.
		pPOPNotifier = DEBUG_NEW_NOTHROW QCPOPNotifier(1);
	}

	while ( ! Persona.IsEmpty() )
	{
		if ( Persona == Dominant )	// "<Dominant>" is just a placeholder
			Persona = "";

		if (Persona.IsEmpty() || UsingFullFeatureSet())
		{
			g_Personalities.SetCurrent( Persona );
			
			// decide if we're sending here, because we might not be checking
			// I think this is a weird way to check these bits, but it seemed
			// to be working for Joel, so...
			bool bSendOnCheck = GetIniShort(IDS_INI_SEND_ON_CHECK)?true:false;
			bool bSending = ((bitflags & kSendMailBit) || 
				(bChecking && (bitflags & kSendIfSendOnCheckBit) && bSendOnCheck))?true:false;

			if ( bChecking )
			{
				// If this is NOT a POP personality, don't check mail!!
				// JOK - 9/19/97.
				if (! GetIniShort (IDS_INI_USES_IMAP) )
				{
					
					QCPOPSettings *settings = DEBUG_NEW QCPOPSettings(Persona, bitflags);
					CPOPSession *popThread = DEBUG_NEW CPOPSession(settings, pPOPNotifier);
								
					BOOL bResult = TRUE;
					if (settings->NeedPOP())
						bResult = popThread->DoGetPassword_();
	
					if(bResult)
					{
						// For users who don't quit Eudora very often, we sometimes
						// want to trim junk on a mail check.  Let TrimJunk()
						// decide if now is the appropriate time.
						TrimJunk(false/*bUserRequested*/, true/*bOnMailCheck*/);

						// Queue the mail check thread.
						if( QCGetTaskManager()->QueueWorkerThread(popThread) == false)
						{
							//cleanup as task can't be scheduled to run..
							
							//Don't need taskinfo anymore so nuke it
							delete (popThread->GetTaskInfo());
							delete popThread;  //decrements the notifier
						}
					}
					else
					{
						//we are not checking mail, so decrement the count
						//pPOPNotifier->Decrement();
						delete (popThread->GetTaskInfo());
						delete popThread;  //decrements the notifier
					}
	
								
#ifdef IMAP4
					// Send on check would normally be handled whan a POP account
					// is checkec for new mail. If no pop account, we need to
					// handle it below:
					//
					bCheckedAPopAccount = TRUE;
#endif // IMAP4
				
				}
				else
				{
					// Start the check for mail from the IMAP account. Do it in the
					// background from here.
					//
					GetImapMailMgr()->ImapGetMail(Persona,
												  bitflags,
												  pPOPNotifier,
												  TRUE/*bInBackground*/,
												  bFullMailCheck);
				}
			}

			//do we need to schedule any sending threads??? SendOnCheckMail
			if(bSending)
				SendMail(Persona);
		}

		// advance to next personality
		lpPersonalities += strlen( lpPersonalities ) + 1;
		Persona = lpPersonalities;
	}

	if (pPOPNotifier)
	{
		// Decrement to account for default ref count of 1.
		pPOPNotifier->Decrement();
	}

	g_Personalities.SetCurrent( homie );
}



void NotifyMailCheckResults(int NotifyFlags)
{
	if( (NotifyFlags & CMF_RECEIVED_MAIL) || (NotifyFlags & CMF_DELETED_MAIL))
		CTocDoc::UpdateAllServerStatus();

	if( NotifyFlags & CMF_PLAY_SOUND)   //if ( s_bPlaySound )
		PlaySound();
	
	if (GetIniShort(IDS_INI_ALERT))
	{
		// Show dialog with results of mail check
		// No dialog is displayed if called from Automation
		if (!gbAutomationCheckMail)
		{
			CheckForAutoOk( TRUE );
			if( NotifyFlags & CMF_ALERT_NEWMAIL) //if ( s_bAlertNewMail )
				AlertDialog(IDD_NEW_MAIL);

			if( (NotifyFlags & CMF_NOTIFY_NOMAIL) && !(NotifyFlags & CMF_RECEIVED_MAIL) )
				AlertDialog(IDD_NO_NEW_MAIL);
			CheckForAutoOk( FALSE );
		}
	}
}



////////////////////////////////////////////////////////////////////////
// PlaySound [static]
//
////////////////////////////////////////////////////////////////////////
void PlaySound( void )
{
	BOOL bPlayedSound = FALSE;

	{
		//
		// First try.  Load up the media file pathname from the user
		// INI setting.
		//
		const char* pszSoundFile = GetIniString(IDS_INI_NEW_MAIL_SOUND);
		if (*pszSoundFile)
		{
			char szPathname[_MAX_PATH + 1];
			if (strchr(pszSoundFile, SLASH))
				strcpy(szPathname, pszSoundFile);
			else
				wsprintf(szPathname, "%s%s", (const char*) EudoraDir, pszSoundFile);

			if (::FileExistsMT(szPathname))
			{
				static HWND hNewMailSound = NULL;

				if (hNewMailSound)
				{
					MCIWndStop(hNewMailSound);
					MCIWndDestroy(hNewMailSound);
				}
				hNewMailSound = AsyncPlayMedia(szPathname);
				if (hNewMailSound)
					bPlayedSound = TRUE;
			}
		}
	}

	if (! bPlayedSound)
	{
		//
		// Second try.  Load up the default Wav file from the resource
		// DLL.
		//
		HINSTANCE hInst = ::QCFindResourceHandle( MAKEINTRESOURCE(IDW_NEW_MAIL_WAVE), "WAVE" );
		bPlayedSound = PlaySound(MAKEINTRESOURCE(IDW_NEW_MAIL_WAVE), hInst, SND_ASYNC | SND_NODEFAULT | SND_RESOURCE);
	}

	if (! bPlayedSound)
	{
		//
		// Third try.  Just sound a default beep.
		//
		::MessageBeep(MB_ICONINFORMATION);
	}
}
