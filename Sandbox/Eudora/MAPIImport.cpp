#include "stdafx.h"

#ifdef WIN32
#include <afxcmn.h>
#include <afxrich.h>
#endif
#include <fstream.h>
#include "mapix.h"
#include "mapiutil.h"
#include "MAPIImport.h"
#include "MboxConverters.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCPersonalityDirector.h"
#include "Fileutil.h"
#include "mapiinst.h"
#include "persona.h"
#include "WizardPropSheet.h"
#include "mainfrm.h"
#include "rs.h"
#include "doc.h"
#include "nickdoc.h"
#include "progress.h"
#include "guiutils.h"	// For EscapePressed()
#include "resource.h"
#include "MboxConverters.h"
#include "mime.h"

extern QCMailboxDirector		g_theMailboxDirector;
extern QCPersonalityDirector	g_thePersonalityDirector;


#define TEXT_PLAIN	0
#define TEXT_RICH	1
#define TEXT_HTML	2


ImportChild::ImportChild()
{
	LMOS = false;
	IsIMAP = false;
	hasMail = false;
	hasAddresses = false;
	hasLdif = false;
}

ImportProvider::ImportProvider()
{
	hasMail = false;
	hasAddresses = false;
	hasLdif = false;
}

MAPIImport::MAPIImport()
{
	m_lpSession			= NULL;
	m_lpTblStores		= NULL;		// First Table containing the message store.
	m_lpMDB				= NULL;     // global pointer to open message store
	m_lpRootFolder		= NULL;
	m_MAPIFreeBuffer	= NULL;
}

bool MAPIImport::Initialize()
{
	m_MAPIFreeBuffer	=	(CMIMAPIFreeBuffer)GetProcAddress(m_hDLL, "MAPIFreeBuffer");
	if (!m_MAPIFreeBuffer)
		return false;

	return true;
}

MAPIImport::~MAPIImport()
{	

} 

ULONG GetRefcount(IUnknown *iface)
{
	iface->AddRef();
	return iface->Release();
}

bool MAPIImport::Convert(HINSTANCE hDLL, LPMAPISESSION lpSession)
{
	m_hDLL = hDLL;
	m_lpSession = lpSession;

	if (!Initialize())
		return false;

	if (!OpenDefaultStore())
	{
		return false;
	}

	if (!ProcessFolder(0, NULL, NULL))	// Starts at the root (NULL) and works down.
	{
		return false;
	}

	CloseDefaultStore();

	EscapePressed(); // Just to clear the key from the queue

	::CloseProgress();

	return true;
}

void MAPIImport::CloseDefaultStore()
{	
	unsigned long	ulFlags = LOGOFF_NO_WAIT;
	m_lpMDB->StoreLogoff(&ulFlags);
	m_lpMDB->Release();
	m_MAPIFreeBuffer(m_lpMDB);
	m_lpMDB = NULL;

	m_lpSession->Release();

	m_lpTblStores->Release();
	m_MAPIFreeBuffer(m_lpTblStores);
	m_lpTblStores = NULL;

}

bool MAPIImport::OpenDefaultStore()
{
	HRESULT			hResult;
	ULONG			ulDefault	= 1000000;
	unsigned long	iRow;
	LONG			rowCount	= 1;
	ULONG			flags		= 0;
	LPSRowSet		lpRows		= 0;
	unsigned long	i			= 0;
	LPENTRYID		EntryIDValue= 0;
	ULONG			EntryIDSize = 0;
	int				cMaxRows	= 50;

	if (!m_lpSession)
	{
		ErrorDialog(IDS_IMPORT_MAIL_OPENING_ERROR);
		return false;
	}
	hResult = m_lpSession->GetMsgStoresTable(0, &m_lpTblStores);
	if(HR_FAILED(hResult) || !m_lpTblStores)
	{
		ErrorDialog(IDS_IMPORT_MAIL_OPENING_ERROR2, m_lpTblStores, hResult);
		return false;
 	}

	hResult = m_lpTblStores->SeekRow( BOOKMARK_BEGINNING,0 , NULL );
	if( HR_FAILED(hResult))
    {
	// Error code
		ErrorDialog(IDS_IMPORT_MAIL_READING_ERROR);
		return false;
    }

	hResult = m_lpTblStores->QueryRows(cMaxRows, NULL, &lpRows );	// Get first 50 message stores. there should be maybe 3 at the most
	if( HR_FAILED(hResult) || !lpRows)
    {
	// Error code
		ErrorDialog(IDS_IMPORT_MAIL_READING_ERROR2, lpRows, hResult);
		return false;
    }

	for (iRow = 0; iRow < lpRows->cRows; iRow++)		// Which is default Msg Store?
	{
		if (ulDefault != 1000000)
			break;

		for (i = 0; i < lpRows->aRow[iRow].cValues; i++)
		{
			if ((lpRows->aRow[iRow].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_DEFAULT_STORE & 0xFFFF0000))
			{
				if (lpRows->aRow[iRow].lpProps[i].Value.i == 1)
				{
				ulDefault = iRow;
				break;
				}
			}
		}
	}
	if (ulDefault == 1000000)	// None were marked as default
	{
		if (lpRows)
			FreeRowSet(m_MAPIFreeBuffer, lpRows);
		m_lpTblStores->Release();
		if (m_lpTblStores)
			m_MAPIFreeBuffer(m_lpTblStores);
		m_lpTblStores = NULL;
		return false;
	}

	// Now that we know which is the default msg store, what's it's ENTRYID?

	for (i = 0; i < lpRows->aRow[ulDefault].cValues ; i++)
	{
		if (EntryIDValue)
			break;

		if ((lpRows->aRow[ulDefault].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ENTRYID & 0xFFFF0000))
		{
			EntryIDValue = (LPENTRYID)(lpRows->aRow[ulDefault].lpProps[i].Value.bin.lpb);
			EntryIDSize	= (ULONG)(lpRows->aRow[ulDefault].lpProps[i].Value.bin.cb);
		}
	}

	if (EntryIDValue == 0)
		return false;

	hResult = m_lpSession->OpenMsgStore( NULL, 
		EntryIDSize,
		EntryIDValue,
		NULL,
		MDB_NO_DIALOG,
		&m_lpMDB );

	if (lpRows)
		FreeRowSet(m_MAPIFreeBuffer, lpRows);

    if( HR_FAILED( hResult ) || !m_lpMDB)
    {
		ErrorDialog(IDS_IMPORT_MAIL_READING_ERROR3, m_lpMDB, hResult);
		return false;
    }
	return true;
}

bool MAPIImport::ProcessFolder(ULONG cbEntryID, LPENTRYID lpEntryID, QCMailboxCommand *ParentFolderCmd)
{
	bool			success;
	LPMAPITABLE		lpTable = NULL;
	LPSPropTagArray	lpAllPropTags = NULL;
	LPSRowSet lpRowSet = NULL; 
	HRESULT	hresult;
	ULONG i = 0;

	LPENTRYID		EntryIDValue= 0;
	ULONG			EntryIDSize = 0;

	ULONG			EntryObjectType;
	ULONG			ReturnedNumOfMsgs = 0;
	LPMAPIPROP		childFolder = NULL;
	QCMailboxCommand *CurrentFolderCmd = NULL;
	ULONG			ulAccess = 0;
	ULONG			ulObjType;
	LPMAPIPROP		lpFolder;


	hresult = m_lpMDB->OpenEntry( cbEntryID, lpEntryID, NULL, ulAccess, &ulObjType, (LPUNKNOWN*)&lpFolder);

	if (lpEntryID == NULL)
		m_lpRootFolder = lpFolder;

	// We want to make sure this isn't a contacts folder or some such bizzarre thing.
	if (!IsValidMailFolder(lpFolder))
	{
		lpFolder->Release();
		m_MAPIFreeBuffer(lpFolder);
		lpFolder = NULL;
		return false;
	}
/*
	if (ContainsSubfolders(lpFolder))
	{
		// If inbox was a folder and a mailbox we'd want the mailbox inbox to be in the folder inbox.
		if (!(CurrentFolderCmd = CreateEudoraFolder(lpFolder, ParentFolderCmd)))
			return (false);

			if (ContainsMessages(lpFolder, &ReturnedNumOfMsgs) && ReturnedNumOfMsgs)
			{
				GetMessages(lpFolder, CurrentFolderCmd);
				if (EscapePressed(TRUE))
					return false;
			}
	}
	else 
	{
		GetMessages(lpFolder, ParentFolderCmd);
		if (EscapePressed(TRUE))
			return false;
		return true;
	}
*/
	if (!ContainsSubfolders(lpFolder))
	{
		GetMessages(lpFolder, ParentFolderCmd);
		if (EscapePressed(TRUE))
			return false;
		return true;
	}
	else
	{
		// If inbox was a folder and a mailbox we'd want the mailbox inbox to be in the folder inbox.
		if (!(CurrentFolderCmd = CreateEudoraFolder(lpFolder, ParentFolderCmd)))
			return (false);

			if (ContainsMessages(lpFolder, &ReturnedNumOfMsgs) && ReturnedNumOfMsgs)
			{
				GetMessages(lpFolder, CurrentFolderCmd);
				if (EscapePressed(TRUE))
					return false;
			}
	

			// We should now have finished messing with this mailbox, we now have to figure out who its children are.
			hresult =((IMAPIContainer *)lpFolder)->GetHierarchyTable(NULL, &lpTable);
			if (HR_FAILED(hresult))
			{
				if (lpTable)
					m_MAPIFreeBuffer(lpTable);
				return false;
			}

			ULONG abc = lpFolder->Release();
			m_MAPIFreeBuffer(lpFolder);
			lpFolder = NULL;

			hresult = lpTable->QueryColumns(NULL, &lpAllPropTags);
			if (HR_FAILED(hresult))
			{
				if (lpTable)
					m_MAPIFreeBuffer(lpTable);
				if (lpAllPropTags)
					m_MAPIFreeBuffer(lpAllPropTags);
				return false;
			}	

			hresult = lpTable->SetColumns( (LPSPropTagArray) lpAllPropTags, 0);
			if (HR_FAILED(hresult))
			{
				if (lpTable)
					m_MAPIFreeBuffer(lpTable);
				if (lpAllPropTags)
					m_MAPIFreeBuffer(lpAllPropTags);
				return false;
			}
			m_MAPIFreeBuffer(lpAllPropTags);

			hresult = lpTable->SeekRow( BOOKMARK_BEGINNING, 0, NULL );

			while (!(hresult = lpTable->QueryRows(1,0, &lpRowSet)))
			{
				if (EscapePressed(TRUE))
					break;

				if (lpRowSet->cRows != 0)
				{
					for (i = 0; i < lpRowSet->aRow[0].cValues; i++)
					{
						if ((lpRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ENTRYID & 0xFFFF0000))
						{
							EntryIDValue = (LPENTRYID)(lpRowSet->aRow[0].lpProps[i].Value.bin.lpb);
							EntryIDSize	= (ULONG)(lpRowSet->aRow[0].lpProps[i].Value.bin.cb);
						}
						else if ((lpRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_OBJECT_TYPE & 0xFFFF0000))
						{
							EntryObjectType = (ULONG)lpRowSet->aRow[0].lpProps[i].Value.ul;
						}
					}

					if ((EntryIDValue) && (EntryObjectType == MAPI_FOLDER))
					{
						char *buf;
						buf = new char[100];
						sprintf(buf, "%x\n", EntryIDValue->ab);
						TRACE(buf);
						delete [] buf;

						success = ProcessFolder(EntryIDSize, EntryIDValue, CurrentFolderCmd);
						FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
						lpRowSet = NULL;
					}
					else
					{
						FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
						lpRowSet = NULL;
						break;
					}
				}
				else break;
			}

			lpTable->Release();
			m_MAPIFreeBuffer(lpTable);
			lpTable = NULL;
	}

	return true;
}

