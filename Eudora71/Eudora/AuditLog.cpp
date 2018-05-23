// AuditLog.cpp
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

#include  <afxmt.h>							// Needed: CSingleLock

#include  <time.h>							// Needed:	time,
											//			time_t

#include "resource.h"
#include "utils.h"

#ifndef  __AuditLog__
	#include  "AuditLog.h"
#endif

#ifndef  __AuditEvents__
	#include  "AuditEvents.h"
#endif

#include "DebugNewHelpers.h"


/*
**	Definitions for the class: CAuditLog
*/

// ** STATIC DEFINITIONS **

	// NOTE: "AUDIT_INTRO_FORMAT" is the Mac equivalent to "PREAMBLE_FORMAT".

	const CString		CAuditLog::PREAMBLE_FORMAT	= "%.2d%.2d%.2d%.2d%.2d %d %d ";

	const int			CAuditLog::PRODUCT_CODE		= 32;

	CCriticalSection	CAuditLog::s_Guard;


// ** PUBLIC METHODS **

	/*
	** Constructor
	*/
	CAuditLog::CAuditLog (LPCTSTR logDirName, LPCTSTR logFileName)
	: m_LogFile ((JJFileMT*) NULL),
		m_logDir(logDirName),
		m_logName(logFileName),
		m_doRead(true)

	{
		this->OpenLog ();
	}

	/*
	** Destructor
	*/
	CAuditLog::~CAuditLog (void)
	{
		CloseLog();
	}


	void CAuditLog::GetLogFileName(CString& outFileName)
	{
		outFileName.Format("%s%s", m_logDir, m_logName);
	}

 	/*
	** [PUBLIC] LogIt
	*/
    void CAuditLog::LogIt (CAuditEvents::Types event, CString* args)
    {
		ASSERT( args != (CString*) NULL );

		if (!m_doRead) 
		{
			CString*  entry = DEBUG_NEW CString();

			this->AddPreamble  (entry, event);
			this->AddArguments (entry, args);

			this->AddEntry (entry);

			// NOTE: It is this method's responsibility to delete the CString object, "entry".

			delete entry;
			entry = (CString*) NULL;

			// NOTE: A select few of the events require the log to be flushed to the disk.  Also,
			//			the value of "result" does not play a role in determining whether or not
			//			the log file is flushed.  The reasoning being that it is the event that
			//			is important not whether or not the entry is successfully written to the
			//			log file.

			switch (event) {
				case CAuditEvents::SHUTDOWN:
				case CAuditEvents::TIMESTAMP:
				case CAuditEvents::CHECKSTART:
				case CAuditEvents::SENDSTART:
				case CAuditEvents::STARTUP: {
					this->Flush();
				}
			}
		} 
		else
		{
			TRACE("CAuditLog::LogIt: Cannot write audit log - in readonly mode\n");
			return;	// can't write anything
		}

		// NOTE: It is this method's responsibility to delete the CString object, "args".

		delete args;
		args = (CString*) NULL;
	}

	/*
	** [PUBLIC] OpenLog
	*/
	bool CAuditLog::OpenLog (bool doRead /* = false */, bool doTruncate /* = false */)
	{
		// How are you supposed to truncate a file in read-only mode, huh?
		ASSERT(!doRead || !doTruncate);

		if ( this->GetLogFile() != (JJFileMT*) NULL ) {
			delete this->GetLogFile();  this->SetLogFile ((JJFileMT*) NULL);
		}

		this->SetLogFile (DEBUG_NEW_NOTHROW JJFileMT());	// Use the default buffer size.

		// TIME TO BAIL: When unable to create the file.
		
		if ( this->GetLogFile() == (JJFileMT*) NULL )  return false;

		char pathname [_MAX_PATH + 1];

		(void) wsprintf (pathname, "%s%s", m_logDir, m_logName);

		int mode;

		if (doRead)
		{
			mode = (O_RDONLY);
			m_doRead = true;
		}
		else 
		{
			mode = (O_CREAT | O_APPEND | O_WRONLY);
			if (doTruncate) mode |= O_TRUNC;

			m_doRead = false;
		}

		if ( FAILED( (this->GetLogFile())->Open (pathname, mode) ) ) {
			delete this->GetLogFile(); 
			this->SetLogFile ((JJFileMT*) NULL);
			
			// TIME TO BAIL: When unable to obtain access to the log file.

			return false;
		}

		if (!doRead)
		{
			long Offset = 0;
			if (SUCCEEDED(this->GetLogFile()->Tell(&Offset)) && Offset == 0)
			{
				// If we're starting from scratch, put the friendly message text in first
				CString AuditPreamble;
				QCLoadTextData(IDT_AUDIT_PREAMBLE, AuditPreamble);
				(this->GetLogFile())->PutLine(AuditPreamble);
			}
		}

		return true;
	}

	/*
	** [PUBLIC] CloseLog
	*/
	void CAuditLog::CloseLog (void)
	{
		if ( this->GetLogFile() != (JJFileMT*) NULL ) {

			if ( (this->GetLogFile())->IsOpen() == S_OK ) {
				(this->GetLogFile())->Close();		// NOTE: Closing involves a flush
			}

			delete this->GetLogFile();
			this->SetLogFile ((JJFileMT*) NULL);
		}
	}



