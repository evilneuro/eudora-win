// AutoCompleteSearcher.cpp
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

#include <xstddef>
#pragma warning (disable: 4663 4244)
#include "AutoCompleteSearcher.h"
#pragma warning (default: 4663 4244)


#include "fileutil.h"
#include "rs.h"
#include "doc.h"
#include "nickdoc.h"
#include "resource.h"
#include "headervw.h"
#include "font.h"
#include "CompMessageFrame.h"
#include "address.h"
#include "guiutils.h"


#include "DebugNewHelpers.h"


IMPLEMENT_DYNAMIC( AutoCompCEdit, CEdit )

AutoCompNode::AutoCompNode(const char* data)
{
	m_Data = data;
	next = NULL;
}

AutoCompList::AutoCompList(const char *filename)
{
	m_Filename = filename;
	m_Head = NULL;
	m_size = 0;
	m_MaxSize = 0;
	m_bDirty = FALSE;
}

AutoCompList::~AutoCompList()
{
	AutoCompNode* reaper = m_Head;

	while (reaper != NULL)
	{
		AutoCompNode* placeholder = reaper->next;
		delete reaper;
		reaper = placeholder;
	}
}

bool AutoCompList::ReadAutoCompleteList()
{
	JJFile ThirdInput(4096, false);
	char Filename[MAX_PATH + 1];
	char pBuffer[256];

	strcpy(Filename, EudoraDir);
	strncat(Filename, m_Filename, MAX_PATH-(strlen(Filename)+1));

	// Consider no file to be okay.  It just means the list is empty.
	if (!FileExistsMT(Filename))
		return true;

	if (SUCCEEDED(ThirdInput.Open(Filename, O_RDONLY)))
	{
		HRESULT hr;
		while (SUCCEEDED(hr = ThirdInput.GetLine(pBuffer, sizeof(pBuffer) - 1)) && hr != S_FALSE)
			Add(pBuffer, true);

		// A return of S_FALSE (success, no data) means that we reached end-of-file
		if (hr == S_FALSE)
		{
			m_bDirty = FALSE;
			return true;
		}
	}

	return false;
}

void AutoCompList::SaveToDisk()
{
	if (m_bDirty)
	{
		JJFile OutFile;
		CString Filename;
		CString TempFilename;

		Filename = EudoraDir;
		Filename += m_Filename;
		TempFilename = GetTmpFileNameMT();

		if (SUCCEEDED(OutFile.Open(TempFilename, O_RDWR | O_CREAT | O_TRUNC)))
		{
			AutoCompNode* CurrNode = m_Head;

			while (CurrNode)
			{
				const char* Data = CurrNode->GetData();
				if (Data && *Data)
				{
					if (FAILED(OutFile.PutLine(Data)))
					{
						OutFile.Delete();
						return;
					}
				}
				CurrNode = CurrNode->next;
			}

			if (SUCCEEDED(OutFile.Rename(Filename)))
				m_bDirty = FALSE;
		}
	}
}

bool AutoCompList::Add(const char *Data, bool ToEnd /*= false*/, bool bAddOnlyEmailIDs /*= false*/)
{
	CString ToAdd(Data);
	ToAdd.TrimLeft();
	ToAdd.TrimRight();

	if (ToAdd.IsEmpty())
		return false;

	//add only email ids to the history list
	//do not add nicknames to the history list
	//hence check for the '@' character to make sure
	//that it is an email id we add to the list
	//made it same behavior as MAC Eudora
	if( bAddOnlyEmailIDs && (ToAdd.Find('@') == -1 ) )
		return false;

	int MaxSize = GetIniLong(IDS_INI_MAXIMUM_HISTORY_SIZE);
	if (MaxSize > 5000)			// Not too big
		MaxSize = 5000;
	else if (MaxSize < 15)		// Yet not too small a list
		MaxSize = 15;

	if (m_size > MaxSize && m_MaxSize != -1)
	{
		AutoCompNode* pNode = m_Head;
		int NewSize = MaxSize - (MaxSize / 10);

		for (int i = 0; i < NewSize; i++)
			pNode = pNode->next;

		AutoCompNode* leader = pNode->next;
		pNode->next = NULL;
		while (leader != NULL)
		{
			AutoCompNode* temp = leader->next;
			delete leader;
			leader = temp;
		}
		m_size = NewSize;
	}

	while (Remove(ToAdd) == true){}

	if (ToEnd == false)
	{
		AutoCompNode* CurrPlace = m_Head;

		m_Head = DEBUG_NEW AutoCompNode(ToAdd);
		m_Head->next = CurrPlace;
	}
	else
	{
		if (!m_Head)
			m_Head = DEBUG_NEW AutoCompNode(ToAdd);
		else
		{
			AutoCompNode* CurrPlace = m_Head;
			while (CurrPlace->next)
				CurrPlace = CurrPlace->next;
			
			CurrPlace->next = DEBUG_NEW AutoCompNode(ToAdd);
		}
	}

	m_size++;
	m_bDirty = TRUE;

	return true;
}

bool AutoCompList::Remove(const char *Data)
{
	AutoCompNode* Leader = NULL;
	AutoCompNode* CurrPlace = m_Head;
	bool foundsome = false;

	if (CurrPlace)
	{
		Leader = CurrPlace->next;	
	
		if (stricmp(CurrPlace->GetData(), Data) == 0)
		{

			delete CurrPlace;
			CurrPlace = NULL;
			foundsome = true;
			m_size--;
			m_Head = Leader;
		}

		while (Leader)
		{
			if (stricmp(Leader->GetData(), Data) == 0)
			{
				CurrPlace->next = Leader->next;
				delete Leader;
				Leader = NULL;
				foundsome = true;
				m_size--;
				Leader = CurrPlace->next;
			}
			else
			{
				CurrPlace = Leader;
				Leader = Leader->next;
			}
		}
	}

	if (foundsome)
		m_bDirty = TRUE;

	return foundsome;
}

