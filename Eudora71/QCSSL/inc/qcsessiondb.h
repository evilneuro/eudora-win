

#ifndef _SSESSDB_H_
#define _SSESSDB_H_

#include "QCSSL.h"
#include "qcthread.h"

#define SESSION_LIFETIME (3600UL * 24UL)// 1 day (24 hrs)

typedef struct SessionDBRecord
{	struct SessionDBRecord	*next;
	unsigned char			*keyData;
	unsigned long			keyLength;
	unsigned char			*sessData;
	unsigned long			sessLength;
	unsigned long			addTime;
} SessionDBRecord;



class QCSessionDB
{
private:
	QCMutex m_Mutex;
	SessionDBRecord	*m_pFirstRecord;
public:
	int InitializeDatabase();
	int DeleteDatabase();
	int ExpireDatabase();
	int AddRecord(SessionDBRecord *rec);
	int DeleteRecord(SessionDBRecord *rec);
	int FreeRecord(SessionDBRecord *rec);
	static cic_Err AddSessionCallback(const cic_Buffer sessionKey, const cic_Buffer sessionData, void * const sessionRef);
    static cic_Err GetSessionCallback(const cic_Buffer sessionKey, cic_Buffer sessionData, void * const sessionRef);
	static cic_Err DeleteSessionCallback(const cic_Buffer sessionKey, void * const sessionRef);
};





#endif
