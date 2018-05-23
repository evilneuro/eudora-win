// MSGUTILS.CPP
//
// Helper routines for messages
//

#include "stdafx.h"

#include <QCUtils.h>

#include "msgutils.h"
#include "resource.h"
#include "rs.h"
#include "eudora.h"
#include "utils.h"
#include "text2html.h"
#include "etf2html.h"
#include "fileutil.h"
#include "QCGraphics.h"
#include "trnslate.h"
#include "guiutils.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


char* HeaderContents(UINT HeaderID, const char* message)
{
	CRString HeaderName(HeaderID);
	
	if (!message || !*message)
		return (NULL);

	// Find out if this header exists
	int len = strlen(HeaderName);
	while (1)
	{
		if (!strnicmp(message, HeaderName, len))
			break;
		if (*message == '\r' || *message == '\n')
			return (NULL);
		if (message = strchr(message, '\n'))
		 	message++;
		else
			return (NULL);
	}

	// Find end of header
	const char* beg = message + len;
	while (*beg == ' ' || *beg == '\t')
		beg++;
	const char* end;
	const char* next = beg;
	do
	{
		if (!(end = strchr(next, '\n')))
			return (NULL);
		end--;
		next = end + 2;
		if (*end == '\r')
			end--;
	} while (*next == ' ' || *next == '\t');

	if (beg > end)
		return (NULL);

	// Copy contents in to new buffer, unwrapping lines as we go
	char* contents = new char[end - beg + 2];
	char* p = contents;
	BOOL bAtLineStart = FALSE;
	for (; beg <= end; beg++)
	{
		if (bAtLineStart)
		{
			if (*beg != ' ' && *beg != '\t')
			{
				*p++ = *beg;
				bAtLineStart = FALSE;
			}
		}
		else
		{
			if (*beg == '\n')
			{
				if (beg < end)
					*p++ = ' ';
				bAtLineStart = TRUE;
			}
			else if (*beg != '\r')
				*p++ = *beg;
		}
	}

	*p = 0;
	
	return (contents);
}



// QuoteText
//
char* QuoteText(const char* message, BOOL IsForward, UINT IsRich /*= 0 */, const char* PlainStartText /*= NULL*/)
{
	char* NewMessageBuffer = NULL;

	if (!message)
		return (NULL);

	const BOOL bInterpretFormatFlowed = GetIniShort(IDS_INI_INTERPRET_FORMAT_FLOWED);
	const BOOL bConvertFFToExcerpt = GetIniShort(IDS_INI_CONVERT_FF_TO_EXCERPT);
	const char* Prefix = NULL;
	const char* Start = NULL;
	const char* End = NULL;

	if (IsForward)
	{
		Prefix = GetIniString(IDS_INI_QUOTE_PREFIX);
		Start = GetIniString(IDS_INI_QUOTE_START);
		End = GetIniString(IDS_INI_QUOTE_END);
	}
	else
	{
		Prefix = GetIniString(IDS_INI_REPLY_PREFIX);
		Start = GetIniString(IDS_INI_REPLY_START);
		End = GetIniString(IDS_INI_REPLY_END);
	}
	if (*Prefix)
		Prefix = ">";
	int Plen = ::SafeStrlenMT(Prefix);

	if ((PlainStartText && GetIniShort(IDS_INI_ALWAYS_EXCERPT_RESPONSE)) ||
		(IsRich > IS_FLOWED || (IsRich == IS_FLOWED && (bInterpretFormatFlowed && bConvertFFToExcerpt))))
	{
		CString NewQuotedText;
		CString ExcerptOn;
		CString ExcerptOff;
		CString xHtmlOn;
		CString xHtmlOff;
		CRString xHtml(IDS_MIME_XHTML);
		CString HtmlStartText(Text2Html(PlainStartText, TRUE, FALSE));
		CString HtmlEndText;

		if (*Prefix)
		{
			ExcerptOn.Format("<%s>", (LPCTSTR)CRString(IDS_HTML_BLOCKQUOTE_CITE));
			ExcerptOff.Format("</%s>", (LPCTSTR)CRString(IDS_HTML_BLOCKQUOTE));
		}

		if (*Start)
		{
			HtmlStartText += Text2Html(Start, TRUE, FALSE);
			HtmlStartText += "<br>";
		}

		if (*End)
		{
			if (ExcerptOn.IsEmpty())
				HtmlEndText = "<br>";
			HtmlEndText += Text2Html(End, TRUE, FALSE);
		}

		GetBodyAsHTML(NewQuotedText, message);

		xHtmlOn.Format("<%s>", (LPCTSTR)xHtml);
		xHtmlOff.Format("</%s>", (LPCTSTR)xHtml);

		int NewLen = xHtmlOn.GetLength() + HtmlStartText.GetLength() + ExcerptOn.GetLength() +
						NewQuotedText.GetLength() + ExcerptOff.GetLength() + HtmlEndText.GetLength() +
						xHtmlOff.GetLength();
		NewMessageBuffer = new char[NewLen + 1];
		sprintf(NewMessageBuffer, "%s%s%s%s%s%s%s", (LPCTSTR)xHtmlOn, (LPCTSTR)HtmlStartText, (LPCTSTR)ExcerptOn,
													(LPCTSTR)NewQuotedText, (LPCTSTR)ExcerptOff, (LPCTSTR)HtmlEndText,
													(LPCTSTR)xHtmlOff);

		return NewMessageBuffer;
	}

	if (IsRich == IS_FLOWED && bInterpretFormatFlowed)
	{
		// Flow the text
		NewMessageBuffer = FlowText(message);
	}
	else
	{
		if (GetIniShort(IDS_INI_EXCERPT_FIXED_TEXT))
		{
			// I know, it's bad to cast away const-ness, but we handle the case below and make
			// sure not to delete NewMessageBuffer if it's just been asigned to message.
			NewMessageBuffer = (char*)message;
		}
		else
		{
			// Get a wrapped version of the message as it will look much better when plain quoted
			NewMessageBuffer = WrapText(message);
		}
	}
	const char* NewMessage = NewMessageBuffer;

	// Find out how many lines
	int count = 0;
	const char* line = NewMessageBuffer;
	for (; line; count++)
		if (line = strchr(line, '\n'))
			line++;
	
	char* NewText = new char[::SafeStrlenMT(PlainStartText) + (::SafeStrlenMT(Start) + 2) + (count * (Plen + 1)) +
		::SafeStrlenMT(NewMessageBuffer) + (::SafeStrlenMT(End) + 2) + 1];
	
	*NewText = 0;
	// attribution?
	if (PlainStartText && *PlainStartText)
	{
		strcat(NewText,PlainStartText);
	}
	
	// quote start string?
	if (Start && *Start)
	{
		strcat(NewText, Start);
		strcat(NewText, "\r\n");
	}

	CRString XFlowed(IDS_MIME_XFLOWED);
	CString FlowedOn, FlowedOff;
	FlowedOn.Format( CRString(IDS_MIME_RICH_ON) , (LPCTSTR)XFlowed);
	FlowedOff.Format(CRString(IDS_MIME_RICH_OFF), (LPCTSTR)XFlowed);

	char* p = NewText + strlen(NewText);
	// Copy line by line into p
	while (1)
	{
		if (strncmp(FlowedOn, NewMessage, FlowedOn.GetLength()) == 0)
		{
			NewMessage += FlowedOn.GetLength();
			if (*NewMessage == '\r')
				NewMessage++;
			if (*NewMessage == '\n')
				NewMessage++;
			continue;
		}
		else if (strncmp(FlowedOff, NewMessage, FlowedOff.GetLength()) == 0)
		{
			NewMessage += FlowedOff.GetLength();
			if (*NewMessage == '\r')
				NewMessage++;
			if (*NewMessage == '\n')
				NewMessage++;
			continue;
		}

		strcpy(p, Prefix);
		p += Plen;

		const char* end = strchr(NewMessage, '\n'); 
		if (end)
		{
			strncpy(p, NewMessage, end - NewMessage + 1);
			p += end - NewMessage + 1;
			NewMessage = end + 1;
		}
		if (!end || !*NewMessage)
		{
			strcpy(p, NewMessage);
			if (End && *End)
			{
				if (!end)
					strcat(p, "\r\n");
				strcat(p, End);
			}

			if (NewMessageBuffer != message)
				delete [] NewMessageBuffer;

			return (NewText);
		}
	}
}



