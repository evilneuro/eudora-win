// EMOTICONMANAGER.CPP
//
// Routines for handling emoticons



#include "stdafx.h"
#include "EmoticonManager.h"
#include "fileutil.h"
#include "resource.h"
#include "rs.h"


#include "DebugNewHelpers.h"


extern CString EudoraDir;
extern CString ExecutableDir;

EmoticonMap EmoticonDirector::m_EmoticonMap;
CMapStringToString EmoticonDirector::m_EscapeCharMap;
CStringList EmoticonDirector::m_ImageExtensionList;
CMapStringToString EmoticonDirector::m_HTMLEscapeCharMap;
EmoticonList EmoticonDirector::m_EmoticonList;
EmoticonIDMap EmoticonDirector::m_EmoticonIDMap;
EmoticonTriggerMap EmoticonDirector::m_EmoticonTriggerMap;

IMPLEMENT_DYNAMIC(Emoticon, QCCommandObject)

/*Emoticon::Emoticon()
{
	//never use the default constructor to create emoticon objects.
	//all emoticon objects must have a trigger associated.
	ASSERT(0);
}*/



Emoticon::Emoticon(EmoticonDirector* pDirector, CString szImageFullPath16, CString szImageFullPath24, CString szImageFullPath32, CString trigger, CString meaning /*= ""*/, bool bIsSynonym /*= false*/)
	:	QCCommandObject(pDirector), m_szImageFullPath16(szImageFullPath16), m_szImageFullPath24(szImageFullPath24), m_szImageFullPath32(szImageFullPath32), m_szMeaning(meaning),
		m_bIsSynonym(bIsSynonym), m_pImage(NULL), m_nImageWidth(0), m_nImageHeight(0), m_ID(0)
{
	if(trigger.IsEmpty())
		throw DEBUG_NEW_MFCOBJ_NOTHROW InvalidEmoticonException();

	m_szTrigger = trigger;
	m_TriggerLength = trigger.GetLength();
}

const CString& Emoticon::GetImageFullPath() const 
{ 

	//this function is called when parsing incoming emails / composing an email
	//first we try to look at the INI setting. 
	//if IDS_INI_MESSAGE_EMOTICON_SIZE can be set to use 16, 24 or 32 bit images.
	//if the ini setting is not present then we look at the font size that the user has set under Tools->Options->Font
	//size smallest & small & medium we use the 16 bit icons
	//size large we use the 24 bit icons
	//size largest we use the 32 bit icons

	const CString & szImageFullPath = m_szImageFullPath16;

	int emoticonSize = GetIniShort(IDS_INI_MESSAGE_EMOTICON_SIZE);
	
	if( (emoticonSize == IMG_SIXTEEN) && !m_szImageFullPath16.IsEmpty() )
		return m_szImageFullPath16;
	else if( (emoticonSize == IMG_TWENTYFOUR) && !m_szImageFullPath24.IsEmpty() )
		return m_szImageFullPath24;
	else if( (emoticonSize == IMG_THIRTYTWO) && !m_szImageFullPath32.IsEmpty() )
		return m_szImageFullPath32;

	int fontSize = GetIniShort(IDS_INI_MESSAGE_FONT_SIZE);

	if( (fontSize == FONT_SMALLEST) || (fontSize == FONT_SMALLER) || (fontSize == FONT_MEDIUM) ) {
		if( !m_szImageFullPath16.IsEmpty() )	
			return m_szImageFullPath16;
		else
			return ( !m_szImageFullPath24.IsEmpty() ) ? m_szImageFullPath24 : m_szImageFullPath32;
	} else if( fontSize == FONT_LARGER ) {
		if( !m_szImageFullPath24.IsEmpty() )	
			return m_szImageFullPath24;
		else
			return ( !m_szImageFullPath16.IsEmpty() ) ? m_szImageFullPath16 : m_szImageFullPath32;
	} else if( fontSize == FONT_LARGEST ) {
				if( !m_szImageFullPath32.IsEmpty() )	
			return m_szImageFullPath32;
		else
			return ( !m_szImageFullPath24.IsEmpty() ) ? m_szImageFullPath24 : m_szImageFullPath16;
	}

	return szImageFullPath;
}

