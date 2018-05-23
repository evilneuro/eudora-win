#ifndef _QCFILEIO_H_
#define _QCFILEIO_H_

#include "stdafx.h"
#include "QCWorkerSocket.h"
#include "fileutil.h"


// -- File base connection --
class CFileIO : public Network
{
	//DECLARE_DYNAMIC(CFileIO)

public:
	CFileIO(int Size = -1);
	virtual ~CFileIO();

	virtual int Open(const char* Machine, UINT ServiceID, UINT PortIniID, UINT DefaultPort, int Quiet = FALSE, int FailOver = FALSE);	// required override

	//
	// FORNOW, it seems to be bad style to override a pure virtual
	// like this with the same name as the pure virtual.  Probably
	// better to use a different name here.
	//
	int OpenFile(const char* FileName, int mode);	
	
	virtual int Close();												// required override

	virtual void Reset();												// required override
	virtual void Flush();												// required override

	virtual int PutDirect(const char* buf = NULL, int length = -1);		// required override
	virtual int Put(const char* bufptr = NULL, int length = -1);


protected:

	virtual int Read();													// required override
	virtual int Write();												// required override
	JJFile m_jjFile;

private:
	friend Network* CreateNetConnection(BOOL, BOOL);
	
};

#endif
