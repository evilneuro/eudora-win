// MIMEMap.cpp
//
// MIMEMap object that handles mappings between MIME type/subtype,
// filename extension, and Mac creator and type

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "fileutil.h"
#include "header.h"
#include "mime.h"
#include "MIMEMap.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



MIMEMap::MIMEMap(void)
{
	Init();
}

MIMEMap::~MIMEMap()
{
	for (int i = 0; i < m_Maps.GetSize(); i++)
		delete m_Maps[i];
}

void MIMEMap::Init()
{
	memset(m_Suffix, 0, sizeof(m_Suffix));
	memset(m_Mimetype, 0, sizeof(m_Mimetype));
	memset(m_Subtype, 0, sizeof(m_Subtype));
	memset(m_Creator, 0, sizeof(m_Creator));
	memset(m_Type, 0, sizeof(m_Type));
	m_MapType = MT_BOTH;
}

// Find the extension of a MIME attachment
BOOL MIMEMap::Find(MIMEState* ms)
{
	MIMEMap* maybe;
	MIMEMap* best;
	MIMEMap* bestMIME = NULL;
	MIMEMap* bestMac = NULL;
	HeaderDesc* hd = ms->m_hd;
	LPTSTR dot;

	if (!ms || !ms->m_hd->isMIME || !GetMaps())
		return (FALSE);

	strcpy(m_Mimetype, hd->contentType);
	strcpy(m_Subtype, hd->contentSubType);

	// fetch the filename and suffix, if any
	AttributeElement* AE = hd->GetAttribute(CRString(IDS_MIME_NAME));
	if (!AE)
		AE = hd->GetAttribute(CRString(IDS_MIME_CONTENT_DISP_FILENAME));
	if (AE && (dot = strrchr(AE->m_Value, '.')))
	{
		strncpy(m_Suffix, dot + 1, sizeof(m_Suffix) - 1);
		m_Suffix[sizeof(m_Suffix) - 1] = 0;
	}

	// See if there's x-mac-type and x-mac-creator fields
	if ((AE = hd->GetAttribute(CRString(IDS_MIME_MAC_TYPE))))
		HexToString(m_Type, AE->m_Value);
	if ((AE = hd->GetAttribute(CRString(IDS_MIME_MAC_CREATOR))))
		HexToString(m_Creator, AE->m_Value);

	// search for a match
	for (int i = 0; i < m_Maps.GetSize(); i++)
	{
		maybe = (MIMEMap*)m_Maps[i];
		if (maybe->m_MapType == MT_OUT || !maybe->m_Suffix[0])
        	continue;

		// a zero-length subtype is a match, and so are identical strings
		if (maybe->m_Mimetype[0] && !stricmp(m_Mimetype, maybe->m_Mimetype) &&
			(!maybe->m_Subtype[0] || !stricmp(m_Subtype, maybe->m_Subtype)))
		{
			if (!bestMIME || (!bestMIME->m_Subtype[0] && maybe->m_Subtype[0]))
				bestMIME = maybe;
		}

		// a zero-length creator/type is a match, and so are identical strings
		if (maybe->m_Creator[0] || maybe->m_Type[0])
		{
			if ((!maybe->m_Creator[0] || !strncmp(m_Creator, maybe->m_Creator, 4)) &&
				(!maybe->m_Type[0] || !strncmp(m_Type, maybe->m_Type, 4)))
			{
				if (!bestMac || (maybe->m_Creator[0] &&
					(!bestMac->m_Creator[0]||(!bestMac->m_Type[0] && maybe->m_Type))))
				{
					bestMac = maybe;
				}
			}
		}
	}

	if (!bestMIME && !bestMac)
		return (FALSE);

	best = bestMac;
	if (!bestMac ||
		(bestMIME && (bestMIME->m_Mimetype[0] && bestMIME->m_Subtype[0]) &&
		(!bestMac->m_Creator[0] || !bestMac->m_Type[0])))
	{
		best = bestMIME;
	}

	strcpy(m_Suffix, best->m_Suffix);

	return (TRUE);
}