const CString& Emoticon::GetImageFullPathForMenu() const 
{ 

	//this function is called by the toolbar menu
	//we prefer showing the 16 bit icons if we have one since it looks better on the small toolbar menu
	//if we don't have 16 bit, then try the 24 and then the 32
	const CString& szImageFullPath = m_szImageFullPath16;

	if(!m_szImageFullPath16.IsEmpty())
		return m_szImageFullPath16;
	else if(!m_szImageFullPath24.IsEmpty())
		return m_szImageFullPath24;
	else if(!m_szImageFullPath32.IsEmpty())
		return m_szImageFullPath32;

	return szImageFullPath;
}

Emoticon::~Emoticon()
{
	delete m_pImage;
}

void Emoticon::Execute(COMMAND_ACTION_TYPE	theAction, void* pData )
{

}

EmoticonDirector::EmoticonDirector()
{

}

bool EmoticonDirector::Build()
{

	//if any of the directories Emoticons, Emoticon24 or Emoticons32 do not exist either in
	//data or exec directory, we have no Emoticons
	//even if the directories don't exist RETURN *TRUE* - we don't want to stop Eudora from launching
	// just because the user doesn't have an Emoticon directory. 
	if ( !::FileExistsMT(EudoraDir + CRString(IDS_EMOTICON_DIRECTORY16)) && 
		 !::FileExistsMT(EudoraDir + CRString(IDS_EMOTICON_DIRECTORY24)) &&
		 !::FileExistsMT(EudoraDir + CRString(IDS_EMOTICON_DIRECTORY32)) &&
		 !::FileExistsMT(ExecutableDir + CRString(IDS_EMOTICON_DIRECTORY16)) &&
		 !::FileExistsMT(ExecutableDir + CRString(IDS_EMOTICON_DIRECTORY24)) &&
		 !::FileExistsMT(ExecutableDir + CRString(IDS_EMOTICON_DIRECTORY32)) )
		return true;

	//build the escape characters map
	BuildEscapeCharacterMap();

	//build list of image types that we support
	BuildImageExtensionList();

	//load the emoticon synonymn char
	m_emoticon_synonym = CRString(IDS_EMOTICON_CHAR_SYNONYM).GetAt(0);

	//read the image files and build a bucket hash of all the emoticons
	// Don't return false if this fails - we don't want to stop Eudora from
	// launching just because the user doesn't have an Emoticon directory.
	BuildEmoticonList();

	BuildHTMLEscapeCharList();

	//TestEmoticonMap();

	return true;
}

void EmoticonDirector::TestEmoticonMap()
{
	//replace all escape characters in trigger 
	for( POSITION pos = m_EmoticonMap.GetStartPosition(); pos != NULL; ) {
			CString key;
			EmoticonList* emoticonList;
			m_EmoticonMap.GetNextAssoc(pos, key, emoticonList);
			for( POSITION pos1 = emoticonList->GetHeadPosition(); pos1 != NULL; ) {
				Emoticon* emoticon = emoticonList->GetNext(pos1);
				CString trigger = emoticon->GetTrigger();
				CString meaning = emoticon->GetMeaning();
			}

	}
}

