// ImapSettings.h - Declaration of the class that stores network settings
// so they can be passed to the IMAP dll.
// Instantiate one of these for every CIMAP connection.
//

#ifndef __IMAPSETTINGS_H
#define __IMAPSETTINGS_H

class CImapSettings
{
public:
	CImapSettings();
	~CImapSettings();

	// call this to fill the object with settings.
	//
	void	GrabSettings(unsigned long AccountID);

// Generic interface:
//	
	short	GetSettingsShort	(UINT StringNum);
	long	GetSettingsLong		(UINT StringNum);
	LPCSTR	GetSettingsString	(UINT StringNum, char *Buffer = NULL, int len = -1);


private:
	// Longs:

	// Ints.

	// Shorts:
	short	m_bRemoveOnDelete;
	short	m_nDoingMinimalDownload;


	// Settings are for  this account:
	unsigned long m_AccountID;

};

	
#endif // __IMAPSETTINGS_H

