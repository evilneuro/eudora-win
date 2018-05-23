// NICKDOC.CPP
//
// Routines for handling the content of nicknames
//

#include "stdafx.h"

#include <dos.h>
#include <direct.h>

#include <afxcmn.h>
#include <afxrich.h>		// for CRichEditView

#include "QCUtils.h"

#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "fileutil.h"
#include "guiutils.h"
#include "utils.h"
#include "doc.h"
#include "nickdoc.h"
#include "address.h"
#include "eudora.h"
#include "helpxdlg.h"
#include "namenick.h"

#include "AutoCompleteSearcher.h"
#include "lex822.h"

#include "QCSharewareManager.h"

#include "QCCommandActions.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"

#include <set.h>
#include <algo.h>

extern QCRecipientDirector	g_theRecipientDirector;


#ifdef _DEBUG
#undef THIS_FILE
#ifndef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#endif
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


static const char NotesNewlineChar = '\003';

IMPLEMENT_DYNCREATE(CNickname, CObject)

CNickname::CNickname(const char* Name /*= NULL*/)
{
	m_Name = Name;
	m_NotesOffset = m_AddressesOffset = -1L;
	m_AddressesLength = m_NotesLength = -1L;
	m_Recipient = m_Visited = FALSE;

	if (! m_Name.IsEmpty())
	{
		VERIFY(HasValidName());
	}
}

CNickname::CNickname(const CNickname& srcNickname)
{
	if (&srcNickname == this)
		return;

	m_Name =			srcNickname.m_Name;
	m_Addresses =		srcNickname.m_Addresses;			// Addresses
	m_Notes = 			srcNickname.m_Notes;				// Notes
	m_Recipient = 		FALSE;
	m_Visited = 		FALSE;
	m_AddressesOffset =	-1;
	m_NotesOffset = 	-1;
	m_AddressesLength =	m_Addresses.GetLength();
	m_NotesLength =		m_Notes.GetLength();

	if (! m_Name.IsEmpty())
	{
		VERIFY(HasValidName());
	}
}

void CNickname::UpdateRecipientList()
{
	QCRecipientCommand*	pCommand;

	pCommand = g_theRecipientDirector.Find( GetName() );
	
	if( IsRecipient() )
	{
		if( pCommand == NULL )
		{
			g_theRecipientDirector.AddCommand( GetName() );
		}
	}
	else if( pCommand != NULL ) 
	{
		pCommand->Execute( CA_DELETE );
	}
} 


const CString& CNickname::GetName() const
{
	if (! m_Name.IsEmpty())
	{
		VERIFY(HasValidName());
	}
	return (m_Name);
}


void CNickname::SetName(const CString& newName) 
{
	m_Name = newName; 

	if (! m_Name.IsEmpty())
	{
		VERIFY(HasValidName());
	}
}