// FindBody
// Returns a pointer to the body of a message
//
const char* FindBody(const char* message)
{
	const char* orig = message;
	const char* szHeader;
	const char* szColon;
	const char*	szEnd;

	if (!message || ( *message == '\0' ) )
		return (NULL);

	szHeader = message;

	while( *szHeader )
	{
		szColon = strpbrk( szHeader, ":\r\n" );
		
		if( ( szColon == NULL ) || ( *szColon != ':' ) )
		{
			break;
		}
		
		szHeader = szColon;

		do
		{
			szEnd = strpbrk( szHeader, "\r\n" );
			
			if( szEnd == NULL )
			{
				szEnd = szHeader + strlen( szHeader );
			}
			else
			{
				++szEnd;

				// Yes, it's ugly, but it looks like previous versions of Eudora would
				// write a CR CR LF at the end of a header, so account for that
				while( *szEnd == '\r' )
				{
					szEnd++;
				}
				if( *szEnd == '\n' )
				{
					szEnd++;
				}
			}

			szHeader = szEnd;
		}
		while( ( *szHeader == ' ' ) || ( *szHeader == '\t' ) );	// if the line starts with 
																// a space or tab, it's 
																// still part of this 
																// header
	}

	if( szHeader )
	{
		if( ( *szHeader == '\r' ) || ( *szHeader == '\n' ) )
		{
			szHeader++;
		}

		if( ( *szHeader == '\r' ) || ( *szHeader == '\n' ) )
		{
			szHeader++;
		}
	}

	return szHeader;	
}


// WrapText
//
// Takes text and adds newlines at the word wrap column.
// Returns a pointer to an allocated buffer, so remeber to delete!
//
char* WrapText(const char* text)
{
	if (!text)
		return (NULL);

	int WWColumn = GetIniShort(IDS_INI_WORD_WRAP_COLUMN);
	int WWMax = GetIniShort(IDS_INI_WORD_WRAP_MAX);

	// Find out how many newlines will have to be added (approximately)
	long count = 0L;
	const char* p = text;
	while (*p)
	{
		const char *nl = strchr(p, '\n');
		if (!nl)
			nl = p + strlen(p) - 1;
		if (nl - p > WWColumn)
			count += (nl - p) / WWColumn;
		p = nl + 1;
	}

	long total = 0, length = strlen(text) + count * 2 + 1024;
	char* wrapped = new char[length];

	if (!wrapped)
		return (NULL);

	*wrapped = 0;

	char* w = wrapped;
	while (*text)
	{
		const char *stop = strchr(text, '\n');
		int extra = 0;

		if (!stop)
			stop = text + strlen(text) - 1;
		else
		{
			if (*--stop == '\r')
				stop--;
		}
		count = stop - text + 1;
		if (count > WWMax - 1)
		{
			p = text + WWColumn;
			while (*p != ' ' && *p != '\t' && p > text)
				p--;
			if (p == text)
				count = WWColumn;
			else
				count = p - text + 1;
		}
		else
		{
			if (text[count] == '\r')
			{
				extra++;
				if (text[count + 1] == '\n')
					extra++;
			}
			else if (text[count] == '\n')
				extra++;
		}
		if (total + count + 3 > length)
		{
			long OldSpot = w - wrapped;
			length += 1024;
			char* n = new char[length];
			if (!n)
			{
				delete [] wrapped;
				return (NULL);
			}
			strcpy(n, wrapped);
			delete [] wrapped;
			wrapped = n;
			w = wrapped + OldSpot;
		}
		strncpy(w, text, (int)count);
		w += count;
		strcpy(w, "\r\n");
		w += 2;
		text += count + extra;
	}

	return (wrapped);
}

