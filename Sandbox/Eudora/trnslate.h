#ifndef TRNSLATE_H
#define TRNSLATE_H

#include <stdarg.h>

#ifndef EMSR_OK
#include "ems-wglu.h"
#endif
 
class QCProtocol;

// =============================================================================  
//    trnslate.h  -- Eudora translator API classes
//											   
//	This defines two classes, one for the API itself and one for 
//	individual translators.
//
//   	Note: The types that start with "ems_" and most that start with "tl" all come
//   	from tlapi.h which should be included before this is included.
//									
// 

//Some Defines
#define EMS_PLUGIN_VERSION	"ems_plugin_version"
#define EMS_PLUGIN_INIT		"ems_plugin_init"
#define EMS_TRANS_INFO		"ems_translator_info"
#define EMS_CAN_TRANS		"ems_can_translate"	
#define EMS_TRANS_FILE		"ems_translate_file"
#define EMS_TRANS_BUF		"ems_translate_buf"
#define EMS_PLUGIN_FINISH	"ems_plugin_finish"
#define EMS_FREE			"ems_free"
#define EMS_PLUGIN_SETTINGS	"ems_plugin_config"
#define EMS_QUEUE_TRANS		"ems_queued_properties"
#define EMS_ATTACH_INFO		"ems_attacher_info"
#define EMS_ATTACH_HOOK		"ems_attacher_hook"
#define EMS_SPECIAL_INFO	"ems_special_info"
#define EMS_SPECIAL_HOOK	"ems_special_hook"
#define EMS_IDLE			"ems_idle"
#define EMS_MBOX_CONEXT_INFO	"ems_mbox_context_info"
#define EMS_MBOX_CONEXT_HOOK	"ems_mbox_context_hook"

#define PARAM_VERSION	(2)

#define ADD_BUF_SIZE	3000

// =======================================================================
// CTLAddress
// This contains address message info need for a translation
// =======================================================================

// Forwarard Decl
class CCompMessageDoc;
class URLEditType;
class CCompMessageView;

class CTLAddress
{
public:
	CTLAddress();
	virtual ~CTLAddress();

private:
	BOOL CreateAddressList();
	BOOL AddEntry(const char*address, const char*name, const char*nick);
	BOOL AddRecipients(const char *recLine, emsAddressP *field);
	
	emsAddressP CreateEntryV3(const char*address, const char*name);
	void RemoveEntryV3(emsAddressP entry);

public:
	// Deal with address lists
	BOOL CreateAddressList(CDocument* msg);
	BOOL CreateAddressList(const char *to, const char *from, const char *subject, const char *cc, const char *bcc, const char *rawHeaders);
	BOOL CleanAddressList();
	char **GetAddressList() {return m_AddressList;}

	emsHeaderData m_HeaderData;			// This is the new V3 Address PB

private:
	char		*m_AddressList[ADD_BUF_SIZE];	// This will hold AddressList
	int		    m_AddressEnd;					// This is the end of the Address list


};

// =============================================================================  
class CEMSMailConfig
{
public:
	CEMSMailConfig();
	virtual ~CEMSMailConfig();
	
	void SetHWnd(HWND *wnd)	{ m_Config.eudoraWnd = wnd; }
	operator emsMailConfig() const	{ return m_Config; }

private:
	emsMailConfig	m_Config;
};

// =============================================================================  
class CEMSPluginInfo
{
public:
	CEMSPluginInfo();
	virtual ~CEMSPluginInfo(){};
	operator emsPluginInfo() const { return m_PluginInfo; }

	emsPluginInfo	m_PluginInfo;
};

// =============================================================================  
class CEMSTransInfo
{
public:
	CEMSTransInfo(long id);
	virtual ~CEMSTransInfo(){};
	operator emsTranslator() const { return m_TransInfo; }

	emsTranslator	m_TransInfo;
};

// =============================================================================  
class CEMSDataFile
{
public:
	CEMSDataFile(long context);
	CEMSDataFile(long context, const char* filename);
	virtual ~CEMSDataFile();

