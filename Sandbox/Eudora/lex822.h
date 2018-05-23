// LEX822.HPP
//
// Routines for parsing RFC 822 messages
//

#ifndef _LEX822_H_
#define _LEX822_H_

// Data Types

typedef enum
{
	LinearWhite,
	Atom,
	QText,
	RegText,
	DomainLit,
	Comment,
	Special,
	EndOfHeader,
	EndOfMessage,
	ErrorToken,				// something went wrong
	Continue822
} Token822Enum;

typedef enum
{
	Init822,			// get started
	CollectLine,		// not doing anything special right at the moment
	CollectLWSP,		// collecting a run of LWSP
	CollectAtom,		// collecting an atom
	CollectComment,		// collecting an RFC 822 comment
	CollectQText,		// collecting a quoted text string
	CollectDL,			// collecting a domain literal
	CollectSpecial,		// collecting a single special character
	CollectText,		// collecting an unstructured field body
						// we have to be put in this state by an external force
	ReceiveError,		// ran out of characters
	State822Limit
} State822Enum;

/*
 * this enum names the various character classes
 */
typedef enum
{
	ALPHA822,	DIGIT822,	CTL822,		CR822,
	LF822,		SPACE822,	HTAB822,	QUOTE822,
	RPAR822,	LPAR822,	AT822,		COMMA822,
	SEM822,		COLON822,	BSLSH822,	DOT822, 
	LSQU822,	RSQU822,	CHAR822,	EQUAL822,
	SLASH822,	QMARK822,
	CHARTYPE_LIMIT
} Char822Enum;

// Forward Declaration
class LineReader;

// Lex822State
class EXPORT Lex822State
{
public:
	Lex822State(LineReader* pLR);

	Token822Enum Lex822();
	Token822Enum WriteHeaderToken(Token822Enum tokenType);

	State822Enum State;
	BYTE Token[256];		// tokens over 255 characters will be shot

private:
	Char822Enum LexFill();
	void ADD(BYTE c) { Token[TokenSpot++] = c; Token[TokenSpot] = 0; }

	int TokenSpot;
	BYTE Buffer[2048];		// input buffer line
	BYTE* Spot;				// spot we're currently processing
	BYTE* End;				// time to die
	short InStructure;		// in a quote, comment, or domain literal
	short ReinitToken;		// have we seen the end of the input stream?
	BOOL UhOh;				// we tawt we taw a putty tat
	BOOL Has2047;			// RFC 2047 parsing for 8bit chars in headers

	LineReader* m_LR;		// make this a member because we can have different inputs
};


// Function Prototypes

char* Quote822(char* into, const char* from, BOOL space = TRUE, BOOL Param1521 = FALSE);

#endif