////////////////////////////////////////////////////////////////////////
// HasValidName [public]
//
// Nickname names cannot contain leading or trailing spaces, or embedded
// commas.  Everything else is free game.
//
////////////////////////////////////////////////////////////////////////
BOOL CNickname::HasValidName() const
{
	if (m_Name.IsEmpty() ||
		m_Name[0] == ' ' ||
		m_Name[m_Name.GetLength() - 1] == ' ' ||
		m_Name.Find(',') >= 0 ||
		m_Name.GetLength() > CNickname::MAX_NAME_LENGTH)
	{
		return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// GetElaboratedAddresses [public]
//
////////////////////////////////////////////////////////////////////////
CString CNickname::GetElaboratedAddresses() const
{
	//
	// If the user wants the old, "unelaborated" behavior, then
	// just return the plain address string.
	//
	if (GetIniLong(IDS_INI_SIMPLE_NICKNAME_EXPANSION))
		return GetAddresses();

	//
	// Get the "real name" field value, massaging it as necessary
	// to deal with reserved characters and such.
	//
	CString name;
	GetNotesFieldValue(CRString(IDS_NICK_FIELD_NAME), name);
	::TrimWhitespaceMT(name);		// remove whitespace

	if (!name.IsEmpty() && !m_Addresses.IsEmpty())
	{
		//
		// Determine the number of addresses in addresses field, in
		// order to determine whether to use the "group" syntax, or
		// the simple real name syntax.
		//
		int num_addresses = 0;
		const char* p_start = m_Addresses;
		while (num_addresses <= 1)
		{
			if ('\0' == *p_start)
				break;

			const char* p_end = ::FindAddressEnd(p_start, FALSE);
			if (NULL == p_end)
			{
				ASSERT(0);			// FindAddressEnd() should always return a non-NULL ptr
				return GetAddresses();
			}

			// Get an address
			char addr[256];
			int len = p_end - p_start;
			if (len > sizeof(addr) - 1)
				return GetAddresses();			// address too long

			strncpy(addr, p_start, len);
			addr[len] = 0;
			::TrimWhitespaceMT(addr);

			if (strlen(addr) > 0)
				num_addresses++;

			// Seek to the start of the next address
			for (p_start = p_end; *p_start == ',' || *p_start == '\r' || *p_start == '\n'; p_start++)
				;
		}

		if (num_addresses > 1)
		{
			//
			// Use group syntax for addressing:  
			//
			//        name:addr1,addr2;
			//
			//TRACE0("CNickname::GetElaboratedAddresses() generating multi-address group syntax\n");
			{
				//
				// Quote the group name, if necessary.  The quotes will be
				// added if the group name contains spaces or other specials.
				//
				CString quoted_name;
				::Quote822(quoted_name.GetBuffer(name.GetLength() + 40), name, FALSE);
				quoted_name.ReleaseBuffer();
				name = quoted_name;
			}

			//
			// Add the colon separator, then add the addresses, converting
			// any "\r\n" sequences to ", " sequences.  Yup, building up
			// the string one character at a time is slow and expensive,
			// but it works.
			//
			// Once we have the addresses in comma-separated form, then
			// run them immediately through ExpandAliases() so that the
			// group syntax wrapper applies to the group as a whole.
			//
			name += ":";
			const char* a = m_Addresses;
			CString addresses;
			for (; *a; a++)
			{
				if (*a == '\r' || *a == '\n')
				{
					// Append a comma separator, if necessary
					if ((a != (const char*) m_Addresses) && (name[name.GetLength() - 1] != ','))
					{
						addresses += ',';
						addresses += ' ';
					}
					while (a[1] == '\r' || a[1] == '\n')
						a++;
				}
				else
					addresses += *a;
			}

			char *p_expanded_addrs = ::ExpandAliases(addresses, TRUE, FALSE, FALSE);
			if (p_expanded_addrs)
			{
				name += p_expanded_addrs;
				delete [] p_expanded_addrs;
			}
			else
				return CString("");		// probably a circular reference

			//
			// Add the final terminating semi-colon.
			//
			name += ";";

			return name;
		}
		else
		{
			//
			// Use real name syntax for addressing:
			//
			//        name <addr>
			//
			// or
			//
			//        addr (name)
			//
			// if there is the proper override for the ReturnAddressFormat
			// INI setting.
			//
			//TRACE0("CNickname::GetElaboratedAddresses() generating single-address syntax\n");

			//
			// If the lone address is a nickname, then don't elaborate it.
			// If you do elaborate it, then ExpandAliases() won't recognize
			// it as a nickname anymore, so it won't get expanded.
			//
			if (g_Nicknames && (NULL == g_Nicknames->Find(m_Addresses)))
			{
				//
				// First, if the address already contains a real name part,
				// strip it out in favor of the Real Name field from the
				// nickname entry.
				//
				CString address(m_Addresses);		// make working copy
				::StripAddress(address.GetBuffer(address.GetLength()));
				address.ReleaseBuffer();
				if (! address.IsEmpty())
				{
					CString final_address;
					::FormatAddress(final_address, name, address);
					return final_address;
				}
			}
		}
	}

	//
	// If all else fails, just return the plain, raw addresses.
	//
	return GetAddresses();
}


////////////////////////////////////////////////////////////////////////
// GetNotes [public]
//
// Strip the valid fields from the m_Notes string and return the remaining
// stuff.
////////////////////////////////////////////////////////////////////////
CString CNickname::GetNotes() const
{
	CString notes(m_Notes);
	CStringList field_list;		// unused
	RemoveFieldsFromNotes(notes, field_list);

	return notes;
}


////////////////////////////////////////////////////////////////////////
// SetNotes [public]
//
// Temporarily strip the valid fields from the m_Notes string,
// toss what's left, then add back the fields to the new value.
////////////////////////////////////////////////////////////////////////
void CNickname::SetNotes(const CString& newNotes)
{
	CStringList field_list;
	RemoveFieldsFromNotes(m_Notes, field_list);

	m_Notes = newNotes;
	while (! field_list.IsEmpty())
	{
		CString field = field_list.RemoveHead();
		m_Notes = field + m_Notes;
	}

	SetRawNotesLength(m_Notes.GetLength());
}


////////////////////////////////////////////////////////////////////////
// GetNotesFieldValue [public]
//
// Search the m_Notes string for a field with a matching name (case
// insensitive), then copy the value to the caller-provided fieldValue
// string.  Return TRUE if a match is found.  Otherwise, return FALSE
// if there is no field with a matching name.
////////////////////////////////////////////////////////////////////////
BOOL CNickname::GetNotesFieldValue(const CString& fieldName, CString& fieldValue) const
{

	//
	// Parse the given Notes string, searching for a field with a
	// matching field name.
	//
	ASSERT(! fieldName.IsEmpty());
	int start_idx = 0;
	for (;;)
	{
		//
		// Search for the next available left bracket.
		//
		while (start_idx < m_Notes.GetLength())
		{
			if ('<' == m_Notes[start_idx])
				break;		// found left bracket
			start_idx++;
		}

		if (start_idx >= m_Notes.GetLength())
			break;			// done processing m_Notes string

		//
		// If we get this far, we've found the next available opening
		// bracket, so find the matching closing bracket, if any.
		// Nesting of fields is not supported, so if a field is nested
		// within a field as in "<field1<field2>>", then the "inner"
		// field is simply treated as part of the "outer" field.
		//
		int nesting_level = 0;
		int end_idx = -1;
		int idx = start_idx + 1;
		while (idx < m_Notes.GetLength())
		{
			if ('<' == m_Notes[idx])
				nesting_level++;	// found nested bracket
			else if ('>' == m_Notes[idx])
			{
				if (0 == nesting_level)
				{
					end_idx = idx;
					break;
				}
				else
					nesting_level--;
			}
			idx++;
		}

		if (end_idx != -1)
		{
			//
			// Found bracketed string, so check to see if it has a
			// matching field name.
			//
			CString field(m_Notes.Mid(start_idx, end_idx - start_idx + 1));
			int colon_idx = field.Find(':');
			if (colon_idx != -1)
			{
				// Isolate the field name, sans the left bracket and the colon.
				CString field_name(field.Mid(1, colon_idx - 1));

				if (fieldName.CompareNoCase(field_name) == 0)
				{
					// Isolate the field value, sans the colon and the right bracket.
					fieldValue = field.Mid(colon_idx + 1, field.GetLength() - colon_idx - 2);
					return TRUE;		// found what we were looking for
				}
			}
			//
			// This is either not a valid field or it is not a matching
			// field, so start the next search at the terminating right bracket.
			//
			start_idx = end_idx + 1;
		}
		else
			break;		// no more fields, so we're done processing the notes string
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// SetNotesFieldValue [public]
//
// Search the m_Notes string for a field with a matching fieldName (case
// insensitive), either overwriting any old field value with the caller-provided fieldValue
// string, or adding a whole new field.  Return TRUE if successful.
////////////////////////////////////////////////////////////////////////
BOOL CNickname::SetNotesFieldValue(const CString& fieldName, const CString& fieldValue)
{

	//
	// Parse the given Notes string, searching for a field with a
	// matching field name.
	//
	ASSERT(! fieldName.IsEmpty());
	int start_idx = 0;
	for (;;)
	{
		//
		// Search for the next available left bracket.
		//
		while (start_idx < m_Notes.GetLength())
		{
			if ('<' == m_Notes[start_idx])
				break;		// found left bracket
			start_idx++;
		}

		if (start_idx >= m_Notes.GetLength())
			break;			// done processing m_Notes string

		//
		// If we get this far, we've found the next available opening
		// bracket, so find the matching closing bracket, if any.
		// Nesting of fields is not supported, so if a field is nested
		// within a field as in "<field1<field2>>", then the "inner"
		// field is simply treated as part of the "outer" field.
		//
		int nesting_level = 0;
		int end_idx = -1;
		int idx = start_idx + 1;
		while (idx < m_Notes.GetLength())
		{
			if ('<' == m_Notes[idx])
				nesting_level++;	// found nested bracket
			else if ('>' == m_Notes[idx])
			{
				if (0 == nesting_level)
				{
					end_idx = idx;
					break;
				}
				else
					nesting_level--;
			}
			idx++;
		}

		if (end_idx != -1)
		{
			//
			// Found bracketed string, so check to see if it has a
			// matching field name.
			//
			CString field(m_Notes.Mid(start_idx, end_idx - start_idx + 1));
			int colon_idx = field.Find(':');
			if (colon_idx != -1)
			{
				// Isolate the field name, sans the left bracket and the colon.
				CString field_name(field.Mid(1, colon_idx - 1));

				if (fieldName.CompareNoCase(field_name) == 0)
				{
					//
					// Okay, this is the hard case.  We need to "overwrite" the
					// old field substring from m_Notes with the new field name/value
					// pair.  This involves breaking the existing m_Notes string into
					// three pieces.
					//
					CString left_part(m_Notes.Left(start_idx));								// can be empty
					CString right_part(m_Notes.Right(m_Notes.GetLength() - end_idx - 1));	// can be empty
					m_Notes = left_part + '<' + fieldName + ':' + fieldValue + '>' + right_part;
					SetRawNotesLength(m_Notes.GetLength());
					return TRUE;		// overwrite case complete
				}
			}

			//
			// This is either not a valid field or it is not a matching
			// field, so start the next search at the terminating right bracket.
			//
			start_idx = end_idx + 1;
		}
		else
			break;		// no more fields, so we're done processing the notes string
	}

	//
	// If we get this far, there is no matching field name, so just
	// insert the new field at the front of the string.
	//
	m_Notes = '<' + fieldName + ':' + fieldValue + '>' + m_Notes;
	SetRawNotesLength(m_Notes.GetLength());
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// RemoveFieldsFromNotes [private]
//
// This method scans for and REMOVES field substrings from the m_Notes
// string and ADDS them in the m_FieldList string list.  We clear any
// leftover field strings from m_FieldList so that we start with a
// clean slate each time.
//
////////////////////////////////////////////////////////////////////////
BOOL CNickname::RemoveFieldsFromNotes(CString& notesString, CStringList& fieldList) const
{

	ASSERT(fieldList.IsEmpty());

	//
	// Parse the given Notes string, extracting known nickname fields
	// from the string.  Unrecognized fields are left in the original
	// Notes string as-is.
	//
	int start_idx = 0;
	for (;;)
	{
		//
		// Search for the next available left bracket.
		//
		while (start_idx < notesString.GetLength())
		{
			if ('<' == notesString[start_idx])
				break;		// found left bracket
			start_idx++;
		}

		if (start_idx >= notesString.GetLength())
			break;			// done processing notes string

		//
		// If we get this far, we've found the next available opening
		// bracket, so find the matching closing bracket, if any.
		// Nesting of fields is not supported, so if a field is nested
		// within a field as in "<field1<field2>>", then the "inner"
		// field is simply treated as part of the "outer" field.
		//
		int nesting_level = 0;
		int end_idx = -1;
		int idx = start_idx + 1;
		while (idx < notesString.GetLength())
		{
			if ('<' == notesString[idx])
				nesting_level++;	// found nested bracket
			else if ('>' == notesString[idx])
			{
				if (0 == nesting_level)
				{
					end_idx = idx;
					break;
				}
				else
					nesting_level--;
			}
			idx++;
		}

		if (end_idx != -1)
		{
			//
			// Extract bracketed string (complete with outer brackets) and
			// verify whether or not it is a valid, recognized field.
			//
			CString field(notesString.Mid(start_idx, end_idx - start_idx + 1));
			int colon_idx = field.Find(':');
			if (colon_idx != -1)
			{
				// Isolate the field name, sans the left bracket and the colon.
				CString field_name(field.Mid(1, colon_idx - 1));

				ASSERT(g_Nicknames != NULL);
				if (g_Nicknames->IsNicknameFieldName(field_name))
				{
					// Isolate the field value, sans the colon and the right bracket.
					CString field_value(field.Mid(colon_idx + 1, field.GetLength() - colon_idx - 2));

					//
					// Remove the bracketed field from the given notes string,
					// then add it to the caller-provided list of Nickname fields,
					// overwriting any existing field with the same name.
					//
					notesString = notesString.Left(start_idx) + notesString.Mid(end_idx + 1);
					UpdateFieldList(fieldList, field_name, field_value);
				}
				else
				{
					//
					// This is not a valid field, so start the next search 
					// at the terminating right bracket.
					//
					start_idx = end_idx + 1;
				}
			}
			else
			{
				//
				// This is not a valid field, so start the next search 
				// at the terminating right bracket.
				//
				start_idx = end_idx + 1;
			}
		}
		else
			break;		// no more fields, so we're done processing the notes string
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// UpdateFieldList [private]
//
////////////////////////////////////////////////////////////////////////
BOOL CNickname::UpdateFieldList(CStringList& fieldList, const CString& fieldName, const CString& fieldValue) const
{

	ASSERT(-1 == fieldName.FindOneOf("<:>"));

	POSITION pos = fieldList.GetHeadPosition();
	while (pos != NULL)
	{
		CString existing_field = fieldList.GetAt(pos);

		int colon_idx = existing_field.Find(':');
		if (colon_idx != -1)
		{
			// Isolate the existing field name, sans the left bracket and the colon.
			CString existing_field_name(existing_field.Mid(1, colon_idx - 1));
			if (fieldName.CompareNoCase(existing_field_name) == 0)
			{
				//
				// Overwrite the existing field with a new value.
				// code below to add back a replacement field with
				// the same field name.
				//
				fieldList.SetAt(pos, '<' + fieldName + ':' + fieldValue + '>');
				return TRUE;
			}
		}
		else
		{
			ASSERT(0);			// uh, oh ... malformed field
			return FALSE;
		}

		fieldList.GetNext(pos);
	}

	//
	// If we get this far, there is either no match for the field name,
	// so add a new field to the end of the list.
	//
	fieldList.AddTail('<' + fieldName + ':' + fieldValue + '>');
	return TRUE;
}


IMPLEMENT_DYNCREATE(CNicknameFile, CObList)

CNicknameFile::CNicknameFile(const char* Filename) : CObList(),
	m_Filename(Filename)
{
	CRString preExt(IDS_FILTER_PRE_EXT);
	
	m_ReadOnly = FALSE;
	m_IsModified = FALSE;
	m_Invisible = FALSE;


	// For the name, just get the file name and remove the extension
	char Name[_MAX_PATH + 1];
	strcpy(Name, m_Filename);
	char* s = strrchr(Name, SLASH);
	if (s)
		strcpy(Name, s + 1);
	s = strrchr(Name, '.');
	if (s)
	{
		if (preExt.CompareNoCase(s+1) == 0)
			m_Invisible = TRUE;
		*s = 0;
	}
	m_Name = Name;
}

CNicknameFile::~CNicknameFile()
{
	RemoveAll();
}

void CNicknameFile::SetModified(BOOL IsModified /*= TRUE*/)
{
	m_IsModified = IsModified;
	if (g_Nicknames && IsModified)
		g_Nicknames->SetModifiedFlag(TRUE);
}

CNickname* CNicknameFile::CreateNewNickname(const char *Name)
{
	CNickname* nn = new CNickname(Name);
	
	// If the item is on the recipient list, then mark it as such
	if( g_theRecipientDirector.Find( Name ) )
	{
		nn->AddRecipient();
	}

	AddTail(nn);
	
	SetModified(TRUE);
	
	return (nn);
}


////////////////////////////////////////////////////////////////////////
// AddNickname [public]
//
// Really smart routine that adds the data from the given CNickname 
// to this CNicknameFile.  If this CNicknameFile already contains a
// nickname with the same name, then prompt the user for whether she
// wishes to:
//
//	1.  Append the nickname data,
//	2.  Overwrite the nickname data, or
//	3.  Get prompted for a new nickname name.
//
// Returns a pointer to either an existing updated CNickname object,
// or a newly added CNickname object.
////////////////////////////////////////////////////////////////////////
CNickname* CNicknameFile::AddNickname(const CNickname& srcNickname, BOOL bAddToRecipientList /*=FALSE*/)
{
	if (! srcNickname.HasValidName())
	{
		ASSERT(0);
		return NULL;
	}

	//
	// Check for a duplicate nickname.
	//
	CNickname* p_target_nickname = FindNickname(srcNickname.GetName());

	if (p_target_nickname)
	{
		//
		// Found duplicate, so ask user how we should proceed...
		//
		switch (AlertDialog(IDD_NICK_DUPLICATE, (const char*) p_target_nickname->GetName()))
		{
		case IDC_NICK_ADD_TO_IT:
			{
				// Make sure existing data is read in
				if (p_target_nickname->NeedsReadIn())
					ReadNickname(p_target_nickname);

				//
				// Append address info.  FORNOW, not doing anything
				// smart about stripping duplicate addresses.
				//
				if (p_target_nickname->GetAddresses().IsEmpty())
					p_target_nickname->SetAddresses(srcNickname.GetAddresses());
				else
					p_target_nickname->SetAddresses(p_target_nickname->GetAddresses() + "\r\n" + srcNickname.GetAddresses());
				p_target_nickname->SetAddressesLength(p_target_nickname->GetAddresses().GetLength());
				p_target_nickname->SetAddressesOffset(-1);				// FORNOW, is this right?

				//
				// FORNOW, what do we do with the Notes stuff?  Attempt
				// a smart Notes merge?  (Don't want duplicate special
				// fields like Name, Postal Address, etc.) Just leave
				// the original Notes alone?
				//

				//
				// Set recipient list status.
				//
				if (bAddToRecipientList)
					p_target_nickname->AddRecipient();
			}
			return p_target_nickname;
		case IDOK:
			{
				p_target_nickname->SetName(srcNickname.GetName());		// in case user is picky about lower vs. upper case
				p_target_nickname->SetAddresses(srcNickname.GetAddresses());
				p_target_nickname->SetAddressesLength(p_target_nickname->GetAddresses().GetLength());
				p_target_nickname->SetAddressesOffset(-1);				// FORNOW, is this right?
				p_target_nickname->SetRawNotes(srcNickname.GetRawNotes());
				p_target_nickname->SetRawNotesLength(p_target_nickname->GetRawNotes().GetLength());
				p_target_nickname->SetNotesOffset(-1);					// FORNOW, is this right?

				//
				// Set recipient list status.
				//
				if (bAddToRecipientList)
					p_target_nickname->AddRecipient();
			}
			return p_target_nickname;
		case IDCANCEL:
			{
				//
				// Let the user choose a new name.  Loop until user picks
				// a unique name or cancels.
				//
				while (1)
				{
					CNameNickDialog dlg(IDCANCEL, this);

					ASSERT(dlg.m_NewName.IsEmpty());		// can be empty
					ASSERT(NULL == dlg.m_pNicknameFile);
					
					if (dlg.DoModal() == IDCANCEL)
					{
						p_target_nickname = NULL;			// skip this nickname
						break;
					}

					//
					// If we get this far, then check to see if have a
					// candidate nickname name to work with.
					//
					ASSERT(this == dlg.m_pNicknameFile);

					//
					// Double check that duplicate nickname name doesn't exist and
					// that the candidate name is valid.
					//
					CNickname tempnick(dlg.m_NewName);
					if ((! tempnick.HasValidName()) || FindNickname(dlg.m_NewName))
					{
						ASSERT(0);
						break;
					}

					//
					// At this point, we are ready to add a new nickname
					// to the database....
					//
					p_target_nickname = CreateNewNickname(dlg.m_NewName);

					// Set recipient list status...
					if (dlg.m_PutOnRecipientList)
						bAddToRecipientList = TRUE;

					//
					// Fall through to generic nickname population code below...
					//
					break;
				}
			}
			break;
		default:
			ASSERT(0);
			return NULL;
		}
	}
	else
	{
		//
		// No existing nickname with the given name, so create one.
		//
		p_target_nickname = CreateNewNickname(srcNickname.GetName());
	}

	if (p_target_nickname)
	{
		//
		// Set recipient list status.
		//
		if (bAddToRecipientList)
			p_target_nickname->AddRecipient();

		//
		// Populate a newly created nickname with stuff from the
		// source nickname.
		//
		if (! srcNickname.GetAddresses().IsEmpty())
		{
			p_target_nickname->SetAddresses(srcNickname.GetAddresses());
			p_target_nickname->SetAddressesLength(p_target_nickname->GetAddresses().GetLength());
		}
		ASSERT(-1 == p_target_nickname->GetAddressesOffset());
		if (! srcNickname.GetRawNotes().IsEmpty())
		{
			p_target_nickname->SetRawNotes(srcNickname.GetRawNotes());
			p_target_nickname->SetRawNotesLength(p_target_nickname->GetRawNotes().GetLength());
		}
		ASSERT(-1 == p_target_nickname->GetNotesOffset());

		//
		// Okay, now update the view to display the newly added
		// nickname.  However, since CNickname objects don't know
		// which file they're in, we need to give the view a clue
		// which file the new nickname belongs to.  We do a clever
		// hack here -- since we know that a new nickname has an empty
		// addresses section, we *temporarily* stuff the CNicknameFile
		// pointer into the "addresses offset" member of the CNickname
		// object.  After we return from the view updater, set the
		// addresses offset back to its "uninitialized" value.
		//
		ASSERT(p_target_nickname->GetAddressesOffset() == -1L);
		p_target_nickname->SetAddressesOffset(long(this));				// do hack
		g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_ADDED_NEW_NICKNAME, p_target_nickname);
		ASSERT(p_target_nickname->GetAddressesOffset() == long(this));
		p_target_nickname->SetAddressesOffset(-1L);						// undo hack

		return p_target_nickname;
	}

	return NULL;
}


void CNicknameFile::RemoveAt(POSITION pos)
{
	CNickname*			nn = GetAt(pos);
	QCRecipientCommand*	pCommand;

	if (nn->IsRecipient())
	{
		pCommand = g_theRecipientDirector.Find( nn->GetName() );
		
		if( pCommand != NULL )
		{
			pCommand->Execute( CA_DELETE );
		}
	}

	CObList::RemoveAt(pos);
	delete nn;
}

void CNicknameFile::RemoveAll()
{
	while (!IsEmpty())
		delete RemoveHead();
}

CNickname* CNicknameFile::FindNickname(const char* pName)
{
	if (NULL == pName)
	{
		ASSERT(0);
		return NULL;
	}

	POSITION pos = GetHeadPosition();
	
	while (pos)
	{
		CNickname* nn = GetNext(pos);
		if (nn)
		{
			if (!stricmp(pName, nn->GetName()))
				return (nn);
		}
		else
		{
			ASSERT(0);
			return NULL;
		}
	}
	
	return (NULL);
}

POSITION CNicknameFile::FindNicknameStartingWith(const char* Prefix, POSITION pos /*= NULL*/)
{
	if (!Prefix || !*Prefix)
		return (NULL);
		
	if (pos)
		GetNext(pos);
	else
		pos = GetHeadPosition();
	
	int Len = strlen(Prefix);
	POSITION NextPos = pos;
	for (; pos; pos = NextPos)
	{
		CNickname* nn = GetNext(NextPos);
		if (!strnicmp(Prefix, nn->GetName(), Len))
			return (pos);
	}
	
	return (NULL);
}


BOOL CNicknameFile::ReadToc()
{
	CFileStatus TxtStatus, TocStatus;
	char TocName[_MAX_PATH + 1];
	char buf[128];
	short Version;
	long Status;
	JJFile in;

	strcpy(TocName, m_Filename);
	::SetFileExtensionMT(TocName, CRString(IDS_NICK_TOC_EXTENSION));
	
	while (1)
	{
		// If the toc file doesn't exist or has an earlier date than the text file,
		// then something is amiss, and so rebuild the toc file
		if (CFile::GetStatus(m_Filename, TxtStatus) &&
			(!CFile::GetStatus(TocName, TocStatus) || TxtStatus.m_mtime > TocStatus.m_mtime))
		{
			break;
		}
		
		long TxtLength = TxtStatus.m_size;
		
		if (FAILED(in.Open(TocName, O_RDONLY)))
			break;
		
		if (FAILED(in.Get(&Version)) || Version != TocVersion)
			break;
		
		while (SUCCEEDED(in.GetLine(buf, sizeof(buf), &Status)) && (Status > 0))
		{
			long addrs_offset = -1;
			long notes_offset = -1;
			long addrs_length = -1;
			long notes_length = -1;

			CNickname* nn = CreateNewNickname(buf);
			if (!nn ||
				FAILED(in.Get(&addrs_offset)) ||
				FAILED(in.Get(&addrs_length)) ||
				FAILED(in.Get(&notes_offset)) ||
				FAILED(in.Get(&notes_length)))
			{
				Status = -1;
				break;
			}
			
			nn->SetAddressesOffset(addrs_offset);
			nn->SetAddressesLength(addrs_length);
			nn->SetNotesOffset(notes_offset);
			nn->SetRawNotesLength(notes_length);

			// Do some checks on the info that was just read in
			// Offset+length may be longer than file because <CR><LF>s get replaceed with single characters
			if (nn->GetAddressesOffset() < -1 || nn->GetAddressesLength() < -1 ||
				nn->GetAddressesOffset() > TxtLength ||
				nn->GetNotesOffset() < -1 || nn->GetRawNotesLength() < -1 ||
				nn->GetNotesOffset() > TxtLength)
			{
				ASSERT(FALSE);
				Status = -1;
				break;
			}
		}
		
		if (Status < 0)
			break;
			
		SetModified(FALSE);
			
		return (TRUE);
	}

	in.Close();
	
	return (RebuildToc());
}

BOOL CNicknameFile::ReadNickname(CNickname* nn)
{
	long Length, OldLength;
	JJFile in;
	char* NewBuf;
	char* buf;
	char* b;
	char ch;

	if (FAILED(in.Open(m_Filename, O_RDONLY)))
		return (FALSE);

	while (1)
	{
		// Read in the addresses
		if (nn->GetAddressesOffset() >= 0)
		{
			if (FAILED(in.Seek(nn->GetAddressesOffset())))
				break;
			
			Length = nn->GetAddressesLength() + 256;
			buf = new char[Length + 1];
			int Quote = 0;
			int Paren = 0;
			bool wasComma = false;

			//
			// This loop reads one character at a time, until we hit EOF
			// or a <CR><LF>.
			//
			HRESULT hrGet = S_OK;  // need this to know when we've hit the EOF
			for (b = buf; ((hrGet = in.Get(&ch)) == S_OK) && ch != '\r' && ch != '\n'; Length--)
			{
				if (!Quote && ch == '(')
					Paren++;
				else if (!Quote && ch == ')')
					Paren--;
				else if (!Paren && ch == '"')
					Quote = !Quote;
					
				// If we're running out of buffer space, allocate a bigger buffer
				if (Length < 3)
				{
					*b = 0;
					OldLength = strlen(buf);
					Length = OldLength + 256;
					if (!(NewBuf = new char[Length + 1]))
						break;
					strcpy(NewBuf, buf);
					delete [] buf;
					buf = NewBuf;
					b = buf + OldLength;
					// this properly updates 
					// control variable [jab]
					Length -= OldLength;
				}
				
				if (!Quote && !Paren && ch == ',')
				{
					// Separator commas get replaced with <CR><LF>s
					*b++ = '\r';
					*b++ = '\n';
					Length--;
					wasComma = true;
				}
				else
				{
					if (wasComma && ch == ' ')
						Length++;	// skip space after comma; leaves extra space in buffer
					else
						*b++ = ch;
					wasComma = false;
				}
			}
			if (FAILED(hrGet) && ch != '\r' && ch != '\n')
			{
				//
				// It looks like the only way to get here is if we can't allocate
				// a larger buffer.  In this case, cleanup and bail the outer 
				// while(1) loop.
				// OR we encountered a file error
				//
				delete [] buf;
				break;
			}
			*b = 0;
			int i = strlen(buf);
			nn->SetAddresses(buf);
			delete [] buf;
		}
	
		// Read in the notes
		if (nn->GetNotesOffset() >= 0 && nn->GetRawNotesLength() > 0)
		{
			if (FAILED(in.Seek(nn->GetNotesOffset())))
				break;
			
			Length = nn->GetRawNotesLength() + 256;
			buf = new char[Length + 1];
			
			//
			// This loop reads one character at a time, until we hit EOF
			// or a <CR><LF>.
			//
			HRESULT hrGet = S_OK;  // need this to know when we've hit the EOF
			for (b = buf; ((hrGet = in.Get(&ch)) == S_OK) && ch != '\r' && ch != '\n'; Length--)
			{
				// If we're running out of buffer space, allocate a bigger buffer
				if (Length < 3)
				{
					*b = 0;
					OldLength = strlen(buf);
					Length = OldLength + 256;
					if (!(NewBuf = new char[Length + 1]))
						break;
					strcpy(NewBuf, buf);
					delete [] buf;
					buf = NewBuf;
					b = buf + OldLength;
				}

				if (ch == NotesNewlineChar)
				{
					// Ctrl+Cs get replaced with <CR><LF>s (compatibility with Mac Eudora)
					*b++ = '\r';
					*b++ = '\n';
					Length--;
				}
				else
			    	*b++ = ch;
			}
			if (FAILED(hrGet) && ch != '\r' && ch != '\n')
			{
				//
				// It looks like the only way to get here is if we can't allocate
				// a larger buffer.  In this case, cleanup and bail the outer 
				// while(1) loop.
				// OR we encountered a file error
				//
				delete []buf;
				break;
			}
			*b = 0;
			nn->SetRawNotes(buf);
			delete []buf;
		} 

		return (TRUE);
	}
	
	in.Close();
	
	static BOOL PreventRecursion = FALSE;
	
	if (PreventRecursion)
		return (FALSE);
	PreventRecursion = TRUE;
	BOOL Success = RebuildToc();
	PreventRecursion = FALSE;
	if (!Success)
		return (FALSE);
	
	return (ReadNickname(nn));
}

BOOL CNicknameFile::WriteTxt()
{
	CRString AddressesKeyword(IDS_NICK_ALIAS);
	CRString NotesKeyword(IDS_NICK_NOTE);
	int AddressesLen = AddressesKeyword.GetLength();
	int NotesLen = NotesKeyword.GetLength();
	char TempFilename[_MAX_PATH + 1];
	JJFile out;

	strcpy(TempFilename, TempDir);
	strcat(TempFilename, CRString(IDS_NICK_DBASE_TMP));

	if (FAILED(out.Open(TempFilename, O_WRONLY | O_CREAT | O_TRUNC)))
		return (FALSE);

	POSITION NextPos, pos = GetHeadPosition();
	for (NextPos = pos; pos; pos = NextPos)
	{
		CNickname* nn = GetNext(NextPos);
		
		if (nn->NeedsReadIn() && !ReadNickname(nn))
			break;
		
		// Write addresses keyword and the name
		if (FAILED(out.Put(AddressesKeyword, AddressesLen)) || FAILED(out.Put(' ')))
			break;
		
		// Nicknames with spaces in name need to be surrounded in quotes
		BOOL HasSpace = nn->GetName().Find(' ') >= 0;
		if (HasSpace && FAILED(out.Put('"')))
			break;
		if (FAILED(out.Put(nn->GetName(), nn->GetName().GetLength())))
			break;
		if (HasSpace && FAILED(out.Put('"')))
			break;
		if (FAILED(out.Put(' ')))
			break;
		
		// Set offset for the start of the addresses
		{
			long lCurrent = 0;
			out.Tell(&lCurrent);
			ASSERT(lCurrent >= 0);
			nn->SetAddressesOffset(lCurrent);
		}
					
		// Write addresses, replacing <CR><LF>s with commas
		nn->SetAddressesLength(nn->GetAddresses().GetLength());
		const char* a = nn->GetAddresses();
		for (; *a; a++)
		{
			if (*a == '\r' || *a == '\n')
			{
				// If not at the beginning of the Addresses, put out comma as a separator
				if (a != (const char*)nn->GetAddresses() && FAILED(out.Put(',')))
					break;
				while (a[1] == '\r' || a[1] == '\n')
					a++;
			}
			else if (FAILED(out.Put(*a)))
				break;
		}
		// If not pointing at the end of the string here, something went wrong
		if (*a)
			break;
		if (FAILED(out.PutLine()))
			break;
			
		if (nn->GetRawNotesLength() > 0)
		{
			// Write notes keyword and the name (including quotes if the name has spaces)
			if (FAILED(out.Put(NotesKeyword, NotesLen)) || FAILED(out.Put(' ')))
				break;
			if (HasSpace && FAILED(out.Put('"')))
				break;
			if (FAILED(out.Put(nn->GetName(), nn->GetName().GetLength())))
				break;
			if (HasSpace && FAILED(out.Put('"')))
				break;
			if (FAILED(out.Put(' ')))
				break;

			// Set offset for the start of the notes
			{
				long lCurrent = 0;
				out.Tell(&lCurrent);
				ASSERT(lCurrent >= 0);
				nn->SetNotesOffset(lCurrent);
			}
				
			// Write notes, replacing <CR><LF>s with Ctrl+Cs (compatibility with Mac Eudora)
			nn->SetRawNotesLength(nn->GetRawNotes().GetLength());
			const char* n = nn->GetRawNotes();
			for (; *n; n++)
			{
				if (*n == '\r')
				{
					if (FAILED(out.Put(NotesNewlineChar)))
						break;
					if (n[1] == '\n')
						n++;
				}
				else if (FAILED(out.Put(*n)))
					break;
			}
			// If not pointing at the end of the string here, something went wrong
			if (*n)
				break;
			if (FAILED(out.PutLine()))
				break;
		}
	}
	
	if (pos)
	{
		out.Delete();
		return (FALSE);
	}

	out.Rename(m_Filename);

	return (TRUE);
}

BOOL CNicknameFile::RebuildToc()
{
	JJFile in(JJFileMT::BUF_SIZE, FALSE);
	char ch;

	if (FAILED(in.Open(m_Filename, O_RDONLY)))
		return FALSE;
	else if (FAILED(in.Get(&ch)))
		return (FALSE);

	in.Seek(0);

	// A pretty primitive check to see if we're dealing with an old PC Eudora
	// nickname file or a .mailrc file
	if (ch == '<')
		ReadOld(in);
	else
		ReadMailrc(in);

	in.Close();

	return (WriteToc());
}

BOOL CNicknameFile::ReadOld(JJFile& in)
{
	CNickname* nn = NULL;
	char buf[1024];
	char OldName[64];
    int Section; 	// Can be ADDRESSES or NOTES

	Section = ADDRESSES;
    *OldName = 0;
	long lNumBytesRead = 0;
	HRESULT hrGetLine = in.GetLine(buf, sizeof(buf), &lNumBytesRead); 
	while (SUCCEEDED(hrGetLine) && (lNumBytesRead > 0))
	{                        
		char* ptr = strrchr(buf, '>');
		if (ptr)
			*ptr = 0;
		else
		{
			ErrorDialog(IDS_ERR_NNDBASE_REBUILD);
			return (FALSE);
		}

		// If we have the same tag field again, just go on.  If not, store the new tag field
		// for reference and if we had an item, put it in the list.
		if (stricmp(OldName, buf + 1))
		{
			strcpy(OldName, buf + 1);
			nn = CreateNewNickname(OldName);
			Section = ADDRESSES;
		}
		else if (Section == ADDRESSES)
		{
			ErrorDialog(IDS_ERR_NNDBASE_REBUILD);
			return (FALSE);
		}

		CString Text;
		while (SUCCEEDED(hrGetLine = in.GetLine(buf, sizeof(buf), &lNumBytesRead)) && (lNumBytesRead > 0))
		{
			if (*buf == '<')
				break;
			if (!Text.IsEmpty())
				Text += (Section == ADDRESSES? "," : "\r\n");
			Text += (*buf == '>'? buf + 1 : buf);
		}
		
		if (FAILED(hrGetLine))
			break;
			
		if (Section == ADDRESSES)
		{
			nn->SetAddresses(Text);
			nn->SetAddressesLength(Text.GetLength());
			Section = NOTES;
		}
		else
		{
			nn->SetRawNotes(Text);
			nn->SetRawNotesLength(Text.GetLength());
			Section = ADDRESSES;
		}
	}

	if (FAILED(hrGetLine))
		return (FALSE);

	// Nickname files always get stored as .mailrc format, so write that format,
	// but first make a backup of the original
	strcpy(buf, m_Filename);
	::SetFileExtensionMT(buf, "bak");
	in.Rename(buf);

	return (WriteTxt());
}

BOOL CNicknameFile::ReadMailrc(JJFile& in)
{
	CRString AddressesKeyword(IDS_NICK_ALIAS);
	CRString NotesKeyword(IDS_NICK_NOTE);
	int AddressesLen = AddressesKeyword.GetLength();
	int NotesLen = NotesKeyword.GetLength();
	char buf[4096];
	CNickname* nn;
	POSITION pos;
	long lOffset;
    int Section;
	char* Name;
	char* Text;
	short BufOverflow = 0;

	// Initialize the lengths and offsets of all existing nicknames.
	// This will only happen when the toc is being rebuilt due to corruption.
	for (pos = GetHeadPosition(); pos;)
	{
		if (nn = GetNext(pos))
		{
			nn->SetAddressesOffset(-1);
			nn->SetNotesOffset(-1);
			nn->SetAddressesLength(-1);
			nn->SetRawNotesLength(-1);
		}
	}

	// BufOverflow gets set when there is more the 4096 of data behind a label
	BufOverflow = 0;
	in.Tell(&lOffset);
	ASSERT(lOffset >= 0);
	HRESULT hrGetLine = S_OK;
	long lNumBytesRead = 0;
	while (SUCCEEDED(hrGetLine = in.GetLine(buf, sizeof(buf), &lNumBytesRead)) && (lNumBytesRead > 0))
	{
		if (!strnicmp(AddressesKeyword, buf, AddressesLen))
		{
			BufOverflow = 0;
			Section = ADDRESSES;
			Name = buf + AddressesLen;
		}
		else if (!strnicmp(NotesKeyword, buf, NotesLen))
		{
			BufOverflow = 0;
			Section = NOTES;
			Name = buf + NotesLen;
		}
		else if (!BufOverflow)
		{	
			in.Tell(&lOffset);
			ASSERT(lOffset >= 0);
			continue;
		}

		while (*Name == ' ' || *Name == '\t')
			Name++;
		if (*Name)
		{
			// if the last block was bigger than our 4096 buffer, we're not expecting
			// label
			if (!BufOverflow)
			{
				// If name has spaces in it, it is surrounded by quotes
				Text = Name;
				if (*Text == '"')
				{
					Text++;
					Name++;
					while (*Text && *Text != '"')
						Text++;
				}
				else
				{
					while (*Text && *Text != ' ' && *Text != '\t')
						Text++;
				}
				if (*Text)
					*Text++ = 0;
				if (!(nn = FindNickname(Name)))
					nn = CreateNewNickname(Name);
			}
			while (*Text == ' ' || *Text == '\t')
				Text++;
			if (Section == ADDRESSES)
			{
				if (!BufOverflow)
				{
					nn->SetAddressesOffset(lOffset + (Text - buf));
					nn->SetAddressesLength(strlen(Text));
				}
				else
					nn->SetAddressesLength(nn->GetAddressesLength() + strlen(Text));
			}
			else
			{
				nn->SetNotesOffset(lOffset + (Text - buf));
				// Conversion of <Ctrl+C>s to <CR><LF>s adds a character
				int Count = 0;
				for (; *Text; Text++, Count++)
				{
					if (*Text == NotesNewlineChar)
						Count++;
				}
				nn->SetRawNotesLength(Count);
			}
		}
		in.Tell(&lOffset);
		ASSERT(lOffset >= 0);
		if (lNumBytesRead == 4096)
			BufOverflow = 1;
		else
			BufOverflow = 0;
	}
	
	if (FAILED(hrGetLine))
		return (FALSE);

	return (TRUE);
}

BOOL CNicknameFile::WriteToc()
{
	char Filename[_MAX_PATH + 1];
	JJFile out(JJFileMT::BUF_SIZE, FALSE);

	// Get filename of Toc
	strcpy(Filename, m_Filename);
	::SetFileExtensionMT(Filename, CRString(IDS_NICK_TOC_EXTENSION));

	if (FAILED(out.Open(Filename, O_WRONLY | O_CREAT | O_TRUNC)))
	{
		// If we fail to open the file because the directory is readonly,
		// then return TRUE because we've got all the data in memory, so
		// it's ok that it doesn't get saved
		return (TRUE);
	}

	out.DisplayErrors(TRUE);
		
	if (FAILED(out.Put((short)TocVersion)))
		return (FALSE);

	POSITION pos = GetHeadPosition();
	while (pos)
	{
		CNickname* nn = GetNext(pos);
		
		if (!nn ||
			FAILED(out.PutLine(nn->GetName())) ||
			FAILED(out.Put(nn->GetAddressesOffset())) ||
			FAILED(out.Put(nn->GetAddressesLength())) ||
			FAILED(out.Put(nn->GetNotesOffset())) ||
			FAILED(out.Put(nn->GetRawNotesLength())))
		{
			return (FALSE);
		}
	}
	
	SetModified(FALSE);

	return (TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// Refresh [public]
//
// The file has possibly changed (Peanut changed the file) so unload
// the current contents and reload it
void CNicknameFile::Refresh()
{
	BOOL bModified = IsModified();
	RemoveAll();
	RebuildToc();
	ReadToc();
	g_Nicknames->SetModifiedFlag(bModified);
}


CNicknamesDoc* g_Nicknames = NULL;

/////////////////////////////////////////////////////////////////////////////
// CNicknamesDoc

IMPLEMENT_DYNCREATE(CNicknamesDoc, CDoc)

CNicknamesDoc::CNicknamesDoc() :
	m_currentPos(NULL), m_bSavedFeatureMode(false)
{
	// If the user hits No to the save changes dialog, we want the nicknames to be deleted
	// so that the old ones can be read up from disk.  The problem is that the low level
	// document handles the save changes dialog, so CNicknamesDoc doesn't know if the No button
	// was selected in the dialog, so the best assumption is just to get rid of everything.
//	m_bAutoDelete = FALSE;
	
	SetTitle(CRString(IDR_NICKNAMES));

	// Need to set the filename to something because otherwise MFC will give the
	// Save As file dialog when saving 
	m_strPathName = m_strTitle;
	g_theRecipientDirector.Register( this );
}

CNicknamesDoc::~CNicknamesDoc()
{
	NukeAllData();

	g_theRecipientDirector.UnRegister( this );
}


////////////////////////////////////////////////////////////////////////
// NukeAllData [protected]
//
// She does what she says...
////////////////////////////////////////////////////////////////////////
void CNicknamesDoc::NukeAllData()
{
	while (! m_NicknameFiles.IsEmpty())
	{
		CNicknameFile* pNickfile = (CNicknameFile *) m_NicknameFiles.RemoveTail();
		ASSERT(pNickfile != NULL);
		ASSERT_KINDOF(CNicknameFile, pNickfile);
		delete pNickfile;
	}
}


////////////////////////////////////////////////////////////////////////
// CanCloseFrame [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	// Give a chance to move info from controls to filter
	if (pFrame)
	{
		CView* pView = pFrame->GetActiveView();
		if (pView)
			pView->UpdateData();
		else
		{
			ASSERT(0);
		}
	}

	//
	// Don't call the base class implementation here since it puts up
	// the default Yes/No/Cancel "do ya wanna save" UI.  Now that the
	// filters are Wazooed, we must do our own UI since only Yes/No
	// are applicable.
	//
//	return (CDoc::CanCloseFrame(pFrame));
	if (! IsModified())
		return TRUE;        // ok to continue

	CString strPrompt;
	AfxFormatString1(strPrompt, AFX_IDP_ASK_TO_SAVE, m_strTitle);
	switch (AfxMessageBox(strPrompt, MB_YESNO, AFX_IDP_ASK_TO_SAVE))
	{
	case IDYES:
		// Do the save-to-disk thing...
		OnSaveDocument(NULL);
		break;
	case IDNO:
		// Reload original data from disk
		NukeAllData();
		ReadTocs();
		UpdateAllViews(NULL, NICKNAME_HINT_REFRESH_LHS, NULL);
		break;
	default:
		ASSERT(0);		// Hmm.  Should never get here?
		break;
	}

	//
	// Always return TRUE since our Wazooed views are not housed in
	// traditional MDI child windows anymore.
	//
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnCloseDocument [public, virtual]
//
// The standard base implementation ends up calling CDocument::OnCloseDocument().
// This is really bad for this since it destroys the "parent frame", which
// could be main frame itself in the case of a docked filters view.
//
// So, instead, all we need to do is to divorce ourselves from all of
// our views and optionally destroy ourselves.  The Wazoo windows take 
// care of destroying the views.
////////////////////////////////////////////////////////////////////////
void CNicknamesDoc::OnCloseDocument()
{
	//
	// WARNING:  Do NOT call the base class OnCloseDocument()
	// implementation since it will destroy the frame associated with
	// each view!
	//
	BOOL bAutoDelete = m_bAutoDelete;
	m_bAutoDelete = FALSE;  // prevents destruction of document while closing views

	// manually divorce ourselves from our views...
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		RemoveView(pView);
	}
	m_bAutoDelete = bAutoDelete;

	// clean up contents of document before destroying the document itself
	DeleteContents();

	// delete the document if necessary
	if (m_bAutoDelete)
	{
		delete this;
		g_Nicknames = NULL;
	}
}


////////////////////////////////////////////////////////////////////////
// OnSaveDocument [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesDoc::OnSaveDocument(const char* pszPathName)
{
	POSITION viewpos = GetFirstViewPosition();
	CView* pView = (viewpos? GetNextView(viewpos) : NULL);
	BOOL bFoundModified = FALSE;
	
	// Make sure view has updated all the fields
	if (pView)
	{
		pView->UpdateData(TRUE);
		UpdateAllViews(NULL, NICKNAME_HINT_REQUESTING_NICKNAME, NULL);
	}
	
	POSITION listpos = m_NicknameFiles.GetHeadPosition();
	while (listpos != NULL)
	{
		CNicknameFile* pNickfile = (CNicknameFile *) m_NicknameFiles.GetNext(listpos);
		ASSERT(pNickfile != NULL);
		ASSERT_KINDOF(CNicknameFile, pNickfile);
		if (pNickfile->IsModified())
		{
			if (!pNickfile->WriteTxt() || !pNickfile->WriteToc())
				return FALSE;
			bFoundModified = TRUE;
		}
	}
	
	// Did the document get set as modified when none of the nickname files did?
	ASSERT(bFoundModified);
	
	SetModifiedFlag(FALSE);
	
	return TRUE;
}


BOOL CNicknamesDoc::ReadTocDir(const char* Dir)
{
	char Filename[_MAX_PATH + 1];
	int PathLen;

	if (!::FileExistsMT(Dir))
	{
		mkdir(Dir);
		return (FALSE);
	}
		
	strcpy(Filename, Dir);
	strcat(Filename, SLASHSTR);
	PathLen = strlen(Filename);

	// Add the Standard Nicknames
	WIN32_FIND_DATA	Find;
	HANDLE			FindHandle;
	wsprintf(Filename + PathLen, "*%s", (const char*)CRString(IDS_NICK_FILE_EXTENSION));
	Find.dwFileAttributes = _A_NORMAL;

	set<CString> NickFilesList;
	FindHandle = FindFirstFile(Filename,&Find);
	if (FindHandle != INVALID_HANDLE_VALUE)
	{
		int Result = 1;
		while (Result)
		{
			strcpy(Filename + PathLen, Find.cFileName);
			NickFilesList.insert(Filename);
			Result = FindNextFile(FindHandle,&Find);
		}
		FindClose(FindHandle);
	}

	for (set<CString>::iterator i = NickFilesList.begin(); i != NickFilesList.end(); i++)
		AddNicknameFile(*i);
		
	strcpy(Filename, Dir);
	strcat(Filename, SLASHSTR);
	PathLen = strlen(Filename);
	
	
	// Add any .pre Nicknames (that peanut plugins added)
	wsprintf(Filename + PathLen, "*%s", (const char*)CRString(IDS_FILTER_PRE_EXT));
	Find.dwFileAttributes = _A_NORMAL;

	FindHandle = FindFirstFile(Filename, &Find);
	if (FindHandle != INVALID_HANDLE_VALUE)
	{
		int Result = 1;
		while (Result)
		{
			strcpy(Filename + PathLen, Find.cFileName);
			AddNicknameFile(Filename, TRUE);
			Result = FindNextFile(FindHandle,&Find);
		}
		FindClose(FindHandle);
	}
	
	return (TRUE);
}

BOOL CNicknamesDoc::ReadTocs()
{
	char Filename[_MAX_PATH + 1];
	int PathLen;
	CNicknameFile* NickFile;
	
	// Read in main nickname file first
	strcpy(Filename, EudoraDir);
	PathLen = strlen(Filename);
	strcpy(Filename + PathLen, CRString(IDS_NICK_DBASE_NAME));
	
	if (!(NickFile = new CNicknameFile(Filename)))
		return (FALSE);

	ASSERT(m_NicknameFiles.IsEmpty());
	NickFile->m_Name = CRString(IDS_MAIN_NICK_FILE_NAME);

	//
	// WARNING! WARNING!  The CObList::AddTail() method is
	// overloaded to accept CObList ptrs as well as CObject
	// ptrs, with decidely different behaviors.  Therefore,
	// we need to cast the CObList to a CObject to force
	// the behavior we want.
	//
	m_NicknameFiles.AddTail((CObject *) NickFile);
	if (::FileExistsMT(Filename))
		NickFile->ReadToc();
	
	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		// Add files from Nickname directory off of mail directory
		char ExtraDirs[_MAX_PATH + 1];
		char NicknameDir[_MAX_PATH + 1];
		strcpy(Filename + PathLen, CRString(IDS_NICK_DIR_NAME));
		strcpy(NicknameDir, Filename);
		ReadTocDir(NicknameDir);
		
		// Add files for extra directories specified
		GetIniString(IDS_INI_EXTRA_NICKNAME_DIRS, ExtraDirs, sizeof(ExtraDirs));
		char* Start = ExtraDirs;
		while (*Start)
		{
			char* Next = strchr(Start, ';');
			if (Next)
				*Next++ = 0;
			else
				Next = Start + strlen(Start);
				
			::TrimWhitespaceMT(Start);
			if (stricmp(NicknameDir, Start))
				ReadTocDir(Start);
				
			Start = Next;
		}

		m_bSavedFeatureMode = true;
	}
	else
	{
		m_bSavedFeatureMode = false;
	}

	SetModifiedFlag(FALSE);
	
	return (TRUE);
}


////////////////////////////////////////////////////////////////////////
// GetFirstNicknameFile [public]
//
// Returns a pointer to the first CNicknameFile object in the internal
// collection, else NULL if the collection is empty.
////////////////////////////////////////////////////////////////////////
CNicknameFile* CNicknamesDoc::GetFirstNicknameFile()
{
	m_currentPos = m_NicknameFiles.GetHeadPosition();
	return GetNextNicknameFile();
}


////////////////////////////////////////////////////////////////////////
// GetNextNicknameFile [public]
//
// Returns a pointer to the next CNicknameFile object in the internal
// collection, else NULL if there are no more items.
////////////////////////////////////////////////////////////////////////
CNicknameFile* CNicknamesDoc::GetNextNicknameFile()
{
	if (m_currentPos != NULL)
	{
		CNicknameFile* pNickfile = (CNicknameFile *) m_NicknameFiles.GetNext(m_currentPos);
		ASSERT(pNickfile != NULL);
		ASSERT(pNickfile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));
		return pNickfile;
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// GetNumWritableFiles [public]
//
// Returns the number of writable nickname files in the internal
// collection.
////////////////////////////////////////////////////////////////////////
int CNicknamesDoc::GetNumWritableFiles() const
{
	int num_writable_files = 0;
	POSITION pos = m_NicknameFiles.GetHeadPosition();
	while (pos)
	{
		CNicknameFile* pNickfile = (CNicknameFile *) m_NicknameFiles.GetNext(pos);
		ASSERT(pNickfile != NULL);
		ASSERT(pNickfile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));

		if (! pNickfile->m_ReadOnly)
			num_writable_files++;
	}

	return num_writable_files;
}


////////////////////////////////////////////////////////////////////////
// AddNicknameFile [public]
//
// Given a fully-qualified pathname to a Nicknames file, create a
// corresponding CNicknameFile object and add it to the collection of
// CNicknameFile objects maintained by this document.  Returns a pointer
// to the new CNicknameFile object if successful, else returns NULL
// on error.
////////////////////////////////////////////////////////////////////////
CNicknameFile* CNicknamesDoc::AddNicknameFile(const CString& fileName, BOOL bPrepend /*= FALSE*/)
{
	CNicknameFile* pNickfile;
	if (pNickfile = new CNicknameFile(fileName))
	{
		CFileStatus status;
		CFile::GetStatus(fileName, status);

		if (status.m_attribute & CFile::readOnly)
			pNickfile->m_ReadOnly = TRUE;
		if (pNickfile->ReadToc())
		{
			//
			// WARNING! WARNING!  The CObList::AddTail() method is
			// overloaded to accept CObList ptrs as well as CObject
			// ptrs, with decidely different behaviors.  Therefore,
			// we need to cast the CObList to a CObject to force
			// the behavior we want.
			//
			// FORNOW, adding new nickname files to the *end* of the
			// nickname file list.  Really, this should be inserted
			// after the last file in the Eudora nicknames directory.
			//

			if (bPrepend) 
				m_NicknameFiles.AddHead((CObject *) pNickfile);
			else
				m_NicknameFiles.AddTail((CObject *) pNickfile);
			return pNickfile;
		}
			
		delete pNickfile;
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////
// UpdateNicknameFile [public]
//
// Given a fully-qualified pathname to a Nicknames file, see if it is 
// already loaded, if so, reload it, otherwise, just load it up - this
// occurs when a plugin (Peanut specifically) adds a nickname file
////////////////////////////////////////////////////////////////////////
void CNicknamesDoc::UpdateNicknameFile(const CString& fileName)
{
	// Only update the 'pre' files, do not mess with the files .txt
	// that are in the view!
	if (fileName.Right(3).CompareNoCase(CRString(IDS_FILTER_PRE_EXT)) == 0)
	{	
		BOOL got_it = FALSE;
		CNicknameFile* pNNFile = GetFirstNicknameFile();
		
		while(!got_it && pNNFile)
		{
			if (pNNFile->m_Filename.CompareNoCase(fileName) == 0)
			{
				pNNFile->Refresh();
				got_it = TRUE;
			}
			else
				pNNFile =  GetNextNicknameFile();
		}
		if (!got_it)
		{
			AddNicknameFile(fileName, TRUE);
		}
	}
}

////////////////////////////////////////////////////////////////////////
// RemoveNicknameFile [public]
//
// Given a CNicknameFile object, find it in the file array and blow it
// away.  Return TRUE if successful.
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesDoc::RemoveNicknameFile(CNicknameFile* pNicknameFile)
{
	ASSERT(pNicknameFile != NULL);
	POSITION pos = m_NicknameFiles.GetHeadPosition();
	while (pos != NULL)
	{
		CNicknameFile* pNickfile = (CNicknameFile *) m_NicknameFiles.GetAt(pos);
		if (pNicknameFile == pNickfile)
		{
			//
			// Deleting a CNicknameFile object automatically
			// deletes all contained CNickname objects.
			//
			m_NicknameFiles.RemoveAt(pos);
			delete pNickfile;
			return TRUE;
		}
		m_NicknameFiles.GetNext(pos);
	}

	return FALSE;
}


/*FORNOW, this is probably obsolete
CNicknameFile* CNicknamesDoc::GetNicknameFile(int Index)
{
	int Count = m_NicknameFiles.GetSize();
	int EndIndex = 0;
	
	for (int i = 0; i < Count; i++)
	{
		CNicknameFile* NickFile = (CNicknameFile*)m_NicknameFiles[i];

		if (NickFile)
		{
			EndIndex += NickFile->GetCount();
			if (Index < EndIndex)
				return (NickFile);
		}
	}
	
	ASSERT(FALSE);
	
	return (NULL);
}
FORNOW*/


CNickname* CNicknamesDoc::Find(const char* Name, int Start /*= 0*/, BOOL OneFileOnly /*= FALSE*/)
{
	// If the place to start looking is not the beginning, then we should
	// always be looking in just the one nickname file
	ASSERT(Start == 0 || OneFileOnly);
	
	CNickname* nn;
	int EndIndex = 0;

	POSITION pos = m_NicknameFiles.GetHeadPosition();
	while (pos != NULL)
	{
		CNicknameFile* NickFile = (CNicknameFile*) m_NicknameFiles.GetNext(pos);
		ASSERT(NickFile);
		EndIndex += NickFile->GetCount();
		if (Start < EndIndex)
		{
			if (nn = NickFile->FindNickname(Name))
				return (nn);
			if (OneFileOnly)
				return (NULL);
		}
	}
	
	return (NULL);
}

CNickname* CNicknamesDoc::ReadNickname(const char* Name, int Start /*= 0*/, BOOL OneFileOnly /*= FALSE*/)
{
	// If the place to start looking is not the beginning, then we should
	// always be looking in just the one nickname file
	ASSERT(Start == 0 || OneFileOnly);
	
	CNickname* nn;
	int EndIndex = 0;
	
	POSITION pos = m_NicknameFiles.GetHeadPosition();

	char *floater;
	if (floater = strstr(Name, "::"))	// We're dealing with the special case of a nickname file being specified in the nickname
	{									// Under normal circumstances you'll only see this when the nickname is ambiguous.
		char *pNNFile = NULL;
		char *pTheNickname = NULL;
		char *temp;
		temp = new char[strlen(Name)+1];
		strcpy(temp, Name);
		floater = strstr(temp, "::");
		if (!floater)		// How???
			return NULL;

		*floater = 0;

		pNNFile = new char[strlen(temp)+1];
		strcpy(pNNFile, temp);

		floater += 2;
		pTheNickname = new char[strlen(floater)+1];
		strcpy(pTheNickname, floater);

		delete [] temp;

		while (pos != NULL)
		{
			CNicknameFile* NickFile = (CNicknameFile*) m_NicknameFiles.GetNext(pos);
			ASSERT(NickFile);
			EndIndex += NickFile->GetCount();
			if (Start < EndIndex)
			{
				if (!strcmp(pNNFile, NickFile->GetName()))
				{
					if (nn = NickFile->FindNickname(pTheNickname))
					{
						delete [] pTheNickname;
						delete [] pNNFile;
						if (nn->NeedsReadIn() && NickFile->ReadNickname(nn) == FALSE)
							return (NULL);
						return (nn);
					}
					if (OneFileOnly)
					{
						delete [] pTheNickname;
						delete [] pNNFile;
						return (NULL);
					}
				}
			}
		}
	}
	else		// It's just your normal resolve-it-to-the-first-one-you-find kind of nickname
	{
		while (pos != NULL)
		{
			CNicknameFile* NickFile = (CNicknameFile*) m_NicknameFiles.GetNext(pos);
			ASSERT(NickFile);
			EndIndex += NickFile->GetCount();
			if (Start < EndIndex)
			{
				if (nn = NickFile->FindNickname(Name))
				{
					if (nn->NeedsReadIn() && NickFile->ReadNickname(nn) == FALSE)
						return (NULL);
					return (nn);
				}
				if (OneFileOnly)
					return (NULL);
			}
		}
	}
	
	return (NULL);
}

/*FORNOW -- appears to be obsolete
void CNicknamesDoc::RemoveAt(int Index)
{
	int Count = m_NicknameFiles.GetSize();
	int StartIndex = 0;
	int EndIndex = 0;
	
	for (int i = 0; i < Count; i++)
	{
		CNicknameFile* NickFile = (CNicknameFile*)m_NicknameFiles[i];
		
		if (NickFile)
		{
			EndIndex += NickFile->GetCount();
			if (Index < EndIndex)
			{
				POSITION pos = NickFile->FindIndex(Index - StartIndex);
				if (!pos)
					ASSERT(FALSE);
				else
				{
					NickFile->RemoveAt(pos);
					NickFile->SetModified(TRUE);
				}
				return;
			}
			StartIndex += NickFile->GetCount();
		}
	}
}
FORNOW*/

/*FORNOW
CNickname* CNicknamesDoc::Add(CNicknameFile* pNicknameFile, const char* Name)
{
	CNicknameFile* NickFile = (CNicknameFile*)m_NicknameFiles[FileIndex];
			
	return (NickFile? NickFile->Add(Name) : NULL);
}
FORNOW*/


/*FORNOW
CNicknamesView* CNicknamesDoc::GetView()
{
	POSITION pos = GetFirstViewPosition();
	if (pos)
		return ((CNicknamesView*)GetNextView(pos));
		
	return (NULL);
}
FORNOW*/


////////////////////////////////////////////////////////////////////////
//
// CNicknamesDoc::GetNicknames [public]
//
// Walks the list of all nicknames, and places a comma-separated list
// of nicknames (Names only) into the caller-provided string object.
// Returns TRUE if successful, else FALSE on error.  Note that it is
// valid to return a TRUE status along with an empty list of
// nicknames.
//
// The format of the returned nickname list is
//
//    Nickname1,Nickname2,Nickname3,...
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesDoc::GetNicknames(CString& nickNames)	//(o) returned list of comma-separated nickname names
{
	OutputDebugString("CNicknamesDoc::GetNicknames()\n");
	nickNames.Empty();
	
	POSITION filepos = m_NicknameFiles.GetHeadPosition();
	while (filepos != NULL)
	{
		CNicknameFile* pNickfile = (CNicknameFile *) m_NicknameFiles.GetNext(filepos);
		ASSERT(pNickfile);

		POSITION pos = pNickfile->GetHeadPosition();
		while (pos != NULL)
		{
			CNickname* p_nickname = pNickfile->GetNext(pos);
			if (NULL == p_nickname)
			{
				ASSERT(0);
				return FALSE;
			}

			//
			// Okay, got a nickname, so pray that it is not empty and
			// doesn't contain any embedded commas then blindly add it
			// to the list.
			//
			ASSERT(! p_nickname->GetName().IsEmpty());
			ASSERT(p_nickname->GetName().Find(',') == -1);
			if (! nickNames.IsEmpty())
				nickNames += ",";
			nickNames += p_nickname->GetName();
		}
	}
	
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// RegisterNicknameFieldName [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesDoc::RegisterNicknameFieldName(const CString& fieldName)
{
	ASSERT(! fieldName.IsEmpty());

	if (! IsNicknameFieldName(fieldName))
		m_NicknameFieldNameList.AddTail(fieldName);
}


////////////////////////////////////////////////////////////////////////
// IsNicknameFieldName [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesDoc::IsNicknameFieldName(const CString& fieldName)
{
	ASSERT(! fieldName.IsEmpty());

	POSITION pos;
	pos = m_NicknameFieldNameList.GetHeadPosition();
	while (pos != NULL)
	{
		CString fieldname = m_NicknameFieldNameList.GetNext(pos);
		if (fieldname.CompareNoCase(fieldName) == 0)
			return TRUE;
	}

	return FALSE;
}


BEGIN_MESSAGE_MAP(CNicknamesDoc, CDoc)
	//{{AFX_MSG_MAP(CNicknamesDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNicknamesDoc commands


/////////////////////////////////////////////////////////////////////////////

//FORNOWBOOL LoadNicknames()
//FORNOW{
//FORNOW	if (!g_Nicknames)
//FORNOW	{
//FORNOW		g_Nicknames = (CNicknamesDoc*)NewChildDocument(NicknamesTemplate);
//FORNOW		if (!g_Nicknames->ReadTocs())
//FORNOW		{
//FORNOW			g_Nicknames->OnCloseDocument();
//FORNOW			return (FALSE);
//FORNOW		}
//FORNOW	}
//FORNOW	
//FORNOW	return (TRUE);
//FORNOW}

char* ExpandAliases(const char* pAddresses, 
					BOOL domainQualify /*= TRUE*/,
					BOOL StripFCC /*= FALSE*/,
					BOOL bElaborateNicknames /*=TRUE*/,
					BOOL bDisplayError /*= FALSE*/)
{
	if (!pAddresses || !g_Nicknames)
		return (NULL);

	//
	// Setup initial return buffer.
	//
	int BufAvail = ::SafeStrlenMT(pAddresses) + 1;
	char* pExpandedAddrs = new char[BufAvail];
	*pExpandedAddrs = 0;		// start with an empty expanded alias string

	const char* pStart = pAddresses;
	const int BUFFERFUDGE = 50;
	BOOL Success = FALSE;

	while (1)
	{
		if (!*pStart)
		{
			Success = TRUE;
			break;
		}
		
		//
		// Find the end of the address.  Commas and newlines are
		// considered address separators.
		//
		const char* pEnd = ::FindAddressEnd(pStart, bDisplayError);
		if (NULL == pEnd)
			break;

		// Get an address
		char CurrentAddr[256];
		int Len = pEnd - pStart;
		if (Len > sizeof(CurrentAddr) - 1)
		{
			if (bDisplayError)
				ErrorDialog(IDS_ERR_ADDRESS_TOO_LONG, pStart);
			break;
		}

		strncpy(CurrentAddr, pStart, Len);
		CurrentAddr[Len] = 0;
		::TrimWhitespaceMT(CurrentAddr);

		// Seek to the start of the next address
		for (pStart = pEnd; *pStart == ',' || *pStart == '\r' || *pStart == '\n'; pStart++)
			;

		if (*pExpandedAddrs)
		{
			strcat(pExpandedAddrs, ", ");
			BufAvail -= 2;
		}

		CNickname* pNickname = g_Nicknames->ReadNickname(CurrentAddr);
		if (pNickname != NULL)
		{
			//
			// The current address is a nickname, so recursively expand it.
			//
			if (pNickname->IsVisited())
			{
				if (bDisplayError)
					ErrorDialog(IDS_ERR_RCPT_CIRCULAR_REF, (const char*) pNickname->GetName());
				break;
			}
			pNickname->SetVisited(TRUE);
			char* pExpandedNickname = NULL;
			if (bElaborateNicknames)
			{
				pExpandedNickname = ExpandAliases(pNickname->GetElaboratedAddresses(), domainQualify,
					StripFCC, bElaborateNicknames, bDisplayError);
			}
			else
			{
				pExpandedNickname = ExpandAliases(pNickname->GetAddresses(), domainQualify,
					StripFCC, bElaborateNicknames, bDisplayError);
			}
			pNickname->SetVisited(FALSE);
			if (NULL == pExpandedNickname)
				break;
			Len = strlen(pExpandedNickname);

			if ((BufAvail - Len - 5) < 0)
			{
				char* p_temp = new char[Len + ::SafeStrlenMT(pExpandedAddrs) + BUFFERFUDGE];
				BufAvail = BUFFERFUDGE + Len;
				strcpy(p_temp, pExpandedAddrs);
				delete [] pExpandedAddrs;
				pExpandedAddrs = p_temp;
			}

			char* p_src;
			char* p_dst;
// Macro to trim off trailing spaces and commas
#define TRIM_TRAILING_SEPS while (p_dst>pExpandedAddrs && (p_dst[-1]==' '||p_dst[-1]==',')) {p_dst--; Len--;}

			for (p_src = pExpandedNickname, p_dst = pExpandedAddrs + strlen(pExpandedAddrs); *p_src; p_src++)
			{
				if (*p_src == '\r')
					Len--;
				else if (*p_src == '\n')
				{
					if (p_dst == pExpandedAddrs || p_dst[-1] == ',')
						Len--;
					else
					{
						*p_dst++ = ',';
						*p_dst++ = ' ';
						Len++;	// We added a char to the total length here
					}
				}
				else
				{
					if (*p_src == ';') TRIM_TRAILING_SEPS; // trim before semicolon
					*p_dst++ = *p_src;
				}
			}

			TRIM_TRAILING_SEPS; // trim when done

#undef TRIM_TRAILING_SEPS

			*p_dst = 0;
			BufAvail -= Len;
			delete [] pExpandedNickname;
		}
		else
		{
			if (!StripFCC || (unsigned char)*CurrentAddr != 131)
			{
				Len = ::SafeStrlenMT(CurrentAddr);
				if ((BufAvail - Len - 5) < 0)
				{
					char* p_temp = new char[Len +	::SafeStrlenMT(pExpandedAddrs) + BUFFERFUDGE];
					BufAvail = BUFFERFUDGE + Len;
					strcpy(p_temp, pExpandedAddrs);
					delete [] pExpandedAddrs;
					pExpandedAddrs = p_temp;
				}
				strcat(pExpandedAddrs, CurrentAddr);
				BufAvail -= Len;
			}
		}
	}

	if (Success)
	{
		// Qualify domain names if necessary
		if (domainQualify)
			pExpandedAddrs = QualifyAddresses(pExpandedAddrs);
	}
	else
	{
		delete [] pExpandedAddrs;
		pExpandedAddrs = NULL;
	}
	
	return (pExpandedAddrs);
}
/*
void FinishNNAutoComplete(CWnd* pWnd)
{
	if (!pWnd)
	{
		ASSERT(0);
		return;
	}

	//
	// Determine whether we're dealing with a CEdit or CRichEditCtrl object.
	//
	CEdit* pEditCtrl = NULL;
	if (pWnd->IsKindOf(RUNTIME_CLASS(CEdit)))
		pEditCtrl = (CEdit *) pWnd;
	CRichEditCtrl* pRichEditCtrl = NULL;
	if (NULL == pEditCtrl)
	{
		if (pWnd->IsKindOf(RUNTIME_CLASS(CRichEditCtrl)) ||
			pWnd->IsKindOf(RUNTIME_CLASS(CRichEditView)))
		{
			pRichEditCtrl = (CRichEditCtrl *) pWnd;
		}
		else
		{
			ASSERT(0);
			return;
		}
	}

	ASSERT((pEditCtrl && !pRichEditCtrl) || (!pEditCtrl && pRichEditCtrl));

	int Index = 0;
	if (pEditCtrl)
		Index = pEditCtrl->LineFromChar();
	else
		Index = pRichEditCtrl->LineFromChar(-1);

	int LineOffset = 0;
	if (pEditCtrl)
		LineOffset = pEditCtrl->LineIndex();
	else
		LineOffset = pRichEditCtrl->LineIndex();

	char buf[256];
	char Partial[256];
	Partial[0] = 0;
	unsigned int Len;
	
	// Grab the line that the caret is in
	if (pEditCtrl)
	{
		if ((Len = pEditCtrl->GetLine(Index, buf, sizeof(buf)-1)) <= 0 || !*buf)
			return;
	}
	else
	{
		if ((Len = pRichEditCtrl->GetLine(Index, buf, sizeof(buf)-1)) <= 0 || !*buf)
			return;
	}

	buf[Len] = 0;

	long SelStart, SelEnd;
	if (pEditCtrl)
	{
		int nSelStart, nSelEnd;
		pEditCtrl->GetSel(nSelStart, nSelEnd);
		SelStart = nSelStart;
		SelEnd = nSelEnd;
	}
	else
		pRichEditCtrl->GetSel(SelStart, SelEnd);
	char* Start = buf + SelStart - LineOffset;

	// Find the beginning of the string if no text is selected
	if (SelStart == SelEnd)
	{
		while (Start > buf && Start[-1] != ',')
			Start--;
	}

	// Skip by leading space
	while (*Start == ' ')
		Start++;

	ASSERT(g_Nicknames != NULL);
	int NumFound = 0;
	Len = strlen(Start);
	BOOL keep_going = TRUE;		// flag to terminate all loops from inner loop

	// Here we want to first check the history list for a match then we go on to the addressbook. 
	char *Match = NULL;
//	Match = g_AutoCompleter->FindACStartingWith(Start);

	CStringList* Matches = g_AutoCompleter->FindListOfACStartingWith(Start);


	if (Matches->GetCount() != 0)
	{
		char * floater = Match;
		char * floater2 = Start;
		while (*floater == *floater2 && *floater2 != 0)
		{
			floater++;
			floater2++;
		}

		strncpy(Partial, floater, sizeof(Partial));
		keep_going = false;
	}

	for (CNicknameFile* NickFile = g_Nicknames->GetFirstNicknameFile();
	     keep_going && (NickFile != NULL);
		 NickFile = g_Nicknames->GetNextNicknameFile())
	{
		POSITION pos = NULL;
		
		while (keep_going && ((pos = NickFile->FindNicknameStartingWith(Start, pos)) != NULL))
		{
			CNickname* nn = NickFile->GetAt(pos);
			
			if (!nn)
				continue;
			
			const char* Name = nn->GetName();
			
			if (strlen(Name) > Len)
			{
				strcpy(Partial, Name + Len);
			}
			keep_going = false;
		}
	}

			if (pEditCtrl)
			{
				if (Partial[0])
				{
					pEditCtrl->ReplaceSel(Partial);
					pEditCtrl->SetSel((int)SelStart, (int)(strlen(Partial)+SelStart));
				}
			}
			else
				pRichEditCtrl->ReplaceSel(Partial);
}
*/

/*
void FinishNNAutoComplete(CWnd* pWnd)
{
	if (!pWnd)
	{
		ASSERT(0);
		return;
	}

	//
	// Determine whether we're dealing with a CEdit or CRichEditCtrl object.
	//
	CEdit* pEditCtrl = NULL;
	if (pWnd->IsKindOf(RUNTIME_CLASS(CEdit)))
		pEditCtrl = (CEdit *) pWnd;
	CRichEditCtrl* pRichEditCtrl = NULL;
	if (NULL == pEditCtrl)
	{
		if (pWnd->IsKindOf(RUNTIME_CLASS(CRichEditCtrl)) ||
			pWnd->IsKindOf(RUNTIME_CLASS(CRichEditView)))
		{
			pRichEditCtrl = (CRichEditCtrl *) pWnd;
		}
		else
		{
			ASSERT(0);
			return;
		}
	}

	ASSERT((pEditCtrl && !pRichEditCtrl) || (!pEditCtrl && pRichEditCtrl));

	int Index = 0;
	if (pEditCtrl)
		Index = pEditCtrl->LineFromChar();
	else
		Index = pRichEditCtrl->LineFromChar(-1);

	int LineOffset = 0;
	if (pEditCtrl)
		LineOffset = pEditCtrl->LineIndex();
	else
		LineOffset = pRichEditCtrl->LineIndex();

	char buf[256];
	char Partial[256];
	Partial[0] = 0;
	unsigned int Len;
	
	// Grab the line that the caret is in
	if (pEditCtrl)
	{
		if ((Len = pEditCtrl->GetLine(Index, buf, sizeof(buf)-1)) <= 0 || !*buf)
			return;
	}
	else
	{
		if ((Len = pRichEditCtrl->GetLine(Index, buf, sizeof(buf)-1)) <= 0 || !*buf)
			return;
	}

	buf[Len] = 0;

	long SelStart, SelEnd;
	if (pEditCtrl)
	{
		int nSelStart, nSelEnd;
		pEditCtrl->GetSel(nSelStart, nSelEnd);
		SelStart = nSelStart;
		SelEnd = nSelEnd;
	}
	else
		pRichEditCtrl->GetSel(SelStart, SelEnd);
	char* Start = buf + SelStart - LineOffset;

	// Find the beginning of the string if no text is selected
	if (SelStart == SelEnd)
	{
		while (Start > buf && Start[-1] != ',')
			Start--;
	}

	// Skip by leading space
	while (*Start == ' ')
		Start++;

	ASSERT(g_Nicknames != NULL);
	int NumFound = 0;
	Len = strlen(Start);
	BOOL keep_going = TRUE;		// flag to terminate all loops from inner loop

	// Here we want to first check the history list for a match then we go on to the addressbook. 
	char *Match = NULL;
	Match = g_AutoCompleter->FindACStartingWith(Start);
	if (Match)
	{
		char * floater = Match;
		char * floater2 = Start;
		while (*floater == *floater2 && *floater2 != 0)
		{
			floater++;
			floater2++;
		}

		strncpy(Partial, floater, sizeof(Partial));
		keep_going = false;
	}

	for (CNicknameFile* NickFile = g_Nicknames->GetFirstNicknameFile();
	     keep_going && (NickFile != NULL);
		 NickFile = g_Nicknames->GetNextNicknameFile())
	{
		POSITION pos = NULL;
		
		while (keep_going && ((pos = NickFile->FindNicknameStartingWith(Start, pos)) != NULL))
		{
			CNickname* nn = NickFile->GetAt(pos);
			
			if (!nn)
				continue;
			
			const char* Name = nn->GetName();
			
			if (strlen(Name) > Len)
			{
				strcpy(Partial, Name + Len);
			}
			keep_going = false;
		}
	}

			if (pEditCtrl)
			{
				if (Partial[0])
				{
					pEditCtrl->ReplaceSel(Partial);
					pEditCtrl->SetSel((int)SelStart, (int)(strlen(Partial)+SelStart));
				}
			}
			else
				pRichEditCtrl->ReplaceSel(Partial);
}
*/

void FinishNickname(CWnd* pWnd)
{
	if (!pWnd)
	{
		ASSERT(0);
		return;
	}

	//
	// Determine whether we're dealing with a CEdit or CRichEditCtrl object.
	//
	CEdit* pEditCtrl = NULL;
	if (pWnd->IsKindOf(RUNTIME_CLASS(CEdit)))
		pEditCtrl = (CEdit *) pWnd;
	CRichEditCtrl* pRichEditCtrl = NULL;
	if (NULL == pEditCtrl)
	{
		if (pWnd->IsKindOf(RUNTIME_CLASS(CRichEditCtrl)) ||
			pWnd->IsKindOf(RUNTIME_CLASS(CRichEditView)))
		{
			pRichEditCtrl = (CRichEditCtrl *) pWnd;
		}
		else
		{
			ASSERT(0);
			return;
		}
	}

	ASSERT((pEditCtrl && !pRichEditCtrl) || (!pEditCtrl && pRichEditCtrl));

	BOOL DoExpand = ShiftDown();
	int Index = 0;
	if (pEditCtrl)
		Index = pEditCtrl->LineFromChar();
	else
		Index = pRichEditCtrl->LineFromChar(-1);

	int LineOffset = 0;
	if (pEditCtrl)
		LineOffset = pEditCtrl->LineIndex();
	else
		LineOffset = pRichEditCtrl->LineIndex();

	char buf[256];
	char Partial[256];
	int Len;
	
	// Grab the line that the caret is in
	if (pEditCtrl)
	{
		if ((Len = pEditCtrl->GetLine(Index, buf, sizeof(buf))) <= 0 || !*buf)
			return;
	}
	else
	{
		if ((Len = pRichEditCtrl->GetLine(Index, buf, sizeof(buf))) <= 0 || !*buf)
			return;
	}

	long SelStart, SelEnd;
	if (pEditCtrl)
	{
		int nSelStart, nSelEnd;
		pEditCtrl->GetSel(nSelStart, nSelEnd);
		SelStart = nSelStart;
		SelEnd = nSelEnd;
	}
	else
		pRichEditCtrl->GetSel(SelStart, SelEnd);
	char* Start = buf + SelStart - LineOffset;

	buf[Len] = 0;
	buf[SelEnd - LineOffset] = 0;
	// Find the beginning of the string if no text is selected
	if (SelStart == SelEnd)
	{
		while (Start > buf && Start[-1] != ',')
			Start--;
	}

	// Skip by leading space
	while (*Start == ' ')
		Start++;

//FORNOW	if (!LoadNicknames())
//FORNOW		return;

	ASSERT(g_Nicknames != NULL);
	int NumFound = 0;
	Len = strlen(Start);
	BOOL keep_going = TRUE;		// flag to terminate all loops from inner loop
	for (CNicknameFile* NickFile = g_Nicknames->GetFirstNicknameFile();
	     keep_going && (NickFile != NULL);
		 NickFile = g_Nicknames->GetNextNicknameFile())
	{
		POSITION pos = NULL;
		
		while (keep_going && ((pos = NickFile->FindNicknameStartingWith(Start, pos)) != NULL))
		{
			CNickname* nn = NickFile->GetAt(pos);
			
			if (!nn)
				continue;
			
			const char* Name = nn->GetName();
			
			// If we haven't found one before, then just copy the whole string
			if (!NumFound++)
				strcpy(Partial, Name + Len);
			else
			{
				// We have found other matches, so find the longest matching prefix
				char* p = Partial;
				Name += Len;
				while (tolower(*p) == tolower(*Name))
				{
					p++;
					Name++;
				}
				*p = 0;
				if (p == Partial)
				{
					// If there's no commonality, then get all the way out
					keep_going = FALSE;
					break;
				}
			}
		}
	}

	if (NumFound)
	{
		if (*Partial || NumFound == 1)
		{
			char* Expand = NULL;
			DoExpand = DoExpand && (NumFound == 1);
			if (SelStart != SelEnd || DoExpand)
			{
				memmove(Partial + Len, Partial, strlen(Partial) + 1);
				strncpy(Partial, Start, Len);
				if (DoExpand)
				{
					Expand = ExpandAliases(Partial);
					if (SelStart == SelEnd)
					{
						if (pEditCtrl)
							pEditCtrl->SetSel(Start - buf + LineOffset, int(SelEnd));
						else
							pRichEditCtrl->SetSel(Start - buf + LineOffset, SelEnd);
					}
				}
			}
			if (pEditCtrl)
				pEditCtrl->ReplaceSel(DoExpand? Expand : Partial);
			else
				pRichEditCtrl->ReplaceSel(DoExpand? Expand : Partial);

			if (Expand)
				delete [] Expand;
		}
		if (NumFound > 1)
			MessageBeep(MB_ICONEXCLAMATION);
	}
}

////////////////////////////////////////////////////////////////////////
// ConvertEndOfLinesToCommas [extern]
//
// Convert CR, LF, or CR-LF sequences into commas so that they
// display better in the single-line portions of the UI.  FORNOW,
// there is no limit to the string length.
//
////////////////////////////////////////////////////////////////////////
void ConvertEndOfLinesToCommas(CString& itemText)
{
	ASSERT(itemText.FindOneOf("\r\n") != -1);

	char* p_buf = new char[itemText.GetLength() + 1];
	if (NULL == p_buf)
		return;			// oh, well ...

	int buf_idx = 0;
	for (int idx = 0; idx < itemText.GetLength(); idx++)
	{
		if ('\r' == itemText[idx])
		{
			if (itemText[idx+1] != '\0')
			{
				p_buf[buf_idx++] = ',';		// add only if not at end of string
				p_buf[buf_idx++] = ' ';
			}
			if ('\n' == itemText[idx+1])
				idx++;						// treat CR-LF sequence as one char
		}
		else if ('\n' == itemText[idx])
		{
			if (itemText[idx+1] != '\0')
			{
				p_buf[buf_idx++] = ',';		// add only if not at end of string
				p_buf[buf_idx++] = ' ';
			}
		}
		else
			p_buf[buf_idx++] = itemText[idx];
	}
	p_buf[buf_idx] = '\0';
	itemText = p_buf;
	delete [] p_buf;
}


void CNicknamesDoc::Notify( 
QCCommandObject*	pCommand,
COMMAND_ACTION_TYPE	theAction,
void*)
{
	CNickname*	pNickName;

	ASSERT( pCommand );

	if( ( pCommand->IsKindOf( RUNTIME_CLASS( QCRecipientCommand ) ) ) &&
		( theAction == CA_DELETE ) )
	{
		pNickName = Find( ( ( QCRecipientCommand* ) pCommand )->GetName() );
		
		if( pNickName != NULL )
		{
			pNickName->SetRecipient( FALSE );			
			UpdateAllViews( NULL, NICKNAME_HINT_RECIPLIST_CHANGED, NULL );
		}
	}
}


// Called when a change in feature mode has occurred. May
// be called multiple times by various views/owners.
void CNicknamesDoc::UpdateSharewareFeatureMode()
{
	// Shareware: Reduced feature mode only allows one nickname file
	const bool bNewMode = UsingFullFeatureSet();

	if (m_bSavedFeatureMode != bNewMode)
	{
		CanCloseFrame(NULL); // Ask them to save stuff

		NukeAllData();
		ReadTocs();
		UpdateAllViews(NULL, NICKNAME_HINT_REFRESH_LHS, NULL);

		m_bSavedFeatureMode = bNewMode;
	}
}