bool AutoCompList::Contains(const char* data)//ignores case
{
	AutoCompNode *CurrNode = m_Head;
	while (CurrNode != NULL)
	{
		const char* Data = CurrNode->m_Data;

		if (*Data && stricmp(Data, data) == 0)
			return TRUE;

		CurrNode = CurrNode->next;
	}
	return FALSE;
}

const char* AutoCompList::FindACStartingWith(const char* Start)
{
	CString matchThis(Start);

	matchThis.TrimLeft();
	matchThis.MakeLower();

	const int MatchThisLen = matchThis.GetLength();
	if (MatchThisLen == 0)
		return NULL;
	
	AutoCompNode* CurrNode = m_Head;
	while (CurrNode != NULL)
	{
		CString Data(CurrNode->m_Data);
		Data.MakeLower();

		if (strnicmp(Data, matchThis, MatchThisLen) == 0)
			return CurrNode->m_Data;

		CurrNode = CurrNode->next;
	}

	return NULL;
}

void AutoCompList::PullNamesFromStringAndAdd(const char* string)
{
	while (string && *string)
	{
		const char* end = FindAddressEnd(string, FALSE, TRUE);
		if (!end)
			break;

		// Yes, I know, string is const, but we're just temporarily changing it
		const char SaveChar = *end;
		*(char*)end = 0;
		Add(string);
		*(char*)end = SaveChar;

		if (!SaveChar)
			break;
		string = end + 1;
	}
}


CStringList* AutoCompList::FindListOfACStartingWith(const char* Start)
{
	CStringList* CSList = DEBUG_NEW CStringList;
	CString matchThis(Start);

	matchThis.TrimLeft();
	matchThis.MakeLower();
	const int MatchThisLen = matchThis.GetLength();

	if (MatchThisLen)
	{
		const char *	szMatchThis = matchThis;
		const char *	szData;
		
		AutoCompNode *CurrNode = m_Head;
		while (CurrNode != NULL)
		{
			CString Data(CurrNode->m_Data);

			if (!Data.IsEmpty())
			{
				Data.MakeLower();

				szData = Data;

				//	If the first character in the auto-complete entry is a quote and
				//	we have more characters start the comparison with the next character
				//	(unless we were looking for a string starting with a quote).
				if ( (*szData == '"') && (*szData != *szMatchThis) && (Data.GetLength() > 1) )
					++szData;

				if (strnicmp(szData, szMatchThis, MatchThisLen) == 0)
					CSList->AddTail(CurrNode->m_Data);
			}

			CurrNode = CurrNode->next;
		}
	}

	return CSList;
}

CStringList* AutoCompList::FindListOfFilesStartingWith(const char* Start)
{
	CString matchThis(Start);
	matchThis.TrimRight();
	matchThis.TrimLeft();

	const int MatchThisLen = matchThis.GetLength();
	if (MatchThisLen == 0)
		return NULL;

	char BasePath[MAX_PATH + 4];
	int index = matchThis.ReverseFind('\\');

	strncpy(BasePath, matchThis, index + 1);
	BasePath[index+1] = 0;

	strcat(BasePath, "*.*");

	struct _finddata_t c_file;
	long hFile = _findfirst( BasePath, &c_file );
	if ( hFile == -1L )
		return NULL;

	BasePath[strlen(BasePath) - 3] = 0;

	CString Filename = matchThis.Right((MatchThisLen - index) - 1);
	CStringList* CSList = DEBUG_NEW CStringList;
	do
	{
		if (strnicmp(Filename, c_file.name, Filename.GetLength()) == 0)
		{
			if ( strcmp(c_file.name, _T(".")) != 0 && strcmp(c_file.name, _T("..")) )
			{
				CString fullPath = BasePath;
				fullPath += c_file.name;
				CSList->AddTail(fullPath);
			}
		}
	} while ( _findnext( hFile, &c_file ) == 0 );

	return CSList;
}

//
//	AutoCompList::GetAllAC()
//
//	Returns the complete history list (comma separated) in a CString.
//
CString AutoCompList::GetAllAC()
{
	CString			 strList;
	AutoCompNode	*pacnCurrNode = m_Head;

	while (pacnCurrNode != NULL)
	{
		CString Data(pacnCurrNode->m_Data);
		Data.MakeLower();

		if (!strList.IsEmpty())
		{
			strList += ",";
		}
		strList += pacnCurrNode->m_Data;

		pacnCurrNode = pacnCurrNode->next;
	}

	return strList;
}

static AutoCompList* g_AutoCompleter = NULL;

AutoCompList* GetAutoCompList()
{
	if (!g_AutoCompleter)
	{
		g_AutoCompleter = DEBUG_NEW AutoCompList(CRString(IDS_RECIP_HIST_LIST));
		if (g_AutoCompleter)
		{
			if (!g_AutoCompleter->ReadAutoCompleteList())
			{
				delete g_AutoCompleter;
				g_AutoCompleter = NULL;
			}
		}
	}

	return g_AutoCompleter;
}

void DeleteAutoCompList()
{
	if (g_AutoCompleter)
	{
		g_AutoCompleter->SaveToDisk();
		delete g_AutoCompleter;
		g_AutoCompleter = NULL;
	}
}


AutoCompleterListBox::AutoCompleterListBox(CHeaderField* pHeaderField)
{
	ASSERT(pHeaderField != NULL);

	m_HotRollover = true;
	m_bForceSelection = false;
	m_pEditControl = (AutoCompCEdit*)pHeaderField;

	m_AutoCompList = DEBUG_NEW DropDownListBox(pHeaderField, this);
	m_MinWidth = -1;

	CRect rcPos;
	pHeaderField->GetWindowRect(rcPos);

	AfxGetMainWnd()->ScreenToClient(rcPos);

	rcPos.top = rcPos.bottom - 1;

	m_AutoCompList->Create( IDD_AUTOCOMP, AfxGetMainWnd());
	m_AutoCompList->MoveWindow(rcPos);
}

