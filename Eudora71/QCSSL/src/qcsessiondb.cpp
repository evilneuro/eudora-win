#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "qcssl.h"
#include "qcsessiondb.h"
#include "sslutils.h"
#include "DebugNewHelpers.h"


int QCSessionDB::InitializeDatabase()
{
	m_pFirstRecord = 0;
	return 0;
}

int QCSessionDB::DeleteDatabase()
{
	SessionDBRecord		*del;	
	m_Mutex.Lock();
	SessionDBRecord *currentRecord = m_pFirstRecord;
	while (currentRecord)
	{
		del = currentRecord;
		currentRecord = del->next;
		FreeRecord(del);
	}
	m_Mutex.Unlock();
	return 0;
}

int QCSessionDB::AddRecord(SessionDBRecord *rec)
{
	m_Mutex.Lock();
	rec->next = m_pFirstRecord;
	m_pFirstRecord = rec;
	m_Mutex.Unlock();
	return 0;
}


int QCSessionDB::DeleteRecord( SessionDBRecord *rec)
{
	SessionDBRecord		*cur;
	m_Mutex.Lock();
	if (rec == m_pFirstRecord )
	{
		m_pFirstRecord = rec->next;
		FreeRecord(rec);
	}
	else
	{	cur = m_pFirstRecord;
		while (cur)
		{	if (cur->next == rec)
			{	cur->next = rec->next;
				FreeRecord(rec);
				break;
			}
			cur = cur->next;
		}
	}
	m_Mutex.Unlock();
	return 0;
}

int QCSessionDB::FreeRecord(SessionDBRecord *del)
{	
	free(del->keyData);
	free(del->sessData);
	free(del);
	return 0;
}

int QCSessionDB::ExpireDatabase()
{	int					err;
	SessionDBRecord		*rec, *next;
	unsigned long		deadline;

	m_Mutex.Lock();

	deadline = time(0) - SESSION_LIFETIME;

	rec = m_pFirstRecord;
	while (rec)
	{	next = rec->next;
		if (rec->addTime < deadline)
		{	err = DeleteRecord(rec);
			if (err) return err;
		}
		rec = next;
	}
	
	m_Mutex.Unlock();

	return 0;
}

cic_Err QCSessionDB::AddSessionCallback(const cic_Buffer sessionKey, const cic_Buffer sessionData, void * const sessionRef)
{
	int					err = 0;
	SessionDBRecord		*newRec = 0;
	QCSSLReference *pSSLReference = (QCSSLReference*)sessionRef;
	QCSessionDB			*db = (QCSessionDB*)pSSLReference->m_pSessionDB;

	newRec = (SessionDBRecord*)CD_malloc(sizeof(SessionDBRecord), NULL);
	if(newRec)
	{
		newRec->keyData = newRec->sessData = 0;	

		newRec->keyData = (unsigned char*)CD_malloc(sessionKey.length, NULL);
		if (newRec->keyData == 0)
		{
			CD_free(newRec, NULL);
		    return CIC_ERR_MEMORY;
		}
		memcpy(newRec->keyData, sessionKey.data, sessionKey.length);
		newRec->keyLength = sessionKey.length;
	
		newRec->sessData = (unsigned char*)CD_malloc(sessionData.length, NULL);
		if (newRec->sessData == 0)
		{	
			CD_free(newRec->keyData, NULL);
		    CD_free(newRec, NULL);
		    return CIC_ERR_MEMORY;
		}
		memcpy(newRec->sessData, sessionData.data, sessionData.length);
		newRec->sessLength = sessionData.length;
	
		newRec->addTime = time(0);
	
		err = db->AddRecord(newRec);
		if (err)
		{
			CD_free(newRec->keyData, NULL);
			CD_free(newRec->sessData, NULL);
		    CD_free(newRec, NULL);
		}
	}
	return err;
}

cic_Err QCSessionDB::GetSessionCallback(const cic_Buffer sessionKey, cic_Buffer sessionData, void * const sessionRef)
{
	QCSSLReference *pSSLReference = (QCSSLReference*)sessionRef;
	QCSessionDB			*db = (QCSessionDB*)pSSLReference->m_pSessionDB;
	SessionDBRecord		*rec;
	unsigned long		deadline;

	deadline = time(0) - SESSION_LIFETIME;

	rec = db->m_pFirstRecord;
	while (rec)
	{	if (rec->keyLength == sessionKey.length &&
				(memcmp(rec->keyData, sessionKey.data, rec->keyLength) == 0))
		{	if (rec->addTime < deadline)
			{	db->ExpireDatabase();	
				break;	
			}
			else
			{	
				sessionData.length = rec->sessLength;
				sessionData.data = (unsigned char*) CD_malloc(sessionData.length, NULL);
				if (sessionData.data == 0)
					return CIC_ERR_MEMORY;
				memcpy(sessionData.data, rec->sessData, rec->sessLength);
				return CIC_ERR_NONE;
			}
		}
		rec = rec->next;
	}

	return CIC_ERR_NOT_FOUND;
}

cic_Err QCSessionDB::DeleteSessionCallback(const cic_Buffer sessionKey, void * const sessionRef)
{	
	QCSSLReference *pSSLReference = (QCSSLReference*)sessionRef;
	QCSessionDB			*db = (QCSessionDB*)pSSLReference->m_pSessionDB;
	SessionDBRecord		*rec;

	db->m_Mutex.Lock();
	rec = db->m_pFirstRecord;
	while (rec)
	{	
		if (rec->keyLength == sessionKey.length &&
				(memcmp(rec->keyData, sessionKey.data, rec->keyLength) == 0))
		{	
			db->DeleteRecord(rec);
			break;
		}
		rec = rec->next;
	}

	db->m_Mutex.Unlock();
	return CIC_ERR_NONE ;
}