// UnwrapText
//
// Takes wrapped text and heuristically tries to keep newlines only at the ends
// of paragraphs.  
//
char* UnwrapText(char* text)
{
	char* Beg = text;
	char* t = text;
	char* End;
	char* End2; 

	if (!text)
		return (NULL);

	while (*Beg)
	{
		End2 = End = strchr(Beg, '\n');

		// On last line
		if (!End)
			End2 = End = Beg + strlen(Beg) - 1;
		// Next line is quoted
		else if (End[1] == '>')
			;
		// This line is short
		else if (End - Beg < 40)
        	;
		// This line is blank
		else if (End == Beg || (End == Beg + 1 && Beg[1] == '\r'))
			;
		// Next line is blank
		else if (End[1] == '\r' || End[1] == '\n')
        	;
		// Next line is indented
		else if (End[1] == '\t' || (End[1] && End[1] == ' ' && End[2] == ' '))
			;
		else
		{
			End2 = End - 1;
			while (End2 != Beg &&
				(*End2 == '\r' || *End2 == ' ' || *End2 == '\t'))
				End2--;
			if (*End2 == '.' || *End2 == '?' || *End2 == '!')
            	End2 = End;
            else
				*++End2 = ' ';
        }
		strncpy(t, Beg, End2 - Beg + 1);
		t += End2 - Beg + 1;        
        Beg = End + 1;
	}
    *t = 0;

	return (text);
}


// FlowText
//
// Takes (assumed) format=flowed text and removes newlines to flow lines together.
// Returns an allocated buffer, so be sure to delete returned pointer after done.
//
char* FlowText(const char* Text, int Length /*=-1*/, BOOL ForceFlowed /*= FALSE*/)
{
	if (!Text)
	{
		// Passing in a NULL pointer is probably a coding bug
		ASSERT(0);
		return NULL;
	}

	if (Length < 0)
		Length = strlen(Text);

	char* FlowedText = new char[Length + 1];
	char* Out = FlowedText;
	const char* LineStart = Text;
	BOOL bLastWasFlowed = FALSE;
	int OldQuoteLevel = 0;

	CRString XFlowed(IDS_MIME_XFLOWED);
	CString FlowedOn, FlowedOff;
	FlowedOn.Format( CRString(IDS_MIME_RICH_ON) , (LPCTSTR)XFlowed);
	FlowedOff.Format(CRString(IDS_MIME_RICH_OFF), (LPCTSTR)XFlowed);
	int FlowedLevel = ForceFlowed? 1 : 0;

	while (1)
	{
		if (strncmp(FlowedOn, LineStart, FlowedOn.GetLength()) == 0)
		{
			LineStart += FlowedOn.GetLength();
			if (*LineStart == '\r')
				LineStart++;
			if (*LineStart == '\n')
				LineStart++;
			FlowedLevel++;
			continue;
		}
		else if (FlowedLevel && strncmp(FlowedOff, LineStart, FlowedOff.GetLength()) == 0)
		{
			LineStart += FlowedOff.GetLength();
			if (*LineStart == '\r')
				LineStart++;
			if (*LineStart == '\n')
				LineStart++;
			FlowedLevel--;
			OldQuoteLevel = 0;
			continue;
		}

		int NewQuoteLevel = 0;
		if (FlowedLevel)
		{
			const char* QuoteEnd = LineStart;

			while (*QuoteEnd == '>')
				QuoteEnd++;
			NewQuoteLevel = QuoteEnd - LineStart;

			// Lines that begin with one or more quotes that are immediately followed
			// by "From " are mangled From lines, so remove one level of quotes.
			if (NewQuoteLevel && strncmp(QuoteEnd, "From ", 5) == 0)
				NewQuoteLevel--;

			if (bLastWasFlowed)
			{
				if (NewQuoteLevel == OldQuoteLevel)
				{
					if (*QuoteEnd == ' ')
						QuoteEnd++;
					LineStart = QuoteEnd;
				}
				else
				{
					// Bad format=flowed!  You should never have a flowed line
					// that changes quote level.  Who created this text?
					ASSERT(0);

					*Out++ = '\r';
					*Out++ = '\n';
				}
			}
			OldQuoteLevel = NewQuoteLevel;
		}

		const char* LineEnd = strchr(LineStart, '\r');
		if (!LineEnd)
		{
			strcpy(Out, LineStart);
			break;
		}

		const int NumChars = LineEnd - LineStart;
		strncpy(Out, LineStart, NumChars);
		Out += NumChars;

		bLastWasFlowed = FALSE;
		if (FlowedLevel && NumChars > 0 && LineEnd[-1] == ' ' && (NumChars == 1 || LineEnd[-2] != ' '))
		{
			bLastWasFlowed = TRUE;
			int NonQuoteChars = NumChars;
			if (NewQuoteLevel)
				NonQuoteChars -= NewQuoteLevel + 1;
			if (NonQuoteChars == 3 && LineEnd[-2] == '-' && LineEnd[-3] == '-')
				bLastWasFlowed = FALSE;
		}
		if (!bLastWasFlowed)
		{
			*Out++ = '\r';
			if (LineEnd[1] == '\n')
				*Out++ = '\n';
		}
		LineStart = LineEnd + 1;
		if (*LineStart == '\n')
			LineStart++;
	}

	return FlowedText;
}


// BOG: this probably belongs in QCUtils or something

// IsColorIcon95 - This is a hack that only works for Windows 95. On Windows NT,
// it seems to be impossible to reliably tell a color from Black & White icon.
// This means that CreateJpegFromIcon (QCGraphics.cpp) will generate black
// images from Black & White icons on Windows NT. However, on Windows 95 it
// will crash, so it's still worthwhile to try.

static inline bool IsColorIcon95( HICON hIcon )
{
	bool fRet = false;
	ICONINFO ii;

	if ( GetIconInfo( hIcon, &ii ) ) {
		if ( ii.hbmColor ) {
			fRet = true;
			::DeleteObject( ii.hbmColor );
		}

		::DeleteObject( ii.hbmMask );
	}

	return fRet;
}


// GetAssocIcon - try to get the icon associated with a file. requires full
// path to file. compatible with desktop shell and non-shell systems
// (i.e. winnt 3.x). does not use system default icon when association
// cannot be made, but instead uses our own. handles non-existent files.
//
// Note: the Sane Coders Association (SCA) has certified this code to be one
// hundred percent CString Free(tm).

