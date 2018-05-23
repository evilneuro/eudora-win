// MboxConverters.cpp
//
// Routines for converting mailboxes from other email packages
//
//	9/13-9/22/99 - Support for OE5 importing added by c_dalew@qualcomm.com.
//		My changes are marked by comments starting with DRW.

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "header.h"
#include "FileLineReader.h"
#include "mime.h"
#include "lex822.h"
#include "progress.h"
#include "debug.h"
#include "MAPIImport.h"
#include "fileutil.h"
#include "MboxConverters.h"
#include "QCMailboxDirector.h"
#include "doc.h"
#include "nickdoc.h"
#include "persona.h"
#include "guiutils.h"


extern QCMailboxDirector		g_theMailboxDirector;
extern CPersonality				g_Personalities;

extern BOOL NicknameFileExists(const CString& nickFilename);

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


NSMboxConverter::NSMboxConverter()
{
	m_PathToMail = NULL;
	m_PathToEudoraMail = NULL;
	m_InFilePathname = NULL;
	m_TempFilePathname = NULL;
	m_OutFilePathname = NULL;

	m_pRootCommand = NULL;
}
/*
ULONG NSMboxConverter::GetSizeOfData(char * PathToMail)
{
	ULONG bytes = 0;
	int fh;

	struct Pathnames *files = NULL;
	struct Pathnames *folders = NULL;

	BuildFolderList(&folders);
	BuildFileList(&files, folders);

	struct Pathnames *currentfile = files;
	struct Pathnames *currentfolder = folders;

	while(currentfile)
	{
		char *floater;
		floater = strrchr(currentfile->pathname, '.');
		*floater = 0;

		if( (fh = _open( currentfile->pathname , _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE ))  != -1 )
		{
			bytes += _filelength( fh );
			_close( fh );
		}
		currentfile = currentfile->next;
	}

	while (currentfolder)
	{
		char * fullpath;
		fullpath = new char[strlen(currentfolder->pathname)+strlen(PathToMail)+3];
		strcpy(fullpath, PathToMail);
		strcat(fullpath, _T("\\"));
		strcat(fullpath, currentfolder->pathname);
		_chdir (fullpath);
		bytes += GetSizeOfData(fullpath);
		delete [] fullpath;
		currentfolder = currentfolder->next;
	}

	DeleteTree(files);
	DeleteTree(folders);

	return bytes;
}
*/
void NSMboxConverter::DeleteTree(Pathnames * node)
{
	if (!node)
		return;

	if (node->next)
		DeleteTree(node->next);

	delete [] node->pathname;
	delete node;
}

bool NSMboxConverter::Startup()
{
	if(FindNSsPath(&m_PathToMail))
		if(FindEudorasPath())
		{
			return true;
		}
		return false;
}

NSMboxConverter::~NSMboxConverter()
{
	m_PathToMail ?  delete [] m_PathToMail:NULL;
	m_PathToEudoraMail ?  delete [] m_PathToEudoraMail:NULL;
	m_InFilePathname ?  delete [] m_InFilePathname:NULL;
	m_OutFilePathname ?  delete [] m_OutFilePathname:NULL;
}


bool NSMboxConverter::FindEudorasPath()
{

	m_PathToEudoraMail = new char[strlen(EudoraDir)+1];
	strcpy(m_PathToEudoraMail,EudoraDir);

	return (true);
}

int NSMboxConverter::Convert(ImportChild *Child, char * InPathname, char *OutPathname)
{	
	char *DisplayName = NULL;
	int numofmsgs = 0;
	CFileFind	filefinder;
	CFileFind	folderfinder;


	if (!Startup())
		return -1;
/*
	if (InPathname)
	{
		if (m_PathToEudoraMail)
			delete [] m_PathToEudoraMail;
		m_PathToEudoraMail = new char[strlen(InPathname)+1];
		strcpy(m_PathToEudoraMail, InPathname);
	}
	if (OutPathname)
	{
		if (m_PathToMail)
			delete [] m_PathToMail;

		m_PathToMail = new char[strlen(OutPathname)+1];
		strcpy(m_PathToMail, OutPathname);
	}
*/
	if (!Child->PathToData)
	{
		return false;
	}

	char *MailDir;
	MailDir = new char[strlen(Child->PathToData)+6];
	strcpy(MailDir, Child->PathToData);
	strcat(MailDir, _T("\\Mail"));

	ProcessMailboxes(MailDir, m_pRootCommand);

	delete [] MailDir;

	DeleteTempFile();
	::CloseProgress();
	return (numofmsgs);
}

char * NSMboxConverter::GetDispName(char * pathname)
{
	char * floater;
	char * returnstring;
	floater = strrchr(pathname, '\\');
	if (!floater)
		return NULL;

	floater++;
	returnstring = new char[strlen(floater)+1];
	strcpy(returnstring, floater);
	return (returnstring);
}


bool NSMboxConverter::AddNode(struct Pathnames **currentfile, 	struct _finddata_t *file)
{
	*currentfile = new struct Pathnames;
	if (*currentfile == NULL)
		return false;

	(*currentfile)->pathname = new char[strlen(file->name)+1];
	strcpy((*currentfile)->pathname, file->name);

	(*currentfile)->next = NULL;
	return true;
}

bool NSMboxConverter::existsInTree(char * name, struct Pathnames *folders)
{
	struct Pathnames *currentfolder = folders;
	while (currentfolder)
	{
		char * filename;
		char * foldername;
		char * floater= NULL;
		filename = new char[strlen(name)+1];
		foldername = new char[strlen(currentfolder->pathname)+1];
		strcpy(filename, name);
		
		floater = strrchr(filename, '.');
		if (floater)
			*floater = 0;

		strcpy(foldername, currentfolder->pathname);
		floater = strrchr(foldername, '.');
		if (floater)
			*floater = 0;

		if (!strcmp(foldername, filename))
		{
			delete [] filename;
			delete [] foldername;
			return true;
		}
		delete [] filename;
		delete [] foldername;
		currentfolder = currentfolder->next;
	}
	return false;
}

bool NSMboxConverter::BuildFileList(struct Pathnames **files, struct Pathnames *folders)
{
	struct Pathnames	**currentfile;
	struct _finddata_t	c_file;
	long hFile;

	if (*files != NULL)
		return false;		// Someone's already populated the list.
	
	currentfile = files;


	if( (hFile = _findfirst("*.snm", &c_file )) == -1L )
		return false;	// No files

	else
	{            
		if (!existsInTree(c_file.name, folders))
		{
			AddNode(currentfile, &c_file);
			currentfile = &((*currentfile)->next);
		}
		while( _findnext( hFile, &c_file ) == 0 )
		{
			if (!existsInTree(c_file.name, folders))	// We only want to add a node if it doesn't have a folder associated with it.
			{
				AddNode(currentfile, &c_file);
				currentfile = &((*currentfile)->next);
			}
		}
		_findclose( hFile );
	}
	return true;
}

bool NSMboxConverter::BuildFolderList(struct Pathnames **files)
{
	struct Pathnames	**currentfile;
	struct _finddata_t	c_file;
	long hFile;

	if (*files != NULL)
		return false;		// Someone's already populated the list.
	
	currentfile = files;


	if( (hFile = _findfirst( "*.sbd", &c_file )) == -1L )
		return false;	// No files

	else
	{            
		AddNode(currentfile, &c_file);
		currentfile = &((*currentfile)->next);
		while( _findnext( hFile, &c_file ) == 0 )
		{
			AddNode(currentfile, &c_file);
			currentfile = &((*currentfile)->next);
		}
		_findclose( hFile );
	}
	return true;
}


bool NSMboxConverter::ProcessMailboxes(char *pathname, QCMailboxCommand * MboxCmd )
{
	QCMailboxCommand * NewMboxCmd;

	if (MboxCmd == NULL)
	{
		m_pRootCommand = CreateMbox(NULL, NULL);
		MboxCmd = m_pRootCommand;
		NewMboxCmd = m_pRootCommand;
	}
	else
	{
		char *DisplayName;
		char *filespath;
		char *floater;
		filespath = new char[strlen(pathname)+1];
		strcpy(filespath, pathname);
		floater = strrchr(filespath, '.');
		if (floater)
			*floater = 0;

		DisplayName = GetFoldersName(pathname);
		NewMboxCmd = CreateMbox(DisplayName, MboxCmd, MBT_FOLDER);
		ProcessMbox(filespath, NewMboxCmd);
		delete [] filespath;
		delete [] DisplayName;
	}

	if (!MboxCmd)
		return false;

	if ( _chdir(pathname))// OK, now go there.
	{
		return false;
	}

	struct Pathnames *files = NULL;
	struct Pathnames *folders = NULL;

	BuildFolderList(&folders);
	BuildFileList(&files, folders);

	struct Pathnames *currentfile = files;
	struct Pathnames *currentfolder = folders;



		while(currentfile)
		{
			if (EscapePressed(1))
				return false;
			char * filespath;
			char * floater;
			filespath = new char[strlen(currentfile->pathname)+strlen(pathname)+2];
			strcpy(filespath,pathname);
			strcat(filespath, _T("\\"));
			strcat(filespath, currentfile->pathname);
			floater = strrchr(filespath, '.');
			if (floater && !strcmp(floater, _T(".snm")))
				*floater = 0;
				
			ProcessMbox(filespath, NewMboxCmd);
			currentfile = currentfile->next;
			delete [] filespath;
		}
		while(currentfolder)
		{
			if (EscapePressed(1))
				return false;
			char *folderspath;
			folderspath = new char[strlen(currentfolder->pathname)+strlen(pathname)+2];
			strcpy(folderspath, pathname);
			strcat(folderspath, _T("\\"));
			strcat(folderspath, currentfolder->pathname);
			ProcessMailboxes(folderspath, NewMboxCmd);
			currentfolder = currentfolder->next;
			delete [] folderspath;
		}

	return false;	// Why false? anyone know?
}

bool NSMboxConverter::OpenNSFile(char * pathname)
{
	if (!pathname) return (false);
	if(FAILED(m_infile.Open(pathname,O_RDONLY)))
	{
		return (false);
	}
	return (true);
}

bool NSMboxConverter::CloseNSFile()
{
	m_infile.Flush();
	m_infile.Close();
	return true;
}

int NSMboxConverter::TempfileGetsBuffer(char *readbuffer, int bufsize)
{
	char * floater;
	floater = readbuffer;
	int posn = 1;


	while ((*floater != 0) && (posn < bufsize))
	{
		floater++;
		posn++;
	}

	if (*floater == 0)
	{	// We're done.
		m_tempfile.Put(readbuffer, posn);
		return (posn);
	}
	else if ( posn >= bufsize)
	{
		m_tempfile.Put(readbuffer, bufsize);
		m_infile.Read(readbuffer,bufsize);
		return(TempfileGetsBuffer(readbuffer, bufsize));
	}
//	ASSERT
	return (-1);
}

bool NSMboxConverter::GrabNextMsg()
{
	char *readbuffer;
	char tempbuffer[2] = {0x00, 0x00};
	int readbufsize = 4096;
//	int returnval;
	long numbytesread;
	int numlines =0;
	bool done = false;
	HRESULT hresult;

	readbuffer = new char[readbufsize+1]; //4097
	readbuffer[readbufsize] = 0;	// Let's never get this far...

	while(!done)
	{
		hresult = m_infile.GetLine(readbuffer, readbufsize-2, &numbytesread);
//		::ProgressAdd(numbytesread);
		if (numbytesread == 0)
			if (numlines == 0)
			{
				delete [] readbuffer;
				return false;
			}
			else done = true;

		else if(strncmp(readbuffer, _T("From "), 5))
		{
			m_tempfile.PutLine(readbuffer);
			numlines++;
		}
		else if (numlines != 0)
			done = true;
	}

	delete [] readbuffer;
	m_tempfile.Flush();

	return (true);
}

LONG NSMboxConverter::GetNumMsgs(char *pathname)
{
	ULONG	NumMsgs = 0;
	char tempbuf[4];
	JJFile stream;
	char *indexfile;
	HRESULT hresult;
//	char blah[1000];

	indexfile = new char [strlen(pathname)+5];
	strcpy(indexfile, pathname);
	strcat(indexfile, _T(".snm"));

   hresult = stream.Open( indexfile, O_RDONLY );
   if (HR_FAILED(hresult))
   {
		delete [] indexfile;
		return -1;  
   }
   else
   {
		hresult = stream.Seek(599);
		if (HR_FAILED(hresult))
		{
			stream.Close();
			delete [] indexfile;
			return -1;  
		}

		hresult = stream.Read( tempbuf, 4);
		if (HR_FAILED(hresult))
		{
			stream.Close();
			delete [] indexfile;
			return -1;  
		}

		stream.Close();
		
		((char *)&NumMsgs)[3] = tempbuf[0];
		((char *)&NumMsgs)[2] = tempbuf[1];
		((char *)&NumMsgs)[1] = tempbuf[2];
		((char *)&NumMsgs)[0] = tempbuf[3];

		delete [] indexfile;
		return NumMsgs;
   }
}


bool NSMboxConverter::ProcessMbox(char *pathname, QCMailboxCommand * MboxCmd )
{
	ULONG	MsgCount;
	char * DisplayName;

	MainProgress(_T("Converting From Netscape Mail"));
	if (!OpenNSFile(pathname))
			return false;
	
		DisplayName = GetDispName(pathname);
		MsgCount = GetNumMsgs(pathname);

		if (MsgCount > 0)
		{
			char * buf;
			buf = new char[strlen(DisplayName)+strlen("Messages left in : ")+2];
			sprintf(buf, "Messages left in %s: ", DisplayName);
			CountdownProgress(buf, MsgCount);
			delete [] buf;
		}
		if (!CreateMbox(DisplayName, MboxCmd))		// does this set outfilepath?????
		{
			return false;
			delete [] DisplayName;
		}

		delete [] DisplayName;

		OpenTempFile();

		while (GrabNextMsg())
		{
			if (EscapePressed(1))
				return false;
			CloseTempFile();
			MoveMsgToEudora();
			DecrementCountdownProgress();
			OpenTempFile();
		}
//		::CloseProgress();
		CloseTempFile();
		CloseNSFile();

	return true;
}


