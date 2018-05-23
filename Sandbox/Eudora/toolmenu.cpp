// toolmenu.cpp
//
// The toobar buttons that trigger menus
//

#include "stdafx.h"
#include <string.h>
#include <ctype.h>

#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "cursor.h"
#include "fileutil.h"
#include "rs.h"
#include "resource.h"
#include "usermenu.h"
#include "eudora.h"
#include "utils.h"
#include "ems-wglu.h"
#include "trnslate.h"

#include "toolmenu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// CTranslator Menu ---------------------------------------------------
BOOL CTranslatorMenu::InitMenu(const long type, const long context, BOOL Check /*= FALSE*/, BOOL Popup /*= FALSE*/)
{
	if (Popup)
		CreatePopupMenu();
			
	CTranslatorManager *theAPI = ((CEudoraApp *)AfxGetApp())->GetTranslators();
	CTranslator *t;
	
	short index = theAPI->FindFirstTranslator(type, context);
	while(index >= 0 && (t = theAPI->GetNextTranslator( &index, type, context )) ) 
	{
		HICON icn;
		if ((t->GetType() != EMST_COALESCED) || t->GetIcon(icn))
			AddMenuItem(t);
	}

	m_ShowCheck = Check;

	// There was none
	if (!GetMenuItemCount( ))
		return FALSE;
	return TRUE;
}

BOOL CTranslatorMenu::AddMenuItem(CTranslator *t)
{
	CString desc;
	t->getDescription(desc);
	UINT menuID = ID_TRANSLATOR;
	
	if (m_ShowCheck)
		menuID = ID_TRANSLATORS_QUEUE; 
	CTranslatorMenuItem *item = new CTranslatorMenuItem(t,desc, menuID);
	if (!item) return FALSE;
	
	AppendMenu( MF_OWNERDRAW, item->m_ID, (LPCTSTR)item );
	
	m_Items.AddTail(item);
	
	// Set the default translators
	if (t->GetFlags() & EMSF_DEFAULT_Q_ON)
		Toggle(item->m_ID, TRUE);
	
	return TRUE;
}

CTranslatorMenuItem *CTranslatorMenu::Find(UINT nID)
{	
	CTranslatorMenuItem *Item = NULL;
	
	POSITION pos = m_Items.GetHeadPosition();
	while (pos)
	{
		Item = (CTranslatorMenuItem *)m_Items.GetNext(pos);
		
		if (Item->m_ID == nID)
			return (Item);
	}

	return (NULL);
}


CTranslatorMenuItem *CTranslatorMenu::Find(long ModuleID, long TransID)
{
	CTranslatorMenuItem *Item = NULL;
	long mID, tID;
	
	POSITION pos = m_Items.GetHeadPosition();
	while (pos)
	{
		Item = (CTranslatorMenuItem *)m_Items.GetNext(pos);
		
		Item->m_Translator->GetID(mID, tID);
		if ( mID == ModuleID && tID == TransID)
			return (Item);
	}

	return (NULL);
}


BOOL CTranslatorMenu::Toggle(UINT id, BOOL on /*=FALSE*/)
{
	UINT State = GetMenuState( id, MF_BYCOMMAND );
	// flags represent what the sate will become
	UINT flags = MF_CHECKED;
	if (!on && (State & MF_CHECKED))
		flags = MF_UNCHECKED;

	// we've been toggled on
	if (!on)
	{
		CTranslatorMenuItem * item = Find(id);
		CTranslator *pTrans = item->GetTranslator();
		//item->m_Properties.Empty();
		if ( !pTrans->QueueTrans(EMSF_Q4_COMPLETION, item->m_Properties, (flags & MF_CHECKED) ? 0:1))
			flags = MF_UNCHECKED;
		else
			flags = MF_CHECKED;
	}

	CheckMenuItem( id, MF_BYCOMMAND | flags );
	return (flags & MF_CHECKED);
}



void CTranslatorMenu::SelectTranslators(const char *hdr)
{
	long ModuleID = 0, TransID = 0;
	char * dot = NULL;
	char * comma = NULL;
	char * space = NULL;

	// The format of the translor line is <modID.transID Properties, modID2.transID Properties...
	while (hdr)
	{
		// get rid of the first char <xxxxx.yyyyy>
		if (hdr[0] == '<')
			hdr += 1;
		ModuleID = atoi(hdr);
		dot = strchr(hdr,'.');
		if (dot)
		{
			hdr = dot+1;
			TransID = atoi(hdr);
		}
		else
			return;

		comma = strchr(hdr,',');
		space = strchr(hdr,' ');
		
		CTranslatorMenuItem *Item = Find(ModuleID, TransID);
		if (Item)
		{
			Toggle(Item->m_ID, TRUE);
			
			if (comma && space)
			{
				space++;
				while (space < comma)
					Item->m_Properties += *space++;
			}
			else if (space)
				Item->m_Properties = ++space;


		}

		if (comma)
			hdr = comma+1;
		else 
			return;
	}
}

