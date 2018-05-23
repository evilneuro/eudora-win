////////////////////////////////////////////////////////////////////////
// File: POP.CPP
//
//
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <mmsystem.h>
#include "resource.h"

#include "QCUtils.h"

#include "pop.h"
#include "compmsgd.h"

#include "POPSession.h"


#include "eudora.h"  // SLASH
#include "guiutils.h"
#include "rs.h"
#include "tocdoc.h"


#include "utils.h"    //QCFindResourcehandle
#include "persona.h"  //g_Personalities

#include "Automation.h"


#include "QCTaskManager.h"   //for QueueWorkerThread


#ifdef IMAP4
#include "ImapMailMgr.h"		// Just for integration with IMAP's checking mail procedure.
#endif // IMAP4



//	Defines and other Constants	=============================
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


extern CString EudoraDir;



//	Externals	=============================================

/*

//	Globals	=================================================
JJFile*			g_pPOPInboxFile 		= NULL;
CPOPSession*	gPOP			= NULL;  //FORNOW
CHostList*		gPopHostList	= NULL;


BOOL 	g_bHasAttachment = FALSE;
long 	g_lEstimatedMsgSize;
BOOL	g_bAttachmentDecodeEnabled = TRUE;
long	g_lBadCoding;

*/


BOOL g_bShowProgress;	// shared with Progress.cpp (yuk)

static void PlaySound( void );







//	================================================================================
// GetMail
// This is a stub entry routine for the Mail Service Object.
//
void 
GetMail(unsigned long bitflags, LPSTR lpCheckMailList /* = NULL */ )
{
	if (GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{
		int res = AlertDialog(IDD_ERR_OFFLINE);

		if (res == IDC_ONLINE_BUTTON)
			SetIniShort(IDS_INI_CONNECT_OFFLINE, 0);
		else if (res == IDCANCEL)
			return;
	}
 
	BOOL bDeletedMail = FALSE;
	
	//BOOL CreatedNetConnection = (NetConnection == NULL);

#ifdef IMAP4
	BOOL bCheckedAnImapAccount = FALSE;
	BOOL bCheckedAPopAccount   = FALSE;
#endif // 

	// sanity check - nobody else creates a global pop object, right?
	if (gPOP != NULL)
		ASSERT(FALSE);

	g_bShowProgress = !( bitflags & kSilentCheckBit );
	
	//accumulate the results of GetMail on each personality here
	unsigned int NotifyFlags = 0;


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
	
	//All the tasks started here will be taken as a group
	QCTaskGroup group(NULL, bIgnoreIdle);
	
	QCPOPNotifier *pPOPNotifier = NULL;

	if (nPersonas > 0)
		pPOPNotifier = new QCPOPNotifier(nPersonas);

	CString Persona = lpPersonalities;
	while ( ! Persona.IsEmpty() )
	{
		if ( Persona == Dominant )	// "<Dominant>" is just a placeholder
			Persona = "";

		g_Personalities.SetCurrent( Persona );

		bool bSending = false;

		// If this is NOT a POP personality, don't check mail!!
		// JOK - 9/19/97.
		if (! GetIniShort (IDS_INI_USES_IMAP) )
		{
			
			QCPOPSettings *settings = new QCPOPSettings(Persona, bitflags);
			CPOPSession *popThread = new CPOPSession(settings, pPOPNotifier);
		
			bSending = settings->Sending();
			
			BOOL bResult = TRUE;
			if (settings->NeedPOP())
				bResult = popThread->DoGetPassword_();

			if(bResult)
			{
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
			BOOL bInBackground = TRUE;
			HRESULT hRes = GetImapMailMgr()->ImapGetMail (Persona, bitflags, pPOPNotifier, bInBackground);

			if ( !SUCCEEDED (hRes) )
			{
				// We didn't launch the check, so decrement the count.
				//
				pPOPNotifier->Decrement();
			}
			bool bSendOnCheck = GetIniShort(IDS_INI_SEND_ON_CHECK)?true:false;
			bSending = ((bitflags & kSendMailBit) || 
				((bitflags & kSendIfSendOnCheckBit) && bSendOnCheck))?true:false;
		}

		//do we need to schedule any sending threads??? SendOnCheckMail
		if(bSending)
			SendMail(Persona);

		// advance to next personality
		lpPersonalities += strlen( lpPersonalities ) + 1;
		Persona = lpPersonalities;
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
		// First try.  Load up the Wav file pathname from the user
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
				bPlayedSound = PlaySound(szPathname, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
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
