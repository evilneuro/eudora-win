// LEX822.HPP
//
// Routines for parsing RFC 822 messages
//

#ifndef _IMAPLEX822_H_
#define _IMAPLEX822_H_

// Include this for tthe data types.
#include "lex822.h"


// Forward Declaration
class ImapLineReaderType;
class CImapDownloader;

// Lex822State
class ImapLex822State
{
public:
	ImapLex822State(CImapDownloader *m_RW);

	Token822Enum Lex822();
	Token822Enum WriteHeaderToken(Token822Enum tokenType);

// Public attributes
public:
	State822Enum m_State;
	BYTE		 m_Token[256];		// tokens over 255 characters will be shot

private:
	Char822Enum LexFill();
	void ADD(BYTE c) { m_Token[m_TokenSpot++] = c; m_Token[m_TokenSpot] = 0; }

	int		m_TokenSpot;
	BYTE	m_Buffer[2048];		// input buffer line
	BYTE*	m_pSpot;			// spot we're currently processing
	BYTE*	m_pEnd;				// time to die
	short	m_InStructure;		// in a quote, comment, or domain literal
	short	m_ReinitToken;		// have we seen the end of the input stream?
	BOOL	m_UhOh;				// we tawt we taw a putty tat
	BOOL	m_Has2047;			// RFC 2047 parsing for 8bit chars in headers

	// This reads from the netword and also writer to the MBX file.
	CImapDownloader *m_pRW;	// make this a member because we can have different inputs
};


//===================== Exported utility functions ======================/
//
BOOL ImapFix2047(char* chars, int& len);


#endif