AutoCompleterListBox::AutoCompleterListBox(DSEdit* pDSQueryField, int minwidth)
{
	ASSERT(pDSQueryField != NULL);

	m_HotRollover = true;
	m_bForceSelection = false;
	m_pEditControl = (AutoCompCEdit*)pDSQueryField;

	m_MinWidth = minwidth;

	m_AutoCompList = DEBUG_NEW DropDownListBox(m_pEditControl, this);

	CRect rcPos;
	m_pEditControl->GetWindowRect(rcPos);

	AfxGetMainWnd()->ScreenToClient(rcPos);

	rcPos.top = rcPos.bottom - 1;

	m_AutoCompList->Create( IDD_AUTOCOMP, AfxGetMainWnd());
	m_AutoCompList->MoveWindow(rcPos);
}

AutoCompleterListBox::~AutoCompleterListBox()
{
	m_AutoCompList->DestroyWindow();
	delete m_AutoCompList;
	m_AutoCompList = NULL;
}

void AutoCompleterListBox::DoACListBox()
{
	m_AutoCompList->SetFont( &GetMessageFont() );
	CRect rct;
	((CHeaderField*)m_pEditControl)->GetACListWindowRect(rct);

	rct.top = rct.bottom - 1;

	rct.right = rct.left +100;

	AfxGetMainWnd()->ScreenToClient(rct);
	m_AutoCompList->MoveWindow(rct);
}

void AutoCompleterListBox::ReCalcACListBoxPos()
{
	CRect rct, rct2;
	
	m_AutoCompList->GetClientRect(rct2);

	((CHeaderField*)m_pEditControl)->GetACListWindowRect(rct);

	rct.top = rct.bottom - 1;
	rct.bottom = rct.top + (rct2.bottom - rct2.top) + 6; // Why 6? I haven't  a clue. I imagine it's because
														 // the border around the window is 3 pixels? I dunno.
														 // Hopefully I'll be able to fix this.
	rct.right = rct.left + (rct2.right - rct2.left) + 6;

	AfxGetMainWnd()->ScreenToClient(rct);
	m_AutoCompList->MoveWindow(rct);
}

void AutoCompleterListBox::KillACListBox()
{

	m_AutoCompList->KillTimer(1);
	m_AutoCompList->DeleteAllItems();
	m_AutoCompList->ShowWindow(SW_HIDE);
}


void AutoCompleterListBox::RemoveFirstItem()
{
	m_AutoCompList->DeleteItem(0);

	CRect rct;
	m_AutoCompList->GetWindowRect(rct);

	CClientDC dc(m_AutoCompList);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	UINT cyChar  = tm.tmHeight + tm.tmExternalLeading;

	rct.bottom -= (cyChar + 2);

	m_AutoCompList->MoveWindow( rct);
}

void AutoCompleterListBox::RemoveLastItem()
{
	m_AutoCompList->DeleteItem(m_AutoCompList->GetItemCount() - 1);

	CRect rct;
	m_AutoCompList->GetWindowRect(rct);

	CClientDC dc(m_AutoCompList);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	UINT cyChar  = tm.tmHeight + tm.tmExternalLeading;

	rct.bottom -= (cyChar + 2);

	m_AutoCompList->MoveWindow( rct);	
}

void AutoCompleterListBox::InitList(bool Showlist)// =true
{
	m_AutoCompList->SetCheck(-1, FALSE);

	CRect rct;
	m_AutoCompList->GetWindowRect(rct);

	CClientDC dc(m_AutoCompList->GetListBox());

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	UINT cyChar  = tm.tmHeight + tm.tmExternalLeading;

	rct.bottom = rct.top + m_AutoCompList->GetItemCount() * (cyChar - 1) + 10;
	if (rct.bottom - rct.top > GetIniLong(IDS_INI_COMPLETER_LIST_HEIGHT) )
	{
		unsigned int iniheight = (unsigned int)GetIniLong(IDS_INI_COMPLETER_LIST_HEIGHT);
		if (iniheight > (((cyChar - 1) * 4) + 10))
			rct.bottom = rct.top + iniheight;
		else
			rct.bottom = rct.top + ((cyChar -1) * 4) + 10;
	}

	m_AutoCompList->MoveWindow(rct);
//	m_AutoCompList->SetCheck(0, TRUE);

	m_pEditControl->OnACListDisplayed();

	m_AutoCompList->SetWindowPos( &CWnd::wndTop , 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );

// Set Timer here? Call rest of function in timer proc?
	// COULD BE!
	m_AutoCompList->KillTimer(1);
	m_AutoCompList->SetTimer(1, GetIniLong(IDS_INI_AUTO_COMP_LISTBOX_DELAY), NULL);

//	if (m_AutoCompList->ShouldShow() && Showlist)
//		m_AutoCompList->ShowWindow(SW_SHOW);
}

void AutoCompleterListBox::InitListDSDropDown(bool Showlist)// =true
{
	m_AutoCompList->SetCheck(-1, FALSE);

	CRect rct;
	m_AutoCompList->GetWindowRect(rct);

	CClientDC dc(m_AutoCompList->GetListBox());

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	UINT cyChar  = tm.tmHeight + tm.tmExternalLeading;

	rct.bottom = rct.top + m_AutoCompList->GetItemCount() * (cyChar - 1) + 10;
	if (rct.bottom - rct.top > GetIniLong(IDS_INI_COMPLETER_LIST_HEIGHT) )
	{
		unsigned int iniheight = (unsigned int)GetIniLong(IDS_INI_COMPLETER_LIST_HEIGHT);
		if (iniheight > (((cyChar - 1) * 4) + 10))
			rct.bottom = rct.top + iniheight;
		else
			rct.bottom = rct.top + ((cyChar - 1) * 4) + 10;
	}

	m_AutoCompList->MoveWindow(rct);
//	m_AutoCompList->SetCheck(0, TRUE);

	m_pEditControl->OnACListDisplayed();

	m_AutoCompList->SetWindowPos( &CWnd::wndTop , 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );

	if (ShouldShow() && Showlist)
		m_AutoCompList->ShowWindow(SW_SHOW);
}