static HICON GetAssocIcon( const char* fullPath )
{
	assert( fullPath );
	HICON theIcon = 0;

	// first we make sure the file even exists. if not, we provide an
	// icon indicating a broken link.
	struct _stat fileInfo;
	bool caller_is_bozo = (fullPath == NULL);

	if ( caller_is_bozo || (_stat( fullPath, &fileInfo ) == -1) ) {
		theIcon = QCLoadIcon( IDI_ATTACH_NOTFOUND );
	}
	else {
		HICON foundIcon = 0;
		HINSTANCE hInst = AfxGetInstanceHandle();
		
		char pathCopy[_MAX_PATH];
		strcpy( pathCopy, fullPath );

		if ( IsVersion4() ) {

			// we use an index of one because otherwise the shell will create
			// thumbnails for image files, which look like crap.

			WORD icoIndex = 1;
			foundIcon = ExtractAssociatedIcon( hInst, pathCopy, &icoIndex );
		}
		else {
			char buf[_MAX_PATH];
			HINSTANCE hInstAssoc = FindExecutable( pathCopy, NULL, buf );
			
			if ( hInstAssoc > (HINSTANCE)32 )
				foundIcon = ExtractIcon( hInst, buf, 0 );

			// if theIcon is nil, we have an exe without an icon; if it is 1,
			// then we have a file that is not an exe, and has no association.
			// setting 'pathCopy' to zero forces the use of our "default"

			if ( !foundIcon || (foundIcon == (HICON)1) )
				*pathCopy = '\0';
		}

		// BPO - CreateJpegFromIcon (QCGraphics.cpp) does not handle
		// Black & White icons, so for that case we use our own default icon.
		
		if ( (stricmp( pathCopy, fullPath  ) == 0) &&
				IsColorIcon95( foundIcon ) ) {

			theIcon = foundIcon;
		}
		else {
			theIcon = QCLoadIcon( IDI_ATTACH_DEFAULT );
		}
	}

	assert( theIcon );
	return theIcon;
}


CString IconToFile(HICON hIcon)
{
	// create a temp filename - the bmp equiv to the hicon lives for
	// the duration of this QCMessage object
    char FinalFname[_MAX_PATH+1];
    char DirName[_MAX_PATH+1];
	CString iconDir;
	CString Path;

	WIN32_FIND_DATA wfd;
	iconDir = EudoraDir + CRString(IDS_ICON_FOLDER); 
	iconDir.MakeLower();
	strcpy(DirName, iconDir);

	// see if we need to create an "icon" directory
	HANDLE findHandle;
	findHandle = FindFirstFile((const char *)iconDir,&wfd);
	if ( findHandle == INVALID_HANDLE_VALUE)
	{
		// if we can't make the directory, reset it to to old way
		if (mkdir(iconDir) < 0)
			if (errno != EACCES)
				strcpy(DirName, EudoraDir);
	}
	FindClose(findHandle);

	GetTmpFile( DirName, "jpg", FinalFname );

	CreateJpegFromIcon( hIcon, FinalFname, GetSysColor(COLOR_WINDOW) );

	Path = FinalFname;

	return Path;
}

UINT GetTmpFile(LPCTSTR lpPathName, LPCTSTR lpExtension, LPTSTR lpTempName)
{
	WIN32_FIND_DATA wfd;
	HANDLE	findHandle;
	BOOL	unique = FALSE;

	while ( ! unique )
	{
		DWORD	count = ::GetTickCount();
		char	szCount[ 20 ];

		strcpy( lpTempName, lpPathName );
		if ( lpTempName[ strlen( lpTempName ) - 1 ] != '\\' )
			strcat( lpTempName, "\\");
		ltoa( count, szCount, 16 );
		strncat( lpTempName, szCount, 8 );
		strcat( lpTempName, "." );
		strcat( lpTempName, lpExtension );

		findHandle = FindFirstFile((const char *)lpTempName,&wfd);
		if ( findHandle == INVALID_HANDLE_VALUE)
			unique = TRUE;

		FindClose(findHandle);
	}

	return 1;
}

