// ImSingle.h - Applesingle attachment decoder.
//


#ifndef _IMSINGLE_H_
#define _IMSINGLE_H_

#include "Decode.h"


class ImapDecodeAS : public Decoder
{
public:
	ImapDecodeAS();
	~ImapDecodeAS();
	int Decode(const char* In, LONG InLen, char* Out, LONG& OutLen);
	int Init();

	// The embedded filname is store in "m_Filename".
	//
	LPCTSTR GetEmbeddedFilename () { return (LPCTSTR) m_Filename; }

	// Specific to this decoder.
	// Note; We must free the decoder when we're done.
	//
	void SetSecondaryDecoder (Decoder* pDecoder)
				{ m_pSecondaryDecoder = pDecoder; }

protected:
	//
	long	m_HeaderCount;
	long	m_NameOffset;
	long	m_NameEnd;
	long	m_TypeOffset;
	long	m_TypeCreatorOffset;
	long	m_DataOffset;
	int		m_LastType;

	// Internal secondary decoder:
	Decoder* m_pSecondaryDecoder;

	// For accumulating embedded filename.
	//
	char	m_Filename[48]; // Room for filename plus extension and 0.
	char*	m_pNamePtr;
	char	m_ProgBuf[128];
	char	m_TypeCreator[8];
	long	m_Number;

	BOOL	m_bDoneWithHeader;
};

#endif // #ifndef _IMSINGLE_H_