void AutoCompleterListBox::SetMinWidth(int width)
{
	m_MinWidth = width;
}

void AutoCompleterListBox::AddToACListBox(LPCTSTR string, int nPicture, DWORD NNItem /*= 0*/)
{
	const int InsertedIndex = m_AutoCompList->InsertItem(m_AutoCompList->GetItemCount(), string, nPicture);

	if (InsertedIndex < 0)
		ASSERT(0);
	else
	{
		if (nPicture == 1)
		{
			// Is a nickname, so it better have a nickname pointer
			ASSERT(NNItem != 0);
			m_AutoCompList->SetItemData(InsertedIndex, NNItem);
		}
		else
		{
			// Not a nickname, so it better not have a nickname pointer
			ASSERT(NNItem == 0);
		}

		CRect rct;
		m_AutoCompList->GetWindowRect(rct);

		CClientDC dc(m_AutoCompList);

		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		UINT cxChar = tm.tmAveCharWidth;

		int width = rct.right - rct.left;
		int NewProposedWidth = (strlen(string) * cxChar) + 45;// +// Scrollbar width;

		width = (NewProposedWidth > width) ? NewProposedWidth : width;

		if (m_MinWidth != -1)
			width =	(m_MinWidth > width) ? m_MinWidth : width;

		rct.right = rct.left + ((NewProposedWidth > width) ? NewProposedWidth : width);

		AfxGetMainWnd()->ScreenToClient(rct);
		m_AutoCompList->MoveWindow( rct);
	}
}

void AutoCompleterListBox::SelectionMade()
{
//	if (!isHotRollover())
//	{
//		int nItem;
//		nItem = m_AutoCompList->GetCurSel();
//		SetControlToItem(nItem-1, false);
//	}

	m_pEditControl->OnACListSelectionMade();

	KillACListBox();
//	m_pEditControl->GetSel(Start, End);
//	m_pEditControl->SetSel((Start > End) ? Start :End , (Start > End) ? Start :End );
}

void AutoCompleterListBox::SaveWindowHeight()
{
	CRect rcList;
	m_AutoCompList->GetWindowRect(rcList);
	int Height = rcList.bottom - rcList.top;

	int OldHeight = GetIniLong(IDS_INI_COMPLETER_LIST_HEIGHT);

	if (Height != OldHeight)
		SetIniLong(IDS_INI_COMPLETER_LIST_HEIGHT, Height);
}

void AutoCompleterListBox::TogglePoppedUpState()
{
	if (m_AutoCompList->IsWindowVisible())
		m_AutoCompList->ShowWindow(SW_HIDE);
	else
		m_AutoCompList->ShowWindow(SW_SHOW);
}

BOOL AutoCompleterListBox::ShouldShow() 
{
	const int ItemCount = m_AutoCompList->GetItemCount();

	if (ItemCount > 1 || (ItemCount > 0 && GetIniShort(IDS_INI_AUTO_COMP_LIST_ONLY)))
		return TRUE;

	return FALSE;
}

BOOL AutoCompleterListBox::ContainsItems()
{
	if (m_AutoCompList->GetItemCount() > 0)
		return TRUE;
	else
		return FALSE;
}

BOOL AutoCompleterListBox::SetCheck(int nItem, BOOL fCheck)
{
	return m_AutoCompList->SetCheck(nItem, fCheck);
}

void AutoCompleterListBox::SetListToControlDS(bool SetToZeroIfNotFound)
{
	CString WholeField;
	m_pEditControl->GetWindowText(WholeField);
	WholeField.TrimLeft();
	WholeField.TrimRight();
	LV_FINDINFO lv_fi;
	lv_fi.flags = LVFI_STRING;
	lv_fi.psz = WholeField.GetBuffer(0);

	int nItem = m_AutoCompList->FindItem(&lv_fi);

	WholeField.ReleaseBuffer();
	
	SetCheck(-1, false);
	SetCheck(nItem, true);

	int Selection = m_AutoCompList->GetCurSel();

	if (SetToZeroIfNotFound && Selection == -1)
	{
		SetCheck(0, true);
		SetControlToItem(0);
	}
	m_AutoCompList->EnsureVisible((Selection == -1) ? 0 : Selection, false);
}

void AutoCompleterListBox::SetListToControl(bool SetToZeroIfNotFound)
{
	CString WholeField;
	int Start, End;
	if (!m_pEditControl)
		return;

	m_pEditControl->GetWindowText(WholeField);
	m_pEditControl->GetSel(Start, End);
	
	char* Field = WholeField.GetBuffer(0);

	char*floater1 = Field + Start;
	while (*floater1 != ',' && floater1 > Field)
		floater1--;

	char *floater2 = Field + End;
	while (*floater2 != ',' && floater2 < (Field + strlen(Field)) )
		floater2++;

	char*Item = DEBUG_NEW char[(floater2 - floater1) + 1];

	strncpy(Item, floater1, (floater2 - floater1));
	Item[(floater2 - floater1)] = 0;

	WholeField.ReleaseBuffer();

	LV_FINDINFO lv_fi;
	lv_fi.flags = LVFI_STRING;
	lv_fi.psz = Item;

	int nItem = m_AutoCompList->FindItem(&lv_fi);

	delete [] Item;

	SetCheck(-1, false);
	SetCheck(nItem, true);

	int Selection = m_AutoCompList->GetCurSel();

	if (SetToZeroIfNotFound && Selection == -1)
	{
		SetCheck(0, true);
		SetControlToItem(0);
	}
	m_AutoCompList->EnsureVisible((Selection == -1) ? 0 : Selection, false);
}