void EmoticonDirector::BuildEscapeCharacterMap()
{
	//build the escape characters map
	m_EscapeCharMap.SetAt("!21", "!");
	m_EscapeCharMap.SetAt("!22", "\"");
	m_EscapeCharMap.SetAt("!23", "#");
	m_EscapeCharMap.SetAt("!27", "'");
	m_EscapeCharMap.SetAt("!2a", "*");
	m_EscapeCharMap.SetAt("!2e", ".");
	m_EscapeCharMap.SetAt("!2f", "/");
	m_EscapeCharMap.SetAt("!3a", ":");
	m_EscapeCharMap.SetAt("!3c", "<");
	m_EscapeCharMap.SetAt("!3e", ">");
	m_EscapeCharMap.SetAt("!3f", "?");
	m_EscapeCharMap.SetAt("!40", "@");
	m_EscapeCharMap.SetAt("!5c", "\\");
	m_EscapeCharMap.SetAt("!7b", "{");
	m_EscapeCharMap.SetAt("!7c", "|");
	m_EscapeCharMap.SetAt("!7d", "}");
	m_EscapeCharMap.SetAt("!2A", "*");
	m_EscapeCharMap.SetAt("!2E", ".");
	m_EscapeCharMap.SetAt("!2F", "/");
	m_EscapeCharMap.SetAt("!3A", ":");
	m_EscapeCharMap.SetAt("!3C", "<");
	m_EscapeCharMap.SetAt("!3E", ">");
	m_EscapeCharMap.SetAt("!3F", "?");
	m_EscapeCharMap.SetAt("!5C", "\\");
	m_EscapeCharMap.SetAt("!7B", "{");
	m_EscapeCharMap.SetAt("!7C", "|");
	m_EscapeCharMap.SetAt("!7D", "}");
}

void EmoticonDirector::BuildImageExtensionList()
{
	m_ImageExtensionList.AddTail(CRString(IDS_IMAGE_EXTENSION_JPEG));
	m_ImageExtensionList.AddTail(CRString(IDS_IMAGE_EXTENSION_JPG));
	m_ImageExtensionList.AddTail(CRString(IDS_IMAGE_EXTENSION_PNG));
	m_ImageExtensionList.AddTail(CRString(IDS_IMAGE_EXTENSION_BMP));
	//m_ImageExtensionList.AddTail(".gif");

}

void EmoticonDirector::BuildHTMLEscapeCharList()
{
	m_HTMLEscapeCharMap.SetAt(CRString(IDS_HTML_LESSTHAN), "<");
	m_HTMLEscapeCharMap.SetAt(CRString(IDS_HTML_GREATERTHAN), ">");
	m_HTMLEscapeCharMap.SetAt(CRString(IDS_HTML_QUOTE), "\"");
	m_HTMLEscapeCharMap.SetAt(CRString(IDS_HTML_AMPERSAND), "&");
	//m_HTMLEscapeCharMap.SetAt(CRString(IDS_HTML_SPACE), " ");

}

void EmoticonDirector::BuildEmoticonList()
{
	//read in all the different image types from all the directories
	//first read the data dir and then the exec directory
	//since data directory emoticons override the exec directory emoticons
	//if we come across the same trigger and same size(16, 24, 32) in exec directory, do
	//not use the emoticon... just throw it away

	BuildEmoticonList(EudoraDir);
	BuildEmoticonList(ExecutableDir);

}

void EmoticonDirector::BuildEmoticonList(const CString & szDirPath)
{

	//read in all the emoticons in the data and exec directory.
	//we can have upto 3 sizes of emoticons 
	//16x16 in the Emoticons directory
	//24x24 in the Emoticons24 directory
	//32x32 in the Emoticons32 directory
	CString szEmoticonDir = szDirPath;
	
	szEmoticonDir += CRString(IDS_EMOTICON_DIRECTORY16) + SLASHSTR;
	if(::FileExistsMT(szEmoticonDir))
		BuildEmoticonsFromDirectory(szEmoticonDir, IMG_SIXTEEN);

	szEmoticonDir = szDirPath;
	szEmoticonDir += CRString(IDS_EMOTICON_DIRECTORY24) + SLASHSTR;
	if(::FileExistsMT(szEmoticonDir))
		BuildEmoticonsFromDirectory(szEmoticonDir, IMG_TWENTYFOUR);

	szEmoticonDir = szDirPath;
	szEmoticonDir += CRString(IDS_EMOTICON_DIRECTORY32) + SLASHSTR;
	if(::FileExistsMT(szEmoticonDir))
		BuildEmoticonsFromDirectory(szEmoticonDir, IMG_THIRTYTWO);

}


