#ifndef _CSOCKCALLBACKS_H_
#define _CSOCKCALLBACKS_H_


class AFX_EXT_CLASS CSockCallbacks {
public:
	BOOL m_InProgress;
	const char * (* m_GetIniString)(UINT StringNum, char* Buffer, int size, BOOL getname);
	
	short (* m_GetIniShort)(UINT StringNum);
	long (* m_GetIniLong)(UINT StringNum);

	BOOL (* m_SetIniString)(UINT StringNum, const char *String);
	BOOL (* m_SetIniShort)(UINT StringNum, short Num);
	BOOL (* m_SetIniLong)(UINT StringNum, long Num);

	
	void (* m_CloseProgress)();
	void (* m_MainProgress)(const char* Message);
	
	//guiutils.h
	void (* m_ErrorDialog)(UINT StringID, ...);
	int (* m_AlertDialog)(UINT DialogID, ...);
	void (* m_HesiodErrorDialog)(int nHesiodError);
	int (* m_EscapePressed)(int Repost);

	//password.h
	const char* (* m_EncodePassword)(const char* ClearText);

	void (* m_Progress)(long SoFar, const char* Message, long Total);
	//void Progress(const char* Message);

};

extern void InitSockCallbacks();

#endif