bool MAPIImport::IsValidMailFolder(LPMAPIPROP ParentFolder)
{
	HRESULT				hresult = 0;
	LPSPropValue		ReturnedValueArray;
	unsigned long		numEntries;
	char *				ContainerClass = NULL;
	char *				folderName = NULL;
	ULONG i;

	SizedSPropTagArray	(2, DesiredProps) =
    {
        2,
        {
            PR_CONTAINER_CLASS,
			PR_DISPLAY_NAME
        }
    };

	ParentFolder->GetProps((LPSPropTagArray) &DesiredProps, NULL,  &numEntries, &ReturnedValueArray);
	
	for (i = 0; i < numEntries ; i++)
	{
		if ((ReturnedValueArray[i].ulPropTag & 0xFFFF0000) == (PR_CONTAINER_CLASS & 0xFFFF0000))
		{
			ContainerClass = ReturnedValueArray[i].Value.lpszA;
			if (ReturnedValueArray[i].ulPropTag	== 0x3613000a)
				ContainerClass = NULL;
		}
		if ((ReturnedValueArray[i].ulPropTag & 0xFFFF0000) == (PR_DISPLAY_NAME & 0xFFFF0000))
		{
			folderName = ReturnedValueArray[i].Value.lpszA;
		}

	}

	if (ContainerClass == NULL)	// No container Class. Must be a mailbox.
	{
		m_MAPIFreeBuffer(ReturnedValueArray);
		return true;
	}
	else
	{
		if (!strcmp(ContainerClass, _T("IPF.Note")))	// But it's note.
		{
			m_MAPIFreeBuffer(ReturnedValueArray);
			return true;
		}
		m_MAPIFreeBuffer(ReturnedValueArray);
		return false;
	}
}

bool MAPIImport::ContainsSubfolders(LPMAPIPROP ParentFolder)
{
	HRESULT				hresult = 0;
	LPSPropValue		ReturnedValue;
	ULONG				numEntries = 1;

	SizedSPropTagArray	(1, DesiredProps) =
    {
        1,
        {
            PR_SUBFOLDERS
        }
    };
	// Do all MAPI providers have this tag? if not this will fail and we'll barf.
	// Does matter since we're only using OL98
	// We can find other ways to find out if the folder contains messages...
	hresult = ParentFolder->GetProps((LPSPropTagArray) &DesiredProps, NULL, &numEntries, &ReturnedValue);
	if (HR_FAILED(hresult))
		return (false);

	if (ReturnedValue)
	{
		if(ReturnedValue->Value.ul)
		{
			m_MAPIFreeBuffer(ReturnedValue);
			return(true);
		}
		else
			m_MAPIFreeBuffer(ReturnedValue);
	}

	return (false);
}

bool MAPIImport::ContainsMessages(LPMAPIPROP ParentFolder, ULONG * NumReturned)
{
	HRESULT				hresult = 0;
	LPSPropValue		ReturnedValue;
	ULONG				numEntries = 1;

	*NumReturned = 0;

	SizedSPropTagArray	(1, DesiredProps) =
    {
        1,
        {
            PR_CONTENT_COUNT
        }
    };
	// Do all MAPI providers have this tag? if not this will fail and we'll barf.
	// doesnt matter since we only use this for OL98
	hresult = ParentFolder->GetProps((LPSPropTagArray) &DesiredProps, NULL, &numEntries, &ReturnedValue);
	if (HR_FAILED(hresult))
		return false;

	if (ReturnedValue)
	{
		*NumReturned = ReturnedValue->Value.ul;
		m_MAPIFreeBuffer(ReturnedValue);
		return true;
	}
	else return false;
}

QCMailboxCommand * MAPIImport::CreateEudoraFolder(LPMAPIPROP	CurrentFolder, QCMailboxCommand *ParentFolderCmd)
{
	ULONG i;
	ULONG MsgCount;
	QCMailboxCommand *NewFolderCmd = NULL;
	char * folderName;
	LPSPropValue	lpPropArray;
	ULONG		NumProps = 0;
	SizedSPropTagArray	(2, DesiredProps) =
    {
        2,
        {
            PR_DISPLAY_NAME,
			PR_CONTENT_COUNT
        }
    };

	if (CurrentFolder == m_lpRootFolder)
	{
		folderName = new char[16];
		strcpy(folderName, _T("Outlook 98"));

		UniquifyName(&folderName, NULL);
		NewFolderCmd = g_theMailboxDirector.AddCommand( folderName, MBT_FOLDER, NULL);

		delete [] folderName;
		return NewFolderCmd;
	}
	else
	{
		NewFolderCmd = NULL;
		CurrentFolder->GetProps((LPSPropTagArray) &DesiredProps, NULL,  &NumProps, &lpPropArray);
		
		for (i = 0; i < NumProps ; i++)
		{
			if ((lpPropArray[i].ulPropTag & 0xFFFF0000) == (PR_CONTENT_COUNT & 0xFFFF0000))
			{
				MsgCount = lpPropArray[i].Value.ul;
			}
			if ((lpPropArray[i].ulPropTag & 0xFFFF0000) == (PR_DISPLAY_NAME & 0xFFFF0000))
			{
				folderName = new char[strlen(lpPropArray[i].Value.lpszA)+1];
				strcpy(folderName, lpPropArray[i].Value.lpszA);
			}

		}
		if  (folderName == NULL)
		{
			folderName = new char[15];
			strcpy(folderName, _T("Unnamed Folder"));
		}

		m_MAPIFreeBuffer(lpPropArray);

		NewFolderCmd = g_theMailboxDirector.AddCommand( folderName, MBT_FOLDER, ParentFolderCmd);
		delete [] folderName;

		return NewFolderCmd;
	}
}

bool MAPIImport::GetMessages(LPMAPIPROP	CurrentFolder, QCMailboxCommand *ParentFolderCmd)
{
	QCMailboxCommand *NewFolderCmd = NULL;
	char * pathToMBX;
	HRESULT hresult;
	LPMAPITABLE lpTable;
	LPSRowSet		lpRowSet;
	SizedSPropTagArray	(10, MsgProps) =
	{
		10,
		{
			PR_ENTRYID,
			PR_MESSAGE_SIZE,
			PR_SUBJECT,
			PR_SENDER_NAME,
			PR_SENDER_EMAIL_ADDRESS,
			PR_HASATTACH,
			PR_CREATION_TIME,
			PR_LAST_MODIFICATION_TIME,
			PR_CLIENT_SUBMIT_TIME,
			PR_DISPLAY_TO
		}
	};
	LPIID           lpInterface;
	lpInterface = (LPIID)(&IID_IStream);
	JJFile	outgoingMbox;

	if (!MakeNewMailbox(CurrentFolder, ParentFolderCmd, &NewFolderCmd))
		return true;

	if (!NewFolderCmd)
		return false;

	pathToMBX = new char[((NewFolderCmd->GetPathname()).GetLength())+1];
	strcpy(pathToMBX, NewFolderCmd->GetPathname());

	KillTheTOCFile(pathToMBX);

	((IMAPIContainer *)CurrentFolder)->GetContentsTable(NULL, &lpTable);

	hresult = lpTable->SetColumns( (LPSPropTagArray) &MsgProps, 0);

	if (FAILED(outgoingMbox.Open(pathToMBX, O_RDWR | O_CREAT | O_APPEND)))
	{
		delete [] pathToMBX;
		return false;
	}

	::MainProgress(_T("Converting From Outlook"));

	while (!(hresult = lpTable->QueryRows(1,0, &lpRowSet)))
	{
		if (lpRowSet->cRows != 0)
		{
			CommitRowToMbx(lpRowSet, &outgoingMbox);
//			lpRowSet->cRows = 1;F
//			FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
			m_MAPIFreeBuffer(lpRowSet);
			lpRowSet = NULL;
			DecrementCountdownProgress();
		}
		else
			break;

		if (EscapePressed(TRUE))
			break;
	}

	lpTable->Release();
	m_MAPIFreeBuffer(lpTable);
	lpTable = NULL;
	
	outgoingMbox.Flush();
	outgoingMbox.Close();

	NewFolderCmd->Execute(CA_COMPACT,NULL);

	delete [] pathToMBX;

	return true;
}