///////////////////////////////////
//	FindNSsPath
///////////////////
// Reads registry and determines where the user's mail store is located since we're going to read the
// raw files.
bool NSMboxConverter::FindNSsPath(char ** path) //Good
{
	HKEY	hKey;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,  _T("Software\\Netscape\\Netscape Navigator\\Users\\Default"),
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		char szData[512];
		DWORD dwKeyDataType;
		DWORD dwDataBufSize = sizeof(szData)/sizeof(szData[0]);

		if (RegQueryValueEx(hKey, _T("DirRoot"), NULL, &dwKeyDataType,
			(LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
		{
			if (dwDataBufSize < 1)
			{
				RegCloseKey(hKey);
				return false;
			}
			*path = new char[dwDataBufSize+strlen(_T("\\Mail"))+1];
			strcpy(*path,(const char *)&szData);
			strcat(*path, _T("\\Mail"));
		}
		else 
		{
			RegCloseKey(hKey);
			return false;
		}
		RegCloseKey(hKey);
	}
	return (true);
}

bool NSMboxConverter::InitMboxFind(CFileFind *filefinder)
{
	if (filefinder->FindFile("*.snm"))
		return true;
	else
		return false;
}

bool NSMboxConverter::InitFolderFind(CFileFind *Folderfinder)
{
	if (Folderfinder->FindFile("*.sbd"))
		return true;
	else
		return false;
}


bool NSMboxConverter::MoveMsgToEudora()
{
	if (!InFileToMBX(m_TempFilePathname, m_OutFilePathname, NS_DISPLAY_NAME))
		return false;

	return true;
}

/////////////////////////////////////
// Create the Netscape folder with the help of the mailbox director.
////////
QCMailboxCommand * NSMboxConverter::CreateMbox(char *name, QCMailboxCommand *ParentFolderCmd, MailboxType Type)
{
	QCMailboxCommand * NewFolderCmd = NULL;
	char * Filename = NULL;
//	MailboxType Type;

	if (!name)
	{
		Filename = new char[21];
		strcpy(Filename, NS_DISPLAY_NAME);
		Type = MBT_FOLDER;
	}
	else
	{
		Filename = new char[strlen(name)+1];
		strcpy(Filename, name);
	}

	UniquifyName(&Filename, ParentFolderCmd);

	NewFolderCmd = g_theMailboxDirector.AddCommand(Filename, Type, ParentFolderCmd);

	delete [] Filename;

	if (Type == MBT_REGULAR)
	{
		char *pathToMBX;
		pathToMBX = new char[((NewFolderCmd->GetPathname()).GetLength())+1];
		strcpy(pathToMBX, NewFolderCmd->GetPathname());
		KillTheTOCFile(pathToMBX);
		if (m_OutFilePathname)
			delete [] m_OutFilePathname;

		m_OutFilePathname = new char[strlen(pathToMBX)+1];
		strcpy(m_OutFilePathname, pathToMBX);
		delete [] pathToMBX;
	}

	return NewFolderCmd;
}


bool NSMboxConverter::OpenTempFile()
{
	if (!m_TempFilePathname)
	{
		m_TempFilePathname = _tempnam(".","EUD");
	}

	if(FAILED(m_tempfile.Open(m_TempFilePathname, O_CREAT | O_RDWR | O_TRUNC)))
	{
		return (false);
	}

	return (true);
}

bool NSMboxConverter::CloseTempFile()
{
	m_tempfile.Flush();
	m_tempfile.Close();
	return true;
}

bool NSMboxConverter::GrabNextMbox(CFileFind *filefinder, CFileFind *folderfinder, QCMailboxCommand *ParentFolderCmd)
{
	// Enumerate the source files.
	if (m_InFilePathname)
	{
		delete [] m_InFilePathname;
		m_InFilePathname = NULL;
	}

	char * temp;
	temp = new char[MAX_PATH];

	if(filefinder->FindNextFile())
	{
		char * temp;
		temp = new char[MAX_PATH];
		char * floater;
		strcpy(temp, (LPCTSTR) filefinder->GetFilePath());
		floater = strrchr(temp, '.');
		ASSERT(!strcmp(floater, _T(".snm")));
		if (strcmp(floater, _T(".snm")))	// Hmm, if the extension isn't .snm howd we get here?
			return false;

		*floater = 0;
		m_InFilePathname = new char[strlen(temp)+1];
		strcpy(m_InFilePathname, temp);
		delete [] temp;
		return true;
	}
	else
	{
		QCMailboxCommand *NextFolderCmd;

		if(NextFolderCmd = GetNextFolder(folderfinder, ParentFolderCmd))
		{
			InitMboxFind(filefinder);
			return GrabNextMbox(filefinder, folderfinder, NextFolderCmd);
		}
		else
			return false;
	}
}

QCMailboxCommand * NSMboxConverter::GetNextFolder(CFileFind *folderfinder, QCMailboxCommand *ParentFolderCmd)
{

	folderfinder->FindNextFile();

	if (!folderfinder->IsDirectory())
		return false;

	if(folderfinder->GetFilePath())
	{
		char * temp;
		temp = new char[MAX_PATH];
//		char * floater;
		QCMailboxCommand *ReturnedFolderCmd;

		strcpy(temp, (LPCTSTR) folderfinder->GetFilePath());

		char *Displayname = GetFoldersName(temp);

		ReturnedFolderCmd = CreateMbox(Displayname, ParentFolderCmd, MBT_FOLDER);

		if ( _chdir(temp))// OK, now go there.
		{
			delete [] temp;
			return NULL;
		}

		delete [] temp;

		return ReturnedFolderCmd;
	}
	else
	{
		return NULL;
	}
}

char * NSMboxConverter::GetFoldersName(char *pathname)
{
	char * floater;
	char *	temp;
	char * returnstring;

	temp = new char[strlen(pathname)+1];
	strcpy(temp, pathname);

	floater = strrchr(temp, '.');
	*floater = 0;

	floater = strrchr(temp, '\\');
	floater++;
	returnstring = new char[strlen(floater)+1];
	strcpy(returnstring, floater);
	delete [] temp;
	return returnstring;
}

bool NSMboxConverter::DeleteTempFile()
{
	if (m_TempFilePathname)
		DeleteFile(m_TempFilePathname);
	return true;
}



OEMboxConverter::OEMboxConverter()
{
	m_PathToMail = NULL;
	m_PathToEudoraMail = NULL;
	m_InFilePathname = NULL;
	m_TempFilePathname = NULL;
	m_OutFilePathname = NULL;
	m_realNames = NULL;
	// DRW 9/13/99 - Added the following for OE5 handling.
	m_bFoundOE5 = false;
	m_mbfpCurrMBox = NULL;
	m_meOE5MsgTable = NULL;
	m_lNumOE5MsgTableEntries = 0;
	m_lCurrOE5MsgTableEntry = 0;
	m_lOE5MBoxID = 0;
	m_lOE5ParentMBoxID = 0;
	m_bOE5HasSubfolders = false;

	m_pRootCommand = NULL;
	FindOEsPath();
	FindEudorasPath();
}

OEMboxConverter::~OEMboxConverter()
{
	delete [] m_PathToMail;
	delete [] m_PathToEudoraMail;
	delete [] m_InFilePathname;
	delete [] m_OutFilePathname;
	if (m_realNames)
		removeRealNames(m_realNames);
}

bool OEMboxConverter::findEntryByToken(JJFile *pchfile) // testtoken should default to 0x00000000
{
	char			buffer[2] = {0x00, 0x00};
	unsigned char	testtoken[4] = {0x00, 0x00, 0x00, 0x00};
	unsigned char	magictoken[4] = {0x00, 0x00, 0x70, 0x02};
	unsigned char	mboxidtoken0[2] = {0x00, 0x80};				// DRW 9/13/99 - OE5 token for mailbox id.
	unsigned char	mboxidtoken1[2] = {0x01, 0x80};				// DRW 9/13/99 - OE5 token for mailbox id.
	unsigned char	parentmboxidtoken[2] = {0x00, 0x81};		// DRW 9/13/99 - OE5 token for parent mailbox id.
	unsigned char	hassubfolderstoken[2] = {0x00, 0x06};		// DRW 9/22/99 - OE5 token for folder with subfolders.
	int				i;

	// DRW 9/13/99 - OE5 has different magictoken.
	if (m_bFoundOE5)
	{
		magictoken[0] = 0x8A;
		magictoken[1] = 0x03;
		magictoken[2] = 0x00;
		magictoken[3] = 0x00;
	}

	m_lOE5MBoxID = 0;
	m_lOE5ParentMBoxID = 0;
	m_bOE5HasSubfolders = false;

	while (!(pchfile->Read(buffer,1)))
	{
		// Test for mailbox entry start token.
		if ((memcmp (magictoken, testtoken,4) == 0)) break;
		if (m_bFoundOE5)
		{
			// Test for mailbox id token.
			if ((memcmp(mboxidtoken0, testtoken, 2) == 0) ||
				(memcmp(mboxidtoken1, testtoken, 2) == 0))
			{
				m_lOE5MBoxID = (long)testtoken[2];
			}
			// Test for parent id token.
			else if (memcmp(parentmboxidtoken, testtoken, 2) == 0)
			{
				m_lOE5ParentMBoxID = (long)testtoken[2];
			}
			// Test for has subfolder(s) token.
			else if (memcmp(hassubfolderstoken, testtoken, 2) == 0)
			{
				m_bOE5HasSubfolders = true;
			}
		}
		testtoken[0] = testtoken[1];
		testtoken[1] = testtoken[2];
		testtoken[2] = testtoken[3];
		testtoken[3] = buffer[0];
	}
	
	if (memcmp (magictoken, testtoken,4) != 0)
		return false;

	// DRW 9/13/99 - We don't need to do this for OE5.
	if (!m_bFoundOE5)
	{
		for (i = 1; i < 15; i++)
		{
			if (pchfile->Read(buffer,1))
				return false;
		}
	}
	return true;
}

void OEMboxConverter::BuildRealFolderNames()
{
	// Here's where we read in the folders.nch file to figure out which folderx.mbx files are what Display name.
	char * folderspath;
	JJFile	folders;
	char * floater;

	Mbox_FnamePair **realnames = &m_realNames;
	if (*realnames)	// Someone's already done this... Hrmmm.
		return;

	if (!m_PathToMail)
		return;

	// DRW 9/13/99 - Added OE5 handling.
	if (!m_bFoundOE5)
	{
		folderspath = new char[strlen(m_PathToMail)+strlen(_T("\\folders.nch"))+1];
		strcpy(folderspath, m_PathToMail);
		strcat(folderspath, _T("\\folders.nch"));
	}
	else
	{
		folderspath = new char[strlen(m_PathToMail)+strlen(_T("\\folders.dbx"))+1];
		strcpy(folderspath, m_PathToMail);
		if (folderspath[strlen(folderspath) - 1] != '\\')
		{
			strcat(folderspath, _T("\\"));
		}
		strcat(folderspath, _T("folders.dbx"));
	}
	folders.Open(folderspath, O_RDONLY);
	
	while (findEntryByToken(&folders))	// Found a token
	{
		if (*realnames)
		{
			while ((*realnames)->next)
				realnames = &((*realnames)->next);
			realnames = &((*realnames)->next);
		}

		char * Buffer = NULL;
		Buffer = new char[601];
		// DRW 9/13/99 - Added OE5 handling.
		if (!m_bFoundOE5)
		{
			// DRW 9/13/99 - If OE4 read 600 character buffer.
			folders.Read(Buffer, 600);
		}
		else
		{
			// DRW 9/13/99 - If OE5 read until next '\0'.
			char	c = '\0';
			char *	szTempBufPtr = Buffer;
			folders.Read(&c, 1);
			while (c != '\0')
			{
				*szTempBufPtr++ = c;
				folders.Read(&c, 1);
			}
			*szTempBufPtr = '\0';
		}

		if (Buffer[0] != 0)
		{
			// DRW 9/13/99 - Added OE5 handling.
			if (!m_bFoundOE5)
			{
				// DRW 9/13/99 - If OE4 skip next 259 characters.
				floater = Buffer+259;	// Distance from existence of Realname of folder and it's Filename
			}
			else
			{
				// DRW 9/13/99 - If OE5 read to next '\0'.
				floater = new char[256];
				if (floater)
				{
					char	c = '\0';
					char *	szTempBufPtr = floater;
					folders.Read(&c, 1);
					while (c != '\0')
					{
						*szTempBufPtr++ = c;
						folders.Read(&c, 1);
					}
					*szTempBufPtr = '\0';
				}
			}

			bool	bValidMailbox = true;
			// If we are importing OE5 see if Buffer and floater appear to represent a valid mailbox.
			if (m_bFoundOE5)
			{
				if ((Buffer == NULL) || (floater == NULL) || (strlen(floater) == 0))
				{
					bValidMailbox = false;
				}
				else if (stricmp(floater + strlen(floater) - 4, ".dbx") != 0)
				{
					// floater does not end in .dbx so there is no mailbox file to process.
					bValidMailbox = false;
				}
				else
				{
					// See if we already inserted an entry with the same info.
					Mbox_FnamePair *	mbpTemp = m_realNames;
					if (mbpTemp)
					{
						do
						{
							if ((stricmp(mbpTemp->DisplayName, Buffer) == 0) &&
								(stricmp(mbpTemp->Filename, floater) == 0))
							{
								bValidMailbox = false;
							}
							mbpTemp = mbpTemp->next;
						}
						while (bValidMailbox && mbpTemp);
					}
				}
			}

			if (bValidMailbox)
			{
				*realnames = new Mbox_FnamePair;
				(*realnames)->DisplayName = new char[strlen(Buffer)+1];
				strcpy((*realnames)->DisplayName, Buffer);
				(*realnames)->Filename = new char[strlen(floater)+1];
				strcpy((*realnames)->Filename, floater);
				(*realnames)->lMBoxID = m_lOE5MBoxID;
				(*realnames)->bHasSubfolders = m_bOE5HasSubfolders;
				(*realnames)->lParentMBoxID = m_lOE5ParentMBoxID;
				(*realnames)->pMailbox = NULL;
				(*realnames)->next = NULL;
			}

			// DRW 9/13/99 - If OE5, free floater.
			if (m_bFoundOE5 && floater)
			{
				delete [] floater;
			}
		}
		delete [] Buffer;
	}

	delete [] folderspath;
}

void OEMboxConverter::removeRealNames(Mbox_FnamePair *realnames)
{
	if(realnames->next)
		removeRealNames(realnames->next);
		
	delete [] realnames->DisplayName;
	realnames->DisplayName = NULL;
	delete [] realnames->Filename;
	realnames->Filename = NULL;
	delete realnames;
}


int OEMboxConverter::Convert(ImportChild *Child, char * InPathname, char *OutPathname)
{	
	if (!m_PathToMail)
		return (false);

	char *DisplayName = NULL;
	int numofmsgs = 0;

	BuildRealFolderNames();

	m_pRootCommand = CreateMbox(NULL, NULL, MBT_FOLDER);
	if (!m_pRootCommand)
		return (false);

	if (!InitMboxFind())
		return false;

	while(GrabNextMbox())
	{
		numofmsgs = 0;
		if (!OpenOEFile())
			break;
		MainProgress(_T("Converting From Outlook Express"));

		DisplayName = FindRealName(m_InFilePathname);

		LONG MsgCount;
		MsgCount = GetNumMsgs(m_InFilePathname);

		if (MsgCount > 0)
		{	
			char * buf;
			buf = new char[strlen(DisplayName)+20];
			sprintf(buf, "Messages left in %s: ", DisplayName);
			CountdownProgress(buf, MsgCount);
			delete [] buf;
		}

		QCMailboxCommand	*pqcmb = NULL;
		if (!m_bFoundOE5)
		{
			// DRW 9/21/99 - Under OE4 the behavior is unchanged.  GetParentMBox() will always
			// return m_pRootCommand so all mailboxes are still created at the root level.
			QCMailboxCommand	*pqcmbParent = GetParentMbox();
			pqcmb = CreateMbox(DisplayName, pqcmbParent);
		}
		else
		{
			// DRW 9/21/99 - Under OE5 for each mailbox, create a containing folder of the same
			// name as the mailbox.  OE5 folders can contain messages, so we need both a folder
			// to hold mailboxes and a mailbox to contain messages.

			// We might be able to optimize this so that we only create a containing folder if
			// we really need it (if the OE5 folder contains both folders and messages) but doing
			// for every folder seems more intuitive.

			// Find the parent mailbox (not a folder).
			QCMailboxCommand	*pqcmbParent = GetParentMbox();
			// Find the parent folder for the above mailbox.  This is where we create the
			// folder for the current mailbox.
			if (pqcmbParent != m_pRootCommand)
			{
				pqcmbParent = g_theMailboxDirector.FindParent(pqcmbParent);
			}

			// Create a folder and a mailbox within that folder with the same name.
			if (m_mbfpCurrMBox->bHasSubfolders)
			{
				pqcmbParent = CreateMbox(DisplayName, pqcmbParent, MBT_FOLDER);
			}
			pqcmb = CreateMbox(DisplayName, pqcmbParent, MBT_REGULAR);
			if (!pqcmb)
			{
				delete [] DisplayName;
				return false;
			}
		}

		if (m_mbfpCurrMBox)	m_mbfpCurrMBox->pMailbox = pqcmb;

		delete [] DisplayName;

		OpenTempFile();

		while (GrabNextMsg())
		{
			if (EscapePressed(1))
				break;

			CloseTempFile();

			MoveMsgToEudora();
			numofmsgs++;
			DecrementCountdownProgress();

			OpenTempFile();
		}

		if (m_mbfpCurrMBox)	m_mbfpCurrMBox = m_mbfpCurrMBox->next;

		CloseTempFile();
		CloseOEFile();
		if (EscapePressed())
			return numofmsgs;
	}
	::CloseProgress();
	DeleteTempFile();
	return (numofmsgs);
}

bool OEMboxConverter::DeleteTempFile()
{
	if (m_TempFilePathname)
		DeleteFile(m_TempFilePathname);
	return true;
}

bool OEMboxConverter::MoveMsgToEudora()
{
	if (!InFileToMBX(m_TempFilePathname, m_OutFilePathname, OE_DISPLAY_NAME))
		return false;

	return true;
}


int OEMboxConverter::TempfileGetsBuffer(char *readbuffer, int bufsize)
{
	char * floater;
	floater = readbuffer;
	int posn = 1;


	while ((*floater != 0) && (posn < bufsize))
	{
		floater++;
		posn++;
	}

	if (*floater == 0)
	{	// We're done.
		m_tempfile.Put(readbuffer, posn);
		return (posn);
	}
	else if ( posn >= bufsize)
	{
		m_tempfile.Put(readbuffer, bufsize);
		m_infile.Read(readbuffer,bufsize);
		return(TempfileGetsBuffer(readbuffer, bufsize));
	}
//	ASSERT
	return (-1);
}

/*
bool OEMboxConverter::GrabNextMsg()		// Large chunk size
{
	char *readbuffer;
	char tempbuffer[2] = {0x00, 0x00};
	int readbufsize = 4096;
	int returnval;

	readbuffer = new char[readbufsize+1]; //4097
	readbuffer[readbufsize] = 0;	// Let's never get this far...


	HRESULT badRead = 0;

	if (!FindToken())
	{
		delete [] readbuffer;
		return (false);
	}


	if (!m_infile.Read(readbuffer,readbufsize))
	{
		returnval = TempfileGetsBuffer(readbuffer, readbufsize);
	}
	m_tempfile.DisplayErrors(false);
	delete [] readbuffer;
	bool test = true;

	if (returnval < readbufsize)
	{	// Has to be negative, we're trying to go backward. shouldn't be able to fail.
		badRead = m_infile.Seek(returnval-readbufsize,SEEK_CUR, NULL);
	}

	long lOffset = 0;
	m_tempfile.Tell(&lOffset);

//if	(!HR_FAILED(badRead) && lOffset != 0)
//{
	do 
	{	
		badRead = m_tempfile.Seek(-1, SEEK_CUR, NULL);
		if (HR_FAILED(badRead))
			break;
		badRead = m_tempfile.Read(tempbuffer, 1);
		if (HR_FAILED(badRead))
			break;
		if (tempbuffer[0] == 0)
			badRead = m_tempfile.Seek(-1, SEEK_CUR, NULL);
			if (HR_FAILED(badRead))
				break;

	} while (tempbuffer[0] == 0);
//}


	m_tempfile.Tell(&lOffset);
	m_tempfile.ChangeSize(lOffset);

	tempbuffer[0] = '\r';
	tempbuffer[1] = '\n';
	m_tempfile.Put(tempbuffer, 2);

	m_tempfile.Flush();

	return (true);
}
*/
bool OEMboxConverter::GrabNextMsg()		// Large chunk size
{
	// DRW 9/13/99 - Added OE5 handling.
	if (!m_bFoundOE5)
	{
		char tempbuffer[2] = {0x00, 0x00};
		int readbufsize = 4096;

		HRESULT badRead = 0;

		if (!FindToken())
		{
			return false;
		}

		return ReadTilDone();
	}
	else
	{
		// DRW 9/13/99 - If importing OE5, look in the table we created
		// earlier to find the offset in the mailbox file for the current message.
		if (m_meOE5MsgTable && (m_lCurrOE5MsgTableEntry < m_lNumOE5MsgTableEntries))
		{
			HRESULT		hresult;
			long		lMessageOffset = m_meOE5MsgTable[m_lCurrOE5MsgTableEntry].lMessageOffset;

			// DRW 9/13/99 - Look in file specified by pathname for number of messages.
			hresult = m_infile.Open(m_InFilePathname, CFile::modeRead);
			if (HR_FAILED(hresult))
			{
				return false;  
			}

			// DRW 9/13/99 - Seek() to the beginning of the current message.
			m_infile.Seek(lMessageOffset, SEEK_SET);

			ReadTilDone();

			m_infile.Close();

			++m_lCurrOE5MsgTableEntry;

			return true;
		}
	}
	return false;
}

bool OEMboxConverter::ReadTilDone()
{
	// DRW 9/13/99 - Added OE5 handling.
	if (!m_bFoundOE5)
	{
		char *readbuffer;
		int readbufsize = 4096;
		readbuffer = new char[readbufsize+1]; //4097
		readbuffer[readbufsize] = 0;	// Let's never get this far...
		HRESULT hresult;
		char * floater;
		int posn;
		long bytesread;
		bool done = false;

		while (!done)
		{
			posn = 1;

			long blah;
			m_infile.Tell(&blah);
			m_infile.Seek(blah);

			hresult = m_infile.RawRead(readbuffer,readbufsize, &bytesread);
			if (HR_FAILED(hresult))
			{
				delete [] readbuffer;
				return false;
			}

			floater = readbuffer;

			while (*floater != 0 && posn < bytesread)
			{
				floater++;
				posn++;
			}

			if (*floater == 0)
			{
				m_tempfile.Put(readbuffer, posn-1);
				m_tempfile.Put(_T("\r\n"), 2);
				if (posn < bytesread)
				{	// Has to be negative, we're trying to go backward. shouldn't be able to fail.
					hresult = m_infile.Seek(posn-bytesread,SEEK_CUR, NULL);
				}

				if (readbufsize != 0)
				{	// Now to make sure the tempfile gets only the data it needs and extraneous nulls are removed.
					

				}
				done = true;
			}
			else if (bytesread < readbufsize)	// End of file may not have a null at the end...
			{									// No need to rewind the file any.
				m_tempfile.Put(readbuffer, bytesread);
				m_tempfile.Put(_T("\r\n"), 2);
				done = true;
			}
			else
			{
				m_tempfile.Put(readbuffer, posn);
			}
		}
		delete [] readbuffer;
	}
	else
	{
		OE5Header *		myHeader = new OE5Header;			// Local Message Header structure
		OE5Table *		myTable = new OE5Table;				// Local Message Table structure
		long			lMessageLocation = 0;				// Message location for unsigned short overflow
		long			lCompare = 0;						// Compare for unsigned short overflow
		long			lFileLength = 0;					// Length of file
		struct stat		st;									// File info (for file size)
		bool			bStillOK = true;					// Status flag
		char *			szMessageText = NULL;				// Message text
		char *			szPtr = NULL;						// Misc char* pointer
		static long		lSubjectNumber = 1;					// Number of subject currently being read
		long			lActualSeekPos = 0;					// Position we want to seek to before the next read
		HRESULT			result = 0;

		// Determine file length.
		result = m_infile.Stat(&st);
		lFileLength = st.st_size;

		// DRW 9/13/99 - What follows is the OE5 code taken from jasmille@qualcomm.com's
		// OE5 translator code with minor changes for optimization and readability.
		m_infile.Read((char*)myTable, sizeof(OE5Table));	// Read the table

		// myTable->lMessagePosition &= 0x00FFFFFF;		//Buncha crap that doesnt work
		// BYTE tmp = HIBYTE(myTable->lMessagePosition);	// gonna subtract but didnt work
		
		for (long x = 0; x < m_lNumOE5MsgTableEntries; x++)	// check if message been done
		{
			if (m_meOE5MsgTable[x].lMessageText == myTable->lMessagePosition) return false;
		}
		// Add the message reference to the directory
		m_meOE5MsgTable[m_lCurrOE5MsgTableEntry].lMessageText = myTable->lMessagePosition;
		lMessageLocation = myTable->lMessagePosition;

		bStillOK = true;
		
		// At this point we have an index into the file for the next message,
		// but the index is a short and may not really point to the actual start
		// of the message.  If the index does not point to the message, keep
		// adding 65536 until we are pointing to the right message.
		m_infile.Seek(lMessageLocation, SEEK_SET);					// Goto Location so we can ...
		m_infile.Read((char*)&lCompare, sizeof(long));				// Read it to compare
		lActualSeekPos = lMessageLocation;							// Remember where to seek before next read

		if (lCompare != lMessageLocation)
		{
			bStillOK = false;
			lMessageLocation = myTable->lMessagePosition;			// Start at location specified in the table
			
			bool		bAtBeginning = false;
			while (!bAtBeginning)									// Keep adding 65536 til we find the beginning
			{
				lMessageLocation += 65536;							// Add padding to get to next possible location
				if (lMessageLocation > lFileLength)	return false;	// Return out if we are past end of file
				m_infile.Seek(lMessageLocation, SEEK_SET);			// Goto Location so we can ...
				m_infile.Read((char*)&lCompare, sizeof(long));		// Read it to compare 
				lActualSeekPos = lMessageLocation;					// Remember where to seek before next read
				if (lMessageLocation == lCompare)
				{
					// DRW 9/22/99 - As per jasmille's comments, if you think you have found the
					// beginning of a message, make sure the previous 32 bytes are all zeros,
					// otherwise you aren't there yet, keep looking.
					bool		bAllZeros = true;
					char		c32bytes[32];
					m_infile.Seek(lMessageLocation - 32, SEEK_SET);
					m_infile.Read(c32bytes, sizeof(char) * 32);
					for (long i = 0; (i < 32) && bAllZeros; ++i)
					{
						if (c32bytes[i] != 0)
						{
							bAllZeros = false;
						}
					}
					if (bAllZeros)
					{
						bAtBeginning = true;
					}
				}
			}
		}
		else	// ahh first few messages nice!
		{
			lActualSeekPos = myTable->lMessagePosition;
		}
		
		m_infile.Seek(lActualSeekPos, SEEK_SET);					// Goto Location so we can ...
		m_infile.Read((char*)myHeader, sizeof(OE5Header));			// Read the header now that we there

		if (myHeader->lFlags == 0x01fc) return false;				// Flag for deleted message(I havent seen???)
		
		if (myHeader->lSectionSize > /*!=*/ 512 && myHeader->lSectionSize != 0) bStillOK = false;	//Not a valid message
		
		while (myHeader->lSectionSize > /*!=*/ 512 && myHeader->lSectionSize != 0)
		{
			bStillOK = false;
			lMessageLocation += 65536;								// Add padding to get to next possible location
			if (lMessageLocation > lFileLength)	return false;		// Return out if we are past end of file
			m_infile.Seek(lMessageLocation, SEEK_SET);				// Goto Location so we can ...
			m_infile.Read((char*)&lCompare, sizeof(long));			// Read it to compare 
			lActualSeekPos = lMessageLocation;						// Remember where to seek before next read
			while (lCompare != lMessageLocation)					// Keep adding 65536 til they match
			{
				lMessageLocation += 65536;							// Add padding to get to next possible location
				if (lMessageLocation > lFileLength)	return false;	// Return out if we are past end of file
				m_infile.Seek(lMessageLocation, SEEK_SET);			// Goto Location so we can ...
				m_infile.Read((char*)&lCompare, sizeof(long));		// Read it to compare 
				lActualSeekPos = lMessageLocation;					// Remember where to seek before next read
			}
			m_infile.Seek(lActualSeekPos, SEEK_SET);				// Goto Location so we can ...
			m_infile.Read((char*)myHeader, sizeof(OE5Header));		// Read the header now that we there
		}

		long	lTextSize = 0;										// Initialize to zero
		long	lTextPos = 0;										// Current read position in m_infile

		do
		{
			// Add section size (for this chunk) to text size so we can allocate
			lTextSize += myHeader->lSectionSize;	

			if (myHeader->lNextHeader == 0) break;					// No more!
			
			if (myHeader->lNextHeader > lFileLength)					// If it goes past the file dummy MS
			{
				lTextSize -= myHeader->lSectionSize;				// Find the section size and ditch it
				lTextSize += (lFileLength - lTextPos);				// Add the real size to read
				break; 
			}
			
			m_infile.Seek(myHeader->lNextHeader, SEEK_SET);			// To next header
			m_infile.Read((char*)myHeader, sizeof(OE5Header));		// Get it
		
			m_infile.Tell(&lTextPos);								// Current position(start of data chunk)	
		} while (true);												// Til there is no more
		
		szMessageText = (LPSTR) new char[lTextSize];				// Allocate space for the message

		// if smaller than first message position we overflowed our unsigned short
		if (bStillOK == false)
		{
			// Wraps over unsigned short bogus for me not needed after all
			m_infile.Seek(lMessageLocation, SEEK_SET);				// Use the correct value if overflow
		}
		else
		{
			m_infile.Seek(myTable->lMessagePosition, SEEK_SET);		// Use correct value if not
		}
		
		m_infile.Read((char*)myHeader, sizeof(OE5Header));			// Get the header again
		szPtr = szMessageText;										// Point to beginning of buffer

		do
		{
			m_infile.Read(szPtr,myHeader->lSectionSize);			// Read section
			szPtr += myHeader->lSectionSize;						// Goto end of section

			if (myHeader->lNextHeader == 0) break;					// No more, exit

			if (myHeader->lNextHeader > lFileLength)				// if bigger than file end deal withit
			{
				szPtr -= 512;										// oop should be sectionsize.but this is value of sectionsize(inconsistent)
				szPtr += (lFileLength - lTextPos);					// Add the rest of file
				break;
			}

			m_infile.Seek(myHeader->lNextHeader, SEEK_SET);			// To next chunk
			m_infile.Read((char*)myHeader, sizeof(OE5Header));		// Read the header
			m_infile.Tell(&lTextPos);								// Current position(start of data chunk)
		} while (true);												// Til no more

		szPtr = szMessageText;

		#if 0 // debugging code
		int q, z = 0, spacer;
		LPSTR Subject='\0';
		CString Holdme;
		CString Writeme;

		for (q = 0; q < lTextSize; q++)// Lazy loop to find subject..hope no XHeaders start with X-Subj
		{
			if (m_szPtr[q] == 'S')
			{
				if (m_szPtr[q+1] == 'u')
				{
					if (m_szPtr[q+2] == 'b')
					{
						if (m_szPtr[q+3] == 'j')
						{
							Subject = &m_szPtr[q];
							break;
						}
					}
				}
			}
		}

		if (Subject != NULL)
		{
			while (Subject[z]!='\r')	// finding where subject ends
			{
				z++;
			};
			Subject[/*++*/z] = '\0';
			spacer = z;					// Make it columnar
			
			Holdme = Subject;
		}
		else
		{
			Holdme = "<No Subject>";	// "<No Subject>\r\n\0";//removed because adding stuff after subj.
			spacer=12;
		}

		if (m_lMessage > 8)				// Make it columnar
		{
			spacer++;
		}
		if (m_lMessage > 98)			// Make it columnar
		{
			spacer++;
		}
			
		char TitleNumber[6];	// To write Message # inf ront of subject
		sprintf(TitleNumber,"%ld",m_SubjectNumber++);
		Writeme += TitleNumber;
		Writeme += ". " + Holdme;
		for(spacer; spacer < 120; spacer++)//Make it columnar
		{
			Writeme += " ";
		}

		switch (myTable->cbRead)		// Flagged message and read/unread byte
		{
			case 1:		{Writeme += "Unread      ";			break;}
			case 33:	{Writeme += "UFlagged    ";			break;}
			case 129:	{Writeme += "Read        ";			break;}
			case 161:	{Writeme += "RFlagged    ";			break;}
			default:	{Writeme += "Unknown     ";			break;}
		}

		switch (myTable->cbAttach)		// if attachment or not byte
		{
			case 0:		{Writeme += "No Attachment    ";	break;}
			case 64:	{Writeme += "Attachment       ";	break;}
			default:	{Writeme += "Unknown          ";	break;}
		}

		switch (myTable->cbStatus)		// Whether replied forwarded or none byte
		{
			case 0:		{Writeme += "Unread      ";			break;}
			case 2:		{Writeme += "Read        ";			break;}
			case 10:	{Writeme += "Replied     ";			break;}
			case 18:	{Writeme += "Forwarded   ";			break;}
			default:	{Writeme += "Unknown     ";			break;}
		}
		
		switch (myTable->cbImportant)	//Important byte
		{
			case 1:		{Writeme += "High Priority";		break;}
			case 3:		{Writeme += "Regular Priority";		break;}
			case 5:		{Writeme += "Low Priority";			break;}
			case 123:	{Writeme +="Unread Regular";		break;}
			default:	{Writeme += "Unknown";				break;}
		}

		Writeme += "\r\n";// next line
		#endif // debugging code

		// DRW 9/15/99 - At this point, szMessageText should contain the text
		// of the message to import.  Save this text to the temporary file which will eventually
		// be read in as the message.
		m_tempfile.Put(szMessageText, lTextSize);

		delete [] szMessageText;

		delete myHeader;
		delete myTable;
	}

	return true;
}

bool OEMboxConverter::CloseTempFile()
{
	m_tempfile.Flush();
	m_tempfile.Close();
	return true;
}

bool OEMboxConverter::CloseOEFile()
{
	m_infile.Flush();
	m_infile.Close();
	return true;
}

char *OEMboxConverter::FindRealName(char *pathname)
{
	char *returnstring = NULL;
	char *mbxName = NULL;
	char * temp;
	char * floater;
	Mbox_FnamePair * pRnames = m_realNames;
	if (!pathname)
		return NULL;

	temp = new char[strlen(pathname)+1];
	strcpy(temp, pathname);
	floater = strrchr(temp, '.');
	if (floater)
		*floater = 0;
	floater = strrchr(temp, '\\');
	if (floater)
		floater++;	// Skip the slash.
	else
		floater = pathname;

	mbxName = new char[strlen(floater)+1];
	strcpy(mbxName, floater);

	delete [] temp;

	if (!pRnames)	// unpopulated list, just call it the mbox name
	{	
		return mbxName;
	}
	else if (!strcmp(pRnames->Filename, mbxName))// First entry was a match
	{
		delete [] mbxName;
		returnstring = new char[strlen(pRnames->DisplayName)+1];
		strcpy(returnstring, pRnames->DisplayName);
		return returnstring;
	}
	else while(pRnames->next)
	{
		pRnames = pRnames->next;
		if (!strcmp(pRnames->Filename, mbxName))
		{
			delete [] mbxName;
			returnstring = new char[strlen(pRnames->DisplayName)+1];
			strcpy(returnstring, pRnames->DisplayName);
			return returnstring;
		}
	}
	return mbxName;		// Didn't find this name in the list.
}



/////////////////////////////////////
// Create the Outlook express folder with the help of the mailbox director.
// DRW 9/21/99 - Added Type parameter so caller can specify type of mailbox.
////////
QCMailboxCommand * OEMboxConverter::CreateMbox(char *name, QCMailboxCommand *ParentFolderCmd, MailboxType Type)
{
	QCMailboxCommand * NewFolderCmd = NULL;
	char * Filename = NULL;
	// DRW 9/21/99 - Made Type a parameter.
	// MailboxType Type;

	if (!name)
	{
		Filename = new char[21];
		strcpy(Filename, OE_DISPLAY_NAME);
		// DRW 9/21/99 - Made Type a parameter.
		// Type = MBT_FOLDER;
	}
	else
	{
		Filename = new char[strlen(name)+1];
		strcpy(Filename, name);
		// DRW 9/21/99 - Made Type a parameter.
		// Type = MBT_REGULAR;
	}

	UniquifyName(&Filename, ParentFolderCmd);

	NewFolderCmd = g_theMailboxDirector.AddCommand(Filename, Type, ParentFolderCmd);

	delete [] Filename;

	if (Type == MBT_REGULAR)
	{
		char *pathToMBX;
		pathToMBX = new char[((NewFolderCmd->GetPathname()).GetLength())+1];
		strcpy(pathToMBX, NewFolderCmd->GetPathname());
		KillTheTOCFile(pathToMBX);
		delete [] m_OutFilePathname;
		m_OutFilePathname = new char[strlen(pathToMBX)+1];
		strcpy(m_OutFilePathname, pathToMBX);
		delete [] pathToMBX;
	}

	return NewFolderCmd;
}

//////////////////////////////////////
// FindToken
//////////////////////
// Read one byte at a time, this is slow, but it's only for a few hundred bytes,
// the mass email moving stuff is read in chunks.
// Does it's reading/seeking from m_infile.

bool OEMboxConverter::FindToken() // testtoken should default to 0x00000000
{
	char buffer[2] = {0x00, 0x00};
	char testtoken[4] = {0x00, 0x00, 0x00, 0x00};
	char magictoken[4] = {0x00, 0x7F, 0x00, 0x7F};
	int i;
	
//	i = m_infile.Read(buffer,1);

	while ((memcmp (magictoken, testtoken,4) != 0) && !(m_infile.Read(buffer,1)))
	{
		testtoken[0] = testtoken[1];
		testtoken[1] = testtoken[2];
		testtoken[2] = testtoken[3];
		testtoken[3] = buffer[0];
	}
	
	if (memcmp (magictoken, testtoken,4) != 0)
		return false;

	for (i = 1; i < 13; i++)
	{
		if (m_infile.Read(buffer,1))
			return false;
	}
	return true;
}

///////////////////////////////////
//	FindOEsPath
///////////////////
// Reads registry and determines where the user's mail store is located since we're going to read the
// raw files.
bool OEMboxConverter::FindOEsPath() //Good
{
	HKEY	hKey;
	char szData[512];
	DWORD dwKeyDataType;
	DWORD dwDataBufSize;
	// DRW 9/13/99 - Removed bFoundOE5 declaration since it is now a member.

	delete [] m_PathToMail;
	m_PathToMail = NULL;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Identities"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		dwDataBufSize = sizeof(szData)/sizeof(szData[0]);
		if (RegQueryValueEx(hKey, _T("Last User ID"), NULL, &dwKeyDataType,
			(LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
		{
			HKEY hKeyOE;
			strcat(szData, "\\Software\\Microsoft\\Outlook Express");
			if (RegOpenKeyEx(hKey, szData, 0, KEY_READ, &hKeyOE) == ERROR_SUCCESS)
			{
				TCHAR Version[64];
				DWORD dwVersionSize = sizeof(Version);
				double HighestVersion = 0;
				for (DWORD dwIndex = 0;
					RegEnumKeyEx(hKeyOE, dwIndex, Version, &dwVersionSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
					dwIndex++)
				{
					double ThisVersion = atof(Version);
					if (ThisVersion > HighestVersion)
					{
						HKEY hKeyOEVersion;
						if (RegOpenKeyEx(hKeyOE, Version, 0, KEY_READ, &hKeyOEVersion) == ERROR_SUCCESS)
						{
							dwDataBufSize = sizeof(szData)/sizeof(szData[0]);
							if (RegQueryValueEx(hKeyOEVersion, _T("Store Root"), NULL, &dwKeyDataType,
								(LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
							{
								if (ThisVersion >= 5.0)
								{
									m_bFoundOE5 = TRUE;
									// DRW 9/14/99 - We've gotten this far, remember what we just found.
									m_PathToMail = new char[_MAX_PATH + 1];
									if (m_PathToMail)
									{
										// DRW 9/14/99 - Expand the %UserProfile% portion.
										if (ExpandEnvironmentStrings(szData, m_PathToMail, _MAX_PATH + 1) == 0)
										{
											delete [] m_PathToMail;
											m_PathToMail = NULL;
										}
										else
										{
											char	szOE5FolderFile[_MAX_PATH + 1];
											strcpy(szOE5FolderFile, m_PathToMail);
											if (szOE5FolderFile[strlen(szOE5FolderFile) - 1] != '\\')
											{
												strcat(szOE5FolderFile, _T("\\"));
											}
											strcat(szOE5FolderFile, _T("folders.dbx"));
											struct _stat		st;									// File info (for file size)
											if (_stat(szOE5FolderFile, &st) == -1)
											{
												m_bFoundOE5 = false;
											}
										}
									}
								}
								HighestVersion = ThisVersion;
							}
							RegCloseKey(hKeyOEVersion);
						}
					}
				}
				RegCloseKey(hKeyOE);
			}
		}

		RegCloseKey(hKey);
	}

	// DRW 9/13/99 - Added OE5 handling.
	if (!m_bFoundOE5)
	{
		if (RegOpenKeyEx(HKEY_CURRENT_USER,  _T("Software\\Microsoft\\Outlook Express"),
			0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			dwDataBufSize = sizeof(szData)/sizeof(szData[0]);
			if (RegQueryValueEx(hKey, _T("Store Root"), NULL, &dwKeyDataType,
				(LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
			{
				if (!m_bFoundOE5 || YesNoDialogPlain(IDS_IMPORT_MAIL_OE4_AND_OE5_FOUND) == IDOK)
				{
					m_PathToMail = new char[dwDataBufSize+strlen(_T("\\Mail"))+1];
					strcpy(m_PathToMail,(const char *)&szData);
					strcat(m_PathToMail, _T("\\Mail"));
				}
				else
					return (false);
			}
			RegCloseKey(hKey);
		}

		if (m_PathToMail)
			return (true);
	}
	else
	{
		// DRW 9/14/99 - If we found OE5, we already know the mail path.
		if (m_PathToMail)
		{
			return true;
		}
	}

	ErrorDialog(IDS_IMPORT_MAIL_NO_OE_FOUND_ERROR);

	return (false);
}

bool OEMboxConverter::FindEudorasPath()
{

	m_PathToEudoraMail = new char[strlen(EudoraDir)+1];
	strcpy(m_PathToEudoraMail,EudoraDir);

/*	if (stricmp(m_PathToMail, m_PathToEudoraMail) == 0)
	{
		// uh oh, same directory. What do we do?
		return (false);
	}
*/
	if ( _chdir(m_PathToMail))// OK, now go there.
	{
			// Error, can't go there!!!
		return (false);
	}

	return (true);
}

bool OEMboxConverter::InitMboxFind()
{
	// DRW 9/13/99 - Added OE5 handling.
	if (!m_bFoundOE5)
	{
		m_bWorking = m_finder.FindFile("*.mbx");
	}
	else
	{
		// DRW 9/13/99 - We already determined the files in
		// BuildRealFolderNames() so we don't actually search for files here.
		// There is a good reason for this: searching for all dbx files would
		// find at least one file that isn't actually a mailbox file (the folder
		// index file: folders.dbx).  To avoid confusion, just use the list
		// we already created.
		m_mbfpCurrMBox = m_realNames;
		m_bWorking = true;
	}
	if (m_bWorking)
		return true;
	else
		return false;
}

bool OEMboxConverter::GrabNextMbox()
{
	delete [] m_InFilePathname;
	m_InFilePathname = NULL;

	if (!m_bFoundOE5)
	{
		// Enumerate the source files.
		m_bWorking = m_finder.FindNextFile();
		if (m_bWorking)
		{
			m_InFilePathname = new char[_MAX_PATH + 1];
			strcpy(m_InFilePathname, m_finder.GetFilePath());
		}
	}
	else
	{
		if (m_mbfpCurrMBox)
		{
			m_InFilePathname = new char[_MAX_PATH + 1];
			strcpy(m_InFilePathname, m_PathToMail);
			if (m_InFilePathname[strlen(m_InFilePathname) - 1] != '\\')
			{
				strcat(m_InFilePathname, _T("\\"));
			}
			strcat(m_InFilePathname, m_mbfpCurrMBox->Filename);
//			m_mbfpCurrMBox = m_mbfpCurrMBox->next;
		}
	}

	if (m_InFilePathname == NULL)
	{
		return false;
	}

	return true;
}

// Return the OE specified parent mailbox for the mailbox currently being imported.
// If we are imported OE4 always return m_pRootCommand so the behavior doesn't change.
QCMailboxCommand *OEMboxConverter::GetParentMbox()
{
	QCMailboxCommand	*pqcmbParent = m_pRootCommand;
	if (m_bFoundOE5)
	{
		if (m_mbfpCurrMBox &&
			(m_mbfpCurrMBox->lParentMBoxID != 0) &&		// Parent ID of 0 indicates no parent specified
			(m_mbfpCurrMBox->lParentMBoxID != 1))		// Parent ID of 1 indicates parent is root
		{
			Mbox_FnamePair *		pmbfp = m_realNames;
			bool					bMatchFound = false;
			while (pmbfp && !bMatchFound)
			{
				if (pmbfp->lMBoxID == m_mbfpCurrMBox->lParentMBoxID)
				{
					pqcmbParent = pmbfp->pMailbox;
					bMatchFound = true;
				}
				else
				{
					pmbfp = pmbfp->next;
				}
			}
		}
	}
	return pqcmbParent;
}

bool OEMboxConverter::OpenOEFile()
{
	if (!m_InFilePathname) return (false);

	if(FAILED(m_infile.Open(m_InFilePathname,O_RDONLY)))
	{
		// Argh
		return (false);
	}
	return (true);
}


bool OEMboxConverter::OpenTempFile()
{
	if (!m_TempFilePathname)
	{
		m_TempFilePathname = _tempnam(".","EUD");
	}

	if(FAILED(m_tempfile.Open(m_TempFilePathname, O_CREAT | O_RDWR | O_TRUNC)))
	{
		return (false);
	}

	return (true);
}


AbookEntry::AbookEntry()
{
	notes			= NULL;
	wrk_office		= NULL;
	wrk_dept		= NULL;
	wrk_title		= NULL;
	company			= NULL;
	wrk_pgr			= NULL;
	wrk_fax			= NULL;
	wrk_phone		= NULL;
	wrk_web_page	= NULL;
	wrk_country		= NULL;
	wrk_state		= NULL;
	wrk_zip			= NULL;
	wrk_city		= NULL;
	wrk_street_addr	= NULL;
	cell_phone		= NULL;
	fax				= NULL;
	phone			= NULL;
	web_page		= NULL;
	country			= NULL;
	state			= NULL;
	zip				= NULL;
	city			= NULL;
	street_addr		= NULL;
	first_name		= NULL;
	middle_name		= NULL;
	last_name		= NULL;
	nickname		= NULL;
	addresses		= NULL;
	type			= 0;
	next			= NULL;
}

AbookEntry::~AbookEntry()
{
	delete [] notes;
	delete [] wrk_office;
	delete [] wrk_dept;
	delete [] wrk_title;
	delete [] company;
	delete [] wrk_pgr;
	delete [] wrk_fax;
	delete [] wrk_phone;
	delete [] wrk_web_page;
	delete [] wrk_country;
	delete [] wrk_state;
	delete [] wrk_zip;
	delete [] wrk_city;
	delete [] wrk_street_addr;
	delete [] cell_phone;
	delete [] fax;
	delete [] web_page;
	delete [] country;
	delete [] state;
	delete [] zip;
	delete [] city;
	delete [] street_addr;
	delete [] first_name;
	delete [] middle_name;
	delete [] last_name;
	delete [] nickname;
	delete [] addresses;
	delete [] next;
	delete [] phone;
}

char *	FindTitle(char *inFilename)
{		char *filename;
		char * floater;
		char* floater2;
		floater = strrchr(inFilename, '\\');
		if (!floater)
		{
			floater = inFilename;
		}
		else
			floater++;

		floater2 = strrchr(inFilename, '.');
		if (floater)
		{
			*floater2 = 0;
		}
		else
			return NULL;
				
		filename = new char[strlen(floater)+1];	//ldif

		strcpy(filename, floater);
		*floater2 = '.';

		return filename;
}

bool NSLdifConverter::Convert(LPCSTR File)	
{
	bool done = false;
	JJFile *theFile;
	AbookEntry *Entry;
	AbookCreator *abookcreator;
	abookcreator = new AbookCreator;
	char *NNFilePath = NULL;
	char *filename	= NULL; 
	char *inFilename = new char[strlen(File)+1];
	strcpy(inFilename, File);

	if (inFilename == NULL)
	{
		return false;
	}
	theFile = new JJFile;
	theFile->Open(inFilename, O_RDONLY);

	filename = FindTitle(inFilename);

	if (!abookcreator->FindNNFileName(&filename, &NNFilePath))
		return false;

	if (!abookcreator->OpenNNFile(NNFilePath))
		return false;

	bool MoreEntries = true;
	while (MoreEntries)
	{
		Entry = GetNextEntry(theFile);
		if (!Entry)
		{
			MoreEntries = false;
			break;
		}

		abookcreator->AddEntry(Entry);
		delete Entry;
	}

	abookcreator->CloseNNFile();

	theFile->Close();
	delete theFile;
	delete [] inFilename;

	return true;
}

bool NSLdifConverter::Convert(ImportChild* Child)	
{
	char *inFilename;
	bool done = false;
	JJFile *theFile;
	AbookEntry *Entry;
	AbookCreator *abookcreator;
	abookcreator = new AbookCreator;
	char *NNFilePath = NULL;
	char *filename	= NULL;   
	
	while (!done)
	{
		inFilename = FindNextFile(Child->PathToData);
		if (inFilename == NULL)
		{
			done = true;
			break;
		}
		theFile = new JJFile;
		theFile->Open(inFilename, O_RDONLY);

		filename = FindTitle(inFilename);

		if (!abookcreator->FindNNFileName(&filename, &NNFilePath))
			return false;

		if (!abookcreator->OpenNNFile(NNFilePath))
			return false;

		bool MoreEntries = true;
		while (MoreEntries)
		{
			Entry = GetNextEntry(theFile);
			if (!Entry)
			{
				MoreEntries = false;
				break;
			}

			abookcreator->AddEntry(Entry);
			delete Entry;
		}

		abookcreator->CloseNNFile();

		theFile->Close();
		delete theFile;
		delete [] inFilename;
	}

	return true;
}

char * NSLdifConverter::FindNextFile(char *basePath)
{
	char *Pathname;
	struct _finddata_t c_file;

	if (!m_findBegun && basePath != NULL)
	{	
		m_findBegun = true;
		_chdir(basePath);

		if( (m_hFile = _findfirst( "*.ldif", &c_file )) == -1L )
			return NULL;

		else
		{
			Pathname = new char[strlen(c_file.name)+1];
			strcpy(Pathname, c_file.name);
			return Pathname;
		}
	}

	if ( _findnext( m_hFile, &c_file ) == 0 )
	{
		Pathname = new char[strlen(c_file.name)+1];
		strcpy(Pathname, c_file.name);
		return Pathname;
	}
	else
		return NULL;
}

AbookEntry *NSLdifConverter::GetNextEntry(JJFile *theFile)
{
	char *line = NULL;
	line = new char [513];
	long numBytes;
	bool stop = false;
	int i;
	AbookEntry *Entry;
	Entry = new AbookEntry;
	bool done = false;
	char * objectclass = NULL;
	char *LastTag = NULL;
	char *Tag = NULL;

	// Get a line, if it's a blank line try again like 100 times, then return NULL
	// If it's a dn line then you're off to the races. 
	for(i=0; i < 100 && !stop; i++)
	{
		 theFile->GetLine(line, 512, &numBytes);
		 if (line[0] != 0)
		 {
			LastTag = new char[strlen(_T("dn:"))+1];
			strcpy(LastTag, _T("dn:"));
			stop = true;
		 }
	}
	if (i >= 100)
		return NULL;

	while (!done && line[0] != 0 && !HR_FAILED(theFile->GetLine(line, 100, &numBytes)))
	{
		char * floater;
		if (line[0] == ' ')	// Continuation from previous line
		{
			Tag = LastTag;
			floater = &line[1];
		}
		else
		{
			floater = strchr(line, ' ');
			if (!floater)
			{
				done = true;
				break;
			}
			else
				*floater = 0;

			Tag = new char[strlen(line)+1];
			strcpy(Tag, line);

			LastTag ? delete [] LastTag : NULL;
			LastTag = new char[strlen(line)+1];
			strcpy(LastTag, line);


			*floater = ' ';		// Set it back

			while (*floater == ' ')
				floater++;		// Set it to the first character of the string.
		}

		if(!strcmp(Tag, _T("givenname:")))
		{
			if (Entry->first_name)
			{
				char *temp;
				temp = Entry->first_name;
				Entry->first_name = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->first_name, temp);
				strcat(Entry->first_name, floater);
				delete [] temp;
			}
			else
			{
				Entry->first_name = new char[strlen(floater)+1];
				strcpy(Entry->first_name, floater);
			}
		}
		else if (!strcmp(Tag, _T("sn:")))
		{
			if (Entry->last_name)
			{
				char *temp;
				temp = Entry->last_name;
				Entry->last_name = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->last_name, temp);
				strcat(Entry->last_name, floater);
				delete [] temp;
			}
			else
			{
				Entry->last_name = new char[strlen(floater)+1];
				strcpy(Entry->last_name, floater);	
			}
		}
		else if (!strcmp(Tag, _T("telephonenumber:")))
		{
			if (Entry->wrk_phone)
			{
				char *temp;
				temp = Entry->wrk_phone;
				Entry->wrk_phone = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_phone, temp);
				strcat(Entry->wrk_phone, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_phone = new char[strlen(floater)+1];
				strcpy(Entry->wrk_phone, floater);
			}
		}
		else if (!strcmp(Tag, _T("locality:")))
		{
			if (Entry->wrk_city)
			{
				char *temp;
				temp = Entry->wrk_city;
				Entry->wrk_city = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_city, temp);
				strcat(Entry->wrk_city, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_city = new char[strlen(floater)+1];
				strcpy(Entry->wrk_city, floater);	
			}
		}
		else if (!strcmp(Tag, _T("st:")))
		{
			if (Entry->wrk_state)
			{
				char *temp;
				temp = Entry->wrk_state;
				Entry->wrk_state = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_state, temp);
				strcat(Entry->wrk_state, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_state = new char[strlen(floater)+1];
				strcpy(Entry->wrk_state, floater);	
			}
		}
		else if (!strcmp(Tag, _T("title:")))
		{
			if (Entry->wrk_title)
			{
				char *temp;
				temp = Entry->wrk_title;
				Entry->wrk_title = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_title, temp);
				strcat(Entry->wrk_title, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_title = new char[strlen(floater)+1];
				strcpy(Entry->wrk_title, floater);		
			}
		}
		else if (!strcmp(Tag, _T("postOfficeBox:")))
		{
			if (Entry->wrk_street_addr)
			{
				char *temp = Entry->wrk_street_addr;
				Entry->wrk_street_addr = new char[strlen(temp)+strlen(floater)+3];
				strcpy(Entry->wrk_street_addr, floater);
				strcat(Entry->wrk_street_addr, _T("\r\n"));
				strcat(Entry->wrk_street_addr, temp);
				delete [] temp;
			}
			else
			{
				Entry->wrk_street_addr = new char[strlen(floater)+1];
				strcpy(Entry->wrk_street_addr, floater);
			}
		}
		else if (!strcmp(Tag, _T("streetaddress:")))
		{
			if (Entry->wrk_street_addr)
			{
				char *temp = Entry->wrk_street_addr;
				Entry->wrk_street_addr = new char[strlen(temp)+strlen(floater)+3];
				strcpy(Entry->wrk_street_addr, temp);
				strcat(Entry->wrk_street_addr, _T("\r\n"));
				strcat(Entry->wrk_street_addr, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_street_addr = new char[strlen(floater)+1];
				strcpy(Entry->wrk_street_addr, floater);				
			}
		}

		else if (!strcmp(Tag, _T("postalcode:")))
		{
			if (Entry->wrk_zip)
			{
				char *temp;
				temp = Entry->wrk_zip;
				Entry->wrk_zip = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_zip, temp);
				strcat(Entry->wrk_zip, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_zip = new char[strlen(floater)+1];
				strcpy(Entry->wrk_zip, floater);		
			}
		}
		else if (!strcmp(Tag, _T("countryname:")))
		{
			if (Entry->wrk_country)
			{
				char *temp;
				temp = Entry->wrk_country;
				Entry->wrk_country = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_country, temp);
				strcat(Entry->wrk_country, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_country = new char[strlen(floater)+1];
				strcpy(Entry->wrk_country, floater);
			}
		}
		else if (!strcmp(Tag, _T("facsimiletelephonenumber:")))
		{
			if (Entry->wrk_fax)
			{
				char *temp;
				temp = Entry->wrk_fax;
				Entry->wrk_fax = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->wrk_fax, temp);
				strcat(Entry->wrk_fax, floater);
				delete [] temp;
			}
			else
			{
				Entry->wrk_fax = new char[strlen(floater)+1];
				strcpy(Entry->wrk_fax, floater);	
			}
		}
		else if (!strcmp(Tag, _T("homephone:")))
		{
			if (Entry->phone)
			{
				char *temp;
				temp = Entry->phone;
				Entry->phone = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->phone, temp);
				strcat(Entry->phone, floater);
				delete [] temp;
			}
			else
			{
				Entry->phone = new char[strlen(floater)+1];
				strcpy(Entry->phone, floater);
			}
		}
		else if (!strcmp(Tag, _T("o:")))
		{
			if (Entry->company)
			{
				char *temp;
				temp = Entry->company;
				Entry->company = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->company, temp);
				strcat(Entry->company, floater);
				delete [] temp;
			}
			else
			{
				Entry->company = new char[strlen(floater)+1];
				strcpy(Entry->company, floater);
			}
		}
		else if (!strcmp(Tag, _T("xmozillanickname:")))
		{	
			if (Entry->nickname)
			{
				char *temp;
				temp = Entry->nickname;
				Entry->nickname = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->nickname, temp);
				strcat(Entry->nickname, floater);
				delete [] temp;
			}
			else
			{
				Entry->nickname = new char[strlen(floater)+1];
				strcpy(Entry->nickname, floater);	
			}
		}
		else if (!strcmp(Tag, _T("objectclass:")))
		{
			if (objectclass)
			{
				char * temp = objectclass;
				objectclass = new char[strlen(temp)+strlen(floater)+2];
				strcpy(objectclass, temp);
				strcat(objectclass, _T(" "));
				strcat(objectclass, floater);
				delete [] temp;
			}
			else
			{
				objectclass = new char[strlen(floater)+1];
				strcpy(objectclass, floater);
			}
		}
		else if (!strcmp(Tag, _T("mail:")))
		{
			Entry->addresses = new char[strlen(floater)+1];
			strcpy(Entry->addresses, floater);
		}
		else if (!strcmp(Tag, _T("description:")))
		{
			if (Entry->notes)
			{
				char *temp;
				temp = Entry->notes;
				Entry->notes = new char[strlen(temp)+strlen(floater)+1];
				strcpy(Entry->notes, temp);
				strcat(Entry->notes, floater);
				delete [] temp;
			}
			else
			{
				Entry->notes = new char[strlen(floater)+1];
				strcpy(Entry->notes, floater);
			}
		}
		if (Tag != LastTag)		// If we've just reused LastTag then we shouldn't be deleting it.
			delete [] Tag;
	}
	if (objectclass)
	{
		if (!strstr(objectclass, _T("person")))
		{
			delete Entry;
			delete [] line;
			delete [] LastTag;
			delete [] objectclass;
			return GetNextEntry(theFile);	// try the next record then...
		}
	}

	// Now the entry is fully filled out.
	if (  ((Entry->first_name) ||(Entry->last_name)) && Entry->addresses)
	{
		delete [] line;
		delete [] LastTag;
		delete [] objectclass;
		return Entry;
	}
	else
	{
		delete Entry;
		delete [] line;
		delete [] LastTag;
		delete [] objectclass;
		return GetNextEntry(theFile);	// try the next record then...
	}
}

NSLdifConverter::NSLdifConverter()
{
	m_findBegun = false;
	m_hFile = 0;
}

NSLdifConverter::~NSLdifConverter()
{

}

bool AbookCreator::FindNNFileName(char **name, char **fullpath)
{
	char * floater;
	char * floater2;
	char * temp;

	floater = strstr(*name, _T(".txt")); 
	if (!floater)	// Improper extension
	{
		floater2 = strrchr(*name, '.');
		if (!floater2) // No extension
		{
			temp = new char[ strlen(*name) + 5];
			strcpy(temp, *name);
			strcat(temp, _T(".txt"));
		}
		else	// There's an extension, just the wrong one.
		{
			*floater2 = 0;
			temp = new char[strlen(*name) + 5];
			strcpy(temp, *name);
			strcat(temp, _T(".txt"));
		}
		delete [] *name;
		*name = temp;
	}

	while (NicknameFileExists(*name))
	{
		floater = strrchr(*name, '-');
		if (!floater)
		{
			char * newname;
			newname = new char [strlen(*name)+3];	// -1\0
			strcpy(newname, *name);
			floater = strstr(newname, _T(".txt"));
		
			if (!floater)
				return false;	// If this happens I'll eat a bug

			*floater = 0;

			strcat(newname, _T("-1"));
			strcat(newname, _T(".txt"));
			delete [] *name;
			*name = newname;
		}
		else // we already have the dash -X, we need to increment.
		{
			floater++;
			char *	num;
			int version;
			int firstlen;
			int lastlen;

			version = atoi(floater);
			firstlen = strlen(floater);
			version++;

			num = new char[strlen(_T(".txt")) + firstlen+2];
			sprintf(num, "%d.txt", version);
			lastlen = strlen(num);
			if (lastlen > firstlen)
			{
				char * newname;
				newname = new char[strlen(*name)+(lastlen-firstlen)+1];
				strcpy(newname, *name);
				delete [] *name;
				*name = newname;
			}
				floater = strrchr(*name, '-');
				ASSERT(floater);

				floater++;
				strcpy(floater, num);
				delete [] num;
		}

	}
	*fullpath = new char[strlen(EudoraDir) + strlen(CRString(IDS_NICK_DIR_NAME)) + strlen(*name) + 2]; // one for the slash one for the null
	sprintf(*fullpath, "%s%s\\%s", EudoraDir, CRString(IDS_NICK_DIR_NAME), *name);

	return true;
}

// This uses a complex algorithm to figure out what the nickname for this entry should be. 
// Ok, not really.

bool AbookCreator::FormNickname(AbookEntry *Entry, char **Nickname)
{
	char *floater;
	char *floater2;
	if (Entry->first_name && Entry->last_name)
	{
		*Nickname = new char[strlen(Entry->first_name) + strlen(Entry->last_name) + 2];
		sprintf(*Nickname, "%s_%s", Entry->first_name, Entry->last_name);
	}
	else if (!Entry->first_name && Entry->last_name)
	{
		*Nickname = new char[strlen(Entry->last_name) + 1];
		strcpy(*Nickname, Entry->last_name);
	}
	else if (!Entry->last_name && Entry->first_name)
	{
		*Nickname = new char[strlen(Entry->first_name) + 1];
		strcpy(*Nickname, Entry->first_name);
	}
	else	// No names, we'll pull it from the email address
	{
		if (!Entry->addresses)
		{			// Lost cause, no first name, no last name and no addresses...
			return false;
		}
		floater = strchr(Entry->addresses, '@');
		floater2 = strchr(Entry->addresses, ' ');
		if (!floater && !floater2)
		{
			return false;		// Oh bah!
		}

		if ((floater2 != NULL) && floater2 < floater)		// We prefer spaces to the @ sign
		{
			*Nickname = new char[(floater2 - (Entry->addresses))+1];
			strncpy(*Nickname, Entry->addresses, floater2 - (Entry->addresses));
		}
		else
		{
			*Nickname = new char[(floater - (Entry->addresses))+1];
			strncpy(*Nickname, Entry->addresses, floater - (Entry->addresses));
		}
	}

	floater = *Nickname;
	while (floater != NULL)		// Change spaces to Underscores.
	{
		floater = strchr(*Nickname, ' ');
		if (floater)
			*floater = '_';
	}

	floater = *Nickname;
	while (floater != NULL)		// Change commas to Underscores.
	{
		floater = strchr(*Nickname, ',');
		if (floater)
			*floater = '_';
	}

	if (strlen(*Nickname) > CNickname::MAX_NAME_LENGTH)
	{
		(*Nickname)[CNickname::MAX_NAME_LENGTH] = 0;
	}

	UniquifyNickname(Nickname);
	
	AddToList(*Nickname);

	return true;
}

AbookCreator::AbookCreator()
{
	m_head = NULL;
	m_NNFile = NULL;
}

AbookCreator::~AbookCreator()
{
	
}

void AbookCreator::UniquifyNickname(char **nickname)
{
	
}

void AbookCreator::AddToList(char * nickname)
{
	


}

// I'm sure there's a much simpler way to resolve this, mainly I
// just want to take linefeeds and swap them to the hex value
// 0x03. sometimes there will be both a CR and an LF, have to check...
CString AbookCreator::stripLines(char *source)
{
	CString	returnMe;
	char * line = NULL;
	char * head;
	char * floater;
	char * floater2;
	bool done = false;

	head = source;
	floater = source;

	returnMe.Empty();
	if (!*source)
		return returnMe;

	while (!done)
	{
		if (!returnMe.IsEmpty())
			returnMe += 3;

		floater = strchr(head, '\r');
		floater2 = strchr(head, '\n');

		if (floater > floater2)		// We want to make sure floater is the lower of the two so the following crap works
		{
			char * temp;
			temp = floater2;
			floater2 = floater;
			floater = temp;
		}

		if (!floater && !floater2)
		{
			line = new char[strlen(head)+1];
			strcpy(line, head);
			done = true;
		}
		else if (!floater)
		{
			char ch;
			line = new char[(floater2 - head) +1];
			ch = *floater2;
			*floater2 = 0;
			strcpy(line, head);
			*floater2 = ch;

			while (*floater2 == '\r' || *floater2 == '\n')
				floater2++;

			if (*floater2 == 0)
				done = true;

			head = floater2;
		}
		else if (!floater2)
		{
			char ch;
			line = new char[(floater - head) +1];
			ch = *floater;
			*floater = 0;
			strcpy(line, head);
			*floater = ch;
			
			while (*floater == '\r' || *floater == '\n')
				floater++;

			if (*floater == 0)
				done = true;
			
			head = floater;
		}
		else		// Both exist
		{
			char ch;
			line = new char[(floater - head) +1];
			ch = *floater;
			*floater = 0;
			strcpy(line, head);
			*floater = ch;

			while (*floater == '\r' || *floater == '\n')
				floater++;

			if (*floater == 0)
				done = true;
			
			head = floater;	
		}
		returnMe += line;
		delete [] line;
	}
	return returnMe;
}

bool AbookCreator::FormNotes(AbookEntry *Entry, char * nickname, char **line)
{
	CString fax;
	CString phone;
	CString st_address;
	CString name;
	CString notes;
	CString nick = nickname;
	CString temp;

	// If there aren't any interesting fields then just skip this whole thing, we don't want an empty notes field.
	if (!Entry->notes && !Entry->wrk_office && !Entry->wrk_dept && !Entry->wrk_title && !Entry->company && !Entry->wrk_pgr && !Entry->wrk_fax && !Entry->wrk_phone && !Entry->wrk_web_page && !Entry->wrk_country
		&& !Entry->wrk_state && !Entry->wrk_zip && !Entry->wrk_city && !Entry->wrk_street_addr && !Entry->cell_phone && !Entry->fax && !Entry->phone && !Entry->web_page && !Entry->country && !Entry->state
		&& !Entry->zip && !Entry->city && !Entry->street_addr && !Entry->first_name && !Entry->middle_name && !Entry->last_name && !Entry->nickname)
	{
		*line = NULL;
		return true;
	}

	if (Entry->fax)
	{
		fax = stripLines(Entry->fax);
	}
	else if (Entry->wrk_fax)
	{
		fax = stripLines(Entry->wrk_fax);
	}

	if (Entry->phone)
	{
		phone = stripLines(Entry->phone);
	}
	else if(Entry->wrk_phone)
	{
		phone = stripLines(Entry->wrk_phone);
	}

	if (Entry->street_addr)
	{
		st_address = stripLines(Entry->street_addr);
	}
	else if (Entry->wrk_street_addr)
	{
		st_address = stripLines(Entry->wrk_street_addr);
	}


	if (Entry->first_name)
	{
		name = stripLines(Entry->first_name);

		if (Entry->last_name || Entry->middle_name)
		{
			name += ' ';
		}
	}
	else
		name.Empty();

	if (Entry->middle_name)
	{
		name += stripLines(Entry->middle_name);

		if (Entry->last_name)
		{
			name += ' ';
		}
	}
	if (Entry->last_name)
	{
		name += stripLines(Entry->last_name);
	}


	temp = "note " + nick + " ";

	if (!fax.IsEmpty())
	{
		temp += "<fax:" + fax + ">";
	}

	if (!phone.IsEmpty())
	{
		temp += "<phone:" + phone + ">";
	}

	if (!st_address.IsEmpty())	// Need stuff to handle line breaks....
	{
		temp += "<address:" + st_address + ">";
	}

	if (!name.IsEmpty())
	{
		temp += "<name:" + name + ">";
	}

	if (Entry->notes)
	{
		temp += stripLines(Entry->notes);
		temp += 3;
	}

	// Are there any Work fields? If not, lets skip the WORK: section.
	if (Entry->wrk_office || Entry->wrk_dept || Entry->wrk_title || Entry->company || Entry->wrk_pgr || Entry->wrk_fax || Entry->wrk_phone || Entry->wrk_web_page || Entry->wrk_country
		|| Entry->wrk_state || Entry->wrk_zip || Entry->wrk_city || Entry->wrk_street_addr)
	{
		temp += 3;
		temp += "Work:";
		temp += 3;

		if (Entry->company)
		{
			temp += stripLines(Entry->company);
			temp += 3;
		}

		if (Entry->wrk_office)
		{
			temp += stripLines(Entry->wrk_office);
			if (Entry->wrk_dept)
			{
				temp += ", ";
				temp += stripLines(Entry->wrk_dept);
				temp += 3;
			}
		}
		else if (Entry->wrk_dept)
		{
			temp += stripLines(Entry->wrk_dept);
			temp += 3;
		}

		if (Entry->wrk_title)
		{
			temp += stripLines(Entry->wrk_title);
			temp += 3;
		}

		if (Entry->wrk_street_addr)
		{
			temp += stripLines(Entry->wrk_street_addr);
			temp += 3;
		}
		
		if (Entry->wrk_city)
		{
			temp += stripLines(Entry->wrk_city);
			
			if (Entry->wrk_zip || Entry->wrk_state || Entry->wrk_country)
			{
				temp += ", ";
			}
			else
			{
				temp += 3;
			}
		}

		if (Entry->wrk_state)
		{
			temp += stripLines(Entry->wrk_state);
			if (Entry->wrk_zip || Entry->wrk_country)
			{
				temp += " ";
			}
			else
			{
				temp += 3;
			}
		}

		if (Entry->wrk_zip)
		{
			temp += stripLines(Entry->wrk_zip);
			if (Entry->wrk_country)
			{
				temp += " ";
			}
			else
			{
				temp += 3;
			}
		}

		if (Entry->wrk_country)
		{
			temp += stripLines(Entry->wrk_country);
			temp += 3;
		}

		if (Entry->wrk_phone)
		{
			temp += "Work Phone: ";
			temp += stripLines(Entry->wrk_phone);
			temp += 3;
		}

		if (Entry->wrk_fax)
		{
			temp += "Work Fax: ";
			temp += stripLines(Entry->wrk_fax);
			temp += 3;
		}

		if (Entry->wrk_pgr)
		{
			temp += "Work Pager: ";
			temp += stripLines(Entry->wrk_pgr);
			temp += 3;
		}

		if (Entry->wrk_web_page)
		{
			temp += stripLines(Entry->wrk_web_page);
			temp += 3;
		}
	}

	// Are there any Home fields? If not, lets skip the HOME: section.
	if (Entry->cell_phone || Entry->fax || Entry->phone || Entry->web_page || Entry->country || Entry->state || Entry->zip || Entry->city || Entry->street_addr)
	{
		temp += 3;
		temp += "Home:";
		temp += 3;

		if (Entry->street_addr)
		{
			temp += stripLines(Entry->street_addr);
			temp += 3;
		}

		if (Entry->city)
		{
			temp += stripLines(Entry->city);
			
			if (Entry->zip || Entry->state || Entry->country)
			{
				temp += ", ";
			}
			else
			{
				temp += 3;
			}
		}

		if (Entry->state)
		{
			temp += stripLines(Entry->state);
			if (Entry->zip || Entry->country)
			{
				temp += " ";
			}
			else
			{
				temp += 3;
			}
		}

		if (Entry->zip)
		{
			temp += stripLines(Entry->zip);
			if (Entry->country)
			{
				temp += " ";
			}
			else
			{
				temp += 3;
			}
		}

		if (Entry->country)
		{
			temp += stripLines(Entry->country);
			temp += 3;
		}

		if (Entry->phone)
		{
			temp += "Phone: ";
			temp += stripLines(Entry->phone);
			temp += 3;
		}

		if (Entry->fax)
		{
			temp += "Fax: ";
			temp += stripLines(Entry->fax);
			temp += 3;
		}

		if (Entry->cell_phone)
		{
			temp += "Cellular: ";
			temp += stripLines(Entry->cell_phone);
			temp += 3;
		}

		if (Entry->web_page)
		{

			temp += stripLines(Entry->web_page);
			temp += 3;
		}
	}

	*line = new char[temp.GetLength()+1];
	strcpy(*line, temp);

	return true;
}

bool AbookCreator::AddEntry(AbookEntry *Entry)
{
	char *aliasLine = NULL;	
	char *notesLine = NULL;	
	char *nickname = NULL;

	if (!FormNickname(Entry, &nickname))
		return false;

	if (Entry->addresses)
	{
		aliasLine = new char[strlen(Entry->addresses) + strlen(_T("alias ")) + strlen(nickname) + 2];
		sprintf(aliasLine, "alias %s %s", nickname, Entry->addresses);
	}
	else
	{
		aliasLine = new char[strlen(_T("alias ")) + strlen(nickname) + 2];
		sprintf(aliasLine, "alias %s", nickname);
	}

	m_NNFile->PutLine(aliasLine);
	delete [] aliasLine;


	if (!FormNotes(Entry, nickname, &notesLine))
		return false;

	m_NNFile->PutLine(notesLine);
	delete [] notesLine;
	delete [] nickname;
	return true;
}

bool AbookCreator::OpenNNFile(char *filename)
{
	HRESULT hresult;
	if (!m_NNFile)
		m_NNFile = new JJFile;

	if (!filename)
		return false;

	hresult = m_NNFile->Open(filename, O_RDWR | O_CREAT);
	if (HR_FAILED(hresult))
		return false;

	return true;
}

bool AbookCreator::CloseNNFile()
{
	HRESULT hresult;

	hresult = m_NNFile->Close();
	if (HR_FAILED(hresult))
	{
		delete m_NNFile;
		m_NNFile = NULL;
		return false;
	}

	delete m_NNFile;
	m_NNFile = NULL;
	return true;
}

///////////////////////
/// Outlook Express Address Book conversion stuff
///////////
OEAddrConverter::OEAddrConverter()
{

}

OEAddrConverter::~OEAddrConverter()
{

}

bool OEAddrConverter::Convert(ImportChild *Child)
{
	char *abookPath = NULL;
	JJFile	*Abook;
	Abook = new JJFile;
	long NumFields;
	AbookEntry *CurEntry;
	char *NNFilePath = NULL;

	AbookCreator *abookcreator;
	abookcreator = new AbookCreator;
	
	FindWABPath(&abookPath);

	if (!OpenWABFile(abookPath, Abook))
	{
		delete [] abookPath;
		delete [] abookcreator;
		delete [] Abook;
		return false;
	}
	
	char *filename;
	filename = new char[strlen(_T("Outlook Express.txt"))+1];
	strcpy(filename, _T("Outlook Express.txt"));

	if (!abookcreator->FindNNFileName(&filename, &NNFilePath))
		return false;

	if (!abookcreator->OpenNNFile(NNFilePath))
		return false;

	while(FindNextEntry(Abook, &NumFields))
	{
		CurEntry = ReadInEntry(Abook, NumFields);
		if (CurEntry != NULL)		// It's possible we read in some other type of data, like a group,
		{							// which we're not importing. Just skip it.
			abookcreator->AddEntry(CurEntry);
		
			delete CurEntry;
			CurEntry = NULL;
		}
	}

	CloseWABFile(Abook);
	abookcreator->CloseNNFile();

	delete Abook;
	CNicknameFile* NickFile = g_Nicknames->AddNicknameFile(NNFilePath);

	if (NickFile)
		g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_ADDED_NEW_NICKFILE, NickFile);

	delete [] NNFilePath;
	NNFilePath = NULL;
	delete [] abookPath;
	abookPath = NULL;
	delete [] filename;
	filename = NULL;

	delete abookcreator;

	return true;
}

bool OEAddrConverter::CloseWABFile(JJFile *File)
{
	HRESULT hresult;

	hresult = File->Close();
	if (HR_FAILED(hresult))
		return false;

	return true;
}

bool OEAddrConverter::OpenWABFile(char *path, JJFile *File)
{
	HRESULT hresult;

	hresult = File->Open(path, O_RDONLY);
	if (HR_FAILED(hresult))
		return false;

	return true;
}

// Returns Header length. We can have variable record length depending on which fields are
// included, we're going to figure out how many fields are in this record, and tell the field grabbing function.
// This way it'll know when it's record is over and it's time to call it quits.

bool OEAddrConverter::FindNextEntry(JJFile *File, long *NumFields)
{
	char buffer[2] = {0x00, 0x00};
	char testtoken[4] = {0x00, 0x00, 0x00, 0x00};
	char magictoken[4] = {0x40, 0x00, 0x08, 0x30};
	long FirstOffset;
	long SecondOffset;

	while ((memcmp (magictoken, testtoken,4) != 0) && !(File->Read(buffer,1)))
	{
		testtoken[0] = testtoken[1];
		testtoken[1] = testtoken[2];
		testtoken[2] = testtoken[3];
		testtoken[3] = buffer[0];
	}
	
	if (memcmp (magictoken, testtoken,4) != 0)
		return false;

	File->Tell(&FirstOffset);

	testtoken[0] = 0;
	testtoken[1] = 0;
	testtoken[2] = 0;
	testtoken[3] = 0;

	while ((memcmp (magictoken, testtoken,4) != 0) && !(File->Read(buffer,1)))
	{
		testtoken[0] = testtoken[1];
		testtoken[1] = testtoken[2];
		testtoken[2] = testtoken[3];
		testtoken[3] = buffer[0];
	}
	
	if (memcmp (magictoken, testtoken,4) != 0)
		return false;

	File->Tell(&SecondOffset);

	*NumFields = ((SecondOffset - FirstOffset) /4);


	// Lets chaeck to make sure this record is a valid record. The database stores invalid records for speed reasons
	// We need to make sure we don't import them because they'll undoubtedly have a valid twin.

	File->Seek(FirstOffset-36);
	long Valid;
	Valid = readnextLW(File);

	if (!Valid)
	{
		File->Seek(SecondOffset);
		return FindNextEntry(File, NumFields);
	}
	else
	{
		File->Seek(SecondOffset-4);	// rewind so the readin algorithm can processthe field...
	}

	return true;
}

long OEAddrConverter::readnextLW(JJFile *File)
{
	long returnval;
	HRESULT hresult;

	hresult = File->Read((char *)&returnval, 4);
	if (HR_FAILED(hresult))
		return -1;

//	((BYTE *)&returnval)[0] = ((BYTE *)&temp)[3];
//	((BYTE *)&returnval)[1] = ((BYTE *)&temp)[2];
//	((BYTE *)&returnval)[2] = ((BYTE *)&temp)[1];
//	((BYTE *)&returnval)[3] = ((BYTE *)&temp)[0];

	return returnval;
}


AbookEntry * OEAddrConverter::ReadInEntry(JJFile *File, long NumFields)
{
	long value = 0;
	const	long MVStringType	= 0x0000101E;
	const	long StringType		= 0x0000001E;
	const	long LongType		= 0x00000003;
	const	long BinaryType		= 0x00000102;
	const	long ShortType		= 0x00000002;
	const	long SysTimeType	= 0x00000040;
	const	long FunkyType		= 0x00001102;


	// We have a few conditions:
	// we haven't gotten to the interesting data in which case we should continue
	// or we have gotten into the interesting data and the thing we just read in is also interesting so we continue
	// or we're in the interesting data, but what we just read in is no longer interesting so we stop.
	// Just because something's interesting doesn't necessarily mean we save it.

	HRESULT hresult;
	bool done = false;
	bool interested = false;	
	long lOffset;
	long FieldsProcessed = 0;
	AbookEntry *Entry;
	Entry = new AbookEntry;


	while (FieldsProcessed < NumFields)
	{
		value = readnextLW(File);
		if (value == -1)
			break;
		
		switch (value & 0x0000FFFF)
		{
			case MVStringType:
			{
				long NumEntries = 0;
				long EntryLen = 0;
				long TotalLen = 0;
				long realsize;

				NumEntries = readnextLW(File);
				TotalLen = readnextLW(File);
				EntryLen = readnextLW(File);
				realsize = TotalLen - (NumEntries * 4);

				switch (value)
				{
					case 0x3A56101E :		// Email Addresses
					{
						if (Entry->addresses)
							delete [] Entry->addresses;

						realsize = TotalLen - (NumEntries * 2);
						long num = 0;
						char *floater;

						char *temp;
						temp = new char[TotalLen];

						hresult = File->Read(temp, TotalLen-4);	// minus the four bytes read in for the EntryLen
						if (HR_FAILED(hresult))
						{
							FieldsProcessed = NumEntries;
							break;
						}

						Entry->addresses = new char[realsize];
						floater = temp;
						(Entry->addresses)[0] = 0;

						while (num < NumEntries)
						{

							strcat(Entry->addresses, floater);
							strcat(Entry->addresses, _T(","));
							floater += strlen(floater)+5;		// Skip over the longword telling string size...
							num++;
						}
						
						floater = strrchr(Entry->addresses, ',');
						if (floater != NULL)
							*floater = 0;

						delete [] temp;
						break;
					}
					default:				// Any unwanted MVstring, like Transport type...
					{
						hresult = File->Tell(&lOffset);
						if (HR_FAILED(hresult))
						{
							FieldsProcessed = NumEntries;
							break;
						}

						hresult = File->Seek(lOffset+(TotalLen-4));
						if (HR_FAILED(hresult))
						{
							FieldsProcessed = NumEntries;
							break;
						}

						break;
					}
				}
				break;
			}

			case StringType:
			{
				long EntryLen = 0;
				EntryLen = readnextLW(File);
				
				switch (value)
				{
					case 0x3A06001E :		// First name
						{
							if (Entry->first_name)
								delete [] Entry->first_name;

							Entry->first_name = new char[EntryLen];
							hresult = File->Read(Entry->first_name, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;

							break;
						}
					case 0x3A44001E :		// Middle name
						{
							if (Entry->middle_name)
								delete [] Entry->middle_name;

							Entry->middle_name = new char[EntryLen];
							hresult = File->Read(Entry->middle_name, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A11001E :		// Last name
						{
							if (Entry->last_name)
								delete [] Entry->last_name;

							Entry->last_name = new char[EntryLen];
							hresult = File->Read(Entry->last_name, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A4F001E :		// Nickname
						{
							if (Entry->nickname)
								delete [] Entry->nickname;

							Entry->nickname = new char[EntryLen];
							hresult = File->Read(Entry->nickname, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3004001E :		// Notes
						{
							if (Entry->notes)
								delete [] Entry->notes;

							Entry->notes = new char[EntryLen];
							hresult = File->Read(Entry->notes, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A19001E :		// Work Office
						{
							if (Entry->wrk_office)
								delete [] Entry->wrk_office;

							Entry->wrk_office = new char[EntryLen];
							hresult = File->Read(Entry->wrk_office, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A18001E :		// Work Dept.
						{
							if (Entry->wrk_dept)
								delete [] Entry->wrk_dept;

							Entry->wrk_dept = new char[EntryLen];
							hresult = File->Read(Entry->wrk_dept, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A17001E :		// Work Title
						{
							if (Entry->wrk_title)
								delete [] Entry->wrk_title;

							Entry->wrk_title = new char[EntryLen];
							hresult = File->Read(Entry->wrk_title, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A16001E :		// Company
						{
							if (Entry->company)
								delete [] Entry->company;

							Entry->company = new char[EntryLen];
							hresult = File->Read(Entry->company, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A21001E :		// work Pager
						{
							if (Entry->wrk_pgr)
								delete [] Entry->wrk_pgr;

							Entry->wrk_pgr = new char[EntryLen];
							hresult = File->Read(Entry->wrk_pgr, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A24001E :		// work fax
						{
							if (Entry->wrk_fax)
								delete [] Entry->wrk_fax;

							Entry->wrk_fax = new char[EntryLen];
							hresult = File->Read(Entry->wrk_fax, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A08001E :		// work phone
						{
							if (Entry->wrk_phone)
								delete [] Entry->wrk_phone;

							Entry->wrk_phone = new char[EntryLen];
							hresult = File->Read(Entry->wrk_phone, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A51001E :		// work web page
						{
							if (Entry->wrk_web_page)
								delete [] Entry->wrk_web_page;

							Entry->wrk_web_page = new char[EntryLen];
							hresult = File->Read(Entry->wrk_web_page, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A26001E :		// work country
						{
							if (Entry->wrk_country)
								delete [] Entry->wrk_country;

							Entry->wrk_country = new char[EntryLen];
							hresult = File->Read(Entry->wrk_country, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A28001E :		// work state
						{
							if (Entry->wrk_state)
								delete [] Entry->wrk_state;

							Entry->wrk_state = new char[EntryLen];
							hresult = File->Read(Entry->wrk_state, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A2A001E :		// work zip
						{
							if (Entry->wrk_zip)
								delete [] Entry->wrk_zip;

							Entry->wrk_zip = new char[EntryLen];
							hresult = File->Read(Entry->wrk_zip, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A27001E :		// work city
						{
							if (Entry->wrk_city)
								delete [] Entry->wrk_city;

							Entry->wrk_city = new char[EntryLen];
							hresult = File->Read(Entry->wrk_city, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A29001E :		// work adddress
						{
							if (Entry->wrk_street_addr)
								delete [] Entry->wrk_street_addr;

							Entry->wrk_street_addr = new char[EntryLen];
							hresult = File->Read(Entry->wrk_street_addr, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A1C001E :		// Cell phone
						{
							if (Entry->cell_phone)
								delete [] Entry->cell_phone;

							Entry->cell_phone = new char[EntryLen];
							hresult = File->Read(Entry->cell_phone, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A25001E :		// fax
						{
							if (Entry->fax)
								delete [] Entry->fax;

							Entry->fax = new char[EntryLen];
							hresult = File->Read(Entry->fax, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A09001E :		// phone
						{
							if (Entry->phone)
								delete [] Entry->phone;

							Entry->phone = new char[EntryLen];
							hresult = File->Read(Entry->phone, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A50001E :		// personal web page
						{
							if (Entry->web_page)
								delete [] Entry->web_page;

							Entry->web_page = new char[EntryLen];
							hresult = File->Read(Entry->web_page, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A5A001E :		// country
						{
							if (Entry->country)
								delete [] Entry->country;

							Entry->country = new char[EntryLen];
							hresult = File->Read(Entry->country, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A5C001E :		// state
						{
							if (Entry->state)
								delete [] Entry->state;

							Entry->state = new char[EntryLen];
							hresult = File->Read(Entry->state, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A5B001E :		// zip
						{
							if (Entry->zip)
								delete [] Entry->zip;

							Entry->zip = new char[EntryLen];
							hresult = File->Read(Entry->zip, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A59001E :		// city
						{
							if (Entry->city)
								delete [] Entry->city;

							Entry->city = new char[EntryLen];
							hresult = File->Read(Entry->city, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3A5D001E :			// Street Address
						{
							if (Entry->street_addr)
								delete [] Entry->street_addr;

							Entry->street_addr = new char[EntryLen];
							hresult = File->Read(Entry->street_addr, EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;
							break;
						}
					case 0x3003001E:			// Default address
						{
							if (!Entry->addresses)
							{
								Entry->addresses = new char[EntryLen];
								hresult = File->Read(Entry->addresses, EntryLen);
								if (HR_FAILED(hresult))
									FieldsProcessed = NumFields;
								break;
							}
							// else fall thru
						}
					default:
						{
							hresult = File->Tell(&lOffset);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;

							hresult = File->Seek(lOffset+EntryLen);
							if (HR_FAILED(hresult))
								FieldsProcessed = NumFields;

							break;
						}
				}
				break;
			}
			case LongType:
			{
				long FieldLen;
				long LWValue;
				FieldLen = readnextLW(File);
				if (FieldLen != 4)			// Ummm, error... SKIP!
				{
					File->Tell(&lOffset);
					File->Seek(lOffset+FieldLen);
					break;
				}
				else
				{
					LWValue = readnextLW(File);	// No error, read in the next LW
				}

				if (value == 0x0FFE0003)	// Object Type		06 == address entry
				{
					Entry->type = LWValue;
					break;
				}
				else
				{
					// Do anything? not now.
				}
				break;
			}
			case FunkyType:	// I don't know what type this is, but it's like a multivalued longword type or something.
			{
				long NumEntries = 0;
				long Size = 0;
				NumEntries = readnextLW(File);
				Size = readnextLW(File);

				hresult = File->Tell(&lOffset);
				if (HR_FAILED(hresult))
					FieldsProcessed = NumFields;	
				hresult = File->Seek(lOffset+Size);
				if (HR_FAILED(hresult))
					FieldsProcessed = NumFields;
				break;
			}
			case ShortType:
			{
				// Fall thru
			}
			case SysTimeType:
			{
				// Fall thru
			}
			default:
			{
				long Size;
				Size = readnextLW(File);
				hresult = File->Tell(&lOffset);
				if (HR_FAILED(hresult))
					FieldsProcessed = NumFields;
				hresult = File->Seek(lOffset+Size);
				if (HR_FAILED(hresult))
					FieldsProcessed = NumFields;
				break;
			}
		}
		FieldsProcessed++;
	}

	if (Entry->type != 6)
	{
		delete Entry;
		return NULL;
	}

	else return Entry;
}


bool OEAddrConverter::FindWABPath(char ** path) //Good
{
	HKEY	hKey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER,  _T("Software\\Microsoft\\WAB\\WAB4\\Wab File Name"),
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		char szData[512];
		DWORD dwKeyDataType;
		DWORD dwDataBufSize = sizeof(szData)/sizeof(szData[0]);

		if (RegQueryValueEx(hKey, NULL, NULL, &dwKeyDataType,
			(LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
		{
			if (dwDataBufSize < 1)
			{
				RegCloseKey(hKey);
				return false;
			}

			*path = new char[dwDataBufSize+1];
			strcpy(*path,(const char *)&szData);
		}
		else 
		{
			RegCloseKey(hKey);
			return false;
		}
		RegCloseKey(hKey);
	}
	return (true);
}

//////////////////////////
// Utility Functions used in both classes.
///////////////////////


bool KillTheTOCFile(char *pathToMBX)
{
	char * pathToTOC;
	char * floater;

	if (!pathToMBX)
		return false;

	pathToTOC = new char[(strlen(pathToMBX))+1];
	strcpy(pathToTOC, pathToMBX);
	floater = strrchr(pathToTOC, '.');
	*floater = 0;
	strcat(pathToTOC, _T(".toc"));

	DeleteFile(pathToTOC);	// We want Eudora to build the toc itself from the file we're about to write.
	delete [] pathToTOC;
	return true;
}


//================================================================
// Take inputfile and run through decoder, attach to end of outfile mailbox:
//
//================================================================
long InFileToMBX(const char *inFileName, const char *outFileName, const char * ProgDisplay)
{
	extern JJFile *g_pPOPInboxFile;
	extern long g_lEstimatedMsgSize;

	long ret = 0;
//	BoundaryType endType;
	CObArray MimeStates;
	char buf[500];

	FileLineReader lineReader;
	JJFile in, out;
	
	// read from file
	if (FAILED(in.Open(inFileName, O_RDWR)))
		return -1;
	long lSize = 0;
	HRESULT hrSeek = in.Seek(0, SEEK_END, &lSize);
	if (FAILED(hrSeek))
		return -1;
	ASSERT(lSize >= 0);
	g_lEstimatedMsgSize = lSize;
	in.Reset();
	lineReader.SetFile(&in);
	
	// g_pPOPInboxFile is a global (defined in pop.cpp) that is used by mime.cpp, header.cpp code
	JJFile * oldPopMBox = g_pPOPInboxFile;
	if (FAILED(out.Open(outFileName, O_RDWR | O_APPEND | O_CREAT)))
		return -1;
	g_pPOPInboxFile = &out;

	// Create A hd & Load it up, line reader must be set up, output g_pPOPInboxFile must be set up
	HeaderDesc hd(lSize);

	// Create a file base line reader
	MIMEState ms(&lineReader);

		::mimeInit();

	//
	// This looks like a loop, but it's not.  It's just a hack to
	// get the 'break' statement to behave like a goto.  :-)
	//
	while (1)
	{
		long lStartOffset = 0;
		g_pPOPInboxFile->Tell(&lStartOffset);
		if (lStartOffset < 0)
			break;

		hd.lDiskStart = lStartOffset;

		//
		// Write time-stamped envelope marker line.  You know, the
		// one that looks like:
		//
		//           From ???@??? Fri Mar 10 09:04:22 1995
		//
		{
			time_t currentTime;
			time(&currentTime);
			if (currentTime < 0)
				currentTime = 1;
			struct tm* pTime = localtime(&currentTime);

			const char *Weekdays = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat\0";
			const char *Months = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec\0";
			wsprintf(buf, "From ???@??? %s %s %d %d:%d:%d %d" , 
								((const char *) Weekdays) + pTime->tm_wday * 4,
								((const char *) Months) + pTime->tm_mon * 4, 
								pTime->tm_mday, 
								pTime->tm_hour,
								pTime->tm_min, 
								pTime->tm_sec, 
								pTime->tm_year + 1900);
			if (FAILED(g_pPOPInboxFile->PutLine(buf)))
				break;
		}

		//
		// If we have a non-default personality, write the X-Persona header
		//
		CString strCurrentPersona = g_Personalities.GetCurrent();
		if ( ! strCurrentPersona.IsEmpty() )
		{
			CString XPersonaString;
			XPersonaString.Format(CRString(IDS_PERSONA_XHEADER_FORMAT), (const char *) strCurrentPersona);
			if (FAILED(g_pPOPInboxFile->PutLine( XPersonaString )))
				break;
		}
	
		//
		// Download the header and write it to the MBX file.
		//
		int nReadStatus = hd.Read(&lineReader);
		hd.lDiskStart = lStartOffset;		// Envelope is part of the header

		//
		// Did we get the end of the message already?  We weren't
		// supposed to, so the message must be truncated (i.e.  no
		// body).  Oh well, let's just write what we got to file, and
		// consider it a successful download because there's really
		// not much else we can do.
		//
		if (nReadStatus != EndOfMessage)
		{
			if (nReadStatus != EndOfHeader) 
				break;
		

			wsprintf(buf, "%s, %s", hd.who, hd.subj);
			if (strlen(buf) > 64)
			{
				buf[61] = '.';
				buf[62] = '.';
				buf[63] = '.';
				buf[64] = 0;
			}
			::Progress(buf);
			//
			// Instantiate the appropriate MIME reader object
			// using info gleaned from the Content-Type: header.
			//
			if (! ms.Init(&hd) || MimeStates.Add(&ms) < 0)
				return -1;

			//
			// Okay, call the polymorphic MIME reader to really 
			// download the message body...
			//
			BoundaryType endType = ms.m_Reader->ReadIt(MimeStates, buf, sizeof(buf));
			MimeStates.RemoveAt(MimeStates.GetUpperBound());

			//
			// If the top-level type was message/[rfc822|news] then
			// hd.diskStart got munged so set it back to the very
			// beginning of the message
			//
			hd.lDiskStart = lStartOffset;

			if (endType == btError)
				break;
		}
	
		if (g_lEstimatedMsgSize < 0)
		{
			//
			// Write out warning about skipped message body...
			//
			g_pPOPInboxFile->PutLine(CRString(IDS_WARN_SKIP_BIG_MESSAGE));
			wsprintf(buf, CRString(IDS_BIG_MESSAGE_SIZE), -g_lEstimatedMsgSize);
			g_pPOPInboxFile->PutLine(buf);
			g_pPOPInboxFile->PutLine(CRString(IDS_HOW_TO_GET_BIG_MESSAGE));
		}
	
		::Progress(1, NULL, 1);
	
		long lEndOffset = 0;
		g_pPOPInboxFile->Tell(&lEndOffset);
		if (lEndOffset < 0)
			break;

		wsprintf(buf, CRString(IDS_DEBUG_GOOD_RETRIEVE), hd.who, hd.subj);
		::PutDebugLog(DEBUG_MASK_RCV, buf);


		//
		// We don't use the Status: header to determine if a big
		// message has already been skipped.  That was the old way.
		// The new way is determined elsewhere with LMOS record flags.
		//
		return 1;
	}

	in.Close();
	out.Flush();
	out.Close();
	
	// Restore global
	g_pPOPInboxFile = oldPopMBox;
	return ret;
}


void UniquifyName(char **name, QCMailboxCommand * ParentFolderCmd)
{	// This function makes sure the Mailbox or Folder doesn't already exist, if it does it changes the name by appending a .1
	// to the end of the name alot like we do for attachments.
	QCMailboxCommand *FoundMbox = 0;

	if (!ParentFolderCmd)
		FoundMbox = g_theMailboxDirector.FindByName((g_theMailboxDirector.GetMailboxList()), *name);
	else
		FoundMbox = g_theMailboxDirector.FindByName(&(ParentFolderCmd->GetChildList()), *name);
	
	while (FoundMbox)
	{
		char * floater;
		floater = strrchr(*name, '.');
		if (!floater)
		{
			char * newname;
			newname = new char [strlen(*name)+3];	// .1\0
			strcpy(newname, *name);
			strcat(newname, _T(".1"));
			delete [] *name;
			*name = newname;
		}
		else
		{
			floater++;
			char *	num;
			int version;
			int firstlen;
			int lastlen;

			version = atoi(floater);
			firstlen = strlen(floater);
			version++;

			num = new char[firstlen+2];
			sprintf(num, "%d", version);
			lastlen = strlen(num);
			if (lastlen > firstlen)
			{
				char * newname;
				newname = new char[strlen(*name)+(lastlen-firstlen)+1];
				strcpy(newname, *name);
				delete [] *name;
				*name = newname;
			}
				floater = strrchr(*name, '.');
				ASSERT(floater);

				floater++;
				strcpy(floater, num);
				delete [] num;
		}

		if (!ParentFolderCmd)
			FoundMbox = g_theMailboxDirector.FindByName((g_theMailboxDirector.GetMailboxList()), *name);
		else
			FoundMbox = g_theMailboxDirector.FindByName(&(ParentFolderCmd->GetChildList()), *name);
	}
}

LONG OEMboxConverter::GetNumMsgs(char *pathname)
{
	ULONG	NumMsgs = 0;
	HRESULT hresult;
	// DRW 9/13/99 - Added OE5 handling.
	if (!m_bFoundOE5)
	{
		JJFile	stream;
		char tempbuf[4];
		char *indexfile;
		char * floater;

		if (!pathname)
			return -1;

		indexfile = new char [strlen(pathname)+1];
		strcpy(indexfile, pathname);
		floater = strrchr(indexfile, '.');
		strcpy(floater, _T(".idx"));

	   hresult = stream.Open( indexfile, O_RDONLY );
	   if (HR_FAILED(hresult))
	   {
			delete [] indexfile;
			return -1;  
	   }
	   else
	   {
			hresult = stream.Seek(8);
			if (HR_FAILED(hresult))
			{
				stream.Close();
				delete [] indexfile;
				return -1;  
			}

			hresult = stream.Read( tempbuf, 4);
			if (HR_FAILED(hresult))
			{
				stream.Close();
				delete [] indexfile;
				return -1;  
			}

			stream.Close();
			
			((char *)&NumMsgs)[3] = tempbuf[3];
			((char *)&NumMsgs)[2] = tempbuf[2];
			((char *)&NumMsgs)[1] = tempbuf[1];
			((char *)&NumMsgs)[0] = tempbuf[0];

			delete [] indexfile;
	   }
	}
	else
	{
		// DRW 9/13/99 - Look in file specified by pathname for number of messages.
		hresult = m_infile.Open(pathname, CFile::modeRead);
		if (HR_FAILED(hresult))
		{
			return -1;  
		}

		// DRW 9/13/99 - What follows is the OE5 code taken from jasmille@qualcomm.com's
		// OE5 translator code with minor changes for optimization and readability.
		m_infile.Seek(0xc4, SEEK_SET);									// Total message count is here
		m_infile.Read((char*)&NumMsgs, sizeof(ULONG));					// Read it in to allocate

		if (NumMsgs > 0)
		{
			// DRW 9/13/99 - If a table already exists, free it.
			if (m_meOE5MsgTable)
			{
				delete [] m_meOE5MsgTable;
				m_meOE5MsgTable = NULL;
			}

			m_meOE5MsgTable = new OE5MessageEntry[NumMsgs + 1];					// Allocate space for NumMsgs messages
			if (m_meOE5MsgTable != NULL)
			{
				BYTE	lBlockCount = 0;
				long	lFilePosition = 0;
				long	lMiniBlockCount = 0;
				long	lMessageIndex = 0;									// Index into m_infile for start of message
				long	lMessageCount = 0;									// Count of messages found
				bool	bOk = false;
				long	i = 0;
				long	j = 0;
				long	lCurrPos = 0;									// Current seek pos (see comment below)

				m_infile.Seek(0x30, SEEK_SET);							// First lookup table
				m_infile.Read((char*)&lFilePosition, sizeof(long));		// Start over to get the message

				// Look For Messages and count them
				do
				{
					m_infile.Seek(lFilePosition + 17, SEEK_SET);
					m_infile.Read((char*)&lBlockCount, 1L);
					lMiniBlockCount += lBlockCount;
					i = 0;

					m_infile.Seek(lFilePosition + 24, SEEK_SET);				// First Message Address

					// DRW 9/15/99 - The original code performed Seek()s from the current position
					// but this didn't work with the JJFile default 1000 character buffer.  To keep this code as
					// consistent with the OE4 code, I keep track of where we would be Seek()ing to and always
					// Seek() from the beginning.  This is slower, but for the moment I can live with that.
					lCurrPos = lFilePosition + 24;

					// bFirst = TRUE;											// Again Make sure we have messages

					do															// Find how many messages
					{
						m_infile.Read((char*)&lMessageIndex, sizeof(long));		// Get Address of message
						lCurrPos += (sizeof(long) + 8);							// Update the current Seek() pos
						m_infile.Seek(lCurrPos, SEEK_SET);						// Next Address is 8L away
						
						if (lMessageIndex != 0 /*&& 256 < lMessageIndex*/)		// If we have a message
						{
							// Values could be repeated....
							bOk = true;											// Another loop checker

							// DRW 9/13/99 - Iterate through all messages we have found
							// so far to see if this offset is a repeat.  If so, ignore it.
							for (j = 0; (j < lMessageCount) && bOk; j++)		// thru all messages
							{
								if (m_meOE5MsgTable[j].lMessageOffset == lMessageIndex)	bOk = false;  //repeat
							}

							// If not a repeat, add the entry to the table.
							if (bOk)	m_meOE5MsgTable[lMessageCount++].lMessageOffset = lMessageIndex; 
						}
						
					} while (i++ < lBlockCount);		// while there are more messages and it is before next table
					// while ((lMessageIndex != 0) && ((int)m_infile.GetPosition() < lFilePosition + 0x27c));
							
					// if (bFirst && (lMessageIndex == 0)) break;				// Done with mail

					lFilePosition += 0x27c;										// Ok lotsa mail! next table
				
				} while (lMiniBlockCount < (long)NumMsgs);						// til we are done with mail
			}
			
			m_infile.Close();
		}

		m_lNumOE5MsgTableEntries = NumMsgs;
		m_lCurrOE5MsgTableEntry = 0;
	}
	return NumMsgs;
}