void AutoCompleterListBox::SetControlToItem(int nIndex, bool PreserveCase)
{
	if (nIndex == -1)	// Set it to the currently selected item.
	{
		nIndex = m_AutoCompList->GetCurSel(); 	
	}

	if (m_AutoCompList->GetItemCount() >= nIndex &&
		(m_bForceSelection || !GetIniShort(IDS_INI_AUTO_COMP_LIST_ONLY)))
	{
		m_bForceSelection = false;

		int Start, End;
		m_pEditControl->GetSel(Start, End);

		CString WholeField;
		m_pEditControl->GetWindowText(WholeField);
		if (Start > End)	// Can this happen?
		{
			UINT temp = Start;
			Start = End;
			End = temp;
		}

 		CString Front = WholeField.Left(Start);
		CString Back = WholeField.Right(WholeField.GetLength() - End);

		char* FrontPart = DEBUG_NEW char[Front.GetLength() +1];
		strcpy(FrontPart, Front);

		char* floater = FrontPart + strlen(FrontPart);

		while (*floater != ',' && floater != FrontPart)
			floater--;
		if (*floater == ',' || *floater == '\r' || *floater == '\n')
		{
			floater++;
			while (*floater == ' ' || *floater == '\r' || *floater == '\n')
				floater++;
		}
		// We should now be pointing at the beginning of the word that started this whole mess.
//		blah bjdf, kjhsdfjhm@kjhds.com, joshu
//		                                ^------floater
		char tempchar = *floater;
		*floater = 0;

		WholeField = FrontPart;
		*floater = tempchar;

		char *typedByUser = DEBUG_NEW char[strlen(floater)+1];
		strcpy(typedByUser, floater);
		delete [] FrontPart;
		FrontPart = NULL;

		CString FromListBox;
		FromListBox = m_AutoCompList->GetItemText(nIndex, 0);

		char* FromLB = FromListBox.GetBuffer(0);
		char* TBUtemp = typedByUser;
		char* FLBtemp = FromLB;
		// keep track of the difference between the length of the list
		// entry and the length of the text the user typed; we will use
		// this later to select everything we inserted, removing yet another
		// half-assed half-working address parser.  SD 5/18/99
		int charsAddedFromLB = FromLB ? strlen(FromLB)-strlen(typedByUser) : 0;

		while (*TBUtemp == ' ' || *TBUtemp == '\r' || *TBUtemp == '\n')
			TBUtemp++;

		// If we skipped a quote character in the list box then account for that here
		// if we're preserving case. (If we're not preserving case then the whole
		// entry will be replaced.)
		if ( PreserveCase && (*FLBtemp == '"') && (*FLBtemp != *TBUtemp) )
		{
			FLBtemp++;
			if (charsAddedFromLB > 0)
				charsAddedFromLB--;
		}

		while (*TBUtemp != 0 && *FLBtemp != 0)
		{
			TBUtemp++;
			FLBtemp++;
		}

		if (PreserveCase)
		{
			WholeField += typedByUser;
			WholeField += FLBtemp;
		}
		else
			WholeField += FromListBox;

		bool finished = false;
		if (Back.GetLength() != 0)
		{
			char Character = Back.GetAt(0);
			while (Character != ',' && !finished)
			{
				if (Character != ' ')
				{
					Back = _T(", ") + Back;
					finished = true;
				}
			}
		}

		WholeField += Back;

		delete [] typedByUser;
		typedByUser = NULL;


		char* FullString = WholeField.GetBuffer(0);
		if (Start > (int)strlen(FullString))
			return;

		floater = FullString + Start;
		// The half-assed half-working address parser used to be here to figure
		// out where the end of the address was so that we could select it.  Well,
		// kemosabe, we just inserted the darn address so we already know how
		// long it is, so why bother to reparse?  Somebody clearly is missing
		// something big here; let's hope it's not I.  :-)  SD 5/18/99
		End = Start + charsAddedFromLB;


		m_pEditControl->SetWindowText(WholeField);

		int nID = m_pEditControl->GetDlgCtrlID( );
		(m_pEditControl->GetParent())->SendMessage(WM_COMMAND, MAKEWPARAM(nID, EN_UPDATE), (long)m_pEditControl->GetSafeHwnd());
		
		// Select in the normal direction (left to right) rather than the old reversed direction.
		// This avoids a bug where the user could type an arrow key and then get a strage
		// double completion (full completion, plus ", " and partial completion).
		// This also allows the user to type the right arrow key to trigger completion, and
		// move on to the next address to enter.
		m_pEditControl->SetSel(Start, End);

		ReCalcACListBoxPos();

		m_pEditControl->SetModify(true);

		FromListBox.ReleaseBuffer();
		WholeField.ReleaseBuffer();
	}
}