void EmoticonDirector::BuildEmoticonsFromDirectory(const CString & szDirPath, ImageSize imgSize)
{

	//read in all the different image formats that we support in Eudora from the directory
	//create emoticon objects and insert the same into the emoticon map / list
	for(int i=0; i<m_ImageExtensionList.GetCount(); i++) {

		CString& szImgExtn = m_ImageExtensionList.GetAt(m_ImageExtensionList.FindIndex(i));

		CString szImgPath = szDirPath + "*" + szImgExtn;

		WIN32_FIND_DATA	Find;
		HANDLE			FindHandle;
		Find.dwFileAttributes = _A_NORMAL;

		FindHandle = FindFirstFile(szImgPath, &Find);
		if (FindHandle != INVALID_HANDLE_VALUE) {
			int Result = 1;
				while (Result) {
					CString		szImageFullPath = szDirPath + Find.cFileName;
					Emoticon* emoticon = InsertEmoticonObjectInMap(Find.cFileName, szImageFullPath, szImgExtn, imgSize); // map is for parsing incoming emails
					if(emoticon) {
						// Alphabetically add to list of emoticons for the toolbar and menu
						POSITION pos = m_EmoticonList.GetHeadPosition();
						BOOL bAdded = FALSE;
						while (pos)
						{
							POSITION prevPos = pos;
							Emoticon* emoticonObj = m_EmoticonList.GetNext(pos);
							if (stricmp(static_cast<LPCTSTR>(emoticon->GetMeaning()), static_cast<LPCTSTR>(emoticonObj->GetMeaning())) < 0)
							{
								m_EmoticonList.InsertBefore(prevPos, emoticon);
								bAdded = TRUE;
								break;
							}
						}
						if (!bAdded)
							m_EmoticonList.AddTail(emoticon);
						m_EmoticonTriggerMap.SetAt(emoticon->GetTrigger(), emoticon);
					}
					Result = FindNextFile(FindHandle,&Find);
				}
		}

		FindClose(FindHandle);
	}
}

bool EmoticonDirector::ParseEmoticonName(const CString & szFileName, const CString & szFileFullPath, const CString & szImgExtn, CString & trigger, CString & meaning)
{
	//parse the incoming file name into trigger and meaning
	//<trigger><space><meaning>.<extension>
	//remove the extension from file name
	//replace all escape characters in trigger with the actual character
	//bIsSynonym means that sometimes when we have 2 different triggers for the same emoticons
	//such as :-) as well as :) represent the smiley. we display just one smiley in the toolbar
	//button but as we parse incoming emails or when composing both triggers are used to convert 
	//to emoticons. hence bIsSynonym means that it should not be added to the toolbar but it is
	//just a synonym trigger

	CString		szParseFileName = szFileName;

	if(szParseFileName.Replace(szImgExtn, "") == 0) {
		//replace the extension... might be an upper case extension
		CString szImageExtnUC = szImgExtn;
		szImageExtnUC.MakeUpper();
		szParseFileName.Replace(szImageExtnUC, "");
	}

	bool bIsSynonym = false;
	int spaceIndex = szParseFileName.Find(CRString(IDS_IMAGE_NAME_SEPARATOR));
	if(spaceIndex != -1) {
		trigger = szParseFileName.Left(spaceIndex);
		meaning = szParseFileName.Right(szParseFileName.GetLength() - (spaceIndex + 1));
	} else {
		trigger = szFileName;
	}
	
	//replace all escape characters in trigger 
	for( POSITION pos = m_EscapeCharMap.GetStartPosition(); pos != NULL; ) {
			CString key, value;
			m_EscapeCharMap.GetNextAssoc(pos, key, value);
			trigger.Replace(key, value);
	}

	//if trigger starts with '-', then it should not be displayed in the emoticon menu.
	if(trigger.GetAt(0) == m_emoticon_synonym) {
		trigger.Delete(0);
		bIsSynonym = true;
	}

	return bIsSynonym;
}

