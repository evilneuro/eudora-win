#ifndef _LSDEFINES_H_
#define _LSDEFINES_H_
//////////////////////////////////////////////////////////////////////////////
// GLOBAL DEFINES

#define LS_GLOBAL_BASE				0
#define LS_ERROR_BASE				(LS_GLOBAL_BASE + 10000)

// FOR CAWave
#define	CAWAVE_ERR_BASE				LS_ERROR_BASE

#define	ERROR_MM_CANT_OPEN_DEVICE	CAWAVE_ERR_BASE
#define ERROR_MM_NODRIVER			(CAWAVE_ERR_BASE + 1)
#define ERROR_MM_INUSE				(CAWAVE_ERR_BASE + 2)
#define ERROR_MM_BADDEVICEID		(CAWAVE_ERR_BASE + 3)
#define ERROR_MM_BADFORMAT			(CAWAVE_ERR_BASE + 4)
#define	ERROR_MM_CANT_CLOSE_DEVICE	(CAWAVE_ERR_BASE + 5)
#define	ERROR_MM_ALREADY_OPENED		(CAWAVE_ERR_BASE + 6)
#define ERROR_MM_ALREADY_CLOSED		(CAWAVE_ERR_BASE + 7)
#define	ERROR_MM_CANT_START_RECORD	(CAWAVE_ERR_BASE + 8)
#define	ERROR_MM_BUF_ADD			(CAWAVE_ERR_BASE + 9)

#define	ERROR_FILE_CANT_OPEN		(CAWAVE_ERR_BASE + 10)

#define LSACTION_ERR_BASE			(LS_ERROR_BASE + 1000)
#define LS_ERROR_INVALID_NAME		(LSACTION_ERR_BASE + 1)
#define LS_ERROR_EMPTY_NAME			(LSACTION_ERR_BASE + 2)
#define LS_ERROR_NO_CODEC			(LSACTION_ERR_BASE + 3)
#define LS_ERROR_OLDER_FILE_FORMAT	(LSACTION_ERR_BASE + 4)
#define LS_ERROR_INVALID_FILE		(LSACTION_ERR_BASE + 5)
#define LS_ERROR_FILE_EXIST			(LSACTION_ERR_BASE + 6)
#define	LS_ERROR_INVALID_MAPI		(LSACTION_ERR_BASE + 7)
#define LS_ERROR_MAPI_SEND_FAIL		(LSACTION_ERR_BASE + 8)
#define	LS_ERROR_FILE_NEWER			(LSACTION_ERR_BASE + 9)
#define	LS_ERROR_CODEC_NOT_FOUND	(LSACTION_ERR_BASE + 10)

#define LS_STR_BASE					(LS_GLOBAL_BASE + 20000)
#define LS_STR_NO_CODEC				(LS_STR_BASE + 1)
#define LS_STR_DISCARD				(LS_STR_BASE + 2)
#define	LS_STR_ATTACH_QUIT			(LS_STR_BASE + 3)
#define LS_STR_FILE_QCP				(LS_STR_BASE + 4)
#define LS_STR_FILE_WAV_PCM			(LS_STR_BASE + 5)
#define LS_STR_FILE_ALL				(LS_STR_BASE + 6)

// Window message for sending read end of anonymous file to already existing
// instance of this program
#define WM_USER_FILENAME			(WM_USER + 6)

// I'll send this to Eudora (more specifically, PureVoice32.dll EMSAPI plugin)
// indicating that I've placed the filename in the shared memory that I've
// created.
#define	WM_USER_EUDORA_FN			(WM_USER + 7)

#endif
