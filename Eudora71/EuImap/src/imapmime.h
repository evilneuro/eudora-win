// IMAPMIME.H
//
// 
//

#ifndef IMAPMIME
#define IMAPMIME


// FUNCTION Prototypes.
JJFileMT* OpenLocalAttachFile(char *pParentDir, char* Name, BOOL bCreateStub);
BOOL GetUniqueDir (LPCSTR pParentDir, LPCSTR pSuggestedName, CString &NewName, short MaxNameLength, BOOL bCreate);

#endif