	void AddHeaders(emsHeaderData *headerData) {m_DataFile.header = headerData;}
	void AddMIME(emsMIMEtype3 *mimeData);// {m_DataFile.info = mimeData;}
	operator emsDataFile() const { return m_DataFile; }
	void DestroyMimeInfo();
	void DestroyHeaderInfo();
	void emptyAddresses(emsAddressP entry);
	emsDataFile	m_DataFile;
};

// =============================================================================  
class CEMSResultStatus
{
public:
	CEMSResultStatus();
	virtual ~CEMSResultStatus();
	operator emsResultStatus() const { return m_ResultStatus; }

	emsResultStatus	m_ResultStatus;
};

// =============================================================================  
class CEMSCallbacks
{
public:
	CEMSCallbacks();
	virtual ~CEMSCallbacks(){};
	operator emsCallBack() const { return m_Callbacks; }
	operator emsCallBack *() { return &m_Callbacks; }

	emsCallBack	m_Callbacks;
};


// =============================================================================  
class CEMSIdleData
{
public:
	CEMSIdleData();
	virtual ~CEMSIdleData(){};
	operator emsIdleData() const { return m_IdleData; }

	emsIdleData	m_IdleData;
};

// =============================================================================  
//   This is the translator API class
//
//   It manages the modules/DLL's and the translators in them:
//      - When the constructor is called it looks for all possible translators
//      - It manages which translator DLL's are loaded or not. Currently all
//        stay loaded and it keep track of their handles
//      - Provides a way to select translators by type and function 
// 
//   For a detailed look at the Translation API itself see tlapi.h.
// 
class CTransAPI 
{
public:
    // The constructor for the API interface
	CTransAPI(const char *path,				// Path to DLL
				HINSTANCE libHndl,			// DLL Instance
				short moduleID,				// ID
				short APIVersion,
				char *desc,
				HICON *icn,
				void FAR*globals,
				long idleFreq):				// Initialize member in contructor
				m_Path(path), 
				m_HInstance(libHndl), 
				m_ModuleID(moduleID), 
				m_APIversion(APIVersion),
				m_Desc(desc),
				m_HIcon(icn),
				m_Globals(globals),
				m_fnFreeFunc(NULL),
				m_IdleFreq(idleFreq),
				m_LastTimeCalled(0)
				{}

	
	virtual ~CTransAPI();

	// This is only called by a Translator object.  It is used
	// to request the Translators DLL be loaded (if not already)
	// and to get a pointer to a function to be called to perform
	// some translator operation. It is called with an OP code
	// that specifies the function. The OP codes and function proto
	// types are give in tlapi.h
	short (*GetFunction(LPSTR))(...);			      

	// Informational functions
	HINSTANCE	GetInstance()	{return m_HInstance;}
	void FAR*	GetGlobals()	{return m_Globals;}
	
	void GetDesc(CString &str)	{str = m_Desc;}
	BOOL GetIcon(HICON &icn);
	long GetID()				{return m_ModuleID;}
	short GetVersion()			{return m_APIversion;}
	
	FARPROC GetSettingsFunc();
	BOOL Settings();
	
	FARPROC GetIdleFunc();
	BOOL IdleFunc(const long idleTime, const long idleFlag);
	
	ems2_free_t					*m_fnFreeFunc;

private:
	CString		 m_Path;			// File names of DLL's for each translator
    HINSTANCE    m_HInstance;		// Handles to the DLL's for each translator module
	short        m_ModuleID;
	short        m_APIversion;
	char		 FAR*m_Desc;		// The translator description
	HICON		 FAR*m_HIcon;		// The translator Icon   
	void		 FAR*m_Globals;		// This is the globals that gets passed around
	DWORD		 m_IdleFreq;
	DWORD		 m_LastTimeCalled;
};									


// =======================================================================
// CTLMime
// This contains mime message info need for a translation
// =======================================================================
class CTLMime
{
public:
	CTLMime();
	virtual ~CTLMime();

private:
	emsMIMEtype3 m_MimeType3;

public:
 	CleanMimeType();