void AutoCompleterListBox::SetControlToItemDS(int nIndex, bool PreserveCase)
{
	if (nIndex == -1)	// Set it to the currently selected item.
	{
		nIndex = m_AutoCompList->GetCurSel(); 	
	}

	if (m_AutoCompList->GetItemCount() >= nIndex &&
		(m_bForceSelection || !GetIniShort(IDS_INI_AUTO_COMP_LIST_ONLY)))
	{
		m_bForceSelection = false;

		int Start, End;
		m_pEditControl->GetSel(Start, End);

		CString WholeField;
		m_pEditControl->GetWindowText(WholeField);
		if (Start > End)	// Can this happen?
		{
			UINT temp = Start;
			Start = End;
			End = temp;
		}

 		CString Front = WholeField.Left(Start);
		CString Back = WholeField.Right(WholeField.GetLength() - End);

		char* FrontPart = DEBUG_NEW char[Front.GetLength() +1];
		strcpy(FrontPart, Front);

		char* floater = FrontPart + strlen(FrontPart);

		while (*floater != ',' && floater != FrontPart)
			floater--;
		if (*floater == ',')
		{
			floater++;
			while (*floater == ' ')
				floater++;
		}
		// We should now be pointing at the beginning of the word that started this whole mess.
//		blah bjdf, kjhsdfjhm@kjhds.com, joshu
//		                                ^------floater
		char tempchar = *floater;
		*floater = 0;

		WholeField = FrontPart;
		*floater = tempchar;

		char *typedByUser = DEBUG_NEW char[strlen(floater)+1];
		strcpy(typedByUser, floater);
		delete [] FrontPart;
		FrontPart = NULL;

		CString FromListBox;
		FromListBox = m_AutoCompList->GetItemText(nIndex, 0);

		char* FromLB = FromListBox.GetBuffer(0);
		char* TBUtemp = typedByUser;
		char* FLBtemp = FromLB;

		while (*TBUtemp == ' ' && *TBUtemp != 0)
			TBUtemp++;

		while (*TBUtemp != 0 && *FLBtemp != 0 && tolower(*TBUtemp) == tolower(*FLBtemp))
		{
			TBUtemp++;
			FLBtemp++;
		}

		if (PreserveCase)
		{
			WholeField += typedByUser;
			WholeField += FLBtemp;
		}
		else
			WholeField += FromListBox;
		WholeField += Back;

		delete [] typedByUser;
		typedByUser =NULL;


		char* FullString = WholeField.GetBuffer(0);
		if (Start > (int)strlen(FullString))
			return;

		floater = FullString + Start;

		while (*floater != ',' && (floater <= FullString + strlen(FullString)) )
			floater++;

		if (*floater == ',')
			floater--;

		while (*floater == ' ')
			floater--;


		if ((floater - FullString) < 0 || ((floater - FullString) - 1) > (int)strlen(FullString))
			return;

		End = Start + ((floater - FullString) -1);

		m_pEditControl->SetWindowText(WholeField);
		m_pEditControl->SetSel(End, Start);

		FromListBox.ReleaseBuffer();
		WholeField.ReleaseBuffer();
	}
}


void AutoCompleterListBox::OnArrowKey(bool Up, bool AndChangeEditField)
{

	int Selection = m_AutoCompList->GetCurSel();

	if (Up)
	{
		if (Selection != 0)
			m_AutoCompList->SetCheck(Selection-1, TRUE);;
	}
	else
	{
		if (Selection != m_AutoCompList->GetItemCount() -1)
			m_AutoCompList->SetCheck(Selection+1, TRUE);
	}

	ReCalcACListBoxPos();

	if (AndChangeEditField)
	{
		int Selection = m_AutoCompList->GetCurSel();
		SetControlToItem(Selection, true);
	}
}
// Deletes things from Autocomplete list
// And/or Address book entries
void AutoCompleterListBox::OnDeleteKey()
{
	// Get Current selection and text for it
	int Selection = m_AutoCompList->GetCurSel();
	CString ToRemove(m_AutoCompList->GetItemText(Selection, 0));

	LVITEM lvItem;
	memset(&lvItem, 0, sizeof(LVITEM));
	lvItem.mask = LVIF_IMAGE;
	lvItem.iItem = Selection;
	
	// Get picture to tell what kind of item it is
	m_AutoCompList->GetItem(&lvItem);

	AutoCompList* pACL = GetAutoCompList();

	// Image 3 is a plugin address
	if(lvItem.iImage == 3)
	{
		ErrorDialog(IDS_ERR_DELETE_PLUGIN_NICK);
		return;
	}
	else
	if(lvItem.iImage == 1) // Image 1 is Address Book Entry
	{
		CNicknameFile* NickFile = NULL;
		CNickname* nn = NULL;
		bool keep_going = true;
		POSITION pos = NULL;
		POSITION prevpos = NULL;

		// Need to grab this item before the warning dialog pops up
		// because that will cause the auto-complete list to go away
		CNickname *ItemNN = (CNickname *) m_AutoCompList->GetItemData(Selection);
		if(!ItemNN)
		{
			ASSERT(0);
			return;
		}

		if (WarnYesNoDialog(0, IDS_DELETE_AB_ADDRESS) == IDOK)
		{
			// Delete the address
			LPTSTR lpszRightFile;
			CString tmpRightFile(ItemNN->GetNickContainingFile()); // I am hating these things
			lpszRightFile = tmpRightFile.GetBuffer(tmpRightFile.GetLength());

			for (NickFile = g_Nicknames->GetFirstNicknameFile();
				 keep_going && (NickFile != NULL);
				 NickFile = g_Nicknames->GetNextNicknameFile())
			{
					// if it is a plugin nickfile skip
					 // We won't be deleting plugin nicks here
				if(NickFile->IsPluginNickFile())
					 continue;

				LPTSTR lpszCurFile;


				lpszCurFile = NickFile->m_Filename.GetBuffer(NickFile->m_Filename.GetLength());



				if(_stricmp(lpszCurFile,lpszRightFile) != 0)
					continue;
				
				pos = NickFile->GetHeadPosition();
				
				while (pos)
				{
					prevpos = pos;
					nn = NickFile->GetNext(pos);
					if (nn)
					{
						if (!stricmp(ToRemove, nn->GetName()))
						{
							
							NickFile->RemoveAt(prevpos);
							if (!NickFile->WriteTxt() || !NickFile->WriteToc())
								NickFile->SetModified(TRUE);
							g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_REFRESH_LHS, NULL);
							keep_going = false;
							break;
						}
					}
					else
					{
						ASSERT(0);
						break;
					}
			
				}
			 }
		
				 		
			
			

		}
		else
			return;

	}

	if (pACL)
	{
		pACL->Remove(ToRemove);
		m_AutoCompList->DeleteItem(Selection);
		pACL->SaveToDisk();
	}

	if (m_AutoCompList->GetItemCount() > 0)
	{
		ReCalcACListBoxPos();
		m_AutoCompList->SetCheck(0, TRUE);
		Selection = m_AutoCompList->GetCurSel();
		SetControlToItem(Selection, true);
	}
	else
		KillACListBox();
}

void AutoCompleterListBox::SetControlToCheck()
{
	int Selection = m_AutoCompList->GetCurSel();
	SetControlToItem(Selection, false);
}

