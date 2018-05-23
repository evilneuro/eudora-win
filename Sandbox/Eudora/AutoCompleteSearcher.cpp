#include "stdafx.h"

#ifdef _DEBUG
#ifdef new
#undef new
#endif
#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif
#endif

#include <xstddef>
#pragma warning (disable: 4663 4244)
#include "AutoCompleteSearcher.h"
#pragma warning (default: 4663 4244)

#include "fileutil.h"
#include "rs.h"
#include "resource.h"
#include "headervw.h"
#include "font.h"
#include "CompMessageFrame.h"
#include "address.h"

#ifdef _DEBUG
#undef new
#undef THIS_FILE
#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC( AutoCompCEdit, CEdit )

AutoCompNode::AutoCompNode(char *data)
{
	m_Data = new char[strlen(data)+1];
	strcpy(m_Data, data);
	next = NULL;
}

AutoCompNode::~AutoCompNode()
{
	delete [] m_Data;
	m_Data = NULL;
}

char *AutoCompNode::GetData()
{
	return m_Data;
}

AutoCompList::AutoCompList(char *filename)
{
	m_Filename = new char[MAX_PATH+1];
	strncpy(m_Filename, filename, MAX_PATH);
	m_Head = NULL;
	size = 0;
}

AutoCompList::AutoCompList(CString filename)
{
	m_Filename = new char[MAX_PATH+1];
	strncpy(m_Filename, filename, MAX_PATH);
	m_Head = NULL;
	size = 0;
	m_MaxSize = 0;
}

AutoCompList::~AutoCompList()
{
	delete [] m_Filename;
	m_Filename = NULL;
	AutoCompNode *reaper = m_Head;
	AutoCompNode *placeholder;
	
	if (reaper)
	{
		placeholder = reaper->next;
	}
	
	while (reaper != NULL)
	{
		delete reaper;
		reaper = placeholder;
		if (placeholder)
			placeholder = placeholder->next;
	}
}

bool AutoCompList::ReadAutoCompleteList()
{
	JJFile *ThirdInput = new JJFile(4096, false);
	
	char *pBuffer;
	pBuffer = new char[256];
	long NumBytes;

	char Filename[MAX_PATH];

	strcpy(Filename, EudoraDir);
	strncat(Filename, m_Filename, MAX_PATH-(strlen(Filename)+1));

	if (!(ThirdInput->Open(Filename, O_RDONLY)))
	{
		while (!ThirdInput->GetLine(pBuffer, 255, &NumBytes))
		{
			Add(pBuffer, true);
		}

		ThirdInput->Close();
		delete [] pBuffer;
		pBuffer = NULL;
		delete ThirdInput;
		return true;
	}

	delete [] pBuffer;
	delete ThirdInput;

	return false;
}

void AutoCompList::SaveToDisk()
{
	JJFile *ThirdOutput = new JJFile();
	
	char Filename[MAX_PATH];

	strcpy(Filename, EudoraDir);
	strncat(Filename, m_Filename, MAX_PATH-(strlen(Filename)+1));

	if (!(ThirdOutput->Open(Filename, O_RDWR | O_CREAT | O_TRUNC)))
	{
		AutoCompNode * CurrNode = m_Head;

		while (CurrNode)
		{
			if (strcmp(_T(""), CurrNode->GetData()) != 0 )	// remove empty items.
				ThirdOutput->PutLine(CurrNode->GetData());
			CurrNode = CurrNode->next;
		}

		ThirdOutput->Close();
	}

	delete ThirdOutput;
}

bool AutoCompList::Add(char *Data, bool ToEnd)
{
	if (!Data)
		return false;

	int MaxSize = GetIniLong(IDS_INI_MAXIMUM_HISTORY_SIZE);
	if (MaxSize > 1000)	// Not too big
		MaxSize = 1000;
	if (MaxSize < 15)		// Yet not too small a list
		MaxSize = 15;

	if (size > MaxSize && m_MaxSize != -1)
	{
		AutoCompNode* pNode = m_Head;
		int NewSize = MaxSize - (MaxSize / 10);

		for (int i = 0; i < NewSize; i++)
		{
			pNode = pNode->next;
		}

		AutoCompNode* leader = pNode->next;
		pNode->next = NULL;
		while (leader != NULL)
		{
			AutoCompNode* temp = leader->next;
			delete leader;
			leader = temp;
		}
		size = NewSize;
	}


	CString ToAdd(Data);
	ToAdd.TrimLeft();
	ToAdd.TrimRight();
	char *AddThis = ToAdd.GetBuffer(0);

	while (Remove(AddThis) == true){}

	if (ToEnd == false)
	{
		AutoCompNode* CurrPlace = m_Head;

		*(&m_Head) = new AutoCompNode(AddThis);

		m_Head->next = CurrPlace;
	}
	else
	{
		AutoCompNode* CurrPlace = m_Head;
		if (CurrPlace == NULL)
		{
			m_Head = new AutoCompNode(AddThis);
		}
		else
		{
			while (CurrPlace->next != NULL)
			{
				CurrPlace = CurrPlace->next;
			}
			
			*(&(CurrPlace->next)) = new AutoCompNode(AddThis);
		}

	}
	size++;
	ToAdd.ReleaseBuffer();

	return true;
}

