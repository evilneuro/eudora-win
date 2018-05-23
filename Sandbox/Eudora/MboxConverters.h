//	9/13-9/22/99 - Support for OE5 importing added by c_dalew@qualcomm.com.
//		My changes are marked by comments starting with DRW.

#ifndef _MailboxConverter_H_
#define _MailboxConverter_H_


#include "QCMailboxCommand.h"
#include "MAPIImport.h"

InFileToMBX(const char *inFileName, const char *outFileName);

struct	Pathnames
{
	char *					pathname;
	struct Pathnames*		next;
};

struct	Mbox_FnamePair
{
	char *					DisplayName;
	char *					Filename;
	long					lMBoxID;			// DRW 9/20/99 - Record the OE5 ID for this mailbox.
	long					lParentMBoxID;		// DRW 9/20/99 - Record the OE5 ID for the parent mailbox.
	bool					bHasSubfolders;		// DRW 9/22/99 - Record whether or not folder has subfolders.
	QCMailboxCommand *		pMailbox;			// DRW 9/20/99 - Record the Eudora mailbox created for this mailbox.
	struct Mbox_FnamePair*	next;
};

// DRW 9/13/99 - The following structures are from the DBXwindow project.
struct	OE5MessageEntry
{
	long		lMessageOffset;	// Offset of message in dbx file
	long		lMessageText;	// Text of message in dbx file
};

struct	OE5Header
{
	long		lHeaderFilePosition;
	long		lFlags;
	long		lSectionSize;
	long		lNextHeader;
};

struct	OE5Table
{
	long		lTableFilePosition;
	long		lLength;
	char		UnknownBuffer[9];
	BYTE		cbRead;
	BYTE		cbAttach;
	BYTE		cbStatus;
	char		UnknownBuffer2[5];
#pragma pack(push,1)
	/*long*/ unsigned short lMessagePosition;
	char		UnknownBuffer3[30];
	BYTE		cbImportant;
#pragma pack(pop)
};


class AbookEntry{
public:
	char *notes;
	char *wrk_office;
	char *wrk_dept;
	char *wrk_title;
	char *company;
	char *wrk_pgr;
	char *wrk_fax;
	char *wrk_phone;
	char *wrk_web_page;
	char *wrk_country;
	char *wrk_state;
	char *wrk_zip;
	char *wrk_city;
	char *wrk_street_addr;
	char *cell_phone;
	char *fax;
	char *phone;
	char *web_page;
	char *country;
	char *state;
	char *zip;
	char *city;
	char *street_addr;
	char *first_name;
	char *middle_name;
	char *last_name;
	char *nickname;
	char *addresses;
	long type;
	AbookEntry *next;

	public:
		AbookEntry();
		~AbookEntry();
};


class OEMboxConverter{
public:
	char *					m_PathToMail;
	char *					m_PathToEudoraMail; //Rather, the dir where we're going to put the mbxs.

private:
	char *					m_InFilePathname;
	char *					m_TempFilePathname;
	char *					m_OutFilePathname;
	QCMailboxCommand *		m_pRootCommand;
	JJFile					m_tempfile;
	JJFile					m_infile;
	JJFile					m_outfile;
	CFileFind				m_finder;
    BOOL					m_bWorking;
	Mbox_FnamePair *		m_realNames;
	Mbox_FnamePair *		m_mbfpCurrMBox;				// DRW 9/13/99 - Current mailbox entry in m_realNames
	bool					m_bFoundOE5;				// DRW 9/13/99 - Remember if we found OE5
	OE5MessageEntry *		m_meOE5MsgTable;			// DRW 9/13/99 - Location of message entries in dbx file
	long					m_lNumOE5MsgTableEntries;	// DRW 9/13/99 - Number of entries in message table
	long					m_lCurrOE5MsgTableEntry;	// DRW 9/13/99 - ID of current entry in message table
	long					m_lOE5MBoxID;				// DRW 9/20/99 - ID of mailbox
	long					m_lOE5ParentMBoxID;			// DRW 9/20/99 - ID of parent mailbox
	bool					m_bOE5HasSubfolders;		// DRW 9/22/99 - Folder has subfolders.

public:
							OEMboxConverter();	
							~OEMboxConverter();
	int						Convert(class ImportChild *Child, char *infile = NULL, char *outfile = NULL);

protected:
	LONG					GetNumMsgs(char *pathname);
	bool					ReadTilDone();
	bool					FindOEsPath();
	bool					InitMboxFind();
//	char *					CreateFolder(QCMailboxCommand *pCommandParent);
	bool					MoveMsgToEudora();
//	bool					CreateRootFolder();
	bool					FindEudorasPath();
	QCMailboxCommand *		GetParentMbox();			// DRW 9/20/99 - Return the parent mailbox for the current mailbox.
	bool					GrabNextMbox();
	bool					OpenOEFile();
	QCMailboxCommand *		CreateMbox(char *name, QCMailboxCommand *ParentFolderCmd, MailboxType Type=MBT_REGULAR);
	char *					FindRealName(char *pathname);
	bool					CloseOEFile();
	bool					FindToken();
	bool					GrabNextMsg();
	void					BuildRealFolderNames();
	bool					findEntryByToken(JJFile *pchfile);
	int						TempfileGetsBuffer(char *readbuffer, int bufsize);
	bool					OpenTempFile();
	bool					CloseTempFile();
	bool					DeleteTempFile();
	void					removeRealNames(Mbox_FnamePair *realnames);
};

