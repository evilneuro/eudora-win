// append.h - declaration of the CImapAppend class
//

#ifndef __IMAPAPPEND_H
#define __IMAPAPPEND_H


//======================================================================//
//====================== CImapMailbox class interface ==================//

// Forward declarations
class  CIMAP;
class  CMBXWriter;

// CImapMailbox class.
class CImapAppend
{
public:
	CImapAppend (CIMAP *pDstImap, CIMAP *pSrcImap, CSummary *pSum, unsigned long nSrcUid);
	~CImapAppend();

// Interface
public:
	// append a single message.
	HRESULT AppendMessage ( BOOL Spool, BOOL InBackground = FALSE);
	HRESULT AppendMessageAcrossRemotes (BOOL InBackground = FALSE);

	// Set/Get stuff.
	BOOL IsMultipart ()
			{ return m_bIsMultipart; }

	void CopyMbxStub ( LPCSTR pMbxStub )
			{ m_MbxStub = pMbxStub; }

	void SetSrcAttachDir ( LPCSTR pSrcAttachDir )
			{ m_SrcAttachDir = pSrcAttachDir; }

	void SetSpoolDir ( LPCSTR pSpoolDir )
			{ m_SpoolDir = pSpoolDir; }

	void SetFlags (unsigned long ulFlags);
	void SetMbxStub (LPCSTR p);

// Internal methods
private:
	BOOL SpoolMessageToFile (CString& SpoolFilepath);
	BOOL ReformatMessage ();
	BOOL ReformatHeader ();
	BOOL ReformatBody ();
	BOOL ReformatAttachment (LPCSTR pAttachmentPathname, CFileWriter *pBodyFileWriter);
	BOOL AddTextBodyPart (LPCSTR p, LPCSTR t, CFileWriter *pBodyFileWriter);

	HRESULT AppendSpoolFile (LPCSTR pSpoolFilepath);

	void NewMessageID (CString& szBuffer);


// Internal state:
public:
	CString		m_MbxStub;				// Copy of MBX file stub.
	CString		m_SrcAttachDir;
	CString		m_SpoolDir;				// Use this if it needs to spool stuff.
	CString		m_HeaderSpoolPath;		// Spool file for header
	CString		m_BodySpoolPath;		// Acccumulate body in this.
	CIMAP*		m_pDstImap;			// Append TO this mailbox. Contains by reference. Should "Dup()" this.
	CIMAP*		m_pSrcImap;			// Source CIMAP. Used when doing a IMAP-to-IMAP append.
	unsigned long m_SrcUid;				// Source UID when doing an IMAP-to-IMAP append.
	IMAPFLAGS	m_ImapFlags;

	// Scratch buffer. Allocated when needed.
	unsigned long	m_nBufferSize;
	TCHAR*			m_pBuffer;

	// Keep track of how many bytes left ot download during a remote-to-remote append.
	unsigned long m_nBytesRead;

	// Time of source summary.
	unsigned long m_Seconds;

private:
	// Dynamic state.
	LPCSTR		m_pStartHeader;
	LPCSTR		m_pStartBody;

	// MIME status of message
	BOOL		m_bIsMultipart;
	CString		m_TransferEncoding;
	CString		m_Boundary;
	CString		m_Type;
	CString		m_Subtype;

	CSummary*	m_pSum;
};



#endif // __IMAPAPPEND_H