	BOOL CreateMimeType(emsMIMEtype3 *MimeType);
    BOOL CreateMimeType(BOOL isRich = FALSE, BOOL HasAttach = FALSE);
    BOOL CreateMimeType(const char *filename);
	
	BOOL ChangeInfo(const char *mimeVersion, const char *type, const char *subtype, const char *contentDisp = NULL);
    BOOL AddParam(const char *name, const char *value, emsMIMEparam3 *param = NULL);
    BOOL AddContentParam(const char *name, const char *value);
 	
	emsMIMEtype3 *GetMimeType() {return &m_MimeType3;}
	const char *GetVersion() const { return m_MimeType3.version; }
	const char *GetType() const { return m_MimeType3.type; }
	const char *GetSubtype() const { return m_MimeType3.subType; }
	const emsMIMEparam3 *GetParams() const { return m_MimeType3.params; }

	BOOL CTLMime::FreeMimeTypeCTL();

	emsMIMEtype * DegradeMimeType();
	void UpgradeMimeType(emsMIMEtypeP oldStyleMimeType);

};
// Use this to clean mime type from api
BOOL FreeMimeTypeV2(emsMIMEtype *mimeType, ems2_free_t *freeFunc = NULL); //use delete if no free function
BOOL FreeMimeTypeV3(emsMIMEtype3 *mimeType, ems2_free_t *freeFunc = NULL);//use delete if no free function







// =============================================================================  
//
//   Basic CTranslator class
//
//   This represents an individual translator and can perform
//   translations and returns information about itself. 
//
//   It uses the with the transAPI class which manages the translator dll's,
//   the loading, locating and selecting of translators.
//

class CTranslator : public CObject
{
public:
    // The Constructor for a particular translator
	// The constructor is called by TransAPI as it goes through
	// the translators in all the modules it discovers.
	CTranslator(CTransAPI * pAPI,short transID);
	BOOL InitTranslator();
	BOOL UpdateTranlator();

	virtual ~CTranslator();
	
	// Open the file and make sure it's in some sort of parsable MIME format. If it's not then we want to treat it as t/p
	void EnforceMIMEFormatting(char* filename);

	// Is this translator of this type?
	BOOL matchType(const long);	          // The translator type TLT_xxx from tlapi.h
	
	// Is this translator of a type in a context?
	// This is called by the TransAPI class when selecting by type
	BOOL matchType(const long type, const unsigned long  flags);
	
	// This is called by the TransAPI class when selecting by type
	BOOL matchContext(const unsigned long	flags);     // Bit field to match translator
                
	// The following members are used for basics of 
	// displaying and actually using the translator
    BOOL getDescription(CString& desc);

	// Get the bitmap for the translator
	BOOL GetIcon(HICON &icn);

	void  GetID(long &ModuleID, long &TransID) 
		{ ModuleID = m_TransAPI->GetID(); TransID = m_TransID; }
	
	long  GetType() { return m_Type; }
	long  GetFlags() { return m_TLflags; }

	// Handy Error Function
	BOOL	TranslatorError(const long funReturn, const char *error, const long errorCode);

	// This will actually do the translation
	long	Translate(CTLMime *mimeInfo, const long context, CTLAddress *addInfo, const char *raw, const char *cooked, long *retCode);
	virtual long	TranslateMessage(CWnd* edtCtrl, CTLAddress* addInf = NULL);

	virtual long TranslateMessage(
	QCProtocol*	pProtocol,
	CTLAddress* paddInfo);
	

	BOOL	QueueTrans(const long context, CString& properties, BOOL state = FALSE); 
    // These go right to the dll	
	long CanTranslate(long context,
							CTLMime *in_mime, 
							CTLAddress *addresses);
	
	long TranslateFile(long context,
						CTLMime *in_mime, 
						const char *in_file,        
						CTLAddress *addresses,     
						const char *outputFile,
						CTLMime *out_mime,
						HICON **out_icon,	  
						char **out_desc,
						long *out_code);

