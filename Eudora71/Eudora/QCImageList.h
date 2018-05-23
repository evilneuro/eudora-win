
#ifndef _QCImageList_h_
#define _QCImageList_h_

class QCImageList : public CImageList
{
public:
	~QCImageList();
	bool LoadEmpty(int cx, int cy, int nGrow);
	bool Load(int cx, int cy, int nGrow, LPCTSTR BmpResource, COLORREF xcolor = RGB(192, 192, 192));
	void Free(void);

	void Draw(int Index, int x, int y, CDC* pdc);			// Eudora-specific overload
	BOOL Draw(CDC* pDC, int nImage, POINT pt, UINT nStyle)	// pass-thru to base class method
		{ return CImageList::Draw(pDC, nImage, pt, nStyle); }

protected:
};

class QCMailboxImageList : public QCImageList
{	
public:
	QCMailboxImageList() {}
	virtual ~QCMailboxImageList() {}
	bool Load(void);

	enum ImageOffset
	{
		IMAGE_EUDORA = 0,
		IMAGE_CLOSED_FOLDER,
		IMAGE_OPEN_FOLDER,
		IMAGE_NORMAL_MBOX,
		IMAGE_UNREAD_MBOX,
		IMAGE_IN_MBOX,
		IMAGE_OUT_MBOX,
		IMAGE_RECYCLE_MBOX,
		IMAGE_TRASH_MBOX,
		IMAGE_JUNK_MBOX,
		IMAGE_IN_MBOX2,
		IMAGE_OUT_MBOX2,
		IMAGE_STATIONERY,
		IMAGE_SIGNATURE,
		IMAGE_PERSONALITY,
		IMAGE_FILTER_REPORT,
		IMAGE_IMAP_ACCOUNT,
		IMAGE_IMAP_MBOX,
		IMAGE_IMAP_MBOX_AUTOSYNC,
		IMAGE_IMAP_MBOX_READ,
		IMAGE_IMAP_MBOX_READ_AUTOSYNC,
		IMAGE_IMAP_FOLDER,
		IMAGE_IMAP_FOLDER_AUTOSYNC,
		IMAGE_IMAP_FOLDER_READ,
		IMAGE_IMAP_FOLDER_READ_AUTOSYNC,
		IMAGE_IMAP_FOLDER_OPEN,
		IMAGE_IMAP_FOLDER_OPEN_AUTOSYNC,
		IMAGE_IMAP_FOLDER_OPEN_READ,
		IMAGE_IMAP_FOLDER_OPEN_READ_AUTOSYNC,
		IMAGE_RECIPIENT
	};
};

class QCCommon16ImageList : public QCImageList
{	
public:
	bool Load(void);

	enum ImageOffset
	{
		IMAGE_MAILBOXES = 0,
		IMAGE_ADDRESS_BOOK,
		IMAGE_FILES,
		IMAGE_DIR_SERVICE,
		IMAGE_FILTER,
		IMAGE_FILTER_REPORT,
		IMAGE_PERSONA,
		IMAGE_SIGNATURE,
		IMAGE_STATIONARY,
		IMAGE_LOWEST,
		IMAGE_LOW,
		IMAGE_NORMAL,
		IMAGE_HIGH,
		IMAGE_HIGHEST,
		IMAGE_SORT_READ_STATUS,
		IMAGE_SORT_PRIORITY,
		IMAGE_SORT_ATTACH,
		IMAGE_SORT_LABEL,
		IMAGE_SORT_WHO,
		IMAGE_SORT_DATE,
		IMAGE_SORT_SIZE,
		IMAGE_SORT_SERVER_STATUS,
		IMAGE_SORT_SUBJECT
	};
};

class QCCommon32ImageList : public QCImageList
{	
public:
	bool Load(void);

	enum ImageOffset
	{
		IMAGE_NONE = 0,
	};
};

class QCTocImageList
{	
public:
	QCTocImageList();
	bool Load(void);
	void Free(void);
	void Draw(int Index, int x, int y, CDC* pdc, COLORREF color);

	enum ImageOffset
	{
		IMAGE_FORWARDED = 0,
		IMAGE_TIME_QUEUED,
		IMAGE_QUEUED,
		IMAGE_SENT,
		IMAGE_REDIRECTED,
		IMAGE_REPLIED,
		IMAGE_UNSENT,
		IMAGE_UNREAD,
		IMAGE_SENDABLE,
		IMAGE_READ,
		IMAGE_UNSENDABLE,
		IMAGE_SERVER_LEAVE,
		IMAGE_SERVER_FETCH,
		IMAGE_SERVER_DELETE,
		IMAGE_SERVER_FETCH_DELETE,
		IMAGE_SERVER_PARTIAL,
		IMAGE_ATTACH,
		IMAGE_LOWEST,
		IMAGE_LOW,
		IMAGE_NORMAL,
		IMAGE_HIGH,
		IMAGE_HIGHEST,


		
		
		
		IMAGE_SPOOLED,
		IMAGE_IMAP_PARTIAL_MAYBE_IDUNNO_SD,
		IMAGE_RECOVERED
	};
protected:
	QCImageList m_ImageListColor;
	QCImageList m_ImageListMonochrome;
};

class QCMoodImageList
{	
public:
	QCMoodImageList();
	bool Load(void);
	void Free(void);
	void Draw(int Index, int x, int y, CDC* pdc, COLORREF color);
	CImageList* GetMoodImageList() { return &m_ImageListColor;}
	enum ImageOffset
	{
		IMAGE_MOOD_UNKNOWN =0,
		IMAGE_MOOD_CLEAN, 
		IMAGE_MOOD_LOW ,
		IMAGE_MOOD_MEDIUM ,
		IMAGE_MOOD_HIGH
	};
protected:
	QCImageList m_ImageListColor;
	QCImageList m_ImageListMonochrome;
};

#define TocBitmapWidth 16
#define TocBitmapHeight 16

void LoadImageLists(void);

extern class QCCommon16ImageList g_theCommon16ImageList;
extern class QCCommon32ImageList g_theCommon32ImageList;
extern class QCMailboxImageList g_theMailTreeImageList;
extern class QCTocImageList g_theTocImageList;
extern class QCMoodImageList g_theMoodImageList;

#endif // _QCImageList_h_