BEGIN_MESSAGE_MAP(DropDownListBox, CWnd)
	//{{AFX_MSG_MAP(DropDownList)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void DropDownListBox::OnTimer(UINT nID)
{
	const int ItemCount = GetItemCount();

	if (GetIniShort(IDS_INI_POPUP_NAMECOMPLETER) &&
		(ItemCount > 1 || (ItemCount > 0 && GetIniShort(IDS_INI_AUTO_COMP_LIST_ONLY))))
	{
		ShowWindow(SW_SHOW);
	}

	KillTimer(1);
}

DropDownListBox::DropDownListBox(CEdit* pField, AutoCompleterListBox* pAutocompleter) : m_ListCtrl(0)
{
	m_pAutoCompleter = pAutocompleter;
	m_UserHasSized = false;
	m_pTheField = pField;
}

DropDownListBox::~DropDownListBox()
{
	m_ListCtrl->DestroyWindow();
	delete m_ListCtrl;
}

BOOL DropDownListBox::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
{
	if (nHitTest == HTBOTTOMRIGHT  || nHitTest == HTBOTTOMLEFT  || nHitTest == HTLEFT  || nHitTest == HTRIGHT  ||
		nHitTest == HTTOP || nHitTest == HTTOPRIGHT || nHitTest == HTTOPLEFT)
	{
		return TRUE;
	}
	else
		return CDialog::OnSetCursor( pWnd, nHitTest, message );
}

LRESULT DropDownListBox::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
	if (m_ListCtrl->GetItemCount() > 0)
	{
		int width, height;
		CRect rct, rct2;
		m_pTheField->GetWindowRect(rct);
		GetWindowRect(rct2);
		width = rct2.right - rct2.left;
		height = rct2.bottom - rct2.top;

		rct.top = rct.bottom - 1;
		rct.bottom = rct.top + height;
		rct.right = rct.left + width;

		AfxGetMainWnd()->ScreenToClient(rct);

		MoveWindow(rct);
	}
	return TRUE;
}

BOOL DropDownListBox::PreTranslateMessage( MSG* pMsg )
{
	if (WM_LBUTTONDOWN == pMsg->message)
	{
		CPoint point(pMsg->pt.x, pMsg->pt.y);
		m_ListCtrl->ScreenToClient((POINT*)&point);
		int ItemHit = m_ListCtrl->HitTest(point);
		if (ItemHit != -1)
		{
			SetCheck(-1, FALSE);
			SetCheck(ItemHit, TRUE);
		}
		return TRUE;
	}
	else if(WM_NCLBUTTONDOWN == pMsg->message &&( HTTOP == pMsg->wParam || HTLEFT == pMsg->wParam || HTTOPRIGHT == pMsg->wParam ||
		HTRIGHT == pMsg->wParam || HTTOPLEFT == pMsg->wParam || HTBOTTOMLEFT == pMsg->wParam || HTBOTTOMRIGHT == pMsg->wParam) )
	{
		return TRUE;	// Gobble it up when people click on a broder other than the bottom one to resize.
	}	
	else
		return CDialog::PreTranslateMessage(pMsg);
}

void DropDownListBox::OnSetFocus( CWnd* pOldWnd )
{
	ASSERT(0);
}

void DropDownListBox::MoveWindow(CRect rect)
{
	CDialog::MoveWindow(rect);
	CRect ClientRct;
	GetClientRect(ClientRct);

	ClientRct.bottom -= 1;
	ClientRct.right -= 1;

	m_ListCtrl->MoveWindow(ClientRct);

	LVCOLUMN Column;

	Column.mask = LVCF_WIDTH; 
	Column.cx = (ClientRct.right-ClientRct.left) - 18;//Scrollbar width+1;

	m_ListCtrl->SetColumn(0, &Column);
}

void DropDownListBox::OnSize(UINT nType, int cx, int cy)
{
    CRect rcList;
    GetClientRect( rcList );
	
	m_ListCtrl->MoveWindow(rcList);

	if (m_UserHasSized == true)
	{
		m_pAutoCompleter->SaveWindowHeight();
		m_UserHasSized = false;
	}

	CDialog::OnSize(nType, cx, cy);
}

void DropDownListBox::OnSizing( UINT nSide, LPRECT lpRect )
{
	m_UserHasSized = true;

	CDialog::OnSizing(nSide, lpRect);
}

BOOL DropDownListBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    CRect rcList;
    GetClientRect( rcList );

    // create our only child: the funky listbox

	DWORD Style = WS_CHILD | WS_VISIBLE | WS_EX_TOPMOST | WS_VSCROLL | LVS_REPORT | 
		LVS_SINGLESEL | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS | /*LVS_EX_TRACKSELECT |*/
		LVS_NOCOLUMNHEADER | LVS_OWNERDRAWFIXED;
	
	Style &= ~LVS_SORTDESCENDING; // For some reason it defaults to being on.
	
	if (GetIniShort(IDS_INI_AUTOCOMP_LIST_IN_ALPHA) != 0)
		Style |= LVS_SORTASCENDING;

	m_ListCtrl = DEBUG_NEW MyListCtrl(this);

	m_ListCtrl->Create(Style, rcList, this, 26672);

	m_ListCtrl->InsertColumn(1, (char*)NULL);
	m_ListCtrl->SetHighlightType(LVEX_HIGHLIGHT_ROW);

	// c_dalew 9/30/99 - Changed Create() to Load() now that we use QCImageList.
	if (m_imageList.Load(14, 14, 0, MAKEINTRESOURCE(IDB_AUTOCOMP), RGB(192,192,192)))
	{
		m_ListCtrl->SetImageList( &m_imageList, LVSIL_SMALL );
	}
	return TRUE;
}

int DropDownListBox::GetItemCount()
{
	if (m_ListCtrl)
		return m_ListCtrl->GetItemCount();
	else 
	{
		ASSERT(0);
		return 0;
	}
}