// ** PROTECTED METHODS **

 	/*
	** [PROTECTED] AddPreamble
	*/
    void CAuditLog::AddPreamble (CString* entry, CAuditEvents::Types event)
    {
		ASSERT( entry->GetLength() == 0 );	// NOTE: Should start with a clean slate.

		// Determine the current time.

		time_t  sysTime;

		time (&sysTime);

		CTime  currentTime (sysTime);

		// Combine the time, product code, and event type

		entry->Format (CAuditLog::PREAMBLE_FORMAT,
					   (currentTime.GetYear() % 100),	// Only use the year's last two digits
					   currentTime.GetMonth(),
					   currentTime.GetDay(),
					   currentTime.GetHour(),			// Using military time
					   currentTime.GetMinute(),
					   CAuditLog::PRODUCT_CODE,			// Add the product code
					   event);							// Add the event type
	}
    
 	/*
	** [PROTECTED] AddArguments
	*/
    void CAuditLog::AddArguments (CString* entry, CString* args)
    {
		*entry += *args;
	}

#include <tchar.h>
 	/*
	** [PROTECTED] AddEntry
	*/
    bool CAuditLog::AddEntry (CString* entry)
    {
		ASSERT( entry != (CString*) NULL );

		// TIME TO BAIL: When there is nothing to add.

		if ( entry == (CString*) NULL )  return (false);

		// Assumption: The log file should exist prior to this call and be open.

		ASSERT( this->GetLogFile() != (JJFileMT*) NULL );

		// TIME TO BAIL: When there is no log file.

		if ( this->GetLogFile() == (JJFileMT*) NULL )  return (false);

		ASSERT( (this->GetLogFile())->IsOpen() == S_OK );

		// TIME TO BAIL: When the log file is not open.

		if ( (this->GetLogFile())->IsOpen() == S_FALSE)  return (false);

		CSingleLock lock (&s_Guard, TRUE);

		int  length = entry->GetLength();

		HRESULT  result = (this->GetLogFile())->PutLine (entry->GetBuffer (length), length);

		return (FAILED(result));
	}

 	/*
	** Flush [PROTECTED]
	*/
    void CAuditLog::Flush (void)
    {
		if ( this->GetLogFile() != (JJFileMT*) NULL ) {

			if ( (this->GetLogFile())->IsOpen() == S_OK ) {
				CSingleLock lock (&s_Guard, TRUE);

				(this->GetLogFile())->Flush();
			}
		}
	}

	/*
	** [PROTECTED] DisplayErrorMsg
	**
	** Display the message string associated with the system error code supplied as the
	**		parameter, "error".
	*/
	void  CAuditLog::DisplayErrorMsg (DWORD error)
	{
		LPVOID msgBuffer;

		DWORD  flags = (FORMAT_MESSAGE_ALLOCATE_BUFFER	| 
						FORMAT_MESSAGE_FROM_SYSTEM		|
						FORMAT_MESSAGE_IGNORE_INSERTS);

		FormatMessage (flags,
					   NULL,
					   error,
					   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// Default language
					   reinterpret_cast<LPTSTR>(&msgBuffer),
					   0,
					   NULL );

		MessageBox (NULL, static_cast<LPCTSTR>(msgBuffer), _TEXT("Error"), (MB_OK));

		LocalFree( msgBuffer );
	}