void AddToBody(CString & Body, const char* Part, int Length, int PartType)
{
	// Yes, casting away const'ness is normally a bad thing to do, but in this case we
	// really are treating it as const because we're restoring the character, and it
	// gives a big performance win because we don't have to make a new allocation
	if ( Part == NULL || Length == 0 )
		return;

	char SaveChar = Part[Length];
	((char*)Part)[Length] = 0;

	switch ( PartType )
	{
	case IS_ASCII :
		Body += Text2Html(Part, TRUE, FALSE, GetIniShort(IDS_INI_EXCERPT_FIXED_TEXT));
		break;

	case IS_FLOWED :
		if (!GetIniShort(IDS_INI_INTERPRET_FORMAT_FLOWED))
			Body += Text2Html(Part, TRUE, FALSE, GetIniShort(IDS_INI_CONVERT_FF_TO_EXCERPT));
		else
		{
			char* Flowed = FlowText(Part, Length, TRUE);

			Body += Text2Html(Flowed, TRUE, FALSE, GetIniShort(IDS_INI_CONVERT_FF_TO_EXCERPT));

			delete [] Flowed;
		}
		break;

	case IS_RICH :
		if (Part && *Part)
		{
			int size = Etf2Html_Measure( Part );
			char * buf = new char[ size + 1 ];
			if ( buf )
			{
				Etf2Html_Convert( buf, Part );
				Body += buf;
				delete [] buf;
			}
		}
		break;

	case IS_HTML :
		if (GetIniShort(IDS_INI_RUN_HTML_CODE))
		{
			Body += Part;
		}
		else
		{
			// Strip out stuff that could be potentially dangerous.
			// This means <script>, <applet>, <object>, and <embed> code.
			LPCTSTR NonCodeStart = Part;
			LPCTSTR TagStart = NonCodeStart;
			int ScriptLevel = 0;
			int AppletLevel = 0;
			int ObjectLevel = 0;
			int EmbedLevel = 0;
#define NotInCode() (!ScriptLevel && !AppletLevel && !ObjectLevel && !EmbedLevel)

			while (TagStart = strchr(TagStart, '<'))
			{
				// <script>
				if (strnicmp(TagStart, "<script", 7) == 0)
				{
					if (NotInCode() && TagStart != NonCodeStart)
					{
						// Copy over safe content
						*(LPTSTR)TagStart = 0;
						Body += NonCodeStart;
						*(LPTSTR)TagStart = '<';
					}
					ScriptLevel++;
					TagStart += 7;
				}
				else if (strnicmp(TagStart, "</script", 8) == 0)
				{
					TagStart = strchr(TagStart, '>');
					if (!TagStart)
						break;
					TagStart++;
					if (--ScriptLevel == 0)
						NonCodeStart = TagStart;
					else if (ScriptLevel < 0)
					{
						// Found a </script> before a <script>
						ASSERT(0);
						ScriptLevel = 0;
					}
				}

				// <applet>
				else if (strnicmp(TagStart, "<applet", 7) == 0)
				{
					if (NotInCode() && TagStart != NonCodeStart)
					{
						// Copy over safe content
						*(LPTSTR)TagStart = 0;
						Body += NonCodeStart;
						*(LPTSTR)TagStart = '<';
					}
					AppletLevel++;
					TagStart += 7;
				}
				else if (strnicmp(TagStart, "</applet", 8) == 0)
				{
					TagStart = strchr(TagStart, '>');
					if (!TagStart)
						break;
					TagStart++;
					if (--AppletLevel == 0)
						NonCodeStart = TagStart;
					else if (AppletLevel < 0)
					{
						// Found a </applet> before a <applet>
						ASSERT(0);
						AppletLevel = 0;
					}
				}

				// <object>
				else if (strnicmp(TagStart, "<object", 7) == 0)
				{
					if (NotInCode() && TagStart != NonCodeStart)
					{
						// Copy over safe content
						*(LPTSTR)TagStart = 0;
						Body += NonCodeStart;
						*(LPTSTR)TagStart = '<';
					}
					ObjectLevel++;
					TagStart += 7;
				}
				else if (strnicmp(TagStart, "</object", 8) == 0)
				{
					TagStart = strchr(TagStart, '>');
					if (!TagStart)
						break;
					TagStart++;
					if (--ObjectLevel == 0)
						NonCodeStart = TagStart;
					else if (ObjectLevel < 0)
					{
						// Found a </object> before a <object>
						ASSERT(0);
						ObjectLevel = 0;
					}
				}

				// <embed>
				else if (strnicmp(TagStart, "<embed", 6) == 0)
				{
					if (NotInCode() && TagStart != NonCodeStart)
					{
						// Copy over safe content
						*(LPTSTR)TagStart = 0;
						Body += NonCodeStart;
						*(LPTSTR)TagStart = '<';
					}
					EmbedLevel++;
					TagStart += 6;
				}
				else if (strnicmp(TagStart, "</embed", 7) == 0)
				{
					TagStart = strchr(TagStart, '>');
					if (!TagStart)
						break;
					TagStart++;
					if (--EmbedLevel == 0)
						NonCodeStart = TagStart;
					else if (EmbedLevel < 0)
					{
						// Found a </embed> before a <embed>
						ASSERT(0);
						EmbedLevel = 0;
					}
				}

				// <iframe>
				// Content between <iframe> and </iframe> tags are to be shown
				else if (strnicmp(TagStart, "<iframe", 7) == 0 || strnicmp(TagStart, "</iframe", 8) == 0)
				{
					if (NotInCode() && TagStart != NonCodeStart)
					{
						// Copy over safe content
						*(LPTSTR)TagStart = 0;
						Body += NonCodeStart;
						*(LPTSTR)TagStart = '<';
					}
					if (!(TagStart = strchr(TagStart, '>')))
						break;
					NonCodeStart = ++TagStart;
				}
				else
					TagStart++;
			}
			if (NotInCode())
				Body += NonCodeStart;
		}
		break;
	}

	((char*)Part)[Length] = SaveChar;
}