// Find the extension of an Apple attachment
BOOL MIMEMap::Find(LPCTSTR Creator, LPCTSTR Type)
{
	MIMEMap* maybe;
	MIMEMap* best = NULL;

	if (!Creator || !Type || !GetMaps())
		return (FALSE);

	// search for a match
	for (int i = 0; i < m_Maps.GetSize(); i++)
	{
		maybe = (MIMEMap*)m_Maps[i];
		if (maybe->m_MapType == MT_OUT)
			continue;
		// a zero-length string is a match, and so are identical strings
		if (!maybe->m_Creator[0] && !maybe->m_Type[0])
			continue;
		if (maybe->m_Creator[0] && strnicmp(Creator, maybe->m_Creator, 4) ||
			maybe->m_Type[0] && strnicmp(Type, maybe->m_Type, 4))
		{
			continue;
		}

		if (!best) best = maybe;
		else if (best->m_Creator[0] && !maybe->m_Creator[0]) continue;
		else if (best->m_Type[0] && !maybe->m_Type[0]) continue;
		else if (best->m_Suffix[0] && !maybe->m_Suffix[0]) continue;
		else best = maybe;
	}
		
	if (!best)
		return (FALSE);

	strcpy(m_Suffix, best->m_Suffix);
	
	return (TRUE);
}

BOOL MIMEMap::Find(LPCTSTR Extension)
{
	MIMEMap* maybe;

	if (!Extension || !*Extension || !GetMaps())
		return (FALSE);

	// search for a match
	for (int i = 0; i < m_Maps.GetSize(); i++)
	{
		maybe = (MIMEMap*)m_Maps[i];
		if (maybe->m_MapType == MT_IN)
			continue;
		if (maybe->m_Suffix[0] && !strnicmp(maybe->m_Suffix, Extension, 3))
		{
			break;
		}
	}
		
	if (i >= m_Maps.GetSize())
		return (FALSE);

	strcpy(m_Creator, maybe->m_Creator);
	strcpy(m_Type, maybe->m_Type);
	strcpy(m_Mimetype, maybe->m_Mimetype);
	strcpy(m_Subtype, maybe->m_Subtype);
	
	return (TRUE);
}

