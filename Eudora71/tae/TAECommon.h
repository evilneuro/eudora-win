// TAECommon.h

#ifndef _TAECOMMON_
#define _TAECOMMON_

#include <stdio.h>

#include <string.h>

#ifdef WIN32
#include <malloc.h>

// Do this to get file and line number info on heap allocations.
// The _MFC_OVERRIDES_NEW is a hack to not get new and delete operator
// overrides if you wind up #including this header in a C++ file.
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#ifdef _MFC_OVERRIDES_NEW
#include <crtdbg.h>
#else
#define _MFC_OVERRIDES_NEW
#include <crtdbg.h>
#undef _MFC_OVERRIDES_NEW
#endif
#endif

#define TAE_malloc(iSize) malloc(iSize)
#define TAE_free(memblock) free(memblock)
#else
#define TAE_malloc(iSize)
#define TAE_free(memblock)
#endif

#ifdef _DEBUG
#define SHOW_DEBUG_OUTPUT
//#define DO_MEMORY_DEBUGGING
//#define DO_RANDOM_MEMORY_FAILURES
#endif


// Include publicly available definitions.
#include "TAEPublic.h"




#ifdef DO_MEMORY_DEBUGGING
void		TAE_BeginMemoryDebugging();
void		TAE_EndMemoryDebugging();
#endif

void		TAE_ShowError(char *szMsg);
void		TAE_ShowMessage(char *szMsg);

tae_bool	TAE_IsLetter(char c);


///////////////////////////////////////////////////////////////////////
// TAEVector - Structure and methods for maintaining a list of objects.
///////////////////////////////////////////////////////////////////////

struct TAEVector
{
	long				*plData;
	int					 iSize;
	int					 iMaxSize;
	int					 iGrowIncrement;
};

void					 TAEVector_Blank(struct TAEVector *pvector);
int						 TAEVector_Init(struct TAEVector *pvector, int iInitSize, int iGrowInc);
void					 TAEVector_FreeMemory(struct TAEVector *pvector);

void					 TAEVector_AddElement(struct TAEVector *pvector, long lNewItem);
long					 TAEVector_ElementAt(struct TAEVector *pvector, int iIndex);


///////////////////////////////////////////////////////
// TAEStream - Structure and methods for stream access.
///////////////////////////////////////////////////////

struct TAEStream
{
	char				*pBuf;
	unsigned int		 iDataLen;
	unsigned int		 iPos;
};

void					 TAEStream_Blank(struct TAEStream *ptaestream);
long					 TAEStream_Open(struct TAEStream *ptaestream, char *szFileName);
void					 TAEStream_Close(struct TAEStream *ptaestream);

char					 TAEStream_ReadChar(struct TAEStream *ptaestream);
short					 TAEStream_ReadShort(struct TAEStream *ptaestream);
int						 TAEStream_ReadInt(struct TAEStream *ptaestream);
char					 TAEStream_ReadCharAt(struct TAEStream *ptaestream, unsigned int *iPos);
short					 TAEStream_ReadShortAt(struct TAEStream *ptaestream, unsigned int *iPos);
int						 TAEStream_ReadIntAt(struct TAEStream *ptaestream, unsigned int *iPos);

int						 TAEStream_SetBytesAt(struct TAEStream *ptaestream, unsigned int iInsertPos, 
											  unsigned int iBytes, unsigned int iNumBytes);

#endif	// #ifndef _TAECOMMON_
