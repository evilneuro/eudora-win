// AuditEvents.cpp
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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


#include  "StdAfx.h"						// NOTE: Contains "#include" statements for MFC
											//		 library header files.

#include  "AuditEvents.h"
#include  "AuditLog.h"

#include "DebugNewHelpers.h"


/*
**	Definitions for the class: CAuditEvents
*/

// ** STATIC DEFINITIONS **

			CAuditEvents*	CAuditEvents::s_AuditEvents	= (CAuditEvents*) NULL;

	const	int				CAuditEvents::EVENT_BASE	= 19600;
	const	CString			CAuditEvents::FORMATS[]		= {
														   "%d",
														   "%d %d",
														   "%d %d %d",
														   "%d %d %d %d",
														   "%d %d %d %d %d",
														   "%d %d %d %d %d %d",
														   "%d %d %d %d %d %d %d",
														   "%d %d %d %d %d %d %d %d"
														  };

	/* 
	# Categories: (from auditdefs in the Mac project)
	=1	Your demographic data
	=2	Advertisements
	=3	Non-personal settings
	=4	Your Net/Eudora usage
	=5	The Eudora features you use
	*/
	const	int		CAuditEvents::kCategoryDemographics			= 1;
	const	int		CAuditEvents::kCategoryAdvertisements		= 2;
	const	int		CAuditEvents::kCategoryNonPersonalSettings	= 3;
	const	int		CAuditEvents::kCategoryNetEudoraUsage		= 4;
	const	int		CAuditEvents::kCategoryEudoraFeatures		= 5;

	const	int		CAuditEvents::CategoryMap[MAXEVENT]	= 
					{
						0,	// 0
						kCategoryNetEudoraUsage,	// SHUTDOWN = 1
						kCategoryNetEudoraUsage,	// TIMESTAMP,
						kCategoryNetEudoraUsage,	// CHECKSTART,
						kCategoryNetEudoraUsage,	// CHECKDONE,
						kCategoryEudoraFeatures,	// HIT,
						kCategoryEudoraFeatures,	// WINDOWOPEN,
						kCategoryEudoraFeatures,	// WINDOWCLOSE,
						kCategoryAdvertisements,	// ADOPEN,
						kCategoryAdvertisements,	// ADCLOSE,
						kCategoryAdvertisements,	// ADHIT,
						kCategoryNetEudoraUsage,	// SENDSTART,
						kCategoryNetEudoraUsage,	// SENDDONE,
						kCategoryEudoraFeatures,	// PERSCREATE,
						kCategoryEudoraFeatures,	// PERSDELETE,
						kCategoryNetEudoraUsage,	// STARTUP,
						kCategoryEudoraFeatures,	// PERSRENAME,
						kCategoryNetEudoraUsage,	// CONNECT,
					};