BOOL MIMEMap::GetMaps()
{
	BOOL FoundSection = FALSE;
	LPTSTR value;
	LPTSTR comma;
	JJFile Settings;
	TCHAR buf[256];
	MIMEMap* map;

	if (m_Maps.GetSize())
		return (TRUE);

	CRString Section(IDS_MIME_MAPPINGS);
	CRString In(IDS_MIME_MAP_IN);
	CRString Out(IDS_MIME_MAP_OUT);

	if (FAILED(Settings.Open(INIPath, O_RDONLY)))
		return (FALSE);

	HRESULT hrGetLine = S_OK;
	for (int i = 0; i < 2; i++)
	{
		if (i == 1)
		{
			Settings.Close();
			if (DefaultINIPath.IsEmpty())
				break;
			if (FAILED(Settings.Open(DefaultINIPath, O_RDONLY)))
				return (FALSE);
			FoundSection = FALSE;
		}

		long lNumBytesRead = 0;
		while (SUCCEEDED(hrGetLine = Settings.GetLine(buf, sizeof(buf), &lNumBytesRead)) && (lNumBytesRead > 0))
		{
			::TrimWhitespaceMT(buf);
			if (!FoundSection)
			{
				if (!Section.CompareNoCase(buf))
					FoundSection = TRUE;
				continue;
			}
			// Hit next section, quit out
			if (*buf == '[')
				break;

			// Skip blank, commented, or badly formatted lines
			if (!*buf || *buf == ';' || !(value = strchr(buf, '=')))
				continue;
			if (!(map = new MIMEMap))
				return (FALSE);
			m_Maps.Add(map);

			// Find type of mapping
			if (!strnicmp(In, buf, value - buf))
				map->m_MapType = MT_IN;  
			else if (!strnicmp(Out, buf, value - buf))
				map->m_MapType = MT_OUT;  

			// Parse the entry to find pertinent info
			if (comma = strchr(++value, ','))
				*comma++ = 0;
			strncpy(map->m_Suffix, value, sizeof(m_Suffix) - 1);
			if (!(value = comma))
				continue;
			if (comma = strchr(value, ','))
				*comma++ = 0;
			strncpy(map->m_Creator, value, sizeof(m_Creator) - 1);
			if (!(value = comma))
				continue;
			if (comma = strchr(value, ','))
				*comma++ = 0;
			strncpy(map->m_Type, value, sizeof(m_Type) - 1);
			if (!(value = comma))
				continue;
			if (comma = strchr(value, ','))
				*comma++ = 0;
			strncpy(map->m_Mimetype, value, sizeof(m_Mimetype) - 1);
			if (!(value = comma))
				continue;
			if (comma = strchr(value, ','))
				*comma++ = 0;
			strncpy(map->m_Subtype, value, sizeof(m_Subtype) - 1);
		}
	}

	if (FAILED(hrGetLine))
		return (FALSE);
		
	HKEY ContentTypes;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("Mime\\Database\\Content Type"), 0, KEY_READ, &ContentTypes)== ERROR_SUCCESS)
	{
		DWORD	index=0;
		DWORD	bufLength;
		DWORD	dataType;
		LPTSTR	NameOfKey;
		LPTSTR	extensionData;
		DWORD	sizeofExtension = 32;

		bufLength = (sizeof(m_Mimetype) + sizeof(m_Subtype) + 1);
		NameOfKey = new TCHAR[bufLength];
		extensionData = new TCHAR[sizeofExtension];
		while (RegEnumKeyEx(ContentTypes, index++, NameOfKey, &bufLength, 0, NULL,NULL,NULL) == ERROR_SUCCESS)
		{
			HKEY Extension;
			if (RegOpenKeyEx(ContentTypes, NameOfKey, 0, KEY_READ, &Extension) == ERROR_SUCCESS)
			{
				if (RegQueryValueEx(Extension, _T("Extension"), 0, &dataType, (unsigned char *)extensionData, &sizeofExtension) == ERROR_SUCCESS)
				{
					if (!(map = new MIMEMap))
						return (FALSE);

					LPTSTR floater;
					floater = extensionData;
					if (*floater == '.')
						floater++;
					strncpy(map->m_Suffix, floater, sizeof(m_Suffix) - 1);

					floater = strchr(NameOfKey, '/');
					if (!floater) 
						return (FALSE);

					*floater = 0;
					floater++;

					strncpy(map->m_Mimetype, NameOfKey, sizeof(m_Mimetype) - 1);
					strncpy(map->m_Subtype, floater, sizeof(m_Subtype) - 1);
					m_Maps.Add(map);

					// Do we have Mac types and creators? If so someone added them by hand, 
					// cuz MS doesn't let you do it...
					sizeofExtension = 32;
					if (RegQueryValueEx(Extension, _T("MacType"), 0, &dataType, (unsigned char *)extensionData, &sizeofExtension) == ERROR_SUCCESS)
					{
						strncpy(map->m_Type, extensionData, sizeof(m_Type) - 1);
					}

					sizeofExtension = 32;
					if (RegQueryValueEx(Extension, _T("MacCreator"), 0, &dataType, (unsigned char *)extensionData, &sizeofExtension) == ERROR_SUCCESS)
					{
						strncpy(map->m_Creator, extensionData, sizeof(m_Creator) - 1);
					}

					sizeofExtension = 32;
					// We may need to specify which kind of mapping it is. If so, here ya go!
					map->m_MapType = MT_OUT;
					if (RegQueryValueEx(Extension, _T("EudoraMapType"), 0, &dataType, (unsigned char *)extensionData, &sizeofExtension) == ERROR_SUCCESS)
					{
						if (!stricmp(extensionData, _T("both")))
							map->m_MapType = MT_BOTH;
						else if (!stricmp(extensionData, _T("in")))
							map->m_MapType = MT_IN;
					}
				}
				bufLength = (sizeof(m_Mimetype) + sizeof(m_Subtype) + 1);
				sizeofExtension = 32;
				// close extension key
				RegCloseKey(Extension);
			}
		}
		// Close content key
		RegCloseKey(ContentTypes);
		delete [] NameOfKey;
		delete [] extensionData;
	}

	return (TRUE);
}

BOOL MIMEMap::SetExtension(LPTSTR Filename)
{
	if (!*m_Suffix)
		return (FALSE);

	LPTSTR ExtensionPeriod = strrchr(Filename, '.');

	// Here's what we do when we find various numbers of
	// characters after the last period in the filename:
	//   1, 2, or 3:    leave existing extension
	//   0 or 4:        replace existing extension with new extension
	//   5 or greater:  append new extension
	if (ExtensionPeriod)
	{
		int ExtensionLength = strlen(ExtensionPeriod + 1);
		if (1 <= ExtensionLength && ExtensionLength <= 3)
			return (FALSE);
		if (ExtensionLength >= 5)
			ExtensionPeriod = NULL;
	}

	// If the name with the extension tacked on is too long we're in trouble
	if ((strlen(m_Suffix) + strlen(Filename) + 2) > _MAX_PATH)
	{
		int Difference = _MAX_PATH - (strlen(m_Suffix) + strlen(Filename) +2);
		LPTSTR floater = Filename;
		while (*floater != 0)
			floater++;
		while (Difference--)
		{
			floater--;
		}
		*floater = 0;
	}

	if (ExtensionPeriod)
		*++ExtensionPeriod = 0;
	else
		strcat(Filename, ".");
	strcat(Filename, m_Suffix);

	return (TRUE);
}
