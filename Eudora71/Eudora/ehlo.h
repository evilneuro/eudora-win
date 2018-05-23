// ehlo.h: Helper class for managing the EHLO responses from an ESMTP compliant
// server.
//




typedef enum EsmtpKey
{
	ESMTP_ERROR		= 0,
	ESMTP_SIZE		= 1,
	ESMTP_MIME8BIT	= 2,
	ESMTP_AUTH		= 3,
	ESMTP_STARTTLS  = 4,

};



class CEhlo
{
public:
	CEhlo ();
	~CEhlo();

	void ParseMaxSize (LPCSTR pszBuf);
	void SetMime8bit (BOOL bVal) { m_bMime8Bit = bVal; }
	void SetAuth	 (BOOL bVal) { m_bAuth = bVal; }
	void SetTLS      (BOOL bVal) { m_bTLS = bVal;  }
    BOOL SupportsTLS () { return m_bTLS;}
	static EsmtpKey FindESMTPType (LPCSTR pDirective);


	BOOL SawGreeting()	{ return m_bSawGreeting; }



private:
	// ESMTP capabilities. These are set for each session after reading the EHLO response.
	//
	long m_MaxSize;

	BOOL m_bSawGreeting;

	BOOL m_bMime8Bit;

	// If sever supports AUTH.
	BOOL m_bAuth;
	BOOL m_bTLS;
};