	virtual IsInYerFace() { return (m_TLflags & EMSF_TOOLBAR_PRESENCE);}


private:

	CTransAPI		*m_TransAPI;		// Reference to get at the DLL
	short			m_TransID;			// Which translator is it in the module?
	long			m_Type;				// Translator type (TLT_xxx from tlapi.h)
	long			m_Subtype;			// The subtype
	unsigned long	m_TLflags;			// The flags (TLF_xxx from tlapi.h)
	char			FAR*m_Desc;			// The translator description
	HICON			FAR*m_HIcon;		// The translator Icon   

public:

	CString			m_Properties;	// This temporary per message
};	


// =============================================================================  
class CAttacher : public CObject
{
public:
	CAttacher() : m_TransAPI(NULL){};
	CAttacher(CTransAPI *pTAPI, long ID);
	virtual ~CAttacher();

	virtual operator emsMenu() const{ return m_MenuItem; }
	
	virtual void  GetID(long &ModuleID, long &AttachID) 
	{ ModuleID = m_TransAPI->GetID(); AttachID = m_MenuItem.id; }

	virtual GetIcon(HICON &icn);
	virtual IsInYerFace() { return (m_MenuItem.flags & EMSF_TOOLBAR_PRESENCE);}



	long virtual MenuHook(CCompMessageDoc* msg);

protected:
	CTransAPI		*m_TransAPI;		// Reference to get at the DLL
	emsMenu			m_MenuItem;
};	

// =============================================================================  
class CSpecial : public CAttacher
{
public:
	CSpecial(CTransAPI *pTAPI, long ID);
	long virtual MenuHook(CCompMessageDoc* msg);
	virtual GetIcon(HICON &icn);
};	

// Forward decl.
class CTocDoc;

// =============================================================================  
class CMBoxContext : public CAttacher
{
public:
	CMBoxContext(CTransAPI *pTAPI, long ID);
	long virtual ContextHook(CTocDoc* toc);
};	



// =============================================================================  
class CAttachArray : public CPtrArray
{ 
public: 
	CAttacher *GetAt(int index) 
			{ return ((CAttacher*)CPtrArray::GetAt(index)); } 
			
	void InsertAt(int index, CAttacher *attachMenu) 
			{ CPtrArray::InsertAt(index, (CObject *)attachMenu); } 
};

// =============================================================================  
class CSpecialArray : public CPtrArray
{ 
public: 
	CSpecial *GetAt(int index) 
			{ return ((CSpecial*)CPtrArray::GetAt(index)); } 
			
	void InsertAt(int index, CSpecial *specialMenu) 
			{ CPtrArray::InsertAt(index, (CObject *)specialMenu); } 
};

// =============================================================================  
class CMboxCMArray : public CPtrArray
{ 
public: 
	CMBoxContext *GetAt(int index) 
			{ return ((CMBoxContext*)CPtrArray::GetAt(index)); } 
			
	void InsertAt(int index, CMBoxContext *mboxCM) 
			{ CPtrArray::InsertAt(index, (CObject *)mboxCM); } 
};

// =============================================================================  
class CTranslatorArray : public CPtrArray
{ 
public: 
	CTranslator *GetAt(int index) 
			{ return ((CTranslator*)CPtrArray::GetAt(index)); } 
			
	void InsertAt(int index, CTranslator *trans) 
			{ CPtrArray::InsertAt(index, (CObject *)trans); } 
};


// =============================================================================  
class CTranslatorSortedList : public CObList
{ 
public: 
	CTranslator *GetHead()
		{ return (CTranslator*)CObList::GetHead(); } 
	
	POSITION AddHead(CTranslator * newElement)
		{return CObList::AddHead((CObject*)newElement);}

	POSITION InsertBefore( POSITION position, CTranslator* newElement )
		{ return CObList::InsertBefore(position, (CObject *)newElement); } 