Emoticon* EmoticonDirector::BuildEmoticonObject(const CString & szFileFullPath, ImageSize imgSize, const CString & trigger, const CString & meaning, bool bIsSynonym)
{
	//build the emoticon object

	Emoticon* emoticon = NULL;
	try {
		switch(imgSize) {
			case IMG_SIXTEEN: {
				emoticon = DEBUG_NEW_MFCOBJ_NOTHROW Emoticon(this, szFileFullPath, "", "", trigger, meaning, bIsSynonym);
				break;
			} case IMG_TWENTYFOUR: {
				emoticon = DEBUG_NEW_MFCOBJ_NOTHROW Emoticon(this, "", szFileFullPath, "", trigger, meaning, bIsSynonym);
				break;
			} case IMG_THIRTYTWO: {
				emoticon = DEBUG_NEW_MFCOBJ_NOTHROW Emoticon(this, "", "", szFileFullPath, trigger, meaning, bIsSynonym);
				break;
			} default: {
				ASSERT(0);
				break;
			}
		}
	} catch (const InvalidEmoticonException *) { // if we passed a empty trigger while creating an emoticon object
		//log that emoticon trigger is invalid and cannot be used	
		//maybe display dialog to user also
	}
	return emoticon;
}

Emoticon* EmoticonDirector::InsertEmoticonObjectInMap(const CString & szFileName, const CString & szFileFullPath, const CString & szImgExtn, ImageSize imgSize)
{
	//get the first character from the emoticon trigger
	//get the emoticon list associated with the first trigger character from the emoticon map
	//if emoticon list is not present, then create the emoticon list
	//insert the emoticon object in the emoticon list in order of the trigger length
	//finally insert the emoticon list in the map
	//note we do not always create an emoticon object. we create only a single emoticon
	//for a given trigger even if we have different sizes 16, 24 and 32.
	//hence when we come across an emoticon in the list with the same trigger as the emoticon to be inserted 
	//rather than creating a new emoticon object for that trigger we just update the path in the exisiting emoticon

	CString trigger;
	CString meaning;
	bool bIsSynonym = ParseEmoticonName(szFileName, szFileFullPath, szImgExtn, trigger, meaning);

	Emoticon * emoticon = NULL;
	CString key(trigger[0]);
	EmoticonList* pEmoticonList = NULL;
	if(m_EmoticonMap.Lookup(key, pEmoticonList)) { // key exists... insert the emoticon object into the emoticon list
		const int triggerLength = trigger.GetLength();
		for(POSITION pos=pEmoticonList->GetHeadPosition(); pos!=NULL; ) {
			POSITION prevPosn = pos;
			Emoticon* emoticonObj = pEmoticonList->GetNext(pos);
			if(triggerLength > emoticonObj->GetTriggerLength()) { 
				emoticon = BuildEmoticonObject(szFileFullPath, imgSize, trigger, meaning, bIsSynonym);
				if(emoticon)
					pEmoticonList->InsertBefore(prevPosn, emoticon);
				break;
			} else if( (triggerLength == emoticonObj->GetTriggerLength()) && (strcmp(static_cast<LPCTSTR>(trigger), static_cast<LPCTSTR>(emoticonObj->GetTrigger())) == 0) ) { // emoticon with same trigger already exists... just update the path in the existing emoticon object
				switch(imgSize) {
					case IMG_SIXTEEN: {
						if(emoticonObj->GetImageFullPath16().IsEmpty())
							emoticonObj->SetImageFullPath16(szFileFullPath);
						break;
					} case IMG_TWENTYFOUR: {
						if(emoticonObj->GetImageFullPath24().IsEmpty())
							emoticonObj->SetImageFullPath24(szFileFullPath);
						break;
					} case IMG_THIRTYTWO: {
						if(emoticonObj->GetImageFullPath32().IsEmpty())
							emoticonObj->SetImageFullPath32(szFileFullPath);
						break;
					} default: {
						ASSERT(0);
						break;
					}
				}
				break;
			} else if(!pos) { // reached end of list. so insert at the end of the list here
				emoticon = BuildEmoticonObject(szFileFullPath, imgSize, trigger, meaning, bIsSynonym);
				if(emoticon)
					pEmoticonList->InsertAfter(prevPosn, emoticon);
				break;
			}
		}

	} else { // key does not exist... create the emoticon list and then insert the emoticon object
		pEmoticonList = DEBUG_NEW EmoticonList();
		emoticon = BuildEmoticonObject(szFileFullPath, imgSize, trigger, meaning, bIsSynonym);
		if(emoticon) {
			pEmoticonList->AddTail(emoticon);
			m_EmoticonMap.SetAt(key, pEmoticonList);
		}
	}

	return emoticon;
}