bool MAPIImport::MakeNewMailbox(LPMAPIPROP	CurrentFolder,  QCMailboxCommand *ParentFolderCmd, QCMailboxCommand **NewFolderCmd)
{
	char * folderName = NULL;	
	char * ContClass = NULL;
	LPSPropValue	lpPropArray;
	ULONG		NumMsgs = 0;
	ULONG		NumProps = 0;	
	SizedSPropTagArray	(2, DesiredProps) =
    {
        2,
        {
            PR_DISPLAY_NAME,
			PR_CONTENT_COUNT
        }
    };

	ULONG i;

	char BadNames[4][17] = { "Search Root", "IPM_VIEWS", "IPM_COMMON_VIEWS", "Reminders" };

	CurrentFolder->GetProps((LPSPropTagArray) &DesiredProps, NULL,  &NumProps, &lpPropArray);
		
	if (lpPropArray)
	{
		for (i = 0; i < DesiredProps.cValues; i++)
		{
			if ((lpPropArray[i].ulPropTag & 0xFFFF0000) == (PR_DISPLAY_NAME & 0xFFFF0000))
			{
				folderName = new char[strlen(lpPropArray[i].Value.lpszA)+1];
				strcpy(folderName, lpPropArray[i].Value.lpszA);
			}

			else if ((lpPropArray[i].ulPropTag & 0xFFFF0000) == (PR_CONTENT_COUNT & 0xFFFF0000))
			{
				NumMsgs = lpPropArray[i].Value.ul;
			}
		}
		m_MAPIFreeBuffer (lpPropArray);
		lpPropArray = NULL;

		if (folderName == NULL)
		{
			folderName = new char[15];
			strcpy(folderName, _T("Unnamed Folder"));
		}


		for (i = 0; i < 4; i++)
		{
			if (strcmp(folderName, (const char *)&BadNames[i]) == 0)
			{
				delete [] folderName;
				delete [] ContClass;
				return false;
			}
		}
		TRACE(folderName);
		*NewFolderCmd = g_theMailboxDirector.AddCommand( folderName, MBT_REGULAR, ParentFolderCmd);

		char * buf;
		buf = new char[strlen(folderName)+20];
		sprintf(buf, "Messages left in %s: ", folderName);
		CountdownProgress(buf, NumMsgs);
		delete [] buf;
		delete [] folderName;
		return true;
	}
	else
	{
		return false;
	}
}

bool MAPIImport::KillTheTOCFile(char *pathToMBX)
{
	char * pathToTOC;
	char * floater;

	pathToTOC = new char[(strlen(pathToMBX))+1];
	strcpy(pathToTOC, pathToMBX);
	floater = strrchr(pathToTOC, '.');
	*floater = 0;
	strcat(pathToTOC, _T(".toc"));

	DeleteFile(pathToTOC);	// We want Eudora to build the toc itself from the file we're about to write.
	delete [] pathToTOC;
	return true;
}

bool MAPIImport::CommitRowToMbx(LPSRowSet lpRowSet, JJFile *Mailbox)
{
	ULONG	msgType;
	LPMESSAGE openedMsg;
	LPENTRYID	EntryIDValue;
	ULONG	EntryIDSize;
	ULONG	MessageBodySize;
	char *	MessageSubject = NULL;
	char *	SenderName = NULL;
	char *	SenderEmail = NULL;
	char *	RcptEmail = NULL;
	char * Date = NULL;
	bool	HasAttach;
	ULONG	bodyType = TEXT_PLAIN;		// 0 = plain/unknown, 1 = enriched, 2 = html
	SYSTEMTIME * lpSysTime = 0;

	ULONG i;
	for (i = 0; i < lpRowSet->aRow[0].cValues; i++)
	{
		if ((lpRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ENTRYID & 0xFFFF0000))
		{
			EntryIDValue = (LPENTRYID)(lpRowSet->aRow[0].lpProps[i].Value.bin.lpb);
			EntryIDSize	= (ULONG)(lpRowSet->aRow[0].lpProps[i].Value.bin.cb);
		}
		else if((lpRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_MESSAGE_SIZE & 0xFFFF0000))
		{
			MessageBodySize = (ULONG)(lpRowSet->aRow[0].lpProps[i].Value.ul);
		}
		else if(lpRowSet->aRow[0].lpProps[i].ulPropTag == PR_SUBJECT)
		{
			char * TempMessageSubject = new char[1024];
			strncpy(TempMessageSubject, (char *)(lpRowSet->aRow[0].lpProps[i].Value.lpszA), 1024);
			MessageSubject = new char[strlen(TempMessageSubject)+1];
			strcpy(MessageSubject, TempMessageSubject);
			delete [] TempMessageSubject;
		}
		else if(lpRowSet->aRow[0].lpProps[i].ulPropTag == PR_SENDER_NAME)
		{
			char * TempSenderName = new char[1024];
			strncpy(TempSenderName, (char *)(lpRowSet->aRow[0].lpProps[i].Value.lpszA), 1024);
			SenderName = new char[strlen(TempSenderName)+1];
			strcpy(SenderName, TempSenderName);
			delete [] TempSenderName;
		}
		else if(lpRowSet->aRow[0].lpProps[i].ulPropTag == PR_SENDER_EMAIL_ADDRESS)
		{
			char * TempSenderEmail = new char[1024];
			strncpy(TempSenderEmail, (char *)(lpRowSet->aRow[0].lpProps[i].Value.lpszA), 1024);
			SenderEmail = new char[strlen(TempSenderEmail)+1];
			strcpy(SenderEmail, TempSenderEmail);
			delete [] TempSenderEmail;
		}

		else if(lpRowSet->aRow[0].lpProps[i].ulPropTag == PR_HASATTACH)
		{
			HasAttach = (lpRowSet->aRow[0].lpProps[i].Value.b == 0); // which is bool type?
		}
		else if (lpRowSet->aRow[0].lpProps[i].ulPropTag == PR_DISPLAY_TO)
		{
			char * TempRcptEmail = new char[1024];
			strncpy(TempRcptEmail, (char *)(lpRowSet->aRow[0].lpProps[i].Value.lpszA), 1024);
			RcptEmail = new char[strlen(TempRcptEmail)+1];
			strcpy(RcptEmail, TempRcptEmail);
			delete [] TempRcptEmail;
		}
		else if(lpRowSet->aRow[0].lpProps[i].ulPropTag == PR_CREATION_TIME)
		{

		}
		else if(lpRowSet->aRow[0].lpProps[i].ulPropTag == PR_LAST_MODIFICATION_TIME)
		{

		}
		else if(lpRowSet->aRow[0].lpProps[i].ulPropTag == PR_CLIENT_SUBMIT_TIME)
		{

		}
	}
	char * buf;
	if (SenderName && MessageSubject)
	{
		buf = new char[strlen(SenderName) +strlen(MessageSubject)+3];
		wsprintf(buf, "%s, %s",SenderName, MessageSubject);
	}
	else if (!SenderName && MessageSubject)
	{
		buf = new char[strlen(MessageSubject)+1];
		strcpy(buf, MessageSubject);
	}
	else if (!MessageSubject && SenderName)
	{
		buf = new char[strlen(SenderName)+1];
		strcpy(buf, SenderName);
	}	
	else
	{
		buf = new char[4];
		strcpy(buf, _T(" , "));
	}

	if (strlen(buf) > 64)	// So we don't wrap text, hopefully 64 is a good number, I just guessed.
	{
		buf[61] = '.';
		buf[62] = '.';
		buf[63] = '.';
		buf[64] = 0;
	}

	::Progress(buf);
	delete [] buf;

	if (EntryIDValue)
	{	
		ULONG	NumBytesRead;
		LPUNKNOWN		TempIface;
		char * HeaderText;

		CPtrList	Attachments;
		HRESULT	hresult;
		LPIID           lpInterface     = 0;
		lpInterface = (LPIID)(&IID_IStream);

		Progress(0, NULL, MessageBodySize);

		hresult = m_lpSession->OpenEntry( EntryIDSize, EntryIDValue, NULL, NULL, &msgType, (LPUNKNOWN*)&openedMsg);
		if (HR_FAILED(hresult))
		{
			return false;
		}

		hresult = ((IMAPIProp *) openedMsg)->OpenProperty(PR_TRANSPORT_MESSAGE_HEADERS, lpInterface, 0, NULL, &TempIface);
		if ((!(HR_FAILED(hresult))) && TempIface)
		{
			char MIMEheader[] = "Mime-Version:";
			char Contentheader[] = "Content-Type:";

			// Take off multipart mixed header.
			ULONG totalbytes=0;
			Mailbox->Put(_T("From ???@??? Fri Apr 10 11:02:36 1998\r\n"), 39); 
			while((HeaderText) = (GetLine((IStream *)TempIface, &NumBytesRead, &totalbytes)))
			{
				ProgressAdd(NumBytesRead);

				if ((HeaderText[0] != '\r') && (HeaderText[0] != '\n'))
				{
					if ((_strnicmp(HeaderText, MIMEheader, sizeof(MIMEheader)-1) != 0) && (_strnicmp(HeaderText, Contentheader, sizeof(Contentheader)-1) != 0))
					{
						Mailbox->Put(HeaderText, NumBytesRead);
					}
				}
				delete [] HeaderText;
			}
			TempIface->Release();
			m_MAPIFreeBuffer(TempIface);
			TempIface = NULL;
		}
		else
		{// We need to build our own headers.
			GenerateHeaders(Mailbox, SenderName, SenderEmail, Date, RcptEmail, MessageSubject);
		}
		delete [] SenderName;
		SenderName = NULL;
		delete [] SenderEmail;
		SenderEmail = NULL;
		delete [] Date;
		Date = NULL;
		delete [] RcptEmail;
		RcptEmail = NULL;
		delete [] MessageSubject;
		MessageSubject = NULL;
		

		hresult = ((IMAPIProp *) openedMsg)->OpenProperty(PROP_TAG( PT_TSTRING,   0x1013), lpInterface, 0, NULL, &TempIface);
		if ((!(HR_FAILED(hresult))) && TempIface)
		{
			bodyType = TEXT_RICH;
			AddContentHeader((IStream *)TempIface, Mailbox, &bodyType);

			GrabMsgBody((IStream *)TempIface, Mailbox);

			TempIface->Release();
			m_MAPIFreeBuffer(TempIface);
			TempIface = NULL;
		}
		else
		{
			TempIface = NULL;
			hresult = ((IMAPIProp *) openedMsg)->OpenProperty(PR_BODY, lpInterface, 0, NULL, &TempIface);
			if ((!(HR_FAILED(hresult))) && TempIface)
			{
				bodyType = TEXT_PLAIN;
				AddContentHeader((IStream *)TempIface, Mailbox, &bodyType);

				GrabMsgBody((IStream *)TempIface, Mailbox);

				TempIface->Release();
				m_MAPIFreeBuffer(TempIface);
				TempIface = NULL;
			}
			else
			{	// There's apparently no body. Huh..
				bodyType = TEXT_PLAIN;
				AddContentHeader((IStream *)TempIface, Mailbox, &bodyType);

				Mailbox->Put(_T(" \r\n"), 3);	// just a space and a newline to give it a bit of a body...
				if (TempIface)
				{
					TempIface->Release();
					m_MAPIFreeBuffer(TempIface);
					TempIface = NULL;
				}
			}
		}

		LPMAPITABLE lpAttachTable;
		LPSRowSet	lpAttachRowSet;
		SizedSPropTagArray	(4, AttachProps) =
		{
			4,
			{
				PR_ATTACH_LONG_FILENAME,
				PR_ATTACH_NUM,
				PR_STORE_ENTRYID,
				PR_ATTACH_DATA_BIN
			}
		};
		char * Filename = NULL;

		hresult = ((IMessage*) openedMsg)->GetAttachmentTable( NULL,	&lpAttachTable);
		if (!HR_FAILED(hresult))
		{
			hresult = lpAttachTable->SetColumns( (LPSPropTagArray) &AttachProps, 0);

			while (!(hresult = lpAttachTable->QueryRows(1,0, &lpAttachRowSet)))
			{
				if (lpAttachRowSet->cRows != 0)
				{
					LONG AttachNum = 0;

					for (i = 0; i < lpAttachRowSet->aRow[0].cValues; i++)
					{

						if ((lpAttachRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ATTACH_NUM & 0xFFFF0000))
						{
							AttachNum = (LONG)(lpAttachRowSet->aRow[0].lpProps[i].Value.l);
						}
						else if ((lpAttachRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ATTACH_LONG_FILENAME & 0xFFFF0000))
						{	
							if ((((ULONG)(lpAttachRowSet->aRow[0].lpProps[i].Value.lpszA)) & 0xFFFF0000) != (0x8004010f & 0xFFFF0000))
							{
								Filename = new char[strlen(lpAttachRowSet->aRow[0].lpProps[i].Value.lpszA)+1];
								strcpy(Filename, lpAttachRowSet->aRow[0].lpProps[i].Value.lpszA);
							}
						}
					}
					if (AttachNum && Filename && strlen(Filename) != 0)
					{
						LPATTACH	lpAttach = NULL;

						hresult = ((IMessage *)openedMsg)->OpenAttach(AttachNum, NULL, NULL, &lpAttach);
						if (!HR_FAILED(hresult))	
						{
							hresult = ((IMAPIProp *) lpAttach)->OpenProperty(PR_ATTACH_DATA_BIN, lpInterface, 0, NULL, &TempIface);
							if ((!(HR_FAILED(hresult))) && TempIface)
							{
							
								WriteAttachToMbx((IStream *)TempIface, Mailbox, Filename);

								TempIface->Release();
								m_MAPIFreeBuffer(TempIface);
								TempIface = NULL;
							}
							lpAttach->Release();
							m_MAPIFreeBuffer(lpAttach);
							lpAttach = NULL;
						}
					}

					if (Filename)
						delete [] Filename;
					Filename = NULL;
					FreeRowSet(m_MAPIFreeBuffer, lpAttachRowSet);
					lpRowSet = NULL;
				}
				else break;

				if (EscapePressed(TRUE))
					return false;
			}

			lpAttachTable->Release();
			m_MAPIFreeBuffer(lpAttachTable);
			lpAttachTable = NULL;

			if (lpRowSet)
				FreeRowSet(m_MAPIFreeBuffer, lpRowSet);

			openedMsg->Release();
			m_MAPIFreeBuffer(openedMsg);
			openedMsg = NULL;
		}
	}
	else
	{// No entry ID? No message. Delete our allocated data...
		delete [] SenderName;
		delete [] SenderEmail;
		delete [] Date;
		delete [] RcptEmail;
		delete [] MessageSubject;
	}
	return true;
}