CString CTranslatorMenu::GetSelectedTranslators()
{
	CString trans;
	CString wholeStr;
	trans.Empty();
	wholeStr.Empty();
	
	UINT State = 0;
	
	for (UINT i = 0; i < GetMenuItemCount(); i++)
	{
		State = GetMenuState( i, MF_BYPOSITION );
		if (State & MF_CHECKED)
		{
			UINT nid = GetMenuItemID(i);
			CTranslatorMenuItem * item = Find(nid);
			
			if (item)
			{
				long modID = 0, trID = 0;
				item->m_Translator->GetID(modID,trID);
				trans.Format("%04ld.%04ld", modID,trID);

				if (!item->m_Properties.IsEmpty())
				{
					trans += " ";
					trans += item->m_Properties;
				}


				if (wholeStr.IsEmpty())
					wholeStr = trans;
				else
				{
					wholeStr += ",";
					wholeStr += trans;
				}
			}
		}
	}
	return wholeStr;
}
  
// CTranslatorMenu ---------------------------------------------------
void CTranslatorMenu::EnableItems(BOOL enable /*= TRUE*/)
{
	UINT numItems = GetMenuItemCount();
	for (UINT i = 0; i < numItems; i++)
		EnableMenuItem( i, MF_BYPOSITION |(enable ? MF_ENABLED : MF_GRAYED));
}

