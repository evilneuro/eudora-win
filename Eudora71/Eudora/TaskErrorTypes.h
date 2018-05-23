#ifndef _TASK_ERROR_TYPES_H_
#define _TASK_ERROR_TYPES_H_

	
	enum TaskErrorType
	{
		TERR_UNKNOWN	= 0,
		TERR_WINSOCK	= 1,
		TERR_RAS		= 2,
		TERR_SMTP		= 4,
		TERR_POP		= 8,
		TERR_AUTH		= 16,

		TERR_FATAL		= 1024,
		TERR_TEMP		= 2048,
		TERR_MSG		= 4096,

		TERR_POPAUTH    = TERR_POP | TERR_AUTH,
		TERR_NETWORK	= TERR_WINSOCK | TERR_RAS
		
	};

	
	class QCError {
	
	public:
		
		virtual ~QCError(){ /*do nothing*/ }
		
		virtual void ShowContext() = 0;
	};


	class QCSmtpMsgError : public QCError 
	{
	public:

		QCSmtpMsgError(int nMsgID) : m_nMsgID(nMsgID){}
		virtual ~QCSmtpMsgError() { }

		virtual void ShowContext();

	
	private:
		int m_nMsgID;
	};



#endif