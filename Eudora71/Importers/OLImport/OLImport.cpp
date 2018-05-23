// OLImport.cpp : Defines the entry point for the DLL application.

#include "stdafx.h"
#include "OLImport.h"
#include "OLImportClass.h"

OLImportClass olImport;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{

	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
	{

		//initialize
		olImport.LoadModuleHandle(hModule);
		
	}
	else
	if(ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		//uninit
	}
    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// Exported LoadProviders
// return all providers, when empty return null
/////////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) CImportProvider* LoadProvider(bool bDisplayErrDialog)
{	

	olImport.LoadOLProvider(bDisplayErrDialog);

	if(olImport.GetNumProviderChildren() > 0)
		return olImport.GetProvider();
	else
		return NULL;
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Exported WHOAMI, Tells who we are speaking with.
/////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) char* QueryImportProvider()
{
	char *szImportProvider = "Microsoft Outlook";
	return szImportProvider;
}

/////////////////////////////////////////////////////////////////////////////
// Exported - returns the list of message stores and
// the folder hierarchy for each of the message stores found
/////////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) CMessageStore* LoadFolders(char *lpszFolderPath, char *lpszProfileName)
{
	return olImport.LoadFolders(lpszFolderPath, lpszProfileName);
}

/////////////////////////////////////////////////////////////////////////////
// Exported - Makes a mime file for the required folder
/////////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) char* MakeMimeDigest(char *lpszPath, void * pID, unsigned int iSize, void *pDB)
{

	if(lpszPath)
	{
		if (strlen(lpszPath) < 1)
		{
			return NULL;
		}

		return olImport.MakeMimeWithFile(lpszPath, pID, iSize, pDB);
	
	}
	else
		return NULL;

	return NULL;

}

/////////////////////////////////////////////////////////////////////////////
// Exported - returns the list of address books as well as the details of the 
// messaging users and distribution lists under each of the address books
/////////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) CAddressBookTree* GetAddressTree(char *lpszPath, char * lpszProfileName)
{
	return olImport.GetAddressTree(lpszPath, lpszProfileName);

}


extern "C" __declspec(dllexport) char* GetMailExtensionFilter()
{
	return olImport.GetMailExtensionFilter();

}

extern "C" __declspec(dllexport) char* GetMailExtension()
{
	return olImport.GetMailExtension();

}

extern "C" __declspec(dllexport) char* GetAddrExtensionFilter()
{
	return olImport.GetAddrExtensionFilter();

}
extern "C" __declspec(dllexport) char* GetAddrExtension()
{
	return olImport.GetAddrExtension();

}

/////////////////////////////////////////////////////////////////////////////
// Exported - returns the list of personalities
/////////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) CPersonalityTree* GetPersonalityTree(char * lpszProfileName, bool bIsProfile)
{
	return olImport.GetPersonalityTree(lpszProfileName, bIsProfile);

}