bool MAPIImport::WriteAttachToMbx(IStream * stream, JJFile * Mailbox, char * name)
{
	ULONG numBytes;
	char *bodyChunk = NULL;
	char *attachDir = NULL;
	JJFile	*AttachFile;

	AttachFile = OpenAttachFile(name);

	if (!AttachFile)
		return false;

	BSTR Filename;
	AttachFile->GetFName(&Filename);
	CString StrFilename = Filename;
	if (!StrFilename)
	{	
		delete AttachFile;
		return false;
	}

	bodyChunk = new char[10000];
	if (bodyChunk == NULL)
	{
		delete AttachFile;
		return false;
	}

	while(!HR_FAILED(stream->Read(bodyChunk, 9999, &numBytes)) && numBytes != 0)
	{
		// Write chunk to file
		ProgressAdd(numBytes);
		AttachFile->Put(bodyChunk, numBytes);
		if (EscapePressed(TRUE))
		{
			delete [] bodyChunk;
			delete AttachFile;
			return false;
		}
	}

	delete [] bodyChunk;

	char * AttachLine;
	AttachLine = new char[24+ (StrFilename ? strlen(StrFilename) : 0)+1];
	sprintf(AttachLine, "Attachment Converted: \"%s\"", StrFilename);
	Mailbox->PutLine(AttachLine,strlen(AttachLine));	// Just in case someone forgot to end a msg with a CR.
	delete [] AttachLine;

	delete AttachFile;
	return true;
}

bool MAPIImport::GenerateHeaders(JJFile *Mailbox, char *SenderName, char *SenderEmail, char *Date, char *RcptEmail, char *MessageSubject)
{
			Mailbox->Put(_T("From ???@??? Fri Apr 10 11:02:36 1998\r\n"), 39);
			Mailbox->Put(_T("From: "),6);
			if (SenderName)
			{
				Mailbox->Put(SenderName, strlen(SenderName));
			}
			if (SenderEmail)
			{
				Mailbox->Put(_T(" <"),2);
				Mailbox->Put(SenderEmail, strlen(SenderEmail));
				Mailbox->Put(_T(">\r\n"),3);
			}
			else
			{
				Mailbox->Put(_T("\r\n"),2);
			}
			if (Date)
			{
				Mailbox->Put(_T("Date: "),6);
	//			Mailbox->Put(RcptEmail, strlen(RcptEmail));
				Mailbox->Put(_T("\r\n"),2);	
			}
			if (RcptEmail)
			{
				Mailbox->Put(_T("To: "),4);
				Mailbox->Put(RcptEmail, strlen(RcptEmail));
				Mailbox->Put(_T("\r\n"),2);	
			}
			if (MessageSubject)
			{
				Mailbox->Put(_T("Subject: "),9);
				Mailbox->Put(MessageSubject, strlen(MessageSubject));
				Mailbox->Put(_T("\r\n"),2);	
			}
			return true;
}

bool MAPIImport::GrabMsgBody(IStream * BodyStream, JJFile *Mailbox)
{
	ULONG numBytes;
	char *bodyChunk;
	HRESULT hresult;

	bodyChunk = new char[1025];
	hresult = BodyStream->Read(bodyChunk, 1024, &numBytes);
	if (!HR_FAILED(hresult) && numBytes != 0)
	{
		ProgressAdd(numBytes);
		Mailbox->Put(bodyChunk, numBytes);
		GrabMsgBody(BodyStream, Mailbox);
	}
	
	delete [] bodyChunk;

	Mailbox->Put(_T("\r\n"),2);	// Just in case someone forgot to end a msg with a CR.

	return (true);
}

bool MAPIImport::AddContentHeader(IStream *Stream, JJFile *Mailbox, ULONG *bodytype)
{
	Mailbox->Put(_T("Mime-Version: 1.0\r\n"), 19);
	HRESULT hresult;
	ULONG numBytes=0;
	char * readBuffer = NULL;
	char * strReturns = NULL;
	LARGE_INTEGER SeekDisp;

	SeekDisp.LowPart = 0;
	SeekDisp.HighPart = 0;

	readBuffer = new char[1026];
	if (readBuffer == NULL)
		*bodytype = TEXT_PLAIN;
	else if (*bodytype != TEXT_PLAIN)
	{
		hresult = Stream->Read(readBuffer, 1024, &numBytes);
		if (HR_FAILED(hresult))
		{
			*bodytype = TEXT_PLAIN;
		}

		Stream->Seek(SeekDisp, STREAM_SEEK_SET, NULL);
		if (*bodytype != TEXT_PLAIN)
		{
			strReturns = strstr(readBuffer, _T("html>"));
			if (strReturns == 0)
				*bodytype = TEXT_RICH;
			else 
				*bodytype = TEXT_HTML;
		}
	}
	if (readBuffer)
		delete [] readBuffer;

	if (*bodytype == TEXT_PLAIN)
	{
		Mailbox->Put(_T("Content-Type: text/plain; charset=\"us-ascii\"\r\n\r\n"), 48);
	}
	else if (*bodytype == TEXT_RICH)
	{
		Mailbox->Put(_T("Content-Type: text/enriched; charset=\"us-ascii\"\r\n\r\n<x-rich>\r\n"), 61);
	}
	else if (*bodytype == TEXT_HTML)
	{
		Mailbox->Put(_T("Content-Type: text/html; charset=\"us-ascii\"\r\n\r\n<x-html>\r\n"), 57);
	}
	return true;
}

char * MAPIImport::GetLine(IStream * Stream, ULONG * bytesRead, ULONG *posn)
{
	char * pReturned;
	char * pTemp;
	char * pSrcFloater;
	HRESULT hresult;
	ULONG	NumBytesRead;
	int count = 0;
	LARGE_INTEGER SeekDisp;

	pTemp = new char[1025];
	pSrcFloater =pTemp;
	hresult = ((IStream *)Stream)->Read((void *)pTemp, 1024, &NumBytesRead); 
	if (HR_FAILED(hresult))
	{
		delete [] pTemp;
		return 0;
	}

	if (NumBytesRead == 0)
	{
		delete []pTemp;
		return 0;
	}

	while ((*pSrcFloater != '\r') && (count++ < 1022))
	{
		pSrcFloater++;
	}

	while ((*pSrcFloater == '\r') || (*pSrcFloater == '\n'))
	{
		pSrcFloater++;
		count++;
	}

	pReturned = new char[count+3];
	*posn += count;
	SeekDisp.LowPart = *posn;
	SeekDisp.HighPart = 0;

	hresult = ((IStream *)Stream)->Seek(SeekDisp, STREAM_SEEK_SET, NULL);
	if (HR_FAILED(hresult))
	{
		delete []pTemp;
		delete []pReturned;
		return 0;
	}

	pSrcFloater--;
	while ((*pSrcFloater == '\r') || (*pSrcFloater == '\n'))
	{
		pSrcFloater--;
		count--;
	}
	 pSrcFloater++;
	*pSrcFloater++ = '\r';
	*pSrcFloater++ = '\n';
	*pSrcFloater = 0;
	count += 2;

	*bytesRead = count;
	strcpy(pReturned, pTemp);
	delete [] pTemp;
	return (pReturned);
}






CImportMail::CImportMail()
{
	m_head = NULL;
	m_lpSession = NULL;
	m_LoggedOn = false;
	m_InstalledStatus = CMapiInstaller::STATUS_NOT_INSTALLED;

	FindProviders();
}

CImportMail::~CImportMail()
{
	EmptyTree(&m_head);
}


