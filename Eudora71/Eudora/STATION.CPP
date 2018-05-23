// Station.CPP
//
// Handles Stationery list
//
#include "stdafx.h"
#include "QCUtils.h"

#include <direct.h>
#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "fileutil.h"
#include "station.h"

#include "DebugNewHelpers.h"


CStringList g_StationeryList;

void InitStationery()
{
	int err = 1;
	CString	statDir = EudoraDir;
	if (::LongFileSupportMT(EudoraDir))
		statDir += CRString(IDS_STATIONERY_FOLDER);
	else
		statDir += CRString(IDS_STATIONERY_FOLDER16); 

	// see if we need to create "Statnry" directory
	WIN32_FIND_DATA wfd;
	HANDLE findHandle;
	findHandle = FindFirstFile((const char *)statDir,&wfd);
	if ( findHandle == INVALID_HANDLE_VALUE)
		err = mkdir(statDir);
	
	FindClose(findHandle);
	// We just created folder, so nothing is here yet
	if (!err)
		return;
   
	// Save the current dir so that it can be restored later.
	char oldDir[_MAX_PATH+1]; 
#ifdef WIN32
    GetCurrentDirectory(_MAX_PATH, oldDir); 
#else
	_getcwd(oldDir, _MAX_PATH);
#endif
	// Set the current dir to where we want to start looking.
#ifdef WIN32
	SetCurrentDirectory(statDir);
#else
	_chdir(statDir);
#endif

	// Make sure this is empty
	g_StationeryList.RemoveAll();
 
	// Collect name of all files and add to list
	CString flt = "*." + CRString(IDS_STATIONERY_EXTENSION);
	findHandle = FindFirstFile(flt, &wfd); 
	while (!(findHandle == INVALID_HANDLE_VALUE)) 
	{
		CString tmp = wfd.cFileName;

		AddStationeryItem(tmp);

		if (!FindNextFile(findHandle, &wfd)) 
            break; 
	}
	FindClose(findHandle);

#ifdef WIN32
	SetCurrentDirectory(oldDir);
#else
	_chdir(oldDir);
#endif
}

void AddStationeryItem(const char * fileName)
{
	char temp[_MAX_PATH + 1];

	// Hack off full path 
	char *c = strrchr(fileName,SLASH);
	if (c)
		fileName = c + 1;
	// Hack off the trailing .sta
	CString ext = "." + CRString(IDS_STATIONERY_EXTENSION);

	// copy this for compare
	strcpy(temp,fileName);
	strlwr(temp);
	c = strstr(temp,ext);
	if (c)
	{
		*((char *)fileName + (c - temp)) = 0;
	}
	else
		return;	// This wasn't a .sta file, don't add it

	// Insert it in alphabetical order
	BOOL inserted = FALSE;
	CString listItem;
	CString lowerFileName = fileName;
	lowerFileName.MakeLower();

  	POSITION pos = g_StationeryList.GetHeadPosition();
	while(pos && !inserted)
	{
		POSITION lastPos = pos;
		listItem = g_StationeryList.GetNext(pos);
		listItem.MakeLower();
		if(lowerFileName == listItem)
			inserted = TRUE;
		if(lowerFileName < listItem)
		{
			g_StationeryList.InsertBefore(lastPos,fileName);
			inserted = TRUE;
		}
	}
	if(!inserted)
		g_StationeryList.AddTail(fileName);
}

BOOL GetStatItemPath(const char *item, char *buffer, int bufsize)
{ 
	CString	statDir = EudoraDir;
	if (::LongFileSupportMT(EudoraDir))
		statDir += CRString(IDS_STATIONERY_FOLDER);
	else
		statDir += CRString(IDS_STATIONERY_FOLDER16); 

	CString fileName = statDir + SLASH + item + "." + CRString(IDS_STATIONERY_EXTENSION);

	if (fileName.GetLength() > bufsize)
		return FALSE;
	else
		strcpy(buffer, fileName);

	return TRUE;
	

}

// returns the stationery user name given it's path					
BOOL GetStatItemName(const char *path, char *buffer, int bufsize)
{ 
	char buf[ 128 ];
	
	char * tmp = strrchr( path, '\\' );
	if ( tmp )
		tmp++;
	else
		tmp = (char *)path;

	strcpy( buf, tmp );

	tmp = strchr( buf, '.' );
	if ( tmp )
		*tmp = '\0';

	strncpy( buffer, buf, bufsize );

	return TRUE;
}