class NSMboxConverter{
public:
	char *					m_PathToMail;
	char *					m_PathToEudoraMail; //Rather, the dir where we're going to put the mbxs.

private:
	char *					m_InFilePathname;
	char *					m_TempFilePathname;
	char *					m_OutFilePathname;
	QCMailboxCommand *		m_pRootCommand;
	JJFile					m_tempfile;
	JJFile					m_infile;
	JJFile					m_outfile;
	ULONG					m_BytesToBeConverted;

public:
							NSMboxConverter();	
							~NSMboxConverter();
	int						Convert(ImportChild *Child, char *infile = NULL, char *outfile = NULL);

protected:
	bool					Startup();
	ULONG					GetSizeOfData(char * PathToMail);
	void					DeleteTree(Pathnames * node);
	LONG					GetNumMsgs(char *pathname);
	static	bool			FindNSsPath(char ** path);
	bool					InitMboxFind(CFileFind	*filefinder);
	bool					InitFolderFind(CFileFind *folderfinder);
	bool					MoveMsgToEudora();
	bool					CreateRootFolder();
	bool					FindEudorasPath();
	bool					GrabNextMbox(CFileFind *filefinder, CFileFind *folderfinder, QCMailboxCommand *ParentFolderCmd);
	QCMailboxCommand *		GetNextFolder(CFileFind *folderfinder, QCMailboxCommand *ParentFolderCmd);
	bool					OpenNSFile(char * pathname);
	QCMailboxCommand *		CreateMbox(char *name, QCMailboxCommand *ParentFolderCmd, MailboxType Type = MBT_REGULAR);
	bool					CloseNSFile();
	bool					FindToken();
	bool					GrabNextMsg();
	int						TempfileGetsBuffer(char *readbuffer, int bufsize);
	bool					OpenTempFile();
	bool					CloseTempFile();
	bool					DeleteTempFile();
	char *					GetDispName(char *pathname);
	char *					GetFoldersName(char *pathname);
	bool					ProcessMailboxes(char *pathname, QCMailboxCommand * MboxCmd );
	bool					ProcessMbox(char *pathname, QCMailboxCommand * MboxCmd );
	bool					BuildFileList(struct Pathnames **files, struct Pathnames *folders);
	bool					BuildFolderList(struct Pathnames **files);
	bool					AddNode(struct Pathnames **currentfile, 	struct _finddata_t *file);
	bool					existsInTree(char * name, struct Pathnames *folders);
};


class NSLdifConverter
{

	long	m_hFile;
	bool	m_findBegun;
public:
	NSLdifConverter();
	~NSLdifConverter();
	bool			Convert(LPCSTR File);
	bool			Convert(ImportChild* Child);				// Fires it off, grabs each Ldif File in the netscape directory and converts it to a Eudora AddressBook
	char *			FindNextFile(char *basePath = NULL);		// Keeps track of which File was the last and finds each Ldif file, opening it and passing it back.
	AbookEntry *	GetNextEntry(JJFile *theFile);		

};

class	OEAddrConverter
{
	char *	m_Pathname;

public:
	bool					Convert(ImportChild *Child);
							OEAddrConverter();
							~OEAddrConverter();
private:
	bool					FindWABPath(char ** path);
	AbookEntry *			ReadInEntry(JJFile *File, long NumFields);
	bool					FindNextEntry(JJFile *File, long *NumFields);
	bool					OpenWABFile(char *path, JJFile *File);
	bool					CloseWABFile(JJFile *File);
	long					readnextLW(JJFile *File);


};

class AbookCreator
{
public:
//	static	bool Import();
	bool FindNNFileName(char **filename, char **fullpath);
	bool AddEntry(AbookEntry *floater);
	bool OpenNNFile(char *NNFilePath);
	bool CloseNNFile();
	bool FormNickname(AbookEntry *Entry, char **Nickname);
	void AddToList(char * nickname);
	void UniquifyNickname(char **nickname);
	bool FormNotes(AbookEntry *Entry, char * nickname, char **line);
	CString stripLines(char *source);

	AbookCreator();
	~AbookCreator();
	// AddENtry??? One entry at a time MIGHT be cool, but we may want to 
	// check that two entries with the same name don't exist.
private:
	struct Pathnames		*m_head;
	JJFile					*m_NNFile;
};



bool						KillTheTOCFile(char *pathToMBX);
long						InFileToMBX(const char *inFileName, const char *outFileName, const char * ProgDisplay = NULL);
void						UniquifyName(char **name, QCMailboxCommand * ParentFolderCmd);

#endif