void GetBodyAsHTML(CString& Body, LPCTSTR Text)
{
	const char*	pStartLine = Text;
	int			PartType = IS_ASCII;
	CString		szPath;
	LONG		lLen;
	CString		szBase;
	INT			iPos;
	CString		szTrans;
	const char* pEndLine;
	BOOL		bShowAttachmentIcons;

	// now process the body removing "Attachment converted" and
	// "Embedded content" lines as well as converting things to HTML
	CRString ACTrailer( IDS_ATTACH_CONVERTED );
	CRString ECTrailer( IDS_EMBEDDED_CONTENT );
	CRString TFTrailer( IDS_TRANS_FILE_TAG );
	CRString RichTag( IDS_MIME_XRICH );
	CRString HTMLTag( IDS_MIME_XHTML );
	CRString HTML2Tag( IDS_MIME_HTML );
	CRString FlowedTag( IDS_MIME_XFLOWED );
	bool bHtml1 = false;
	bool bHtml1StartTag = false;
	const bool bHtmlInPlainText = GetIniShort(IDS_INI_HTML_IN_PLAIN_TEXT) != 0;

	CString  RichOn;
	CString  RichOff;
	CString  HtmlOn;
	CString  HtmlOff;
	CString	 Html2On;
	CString  Html2Off;
	CString  FlowedOn;
	CString  FlowedOff;

	CString  XRichExcerptOn = "<excerpt>";
	CString  XRichExcerptOff = "</excerpt>";
	int		 XRichExcerptOnLEN = 9;
	int		 XRichExcerptOffLEN = 10;
	int		 inXRichExcerpt = 0;
	const char* excerptPtr;
	char	*pHtml1, *pHtml2;

	RichOn.Format( "<%s>", ( const char* )RichTag );
	RichOff.Format( "</%s>", ( const char* )RichTag );
	HtmlOn.Format( "<%s", ( const char* )HTMLTag );	// can be <x-html content-base>
	HtmlOff.Format( "</%s>", ( const char* )HTMLTag );
	Html2On.Format( "<%s", ( const char* )HTML2Tag );	
	Html2Off.Format( "</%s>", ( const char* )HTML2Tag );
	FlowedOn.Format( "<%s>", ( const char* )FlowedTag );
	FlowedOff.Format( "</%s>", ( const char* )FlowedTag );

	CString  HtmlOnUpper(HtmlOn);
	CString  HtmlOffUpper(HtmlOff);
	CString	 Html2OnUpper(Html2On);
	CString  Html2OffUpper(Html2Off);

	//Get the tags in uppercase
	HtmlOnUpper.MakeUpper();
	HtmlOffUpper.MakeUpper();
	Html2OnUpper.MakeUpper();
	Html2OffUpper.MakeUpper();

	Body.Empty();

	CString Part;
	const char* PartStart = NULL;
	
	bShowAttachmentIcons = ( GetIniShort( IDS_INI_SHOW_ATTACHMENT_ICONS ) ? TRUE : FALSE );
	
	while (*pStartLine)
	{
		pEndLine = strchr( pStartLine, '\r' );

//FORNOW		//
//FORNOW		// If we ever decide to change our policy on sending HTML
//FORNOW		// "attachments" as inline data in the message body, then we'll
//FORNOW		// need the following code instead of the one-line strchr()
//FORNOW		// call above.  WKS 97.11.16
//FORNOW		//
//FORNOW		{
//FORNOW			int nEndLineIndex = strcspn( pStartLine, "\r\n" );
//FORNOW			if (nEndLineIndex)
//FORNOW				pEndLine = pStartLine + nEndLineIndex;
//FORNOW			else if (*pStartLine == '\r' || *pStartLine == '\n')
//FORNOW				pEndLine = pStartLine;
//FORNOW			else
//FORNOW				pEndLine = NULL;
//FORNOW		}

		if( pEndLine == NULL )
		{
			lLen = strlen( pStartLine );
			pEndLine = pStartLine + lLen;
		}
		else
		{
			lLen = pEndLine - pStartLine;
			// prepare for the next line -  skip '\r\n'
			if ( *pEndLine == '\r' || *pEndLine == '\n' ) pEndLine++;
			if ( *pEndLine == '\r' || *pEndLine == '\n' ) pEndLine++;
		}

		//Need to check if we are within excerpted text for etf msgs, since AC lines
		//should not be processed in such a case. Also the closing excerpt tags don't 
		//necessarily occur at the beginning of the line, so search thru the current line.
		if (PartType == IS_RICH) 
		{
			excerptPtr = pStartLine;
			while ((excerptPtr = strstr(excerptPtr, (LPCTSTR)XRichExcerptOn)) && (excerptPtr < pEndLine))
			{
				++inXRichExcerpt;
				excerptPtr += XRichExcerptOnLEN;
			}

			excerptPtr = pStartLine;
			while ((excerptPtr = strstr(excerptPtr, (LPCTSTR)XRichExcerptOff)) && (excerptPtr < pEndLine))
			{
				--inXRichExcerpt;
				excerptPtr += XRichExcerptOffLEN;
			}
		}

		if (PartType == IS_ASCII && strnicmp( ACTrailer, pStartLine, ACTrailer.GetLength()) == 0)
		{
			// convert and add any existing body part
			AddToBody( Body, PartStart, pStartLine - PartStart, PartType );
			PartStart = NULL;

			// morph the Attachment converted line into HTML
			szPath = CString( pStartLine + ACTrailer.GetLength(), lLen - ACTrailer.GetLength() );

			szPath.TrimLeft();
			szPath.TrimRight();

			// trim ending and starting quotes
			if( ( szPath.IsEmpty() == FALSE ) && ( szPath[ szPath.GetLength() - 1 ] == '"' ) )
			{
				szPath.ReleaseBuffer( szPath.GetLength() - 1 );
			}

			if( ( szPath.IsEmpty() == FALSE ) && ( szPath[ 0 ] == '"' ) )
			{
				szPath = szPath.Right( szPath.GetLength() - 1 );
			}

			const char* Filename = strrchr(szPath, '\\');
			if (Filename && !FileExistsMT(szPath))
			{
				char NewLocation[MAX_PATH + 1];
				const char* AutoReceiveDir = GetIniString(IDS_INI_AUTO_RECEIVE_DIR);

				if (*AutoReceiveDir)
					sprintf(NewLocation, "%s%s", AutoReceiveDir, Filename);

				if (!*AutoReceiveDir || !FileExistsMT(NewLocation))
					sprintf(NewLocation, "%s%s%s", (LPCTSTR)EudoraDir, (LPCTSTR)CRString(IDS_ATTACH_FOLDER), Filename);

				if (FileExistsMT(NewLocation))
					szPath = NewLocation;
			}

			CString szHREF = EscapeURL( CString("file://") + szPath );

			if( bShowAttachmentIcons )
			{
				// szPath now points at the path
				// get the basename
				iPos = szPath.ReverseFind( '\\' );
					
				if( iPos >= 0 )
				{
					szBase = szPath.Right( szPath.GetLength() - iPos - 1 );
				}
				else
				{
					szBase = szPath;
				}

				// Get the associated icon
				HICON hIcon = GetAssocIcon( szPath );
				CString IconPath = IconToFile( hIcon );

				// dynamic HTML here we come...
				Body += "<div>\r\n<a href=\"";
				Body += szHREF;
				Body += "\" EUDORA=ATTACHMENT>";
				Body += "\r\n<img src=\"file://";
				Body += IconPath;
				Body += "\" alt=\"";
				Body += szPath;
				Body += "\"> ";
				Body += szBase;
				Body += " </a>\r\n</div>\r\n";
			}
			else
			{
				// make it an anchor with no icon
				Body += "<div>\r\n<a href=\"";
				Body += szHREF;
				Body += "\" EUDORA=ATTACHMENT>";
				Body += CString( pStartLine, lLen );
				Body += "</a>";
			}
		}
		else if (PartType == IS_ASCII && strnicmp(ECTrailer, pStartLine, ECTrailer.GetLength()) == 0)
		{
			// embedded content lines get dropped - they should already
			// have a reference in the HTML
			// convert and add any existing body part
			AddToBody( Body, PartStart, pStartLine - PartStart, PartType );
			PartStart = NULL;
		}
		else if (PartType == IS_ASCII && strnicmp(TFTrailer, pStartLine, TFTrailer.GetLength()) == 0)
		{
			// convert and add any existing body part
			AddToBody( Body, PartStart, pStartLine - PartStart, PartType );
			PartStart = NULL;

			// morph the Run Plugin: line into HTML
			// The format is "[path] <pluginID.TransID>"
			const char *runPluginLn = pStartLine + TFTrailer.GetLength();
	
			// Get Rid of 1st quote
			if (*runPluginLn == '"')
				runPluginLn++;

			// this will find the end of the path
			char *transTag = strrchr( runPluginLn, '<' );
			if ( !transTag )
			{
				Body += CString( pStartLine, lLen );
				pStartLine = pEndLine;
				continue;
			}
			
			// Paradigm shift to CString, and don't mess up the original...
			*transTag = 0;
			szPath = runPluginLn;
			szPath.TrimRight();
			*transTag = '<';
		
			// szPath now points at the path
			// get the basename
			iPos = szPath.ReverseFind( '\\' );
			
			if( iPos >= 0 )
			{
				szBase = szPath.Right( szPath.GetLength() - iPos - 1 );
			}
			else
			{
				szBase = szPath;
			}

			int pluginID = 0, translatorID = 0;
			if ( 2 != sscanf( transTag, "<%04ld.%04ld>", &pluginID, &translatorID))
			{
				Body += CString( pStartLine, lLen );
				pStartLine = pEndLine;
				continue;
			}

			// Another paradigm shift...
			szTrans.Format("<%04ld.%04ld>",pluginID, translatorID); 

			// extract the icon from the plugin
			HICON hIcon = NULL;
			CString IconPath;
			CString caption;
			CEudoraApp* pTheApp = (CEudoraApp*)AfxGetApp();
			CTranslator * theTranslator = 
				pTheApp->GetTranslators()->GetTranslator( pluginID, translatorID);
			if ( theTranslator )
			{
				theTranslator->GetIcon( hIcon );
				IconPath = IconToFile( hIcon );
				theTranslator->getDescription( caption );
			}

			// dynamic HTML here we come...
			Body += "<div>\r\n<a href=\"file://";
			Body += szPath;
			Body += ' ';
			Body += szTrans;
			Body += "\" EUDORA=PLUGIN";//-";
			Body += " >";
			Body += "\r\n<img src=\"file://";
			Body += IconPath;
			Body += "\" alt=\"";
			Body += szPath;
			Body += "\"> ";
			Body += szBase;
			Body += " </a>\r\n</div>\r\n";
		}
		else if ( strnicmp( RichOn, pStartLine, RichOn.GetLength() ) == 0 )		// <x-rich>
		{
			AddToBody( Body, PartStart, pStartLine - PartStart, PartType );
			PartStart = NULL;

			// <x-rich> And the message all comes on the same line
			PartType = IS_RICH;
			pEndLine = pStartLine + RichOn.GetLength();
			PartStart = pEndLine;
		}
		else if ( strnicmp( RichOff, pStartLine, RichOff.GetLength() ) == 0 )		// </x-rich>
		{
			// convert and add any existing body part
			AddToBody( Body, PartStart, pStartLine - PartStart, PartType );
			PartStart = NULL;
			PartType = IS_ASCII;

			// </x-rich> And the message all comes on the same line
			pEndLine = pStartLine + RichOff.GetLength();
		}
		else if ( strnicmp( FlowedOn, pStartLine, FlowedOn.GetLength() ) == 0 )		// <x-flowed>
		{
			AddToBody( Body, PartStart, pStartLine - PartStart, PartType );
			PartStart = NULL;

			// <x-flowed> And the message all comes on the same line
			PartType = IS_FLOWED;
			pEndLine = pStartLine + FlowedOn.GetLength();
			PartStart = pEndLine;
		}
		else if ( strnicmp( FlowedOff, pStartLine, FlowedOff.GetLength() ) == 0 )	// </x-flowed>
		{
			// convert and add any existing body part
			AddToBody( Body, PartStart, pStartLine - PartStart, PartType );
			PartStart = NULL;
			PartType = IS_ASCII;

			// </x-flowed> And the message all comes on the same line
			pEndLine = pStartLine + FlowedOff.GetLength();
		}
		else if ( (bHtml1 = (strnicmp( HtmlOn,	pStartLine, HtmlOn.GetLength()	)) == 0 ) ||		// <x-html>
							(bHtmlInPlainText && strnicmp( Html2On, pStartLine, Html2On.GetLength() ) == 0 ))		// <html>
		{
			AddToBody( Body, PartStart, pStartLine - PartStart, PartType );
			PartStart = NULL;
			PartType = IS_HTML;

			// <x-html> And the message all comes on the same line
			if ( bHtml1 )
			{
				bHtml1StartTag = true;
				pEndLine = pStartLine + HtmlOn.GetLength();
			}
			else
			{
				pEndLine = pStartLine + Html2On.GetLength();
			}

			while( ( *pEndLine != '>' ) && ( *pEndLine != '\r' ) && ( *pEndLine != '\n' ) && ( *pEndLine != '\0' ) )
			{
				pEndLine++;
			}

			if( *pEndLine == '>' ) 
			{
				pEndLine++;
			}
			PartStart = pEndLine;
		}
		else 
		{
			pHtml1 = pHtml2 = NULL;

			//Set the pEnd char to NULL so we don't search beyond the current line
			//Remember to set it back again after the search is done in each case
			//Either this or check for the substring and check against pEndLine to
			//make sure we are dealing with the same line
			//Temporarily cast away the constness to do this, bad but it saves sone processing time.
			char temp = *pEndLine;
			*((char*)pEndLine) = '\0';

			if ( PartType == IS_HTML &&
					(((pHtml1 = strstr( pStartLine, HtmlOff )) || (pHtml1 = strstr( pStartLine, HtmlOffUpper )) ) ||	// </x-html>
					( (pHtml2 = strstr( pStartLine, Html2Off )) || (pHtml2 = strstr( pStartLine, Html2OffUpper )) ) ) )	// </html>
			{
				//Restore the char at the end of the line
				*((char *)pEndLine) = temp;

				// convert and add any existing body part
				if (pHtml1)
				{
					if (!bHtml1StartTag)
					{
						// Uh oh, found </x-html> without a starting <x-html>
						ASSERT(0);
					}
					else
					{
						AddToBody( Body, PartStart, pHtml1 - PartStart, PartType );
						PartStart = NULL;
						PartType = IS_ASCII;
						bHtml1StartTag = false;
					}
				}
				else
				{
					// Let's wait for the ending </x-html> tag if we started with <x-html>
					if (!bHtml1StartTag)
					{
						AddToBody( Body, PartStart, pHtml2 - PartStart, PartType );
						PartStart = NULL;
						PartType = IS_ASCII;
					}
				}

				// </x-html> And the message all comes on the same line 
				if ( pHtml1 )
					pEndLine = pHtml1 + HtmlOff.GetLength();
				else
					pEndLine = pHtml2 + Html2Off.GetLength();

				if (*pEndLine == '\r')
					pEndLine++;
				if (*pEndLine == '\n')
					pEndLine++;
			}
			else
			{
				//Restore the char at the end of the line
				*((char*)pEndLine) = temp;

				// must be PLAIN
				if ( ! PartStart )
					PartStart = pStartLine;
			}
		}

		pStartLine = pEndLine;
	}

	// convert and add any remaining body part
	AddToBody( Body, PartStart, pEndLine - PartStart, PartType );
}


