// OEImport.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "NSImport.h"

NSImportClass nsImport;


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{

	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
	{

		//nitialize
		nsImport.LoadModuleHandle(hModule);
		nsImport.LoadNSProvider();
		
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
extern "C" __declspec(dllexport) CImportProvider* LoadProvider(bool bDisplayErrDialog)
{	
	if(nsImport.GetNumProviderChildren() > 0)
		return nsImport.GetProvider();
	else
		return NULL;
}
/////////////////////////////////////////////////////////////////////////////
// Exported WHOAMI, Tells who we are speaking with.
extern "C" __declspec(dllexport) char* QueryImportProvider()
{
	char *szImportProvider = "Netscape Messenger";
	return szImportProvider;
return NULL;
}

extern "C" __declspec(dllexport) CMessageStore* LoadFolders(char *lpszFolderPath, char * lpszProfileName)
{
	if(lpszFolderPath)
		return nsImport.LoadFolders(lpszFolderPath);
	else
		return NULL;
}

extern "C" __declspec(dllexport) char* MakeMimeDigest(char *lpszPath, void * pID, unsigned int iSize, void *pDB)
{
	struct _stat		st;

	if(lpszPath)
	{
		if (_stat(lpszPath, &st) == -1)
		{
			return NULL;
		}
		

		if( st.st_mode & _S_IFDIR)
		{
			// Do All Files in Folder
		}
		else
		if( st.st_mode & _S_IFREG)
		{
			// Do File
			return nsImport.MakeMimeWithFile(lpszPath, pID, iSize);
		}
	}
	else
		return NULL;

	return NULL;

}

extern "C" __declspec(dllexport) CAddressBookTree* GetAddressTree(char *lpszPath, char * lpszProfileName)
{
	return nsImport.GetAddressTree(lpszPath);
}

extern "C" __declspec(dllexport) char* GetMailExtensionFilter()
{
	return nsImport.GetMailExtensionFilter();

}

extern "C" __declspec(dllexport) char* GetMailExtension()
{
	return nsImport.GetMailExtension();

}

extern "C" __declspec(dllexport) char* GetAddrExtensionFilter()
{
	return nsImport.GetAddrExtensionFilter();

}
extern "C" __declspec(dllexport) char* GetAddrExtension()
{
	return nsImport.GetAddrExtension();

}

extern "C" __declspec(dllexport) CPersonalityTree* GetPersonalityTree(char * lpszProfileName, bool bIsProfile)
{
	return nsImport.GetPersonalityTree(lpszProfileName);

}