bool AutoCompList::Remove(char *Data)
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
			size--;
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
				size--;
				Leader = CurrPlace->next;
			}
			else
			{
				CurrPlace = Leader;
				Leader = Leader->next;
			}
		}
	}
	return foundsome;
}

char *AutoCompList::FindACStartingWith(char* Start)
{
	CString matchThis(Start);

	matchThis.TrimLeft();

	if (matchThis.GetLength() == 0)
	{
		return NULL;
	}
	
	AutoCompNode *CurrNode = m_Head;
	while (CurrNode != NULL)
	{
		CString data(CurrNode->m_Data);

		if (strnicmp(data, matchThis, matchThis.GetLength()) == 0)
		{
			return CurrNode->GetData();
		}

		CurrNode = CurrNode->next;
	}
	return NULL;
}

void AutoCompList::PullNamesFromStringAndAdd(CString string)
{
	const char *end = NULL;
	char* Buffer = string.GetBuffer(0);
	char *floater = Buffer;
	while ( (floater <= Buffer + strlen(Buffer) ) && *floater != NULL)
	{
		end = FindAddressEnd(floater, FALSE, TRUE);
		if (!end)
			break;

		char *address = new char[(end-floater) + 1];
		strncpy(address, floater, (end-floater));
		address[(end-floater)] = 0;
		Add(address);
		delete [] address;

		floater = (char *)end + 1;
	}
	string.ReleaseBuffer();
}


CStringList *AutoCompList::FindListOfACStartingWith(char* Start)
{
	CStringList *CSList;
	CSList = new CStringList;

	CString matchThis(Start);

	matchThis.TrimLeft();

//	if (matchThis.GetLength() == 0) // We return all hits? Ok, fine.
//	{
//		AutoCompNode *CurrNode = m_Head;
//		while (CurrNode != NULL)
//		{
//			CString data(CurrNode->m_Data);
//
//			CSList->AddTail(CurrNode->GetData());
//
//			CurrNode = CurrNode->next;
//		}
//	}
	
	AutoCompNode *CurrNode = m_Head;
	while (CurrNode != NULL)
	{
		CString data(CurrNode->m_Data);

		if (data.GetLength() > 0 && strnicmp(data, matchThis, matchThis.GetLength()) == 0)
		{
			CSList->AddTail(CurrNode->GetData());
		}

		CurrNode = CurrNode->next;
	}

	return CSList;
}

CStringList *AutoCompList::FindListOfFilesStartingWith(char* Start)
{
	long hFile;
	CStringList *CSList;
	CSList = new CStringList;

	CString matchThis(Start);
	matchThis.TrimRight();
	matchThis.TrimLeft();

	if (matchThis.GetLength() == 0)
		return NULL;

	char *BasePath = new char[matchThis.GetLength()+1];
	int index = matchThis.ReverseFind('\\');

	strncpy(BasePath, matchThis, index+1);
	BasePath[index+1] = 0;

	CString Filename = matchThis.Right(((matchThis.GetLength() - index)-1));

	struct _finddata_t c_file;

	if (_chdir(BasePath))
		return NULL;

	if( (hFile = _findfirst( "*.*", &c_file )) == -1L )
		return NULL;
	else
	{
		if (strnicmp(Filename, c_file.name, Filename.GetLength()) == 0)
		{
			CString fullPath = BasePath;
			fullPath += c_file.name;
			if ( strcmp(c_file.name, _T(".")) != 0 && strcmp(c_file.name, _T("..")) )
				CSList->AddTail(fullPath);
		}
	}

	while ( _findnext( hFile, &c_file ) == 0 )
	{
		if (strnicmp(Filename, c_file.name, Filename.GetLength()) == 0)
		{
			CString fullPath = BasePath;
			fullPath += c_file.name;
			if ( strcmp(c_file.name, _T(".")) != 0 && strcmp(c_file.name, _T("..")) )
				CSList->AddTail(fullPath);
		}
	}

	delete [] BasePath;
	BasePath = NULL;

	return CSList;
}

AutoCompList* g_AutoCompleter = NULL;


AutoCompleterListBox::AutoCompleterListBox(CHeaderField* pHeaderField)
{
	ASSERT(pHeaderField != NULL);

	m_HotRollover = true;
	m_pEditControl = (AutoCompCEdit*)pHeaderField;

	m_AutoCompList = new DropDownListBox(pHeaderField, this);
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
	m_pEditControl = (AutoCompCEdit*)pDSQueryField;

	m_MinWidth = minwidth;

	m_AutoCompList = new DropDownListBox(m_pEditControl, this);

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
	uint cxChar = tm.tmAveCharWidth;
	uint cyChar  = tm.tmHeight + tm.tmExternalLeading;

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
	uint cxChar = tm.tmAveCharWidth;
	uint cyChar  = tm.tmHeight + tm.tmExternalLeading;

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
	uint cyChar  = tm.tmHeight + tm.tmExternalLeading;

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

//	if (m_AutoCompList->GetItemCount() > 1 && Showlist)
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
	uint cyChar  = tm.tmHeight + tm.tmExternalLeading;

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

	if (m_AutoCompList->GetItemCount() > 1 && Showlist)
		m_AutoCompList->ShowWindow(SW_SHOW);
}