BOOL GetMSHTMLVersion( 
INT* pMajorVersion,
INT* pMinorVersion)
{
	DWORD				dwHandle;
	DWORD				dwSize;
	CString				szDllPathname;
	char				szString[1024];
	LPSTR				pData;
	UINT				uBufSize;
	VS_FIXEDFILEINFO*	pFileInfo;
		
	//
	// Do setup to prepare for the incoming version data.
	//
	
	*pMajorVersion = -1;
	*pMinorVersion = -1;

	szString[1023] = '\0';

	if( GetSystemDirectory( szString, 1023 ) )
	{
		szDllPathname = szString;
		szDllPathname += "\\mshtml.dll";

		dwHandle = 0;
		
		dwSize = ::GetFileVersionInfoSize( (char *)(const char *) szDllPathname, &dwHandle);
		
		pData = NULL;
		pFileInfo = NULL;
		uBufSize = 0;

		//
		// Allocate data buffer of the proper size.
		//

		if( dwSize && ( ( pData = new char[ dwSize ] ) != NULL ) )
		{
			//
			// Now, fetch the actual VERSIONINFO data.
			//
			if( ::GetFileVersionInfo((char *)(const char *) szDllPathname, dwHandle, dwSize, pData ) ) 
			{

				//
				// Okay, you would think that passing a raw string literal to an
				// argument that takes a string pointer would be okay.  Well, not
				// if you're running the 16-bit VerQueryValue() function under
				// Windows 3.1/32s.  It GPFs if you pass it a string literal.  So,
				// the workaround is to copy the string to a local buffer first,
				// then pass the string in the buffer.  This, for some inexplicable
				// reason, works under all Windows OSes.
				//
				strcpy( szString, "\\");
				
				if( ::VerQueryValue( pData, szString, (void **) &pFileInfo, &uBufSize) &&
					uBufSize &&
					pFileInfo )
				{
					// now check the version
					*pMajorVersion = ( ( pFileInfo->dwFileVersionMS ) >> 16 );
					*pMinorVersion = ( ( pFileInfo->dwFileVersionMS ) & 0xFFFF );
				}
			}

			delete [] pData;
		}
	}
			
	if( ( *pMajorVersion < 4 ) || 
		( ( *pMajorVersion == 4 ) && ( *pMinorVersion < 70 ) ) )
	{
		return FALSE;
	}

	return TRUE;
}