bool EmoticonDirector::CheckTextForTrigger(const char * szText, int nTextLength, const char * pStartTextBuffer, const char * pEndTextBuffer, Emoticon* pEmoticonObj, int & nTextReplaceLength, bool bIsHTML, bool bCheckPreviousChar/*=true*/ )
{

	if( !szText || !pEmoticonObj) {
		ASSERT(0);
		return false;
	}
	
	bool bFoundTrigger = false;
	if(!bIsHTML) {

		//don't convert trigger to emoticon if preceeded or followed by an alphabet or number
		//however special case, if the alphabet/number belongs to another trigger, then we convert the trigger
		//to an emoticon
		//need to check only following character belongs to another trigger and no need to check if previous
		//character belongs to another trigger bcos we would have already converted the previous trigger into an emoticon
		//boolean - bCheckPreviousChar is required bcos when we recursively call the function to check if the
		//character following a trigger belongs to another trigger ex
		//=-OO:-) here the first trigger is =-O and second trigger is O:-)
		//here when we check trigger =-O, we want to make sure that O:-) is another trigger
		//we recurse this function for O:-). When we are checking for O:-), we do not want to
		//test the previous character for an alpbhabet / number, since we will end up testing =-O, which we are currently converting.
		if( (szText > pStartTextBuffer) && bCheckPreviousChar ) {
			unsigned char prevChar = *(szText-1);
			if( (isdigit(prevChar) != 0) || (isalpha(prevChar) != 0) )
				return false;
		}

		int triggerLength = pEmoticonObj->GetTriggerLength();
		if( (szText+triggerLength) <= pEndTextBuffer ) {
			unsigned char nextChar = *(szText+triggerLength);	
			if( (isdigit(nextChar) != 0) || (isalpha(nextChar) != 0) ) {
				
				int nDummyLength = 0;
				Emoticon* nextEmoticonObj = GetMatchingEmoticon( (szText+triggerLength), pStartTextBuffer, pEndTextBuffer, nDummyLength, bIsHTML, false);
				if(!nextEmoticonObj)
					return false;

			}
		}

		bFoundTrigger = (strncmp(szText, pEmoticonObj->GetTrigger(), triggerLength) == 0);
	} else {
		//HTML email contains escape characters
		//we want to convert the escape characters in the word and check if it matches a trigger
		//we also don't want to modify the original string since there might not be a match and we have to convert them back, 
		//so we create a temp string, check if it contains a trigger. if the converted temp string indeed matches a trigger
		//we keep track of the length of the original temp string including the escape characters that matches the trigger 
		//this length is used to convert the original string trigger into the image tag

		//loop through the length of the trigger until either 
		// - the trigger characters are done matching(match found) 
		// - the end of the word is reached (no match found)
		//check if the character in the word is a HTML escape character, if so convert to value
		//compare the char in the word against the char in the trigger, if equal 
		//keep track of the length of the escape characters

		int i = 0;
		int index = 0;
		int triggerLength = pEmoticonObj->GetTrigger().GetLength();

		while( index < triggerLength ) {

			int incrementLength = 1;

			if(i==nTextLength) {
				bFoundTrigger = false;
				break;
			}
	
			char ch = szText[i];

			//first character since bFoundTrigger is false
			//if character before trigger is alphabet or number do not convert trigger
			if(!bFoundTrigger && (szText > pStartTextBuffer) && bCheckPreviousChar) { 
				unsigned char prevChar = *(szText-1);
				if( (isdigit(prevChar) != 0) || (isalpha(prevChar) != 0) )
					break;
			}

			if( szText[i] == '&' ) {

				if ( szText[i+1] == '#' )	{
					// get the number string	
					for( int len = 2; ( (i+len) < nTextLength ) && isdigit( (int)(unsigned char)(szText[i+len]) ); len++ );	
					// convert the number to a character
					ch = ( char ) ( atoi( static_cast<LPCTSTR>(szText)+i ) & 0xFF );
					incrementLength = (len - 1); // -1 since len gets incremented extra before exiting the loop

				} else {
		
					for( POSITION pos = m_HTMLEscapeCharMap.GetStartPosition(); pos != NULL; ) {
						CString key, value;
						m_HTMLEscapeCharMap.GetNextAssoc(pos, key, value);
						int keyLength = key.GetLength();
						if( (keyLength <= nTextLength) && (strncmp( static_cast<LPCTSTR>(szText)+i, static_cast<LPCTSTR>(key), keyLength) == 0) ) {
							ch = value[0];
							incrementLength = keyLength;
							break;
						}
					}
				}

			}
			
			if(ch == pEmoticonObj->GetTrigger().GetAt(index)) {
				bFoundTrigger = true;
			} else {
				bFoundTrigger = false;
				break;
			}
			
			i += incrementLength;
			index++;

			//last character to test since we have reached end of trigger
			//if character after trigger is an alpbhabet or number, do not convert the trigger to alpbhabet
			//however special case, if the alphabet/number belongs to another trigger, then we convert the trigger
			//to an emoticon
			if( (index == triggerLength) && ((szText+i) <= pEndTextBuffer) && bFoundTrigger ) {
				unsigned char nextChar = szText[i];
				if( (isdigit(nextChar) != 0) || (isalpha(nextChar) != 0) ) {
					
					bFoundTrigger = false;

					int nDummyLength = 0;
					Emoticon* nextEmoticonObj = GetMatchingEmoticon( (szText+i), pStartTextBuffer, pEndTextBuffer, nDummyLength, bIsHTML, false);
					if(nextEmoticonObj)
						bFoundTrigger = true;
				}
			}


		}

		if( ( i != 0 ) && bFoundTrigger)
			nTextReplaceLength = i;//i gets incremented again but that is okai since we are equating the index to the length, length = index + 1
		
	}

	return bFoundTrigger;
}