	POSITION InsertAfter( POSITION position, CTranslator* newElement )
		{ return CObList::InsertAfter(position, (CObject *)newElement); } 

	CTranslator * GetAt( POSITION position  )
		{ return (CTranslator *)CObList::GetAt(position); } 

	CTranslator * GetNext( POSITION &position  )
		{ return (CTranslator *)CObList::GetNext(position); } 

	CTranslator * GetPrev( POSITION &position  )
		{ return (CTranslator *)CObList::GetPrev(position); } 

	POSITION Find(CTranslator * searchValue, POSITION startAfter = NULL ) const
		{ return CObList::Find((CObject *)searchValue, startAfter);}
  
	void Add( CTranslator * trans);
	CTranslator *FindMatch(const long type, const long moduleID = 0);
};



// =============================================================================  
class CTransAPIArray : public CPtrArray
{  
public: 
	CTransAPI *GetAt(int index) 
			{ return ((CTransAPI*)CPtrArray::GetAt(index)); } 
			
	void InsertAt(int index, CTransAPI *trans) 
			{ CPtrArray::InsertAt(index, (CObject *)trans); } 
};


// =============================================================================  
//   This is the CTranslatorManager
//
//   It manages 
//		- the the list of modules/DLL's 
//      - the the list of translators 
//      - It manages which translator DLL's are loaded or not. Currently all
//        stay loaded and it keep track of their handles
//      - Provides a way to select translators by type and function 
// 

// Address Flags, whether translators want them or not
const WORD EMSAPI_ARRIVAL_BASIC	= 0x0001;
const WORD EMSAPI_ARRIVAL_RAW	= 0x0002;
const WORD EMSAPI_DISPLAY_BASIC	= 0x0004;
const WORD EMSAPI_DISPLAY_RAW	= 0x0008;
const WORD EMSAPI_XMISSION_BASIC= 0x0010;
const WORD EMSAPI_XMISSION_RAW	= 0x0020;
const WORD EMSAPI_REQUEST_BASIC	= 0x0040;
const WORD EMSAPI_REQUEST_RAW	= 0x0080;


class CTranslatorManager
{
public:
	CTranslatorManager(
		// constructor searches directory paths and loads DLL's
		short,                // number of paths to search
		...		              // NULL terminated list of paths
	);
	virtual ~CTranslatorManager();

	// This will be set only if there is at least one plugin requiring IDLE time
	static BOOL			m_PluginIdle;

private:
	CTranslatorArray	m_Translators;	// Pointers to all the translator objects
	CTransAPIArray		m_TranAPIs;		// Pointers to all the translator DLL objects

	CAttachArray		m_AttachItems;
	CSpecialArray		m_SpecialItems;
	CMboxCMArray		m_MboxCMItems;

	CTranslatorSortedList *m_SortedList;  // Cache a sorted list for a given context
	long m_SortedListContext;  
	CEMSMailConfig m_MailConfig;

	static WORD			m_HeaderFlags;	
	
public:
	BOOL LoadModule(const char *path, HINSTANCE libHndl);
	
	// Return All Translators DLL's
	CTransAPIArray *GetModules() {return &m_TranAPIs;}
	CTransAPI *GetModule(const long moduleID);
	
	// Return All Translators
	CTranslatorArray *GetTranslators() {return &m_Translators;}

	// Get A Specific Translator
	virtual CTranslator* GetTranslator(long moduleID, long TranslatorID);

	// Find Index of First Translator meeting criteria
	virtual short FindFirstTranslator(const long type = 0, const long context = 0, const char *desc = 0, short startAt = 0);
	
	// Find Next Translator meeting criteria
	virtual CTranslator* GetNextTranslator(short *index, const long type = 0, const long context = 0,  const char *desc = 0);

	// Get an ordered list by type, this will return a reference to and member varible..
	// don't delete what's returned
	CTranslatorSortedList *GetSortedTranslators(const char*sel, const long context = 0);
	CTranslatorSortedList *GetSortedTranslators(const long context = 0);
	
