// RegTransReader.h
//
// MIMEReader class for handling EMSAPI-like registration code
// attachment translations. Parsing of information is done
// utilizing rfc 822 code.

#ifndef _RegInfoReader_h_
#define _RegInfoReader_h_

#include "FileLineReader.h"
#include "LineReader.h"
#include "MIMEReader.h"
#include "stdafx.h"
#include "QCSharewareManager.h"


class CTLAddress;
class JJFile;
class MIMEState;
class CProfileConfirmationDialog;


//==============================================================================
// RegInfoReader
//
// Class for doing EMSAPI translations that an ON_ARRIVAL translator
// has said to do now
//
class RegInfoReader : public MIMEReader
{
  public:
	typedef enum
	{
		rs_Unknown = 0,
		rs_Invalid = 0,
		rs_Reg,
		rs_Profile
	} EudoraInfoEnum;

	static CProfileConfirmationDialog *		s_pProfileConfirmationDlg;

	static EudoraInfoEnum	GetRegInfoFromLineReader(
								LineReader *			pLineReader,
								bool					shouldWriteOutput,
								JJFile *				pOutputFile,
								CString &				outFirstName,
								CString &				outLastName,
								CString &				outRegCode,
								CString *				outProfileID,
								CString *				outMailedTo,
								bool *					outDeleteProfileID,
								CString *				outDistributorID,
								bool *					outEudoraNeedsRegistration);

	static bool				GetInfoFromRegCodeStartupFile(
								const char *			szFilePath,
								CString &				outFirstName,
								CString &				outLastName,
								CString &				outRegCode,
								CString &				outDistributorID,
								SharewareModeType &		outRegMode,
								bool &					outEudoraNeedsRegistration,
								int *					outRegCodeMonth = NULL);

	static void				ProcessEudoraInfoAttachment(
								const char *			szAttachmentPath);

	BoundaryType			ReadIt(
								CObArray &				MimeStates,
								char*					buf,
								LONG					bSize);

  protected:
	static void				DisplayProfileIDConfirmationDialog(
								UINT				nMessageID,
								const char *		szFirstName,
								const char *		szLastName,
								const char *		szMailedTo,
								const char *		szProfileID);
	
	bool					IsMailedToUser(
								CString &				szMailedTo);

	static EudoraInfoEnum	InternalGetInfoFromAttachment(
								const char *			szFilePath,
								CString &				outFirstName,
								CString &				outLastName,
								CString &				outRegCode,
								CString &				outProfileID,
								CString &				outMailedTo,
								bool &					outDeleteProfileID)
								{
									return InternalGetInfoFromFile( szFilePath, outFirstName, outLastName,
																	outRegCode, &outProfileID,
																	&outMailedTo, &outDeleteProfileID, NULL, NULL );
								}
		
	static EudoraInfoEnum	InternalGetInfoFromFile(
									const char *			szFilePath,
									CString &				outFirstName,
									CString &				outLastName,
									CString &				outRegCode,
									CString *				outProfileID,
									CString *				outMailedTo,
									bool *					outDeleteProfileID,
									CString *				outDistributorID,
									bool *					outEudoraNeedsRegistration);
};


// RegInfoLineReader
//
// Class which provides a line reader that detects a given mime boundary
// and returns 0 bytes read to indicate this.
class RegInfoLineReader : public LineReader
{
  public:
					RegInfoLineReader(
						LineReader *		inLineReader,
						MIMEState *			inParentMS)
						:	m_LineReader(inLineReader), m_ParentMS(inParentMS), m_BoundaryType(btEndOfMessage)
						{ ASSERT(m_LineReader != NULL); }
	virtual			~RegInfoLineReader() {}

	virtual long	ReadLine(char* buf, LONG bSize);

	BoundaryType	GetBoundaryType() const { return m_BoundaryType; }

  protected:
	LineReader *	m_LineReader;
	MIMEState *		m_ParentMS;
	BoundaryType	m_BoundaryType;
};


// RegInfoFileLineReader
//
// Class which provides a file line reader that returns an extra blank line.
// This avoid problems with Lex822State::LexFill adding ".CRLF" to the buffer
// to indicate the end of the header area, which previously caused the last
// line to be interpreted incorrectly when the file did not end with a CRLF.
class RegInfoFileLineReader : public FileLineReader
{
  public:
					RegInfoFileLineReader()
						:	bReadEOF(false) {}
	virtual			~RegInfoFileLineReader() {}

	virtual long	ReadLine(char* buf, LONG bufSize);

  protected:
	bool			bReadEOF;
};



#endif // #ifndef _RegInfoReader_h_
