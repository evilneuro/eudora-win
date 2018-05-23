/////////////////////////////////////////////////////////////////////////////
// 	File: ImapDownload.H 
//
//  Purpose:  This file provides the declarations for the IMAP download
//				routines.   
//
//	Author: Based on imap.cpp.
//	Created:   9/26/97
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _IMAPDOWNLOAD_H
#define _IMAPDOWNLOAD_H

#include "exports.h"

#include "Decode.h"

// Points in the download process.
enum DOWNLOADMODE
{
	DOWNLOADING_NONE,
	DOWNLOADING_HEADER,
	DOWNLOADING_HEADER_TO_STR,
	DOWNLOADING_BODY
};

// Enumerated disposition types.
enum DISPOSITION_TYPE
{
	DISPOSITION_UNKNOWN,
	DISPOSITION_INLINE,
	DISPOSITION_ATTACHMENT
};


class CTocDoc;
class CImapSum;	 
class CImapSumList;
class ImapHeaderDesc;
class CTLAddress;
 
// CDownloaded class. Instantiate one of these to download message components.
// Pass it a pointer to an opened CImapConnection object. It will use it to fetch stuff from
// the connected IMAP mailbox instance.
// Also, pass it the path to the MBX file.

class CImapDownloader : public CWriter
{
public:
	CImapDownloader( unsigned long AccountID, CImapConnection* pImap, LPCSTR pPathname);
	~CImapDownloader();

// Methods for managing the MBX file.
public:
// Interface to caller:	
	BOOL	OpenMbxFile (BOOL Truncate);
	BOOL	CloseMbxFile ();

// We MUST override this:
	BOOL	Write (readfn_t readfn, void * read_data, unsigned long size);

	// Append stuff directly to the file.
	BOOL	Write (const char *buffer, ULONG nBytes);
	BOOL	WriteTextLine  (const char *buffer = NULL);

	// Access
	LPCSTR GetMbxPathname ()		{ return (LPCSTR) m_Pathname; }

	// Convenience routines.

	BOOL	Put(char value)
			{ return (Write ((char*)&value, sizeof(char))); }
	BOOL	Put(short value)
			{ return (Write ((char*)&value, sizeof(short))); }
	BOOL	Put(BOOL value)
			{ return (Write ((char*)&value, sizeof(BOOL))); }
	BOOL	Put(unsigned short value)
			{ return (Write ((char*)&value, sizeof(unsigned short))); }
	BOOL	Put(long value)
			{ return (Write ((char*)&value, sizeof(long))); }
	BOOL	Put(unsigned long value)
			{ return (Write ((char*)&value, sizeof(unsigned long))); }
	BOOL	Put(const char* pBuffer = NULL, long lNumBytesToWrite = -1L)
			{ return (Write (pBuffer, lNumBytesToWrite)); }

	BOOL	AddMessageSeparator ();
	BOOL	WriteXHEADERLine ();

	// Decoder stuff.
	void	SetDecoder (unsigned short Encoding);
	void	ResetDecoder ();

// Access methods.
	void	SetDisposition (DISPOSITION_TYPE Type)
			{ m_Disposition = Type; }
	DISPOSITION_TYPE GetDisposition ()
			{ return m_Disposition; }

// Read a line from the network.
	int		ReadLine (char *Buffer, int nBufSize);

// Public interfaces for starting the IMAP download process.
public:
	HRESULT		DoMinimalDownload(CUidMap &uidmap,
								  CImapSumList *pSumList,
								  unsigned long *puiHighestUID);
	HRESULT		DoMinimalDownload (IMAPUID Uid, CImapSum **ppSum);

	bool		CanTranslate(
						BODY *				in_pBody,
						CTLAddress *		in_pAddresses,
						const char *		in_szMimeVersion);
	bool		CanTranslatePart(
						BODY *				in_pParentBody,
						CTLAddress *		in_pAddresses,
						const char *		in_szMimeVersion);
	bool		CanTranslateMessageOrPart(BODY * in_pBody);
	HRESULT		DownloadSingleMessage (CImapSum *pSum, BOOL bDownloadAttachments);
	HRESULT     DownloadSingleMessageFully (CImapSum *pSum);

	void		CheckMoodScore(CImapSum *pSum);
	void		CheckJunkScore(CImapSum *pSum, JJFileMT *pFile);

	HRESULT		WriteOfflineMessage (CImapSum *pSum);

	BOOL		DownloadFullHeaderToString(IMAPUID uid);

// Private methods.
private:
	// Slurp data into internal buffer.
	BOOL Getdata();

	// Internal Download status
	void DownloadingNone()		{ m_DownloadMode = DOWNLOADING_NONE; }
	void DownloadingHeader()	{ m_DownloadMode = DOWNLOADING_HEADER; }
	void DownloadingHeaderToStr()	{ m_DownloadMode = DOWNLOADING_HEADER_TO_STR; }
	void DownloadingBody()		{ m_DownloadMode = DOWNLOADING_BODY;   }

	BOOL IsDownloadingHeader () { return m_DownloadMode == DOWNLOADING_HEADER; }
	BOOL IsDownloadingHeaderToStr () { return m_DownloadMode == DOWNLOADING_HEADER_TO_STR; }
	BOOL IsDownloadingBody ()	{ return m_DownloadMode == DOWNLOADING_BODY; }

	void SetSummaryInfoFromHD ( CImapSum* pSum, ImapHeaderDesc *pHd);


// Internal downloader methods.
private:
	BOOL		DownloadFullHeaderToMbxFile (IMAPUID uid);

	BOOL		DownloadShortHeaderToMbxFile (IMAPUID uid);

	BOOL		DownloadSimpleBody (IMAPUID uid, BODY *pBody, LPSTR pSection);