// ** PUBLIC METHODS **

	/*
	** Destructor
	*/
	CAuditEvents::~CAuditEvents (void)
	{
		if (this->m_AuditLog != (CAuditLog*) NULL) {
			delete this->m_AuditLog;
			this->m_AuditLog = (CAuditLog*) NULL;
		}
	}

	/*
	** [STATIC, PUBLIC] CreateAuditEvents
	*/
	bool CAuditEvents::CreateAuditEvents (CAuditLog* auditLog)
	{
		TRACE( "CAuditEvents::CreateAuditEvents (CAuditLog*)\n" );
		ASSERT( s_AuditEvents == (CAuditEvents*) NULL );

		if (s_AuditEvents != (CAuditEvents*) NULL)  delete s_AuditEvents;

		s_AuditEvents = DEBUG_NEW_NOTHROW CAuditEvents (auditLog);

		return (s_AuditEvents != (CAuditEvents*) NULL);
	}

	/*
	** [STATIC, PUBLIC] DestroyAuditEvents
	*/
	bool CAuditEvents::DestroyAuditEvents (void)
	{
		TRACE( "CAuditEvents::DestroyAuditEvents (void)\n" );

		if (s_AuditEvents != (CAuditEvents*) NULL) {
			delete s_AuditEvents;
			s_AuditEvents = (CAuditEvents*) NULL;
		}

		return (true);
	}

	/*
	** [PUBLIC] ShutdownEvent
	*/
    void CAuditEvents::ShutdownEvent (long faceTime, long rearTime, long connectTime, long totalTime)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [3],		// Arguments: 4
					  faceTime, rearTime, connectTime, totalTime);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (SHUTDOWN, args);
    }
    
	/*
	** [PUBLIC] TimestampEvent
	*/
    void CAuditEvents::TimestampEvent (long faceTime, long rearTime, long connectTime, long totalTime)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [3],		// Arguments: 4
					  faceTime, rearTime, connectTime, totalTime);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (TIMESTAMP, args);
    }
    
	/*
	** [PUBLIC] CheckStartEvent
	*/
    void CAuditEvents::CheckStartEvent (ULONG sessionID, ULONG personalityID, bool isAuto)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [2],		// Arguments: 3
					  sessionID, personalityID, isAuto);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (CHECKSTART, args);
    }
    
	/*
	** [PUBLIC] CheckDoneEvent
	*/
    void CAuditEvents::CheckDoneEvent (ULONG sessionID, long messagesRcvd, long bytesRcvd)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [2],		// Arguments: 3
			    	  sessionID, messagesRcvd, bytesRcvd);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (CHECKDONE, args);
    }
    
 	/*
	** [PUBLIC] HitEvent
	*/
    void CAuditEvents::HitEvent (bool shift, bool control, bool option, bool command, bool alt, ULONG windowID, long controlType, long eventType)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [7],		// Arguments: 8
					  shift, control, option, command, alt, windowID, controlType, eventType);

		ASSERT( this->GetAuditLog() != (CAuditLog*) NULL );

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (HIT, args);
    }
    
 	/*
	** [PUBLIC] WindowOpenEvent
	*/
    void CAuditEvents::WindowOpenEvent (ULONG windowID, ULONG windowKind, long wazooState)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [2],		// Arguments: 3
					  windowID, windowKind, wazooState);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (WINDOWOPEN, args);
    }
    
 	/*
	** [PUBLIC] WindowCloseEvent
	*/
    void CAuditEvents::WindowCloseEvent (ULONG windowID)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [0],		// Arguments: 1
					  windowID);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (WINDOWCLOSE, args);
    }
    
 	/*
	** [PUBLIC] AdOpenEvent
	*/
    void CAuditEvents::AdOpenEvent (LPCTSTR adID)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format ("%s", adID);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (ADOPEN, args);
    }
    
 	/*
	** [PUBLIC] AdCloseEvent
	*/
    void CAuditEvents::AdCloseEvent (LPCTSTR adID)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format ("%s", adID);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (ADCLOSE, args);
    }
    
 	/*
	** [PUBLIC] AdHitEvent
	*/
    void CAuditEvents::AdHitEvent (LPCTSTR adID, bool isOnline)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format ("%s %d", adID, isOnline);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (ADHIT, args);
    }
    
 	/*
	** [PUBLIC] SendStartEvent
	*/
    void CAuditEvents::SendStartEvent (ULONG sessionID, ULONG personalityID, bool isAuto)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [2],		// Arguments: 3
					  sessionID, personalityID, isAuto);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (SENDSTART, args);
	}
    
 	/*
	** [PUBLIC] SendDoneEvent
	*/
    void CAuditEvents::SendDoneEvent (ULONG sessionID, long messagesSent, long bytesSent)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [2],		// Arguments: 3
					  sessionID, messagesSent, bytesSent);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (SENDDONE, args);
    }
    
 	/*
	** [PUBLIC] PersCreateEvent
	*/
    void CAuditEvents::PersCreateEvent (ULONG personalityID)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [0],		// Arguments: 1
					  personalityID);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (PERSCREATE, args);
    }
    
 	/*
	** [PUBLIC] PersDeleteEvent
	*/
    void CAuditEvents::PersDeleteEvent (ULONG personalityID)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [0],		// Arguments: 1
					  personalityID);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (PERSDELETE, args);
    }
    
 	/*
	** [PUBLIC] StartupEvent
	*/
    void CAuditEvents::StartupEvent (long platform, long version, long buildNumber)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [2],		// Arguments: 3
					  platform, version, buildNumber);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (STARTUP, args);
    }
    
 	/*
	** [PUBLIC] PersRenameEvent
	*/
    void CAuditEvents::PersRenameEvent (ULONG oldPersID, ULONG newPersID)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [1],		// Arguments: 2
					  oldPersID, newPersID);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (PERSRENAME, args);
    }

	/*
	** [PUBLIC] ConnectEvent
	*/
    void CAuditEvents::ConnectEvent (bool connectionUp)
    {
		CString*  args = DEBUG_NEW CString();

		args->Format (CAuditEvents::FORMATS [0],		// Arguments: 1
					  connectionUp);

		// NOTE: It is the audit log's responsibility to delete the CString object, "args".

		(this->GetAuditLog())->LogIt (CONNECT, args);
    }


// ** PROTECTED METHODS **

	/*
	** [PROTECTED] Constructor
	*/
	CAuditEvents::CAuditEvents (CAuditLog* auditLog)
	: m_AuditLog (auditLog)
	{
		ASSERT( auditLog != (CAuditLog*) NULL );
	}

 	/*
	** [PROTECTED] GetAuditLog
	*/
    CAuditLog* CAuditEvents::GetAuditLog (void)
    {
		ASSERT( m_AuditLog != (CAuditLog*) NULL );

		return (m_AuditLog);
	}

