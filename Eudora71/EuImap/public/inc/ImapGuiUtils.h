// ImapGuiUtils.h - Some IMAP gui utilities.

#ifndef __IMAPGUIUTILS_H
#define __IMAPGUIUTILS_H


// Class for forcing immediate display of progress window and replacing the original
// INI setting:
//
class CImmediateProgress
{
public:
	CImmediateProgress ();
	~CImmediateProgress();

	// ref count:
	static short	m_sRefCount;
	static short	m_sProgressIdle;

	// Re-entrancy semaphore:
	static BOOL		m_bBusy;

// INstance attrs:
private:
	BOOL			m_bWasBusy;
};


// Exported functions:
BOOL ImapGuiFetchAttachment (LPCSTR Filename);


#endif // __IMAPGUIUTILS_H