bool CImportMail::MAPILogon()	// Umm, edit this, it's not done
{
	HRESULT			hResult;	
	void * EudoraFunc = NULL;

	m_InstalledStatus = CMapiInstaller::Uninstall();

	m_hDLL = LoadLibrary("Mapi32");
	if (m_hDLL != NULL)
	{
		m_MAPIFreeBuffer					=	(CMIMAPIFreeBuffer)GetProcAddress(m_hDLL, "MAPIFreeBuffer");
		CMIMAPIInitialize MAPIInitialize	=	(CMIMAPIInitialize)GetProcAddress(m_hDLL, "MAPIInitialize");
		CMIMAPILogonEx MAPILogonEx			=	(CMIMAPILogonEx)GetProcAddress(m_hDLL, "MAPILogonEx");
		EudoraFunc							=	(void *)GetProcAddress(m_hDLL, "ISEUDORAMAPIDLL");

		if (!MAPIInitialize || !MAPILogonEx || EudoraFunc || !m_MAPIFreeBuffer)
		{
			// handle the error
			FreeLibrary(m_hDLL); 
			if (m_InstalledStatus != CMapiInstaller::STATUS_NOT_INSTALLED)
				CMapiInstaller::Install();
			return false;
		}

		hResult = MAPIInitialize(NULL);

		hResult = MAPILogonEx(NULL, NULL, NULL, MAPI_EXTENDED ,(LPMAPISESSION FAR *) &m_lpSession);
		if( HR_FAILED(hResult) || !m_lpSession)
		{
			ErrorDialog(IDS_IMPORT_MAPI_OPEN_ERROR, m_lpSession, hResult);
			// Logon Failed....
			if (m_InstalledStatus != CMapiInstaller::STATUS_NOT_INSTALLED)
				CMapiInstaller::Install();
			return false;
		}

		m_LoggedOn = true;
		return true;
	}
	else 
	{	
		if (m_InstalledStatus != CMapiInstaller::STATUS_NOT_INSTALLED)
			CMapiInstaller::Install();
		return false;
	}
}

void CImportMail::EmptyTree(ImportProvider **Tree)
{
	if (Tree == NULL)
		Tree = &m_head;

	if (*Tree == NULL)
		return;

	ImportProvider *trash = *Tree;
	if(trash->next)
		EmptyTree(&(trash->next));

	if (trash->FirstChild)
	{
		RemoveChildren(trash->FirstChild);
		trash->FirstChild = NULL;
	}
	trash->DisplayName ? delete [] trash->DisplayName : NULL;
	delete *Tree;
	*Tree = NULL;
}

void CImportMail::RemoveChildren(ImportChild *trash)
{
	if (trash->next)
	{
		RemoveChildren(trash->next);
		trash->next = NULL;
	}
	
	trash->PersonalityName ? delete [] trash->PersonalityName : NULL;
	trash->IncomingUserName ? delete [] trash->IncomingUserName : NULL;
	trash->IncomingServer ? delete [] trash->IncomingServer : NULL;
	trash->RealName ? delete [] trash->RealName : NULL;
	trash->EmailAddress ? delete [] trash->EmailAddress : NULL;
	trash->SMTPServer ? delete [] trash->SMTPServer : NULL;
	trash->AccountDisplayName ? delete [] trash->AccountDisplayName : NULL;
	trash->PathToData ? delete [] trash->PathToData : NULL;
	delete trash;
}

void CImportMail::FindProviders()
{
	LoadOL98Accounts();
	LoadOLExpressAccounts();
	LoadNSAccounts();
}