void ConvertNBSPsToSpaces( char* szText )
{
	INT	i;

	for ( i = strlen( szText) - 1; i >= 0; i-- )
	{
		if( ( UCHAR )( szText[ i ] ) == 0xa0 )
		{
			szText[ i ] = ' ';
		}
	}
}

CString StripNonPreviewHeaders(const char* Message)
{
	CUIntArray	HeaderStartArray;
	const char* PreviewHeadersList = GetIniString(IDS_INI_PREVIEW_HEADERS);
	CString PreviewHeaders;
	const UINT MaxLinesPerHeader = GetIniShort(IDS_INI_PREVIEW_HEADERS_MAX_LINES);

	if (!Message || !PreviewHeadersList)
		return PreviewHeaders;

	// Build up list that contains start of each header in the PreviewHeaders list
	const char* CurrentHeader = PreviewHeadersList;
	while (1)
	{
		HeaderStartArray.Add(CurrentHeader - PreviewHeadersList);
		const char* Comma = strchr(CurrentHeader, ',');
		if (!Comma)
		{
			// Add end of list marker for calculating string lengths below
			HeaderStartArray.Add(CurrentHeader - PreviewHeadersList + strlen(CurrentHeader));
			break;
		}
		CurrentHeader = Comma + 1;
	}

	PreviewHeaders.GetBuffer(256);	// Trick to prevent lots of little buffer allocations
	int PreviewHeadersListSize = HeaderStartArray.GetSize();
	const char* EndHeader = Message;
	for (; *Message && *Message != '\r'; Message = EndHeader + 1)
	{
		BOOL bIncludeIt = FALSE;
		const char* Colon = strchr(Message, ':');
		if (!Colon)
			break;

		if (MaxLinesPerHeader > 0)
		{
			int HeaderLen = Colon - Message + 1;
			for (int i = 0; i < PreviewHeadersListSize - 1; i++)
			{
				int PreviewHeaderLen = HeaderStartArray[i + 1] - HeaderStartArray[i] - 1;
				if (PreviewHeaderLen <= HeaderLen)
				{
					if (strnicmp(Message, PreviewHeadersList + HeaderStartArray[i], PreviewHeaderLen) == 0)
					{
						bIncludeIt = TRUE;
						break;
					}
				}
			}
		}

		// Collect at most MaxLinesPerHeader lines of the header
		EndHeader = Colon;
		UINT LinesLeftToInclude = MaxLinesPerHeader;
		const char* EndInclude = EndHeader;
		do
		{
			EndHeader = strchr(EndHeader + 1, '\n');
			if (!EndHeader)
				EndHeader = Colon + strlen(Colon) - 1;
			if (LinesLeftToInclude > 0)
			{
				LinesLeftToInclude--;
				EndInclude = EndHeader;
			}
		} while (EndHeader[1] == ' ' || EndHeader[1] == '\t');

		if (bIncludeIt)
		{
			// Hack to prevent extra memory allocation
			char SaveChar = EndInclude[1];
			((char*)EndInclude)[1] = 0;
			PreviewHeaders += Message;
			((char*)EndInclude)[1] = SaveChar;

			// Ensure ending newline
			if (*EndInclude != '\n')
				PreviewHeaders += "\r\n";
		}

		Message = EndHeader + 1;
	}

	return PreviewHeaders;
}
