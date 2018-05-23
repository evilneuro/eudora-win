// SENDMAIL.H
//
// Include file for the sendmail routines.
//

// Function Prototypes

class CCompMessageDoc;
class Network;
extern Network* NetConnection;

static BOOL SMTPConnected = FALSE;
static BOOL	DeleteNetObj;
static BOOL	UsingPOPSend;
static CString CurSMTPServer;


int ComposeDate(char* DateBuf, unsigned long GMTTime, int TimeZoneMinutes = -1, CSummary* SumToSetDate = NULL);
inline int ComposeDate(char* DateBuf, CSummary* SumToSetDate)
	{ return ComposeDate(DateBuf, SumToSetDate->m_Seconds, SumToSetDate->m_TimeZoneMinutes, SumToSetDate); }
int StartSMTP(const char *server);
int FinishSMTP();
int EndSMTP(BOOL abort);
int DoSMTPIntro();
int SendMessage(CCompMessageDoc* msg);
int XmitMessage(BOOL SendBody = TRUE, BOOL StdHdrs = TRUE);
int SendRawMIME(const char *FileName);
char* GetSignature(CCompMessageDoc* CompDoc, BOOL ConvertRichToHTML);


HRESULT SpoolOne(CSummary* Sum,HRESULT InResult);
HRESULT SendQueuedMessages2(int WhichToSend /*= QS_READY_TO_BE_SENT*/,
									BOOL bMultiPersona = TRUE,
									bool bSend =TRUE);

void PostProcessOutgoingMessages(void *pv, bool bEnteringLastTime);


#ifndef _QCSMTPMessage_h_
#define _QCSMTPMessage_h_

class CFileIO;

class QCSMTPMessage 
{
public:
	QCSMTPMessage(CCompMessageDoc*);

	HRESULT Start(const char* filename);
	HRESULT End();
	HRESULT WriteHash();
	HRESULT WriteEnvelope();
	HRESULT WriteBody();
	HRESULT RedirectOutput(const char* filename);
	HRESULT EndRedirect();
	HRESULT WriteHeaders();
	HRESULT WriteRawMIME(const char *FileName);

private:
	Network* m_oldNetConnection;
	CFileIO* m_fileIO;
};

#endif

