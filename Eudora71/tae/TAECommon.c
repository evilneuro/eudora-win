// TAECommon.c

#ifdef WIN32
#include <windows.h>
#include <sys/stat.h>
#endif


#include "TAECommon.h"


#ifdef DO_MEMORY_DEBUGGING
#include <stdlib.h>
#include <time.h>
long	gNumAllocs = 0;
int		gDoMemoryDebugging = TAE_FALSE;

void TAE_BeginMemoryDebugging()
{
	gDoMemoryDebugging = TAE_TRUE;
}

void TAE_EndMemoryDebugging()
{
	gDoMemoryDebugging = TAE_FALSE;
}
#endif


void TAE_ShowError(char *szMsg)
{
	TAE_ShowMessage(szMsg);
}

void TAE_ShowMessage(char *szMsg)
{
	#ifdef CMD_LINE_INTERFACE
	printf(szMsg);
	#elif defined(WIN_DEBUG_OUTPUT)
	TRACE(szMsg);
	#else
	szMsg = szMsg;
	#endif
}


tae_bool TAE_IsLetter(char c)
{
	#ifdef WIN32
	return (short)IsCharAlpha(c);
	#endif
	#if defined(macintosh)
	#error TAE will not do anything until someone does something useful with CharType() here.
	#endif
}


///////////////////////////////////////////////////////////////////////
// TAEVector - Structure and methods for maintaining a list of objects.
///////////////////////////////////////////////////////////////////////

void TAEVector_Blank(struct TAEVector *pvector)
{
	if (pvector)
	{
		pvector->plData = NULL;
		pvector->iSize = 0;
		pvector->iMaxSize = 0;
		pvector->iGrowIncrement = 0;
	}
}

int TAEVector_Init(struct TAEVector *pvector, int iInitSize, int iGrowInc)
{
	if (pvector)
	{
		pvector->plData = NULL;
		pvector->iSize = 0;
		pvector->iMaxSize = iInitSize;
		pvector->iGrowIncrement = iGrowInc;

		if (iInitSize > 0)
		{
			pvector->plData = (long*)TAE_malloc(sizeof(long) * iInitSize);
			if (!pvector->plData)
			{
				return TAE_OUTOFMEMORY;
			}
		}
	}
	return TAE_OK;
}

void TAEVector_FreeMemory(struct TAEVector *pvector)
{
	if (pvector && pvector->plData)
	{
		TAE_free(pvector->plData);
		pvector->plData = NULL;
	}
}

void TAEVector_AddElement(struct TAEVector *pvector, long lNewItem)
{
	if (pvector)
	{
		if (pvector->iSize == pvector->iMaxSize)
		{
			long		*plNewData = (long*)TAE_malloc(sizeof(long) * (pvector->iMaxSize + pvector->iGrowIncrement));
			if (plNewData)
			{
				int			i = 0;
				pvector->iMaxSize += pvector->iGrowIncrement;
				for (; i < pvector->iSize; ++i)
				{
					plNewData[i] = pvector->plData[i];
				}
				for (; i < pvector->iMaxSize; ++i)
				{
					plNewData[i] = 0L;
				}
				TAE_free(pvector->plData);
				pvector->plData = plNewData;
			}
		}
		if (pvector->plData)
		{
			pvector->plData[pvector->iSize++] = lNewItem;
		}
	}
}

long TAEVector_ElementAt(struct TAEVector *pvector, int iIndex)
{
	if (pvector && pvector->plData && (iIndex < pvector->iSize))
	{
		return pvector->plData[iIndex];
	}
	return 0;
}


///////////////////////////////////////////////////////
// TAEStream - Structure and methods for stream access.
///////////////////////////////////////////////////////

void TAEStream_Blank(struct TAEStream *ptaestream)
{
	if (ptaestream)
	{
		ptaestream->pBuf = NULL;
		ptaestream->iDataLen = 0;
		ptaestream->iPos = 0;
	}
}

long TAEStream_Open(struct TAEStream *ptaestream, char *szFileName)
{
	if (ptaestream && szFileName)
	{
		#ifdef WIN32
  		#ifndef _stat
		#define _stat stat
		#endif

		struct _stat	 buf;
		FILE			*pfile;

		if (_stat(szFileName, &buf) != 0)
		{
			TAE_ShowError("Not enough bytes to read.\n");
			return 0;
		}
		else
		{
			pfile = fopen(szFileName, "rb");
			if (pfile)
			{
				int			iRead = 0;
				ptaestream->iDataLen = buf.st_size;
				ptaestream->iPos = 0;
				ptaestream->pBuf = (char*)TAE_malloc(sizeof(char) * ptaestream->iDataLen);
				if (!ptaestream->pBuf)
				{
					return 0;
				}
				iRead = fread(ptaestream->pBuf, sizeof(char), ptaestream->iDataLen, pfile);
				fclose(pfile);
				if ((unsigned int)iRead != ptaestream->iDataLen)
				{
					TAE_ShowError("Not enough bytes to read.\n");
					return 0;
				}
				return ptaestream->iDataLen;
			}
		}
		#else
		#endif
	}
	return 0;
}

