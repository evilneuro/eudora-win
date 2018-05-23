// MSGUTILS.H
//
// Helper routines for messages
//

char* HeaderContents(UINT HeaderID, const char* Message);
char* QuoteText(const char* message, BOOL IsForward, UINT IsRich = 0, const char* PlainStartText = NULL, bool bEnsureHTML = true, bool bSingle = true);
const char* FindBody(const char* message);
char* WrapText(const char* text);
char* UnwrapText(char* text);
char* FlowText(const char* Text, int Length = -1, BOOL ForceFlowed = FALSE);
UINT GetTmpFile(LPCTSTR lpPathName, LPCTSTR lpExtension, LPTSTR lpTempName);

void	AddToBody(
				CString &			Body,
				const char *		Part,
				int					Length,
				int					PartType,
				bool				bAlwaysStripHTMLCode,
				bool				bStripDocumentLevelTags,
				bool				bRelaxLocalFileRefStripping,
				bool				bConvertEmoticonTriggersToImageTags = true);

void	AddAttachmentToBody(
				CString &			szBody,
				CString &			szPath,
				const CString &		szNoIconPrefix);

void	GetBodyAsHTML(
				CString &			Body,
				LPCTSTR				Text,
				bool				bAlwaysStripHTMLCode = false,
				bool				bStripDocumentLevelTags = false,
				bool				bRelaxLocalFileRefStripping = false,
				bool				bConvertEmoticonTriggersToImageTags = true);
void ConvertNBSPsToSpaces( char* szText );
BOOL StripAttachFromBody(char* message, UINT tag);
CString StripNonPreviewHeaders(const char* Message);
BOOL FindAttachment(CString &csAttachment);
int		GetAttachments(
				const char *		in_szMessage,
				CString &			out_szAttachments);
