// MAPIImport.cpp
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>

#include "mapix.h"
#include "mapiutil.h"
#include "MAPIImport.h"
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
#include "lex822.h"
#include "mime.h"
#include "FileLineReader.h"
#include "header.h"
#include <wchar.h>

#include "nsprefs.h"
#include "MultiReader.h"
#include "utils.h"  //for QCLoadString

#ifdef IMAP4
#include "QCMailboxDirector.h"
extern QCMailboxDirector g_theMailboxDirector;
#endif


#include "DebugNewHelpers.h"


extern QCMailboxDirector		g_theMailboxDirector;
extern QCPersonalityDirector	g_thePersonalityDirector;
extern BOOL NicknameFileExists(const char* nickFilename);


#define TEXT_PLAIN	0
#define TEXT_RICH	1
#define TEXT_HTML	2

#define OUTLOOK_98		0
#define OUTLOOK_2000	1

extern CString ExecutableDir;

AbookEntry::AbookEntry()
{
	first_name			= NULL;
	middle_name			= NULL;
	last_name			= NULL;
	title				= NULL;
	nickname			= NULL;
	default_add			= NULL;
	addresses			= NULL;
	street_addr			= NULL;
	city				= NULL;
	state				= NULL;
	zip					= NULL;
	country				= NULL;
	phone				= NULL;
	fax					= NULL;
	cell_phone			= NULL;
	web_page			= NULL;
	company				= NULL;
	wrk_street_addr		= NULL;
	wrk_city			= NULL;
	wrk_state			= NULL;
	wrk_zip				= NULL;
	wrk_country			= NULL;
	wrk_title			= NULL;
	wrk_dept			= NULL;
	wrk_office			= NULL;
	wrk_phone			= NULL;
	wrk_fax				= NULL;
	wrk_pgr				= NULL;
	wrk_ip_phone		= NULL;
	wrk_web_page		= NULL;
	spouse				= NULL;
	children			= NULL;
	gender				= NULL;
	birthday			= NULL;
	anniversary			= NULL;
	notes				= NULL;
	conf_server			= NULL;
	def_conf_add		= NULL;
	conf_addresses		= NULL;

	type				= 0;
	m_lImportID			= 0;
	next				= NULL;
}

AbookEntry::~AbookEntry()
{
	delete [] first_name;
	delete [] middle_name;
	delete [] last_name;
	delete [] title;
	delete [] nickname;
	delete [] default_add;
	delete [] addresses;
	delete [] street_addr;
	delete [] city;
	delete [] state;
	delete [] zip;
	delete [] country;
	delete [] phone;
	delete [] fax;
	delete [] cell_phone;
	delete [] web_page;
	delete [] company;
	delete [] wrk_street_addr;
	delete [] wrk_city;
	delete [] wrk_state;
	delete [] wrk_zip;
	delete [] wrk_country;
	delete [] wrk_title;
	delete [] wrk_dept;
	delete [] wrk_office;
	delete [] wrk_phone;
	delete [] wrk_fax;
	delete [] wrk_pgr;
	delete [] wrk_ip_phone;
	delete [] wrk_web_page;
	delete [] spouse;
	delete [] children;
	delete [] gender;
	delete [] birthday;
	delete [] anniversary;
	delete [] notes;
	delete [] conf_server;
	delete [] def_conf_add;
	delete [] conf_addresses;

	delete [] next;
}
AbookCreator::AbookCreator()
{
	m_head = NULL;
	m_NNFile = NULL;
}

AbookCreator::~AbookCreator()
{
	CString		str;
	while (m_sarrNicknames.GetSize())
	{
		str = m_sarrNicknames.GetAt(0);
		str.Empty();
		m_sarrNicknames.RemoveAt(0);
	}
}

bool AbookCreator::NicknameIsUnique(char *nickname)
{
	int			i = 0;
	int			iItems = m_sarrNicknames.GetSize();
	bool		bIsUnique = true;
	CString		strName;

	for (i = 0; (i < iItems) && bIsUnique; ++i)
	{
		strName = m_sarrNicknames.GetAt(i);
		if (strName.Compare(nickname) == 0)
		{
			bIsUnique = false;
		}
	}
	return bIsUnique;
}

void AbookCreator::UniquifyNickname(char **nickname)
{
	int i = 0;

	if (!NicknameIsUnique(*nickname))
	{
		char	*szNewNickname = DEBUG_NEW char[strlen(*nickname) + 5];
		do
		{
			++i;
			if (i == 32768)		return;		// If they have this many identical nicknames just forget it
			sprintf(szNewNickname, "%s%d", *nickname, i);
		} while (!NicknameIsUnique(szNewNickname));
		delete [] *nickname;
		*nickname = szNewNickname;
	}
}

