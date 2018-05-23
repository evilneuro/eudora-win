// MSGUTILS.H
//
// Helper routines for messages
//

char* HeaderContents(UINT HeaderID, const char* Message);
char* QuoteText(const char* message, BOOL IsForward, UINT IsRich = 0, const char* PlainStartText = NULL);
const char* FindBody(const char* message);
char* WrapText(const char* text);
char* UnwrapText(char* text);
char* FlowText(const char* Text, int Length = -1, BOOL ForceFlowed = FALSE);
UINT GetTmpFile(LPCTSTR lpPathName, LPCTSTR lpExtension, LPTSTR lpTempName);
void GetBodyAsHTML(CString& Body, LPCTSTR Text);
BOOL GetMSHTMLVersion( INT* pMajorVersion, INT* pMinorVersion);
void ConvertNBSPsToSpaces( char* szText );
CString StripNonPreviewHeaders(const char* Message);
