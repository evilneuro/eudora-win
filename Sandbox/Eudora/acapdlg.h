// 
//
/////////////////////////////////////////////////////////////////////////////
// CAcapDlg dialog


static BOOL	DeleteNetObj;
static BOOL Connected;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
typedef struct
{
	int nStringID;
	CString strSetting;

}SEARCH_STRING;

class CAcapSettings
{
// Construction
public:
	CAcapSettings();	// standard constructor
	~CAcapSettings();

	CString	m_strUserID;
	CString	m_strServerName;
	CString	m_strPassword;
	char *m_szStamp;
	//}}AFX_DATA


// Implementation
	public:

		enum Authentication
		{
 
            AUTH_XCRAM,
            AUTH_CRAM,
			AUTH_APOP,
			AUTH_PLAIN,
			AUTH_NONE
		} m_Authentication;

		CObList *m_pStringIDList;
		CObArray *m_pPositions;
	public:
	BOOL RetrieveSettings(CString strServer, CString strUserID, CString strPass);

	protected:
		int ConnectAcapServer(void);
		int GetGreeting(void);
		int ConfigureClient(void);
		int AuthenticateAcap(void);
		int LoginAcap(void);
		int LogoutAcap(void);
		void ReceiveSearch(char* );
		void SetFields(int nID, char * strSetting);
		int RetrieveID(CString strSetting);
		void LoadStrings();
		int AuthenticatePlainAcap(void);
		//int AuthenticateAPOPAcap();
        AuthenticateAPOPCRAMAcap(int nType);
		CString GetAcapServer(CString strHostName);
		BOOL CloseConnection();
		
};
//{{AFX_INSERT_LOCATION}}