void TAEStream_Close(struct TAEStream *ptaestream)
{
	if (ptaestream && ptaestream->pBuf)
	{
		TAE_free(ptaestream->pBuf);
		ptaestream->pBuf = NULL;
	}
}

char TAEStream_ReadChar(struct TAEStream *ptaestream)
{
	unsigned char	c = '\0';
	if (ptaestream && ptaestream->pBuf)
	{
		if ((ptaestream->iPos + 1) <= ptaestream->iDataLen)
		{
			c = ptaestream->pBuf[(ptaestream->iPos)++];
		}
		else
		{
			TAE_ShowError("Read past end of stream.\n");
		}
	}
	return c;
}

short TAEStream_ReadShort(struct TAEStream *ptaestream)
{
	short			s = 0;
	if (ptaestream && ptaestream->pBuf)
	{
		if ((ptaestream->iPos + sizeof(short)) <= ptaestream->iDataLen)
		{
			int				i = 0;
			int				j = 0;
			for (i = 0; i < sizeof(short); ++i)
			{
				j <<= 8;
				j += ptaestream->pBuf[(ptaestream->iPos)++];
			}
			s = (short)j;
		}
		else
		{
			TAE_ShowError("Read past end of stream.\n");
		}
	}
	return s;
}

int TAEStream_ReadInt(struct TAEStream *ptaestream)
{
	int				i = 0;
	if (ptaestream && ptaestream->pBuf)
	{
		if ((ptaestream->iPos + sizeof(int)) <= (unsigned long)ptaestream->iDataLen)
		{
			int				j = 0;
			unsigned char	c = '\0';
			for (j = 0; j < sizeof(int); ++j)
			{
				i <<= 8;
				c = ptaestream->pBuf[(ptaestream->iPos)++];
				i += c;
			}
		}
		else
		{
			TAE_ShowError("Read past end of stream.\n");
		}
	}
	return i;
}

char TAEStream_ReadCharAt(struct TAEStream *ptaestream, unsigned int *iPos)
{
	unsigned char	c = '\0';
	if (ptaestream && ptaestream->pBuf)
	{
		if ((*iPos + 2) <= ptaestream->iDataLen)
		{
			c = ptaestream->pBuf[(*iPos)++];
			c = ptaestream->pBuf[(*iPos)++];
		}
		else
		{
			TAE_ShowError("Read past end of stream.\n");
		}
	}
	return c;
}

short TAEStream_ReadShortAt(struct TAEStream *ptaestream, unsigned int *iPos)
{
	short			s = 0;
	if (ptaestream && ptaestream->pBuf)
	{
		if ((*iPos + sizeof(short)) <= ptaestream->iDataLen)
		{
			int				i = 0;
			int				j = 0;
			for (i = 0; i < sizeof(short); ++i)
			{
				j <<= 8;
				j += ptaestream->pBuf[(*iPos)++];
			}
			s = (short)j;
		}
		else
		{
			TAE_ShowError("Read past end of stream.\n");
		}
	}
	return s;
}

int TAEStream_ReadIntAt(struct TAEStream *ptaestream, unsigned int *iPos)
{
	int				i = 0;
	if (ptaestream && ptaestream->pBuf)
	{
		if ((*iPos + sizeof(int)) <= ptaestream->iDataLen)
		{
			int				j = 0;
			unsigned char	c = '\0';
			for (j = 0; j < sizeof(int); ++j)
			{
				i <<= 8;
				c = ptaestream->pBuf[(*iPos)++];
				i += c;
			}
		}
		else
		{
			TAE_ShowError("Read past end of stream.\n");
		}
	}
	return i;
}

int TAEStream_SetBytesAt(struct TAEStream *ptaestream, unsigned int iInsertPos, 
						 unsigned int iBytes, unsigned int iNumBytes)
{
	if (ptaestream && ptaestream->pBuf)
	{
		if ((iInsertPos + iNumBytes) <= ptaestream->iDataLen)
		{
			unsigned int	i = 0;
			unsigned char	c = '\0';

			for (i = iNumBytes; i > 0; --i)
			{
				c = (unsigned char)iBytes;
				ptaestream->pBuf[iInsertPos + i - 1] = c;
				iBytes >>= 8; 
			}
		}
		else
		{
			TAE_ShowError("Read past end of stream.\n");
		}
	}
	return 0;
}