void AbookCreator::AddToList(char * nickname)
{
	m_sarrNicknames.Add(nickname);
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
			returnMe += static_cast<char>(3);

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
			line = DEBUG_NEW char[strlen(head)+1];
			strcpy(line, head);
			done = true;
		}
		else if (!floater)
		{
			char ch;
			line = DEBUG_NEW char[(floater2 - head) +1];
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
			line = DEBUG_NEW char[(floater - head) +1];
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
			line = DEBUG_NEW char[(floater - head) +1];
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
	CString notes;
	CString nick = nickname;
	CString temp;

	bool	bNotesFieldUsed		= (Entry->notes != NULL);
	bool	bEmailFieldUsed		= (Entry->addresses != NULL);
	bool	bHomeFieldsUsed		= (Entry->street_addr || Entry->city || Entry->state || Entry->zip || Entry->country || 
								   Entry->phone || Entry->fax || Entry->cell_phone || Entry->web_page);
	bool	bWorkFieldsUsed		= (Entry->company || Entry->wrk_street_addr || Entry->wrk_city || Entry->wrk_state || Entry->wrk_zip || 
								  Entry->wrk_country || Entry->wrk_title || Entry->wrk_dept || Entry->wrk_office || Entry->wrk_phone || 
								  Entry->wrk_fax || Entry->wrk_pgr || Entry->wrk_ip_phone || Entry->wrk_web_page);
	bool	bPersonalFieldsUsed	= (Entry->spouse || Entry->children || Entry->gender || Entry->birthday || Entry->anniversary);
	bool	bConfFieldsUsed		= (Entry->conf_server || Entry->def_conf_add || Entry->conf_addresses);

	// If there aren't any interesting fields then just skip this whole thing, we don't want an empty notes field.
	if (!bNotesFieldUsed && !bEmailFieldUsed && !bHomeFieldsUsed && !bWorkFieldsUsed && !bPersonalFieldsUsed && !bConfFieldsUsed)
	{
		*line = NULL;
		return true;
	}


	temp = "note " + nick + " ";

	if (Entry->notes)
	{
		temp += stripLines(Entry->notes);
		temp += static_cast<char>(3);
	}


	// Name fields
	if (Entry->first_name || Entry->middle_name || Entry->title)
	{
		temp += "<first:";
		if (Entry->first_name)
		{
			temp += stripLines(Entry->first_name);
		}
		if (Entry->middle_name)
		{
			temp += " " + stripLines(Entry->middle_name);
		}
		if (Entry->title)
		{
			temp += " " + stripLines(Entry->title);
		}
		temp += ">";
	}
	if (Entry->last_name)
	{
		temp += "<last:" + stripLines(Entry->last_name) + ">";
	}

	// Name fields
	if (Entry->first_name || Entry->middle_name || Entry->title || Entry->last_name)
	{
		temp += "<name:";
		if (Entry->first_name)
		{
			temp += stripLines(Entry->first_name);
		}
		if (Entry->middle_name)
		{
			temp += " " + stripLines(Entry->middle_name);
		}
		if (Entry->last_name)
		{
			temp += " " + stripLines(Entry->last_name);
		}
		if (Entry->title)
		{
			temp += " " + stripLines(Entry->title);
		}
		temp += ">";
	}

	// Home info fields
	if (Entry->phone)
	{
		temp += "<phone:" + stripLines(Entry->phone) + ">";
	}
	if (Entry->street_addr)
	{
		temp += "<address:" + stripLines(Entry->street_addr) + ">";
	}
	if (Entry->fax)
	{
		temp += "<fax:" + stripLines(Entry->fax) + ">";
	}


	// Are there any Email addresses?  If not, lets skip the "Email addresses:" section.
	if (bEmailFieldUsed)
	{
		temp += "<otheremail:" + stripLines(Entry->addresses) + ">";
	}

	// Since there is no sense of primary in WABs, assume home is primary.
	temp += "<primary:home>";

	// Are there any Home fields? If not, lets skip the "Home:" section.
	if (bHomeFieldsUsed)
	{
		if (Entry->street_addr)
		{
			temp += "<address:" + stripLines(Entry->street_addr) + ">";
		}
		if (Entry->city)
		{
			temp += "<city:" + stripLines(Entry->city) + ">";
		}
		if (Entry->state)
		{
			temp += "<state:" + stripLines(Entry->state) + ">";
		}
		if (Entry->zip)
		{
			temp += "<zip:" + stripLines(Entry->zip) + ">";
		}
		if (Entry->country)
		{
			temp += "<country:" + stripLines(Entry->country) + ">";
		}
		if (Entry->phone)
		{
			temp += "<phone:" + stripLines(Entry->phone) + ">";
		}
		if (Entry->fax)
		{
			temp += "<fax:" + stripLines(Entry->fax) + ">";
		}
		if (Entry->cell_phone)
		{
			temp += "<mobile:" + stripLines(Entry->cell_phone) + ">";
		}
		if (Entry->web_page)
		{
			temp += "<web:" + stripLines(Entry->web_page) + ">";
		}
	}

	// Are there any Work fields? If not, lets skip the "Work:" section.
	if (bWorkFieldsUsed)
	{
		// These items have corresponding fields in Eudora's address book:
		if (Entry->company)
		{
			temp += "<company:" + stripLines(Entry->company) + ">";
		}
		if (Entry->wrk_title)
		{
			temp += "<title:" + stripLines(Entry->wrk_title) + ">";
		}
		if (Entry->wrk_street_addr)
		{
			temp += "<address2:" + stripLines(Entry->wrk_street_addr) + ">";
		}
		if (Entry->wrk_city)
		{
			temp += "<city2:" + stripLines(Entry->wrk_city) + ">";
		}
		if (Entry->wrk_state)
		{
			temp += "<state2:" + stripLines(Entry->wrk_state) + ">";
		}
		if (Entry->wrk_zip)
		{
			temp += "<zip2:" + stripLines(Entry->wrk_zip) + ">";
		}
		if (Entry->wrk_country)
		{
			temp += "<country2:" + stripLines(Entry->wrk_country) + ">";
		}
		if (Entry->wrk_phone)
		{
			temp += "<phone2:" + stripLines(Entry->wrk_phone) + ">";
		}
		if (Entry->wrk_fax)
		{
			temp += "<fax2:" + stripLines(Entry->wrk_fax) + ">";
		}
		if (Entry->wrk_pgr)
		{
			temp += "<mobile2:" + stripLines(Entry->wrk_pgr) + ">";
		}
		if (Entry->wrk_web_page)
		{
			temp += "<web2:" + stripLines(Entry->wrk_web_page) + ">";
		}
		// These items get collapsed into the Notes field:
		if (Entry->wrk_office)
		{
			temp += "[Work Office] " + stripLines(Entry->wrk_office);
			temp += static_cast<char>(3);
		}
		else if (Entry->wrk_dept)
		{
			temp += "[Work Department] " + stripLines(Entry->wrk_dept);
			temp += static_cast<char>(3);
		}
		temp += static_cast<char>(3);
	}

	// Are there any Personal fields? If not, lets skip the "Personal:" section.
	if (bPersonalFieldsUsed)
	{
		// These items get collapsed into the Notes field:
		if (Entry->gender)
		{
			temp += "[Gender] " + stripLines(Entry->gender);
			temp += static_cast<char>(3);
		}
		// These items are ignored:
		if ((Entry->spouse) || (Entry->children) || (Entry->birthday) || (Entry->anniversary))
		{
		}
	}


	// Are there any Conferencing fields? If not, lets skip the "Conferencing:" section.
	if (bHomeFieldsUsed)
	{
		// These items are ignored:
		if ((Entry->conf_server) || (Entry->conf_addresses))
		{
		}
	}


	*line = DEBUG_NEW char[temp.GetLength()+1];
	strcpy(*line, temp);

	return true;
}

bool AbookCreator::AddEntry(AbookEntry *Entry)
{
	char *aliasLine = NULL;	
	char *notesLine = NULL;	

	//CODE REVIEW
	//loading from the .rc file
	char alias[256];
	QCLoadString( IDS_ADDRESSBOOK_ALIAS, alias, sizeof( alias ) );


	if(!Entry->nickname)
		return false;

	if (Entry->default_add)
	{
		//aliasLine = DEBUG_NEW char[strlen(Entry->default_add) + strlen(_T("alias ")) + strlen(Entry->nickname) + 2];
		aliasLine = DEBUG_NEW char[strlen(Entry->default_add) + strlen(_T(alias)) + strlen(Entry->nickname) + 2];
		sprintf(aliasLine, "alias %s %s", Entry->nickname, Entry->default_add);
	}
	else
	{
		//aliasLine = DEBUG_NEW char[strlen(_T("alias ")) + strlen(Entry->nickname) + 2];
		aliasLine = DEBUG_NEW char[strlen(_T(alias)) + strlen(Entry->nickname) + 2];
		sprintf(aliasLine, "alias %s", Entry->nickname);
	}

	m_NNFile->PutLine(aliasLine);
	delete [] aliasLine;


	if (!FormNotes(Entry, Entry->nickname, &notesLine))
		return false;

	m_NNFile->PutLine(notesLine);
	delete [] notesLine;
	return true;
}

bool AbookCreator::AddEntry(CAbookDistListTree *Entry)
{
	char *aliasLine = NULL;	
	char *notesLine = NULL;	
	//CODE REVIEW
	//loading from the .rc file
	char alias[256];
	QCLoadString( IDS_ADDRESSBOOK_ALIAS, alias, sizeof( alias ) );

	if(!Entry->pNickname)
		return false;
	
	CString dist_list;
	CAbookDistListEntry *pDistList = Entry->pFirstDLChild;

	while(pDistList) {
		AddressType entryType = pDistList->entryType;

		if(entryType == MESSAGING_USER) {
			CAbookEntry *ABEntry = (CAbookEntry *)pDistList->pABNode;
			
			if(dist_list.IsEmpty())
				dist_list = ABEntry->nickname;
			else {
				dist_list += ",";
				dist_list += ABEntry->nickname;
			}

		} else if(entryType == DIST_LIST) {

			CAbookDistListTree *pDLEntry = (CAbookDistListTree *)pDistList->pABNode;
			
			if(dist_list.IsEmpty())
				dist_list = pDLEntry->pNickname;
			else {
				dist_list += ",";
				dist_list += pDLEntry->pNickname;
			}
		}
		
		pDistList = pDistList->pNext;
	}

	if (!dist_list.IsEmpty())
	{
		//aliasLine = DEBUG_NEW char[dist_list.GetLength() + strlen(_T("alias ")) + strlen(Entry->pNickname) + 2];
		aliasLine = DEBUG_NEW char[dist_list.GetLength() + strlen(_T(alias)) + strlen(Entry->pNickname) + 2];
		sprintf(aliasLine, "alias %s %s", Entry->pNickname, dist_list);
	}
	else
	{
		//aliasLine = DEBUG_NEW char[strlen(_T("alias ")) + strlen(Entry->pNickname) + 2];
		aliasLine = DEBUG_NEW char[strlen(_T(alias)) + strlen(Entry->pNickname) + 2];
		sprintf(aliasLine, "alias %s", Entry->pNickname);
	}

	m_NNFile->PutLine(aliasLine);
	delete [] aliasLine;

	m_NNFile->PutLine(notesLine);
	delete [] notesLine;
	return true;
}