Emoticon* EmoticonDirector::GetMatchingEmoticon(const char * pCheckTextHere, const char * pStartTextBuffer, const char * pEndTextBuffer, int & nTextReplaceLength, bool bIsHTML, bool bCheckPreviousChar/*=true*/)
{
	bool			bFoundTrigger = false;
	Emoticon*		pEmoticonObj = NULL;
	EmoticonList*	pEmoticonList = NULL;
	
	//the first character in the trigger might be an HTML escape character
	//convert it and then check if it matches any of the trigger's first char
	CString szChar = CString(*pCheckTextHere);

	if (*pCheckTextHere == '&')
		ConvertIfHTMLEscapeChar(szChar, pCheckTextHere, pEndTextBuffer);

	if(m_EmoticonMap.Lookup(szChar, pEmoticonList)) { // At least one trigger starting with the indicated char
		for(POSITION pos=pEmoticonList->GetHeadPosition(); (pos != NULL) && !bFoundTrigger; ) {
			pEmoticonObj = pEmoticonList->GetNext(pos);
			if( ((pEndTextBuffer-pCheckTextHere) >= pEmoticonObj->GetTrigger().GetLength()) &&
				( CheckTextForTrigger(pCheckTextHere, pEndTextBuffer-pCheckTextHere, pStartTextBuffer, pEndTextBuffer, pEmoticonObj, nTextReplaceLength, bIsHTML, bCheckPreviousChar) ) ) {
				// found the trigger match
				bFoundTrigger = true;
				break;
			}
		}
	}

	return bFoundTrigger ? pEmoticonObj : NULL;
}