BOOL DropDownListBox::DeleteAllItems()
{
	if (m_ListCtrl)
		return m_ListCtrl->DeleteAllItems();
	else 
	{
		ASSERT(0);
		return 0;
	}
}

int DropDownListBox::FindItem( LV_FINDINFO* pFindInfo, int nStart)
{
	if (m_ListCtrl)
		return m_ListCtrl->FindItem(pFindInfo, nStart);
	else 
	{
		ASSERT(0);
		return 0;
	}
}

BOOL DropDownListBox::InsertItem(int nItem, LPCSTR lpszItem, int nImage)
{
	if (m_ListCtrl)
		return m_ListCtrl->InsertItem(nItem, lpszItem, nImage);
	else 
	{
		ASSERT(0);
		return 0;
	}
}
BOOL DropDownListBox::SetItemData(int nItem, DWORD dwData)
{
	if (m_ListCtrl)
		return m_ListCtrl->SetItemData(nItem, dwData);
	else 
	{
		ASSERT(0);
		return 0;
	}

}
BOOL DropDownListBox::SetColumn(int nCol, const LVCOLUMN* pColumn)
{
	if (m_ListCtrl)
		return m_ListCtrl->SetColumn(nCol, pColumn);
	else 
	{
		ASSERT(0);
		return 0;
	}
}

BOOL DropDownListBox::DeleteItem(int nItem)
{
	if (m_ListCtrl)
		return m_ListCtrl->DeleteItem(nItem);
	else 
	{
		ASSERT(0);
		return 0;
	}
}

BOOL DropDownListBox::GetItem(LVITEM* pItem)
{
	if (m_ListCtrl)
		return m_ListCtrl->GetItem(pItem);
	else 
	{
		ASSERT(0);
		return 0;
	}
}

BOOL DropDownListBox::EnsureVisible(int nItem, BOOL bPartialOK)
{
	if (m_ListCtrl)
		return m_ListCtrl->EnsureVisible(nItem, bPartialOK);
	else 
	{
		ASSERT(0);
		return 0;
	}
}

CString DropDownListBox::GetItemText(int nItem, int nSubItem)
{
	if (m_ListCtrl)
		return m_ListCtrl->GetItemText(nItem, nSubItem);
	else 
	{
		ASSERT(0);
		return "";
	}
}
DWORD DropDownListBox::GetItemData(int nItem)
{
	if (m_ListCtrl)
		return m_ListCtrl->GetItemData(nItem);
	else 
	{
		ASSERT(0);
		return 0;
	}


}

BOOL DropDownListBox::GetCheck(int nItem)
{
	LVITEM ListCtrlItemInfo;
	ListCtrlItemInfo.mask = LVIF_STATE;
	ListCtrlItemInfo.iItem = nItem;
	ListCtrlItemInfo.iSubItem = 0; 
	ListCtrlItemInfo.state = 0;
	ListCtrlItemInfo.stateMask= LVIS_SELECTED; 

	GetItem(&ListCtrlItemInfo);

	if (ListCtrlItemInfo.state != 0)
		return TRUE;
	else
		return FALSE;
}

// -1 means all boxes. Lotsa stuff will break if someone allows more than one selection in this control so don't
BOOL DropDownListBox::SetCheck(int nItem, BOOL fCheck)
{
	if (nItem == -1 && fCheck == TRUE)	// You can't turn them all on
		return FALSE;
	int CurrentCheck = GetCurSel();

	if (nItem == CurrentCheck && fCheck == TRUE) // If you're turning on one that's already on, just skip to prevent redrawing
		return TRUE;

	LVITEM ListCtrlItemInfo;
	ListCtrlItemInfo.mask = LVIF_STATE;
	if (nItem == -1)
		ListCtrlItemInfo.iItem = CurrentCheck;
	else
		ListCtrlItemInfo.iItem = nItem;

	ListCtrlItemInfo.iSubItem = 0; 
	if (fCheck == TRUE)
	{
		ListCtrlItemInfo.state = (UINT)-1;
	//	m_ListCtrl->EnsureVisible(nItem, FALSE);

		// Here we should notify our parent of the change
		if (m_pAutoCompleter->isHotRollover())
		{
			m_pAutoCompleter->SetControlToItem(nItem);
		}
	}
	else
		ListCtrlItemInfo.state = 0;
	ListCtrlItemInfo.stateMask= LVIS_SELECTED;
	
	m_ListCtrl->SetItem(&ListCtrlItemInfo);
	m_ListCtrl->EnsureVisible(ListCtrlItemInfo.iItem, false);
	return TRUE;
}

int DropDownListBox::GetCurSel()
{
	int i =0;
	for (i = 0; i < m_ListCtrl->GetItemCount(); i++)
	{
		if (GetCheck(i) == TRUE)
			return i;
	}
	return -1;
}


BOOL MyListCtrl::PreTranslateMessage( MSG* pMsg )
{
//	if (WM_SETFOCUS)
//		ASSERT(0);
/*	if (WM_MOUSEMOVE == pMsg->message)
	{
		CPoint point(pMsg->pt.x, pMsg->pt.y);
		ScreenToClient((POINT*)&point);
		int ItemHit = HitTest(point);
		if (ItemHit != -1)
		{
			pParent->SetCheck(-1, FALSE);
			pParent->SetCheck(ItemHit, TRUE);
		}

		return TRUE;
	}
	else */if (WM_LBUTTONDOWN == pMsg->message)
	{
		CPoint point(pMsg->pt.x, pMsg->pt.y);
		ScreenToClient((POINT*)&point);
		int ItemHit = HitTest(point);
		if (ItemHit != -1)
		{
			pParent->SetCheck(-1, FALSE);
			pParent->SetCheck(ItemHit, TRUE);
		}

		pParent->m_pAutoCompleter->ForceSelection();
		pParent->m_pAutoCompleter->SelectionMade();
		return TRUE;
	}
	else
		return CListCtrl::PreTranslateMessage(pMsg);
}