void CTranslatorMenu::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	BOOL Checked = FALSE;
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	if (NULL == pDC || NULL == pDC->m_hDC)
		return;
	CRect rect(lpDrawItemStruct->rcItem);
	
	// Draw focus rect
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) == ODA_FOCUS)
	{
		pDC->DrawFocusRect(&rect);
		return;
	}

	// Is this grayed?
	BOOL Grayed = (lpDrawItemStruct->itemState & ODS_GRAYED) == ODS_GRAYED;

	// Set up some color values
	COLORREF BackColor, ForeColor;
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		BackColor = GetSysColor(COLOR_HIGHLIGHT);
		ForeColor = Grayed? GetSysColor(COLOR_GRAYTEXT) : GetSysColor(COLOR_MENU);
	}
	else
	{
		BackColor = GetSysColor(COLOR_MENU);
		ForeColor = Grayed? GetSysColor(COLOR_GRAYTEXT) : GetSysColor(COLOR_WINDOWTEXT);
	}
	
	// Draw background
	CBrush BackBrush(BackColor);
	pDC->FillRect(&rect, &BackBrush);

	// Get The Item
	int itemNumber = lpDrawItemStruct->itemID;
	CTranslatorMenuItem* item = NULL;
	if (itemNumber != -1)
		item = (CTranslatorMenuItem *)lpDrawItemStruct->itemData;

	// Are we drawing the checked bitmap?
	if ((lpDrawItemStruct->itemState & ODS_CHECKED) == ODS_CHECKED)
		Checked = TRUE;
	
	if (item)
	{ 
		BITMAP bm;

		// Draw the check
		if (m_ShowCheck && Checked)
		{
			CDC MemDC;
			if (MemDC.CreateCompatibleDC(pDC))
			{
				// The Checked Bitmap
				CBitmap chkBM;
				chkBM.LoadOEMBitmap(OBM_CHECK);
				chkBM.GetObject(sizeof(BITMAP), &bm);

				rect.left += 1;
				rect.top += 1;

				CBitmap *oldBM = MemDC.SelectObject(&chkBM);
				DWORD mode = SRCAND;
				if (lpDrawItemStruct->itemState & ODS_SELECTED)
					mode = NOTSRCCOPY;

				pDC->BitBlt(rect.left, rect.top+2, bm.bmWidth, bm.bmHeight, &MemDC, 0, 0, mode);
				if (oldBM)
					MemDC.SelectObject(oldBM);
			}
		}
		
		// Scoot over
		if (m_ShowCheck)
			rect.left += LOWORD(GetMenuCheckMarkDimensions()) + 2;
 		rect.top += 2;
#ifdef WIN32
		if (item->m_HIcon)
		{
			//
			// Warning!  The ::GetIconInfo() call creates HBITMAPs for
			// the bitmask bitmap and the color bitmap.  We must delete
			// these ourselves before returning!
			//
			ICONINFO ii; 
			if (::GetIconInfo(item->m_HIcon, &ii))
			{
				ASSERT(ii.hbmMask);
				ASSERT(ii.hbmColor);
				GetObject(ii.hbmMask,sizeof(BITMAP), &bm); 
				if ( (bm.bmWidth == 16) && (bm.bmHeight == 16) )   
					pDC->DrawIcon(rect.left, rect.top, item->m_HIcon);
				else
				{
					CDC MemDC;
					if (MemDC.CreateCompatibleDC(pDC))
					{
						// Get the bitmask
						GetObject(ii.hbmMask, sizeof(BITMAP), &bm);
						CBitmap* scaledBM = CBitmap::FromHandle(ii.hbmMask);
						if (scaledBM)
						{
							CBitmap *oldBM = MemDC.SelectObject(scaledBM);
							// Blt the bitmask
							pDC->StretchBlt(rect.left, rect.top, 
											16, 16,
											&MemDC, 
											0, 0,
											bm.bmWidth, bm.bmHeight, 
											SRCAND);
							
							// Blt the colormask
							GetObject(ii.hbmColor, sizeof(BITMAP), &bm);
							CBitmap* coloredBM = CBitmap::FromHandle(ii.hbmColor);
							if (coloredBM)
							{
								CBitmap *old = MemDC.SelectObject(coloredBM);
								pDC->StretchBlt(rect.left, rect.top, 
												16, 16,
												&MemDC, 
												0, 0,
												bm.bmWidth, bm.bmHeight, 
												SRCINVERT);
								if(old)
									MemDC.SelectObject(old);
							}

							// Restore 
							if(oldBM)
								MemDC.SelectObject(oldBM);
						}
					}
				}

				::DeleteObject(ii.hbmMask);
				::DeleteObject(ii.hbmColor);
			}
		}
#else
		if (item->m_HIcon)
		{  
			CBitmap bitmap;
     		CDC MemDC;

			// draw icon into off-screen bitmap
			int cxIcon = ::GetSystemMetrics(SM_CXICON);
			int cyIcon = ::GetSystemMetrics(SM_CYICON);

			if (MemDC.CreateCompatibleDC(pDC) &&
				bitmap.CreateCompatibleBitmap(pDC, cxIcon, cyIcon))
			{
				CBitmap* pBitmapOld = MemDC.SelectObject(&bitmap);
				if (pBitmapOld == NULL)
					return;

				// blt the bits already on the window onto the off-screen bitmap
				MemDC.StretchBlt(0, 0, 16, 16, pDC, 0, 0, cxIcon, cyIcon, SRCCOPY);

				// draw the icon on the background
				MemDC.DrawIcon(0, 0, item->m_HIcon);

				// draw the icon contents
				pDC->BitBlt(rect.left, rect.top, 16, 16, &MemDC, 0, 0, SRCCOPY);  
				
				if (pBitmapOld)
					 MemDC.SelectObject(pBitmapOld);
			}
		}
#endif
		// Set text colors
		pDC->SetTextColor(ForeColor);
		pDC->SetBkMode(TRANSPARENT);

		if (item->m_HIcon)
			rect.left +=16;
		rect.left += 4;
		pDC->DrawText(item->m_Name, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX);
	}
}


void CTranslatorMenu::MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
	CDC	theDC;
	
	theDC.CreateCompatibleDC( NULL );
		
	int itemNumber = lpMeasureItemStruct->itemID;
	CTranslatorMenuItem* item = NULL;
	if (itemNumber != -1)
		item = (CTranslatorMenuItem *)lpMeasureItemStruct->itemData;

 	CString Text;
 	if (item) Text = item->m_Name;
	if (Text.IsEmpty()) Text = "this is only a dummy";
	CSize size(theDC.GetTextExtent(Text, Text.GetLength()));

	if (m_ShowCheck)
		lpMeasureItemStruct->itemWidth = size.cx + LOWORD(GetMenuCheckMarkDimensions()) + 20;
	else
		lpMeasureItemStruct->itemWidth = size.cx + 20; 
	lpMeasureItemStruct->itemHeight = 20;
}

	
// CTranslatorMenuItem ---------------------------------------------------
CTranslatorMenuItem::CTranslatorMenuItem(CTranslator *t, const char *desc, UINT menuID) :
		CUserMenuItem(desc, menuID )

{
	m_Properties.Empty();
	m_Translator = t;
	t->GetIcon(m_HIcon);
}

#ifdef OLDSTUFF
// CAttacherMenuItem ---------------------------------------------------
CAttacherMenuItem::CAttacherMenuItem(CAttacher *pAtt, const char *desc, UINT menuID) :
		CUserMenuItem(desc, menuID )

{
	m_Attacher = pAtt;
}