bool CImportMail::LoadOL98Accounts()
{
	HKEY hKey;
	HKEY hAccountKey;
	int MethodID;
	DWORD dwIndex=0;
	char *tempbuffer;
	DWORD tempbufsize = 128;
	HKEY hChildKey;
	int ChildUID = 0;
	FILETIME fileTime;

//	UnloadEudorasMapi();

	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Office\\8.0\\Outlook\\OMI Account Manager"),
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{

		MethodID = AddProvider(_T("Outlook 98"));
		
		tempbuffer = new char[tempbufsize];

		if (RegOpenKeyEx(hKey, _T("Accounts"), 0, KEY_READ, &hAccountKey) == ERROR_SUCCESS)
		{
			
			while (RegEnumKeyEx(hAccountKey, dwIndex++, tempbuffer, &tempbufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hAccountKey, tempbuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
				{
					if (ValidAccount(hChildKey))		// If SMTP Display Name exists then it's not an LDAP server entry.
						AddChildOL98(MethodID, hChildKey);

					RegCloseKey(hChildKey);
				}
			tempbufsize = 128;
			}
		}
		delete [] tempbuffer;
		RegCloseKey(hAccountKey);
		RegCloseKey(hKey);
		return true;
	}
	else return false;
}

bool CImportMail::ValidAccount(HKEY	account)	// We assume that if there's SMTP info then it's a mail account
{												// Otherwise it's an LDAP or some other account.
	char * trash = NULL;
	OL98ExtractValue(&trash, _T("SMTP Display Name"), account);
	if (trash)
	{
		delete [] trash;
		return true;
	}
	else
	{
		delete [] trash;
		return false;
	}
}

int CImportMail::AddChildOL98(int parent, HKEY hChildKey)
{
	ImportChild *pCurrentChild;
	DWORD dwKeyDataType;
	int ChildID = 0;
	char *szData = 0;
	DWORD	dwDataBufSize=128;

		pCurrentChild = CreateChildNode(m_head, parent);

		szData = new char[dwDataBufSize];
		if (!szData)
			return -1;

		if (!OL98ExtractValue(&(pCurrentChild->AccountDisplayName), _T("Account Name"), hChildKey))
		{
			delete [] szData;
			return -1;
		}

		if (!OL98ExtractValue(&(pCurrentChild->PersonalityName), _T("Account Name"), hChildKey))
		{
			delete [] szData;
			return -1;
		}

		if (!OL98ExtractValue(&(pCurrentChild->RealName), _T("SMTP Display Name"), hChildKey))
		{
			delete [] szData;
			return -1;
		}

		if (!OL98ExtractValue(&(pCurrentChild->EmailAddress), _T("SMTP Email Address"), hChildKey))
		{
			delete [] szData;
			return -1;
		}

		if (!OL98ExtractValue(&(pCurrentChild->SMTPServer), _T("SMTP Server"), hChildKey))
		{
			delete [] szData;
			return -1;
		}

		if (OL98ExtractValue(&(pCurrentChild->IncomingUserName), _T("IMAP User name"), hChildKey))
			pCurrentChild->IsIMAP = true;

		if (OL98ExtractValue(&(pCurrentChild->IncomingServer), _T("IMAP Server"), hChildKey))
			pCurrentChild->IsIMAP = true;

		if (OL98ExtractValue(&(pCurrentChild->IncomingUserName), _T("POP3 User name"), hChildKey))
			pCurrentChild->IsIMAP = false;

		if (OL98ExtractValue(&(pCurrentChild->IncomingServer), _T("POP3 Server"), hChildKey))
			pCurrentChild->IsIMAP = false;

		if (RegQueryValueEx(hChildKey, _T("Leave Mail On Server"), NULL, &dwKeyDataType,
			(LPBYTE) &(pCurrentChild->LMOS), &dwDataBufSize) == ERROR_SUCCESS)
		{
			dwDataBufSize = 128;
		}

		delete [] szData;
		return (pCurrentChild->AccountID);
}

int CImportMail::AddProvider(char * ProviderName)
{
	if (!ProviderName)
		return -1;

	ImportProvider *pCurrentProvider;
	int ProviderID = 0;

	pCurrentProvider = m_head;

	if (pCurrentProvider == NULL)
	{
		m_head = new ImportProvider;
		pCurrentProvider = m_head;
	}
	else
	{
		ProviderID++;					
		while (pCurrentProvider->next != NULL)
		{
			pCurrentProvider = pCurrentProvider->next;
			ProviderID++;
		}

		pCurrentProvider->next = new ImportProvider;
		pCurrentProvider = pCurrentProvider->next;
	}

	pCurrentProvider->DisplayName = new char[strlen(ProviderName)+1];
	strcpy(pCurrentProvider->DisplayName, ProviderName);
	
	pCurrentProvider->UniqueID = ProviderID;
	
	pCurrentProvider->icon = NULL;
	pCurrentProvider->FirstChild = NULL;
	pCurrentProvider->NumChildren = 0;
	pCurrentProvider->hasMail = 1;
	pCurrentProvider->hasAddresses = 1;
	pCurrentProvider->next = NULL;

	return pCurrentProvider->UniqueID;
}
bool CImportMail::LoadEudoraAccounts()
{



return true;



}
bool CImportMail::LoadOLExpressAccounts()
{
	HKEY hKey;
	HKEY hAccountKey;
	int MethodID;
	DWORD dwIndex=0;
	char *tempbuffer;
	DWORD tempbufsize = 128;
	HKEY hChildKey;
	int ChildUID = 0;
	FILETIME fileTime;

	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Internet Account Manager"),
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		// Should we enumerate through the list accounts? Arghhhh
		MethodID = AddProvider(_T("Outlook Express"));
		
		tempbuffer = new char[tempbufsize];

		if (RegOpenKeyEx(hKey, _T("Accounts"), 0, KEY_READ, &hAccountKey) == ERROR_SUCCESS)
		{
			
			while (RegEnumKeyEx(  hAccountKey, dwIndex++, tempbuffer, &tempbufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hAccountKey, tempbuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
				{
					AddChildOLExpress(MethodID, hChildKey);
					RegCloseKey(hChildKey);
				}
			tempbufsize = 128;
			}
		}
		delete [] tempbuffer;
		RegCloseKey(hAccountKey);
		RegCloseKey(hKey);
		return true;
	}
	else return false;
}

int CImportMail::AddChildOLExpress(int parent, HKEY hChildKey)
{
	ImportChild *pCurrentChild;
	DWORD dwKeyDataType;
	int ChildID = 0;
	char *szData;
	char * trash;
	DWORD	trashsize = 128;
	DWORD	dwDataBufSize=128;

	trash = new char[trashsize];

	if (RegQueryValueEx(hChildKey, _T("SMTP Display Name"), NULL, &dwKeyDataType,	// Let's just check that it's a valid EMAIL
																					// Account as opposed to an LDAP account
		(LPBYTE) trash, &trashsize) == ERROR_SUCCESS)
		{
		delete [] trash;

		pCurrentChild = CreateChildNode(m_head, parent);

		if(!OL98ExtractValue(&(pCurrentChild->AccountDisplayName), _T("Account Name"), hChildKey))
			return -1;

		if(!OL98ExtractValue(&(pCurrentChild->PersonalityName), _T("Account Name"), hChildKey))
			return -1;

		if(!OL98ExtractValue(&(pCurrentChild->RealName), _T("SMTP Display Name"), hChildKey))
			return -1;

		if(!OL98ExtractValue(&(pCurrentChild->EmailAddress), _T("SMTP Email Address"), hChildKey))
			return -1;

		if(!OL98ExtractValue(&(pCurrentChild->SMTPServer), _T("SMTP Server"), hChildKey))
			return -1;

		if(OL98ExtractValue(&(pCurrentChild->IncomingUserName), _T("IMAP User name"), hChildKey))
			pCurrentChild->IsIMAP = true;

		if(OL98ExtractValue(&(pCurrentChild->IncomingServer), _T("IMAP Server"), hChildKey))
			pCurrentChild->IsIMAP = true;

		if(OL98ExtractValue(&(pCurrentChild->IncomingUserName), _T("POP3 User name"), hChildKey))
			pCurrentChild->IsIMAP = false;

		if(OL98ExtractValue(&(pCurrentChild->IncomingServer), _T("POP3 Server"), hChildKey))
			pCurrentChild->IsIMAP = false;

		szData = new char[dwDataBufSize];

		if (RegQueryValueEx(hChildKey, _T("Leave Mail On Server"), NULL, &dwKeyDataType,
			(LPBYTE) &(pCurrentChild->LMOS), &dwDataBufSize) == ERROR_SUCCESS)
		{
			dwDataBufSize = 128;
		}
		else
		{
			pCurrentChild->LMOS = false;
		}

		delete [] szData;
		return (pCurrentChild->AccountID);
	}
	else
	{
		delete [] trash;
		return -1;
	}
}

bool CImportMail::LoadNSAccounts()
{
	HKEY hKey;
	int MethodID;
	DWORD dwIndex=0;
	char *tempbuffer;
	DWORD tempbufsize = 128;
	HKEY hChildKey;
	int ChildUID = 0;
	FILETIME fileTime;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("Software\\NetScape\\Netscape Navigator\\Users"),
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		MethodID = AddProvider(_T("Netscape Navigator"));
		
		tempbuffer = new char[tempbufsize];

		while (RegEnumKeyEx(hKey, dwIndex++, tempbuffer, &tempbufsize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
		{
			if(RegOpenKeyEx(hKey, tempbuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
			{
				AddChildNS(MethodID, hChildKey, tempbuffer);
				RegCloseKey(hChildKey);
			}
		tempbufsize = 128;
		}
		delete [] tempbuffer;
		RegCloseKey(hKey);
		return true;
	}
	else return false;
}


int CImportMail::AddChildNS(int parent, HKEY hChildKey, char *PersName)
{
	char * floater;
	char * tempBuf;
	ImportChild *pCurrentChild;
	DWORD dwKeyDataType;
	int ChildID = 0;
	char *szData;
	DWORD	dwDataBufSize=512;
	char *prefsFileName;

	szData = new char[dwDataBufSize];

	if (RegQueryValueEx(hChildKey, _T("DirRoot"), NULL, &dwKeyDataType,
		(LPBYTE) szData, &dwDataBufSize) != ERROR_SUCCESS)
	{

		return false;
	}
	dwDataBufSize = 512;

	prefsFileName = new char[strlen(szData)+10];	// enough for prefs.js\0
	// Open File
	strcpy(prefsFileName, szData);
	strcat(prefsFileName, _T("\\prefs.js"));

	ifstream prefsFile( prefsFileName, ios::nocreate, filebuf::sh_read);
	if ( prefsFile.fail() )
	// The file does not exist ...
	{
		delete [] prefsFileName;
		delete [] szData;
		return false;
	}

	pCurrentChild = CreateChildNode(m_head, parent);

	if (pCurrentChild->AccountDisplayName)
		delete [] pCurrentChild->AccountDisplayName;

	pCurrentChild->AccountDisplayName = new char[strlen(PersName)+1];
	strcpy(pCurrentChild->AccountDisplayName,PersName);

	if (pCurrentChild->PersonalityName)
		delete [] pCurrentChild->PersonalityName;

	pCurrentChild->PersonalityName = new char[strlen(PersName)+1];
	strcpy(pCurrentChild->PersonalityName,PersName);

	if (pCurrentChild->PathToData)
		delete [] pCurrentChild->PathToData;

	pCurrentChild->PathToData = new char[strlen(szData)+1];
	strcpy(pCurrentChild->PathToData, szData);

	delete [] szData;
	szData = NULL;

	tempBuf = new char[512];
	
	while (prefsFile.getline(tempBuf, 510))
	{
		if (strstr(tempBuf, _T("user_pref(\"mail.")))
		{
			if (strstr(tempBuf, _T("user_pref(\"mail.identity.useremail")))
			{
				NSExtractValue(&(pCurrentChild->EmailAddress), tempBuf);
			}
			else if (strstr(tempBuf, _T("user_pref(\"mail.identity.username")))
			{
				NSExtractValue(&(pCurrentChild->RealName), tempBuf);
			}
			else if (strstr(tempBuf, _T("user_pref(\"mail.leave_on_server")))
			{
				floater = strrchr(tempBuf, 0x2c);  //2C is a comma
				floater += 2;			// was three, should be 2 to catch 't' in true -jdboyd
				if (*floater == 't')
				{
					pCurrentChild->LMOS = true;
				}					
			}
			else if (strstr(tempBuf, _T("user_pref(\"mail.pop_name")))
			{
				NSExtractValue(&(pCurrentChild->IncomingUserName), tempBuf);
			}
			else if (strstr(tempBuf, _T("user_pref(\"mail.server_type")))	// server type -jdboyd
			{
				floater = strrchr(tempBuf, 0x2c);  //2C is a comma
				floater += 2;
				if (*floater == '1')
				{
					pCurrentChild->IsIMAP = true;
				}					
			}
		}
		else if(strstr(tempBuf, _T("user_pref(\"network.hosts.")))
		{
			if (strstr(tempBuf, _T("user_pref(\"network.hosts.pop_server")))
			{
				NSExtractValue(&(pCurrentChild->IncomingServer), tempBuf);
			}
			else if (strstr(tempBuf, _T("user_pref(\"network.hosts.smtp_server")))
			{
				NSExtractValue(&(pCurrentChild->SMTPServer), tempBuf);	
			}
		}
	}
	prefsFile.close();
	delete [] prefsFileName;
	delete [] tempBuf;

	// For now Netscape cannot import addresses, Oh well, let's make sure it's never an option.
	// Instead it i,ports from an LDIF file.
	pCurrentChild->parent->hasAddresses = false;
	pCurrentChild->parent->hasLdif = true;

	// Here we figure out if you have any ldif files we can use.
	struct _finddata_t c_file;
	long m_hFile;

	if (_chdir(pCurrentChild->PathToData))
	{
		pCurrentChild->PathtoLdif = NULL;
	}
	else
	{
		if( (m_hFile = _findfirst( "*.ldif", &c_file )) == -1L )
		{
			pCurrentChild->PathtoLdif = NULL;
		}
		else
		{
			pCurrentChild->PathtoLdif = new char[strlen(pCurrentChild->PathToData) + strlen(c_file.name) +2];
			strcpy(pCurrentChild->PathtoLdif, pCurrentChild->PathToData);
			strcat(pCurrentChild->PathtoLdif, "\\");
			strcat(pCurrentChild->PathtoLdif, c_file.name);
		}
	}

	return true;
}


ImportChild *CImportMail::CreateChildNode(ImportProvider * head, int parent)
{
	ImportProvider *pCurrentProvider = head;
	ImportChild	*pCurrentChild;
	int ChildID = 0;
	while (pCurrentProvider->UniqueID != parent)
	{
		pCurrentProvider = pCurrentProvider->next;
	}
	pCurrentProvider->NumChildren++;
	
	pCurrentChild = pCurrentProvider->FirstChild;

	if (pCurrentChild == NULL)
	{
		pCurrentProvider->FirstChild = new ImportChild;
		pCurrentChild = pCurrentProvider->FirstChild;
	}
	else
	{
		ChildID++;			// Get a bonus increment just for being here, we then add one for each level after this.
							// By being here we're naturally at the second level.
		while (pCurrentChild->next != NULL)
		{
			ChildID++;
			pCurrentChild = pCurrentChild->next;
		}

		pCurrentChild->next = new ImportChild;
		pCurrentChild =	pCurrentChild->next;
	}
	pCurrentChild->parent = pCurrentProvider;
	pCurrentChild->AccountID = ChildID;

	pCurrentChild->hasMail = false;
	pCurrentChild->hasAddresses = false;

	// Populate with empty String so we don't crash when Scott tries to display it :)
	pCurrentChild->AccountDisplayName = new char[1];
	pCurrentChild->AccountDisplayName[0] = 0;

	pCurrentChild->PathToData = new char[1];
	pCurrentChild->PathToData[0] = 0;

	pCurrentChild->PersonalityName = new char[1];
	pCurrentChild->PersonalityName[0] = 0;

	pCurrentChild->IncomingUserName = new char[1];
	pCurrentChild->IncomingUserName[0] = 0;

	pCurrentChild->IncomingServer = new char[1];
	pCurrentChild->IncomingServer[0] = 0;

	pCurrentChild->RealName = new char[1];
	pCurrentChild->RealName[0] = 0;

	pCurrentChild->EmailAddress = new char[1];
	pCurrentChild->EmailAddress[0] = 0;

	pCurrentChild->SMTPServer = new char[1];
	pCurrentChild->SMTPServer[0] = 0;

	pCurrentChild->next = NULL;
	pCurrentChild->LMOS = false;
	pCurrentChild->IsIMAP = false;
	
	return pCurrentChild;
}

long CImportMail::NumOfPersonalities()
{
	long returnval = 0;
	ImportProvider *current = m_head;

	while (current)
	{
		returnval += current->NumChildren;
		current = current->next;
	}
	return returnval;
}

bool CImportMail::OL98ExtractValue(char ** Dest, char *KeyName, HKEY hChildKey)
{
	DWORD dwDataBufSize=128;
	DWORD dwKeyDataType;
	char * szData;

	szData = new char[dwDataBufSize];
	if (!szData)
		return false;

	if (RegQueryValueEx(hChildKey, KeyName, NULL, &dwKeyDataType,
		(LPBYTE) szData, &dwDataBufSize) == ERROR_SUCCESS)
	{
		if (*Dest != NULL)	// Some other value has been filled in previously and is now going to be overridden
		{					// Most likely an IMAP server was first and it's going to be overridden with a POP server
			delete [] *Dest;
			*Dest = NULL;
		}

		*Dest = new char[dwDataBufSize+1];
		if (!*Dest)
		{
			delete [] szData;
			return false;
		}

		strcpy(*Dest, szData);
		{
			delete [] szData;
			return true;
		}
	}
	else
	{
		delete [] szData;
		return false;
	}
}

bool CImportMail::NSExtractValue(char ** Dest, char * Line)
{
	char * floater;

	if (*Dest)
	{
		delete [] *Dest;
		*Dest = NULL;
	}
	
	floater = strrchr(Line, 0x22);		// 22 is a quote character
	*floater = 0;
	floater = strrchr(Line, 0x2c);		// 2C is a comma
	floater += 3;
	*Dest = new char[strlen(floater)+1];
	strcpy(*Dest, floater);

	return true;
}

bool CImportMail::MAPILogout(bool InvokeHack)
{
	CMIMAPIUnInitialize MAPIUnInitialize	=	(CMIMAPIUnInitialize)GetProcAddress(m_hDLL, "MAPIUninitialize");

	if (m_lpSession)
	{
		m_lpSession->Logoff(NULL, NULL, 0);
		m_lpSession->Release();

		m_MAPIFreeBuffer(m_lpSession);
		m_lpSession = NULL;
	}

	MAPIUnInitialize();

	FreeLibrary(m_hDLL);

	if (m_InstalledStatus != CMapiInstaller::STATUS_NOT_INSTALLED)
		CMapiInstaller::Install();
	return true;
}

// We need the child, not the provider because in some programs, like Netscape you can have different
// Personalities with different mail folders for each.
bool CImportMail::ImportMail(ImportChild *Child, CWnd * pParentWnd)
{
	if (!strcmp(_T("Outlook Express"), Child->parent->DisplayName))
	{
		OEMboxConverter	OEConv;
		if (!OEConv.Convert(Child))
			return false;
		// Import Outlook Express's mail Store
	}
	else if (!strcmp(_T("Outlook 98"), Child->parent->DisplayName))
	{
		// Import Outlook 98's Mailstore
		if (!MAPILogon())
		{
			ErrorDialog(IDS_IMPORT_MAPI_UNKNOWN_ERROR);
			return false;
		}

		MAPIImport	Outlook;
		Outlook.Convert(m_hDLL, m_lpSession);	// We're going to store the session pointer in the CImportMail struct instead of in the individual
											// Converters because we're going to want to keep it persistent from mail conversion to address
											// conversion, and it must be done in that order.
		MAPILogout();

	}
	else if (!strcmp(_T("Netscape Navigator"), Child->parent->DisplayName))
	{
		NSMboxConverter	NSConv;
		if (!NSConv.Convert(Child))
			return false;
	}
	return true;
}

bool CImportMail::ImportAddresses(ImportChild *Child, CWnd * pParentWnd)
{
	if (!strcmp(_T("Outlook Express"), Child->parent->DisplayName))
	{
		OEAddrConverter	OEConv;
		OEConv.Convert(Child);
		// Import Outlook Express's addressbook
	}
	else if (!strcmp(_T("Outlook 98"), Child->parent->DisplayName))
	{
		// Import Outlook 98's Addresses
		MAPILogon();

		MAPIAddrImport	Outlook;
		Outlook.Convert(m_hDLL, m_lpSession);

		MAPILogout(TRUE);
	}
	else if (!strcmp(_T("Netscape Navigator"), Child->parent->DisplayName))
	{
	//	NSLdifConverter	blah;
	//	blah.Convert(Child);
		return false;
	}

	ErrorDialog(IDS_ADDR_RESTART);
// Update views of addresbooks

	return true;
}
bool CImportMail::ImportLdif(ImportChild *Child, LPCSTR File, CWnd *pParentWnd)
{
	if (strcmp(_T("Netscape Navigator"), Child->parent->DisplayName))	// This can only be called on Netscape Children.
	{
		return false;
	}

	NSLdifConverter	blah;
	bool returnval = blah.Convert(File);

	ErrorDialog(IDS_ADDR_RESTART);

	return returnval;
}


bool CImportMail::ImportPersonality(ImportChild *Personality)
{
	ASSERT(Personality);
	if (Personality)
	{
		CPersParams parms;
		parms.GetDefaultParams();

		if (Personality->PersonalityName)
			parms.PersName = Personality->PersonalityName;

		if (Personality->IncomingUserName)
			parms.POPAccount = Personality->IncomingUserName;

		if (Personality->IncomingServer)
		{
			parms.POPAccount += "@";
			parms.POPAccount += Personality->IncomingServer;
		}
		if (Personality->RealName)
			parms.RealName = Personality->RealName;

		if (Personality->EmailAddress)
			parms.ReturnAddress = Personality->EmailAddress;

		if (Personality->SMTPServer)
			parms.OutServer = Personality->SMTPServer;

		parms.Stationery = "";
		parms.Signature = "";
		parms.bCheckMail = TRUE;
		parms.bLMOS = Personality->LMOS;
		parms.bIMAP = Personality->IsIMAP;

		if (Personality->IncomingServer)
			parms.InServer = Personality->IncomingServer;

		if (Personality->IncomingUserName)
			parms.LoginName = Personality->IncomingUserName;

		parms.bKerberos = FALSE;
		parms.bAPop = Personality->IsIMAP;
		parms.bWinSock = TRUE;
		parms.bRPA		= FALSE;

		g_Personalities.Add(parms);

		UpdatePersonalityWazoo();

		return true;
	}
	return false;
}

void CImportMail::UpdatePersonalityWazoo()	// Why *I* have to do this is beyond me, you'd think a wazoo would 
											// figure out it's been changed all by itself, but I digress.
{
		CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();	//FORNOW
		ASSERT_KINDOF(CMainFrame, pMainWnd);						//FORNOW
		
		CPersonalityView *pView	=	pMainWnd->GetActivePersonalityView() ;
		ASSERT( pView);
		
		pView->PopulateView() ;		
}


bool MAPIAddrImport::Initialize()
{
	m_MAPIFreeBuffer	=	(CMIMAPIFreeBuffer)GetProcAddress(m_hDLL, "MAPIFreeBuffer");
	if (!m_MAPIFreeBuffer)
		return false;


	return true;
}

bool MAPIAddrImport::Convert(HINSTANCE hDLL, LPMAPISESSION lpSession)
{
	m_hDLL = hDLL;
	m_lpSession = lpSession;

	ULONG lsdhf = GetRefcount(m_lpSession);

	char *filename;
	char *NNFilePath;
	AbookCreator *abookcreator;
	abookcreator = new AbookCreator;

	if (!Initialize())
		return false;

	if (!OpenAddressBook())
		return false;

	lsdhf = GetRefcount(m_lpSession);

	filename = new char[strlen(_T("Outlook 98.txt"))+1];
	strcpy(filename, _T("Outlook 98.txt"));

	if (!abookcreator->FindNNFileName(&filename, &NNFilePath))
		return false;

	if (!abookcreator->OpenNNFile(NNFilePath))
		return false;

	if (!ProcessABook(abookcreator))
	{
		return false;
	}
	abookcreator->CloseNNFile();

	CNicknameFile* NickFile = g_Nicknames->AddNicknameFile(NNFilePath);

	if (NickFile)
		g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_ADDED_NEW_NICKFILE, NickFile);

	CloseAddressBook();

	delete [] filename;
	delete [] NNFilePath;
	delete abookcreator;
	return true;
}

void MAPIAddrImport::CloseAddressBook()
{
	// Funny, Seems if you try to clean up after yourself you crash, so fine,
	// lets leave these handles open. I guess when we logoff it decides to clean them up...

/*	ULONG lsdhf = GetRefcount(m_lpSession);

	lsdhf = m_lpABC->Release();
	m_MAPIFreeBuffer(m_lpABC);
	m_lpABC = NULL;
	lsdhf = m_lpAdrBook->Release();
	m_lpSession->Release();

	lsdhf = m_lpAdrBook->Release();
	m_MAPIFreeBuffer(m_lpAdrBook);
	m_lpAdrBook = NULL;
	lsdhf = m_lpSession->Release();
*/
}

bool MAPIAddrImport::OpenAddressBook()
{
	HRESULT			hresult;
	ULONG			ulcbEntryID;
	LPENTRYID		lpEntryID = NULL;
	ULONG			ulObjType;

	if (!m_lpSession)
	{
		ErrorDialog(IDS_IMPORT_ADDR_OPENING_ERROR);
		return false;
	}
	hresult = m_lpSession->OpenAddressBook( NULL, NULL, AB_NO_DIALOG, &m_lpAdrBook);
	if (HR_FAILED(hresult) || !m_lpAdrBook)
	{		
		ErrorDialog(IDS_IMPORT_ADDR_OPENING_ERROR2, m_lpAdrBook, hresult);
		return false;
	}

	hresult = m_lpAdrBook->GetDefaultDir(&ulcbEntryID, &lpEntryID);
	if (HR_FAILED(hresult) || !lpEntryID)
	{
		ErrorDialog(IDS_IMPORT_ADDR_OPENING_ERROR3, lpEntryID, hresult);

		if (m_lpAdrBook)
			m_MAPIFreeBuffer(m_lpAdrBook);
		return false;
	}

	hresult = m_lpAdrBook->OpenEntry(ulcbEntryID, lpEntryID, NULL, NULL, &ulObjType, (LPUNKNOWN*)&m_lpABC);
	if (HR_FAILED(hresult) || !m_lpABC)
	{		
		ErrorDialog(IDS_IMPORT_ADDR_OPENING_ERROR4, m_lpABC, hresult);

		if (m_lpAdrBook)
			m_MAPIFreeBuffer(m_lpAdrBook);
		return false;
	}

	return true;
}

bool MAPIAddrImport::ProcessABook(AbookCreator *abookcreator)
{
	HRESULT			hresult;
	LPMAPITABLE		lpContTable;
	LPSPropTagArray	lpPropTagArray;
	LPSRowSet		lpRowSet;
	SizedSPropTagArray	(2, AbookProps) =
	{
		2,
		{
			PR_DISPLAY_TYPE,
			PR_ENTRYID
		}
	};

	LPENTRYID	EntryIDValue;
	ULONG		EntryIDSize;
	ULONG		EntryObjectType;
	ULONG		i;

	ULONG		ABType;
	LPMAPIPROP	ABEntry;

	hresult = m_lpABC->GetContentsTable(NULL, &lpContTable);
	if (HR_FAILED(hresult))
	{
		return false;
	}

	hresult = lpContTable->QueryColumns(NULL, &lpPropTagArray);
	if (HR_FAILED(hresult))
	{
		lpContTable->Release();
		m_MAPIFreeBuffer(lpContTable);
		return false;
	}

	hresult = lpContTable->SetColumns((LPSPropTagArray)&AbookProps, NULL);
	if (HR_FAILED(hresult))
	{
		lpContTable->Release();
		m_MAPIFreeBuffer(lpContTable);
		m_MAPIFreeBuffer(lpPropTagArray);
		return false;
	}
	while (!HR_FAILED(hresult = lpContTable->QueryRows(1,0, &lpRowSet)))
	{
		if (lpRowSet->cRows != 0)
		{
			for (i = 0; i < lpRowSet->aRow[0].cValues; i++)
			{
				if ((lpRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_ENTRYID & 0xFFFF0000))
				{
					EntryIDValue = (LPENTRYID)(lpRowSet->aRow[0].lpProps[i].Value.bin.lpb);
					EntryIDSize	= (ULONG)(lpRowSet->aRow[0].lpProps[i].Value.bin.cb);
				}
				else if ((lpRowSet->aRow[0].lpProps[i].ulPropTag & 0xFFFF0000) == (PR_DISPLAY_TYPE & 0xFFFF0000))
				{
					EntryObjectType = (ULONG)lpRowSet->aRow[0].lpProps[i].Value.ul;
				}
			}
			
			if ((EntryIDValue) && (EntryObjectType == DT_MAILUSER))
			{
				hresult = m_lpSession->OpenEntry( EntryIDSize, EntryIDValue, NULL, NULL, &ABType, (LPUNKNOWN*)&ABEntry);
				if (HR_FAILED(hresult))
				{
					if (lpContTable)
					{
						lpContTable->Release();
						m_MAPIFreeBuffer(lpContTable);
					}
					if (lpPropTagArray)
						m_MAPIFreeBuffer(lpPropTagArray);
					MAPIImport::FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
						return (false);
				}
				hresult = ProcessABEntry(ABEntry, abookcreator);
				ABEntry->Release();
				m_MAPIFreeBuffer(&ABEntry);
				ABEntry = NULL;
			}
			
			MAPIImport::FreeRowSet(m_MAPIFreeBuffer, lpRowSet);
			if (lpPropTagArray)
				m_MAPIFreeBuffer(lpPropTagArray);
			lpRowSet = NULL;
		}
		else break;
	}
	return true;
}


bool MAPIAddrImport::ProcessABEntry(LPMAPIPROP ABEntry, AbookCreator *abookcreator)
{
	HRESULT hresult;
	LPSPropTagArray	AllPropValues;
	SizedSPropTagArray	(31, EntryProps) =
	{
		31,
		{
			PR_ENTRYID,
			PR_DISPLAY_NAME,
			PR_COMMENT,
			PR_GIVEN_NAME,
			PR_BUSINESS_TELEPHONE_NUMBER,
			PR_HOME_TELEPHONE_NUMBER,
			PR_SURNAME,
			PR_COMPANY_NAME,
			PR_TITLE,
			PR_DEPARTMENT_NAME,
			PR_OFFICE_LOCATION,
			PR_MOBILE_TELEPHONE_NUMBER,
			PR_PAGER_TELEPHONE_NUMBER,
			PR_BUSINESS_FAX_NUMBER,
			PR_HOME_FAX_NUMBER,
			PR_COUNTRY,
			PR_LOCALITY,
			PR_STATE_OR_PROVINCE,
			PR_STREET_ADDRESS,
			PR_POSTAL_CODE,
			PR_MIDDLE_NAME,
			PR_NICKNAME,
			PR_PERSONAL_HOME_PAGE,
			PR_BUSINESS_HOME_PAGE,
			PR_CONTACT_EMAIL_ADDRESSES,
			PR_HOME_ADDRESS_CITY,
			PR_HOME_ADDRESS_COUNTRY,
			PR_HOME_ADDRESS_POSTAL_CODE,
			PR_HOME_ADDRESS_STATE_OR_PROVINCE,
			PR_HOME_ADDRESS_STREET
		}
	};

	AbookEntry *Entry;
	Entry = new AbookEntry;

	ULONG			i;
	ULONG			NumProps;
	LPSPropValue	PropValues;


	hresult = ABEntry->GetPropList(NULL, &AllPropValues);

	hresult = ABEntry->GetProps(AllPropValues, NULL, &NumProps, &PropValues);

	for (i = 0; i <= NumProps; i++)
	{
		switch(PropValues[i].ulPropTag)
		{

		case PR_COMMENT:
			{
				Entry->notes = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->notes, PropValues[i].Value.lpszA);
				break;
			}
		case PR_GIVEN_NAME:
			{
				Entry->first_name = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->first_name, PropValues[i].Value.lpszA);
				break;
			}
		case PR_BUSINESS_TELEPHONE_NUMBER:
			{
				Entry->wrk_phone = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_phone, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_TELEPHONE_NUMBER:
			{
				Entry->phone = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->phone, PropValues[i].Value.lpszA);
				break;
			}
		case PR_SURNAME:
			{
				Entry->last_name = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->last_name, PropValues[i].Value.lpszA);
				break;
			}
		case PR_COMPANY_NAME:
			{
				Entry->company = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->company, PropValues[i].Value.lpszA);
				break;
			}
		case PR_TITLE:
			{
				Entry->wrk_title = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_title, PropValues[i].Value.lpszA);
				break;
			}
		case PR_DEPARTMENT_NAME:
			{
				Entry->wrk_dept = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_dept, PropValues[i].Value.lpszA);
				break;
			}
		case PR_OFFICE_LOCATION:
			{
				Entry->wrk_office = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_office, PropValues[i].Value.lpszA);
				break;
			}
		case PR_MOBILE_TELEPHONE_NUMBER:
			{
				Entry->cell_phone = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->cell_phone, PropValues[i].Value.lpszA);
				break;
			}
		case PR_PAGER_TELEPHONE_NUMBER:
			{
				Entry->wrk_pgr = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_pgr, PropValues[i].Value.lpszA);
				break;
			}
		case PR_BUSINESS_FAX_NUMBER:
			{
				Entry->wrk_fax = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_fax, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_FAX_NUMBER:
			{
				Entry->fax = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->fax, PropValues[i].Value.lpszA);
				break;
			}
		case PR_COUNTRY:
			{
				Entry->wrk_country = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_country, PropValues[i].Value.lpszA);
				break;
			}
		case PR_LOCALITY:
			{
				Entry->wrk_city = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_city, PropValues[i].Value.lpszA);
				break;
			}
		case PR_STATE_OR_PROVINCE:
			{
				Entry->wrk_state = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_state, PropValues[i].Value.lpszA);
				break;
			}
		case PR_STREET_ADDRESS:
			{
				Entry->wrk_street_addr = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_street_addr, PropValues[i].Value.lpszA);
				break;
			}
		case PR_POSTAL_CODE:
			{
				Entry->wrk_zip = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_zip, PropValues[i].Value.lpszA);
				break;
			}
		case PR_MIDDLE_NAME:
			{
				Entry->middle_name = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->middle_name, PropValues[i].Value.lpszA);
				break;
			}
		case PR_NICKNAME:
			{
				Entry->nickname = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->nickname, PropValues[i].Value.lpszA);
				break;
			}
		case PR_PERSONAL_HOME_PAGE:
			{
				Entry->web_page = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->web_page, PropValues[i].Value.lpszA);
				break;
			}
		case PR_BUSINESS_HOME_PAGE:
			{
				Entry->wrk_web_page = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->wrk_web_page, PropValues[i].Value.lpszA);
				break;
			}
		case PR_CONTACT_EMAIL_ADDRESSES:
			{
				ULONG	count;
				ULONG	totalSize = 0;
				char *curAddr;
				char * floater;
				ULONG	j;

				// First we're going to go thru and figure out how much space we need to allocate.
				count = PropValues[i].Value.MVszA.cValues;
				for (j = 0; j < count; j++)
				{
					totalSize += strlen(PropValues[i].Value.MVszA.lppszA[j]);
					totalSize += 1;	// for the comma
				}
				totalSize ++;		// For the Null character

				Entry->addresses = new char[totalSize];
				if (Entry->addresses == NULL)	// If it's too big, just screw it.
					break;

				Entry->addresses[0] = 0;

				for (j = 0; j < count; j++)
				{
					curAddr = new char[strlen(PropValues[i].Value.MVszA.lppszA[j])+1];
					strcpy(curAddr, (PropValues[i].Value.MVszA.lppszA[j]));
					strcat(Entry->addresses, curAddr);
					strcat(Entry->addresses, _T(","));
					delete [] curAddr;
				}
 
				floater = strrchr(Entry->addresses, ',');

				*floater = 0;
				break;
			}
		case PR_HOME_ADDRESS_CITY:
			{
				Entry->city = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->city, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_COUNTRY:
			{
				Entry->country = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->country, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_POSTAL_CODE:
			{
				Entry->zip = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->zip, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_STATE_OR_PROVINCE:
			{
				Entry->state = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->state, PropValues[i].Value.lpszA);
				break;
			}
		case PR_HOME_ADDRESS_STREET:
			{
				Entry->street_addr = new char[strlen(PropValues[i].Value.lpszA)+1];
				strcpy(Entry->street_addr, PropValues[i].Value.lpszA);
				break;
			}
		default:
			{
				break;
			}
		}
	}
	
	abookcreator->AddEntry(Entry);

	delete Entry;

	return true;
}



MAPIAddrImport::MAPIAddrImport()
{
	m_lpSession = NULL;
	m_lpABC		= NULL;
	m_MAPIFreeBuffer = NULL;
}

MAPIAddrImport::~MAPIAddrImport()
{

}













/*
 -  FreeRowSet(LPSRowSet)
 -
 *  Purpose:
 *      Free all the rows in a rowset then the rowset itself
 *
 *  Parameters:
 *      lpRows    - pointer to rows to free
 *
 *  Returns:
 *      BOOL Pass/Fail  - Pass == 1 FAIL == 0
 *
 */

bool MAPIImport::FreeRowSet(CMIMAPIFreeBuffer MAPIFreeBufferfunc, LPSRowSet lpRows)
{
    ULONG   n;
    bool    fRet    = false;

    if(lpRows)
    {
        for( n = 0 ; n < lpRows->cRows ; n++)
        {
            if( MAPIFreeBufferfunc(lpRows->aRow[n].lpProps ) )
            {
                goto end;
            }
        }
        if( MAPIFreeBufferfunc(lpRows) )
        {
            goto end;
        }
    }
    fRet = true;
end:
    return(fRet);
}