void EmoticonDirector::ConvertIfHTMLEscapeChar(CString &szChar, const char * pCheckTextHere, const char * pEndTextBuffer) 
{
	CString		key, value;
	
	for( POSITION pos = m_HTMLEscapeCharMap.GetStartPosition(); pos != NULL; ) {
		m_HTMLEscapeCharMap.GetNextAssoc(pos, key, value);
		int keyLength = key.GetLength();
		if( (pEndTextBuffer-pCheckTextHere+1) >= keyLength ) {
			if(strncmp(pCheckTextHere, key, keyLength) == 0) {
				szChar = value;
				break;
			}
		}
	}

}


bool EmoticonDirector::ParseWordForEmoticons(CString &szWord, bool bIsHTML/*=false*/) 
{
	//go through each letter in the word
	//see if that letter matches any of the emoticon triggers first char
	//if there is a match go through the list and see if the entire trigger matches
	//if a match is found, replace the trigger with the path to the image

	bool			bFoundTrigger = false;
	int				i = 0;
	int				nTextReplaceLength = 0;

	while ( (i < szWord.GetLength()) ) {

		//moved it inside the loop since the word length keeps changing
		const char *	pStartWord = static_cast<LPCTSTR>(szWord);
		const char *	pEndWord = pStartWord + szWord.GetLength();
		
		Emoticon* emoticonObj = GetMatchingEmoticon(static_cast<LPCTSTR>(szWord)+i, pStartWord, pEndWord, nTextReplaceLength, bIsHTML);

		if (emoticonObj) {
			bFoundTrigger = true;
			
			if(nTextReplaceLength == 0)
				szWord.Delete(i, emoticonObj->GetTriggerLength());
			else
				szWord.Delete(i, nTextReplaceLength);

			CString szImagePath;
			szImagePath += "<img src=\"file://";
			szImagePath += emoticonObj->GetImageFullPath();

			// Close the quotes around the src file path, and set the alignment
			// to be absmiddle. absmiddle is deprecated non-standard HTML,
			// but it does what we want and it is recognized by most browsers
			// including IE. Some day we may want to redo this using styles.
			//
			// Using align=middle would not do what we want - middle aligns the
			// center of the image with the base line of the current text line.
			// In contrast align=absmiddle aligns the center of the image with
			// the *center of the current text line*.
			szImagePath += "\" align=absmiddle";

			if(!emoticonObj->GetTrigger().IsEmpty()) {
				szImagePath += " alt=\"";
				szImagePath += emoticonObj->GetTrigger();
				szImagePath += "\"";
			}

			szImagePath += " eudora=emoticon>";
			szWord.Insert(i, szImagePath);
			i += (szImagePath.GetLength() - 1); //gets incremented again by 1 below
		}
		
		i++;
	}

	return bFoundTrigger;
}

EmoticonDirector::~EmoticonDirector()
{
	//delete the emoticon lists... we have stored pointers to the loaded images... delete them 
	for(POSITION pos=m_EmoticonList.GetHeadPosition(); pos!=NULL; ) {
		Emoticon* emoticonObj = m_EmoticonList.GetNext(pos);
		delete emoticonObj;
	}

	for( pos = m_EmoticonMap.GetStartPosition(); pos != NULL; ) {
			CString key;
			EmoticonList* emoticonList;
			m_EmoticonMap.GetNextAssoc(pos, key, emoticonList);	
			delete emoticonList;
	}
}

void EmoticonDirector::SetEmoticonIDMap(WORD wID, Emoticon* emoticonObj) 
{
		m_EmoticonIDMap.SetAt(wID, emoticonObj);
}

void EmoticonDirector::LookUpEmoticonIDMap(WORD wID, Emoticon* & emoticonObj) 
{
		m_EmoticonIDMap.Lookup(wID, emoticonObj);
}

void EmoticonDirector::LookUpEmoticonTriggerMap(CString &trigger, Emoticon* & emoticonObj) 
{
		m_EmoticonTriggerMap.Lookup(trigger, emoticonObj);
}

BOOL InvalidEmoticonException::GetErrorMessage( LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext /*= NULL*/ ) {

	strcpy(lpszError, "Emoticon cannot be created without a valid trigger");
	return true;
}


