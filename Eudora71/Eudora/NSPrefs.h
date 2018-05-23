
#ifndef _NSPREFS_H_
#define _NSPREFS_H_

BOOL LocateNetscapePrefsFile(CStringList& FileList);
CStringList* GetFilesInDirectory(const CString& SearchString, const BOOL bRecurseSubDirs /* = FALSE */, CStringList *pStringList /* = NULL */);
BOOL UpdateNetscapePrefsFile(BOOL bAddEudoraAsDefaultEmailer = TRUE);
BOOL CheckIfNetscapePrefsFileUpdated();

#endif