bool AbookCreator::OpenNNFile(char *filename)
{
	HRESULT hresult;
	if (!m_NNFile)
		m_NNFile = DEBUG_NEW JJFile;

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
			temp = DEBUG_NEW char[ strlen(*name) + 5];
			strcpy(temp, *name);
			strcat(temp, _T(".txt"));
		}
		else	// There's an extension, just the wrong one.
		{
			*floater2 = 0;
			temp = DEBUG_NEW char[strlen(*name) + 5];
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
			newname = DEBUG_NEW char [strlen(*name)+3];	// -1\0
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

			num = DEBUG_NEW char[strlen(_T(".txt")) + firstlen+2];
			sprintf(num, "%d.txt", version);
			lastlen = strlen(num);
			if (lastlen > firstlen)
			{
				char * newname;
				newname = DEBUG_NEW char[strlen(*name)+(lastlen-firstlen)+1];
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
	*fullpath = DEBUG_NEW char[strlen(EudoraDir) + strlen(CRString(IDS_NICK_DIR_NAME)) + strlen(*name) + 2]; // one for the slash one for the null
	sprintf(*fullpath, "%s%s\\%s", EudoraDir, CRString(IDS_NICK_DIR_NAME), *name);

	return true;
}


// This figures out what the nickname for distribution list should be

bool AbookCreator::FormNickname(CAbookDistListTree *Entry, char **Nickname)
{
	char *floater;

	if (Entry->pNickname)
	{
		*Nickname = DEBUG_NEW char[strlen(Entry->pNickname) + 2];
		strcpy(*Nickname, Entry->pNickname);
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


// This uses a complex algorithm to figure out what the nickname for this entry should be. 
// Ok, not really.

bool AbookCreator::FormNickname(CAbookEntry *Entry, char **Nickname)
{
	char *floater;
	char *floater2;
	// DRW - For some reason the previous code never tried to use the
	// nickname field as the actual nickname.
	if (Entry->nickname)
	{
		*Nickname = DEBUG_NEW char[strlen(Entry->nickname) + 2];
		strcpy(*Nickname, Entry->nickname);
	}
	else if (Entry->first_name && Entry->last_name)
	{
		*Nickname = DEBUG_NEW char[strlen(Entry->first_name) + strlen(Entry->last_name) + 2];
		sprintf(*Nickname, "%s_%s", Entry->first_name, Entry->last_name);
	}
	else if (!Entry->first_name && Entry->last_name)
	{
		*Nickname = DEBUG_NEW char[strlen(Entry->last_name) + 1];
		strcpy(*Nickname, Entry->last_name);
	}
	else if (!Entry->last_name && Entry->first_name)
	{
		*Nickname = DEBUG_NEW char[strlen(Entry->first_name) + 1];
		strcpy(*Nickname, Entry->first_name);
	}
	else
	{
		// No names, we'll pull it from the email address
		if (!Entry->default_add)
		{
			// Lost cause, no first name, no last name and no default address...
			return false;
		}
		floater = strchr(Entry->default_add, '@');
		floater2 = strchr(Entry->default_add, ' ');
		if (!floater && !floater2)
		{
			// Oh bah!
			return false;
		}

		if ((floater2 != NULL) && floater2 < floater)
		{
			// We prefer spaces to the @ sign
			*Nickname = DEBUG_NEW char[(floater2 - (Entry->default_add))+1];
			strncpy(*Nickname, Entry->default_add, floater2 - (Entry->default_add));
			// DRW - strncpy() doesn't terminate the string!
			(*Nickname)[floater2 - (Entry->default_add)] = '\0';
		}
		else if (floater != NULL)
		{
			*Nickname = DEBUG_NEW char[(floater - (Entry->default_add))+1];
			strncpy(*Nickname, Entry->default_add, floater - (Entry->default_add));
			// DRW - strncpy() doesn't terminate the string!
			(*Nickname)[floater - (Entry->default_add)] = '\0';
		}
		else
		{
			// DRW - Total failure.  If we get here, Entry->default_add is non-NULL
			// but does not appear to contain an e-mail address.  This is probably
			// an indication that the import didn't really work, but if we don't do
			// something here the import will crash.
			*Nickname = DEBUG_NEW char[strlen(Entry->default_add) + 1];
			strncpy(*Nickname, Entry->default_add, strlen(Entry->default_add));
			// DRW - strncpy() doesn't terminate the string!
			(*Nickname)[strlen(Entry->default_add)] = '\0';
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

ImportChild::ImportChild()
{
	LMOS = false;
	IsIMAP = false;
	hasMail = false;
	hasAddresses = false;
	hasLdif = false;
	memset(m_szIdentityName, 0, _MAX_PATH);
}

ImportProvider::ImportProvider()
{
	hasMail = false;
	hasAddresses = false;
	hasLdif = false;
}

CImportMail::CImportMail()
{
	m_newhead = NULL;
	m_pFolderList = NULL;
	m_pAddrEntry = NULL;
	m_InstalledStatus = CMapiInstaller::STATUS_NOT_INSTALLED;
	m_OutFilePathname = NULL;
	m_iDllStructSize = 0;
	m_psDllStruct = NULL;
	strcpy(m_szEudoraImportSearch, ExecutableDir);
	strcat(m_szEudoraImportSearch, "*.eif");
}

bool CImportMail::InitPlugins()
{
	if(!InitPluginList())
		return false;
	else
		return true;
	
}

bool CImportMail::InitProviders(bool bDisplayErrDialog /* = false */)
{
	FindProviders(bDisplayErrDialog);

	if(m_newhead)
		return true;
	else
		return false;
}

CImportMail::~CImportMail()
{
	CImportProvider *pTmp = m_newhead;
	CImportProvider *pWalk;

	if(pTmp)
	{
		do
		{
			pWalk = pTmp;
			pTmp = pTmp->pNext;
			delete pWalk;

		} while(pTmp);
	}

	delete [] m_psDllStruct;
}

bool CImportMail::DeleteAddrBook(AbookEntry **Tree)
{

	if (Tree == NULL)
		Tree = &m_pAddrEntry;

	if (*Tree == NULL)
		return true;
	
	
	AbookEntry *trash = *Tree;
	if(trash->next)
		DeleteAddrBook(&(trash->next));


	delete *Tree;
	*Tree = NULL;
	
	return true;

}

bool CImportMail::DeleteAddrTree(CAddressBookTree **Tree) {
	
	if (*Tree == NULL)
		return true;
	
	
	CAddressBookTree *trash = *Tree;
	if(trash->pNext)
		DeleteAddrTree(&(trash->pNext));


	delete *Tree;
	*Tree = NULL;
	
	return true;

}

void CImportMail::DeleteFolderNodeData(Mbox_FnamePair *pFolder)
{
	if(!pFolder)
		return;

	if(pFolder->DisplayName)
		delete [] pFolder->DisplayName;
	
	if(pFolder->Filename)
		delete [] pFolder->Filename;

	pFolder->pMailbox = NULL;

	return;
}
bool CImportMail::InitPluginList()
{
	CFileFind	finder;
	BOOL		bWorking = TRUE;
	UINT		iMatchingFiles = 0;

	bWorking = finder.FindFile(m_szEudoraImportSearch);

	if(!bWorking)
		return false;

	while(bWorking)
	{
		bWorking = finder.FindNextFile();
		iMatchingFiles++;
	}
	m_psDllStruct = DEBUG_NEW ImportDllStruct[iMatchingFiles];

	m_iDllStructSize = iMatchingFiles;
	iMatchingFiles = 0;
	bWorking = finder.FindFile(m_szEudoraImportSearch);
	
	while(bWorking)
	{
		bWorking = finder.FindNextFile();
		m_psDllStruct[iMatchingFiles].szDllPath = finder.GetFilePath();
		InitDllStruct(iMatchingFiles);
		iMatchingFiles++;
	}

	
	return true;

}
bool CImportMail::InitDllStruct(UINT iStructNum)
{
	m_psDllStruct[iStructNum].hDll = ::LoadLibrary(m_psDllStruct[iStructNum].szDllPath);

	if(!m_psDllStruct[iStructNum].hDll)
	{
		m_psDllStruct[iStructNum].pQueryFunction = NULL;
		return false;
	}

	m_psDllStruct[iStructNum].pQueryFunction = (QUERYIMPORTPROVIDER*) ::GetProcAddress(m_psDllStruct[iStructNum].hDll, "QueryImportProvider");

	if(!m_psDllStruct[iStructNum].pQueryFunction)
	{
		::FreeLibrary(m_psDllStruct[iStructNum].hDll);
		return false;
	}

	m_psDllStruct[iStructNum].pFindFunction = (LOADPROVIDER*) ::GetProcAddress(m_psDllStruct[iStructNum].hDll, "LoadProvider");
	m_psDllStruct[iStructNum].pLoadFoldersFunc = (LOADFOLDERS*) ::GetProcAddress(m_psDllStruct[iStructNum].hDll, "LoadFolders");
	m_psDllStruct[iStructNum].pMakeMimeFunc = (MAKEMIMEDIGEST*) ::GetProcAddress(m_psDllStruct[iStructNum].hDll, "MakeMimeDigest");
	m_psDllStruct[iStructNum].pAddrFunc = (GETADDRESSTREE*) ::GetProcAddress(m_psDllStruct[iStructNum].hDll, "GetAddressTree");
	m_psDllStruct[iStructNum].pPersonalityFunc = (GETPERSONALITYTREE*) ::GetProcAddress(m_psDllStruct[iStructNum].hDll, "GetPersonalityTree");
	m_psDllStruct[iStructNum].szProviderName = m_psDllStruct[iStructNum].pQueryFunction();
	m_psDllStruct[iStructNum].pAddrExtension = (GETADDREXTENSION*) ::GetProcAddress(m_psDllStruct[iStructNum].hDll, "GetAddrExtension");
	m_psDllStruct[iStructNum].pAddrExtensionFilter = (GETADDREXTENSIONFILTER*) ::GetProcAddress(m_psDllStruct[iStructNum].hDll, "GetAddrExtensionFilter");
	m_psDllStruct[iStructNum].pMailExtension = (GETMAILEXTENSION*) ::GetProcAddress(m_psDllStruct[iStructNum].hDll, "GetMailExtension");
	m_psDllStruct[iStructNum].pMailExtensionFilter = (GETMAILEXTENSIONFILTER*) ::GetProcAddress(m_psDllStruct[iStructNum].hDll, "GetMailExtensionFilter");

	return true;
}

void CImportMail::FindProviders(bool bDisplayErrDialog)
{
	
	CImportProvider *pProvider = NULL;
	CImportProvider *pCurrentProvider = NULL;

	for(int i = 0; i < m_iDllStructSize; i++)
	{
		if(m_psDllStruct[i].pQueryFunction != NULL)
		{

			// uninstall our mapi
			m_InstalledStatus = CMapiInstaller::Uninstall();

			pProvider = (CImportProvider * ) m_psDllStruct[i].pFindFunction(bDisplayErrDialog);

			// install it back
			if (m_InstalledStatus != CMapiInstaller::STATUS_NOT_INSTALLED)
				CMapiInstaller::Install();

			if(pProvider == NULL)
				continue;

			pCurrentProvider = m_newhead;
		
			if (pCurrentProvider == NULL)
			{
				m_newhead = DEBUG_NEW CImportProvider;
				pCurrentProvider = m_newhead;
				pCurrentProvider->pNext = NULL;
			}
			else
			{
				while (pCurrentProvider->pNext != NULL)
				{
					pCurrentProvider = pCurrentProvider->pNext;
				}

				pCurrentProvider->pNext = DEBUG_NEW CImportProvider;
				pCurrentProvider = pCurrentProvider->pNext;
				pCurrentProvider->pNext = NULL;
			}

			pCurrentProvider->bHasAddresses = pProvider->bHasAddresses;
			pCurrentProvider->bHasLdif = pProvider->bHasLdif;
			pCurrentProvider->bHasMail = pProvider->bHasMail;
			pCurrentProvider->iNumChildren = pProvider->iNumChildren;
			pCurrentProvider->lpszDisplayName = pProvider->lpszDisplayName;
			pCurrentProvider->pFirstChild = pProvider->pFirstChild;
		}
	}

}

bool CImportMail::ImportMail2(CImportChild *Child, CWnd *pParentWnd)
{
	for(int i = 0; i < m_iDllStructSize; i++)
	{
		if(strcmp(m_psDllStruct[i].szProviderName, Child->pParent->lpszDisplayName) == 0)
		{
			m_ImporterToUse.hDll = m_psDllStruct[i].hDll;
			m_ImporterToUse.pQueryFunction = m_psDllStruct[i].pQueryFunction; 
			m_ImporterToUse.pFindFunction = m_psDllStruct[i].pFindFunction;
			m_ImporterToUse.pLoadFoldersFunc = m_psDllStruct[i].pLoadFoldersFunc;
			m_ImporterToUse.pMakeMimeFunc = m_psDllStruct[i].pMakeMimeFunc;
			m_ImporterToUse.pAddrFunc = m_psDllStruct[i].pAddrFunc;
			m_ImporterToUse.pPersonalityFunc = m_psDllStruct[i].pPersonalityFunc;
			m_ImporterToUse.szDllPath = m_psDllStruct[i].szDllPath;
			m_ImporterToUse.szProviderName = m_psDllStruct[i].szProviderName;
			m_ImporterToUse.pAddrExtension = m_psDllStruct[i].pAddrExtension;
			m_ImporterToUse.pAddrExtensionFilter = m_psDllStruct[i].pAddrExtensionFilter;
			m_ImporterToUse.pMailExtension = m_psDllStruct[i].pMailExtension;
			m_ImporterToUse.pMailExtensionFilter = m_psDllStruct[i].pMailExtensionFilter;

		}
		else
		{
			m_psDllStruct[i].pQueryFunction = NULL;
			if(m_psDllStruct[i].hDll)
				::FreeLibrary(m_psDllStruct[i].hDll);
		}

	}


	char szImportFile[_MAX_PATH];
	CMessageStore * pMsgStores = NULL;
	Mbox_FnamePair * pFolderIterator = NULL;


	// uninstall our mapi
	m_InstalledStatus = CMapiInstaller::Uninstall();

	pMsgStores = (CMessageStore *) m_ImporterToUse.pLoadFoldersFunc(Child->lpszPathToMailData, Child->lpszIdentityName);

	if(pMsgStores == NULL)
		return false;

	CString str;
	str.Format(IDS_CONVERTING_FROM, m_ImporterToUse.szProviderName);
	::MainProgress(str);

	// first time import is directly into Eudora In, Out and Trash mailboxes. Future imports are
	// into a folder "Microsoft Outlook - <Message Store Name>"
	// This check determines where we import the mailboxes into, it is a first time import or not
	// We check to see if In, Out and Junk are empty and decide that it is a first time import
	QCMailboxCommand * pInMBoxCmd = g_theMailboxDirector.FindByName( NULL, CRString( IDS_IN_MBOX_NAME ) );
	QCMailboxCommand * pOutMBoxCmd = g_theMailboxDirector.FindByName( NULL, CRString( IDS_OUT_MBOX_NAME ) );
	QCMailboxCommand * pJunkMBoxCmd = g_theMailboxDirector.FindByName( NULL, CRString( IDS_JUNK_MBOX_NAME ) );

	//CODE REVIEW
	//added check if one of the mailboxes were not found
	bool isFirstImport = false;
	if(pInMBoxCmd && pOutMBoxCmd && pJunkMBoxCmd)
		isFirstImport = (pInMBoxCmd->IsEmpty() && pOutMBoxCmd->IsEmpty() && pJunkMBoxCmd->IsEmpty());
	

	while(pMsgStores) {				// loop through the list of message stores

		bool isDefaultMsgStore = pMsgStores->bIsDefault;

		if(pMsgStores->lpMBoxID) {
			
			BuildInternalFolderStruct(pMsgStores->lpMBoxID);
	
	
			if(m_pFolderList) {

				pFolderIterator = m_pFolderList;
			
				if(isFirstImport && isDefaultMsgStore) {	// first time import into clean Eudora, we want the mailboxes directly under
															// Eudora, we set the root to be NULL
					m_pRootCommand = NULL;

				} else {
					char szMsgStoreName[_MAX_PATH];
					strcpy(szMsgStoreName, Child->pParent->lpszDisplayName);
					if(pMsgStores->lpszDisplayName) {							// subsequent imports go under 
						strcat(szMsgStoreName, "-");							// "Name of Provider-Message Store name"
						strcat(szMsgStoreName, pMsgStores->lpszDisplayName);	// If no message store name like Netscape and OE
					}															// just use only provider name
			
					m_pRootCommand = CreateMbox(szMsgStoreName, NULL, MBT_FOLDER);
					if (!m_pRootCommand)
						return false;
				}

			}

			BuildEudoraFolders(isDefaultMsgStore, isFirstImport);
	
			while(pFolderIterator) {	// loop through the folders in the message store

				strcpy(szImportFile, Child->lpszPathToMailData);
				strcat(szImportFile, pFolderIterator->Filename);
				Convert2(Child, pFolderIterator, pMsgStores->pDB );
				pFolderIterator = pFolderIterator->next;
			}	// end of loop through folders

		}

		DeleteInternalFolderList();	// cleans up the folder list bcos once it is created in Eudora, we reuse the same to hold the folders of the next message store
		m_pFolderList = NULL;
		pFolderIterator = NULL;
		pMsgStores = pMsgStores->pNext;

	} // end of message stores loop
	
	::CloseProgress();
	
	// install it back
	if (m_InstalledStatus != CMapiInstaller::STATUS_NOT_INSTALLED)
		CMapiInstaller::Install();

	return true;
}

void CImportMail::DeleteInternalFolderList() 
{
	Mbox_FnamePair *pWalker = m_pFolderList;
	Mbox_FnamePair *pDelete = NULL;

	if(pWalker)
	{
		do
		{
			DeleteFolderNodeData(pWalker);
			pDelete = pWalker;
			pWalker = pWalker->next;
			delete pDelete;
		
		}while(pWalker);
	}
}

// we need this bcos when we import into Eudora In, Out, Trash a TOC already exists
// and we want it to rebuild the TOC after the import into the mailboxes
void CImportMail::DeleteTOCFile(QCMailboxCommand * pMBoxCmd) 
{
	char *pathToMBX;
	pathToMBX = DEBUG_NEW char[((pMBoxCmd->GetPathname()).GetLength())+1];
	strcpy(pathToMBX, pMBoxCmd->GetPathname());
	KillTheTOCFile(pathToMBX);
	delete [] m_OutFilePathname;
	m_OutFilePathname = DEBUG_NEW char[strlen(pathToMBX)+1];
	strcpy(m_OutFilePathname, pathToMBX);
	delete [] pathToMBX;
}


bool CImportMail::BuildEudoraFolders(bool isDefaultMsgStore, bool isFirstImport)
{
	Mbox_FnamePair * mbfIterator = m_pFolderList;
	QCMailboxCommand * pqcmbParentMbox;
	
	do // while all folders aren't created
	{
		// If we've gone through the list start over
		// since parent was probably created
		if(mbfIterator == NULL)
			mbfIterator = m_pFolderList;

		// If it is created skip to next. Happens when looking for stray children
		if( ! (mbfIterator->bCreated))
		{
			
			if (isDefaultMsgStore && isFirstImport) 
			{
				if(mbfIterator->folderType == IN_MBOX) {
		
					mbfIterator->pParentMbox = m_pRootCommand; 
					mbfIterator->pMailbox = g_theMailboxDirector.FindByName( NULL, CRString( IDS_IN_MBOX_NAME ) );
					mbfIterator->bCreated = true;
					DeleteTOCFile(mbfIterator->pMailbox);
					continue;

				} else if(mbfIterator->folderType == OUT_MBOX) {

					mbfIterator->pParentMbox = m_pRootCommand;
					mbfIterator->pMailbox = g_theMailboxDirector.FindByName( NULL, CRString( IDS_OUT_MBOX_NAME ) );
					mbfIterator->bCreated = true;
					DeleteTOCFile(mbfIterator->pMailbox);
					continue;

				} else if(mbfIterator->folderType == TRASH_MBOX) {

					mbfIterator->pParentMbox = m_pRootCommand;
					mbfIterator->pMailbox = g_theMailboxDirector.FindByName( NULL, CRString( IDS_TRASH_MBOX_NAME ) );
					mbfIterator->bCreated = true;
					DeleteTOCFile(mbfIterator->pMailbox);
					continue;
				} 
			}
			
			if (mbfIterator &&
				(mbfIterator->lParentMBoxID != 0) &&		// Parent ID of 0 indicates no parent specified
				(mbfIterator->lParentMBoxID != 1))		// Parent ID of 1 indicates parent is root
			{
				// If we haven't set the parent mailbox
				if(mbfIterator->pParentMbox == NULL)
				{
					// Get our parent
					pqcmbParentMbox = GetParentMbox(mbfIterator);
					// If the parent isn't created
					if(pqcmbParentMbox == NULL)
					{
						// skip
						mbfIterator = mbfIterator->next;
						continue;
					}
					else
					{
						// If it isnt a folder get its parent folder
						if(pqcmbParentMbox->GetType() != MBT_FOLDER)
						{
							mbfIterator->pParentMbox = g_theMailboxDirector.FindParent(pqcmbParentMbox);
						}
						// If it has subfolders we will put it in its own folder of same nam
						if(mbfIterator->bHasSubfolders)
						{
							mbfIterator->pParentMbox = CreateMbox(mbfIterator->DisplayName, mbfIterator->pParentMbox, MBT_FOLDER);
						}
						// Create it
						mbfIterator->pMailbox = CreateMbox(mbfIterator->DisplayName, mbfIterator->pParentMbox, MBT_REGULAR);
						mbfIterator->bCreated = true;
						
					}
				}
				else 
				{
					// If it isnt a folder get its parent folder
					if(mbfIterator->pParentMbox->GetType() != MBT_FOLDER)
					{
						mbfIterator->pParentMbox = g_theMailboxDirector.FindParent(mbfIterator->pParentMbox);
					}

					if(mbfIterator->bHasSubfolders)
					{
						mbfIterator->pParentMbox = CreateMbox(mbfIterator->DisplayName, mbfIterator->pParentMbox, MBT_FOLDER);
					}

					mbfIterator->pMailbox = CreateMbox(mbfIterator->DisplayName, mbfIterator->pParentMbox, MBT_REGULAR);
					mbfIterator->bCreated = true;
				}


			}
			else // Just a root or parentless
			{
				if(mbfIterator->bHasSubfolders)
				{
					mbfIterator->pParentMbox = CreateMbox(mbfIterator->DisplayName, m_pRootCommand, MBT_FOLDER);
				}
				else
					mbfIterator->pParentMbox = m_pRootCommand;

				mbfIterator->pMailbox = CreateMbox(mbfIterator->DisplayName, mbfIterator->pParentMbox, MBT_REGULAR);
				mbfIterator->bCreated = true;
				

			}
		}

		mbfIterator = mbfIterator->next;
	}while(!AllFoldersCreated());


	return true;

}
// Return the OE specified parent mailbox for the mailbox currently being imported.
// If we are imported OE4 always return m_pRootCommand so the behavior doesn't change.
QCMailboxCommand *CImportMail::GetParentMbox(Mbox_FnamePair *pFolders)
{
	QCMailboxCommand	*pqcmbParent = m_pRootCommand;

	
	if (pFolders &&
		(pFolders->lParentMBoxID != 0) &&		// Parent ID of 0 indicates no parent specified
		(pFolders->lParentMBoxID != 1))		// Parent ID of 1 indicates parent is root
	{
		Mbox_FnamePair *		pmbfp = m_pFolderList;
		bool					bMatchFound = false;
		while (pmbfp && !bMatchFound)
		{
			// If this is the parent
			if (pmbfp->lMBoxID == pFolders->lParentMBoxID)
			{
				pqcmbParent = pmbfp->pMailbox;
				bMatchFound = true;

				// We haven't created parent yet
				if (pqcmbParent == NULL)
				{
					return NULL;
				}
			}
			else // if not parent
			{
				pmbfp = pmbfp->next;
			}
		}
	}
	return pqcmbParent;
}
bool CImportMail::AllFoldersCreated()
{
	Mbox_FnamePair * mbfIterator = m_pFolderList;
	
	while(mbfIterator)
	{
		if( ! (mbfIterator->bCreated))
			return false;

		mbfIterator = mbfIterator->next;
	}

	return true;

}
QCMailboxCommand * CImportMail::CreateMbox(char *name, QCMailboxCommand *ParentFolderCmd, MailboxType Type)
{
	QCMailboxCommand * NewFolderCmd = NULL;
	char * Filename = NULL;
	// DRW 9/21/99 - Made Type a parameter.
	// MailboxType Type;

	if (!name)
	{
		Filename = DEBUG_NEW char[21];
		strcpy(Filename, OE_DISPLAY_NAME);
		// DRW 9/21/99 - Made Type a parameter.
		// Type = MBT_FOLDER;
	}
	else
	{
		Filename = DEBUG_NEW char[strlen(name)+1];
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
		pathToMBX = DEBUG_NEW char[((NewFolderCmd->GetPathname()).GetLength())+1];
		strcpy(pathToMBX, NewFolderCmd->GetPathname());
		KillTheTOCFile(pathToMBX);
		delete [] m_OutFilePathname;
		m_OutFilePathname = DEBUG_NEW char[strlen(pathToMBX)+1];
		strcpy(m_OutFilePathname, pathToMBX);
		delete [] pathToMBX;
	}

	return NewFolderCmd;
}

void CImportMail::BuildInternalPersonaStruct(CPersParams *pPersona, CPersonalityTree *pPersonEntryList) {
	pPersona->bAPop = pPersonEntryList->bAPop;
	pPersona->bCheckMail = pPersonEntryList->bCheckMail;
	pPersona->bDelServerAfter = pPersonEntryList->bDelServerAfter;
	pPersona->bDelWhenTrashed = pPersonEntryList->bDelWhenTrashed;
	pPersona->BigMsgThreshold = pPersonEntryList->pBigMsgThreshold;
	pPersona->bIMAP = pPersonEntryList->bIMAP;
	pPersona->bIMAPFullDwnld = pPersonEntryList->bIMAPFullDwnld;
	pPersona->bIMAPMarkDeleted = pPersonEntryList->bIMAPMarkDeleted;
	pPersona->bIMAPMinDwnld = pPersonEntryList->bIMAPMinDwnld;
	pPersona->bIMAPXferToTrash = pPersonEntryList->bIMAPXferToTrash;
	pPersona->bKerberos = pPersonEntryList->bKerberos;
	pPersona->bLMOS = pPersonEntryList->bLMOS;
	pPersona->bPassword = pPersonEntryList->bPassword;
	pPersona->bPOP = pPersonEntryList->bPOP;
	pPersona->bRPA = pPersonEntryList->bRPA;
	pPersona->bSkipBigMsgs = pPersonEntryList->bSkipBigMsgs;
	pPersona->bSMTPAuthAllowed = pPersonEntryList->bSMTPAuthAllowed;
	pPersona->bUseSMTPRelay = pPersonEntryList->bUseSMTPRelay;
	pPersona->bWinSock = pPersonEntryList->bWinSock;
	pPersona->DefaultDomain = pPersonEntryList->pDefaultDomain;
	pPersona->IMAPMaxSize = pPersonEntryList->pIMAPMaxSize;
	pPersona->IMAPPrefix = pPersonEntryList->pIMAPPrefix;
	pPersona->IMAPTrashMailbox = pPersonEntryList->pIMAPTrashMailbox;
	pPersona->InServer = pPersonEntryList->pInServer;
	pPersona->LeaveOnServDays = pPersonEntryList->pLeaveOnServDays;
	pPersona->LoginName = pPersonEntryList->pLoginName;
	pPersona->m_SSLReceiveUsage = pPersonEntryList->iSSLReceiveUsage;
	pPersona->m_SSLSendUsage = pPersonEntryList->iSSLSendUsage;
	pPersona->OutServer = pPersonEntryList->pOutServer;
	pPersona->PersName = pPersonEntryList->pPersName;
	pPersona->POPAccount = pPersonEntryList->pPOPAccount;
	pPersona->RealName = pPersonEntryList->pRealName;
	pPersona->ReturnAddress = pPersonEntryList->pReturnAddress;
	pPersona->Signature = pPersonEntryList->pSignature;
	pPersona->Stationery = pPersonEntryList->pStationery;
	

}

void CImportMail::BuildInternalFolderStruct(CMbox_FnamePair *pFolder)
{
	CMbox_FnamePair * fpTmp;
	m_pFolderList = DEBUG_NEW Mbox_FnamePair;
	Mbox_FnamePair * pfpTmpNew;

	pfpTmpNew = m_pFolderList;

	fpTmp = pFolder;

	while(fpTmp)
	{
		m_pFolderList->bCreated = fpTmp->bCreated;
		m_pFolderList->bHasSubfolders = fpTmp->bHasSubfolders;
	
		m_pFolderList->DisplayName = DEBUG_NEW char [strlen(fpTmp->lpszDisplayName)+1];
		strcpy(	m_pFolderList->DisplayName, fpTmp->lpszDisplayName);

		m_pFolderList->Filename = DEBUG_NEW char [strlen(fpTmp->lpszFilename)+1];
		strcpy(m_pFolderList->Filename, fpTmp->lpszFilename);

		m_pFolderList->lMBoxID = fpTmp->lMBoxID;
		m_pFolderList->lParentMBoxID = fpTmp->lParentMBoxID;
		m_pFolderList->pMailbox = NULL;
		m_pFolderList->pParentMbox = NULL;
		m_pFolderList->pID = fpTmp->pID;
		m_pFolderList->iSize = fpTmp->iSize;
		m_pFolderList->folderType = fpTmp->folderType;
	
		fpTmp = fpTmp->pNext;
		
		if(fpTmp)
		{
			m_pFolderList->next = DEBUG_NEW Mbox_FnamePair;
			m_pFolderList = m_pFolderList->next;
			m_pFolderList->next = NULL;
		}
		else
		{
			m_pFolderList->next = NULL;
		}
		
	}
	m_pFolderList->next = NULL;
	m_pFolderList = pfpTmpNew;

}
void CImportMail::BuildInternalAddrStruct(CAbookEntry * pEntryList)
{
	CAbookEntry * fpTmp;
	m_pAddrEntry = DEBUG_NEW AbookEntry;
	AbookEntry * pfpTmpNew;

	pfpTmpNew = m_pAddrEntry;

	fpTmp = pEntryList;

	while(fpTmp)
	{
		m_pAddrEntry->first_name = fpTmp->first_name ;
		m_pAddrEntry->middle_name = fpTmp->middle_name ;
		m_pAddrEntry->last_name = fpTmp->last_name ;
		m_pAddrEntry->title = fpTmp->title ;
		m_pAddrEntry->nickname = fpTmp->nickname ;
		m_pAddrEntry->default_add = fpTmp->default_add ;
		m_pAddrEntry->addresses = fpTmp->addresses ;
		m_pAddrEntry->street_addr = fpTmp->street_addr ;
		m_pAddrEntry->city = fpTmp->city ;
		m_pAddrEntry->state = fpTmp->state ;
		m_pAddrEntry->zip = fpTmp->zip ;
		m_pAddrEntry->country = fpTmp->country ;
		m_pAddrEntry->phone = fpTmp->phone ;
		m_pAddrEntry->fax = fpTmp->fax ;
		m_pAddrEntry->cell_phone = fpTmp->cell_phone ;
		m_pAddrEntry->web_page = fpTmp->web_page ;
		m_pAddrEntry->company = fpTmp->company ;
		m_pAddrEntry->wrk_street_addr = fpTmp->wrk_street_addr ;
		m_pAddrEntry->wrk_city = fpTmp->wrk_city ;
		m_pAddrEntry->wrk_state = fpTmp->wrk_state ;
		m_pAddrEntry->wrk_zip = fpTmp->wrk_zip ;
		m_pAddrEntry->wrk_country = fpTmp->wrk_country ;
		m_pAddrEntry->wrk_title = fpTmp->wrk_title ;
		m_pAddrEntry->wrk_dept = fpTmp->wrk_dept ;
		m_pAddrEntry->wrk_office = fpTmp->wrk_office ;
		m_pAddrEntry->wrk_phone = fpTmp->wrk_phone ;
		m_pAddrEntry->wrk_fax = fpTmp->wrk_fax ;
		m_pAddrEntry->wrk_pgr = fpTmp->wrk_pgr ;
		m_pAddrEntry->wrk_ip_phone = fpTmp->wrk_ip_phone ;
		m_pAddrEntry->wrk_web_page = fpTmp->wrk_web_page ;
		m_pAddrEntry->spouse = fpTmp->spouse ;
		m_pAddrEntry->children = fpTmp->children ;
		m_pAddrEntry->gender = fpTmp->gender ;
		m_pAddrEntry->birthday = fpTmp->birthday ;
		m_pAddrEntry->anniversary = fpTmp->anniversary ;
		m_pAddrEntry->notes = fpTmp->notes ;
		m_pAddrEntry->conf_server = fpTmp->conf_server ;
		m_pAddrEntry->def_conf_add = fpTmp->def_conf_add ;
		m_pAddrEntry->conf_addresses = fpTmp->conf_addresses ;
		m_pAddrEntry->type = fpTmp->type ;
		m_pAddrEntry->m_lImportID = fpTmp->m_lImportID ;

		m_pAddrEntry->next = NULL;
	
		fpTmp = fpTmp->pNext;
		
		if(fpTmp)
		{
			m_pAddrEntry->next = DEBUG_NEW AbookEntry;
			m_pAddrEntry = m_pAddrEntry->next;
			m_pAddrEntry->next = NULL;
		}
		else
		{
			m_pAddrEntry->next = NULL;
		}
		
	}
	m_pAddrEntry->next = NULL;
	m_pAddrEntry = pfpTmpNew;


}
int CImportMail::Convert2(CImportChild *Child, Mbox_FnamePair *pFolders, void *pDB)
{	
	int numofmsgs = 0;
	char szImportFile[_MAX_PATH];

	if(!m_ImporterToUse.pQueryFunction)
		return -1;


	strcpy(szImportFile, Child->lpszPathToMailData);
	strcat(szImportFile, pFolders->Filename);


//************************************
	char *lpszMimeName = NULL;

	lpszMimeName = (char *) m_ImporterToUse.pMakeMimeFunc(szImportFile, pFolders->pID, pFolders->iSize, pDB);

	if(!lpszMimeName)
	{
		return 0;
	}
	struct _stat st;

	_stat(lpszMimeName, &st);

	if( st.st_mode & _S_IFDIR)
	{
		// Do All Files and Folders
	}
	else
	if( st.st_mode & _S_IFREG)
	{
		// Do File
		ProcessMimeDigest(lpszMimeName, (char *) (LPCSTR) pFolders->pMailbox->GetPathname());
	}


	return (numofmsgs);
}

bool CImportMail::ProcessMimeDigest(char *lpszPathToMimeDigest, char* lpszPathToEudMBox)
{
	extern JJFile *g_pPOPInboxFile;
	JJFile jjInDigest, jjOutMbox;
	FileLineReader lineReader;
	extern long g_lEstimatedMsgSize;
	long lSize;
	CObArray MimeStates;

	
	if (FAILED(jjInDigest.Open(lpszPathToMimeDigest, O_RDONLY)))
		return false;

	HRESULT hrSeek = jjInDigest.Seek(0, SEEK_END, &lSize);
	
	if (FAILED(hrSeek))
		return false;
	ASSERT(lSize >= 0);
	g_lEstimatedMsgSize = lSize;
	jjInDigest.Reset();
	lineReader.SetFile(&jjInDigest);
	char buf[500];

	if (FAILED(jjOutMbox.Open(lpszPathToEudMBox, O_RDWR | O_APPEND | O_CREAT)))
	return false;

	
	// g_pPOPInboxFile is a global (defined in pop.cpp) that is used by mime.cpp, header.cpp code
	JJFile * oldPopMBox = g_pPOPInboxFile;
	g_pPOPInboxFile = &jjOutMbox;

	// Create A hd & Load it up, line reader must be set up, output g_pPOPInboxFile must be set up
	HeaderDesc hd(lSize, FALSE);

	// Create a file base line reader
	MIMEState ms(&lineReader,0);

		::mimeInit();
	while (1)
	{
		long lStartOffset = 0;
		g_pPOPInboxFile->Tell(&lStartOffset);
		if (lStartOffset < 0)
			break;

		hd.lDiskStart = lStartOffset;

		//
		// Download the header and write it to the MBX file.
		//
		int nReadStatus = hd.Read(&lineReader, false);
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
		

			//
			// Instantiate the appropriate MIME reader object
			// using info gleaned from the Content-Type: header.
			//
			if (! ms.Init(&hd,MimeStates.GetSize()) || MimeStates.Add(&ms) < 0)
				return false;

			//
			// Okay, call the polymorphic MIME reader to really 
			// download the message body...
			// Set The exploding Digest!

			((MultiReader *)ms.m_Reader)->SetExplodingDigest();

			BoundaryType endType = ms.m_Reader->ReadIt(MimeStates, buf, sizeof(buf));
			((MultiReader *)ms.m_Reader)->ResetExplodingDigest();
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
		
		long lEndOffset = 0;
		g_pPOPInboxFile->Tell(&lEndOffset);
		if (lEndOffset < 0)
			break;

		//
		// We don't use the Status: header to determine if a big
		// message has already been skipped.  That was the old way.
		// The new way is determined elsewhere with LMOS record flags.
		//
		break;
	}
	g_pPOPInboxFile = oldPopMBox;
	jjInDigest.Close();
	jjOutMbox.Flush();
	jjOutMbox.Close();

	DeleteFile(lpszPathToMimeDigest);

	return true;



}


void CImportMail::UpdateWazoo(CString pPersName)
{
	CPersonalityView *pView = CMainFrame::QCGetMainFrame()->GetActivePersonalityView() ;

	if (pView)
	{
		pView->PopulateView();
		
		//
		// Select the newly-added personality.
		//
		LV_FINDINFO lvFindInfo;
		lvFindInfo.flags = LVFI_STRING;
		lvFindInfo.psz = pPersName;
		lvFindInfo.lParam = NULL;

		CListCtrl& theCtrl = pView->GetListCtrl();
		int nIndex = theCtrl.FindItem(&lvFindInfo);
		ASSERT(nIndex != -1);
		theCtrl.SetItemState(nIndex, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}
}


bool CImportMail::ImportPersonalities2(CImportChild *Child, bool bIsProfile, CWnd * pParentWnd)
{

	for(int i = 0; i < m_iDllStructSize; i++)
	{
		if(strcmp(m_psDllStruct[i].szProviderName, Child->pParent->lpszDisplayName) == 0)
		{
			m_ImporterToUse.hDll = m_psDllStruct[i].hDll;
			m_ImporterToUse.pQueryFunction = m_psDllStruct[i].pQueryFunction; 
			m_ImporterToUse.pFindFunction = m_psDllStruct[i].pFindFunction;
			m_ImporterToUse.pLoadFoldersFunc = m_psDllStruct[i].pLoadFoldersFunc;
			m_ImporterToUse.pMakeMimeFunc = m_psDllStruct[i].pMakeMimeFunc;
			m_ImporterToUse.pAddrFunc = m_psDllStruct[i].pAddrFunc;
			m_ImporterToUse.pPersonalityFunc = m_psDllStruct[i].pPersonalityFunc;
			m_ImporterToUse.szDllPath = m_psDllStruct[i].szDllPath;
			m_ImporterToUse.szProviderName = m_psDllStruct[i].szProviderName;

		}
		else
		{
			m_psDllStruct[i].pQueryFunction = NULL;
			if(m_psDllStruct[i].hDll)
				::FreeLibrary(m_psDllStruct[i].hDll);
		}

	}


	CPersonalityTree* pPersonEntryList;
	pPersonEntryList = (CPersonalityTree *) m_ImporterToUse.pPersonalityFunc(Child->lpszIdentityName, bIsProfile);

	while(pPersonEntryList) {
		
		CPersParams *pPersona = DEBUG_NEW CPersParams; //the current account being processed
		
		BuildInternalPersonaStruct(pPersona, pPersonEntryList);

		CPersParams pPerson;
		g_Personalities.GetParams(CRString(IDS_DOMINANT), pPerson);
		bool isDominant = (pPerson.LoginName.IsEmpty()!=0);

		if (isDominant)
		{
			pPersona->PersName = CRString(IDS_DOMINANT);
			// ok its a dominant account, which always exists (so just modify)
			VERIFY(g_Personalities.Modify(*pPersona));
		}
		else
		{
			// ok this is a new personality
			ASSERT(!g_Personalities.IsA(pPersona->PersName));
			VERIFY(g_Personalities.Add(*pPersona));
		}
      
		UpdateWazoo(pPersona->PersName);

#ifdef IMAP4
		// Notify mailbox director that a Persona has been added.
		g_theMailboxDirector.AddPersona( pPersona->PersName, TRUE);
#endif
		pPersona = NULL;
		pPersonEntryList = pPersonEntryList->pNext;
	}

	return true;
}



bool CImportMail::ImportAddresses2(CImportChild *Child, CWnd * pParentWnd)
{



	for(int i = 0; i < m_iDllStructSize; i++)
	{
		if(strcmp(m_psDllStruct[i].szProviderName, Child->pParent->lpszDisplayName) == 0)
		{
			m_ImporterToUse.hDll = m_psDllStruct[i].hDll;
			m_ImporterToUse.pQueryFunction = m_psDllStruct[i].pQueryFunction; 
			m_ImporterToUse.pFindFunction = m_psDllStruct[i].pFindFunction;
			m_ImporterToUse.pLoadFoldersFunc = m_psDllStruct[i].pLoadFoldersFunc;
			m_ImporterToUse.pMakeMimeFunc = m_psDllStruct[i].pMakeMimeFunc;
			m_ImporterToUse.pAddrFunc = m_psDllStruct[i].pAddrFunc;
			m_ImporterToUse.pPersonalityFunc = m_psDllStruct[i].pPersonalityFunc;
			m_ImporterToUse.szDllPath = m_psDllStruct[i].szDllPath;
			m_ImporterToUse.szProviderName = m_psDllStruct[i].szProviderName;

		}
		else
		{
			m_psDllStruct[i].pQueryFunction = NULL;
			if(m_psDllStruct[i].hDll)
				::FreeLibrary(m_psDllStruct[i].hDll);
		}

	}


	//testing sangita
	//load personalities
	/*CPersonalityTree* pPersonEntryList;
	pPersonEntryList = (CPersonalityTree *) m_ImporterToUse.pPersonalityFunc(Child->lpszIdentityName);

	while(pPersonEntryList) {
		
		CPersParams *pPersona = new CPersParams; //the current account being processed
		
		BuildInternalPersonaStruct(pPersona, pPersonEntryList);

		CPersParams pPerson;
		g_Personalities.GetParams(CRString(IDS_DOMINANT), pPerson);
		bool isDominant = (pPerson.LoginName.IsEmpty()!=0);

		if (isDominant)
		{
			pPersona->PersName = CRString(IDS_DOMINANT);
			// ok its a dominant account, which always exists (so just modify)
			VERIFY(g_Personalities.Modify(*pPersona));
		}
		else
		{
			// ok this is a new personality
			ASSERT(!g_Personalities.IsA(pPersona->PersName));
			VERIFY(g_Personalities.Add(*pPersona));
		}
      
		UpdateWazoo(pPersona->PersName);

#ifdef IMAP4
		// Notify mailbox director that a Persona has been added.
		g_theMailboxDirector.AddPersona( pPersona->PersName, TRUE);
#endif
		pPersona = NULL;
		pPersonEntryList = pPersonEntryList->pNext;
	}*/


	//end personality testing

	CAddressBookTree * pEntryList = NULL;
	AbookEntry * pAddrIterator = NULL;

	// uninstall our mapi
	m_InstalledStatus = CMapiInstaller::Uninstall();

	pEntryList = (CAddressBookTree *) m_ImporterToUse.pAddrFunc(Child->lpszPathToAdrData, Child->lpszIdentityName);

	// install it back
	if (m_InstalledStatus != CMapiInstaller::STATUS_NOT_INSTALLED)
		CMapiInstaller::Install();

	if(pEntryList == NULL)
		return false;

	// loop through the address book tree and fill in the unique nick names for the entire addressbook
	CAddressBookTree * pABIterator = pEntryList;

	while(pABIterator) {

			AbookCreator *pABCreator;
			pABCreator = DEBUG_NEW AbookCreator;

			CAbookEntry *pTmpABEntry = pABIterator->pFirstChild;
			while(pABIterator->pFirstChild) {
				char *nickname = NULL;
				if (!pABCreator->FormNickname(pABIterator->pFirstChild, &nickname))
					return false;
				
				pABIterator->pFirstChild->nickname = DEBUG_NEW char[strlen(nickname) + 1];
				strcpy(pABIterator->pFirstChild->nickname, nickname);

				pABIterator->pFirstChild = pABIterator->pFirstChild->pNext;
			}
			pABIterator->pFirstChild = pTmpABEntry;

			CAbookDistListTree *pTmpDistList = pABIterator->pFirstDLChild;
			while(pABIterator->pFirstDLChild) {
				char *nickname = NULL;
				if (!pABCreator->FormNickname(pABIterator->pFirstDLChild, &nickname))
					return false;

				pABIterator->pFirstDLChild->pNickname = DEBUG_NEW char[strlen(nickname) + 1];
				strcpy(pABIterator->pFirstDLChild->pNickname, nickname);

				pABIterator->pFirstDLChild = pABIterator->pFirstDLChild->pNext;
			}
			pABIterator->pFirstDLChild = pTmpDistList;
			

		pABIterator = pABIterator->pNext;
	}


	do {

		BuildInternalAddrStruct(pEntryList->pFirstChild);

		if(m_pAddrEntry)
		{	
			pAddrIterator = m_pAddrEntry;
		}

		CString str;
		str.Format(IDS_CONVERTING_FROM, m_ImporterToUse.szProviderName);

		::MainProgress(str);

		char *NNFilePath = DEBUG_NEW char[_MAX_PATH];
		AbookCreator *abookcreator;
		abookcreator = DEBUG_NEW AbookCreator;

		char	*filename = DEBUG_NEW char[strlen(pEntryList->lpszDisplayName)+1];
		strcpy(filename, pEntryList->lpszDisplayName);

		if (!abookcreator->FindNNFileName(&filename, &NNFilePath))
		{
			delete [] filename;
			delete [] NNFilePath;
			delete abookcreator;
			return false;
		}
		if (!abookcreator->OpenNNFile(NNFilePath))
		{
			delete [] filename;
			delete [] NNFilePath;
			delete abookcreator;
			return false;
		}

		while(pAddrIterator)
		{
			abookcreator->AddEntry(pAddrIterator);
			pAddrIterator = pAddrIterator->next;
		}

		CAbookDistListTree *pDistListIterator = pEntryList->pFirstDLChild;
		while(pDistListIterator)
		{
			abookcreator->AddEntry(pDistListIterator);
			pDistListIterator = pDistListIterator->pNext;
		}

		abookcreator->CloseNNFile();
		::CloseProgress();

		CNicknameFile* NickFile = g_Nicknames->AddNicknameFile(NNFilePath, "");

	// Update views of addresbooks
		if (NickFile) {
			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_ADDED_NEW_NICKFILE, NickFile);

			NickFile->SetModified(true);
		}

		delete [] filename;
		delete [] NNFilePath;
		delete abookcreator;

		pEntryList = pEntryList->pNext;

	} while (pEntryList); 

	ErrorDialog(IDS_ADDR_RESTART);

	return true;
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
			newname = DEBUG_NEW char [strlen(*name)+3];	// .1\0
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

			num = DEBUG_NEW char[firstlen+2];
			sprintf(num, "%d", version);
			lastlen = strlen(num);
			if (lastlen > firstlen)
			{
				char * newname;
				newname = DEBUG_NEW char[strlen(*name)+(lastlen-firstlen)+1];
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


bool KillTheTOCFile(char *pathToMBX)
{
	char * pathToTOC;
	char * floater;

	if (!pathToMBX)
		return false;

	pathToTOC = DEBUG_NEW char[(strlen(pathToMBX))+1];
	strcpy(pathToTOC, pathToMBX);
	floater = strrchr(pathToTOC, '.');
	*floater = 0;
	strcat(pathToTOC, _T(".toc"));

	DeleteFile(pathToTOC);	// We want Eudora to build the toc itself from the file we're about to write.
	delete [] pathToTOC;
	return true;
}