	BOOL		DownloadMultipartBody (IMAPUID uid, BODY *pParentBody, LPSTR pParentSection);

	BOOL		DownloadAllParts (IMAPUID uid, BODY *pParentBody,
							LPCSTR pParentSection);

	BOOL		HandleMultipartAlternative (IMAPUID uid, BODY *pParentBody, 
							LPCSTR pParentSection);

	BOOL		HandleAppleDouble (IMAPUID uid, BODY *pParentBody, LPCSTR pParentSection);

	BOOL		BreakoutBodyToFile (IMAPUID uid, BODY *pBody, LPSTR pSection, LPCSTR pSuggestedFilename = NULL);

	BOOL		BreakoutBodyToEmbeddedFile (IMAPUID uid, BODY *pParentBody, BODY *pBody, LPSTR pSection, int iMRelatedID);

	BOOL		AppendBodyTextToMbxFile (IMAPUID uid, LPSTR pSection, BODY *pBody);

	BOOL		DownloadSubMessage (IMAPUID uid, BODY *pBody, LPSTR pParentSection);

	BOOL		CreateAttachFile (BODY *pBody, CString &Pathname);

	BOOL		WriteAttachmentLink ( LPCSTR pPathname);

	BOOL		ParseMinimalHeaderInMemory(CImapSum* pSum, LPCSTR *ppHeader);

	HRESULT		DoMinimalDownloadUsingHeaders(CUidMap &uidmap,
											  CImapSumList *pSumList,
											  unsigned long *puiHighestUID,
											  BOOL bUpdateStats);
	HRESULT		DoMinimalDownloadUsingHeadersOnRange(IMAPUID uidFirst,
													 IMAPUID uidLast,
													 CUidMap &uidmap,
													 CImapSumList *pSumList,
													 unsigned long *puiHighestUID,
													 BOOL bUpdateStats);
	HRESULT		DoMinimalDownloadUsingHeaders (IMAPUID Uid, CImapSum **ppSum);

	HRESULT		DoMinimalDownloadUsingEnvelope (IMAPUID Uid, CImapSum **ppSum);
	
	BOOL		ParseHeaderInMemory (CImapSum* pSum, LPCSTR pHeader);

	void		DoAddAdditionalHeaders (IMAPUID Uid, CImapSum *pSum);

	BOOL		ParseAdditionalHeadersInMemory (CImapSum* pSum, LPCSTR pHeader);

	BOOL		BadlyParsedBody(BODY *pBody);

	BOOL		BadlyParsedBodyCourier(BODY *pBody);

	// Embedded line management.
	BOOL		SaveEmbeddedElement( const char * filename, ImapHeaderDesc *pHd, int nMPRelatedID );
	BOOL		WriteEmbeddedElements( BOOL isMHTML );
	void		FlushEmbeddedElements();
	BOOL		ImapRebuildSummary (CImapSum *pSum, ImapHeaderDesc *pHd, JJFileMT *pJJFile, long lHeaderLength);
	BOOL		ContentIdOfSection(IMAPUID uid, char *sequence, char **id);
	HRESULT     WriteToMBXFile(CImapSum *pSum, CString spoolFilePath);

// Public data members.
public:
	// Make this public to allow outside manipulation of the file.
	JJFileMT	m_mbxFile;					// Use this to store to file.

	CString		m_strFullHeader;			// String for holding header data.

// State:
private:
	CString		m_Pathname;					// Store contents in this file.
	LONG		m_CurrentSize;				// Length of data in m_Buffer.
	LONG		m_FullSize;					// Size of all of data.
	Decoder*	m_Decoder;
	BOOL		m_bMustReadSingleLines;
	BOOL		m_bEndedWithCRLF;

	// Allocated buffer.
	// When m_pReadBuffer is created, set the buffer size also.
	long		m_nBufferSize;
	LPSTR		m_pReadBuffer;
	LPSTR		m_pWriteBuffer;

	LPSTR		m_Ptr;
	int			m_Vbytes;

	DOWNLOADMODE	m_DownloadMode;			// 

	// The object that gives us access to the IMAP server.
	CImapConnection*		m_pImap;

	// Network READ function.
	readfn_t	m_pReadFn;
	void*		m_pReadData;

	// Header data object. Must be re-initialized between messages.
	ImapHeaderDesc* m_pHd;

	// List of embedded objects.
	CObList		m_EmbeddedElements;

	// Used to get a unique ID for a multipart-related block.
	// Initialized to 0 and incremented for every M/R block we meet.
	int m_iLastMRelatedID;

	// Set the type of the body we are currently fetching.
	// Initialize to TYPETEXT.

	unsigned int m_CurrentBodyType;
	CString		 m_szCurrentBodySubtype;

	// Set to TRUE is writer found HTML.
	BOOL	m_bIsMhtml;

	// Set to TRUE if any part was broken out as an attachment.
	BOOL	m_bHasAttachment;

	// THis is the current summary (CImapSum) for this message.
	// Can be NULL!!.
	//
	 CImapSum*	m_pImapSum;

	// Enumerated disposition type:
	DISPOSITION_TYPE  m_Disposition;

	BOOL m_bUseEnvelope;

	// Caller must set this!!
	unsigned long m_AccountID;

	// If set, force download of all attachments.
	//
	BOOL m_bForceAttachDownload;

	CTaskInfoMT	*m_pTaskInfo;
};


// Exported function.
BOOL WriteBodyPartPlaceHolder (CImapConnection *pImap, IMAPUID Uid, BODY *pBody, LPCSTR pSection, LPCSTR pFilePath);
BOOL ImapDownloaderFetchAttachment (LPCSTR Filename);
BOOL IsImapDownloaded (LPCSTR Filename);

#endif	// #ifndef _IMAPDOWNLOAD_H