// CSpecialMenuItem ---------------------------------------------------
CSpecialMenuItem::CSpecialMenuItem(CSpecial *pSp, const char *desc, UINT menuID) :
		CUserMenuItem(desc, menuID )

{
	m_Special = pSp;
}

// CAttacherMenu Menu ---------------------------------------------------
BOOL CAttacherMenu::InitMenu()
{
	CAttachArray *pAttArray = ((CEudoraApp *)AfxGetApp())->GetTranslators()->GetAttachers();
	CAttacher *pAtt;
	
	int size = pAttArray->GetSize();
	for( short i = 0; i < size; i++ ) 
	{
		pAtt = pAttArray->GetAt(i);
		if (pAtt)	
		{
			emsMenu eM =(emsMenu)*pAtt;
			
			CAttacherMenuItem *item = new CAttacherMenuItem(pAtt, eM.desc, ID_MESSAGE_ATTACH_PLUGIN);
			if (item)
			{
				AppendMenu( MF_STRING, item->m_ID, eM.desc );
				m_Items.AddTail(item);
			}
		}
	}


	// There was none
	if (!GetMenuItemCount( ))
		return FALSE;
	return TRUE;
}


// CTranslatorMenu ---------------------------------------------------
void CAttacherMenu::EnableItems(BOOL enable /*= TRUE*/)
{
	UINT numItems = GetMenuItemCount();
	for (UINT i = 0; i < numItems; i++)
		EnableMenuItem( i, MF_BYPOSITION |(enable ? MF_ENABLED : MF_GRAYED));
}


// CSpecialMenu Menu ---------------------------------------------------
BOOL CSpecialMenu::InitMenu(CUserMenu& shadowMenu)
{
	// The Special menu offset for plugins
	UINT position = 8;
	UINT shadow_position = 0;
	
	CSpecialArray *pSpecialArray = ((CEudoraApp *)AfxGetApp())->GetTranslators()->GetSpecials();
	CSpecial *pSpec;
	BOOL atLeastOne = FALSE;
	
	int size = pSpecialArray->GetSize();
	for( short i = 0; i < size; i++ ) 
	{
		pSpec = pSpecialArray->GetAt(i);
		if (pSpec)	
		{
			emsMenu eM =(emsMenu)*pSpec;
			
			//
			// It is important to note that creation of the shadow
			// copy of the menu item must not increment the global
			// user-defined command id counter, and then the shadow
			// copy must be set to the same command id as the
			// original.
			//
			CSpecialMenuItem *item = new CSpecialMenuItem(pSpec, eM.desc, ID_SPECIAL_PLUGIN);
			CUserMenuItem* p_shadow_item = new CUserMenuItem(eM.desc, ID_SPECIAL_PLUGIN, FALSE);
			if (item && p_shadow_item)
			{
				atLeastOne = TRUE;

				// Insert real item into real menu
				InsertMenu( position++, MF_BYPOSITION, item->m_ID, eM.desc );
				m_Items.AddTail(item);

				// Insert shadow item into shadow menu
				ASSERT(0 == p_shadow_item->m_ID);
				p_shadow_item->m_ID = item->m_ID;
				shadowMenu.InsertMenu(shadow_position++, MF_BYPOSITION, item->m_ID, eM.desc);
				shadowMenu.m_Items.AddTail(p_shadow_item);
			}
			else
			{
				//
				// Memory allocation failure, so cleanup.
				//
				if (item)
					delete item;
				if (p_shadow_item)
					delete p_shadow_item;
			}
		}
	}

	// There was none
	if (!atLeastOne)
		return FALSE;
	else
		InsertMenu( position++, MF_BYPOSITION | MF_SEPARATOR);
	return TRUE;
}

// CSpecialMenu Menu ---------------------------------------------------
BOOL CSpecialMenu::RemoveItems(CUserMenu& shadowMenu)
{
	// The Special menu offset for plugins
	UINT position = 8;

	BOOL atLeastOne = FALSE;
	while (m_Items.IsEmpty() == FALSE)
	{
		atLeastOne = TRUE;
		CUserMenuItem* Item = m_Items.RemoveHead();
		VERIFY(DeleteMenu(Item->m_ID, MF_BYCOMMAND));
		delete Item;
	}

	while (! shadowMenu.m_Items.IsEmpty())
	{
		CUserMenuItem* p_item = shadowMenu.m_Items.RemoveHead();
		VERIFY(shadowMenu.DeleteMenu(p_item->m_ID, MF_BYCOMMAND));
		delete p_item;
	}

	// Delete Separator, if any
	if (atLeastOne)
		DeleteMenu( position, MF_BYPOSITION);
	return TRUE;
}

#endif