void AutoCompleterListBox::SetMinWidth(int width)
{
	m_MinWidth = width;
}

void AutoCompleterListBox::AddToEndOfACListBox(char *string, int nPicture)
{
	m_AutoCompList->InsertItem(m_AutoCompList->GetItemCount(), string, nPicture);

	CRect rct;
	m_AutoCompList->GetWindowRect(rct);

	CClientDC dc(m_AutoCompList);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	uint cxChar = tm.tmAveCharWidth;

	int width = rct.right - rct.left;
	int NewProposedWidth = (strlen(string) * cxChar) + 45;// +// Scrollbar width;

	width = (NewProposedWidth > width) ? NewProposedWidth : width;

	if (m_MinWidth != -1)
		width =	(m_MinWidth > width) ? m_MinWidth : width;

	rct.right = rct.left + ((NewProposedWidth > width) ? NewProposedWidth : width);

	AfxGetMainWnd()->ScreenToClient(rct);
	m_AutoCompList->MoveWindow( rct);
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

BOOL AutoCompleterListBox::ContainsMultItems() 
{
	if (m_AutoCompList->GetItemCount() > 1)
		return TRUE;
	else
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

	char*Item = new char[(floater2 - floater1) + 1];

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

	if (m_AutoCompList->GetItemCount() >= nIndex)
	{
		int Start, End;
		m_pEditControl->GetSel(Start, End);

		CString WholeField;
		m_pEditControl->GetWindowText(WholeField);
		if (Start > End)	// Can this happen?
		{
			uint temp = Start;
			Start = End;
			End = temp;
		}

 		CString Front = WholeField.Left(Start);
		CString Back = WholeField.Right(WholeField.GetLength() - End);

		char* FrontPart = new char[Front.GetLength() +1];
		strcpy(FrontPart, Front);

		char* floater = FrontPart + strlen(FrontPart);

		while (*floater != ',' && floater != FrontPart)
			floater--;
		if (*floater == ',')
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

		char *typedByUser = new char[strlen(floater)+1];
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

		while ( (*TBUtemp == ' ' || *TBUtemp == '\r' || *TBUtemp == '\n')  && *TBUtemp != 0)
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
		m_pEditControl->SetSel(End, Start);

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

	if (m_AutoCompList->GetItemCount() >= nIndex)
	{
		int Start, End;
		m_pEditControl->GetSel(Start, End);

		CString WholeField;
		m_pEditControl->GetWindowText(WholeField);
		if (Start > End)	// Can this happen?
		{
			uint temp = Start;
			Start = End;
			End = temp;
		}

 		CString Front = WholeField.Left(Start);
		CString Back = WholeField.Right(WholeField.GetLength() - End);

		char* FrontPart = new char[Front.GetLength() +1];
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

		char *typedByUser = new char[strlen(floater)+1];
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
/*
void AutoCompleterListBox::SetControlToItem(int nIndex)
{
	if (m_AutoCompList->GetItemCount() >= nIndex)
	{
		int Start, End;
		m_pEditControl->GetSel(Start, End);

		CString WholeField;
		m_pEditControl->GetWindowText(WholeField);
		if (Start > End)	// Can this happen?
		{
			uint temp = Start;
			Start = End;
			End = temp;
		}

 		CString Front = WholeField.Left(Start);
		CString Back = WholeField.Right(WholeField.GetLength() - End);

		char* FrontPart = new char[Front.GetLength() +1];
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
		*floater = 0;

		WholeField = FrontPart;
		delete [] FrontPart;

		CString FromListBox;
		FromListBox = m_AutoCompList->GetItemText(nIndex, 0);

		WholeField += FromListBox;
		WholeField += Back;

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

	}
}
*/

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
	int Showlist = (GetIniShort(IDS_INI_POPUP_NAMECOMPLETER) != 0);

	if (GetItemCount() > 1 && Showlist)
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

	m_ListCtrl = new MyListCtrl(this);
	if (m_ListCtrl != 0) 
	{
	    fBaptizeBlockMT(m_ListCtrl, "DropDownListBox::OnCreate");
	}
    m_ListCtrl->Create(Style, rcList, this, 26672);

	m_ListCtrl->InsertColumn(1, (char*)NULL);
	m_ListCtrl->SetHighlightType(LVEX_HIGHLIGHT_ROW);

	if (m_imageList.Create( IDB_AUTOCOMP, 14, 0, RGB (198,198,198)))
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
		ListCtrlItemInfo.state = (uint)-1;
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

		pParent->m_pAutoCompleter->SelectionMade();
		return TRUE;
	}
	else
		return CListCtrl::PreTranslateMessage(pMsg);
}
