// AuditEvents.h
//
// Copyright (c) 1999 by QUALCOMM, Incorporated
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


#ifndef  __AuditEvents__
#define  __AuditEvents__


#include  <windef.h>						// NOTE: Contains basic Window's type definitions.

#include  "afx.h"							// Needed: CString


/*
** Forward Declarations
*/

class CAuditLog;


/*
** Declarations for the class: CAuditEvents
*/

class CAuditEvents  {
	public:
		static	bool			CreateAuditEvents	(CAuditLog* auditLog);
		static	bool			DestroyAuditEvents	(void);
		static	CAuditEvents*	GetAuditEvents		(void);

		static	enum	Types	{ SHUTDOWN = 1,		// Start at 1 instead of 0 [to match Mac version]
								  TIMESTAMP,
								  CHECKSTART,
								  CHECKDONE,
								  HIT,
								  WINDOWOPEN,
								  WINDOWCLOSE,
								  ADOPEN,
								  ADCLOSE,
								  ADHIT,
								  SENDSTART,
								  SENDDONE,
								  PERSCREATE,
								  PERSDELETE,
								  STARTUP,
								  PERSRENAME,
								  CONNECT,
								  MAXEVENT // not really an event, but an upper bound
								};

		static	const	int		CategoryMap[MAXEVENT];

		static	const	int		kCategoryDemographics;
		static	const	int		kCategoryAdvertisements;
		static	const	int		kCategoryNonPersonalSettings;
		static	const	int		kCategoryNetEudoraUsage;
		static	const	int		kCategoryEudoraFeatures;

		virtual	~CAuditEvents (void);

				void	ShutdownEvent		(long faceTime,
											 long rearTime,
											 long connectTime,
											 long totalTime);

				void	TimestampEvent		(long faceTime,
											 long rearTime,
											 long connectTime,
											 long totalTime);

				void	CheckStartEvent		(ULONG sessionID, ULONG personalityID, bool isAuto);
				void	CheckDoneEvent		(ULONG sessionID, long messagesRcvd, long bytesRcvd);

				void	HitEvent			(bool shift,
											 bool control,
											 bool option,
											 bool command,
											 bool alt,
											 ULONG windowID,
											 long controlType,
											 long eventType);

				void	WindowOpenEvent		(ULONG windowID, ULONG windowKind, long wazooState);
				void	WindowCloseEvent	(ULONG windowID);

				void	AdOpenEvent			(LPCTSTR adID);
				void	AdCloseEvent		(LPCTSTR adID);
				void	AdHitEvent			(LPCTSTR adID, bool isOnline);

				void	SendStartEvent		(ULONG sessionID, ULONG personalityID, bool isAuto);
				void	SendDoneEvent		(ULONG sessionID, long messagesSent, long bytesSent);

				void	PersCreateEvent		(ULONG personalityID);
				void	PersDeleteEvent		(ULONG personalityID);
				void	PersRenameEvent		(ULONG oldPersID, ULONG newPersID);

				void	StartupEvent		(long platform, long version, long buildNumber);
				void	ConnectEvent		(bool connectionUp);

				CAuditLog*	GetAuditLog		(void);

	protected:
		static	CAuditEvents*	s_AuditEvents;

		static	const int		EVENT_BASE;
		static	const CString	FORMATS[];


	private:
				CAuditLog*		m_AuditLog;

				CAuditEvents  (void);
				CAuditEvents  (CAuditLog* auditLog);

				CAuditEvents  (const CAuditEvents&);
				CAuditEvents& operator= (const CAuditEvents&);
};


/*
**	Inline Definitions for the class: CAuditEvents
*/

	/*
	** [PUBLIC] GetAuditEvents
	*/
    inline CAuditEvents* CAuditEvents::GetAuditEvents (void)
    {
		return (s_AuditEvents);
    }

#endif  // __AuditEvents__
