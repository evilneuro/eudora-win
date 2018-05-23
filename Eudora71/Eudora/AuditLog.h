#ifndef  __AuditLog__
#define  __AuditLog__


#include "stdafx.h"

#include  "Afxmt.h"							// Needed: CCriticalSection

#ifndef  __AuditEvents__
	#include  "AuditEvents.h"
#endif

#include <QCUtils.h>

/*
** Declarations for the class: CAuditLog
*/

class CAuditLog  {
	public:
				CAuditLog  (LPCTSTR logDir, LPCTSTR logName);
		virtual	~CAuditLog (void);

				void	GetLogFileName(CString& outFileName);
				void	LogIt	(CAuditEvents::Types event, CString* args);
				bool	OpenLog		(bool doRead = false, bool doTruncate = false);
				void	CloseLog	(void);
				JJFileMT*	GetLogFile		(void);

	protected:
				bool		AddEntry			(CString* entry);

				void		AddPreamble			(CString* entry, CAuditEvents::Types event);
				void		AddArguments		(CString* entry, CString* args);

				void		Flush				(void);

				void		SetLogFile			(JJFileMT* logFile);


				void		DisplayErrorMsg		(DWORD error);

		static	CCriticalSection	s_Guard;

		static	const CString		PREAMBLE_FORMAT;
		static	const int			PRODUCT_CODE;

	private:
				JJFileMT*	m_LogFile;
				CString		m_logDir;
				CString		m_logName;
				bool		m_doRead;

				CAuditLog  (void);
				CAuditLog  (const CAuditLog&);
				CAuditLog& operator= (const CAuditLog&);
};


/*
**	Inline Definitions for the class: CAuditLog
*/

 	/*
	** [PUBLIC] GetLogFile
	*/
    inline JJFileMT*  CAuditLog::GetLogFile (void)
    {
		return m_LogFile;
	}

	/*
	** [PROTECTED] SetLogFile
	*/
    inline void  CAuditLog::SetLogFile (JJFileMT* logFile)
    {
		m_LogFile = logFile;
	}

#endif  // __AuditLog__
