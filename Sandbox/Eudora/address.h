// ADDRESS.H
//
// Utility routines for gleaning information from email address headers
// 


#ifndef _ADDRESS_H_
#define _ADDRESS_H_

// Data Items

extern CString ReturnAddress;


// Function Prototypes
const char* FindAddressEnd(const char* AddressStart, BOOL bDisplayError = TRUE, BOOL bGroupSyntax = FALSE);
inline char* FindAddressEnd(char* AddressStart, BOOL bDisplayError = TRUE, BOOL bGroupSyntax = FALSE)
	{ return ((char*)FindAddressEnd((const char*)AddressStart, bDisplayError, bGroupSyntax)); }
char* GetRealName(char* FromLine);
char* StripAddress(char* line);
BOOL StripMe(char* line, BOOL StripAll = TRUE);
char* QualifyAddresses(char* Addresses);
const char* GetReturnAddress();
const char* FormatAddress(CString& Result, const char* RealName, const char* Address);

#endif