	// This is a function that see's if there's sign & encrypt seleceted == coalese
	void RemoveDupFuncs(CTranslatorSortedList *pSortedList);

	// Send a file and translator selection through translation
	int XLateMessageOut(const long context, CCompMessageDoc* msg, const char *raw, char *cooked);

	// Check if outgoing translation will be able to do it
	int CanXLateMessageOut(CCompMessageDoc* msg, UINT uType);

    // Check if translator ids from message header/body are good
    int ValidateTransIDs( const char*, const long, CString& );

	// Can tranlate on this mime type
	long CanXLateMessage(CTLMime *tlmime, long context, CTranslator **retTrans, CTLAddress *address = NULL);

	// Translate for ON_DISPLAY
	long XLateDisplay(QCProtocol* pXLateProtocol, const char *inFileCmd);

	// Interpret a MIME message and turn it into something Eudora can deal with
	long InterpretFile(const long context, 
							const char *inFile, 
							const char *outFileName, 
							CTLAddress *address,
							long *retCode,
							const char *firstTrans = NULL);

	// Strip off mime stuff
	long DeMimeMsg(const char *inFileName, const char *outFileName); 
	
	// Return All Attachers
	CAttachArray *GetAttachers() {return &m_AttachItems;}

	// Get A Specific Attachers
	virtual CAttacher *GetAttacher(long moduleID, long AttacherID);

	// Return All Special
	CSpecialArray *GetSpecials() {return &m_SpecialItems;}

	// Get A Specific Special
	virtual CSpecial* GetSpecial(long moduleID, long AttacherID);

	// Return All MboxContextMenu
	CMboxCMArray *GetMboxCMs() {return &m_MboxCMItems;}

	// Get A Specific MboxContextMenu
	virtual CMBoxContext* GetMboxCM(long moduleID, long AttacherID);

	static WORD GetHeaderFlag() {return m_HeaderFlags;}
	static void SetHeaderFlag(WORD flag) {m_HeaderFlags |= flag;}

	BOOL IdleEveryone(const long idleTime, const long idleFlag);
	BOOL MboxContextMenu(CTocDoc *tocdoc, CRect &rect);

	BOOL GetDefOutTransString(CString& transString);
};




// Now the useful C routines
int TranslateLiveOut(CCompMessageDoc* msg);
int WriteLiveMessage(const char *fileName);
BOOL IsTranslatorAttachment(const char *attachLine);
void DebugLog(DWORD Mask, UINT Format, ...);
void MorphMHTML( char ** ppHTML );

#ifndef WIN32
#define DeleteFile remove
#endif

//================================================================
// Progress Stuff
//================================================================

#define EMS_PROGRESS_WAIT	-1L
#define EMS_PROGRESS_START	0L
#define EMS_PROGRESS_END	100L

extern CString g_ProgressMsg;
extern BOOL	g_ProgStarted;

#ifdef __cplusplus
extern "C" {
#endif

short EMSProgress(short);					/* The progress function  */
short EMSV3Progress(emsProgressDataP);		/* The V3 progress function  */
short EMSGetMailBox(emsAllocate allocFunc, emsGetMailBoxDataP pMboxInfo);
short EMSSetMailBoxTag(emsSetMailBoxTagDataP pMboxTagInfo);    
short EMSGetPersonality(emsAllocate allocFunc, emsGetPersonalityDataP pPersonaData);
short EMSRegenerate(emsRegenerateDataP pRegenFile);
short EMSGetDirectory(emsGetDirectoryDataP pGetDirData);
short EMSGetMailBoxTag(emsSetMailBoxTagDataP pMboxTagInfo);
short EMSGetPersonalityInfo(emsAllocate allocFunc, emsGetPersonalityInfoDataP pPersonaInfoData);
ABDataP EMSEnumAddressBooks();
short EMSAddABEntry(emsNicknameP srcNickname, LPSTR NicknameFileName);

#ifdef __cplusplus
}
#endif

#endif
