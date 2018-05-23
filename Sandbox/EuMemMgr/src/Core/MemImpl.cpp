///////////////////////////////////////////////////////////////////////////////
// MemImpl.cpp
//
// Implementation: Heap debugging services with mark and sweep capability.
//
// These services support both C and C++. Why? you ask. Well, not everyone
// is programming in C++. Ergo, if we ever get a third party library
// written in C, these heap debugging services can be used as is, without
// any modification. Since C is supported, we have to use C'isms.
//
// Created: 01/12/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#define __MEM_IMPL_CPP__

#include "MemImpl.h"		// Implementation include.

#ifdef __DEBUG__
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char *
pMemMemoryReserve = 0;

// Added 01/14/98
#ifdef __DEBUG__
AssertHandler
ResetAssertHandler(void)
{
    return(SetMAssertHandler(MemAssertFail));
}
#endif // __DEBUG__

// Added 01/14/98
#ifdef __DEBUG__
DbgOutputHandler
ResetDbgOutputHandler(void)
{
    return(SetMDbgOutputHandler(MemOutputMessage));
}
#endif // __DEBUG__

// Added 01/16/98
#ifdef __DEBUG__
MemLeakHandler
ResetMemLeakHandler(void)
{
    return(SetMMemLeakHandler(MemLeakOutputMessage));
}
#endif // __DEBUG__

#ifdef __DEBUG__
static flag
DumpMemMgrLeakAtShutdown(void *, c_char * const buf, pDBF_OutType pDBF_OT,
			 void *pBlock)
{
    // Dump the buffer.
    MMemLeak(buf);

    // Then print the first few bytes if this is a leak block...
    if (pDBF_OT == PDBF_ITEM) {
	int   i = 0;
	char _the_buf[32] = { '\0' };
	for (i = 0; i < 3; i++) {
	    _the_buf[i] = '>';
	}

	MAssertSzInternal(pBlock != NULL, DMM_IE[DMM_IE_PBLOCK_NULL]);
	MemMgrAllocInfo *pAllocInfo = (MemMgrAllocInfo *)(pBlock);

	if (pAllocInfo->SizeBytes >= 25) {
	    memcpy(_the_buf + 3, (void *)pAllocInfo->Address, 25);
	}
	else { // pAllocInfo->SizeBytes < 25.
	    memcpy(_the_buf + 3, (void *)pAllocInfo->Address,
		   pAllocInfo->SizeBytes);
	}

	for (i = 3; i < 28; i++) {
	    if (_the_buf[i] == '\0') {
		_the_buf[i] = 46; // the period character
	    }
	}

	for (i = 28; i < 31; i++) {
	    _the_buf[i] = '<';
	}

        MMemLeak(_the_buf);
	MMemLeak("\n");
    }

    return(TRUE);
}
#endif // __DEBUG__

flag
InitializeDebugMemoryManager(void)
{
    // The following two functions are no-ops in Release builds.
    SetMAssertHandler(MemAssertFail);
    SetMDbgOutputHandler(MemOutputMessage);
    SetMMemLeakHandler(MemLeakOutputMessage);

    flag retVal = TRUE;

    MAssertSzClient(pMemMemoryReserve == 0, DMM_CE[DMM_CE_INIT_TWICE]);

    if (pMemMemoryReserve == 0) {
	if (fNewBlock((void **)(&pMemMemoryReserve), N_RESERVE_MEMORY)) {
	    fBaptizeBlock((void *)(pMemMemoryReserve), "MemoryReserve");
	    fNoteBlockOrigin((void *)(pMemMemoryReserve),
			     THIS_FILE , __LINE__ - 3);
#ifdef __DEBUG__
	    char buf[256] = { '\0' };
	    sprintf(buf, _strAcquiredReserveMem, N_RESERVE_MEMORY);
	    MDbgOutput(buf);
#endif // __DEBUG__
	}
	else { // could not get reserve memory.
	    retVal = FALSE;
	}
    }

    return(retVal);
}

void
ShutdownDebugMemoryManager(void)
{
    
    if (pMemMemoryReserve != NULL) {
	FreeBlock((void *) pMemMemoryReserve);
	pMemMemoryReserve = NULL;
#ifdef __DEBUG__
	char buf[256] = { '\0' };
	sprintf(buf, _strFreedReserveMem, N_RESERVE_MEMORY);
	MDbgOutput(buf);

	uint nLeakBytes = 0;
	if ((nLeakBytes = nAllocBytes()) != 0) {
	    char strLeakBuf[256];
	    sprintf(strLeakBuf, _strDetectMemLeak, nLeakBytes);
	    MDbgOutput(strLeakBuf);
	}

	DumpBlocksLogByLabelAndFile(NULL, DumpMemMgrLeakAtShutdown,
				    0, 0, 0); 
#endif // __DEBUG__
    }
}

// Record that heap is insolvent(bankrupt).
void
SetHeapInsolvent(void)
{
    fHeapInsolvent = TRUE;
}

// Record that heap is solvent.
void
SetHeapSolvent(void)
{
    fHeapInsolvent = FALSE;
}

// The next two functions return the status of the heap. The semantic
// content of the information they return is identical; they differ
// only in the questions they are answers to.

// Return heap solvency status
flag
fIsHeapSolvent(void)
{
    return((flag)(fHeapInsolvent == FALSE));
}

// Return heap solvency status
flag
fIsHeapInsolvent(void)
{
    return((flag)(fHeapInsolvent == TRUE));
}

// These functions increment, decrement, and get allocated bytes, and
// live only in the debug version.
#ifdef __DEBUG__
void
GetAllocBytes(int *pnBytesAlloc, int *pnMaxBytesAlloc)
{
    *pnBytesAlloc = nBytesAllocated;
    *pnMaxBytesAlloc = nMaxBytesAllocated;
}
#endif // __DEBUG__

#ifdef __DEBUG__
uint
nAllocBytes(void)
{
    return(nBytesAllocated);
}
#endif // __DEBUG__

#ifdef __DEBUG__
uint
nMaxAllocBytes(void)
{
    return(nMaxBytesAllocated);
}
#endif // __DEBUG__

#ifdef __DEBUG__
static void
IncrementAllocCounter(size_t sizeBytes)
{
    nBytesAllocated += sizeBytes;
    if (nBytesAllocated > nMaxBytesAllocated) {
	nMaxBytesAllocated = nBytesAllocated;
    }
}
#endif // __DEBUG__

#ifdef __DEBUG__
static void
DecrementAllocCounter(size_t sizeBytes)
{
    nBytesAllocated -= sizeBytes;
}
#endif // __DEBUG__

// This function is THE interface to malloc. It is private.
static void *
pvAllocBlock(size_t size)
{
    MAssertSzInternal(size != 0, DMM_IE[DMM_IE_REQUESTING_ZERO_BYTES]);
    void *pv = (void *) malloc(size);
    return(pv);
}

// This function is THE interface to free. It is private.
static void PrivateFreeBlock(void *pv)
{
    MAssertSzInternal(pv != 0, DMM_IE[DMM_IE_DEALLOCATING_NULL]);

    if (pv) {
	free(pv);
    }
}


#ifdef __DEBUG__
flag
_fNewBlock(void **ppv, size_t size, c_char *szFile, uint nLine)
#else
flag
fNewBlock(void **ppv, size_t size)
#endif // __DEBUG__
{
    byte **ppb = (byte **)ppv;

    // Assert that the address intended to hold the block
    // is not NULL, and the size requested is not zero.
    MAssertSzClient(ppv != 0, DMM_CE[DMM_CE_POINTER_ADDRESS_NULL]);
    MAssertSzClient(size != 0, DMM_CE[DMM_CE_REQUESTING_ZERO_BYTES]);

#ifdef __DEBUG__
    if (fTestOnAllocOrFree) {
	IdleTestBlocks();
    }
    // Allocate extra space for sentinel bytes.
    *ppb = (byte *) pvAllocBlock(size + 2 * nSentinel);
#else
    *ppb = (byte *) pvAllocBlock(size);
#endif

#ifdef __DEBUG__
	{
	    if (*ppb != NULL) {
		uint nIdx = 0;
  
		// Fill in the sentinel bytes.
		for (nIdx = 0; nIdx < nSentinel; nIdx++) {
		    (*ppb)[nIdx]                    = bSentinelGarbage;
		    (*ppb)[size + nSentinel + nIdx] = bSentinelGarbage;
		}
		*ppb += nSentinel;
	    
		memset(*ppb, bNewGarbage, size);
  
		// If unable to create block information,
		// fake a total memory failure.

		if (!fCreateBlockInfo(*ppb, size)) {
		    PrivateFreeBlock(*ppb - nSentinel);
		    *ppb = NULL;
		}
	    }
	}
#endif

    if (*ppb == NULL) {
	SetHeapInsolvent();
    }
    else {
#ifdef __DEBUG__
	if (szFile != NULL) {
	    fNoteBlockOrigin((void *)*ppb, szFile, nLine);
	}
#endif // __DEBUG__
    }
    
    return((flag)(*ppb != NULL));
}


void
FreeBlock(void *pv)
{
#ifdef __DEBUG__
    blockinfo *pbi = 0;
#endif // __DEBUG__

    // Although NULL is a legal argument for free(), we assert
    // against it. The philosophy is no NULL pointers to functions
    // where the NULL has no meaning but one conjured up for
    // convenience.

    MAssertSzClient(pv != 0, DMM_CE[DMM_CE_DEALLOCATING_NULL]);

#ifdef __DEBUG__
    pbi = pbiGetBlockInfo((byte *) pv);
    MAssertSzClient(pbi != 0, DMM_CE[DMM_CE_DEALLOCATING_INVALID_BLOCK]);
#endif // __DEBUG__

#ifdef __DEBUG__
    {
	if (fTestOnAllocOrFree) {
	    IdleTestBlocks();
	}
	
	memset(pv, bFreeGarbage, sizeOfBlock((byte *)pv));
	FreeBlockInfo((byte *) pv);

	// Adjust for the sentinel bytes.
	pv = (void *)((byte *)pv - nSentinel);
    }
#endif // __DEBUG__

    PrivateFreeBlock(pv);
}

#ifdef __DEBUG__
void
SaveUserData(void *pv, void *pvUserData)
{
    blockinfo *pbi = 0;

    MAssertSzClient(pv != 0, DMM_CE[DMM_CE_SAVING_DATA_ON_NULL_BLOCK]);

    // After all, why would you want to save NULL?
    if (pvUserData == NULL) {
	MDbgOutput("Did you really wish to save a NULL user data?");
    }

    pbi = pbiGetBlockInfo((byte *) pv);
    MAssertSzClient(pbi != 0, DMM_CE[DMM_CE_SAVING_DATA_ON_INVALID_BLOCK]);

    pbi->pvUserData = pvUserData;
}
#endif // __DEBUG__

#ifdef __DEBUG__
void *
pvRetrieveUserData(void *pv)
{
    blockinfo *pbi = 0;

    MAssertSzClient(pv != 0, DMM_CE[DMM_CE_RETRIEVING_DATA_ON_NULL_BLOCK]);

    pbi = pbiGetBlockInfo((byte *) pv);
    MAssertSzClient(pbi != 0, DMM_CE[DMM_CE_RETRIEVING_DATA_ON_INVALID_BLOCK]);

    if (pbi->pvUserData == NULL) {
	MDbgOutput("Saved user data is NULL");
    }
    return(pbi->pvUserData);
}
#endif // __DEBUG__

#ifdef __DEBUG__
c_uint
nRetrieveLineInfo(void *pv)
{
    blockinfo *pbi = 0;

    MAssertSzClient(pv != 0,
		    DMM_CE[DMM_CE_RETRIEVING_LINENUMBER_ON_NULL_BLOCK]);

    pbi = pbiGetBlockInfo((byte *) pv);
    MAssertSzClient(pbi != 0,
		    DMM_CE[DMM_CE_RETRIEVING_LINENUMBER_ON_INVALID_BLOCK]);

    return(pbi->nLine);
}
#endif // __DEBUG__

#ifdef __DEBUG__
char *
strRetrieveFileInfo(void *pv)
//c_char * const strRetrieveFileInfo(void *pv)
{
    blockinfo *pbi = 0;
    
    MAssertSzClient(pv != 0, 
	      DMM_CE[DMM_CE_RETRIEVING_FILENAME_ON_NULL_BLOCK]);

    pbi = pbiGetBlockInfo((byte *) pv);
    MAssertSzClient(pbi != 0,
	      DMM_CE[DMM_CE_RETRIEVING_FILENAME_ON_INVALID_BLOCK]);

    return(pbi->szFileName);
//    return((c_char * const) pbi->szFileName);
}
#endif // __DEBUG__

#ifdef __DEBUG__
// DoBaptizeBlock()
// Expect the start of a block. Otherwise we fire asserts.
flag
fDoBaptizeBlock(void *pv, c_char * const szLabel)
{
    blockinfo    *pbi     = NULL;
    char         *strTest = NULL;
    uint          nStrLen = 0;
    flag          fRetVal = TRUE;

    MAssertSzClient(pv != 0, DMM_CE[DMM_CE_BAPTIZING_NULL_BLOCK]);
    MAssertSzClient(szLabel != 0,
		    DMM_CE[DMM_CE_BAPTISMAL_LABEL_LENGTH_INVALID]);

    if (szLabel != NULL) {
	nStrLen = strlen(szLabel);
    }

    MAssertSzClient(nStrLen != 0,
		    DMM_CE[DMM_CE_BAPTISMAL_LABEL_LENGTH_INVALID]);

    if ((pbi = pbiGetBlockInfo((byte *)pv)) != NULL) {

	// No block info. Is it (a) garbage, (b) pointing to a block that
	// was freed? or (c) pointing to a block that moved when it was
	// resized with fResizeBlock()?
	// MAssert(pbi != NULL);	

	// A member variable?
	MAssertSzClient(pbi->pb == (byte *)pv,
		  DMM_CE[DMM_CE_BAPTIZING_MIDDLE_OF_BLOCK]);

	// Before we go on destroying the old label block, we see if
	// we can fit it in the existing blocksize.
	if (nStrLen <= (pbi->nLabelBlockSize - 1)) {
	    strcpy(pbi->szLabel, szLabel);
	    pbi->szLabel[nStrLen] = '\0';
	}
	else {
	    // Before we go on destroying the old block, get memory
	    // memory for the new block first.
	    strTest = (char *) pvAllocBlock(nStrLen + 1);
	    if (strTest != NULL ) {
		// Destroy the old contents of pbi->szLabel before freeing it
		// and assigning it the new string.
		memset(pbi->szLabel, bFreeGarbage, pbi->nLabelBlockSize);
		PrivateFreeBlock(pbi->szLabel);
		// Initialize the new block.
		memset(strTest, bNewGarbage, nStrLen + 1);
		pbi->szLabel          = strTest;
		strcpy(pbi->szLabel, szLabel);
		pbi->szLabel[nStrLen] = '\0';
		pbi->nLabelBlockSize  = nStrLen + 1;
	    }
	    else { // failure, just leave the old label.
		fRetVal = FALSE;
	    }
	}
    }
    else {
	// A stack object.
	fRetVal = FALSE;
    }
    return(fRetVal);
}
#endif // __DEBUG__
    
#ifdef __DEBUG__
// DoNoteBlockOrigin()
// Expect the start of a block. Otherwise we fire asserts.
flag fDoNoteBlockOrigin(void *pv, c_char * const szFile, uint nLine)
{
    blockinfo    *pbi     = NULL;
    char         *strTest = NULL;
    uint          nStrLen = 0;
    flag          fRetVal = TRUE;

    MAssertSzClient(pv != 0, DMM_CE[DMM_CE_NOTING_ORIGIN_OF_NULL_BLOCK]);
    MAssertSzClient(szFile != 0, DMM_CE[DMM_CE_ORIGIN_NULL]);

    if (szFile) {
	nStrLen = strlen(szFile);
    }

    MAssertSzClient(nStrLen > 0, DMM_CE[DMM_CE_ORIGIN_LENGTH_NULL]);
 
    if ((pbi = pbiGetBlockInfo((byte *)pv)) != NULL) {

	// No block info. Is it (a) garbage, (b) pointing to a block that
	// was freed? or (c) pointing to a block that moved when it was
	// resized with fResizeBlock()?
	// MAssert(pbi != NULL);	

	// A member variable?
	MAssertSzClient(pbi->pb == (byte *)pv,
			DMM_CE[DMM_CE_NOTING_ORIGIN_OF_MIDDLE_OF_BLOCK]);

	// Before we go on destroying the old FileName block, we see if
	// we can fit it in the existing blocksize.
	if (nStrLen <= (pbi->nFileNameBlockSize - 1)) {
	    strcpy(pbi->szFileName, szFile);
	    pbi->szFileName[nStrLen] = '\0';
	    pbi->nLine               = nLine;
	}
	else {
	    // Before we go on destroying the old block, get memory
	    // memory for the new block first.
	    strTest = (char *) pvAllocBlock(nStrLen + 1);
	    if (strTest != NULL ) {
		// Destroy the old contents of pbi->szLabel before freeing it
		// and assigning it the new string.
		memset(pbi->szFileName, bFreeGarbage, pbi->nFileNameBlockSize);
		PrivateFreeBlock(pbi->szFileName);
		// Initialize the new block.
		memset(strTest, bNewGarbage, nStrLen + 1);
		pbi->szFileName          = strTest;
		strcpy(pbi->szFileName, szFile);
		pbi->szFileName[nStrLen] = '\0';
		pbi->nFileNameBlockSize  = nStrLen + 1;
		pbi->nLine               = nLine;
	    }
	    else { // failure, just leave the old filename
		fRetVal = FALSE;
	    }
	}
    }
    else {
	// A stack object.
	fRetVal = FALSE;
    }
    return(fRetVal);
}
#endif // __DEBUG__

#ifdef __DEBUG__
// pbiHead points to a singly-linked list of debugging information for
// the memory manager.
static blockinfo *pbiHead = NULL;
#endif

#ifdef __DEBUG__
// IdleTestBlocks()
//
// Called once per idle loop to test the integrity of memory.

void IdleTestBlocks(void)
{
    blockinfo *pbi = 0;
 
    for (pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext) {
	TestSentinel(pbi);
	// Add further tests here.
    }
}
#endif // __DEBUG__

// FillBufferWithCharN()
//
// Support function. Fill buffer buf with n characters c. Caller
// must ensure that n < space in buf.
//
#ifdef __DEBUG__
static void FillBufferWithCharN(char *buf, char c, c_uint n)
{
    uint i = 0;

    for (i = 0; i < n; i++) {
	buf[i] = c;
    }

    buf[i] = '\0';
}
#endif

#ifdef __DEBUG__
static char *PathLessFileName(char *fileName)
{
    MAssertSzInternal(fileName != 0,
		      DMM_IE[DMM_IE_FILENAME_TO_STRIP_PATH_FROM_NULL]);

#if defined(WIN32) || defined(__unix__)
    {
	char *strE = 0;

	strE = (char *) strrchr(fileName, '\\');
	if (strE == NULL)
	    strE = (char *) strrchr(fileName, '/');
	if (strE != NULL)
	    strE++;

	if (strE != NULL && strE[0] != '\0')
	    return(strE);
	else
	    return(fileName);
    }
#else
    return(fileName);
#endif

}
#endif // __DEBUG__

#ifdef __DEBUG__
// DumpItems(blockinfo *, PDBF, c_uint)
// 
// Dump log items in reverse order.

static flag
DumpItems(void *data, blockinfo *pbi, PDBF pDBF, c_uint nRefOnly)
{
    flag fLast = TRUE;
    
    MAssertSzInternal(pbi != 0, DMM_IE[DMM_IE_DUMPITEMS_BLOCKINFO_NULL]);

    if (pbi->pbiNext != NULL) {
	fLast = DumpItems(data, pbi->pbiNext, pDBF, nRefOnly);
    }

    if (fLast == FALSE)
	return(fLast);

    if (nRefOnly && !pbi->fReferenced) {
	 fLast = TRUE;
    }
    else {
	char *strE = 0;
#if defined(WIN32)
//	char *strE;

	strE = (char *) strrchr(pbi->szFileName, '\\');
	if (strE == NULL)
	    strE = (char *) strrchr(pbi->szFileName, '/');
	if (strE != NULL)
	    strE++;
#endif			
	sprintf(MemBuffer, "%08X %15d  %-48s %-48s (%d)\n",
		(uint)pbi->pb, pbi->size, pbi->szLabel,
#if defined(WIN32)
		(strE != NULL && strE[0] != '\0')
		? strE : pbi->szFileName,
#else
		pbi->szFileName,
#endif
		pbi->nLine);

	nDumpBytes += pbi->size;

	memMgrAllocInfo.Address   = (uint) pbi->pb;
	memMgrAllocInfo.SizeBytes = pbi->size;
	memMgrAllocInfo.szLabel   = pbi->szLabel;
	memMgrAllocInfo.szFile    = (strE != NULL && strE[0] != '\0')
	                            ? strE : pbi->szFileName;
	memMgrAllocInfo.Line      = pbi->nLine;

	fLast = (*pDBF)(data, (c_char *) MemBuffer, PDBF_ITEM,
			(void *) &memMgrAllocInfo);
    }
    
    return(fLast);
}
#endif // __DEBUG__

#ifdef __DEBUG__
// DumpBlocksLog()
//
// Call client-supplied callback function to print out the contents of
// all blocks still allocated. If client returns false at any point,
// stop pumping output.
// 
// If refOnly == TRUE, only print out blocks for which fReferenced == TRUE.

void DumpBlocksLog(void *data, PDBF pDBF, c_uint nRefOnly)
{
    blockinfo *pbi = 0;
    flag       fLast      = TRUE;
    
    // If client calls this fucnction with a null pointer to dump
    // blocks callback function, we fire an assert. The philosophy
    // is that if client calls us to dump blocks, he *must* have
    // a valid callback function. Note that "data" can be null since,
    // it usually means that either we are being called from a
    // C module, or that we are not being called from a C++ class.
    // Clients will typically static_cast<void *>(this) and send
    // it as data. We just return "data" as the first argument of
    // of the callback function we call.

    MAssertSzClient(pDBF != 0, DMM_CE[DMM_CE_CLIENT_CALLBACK_FUNCTION_NULL]);

    MemBuffer[0] = '\n';
    FillBufferWithCharN(MemBuffer + 1, '=', 130);
    strcat(MemBuffer, "\n");
    if ((*pDBF)(data, MemBuffer, PDBF_SEP, NULL) == TRUE) {
	sprintf(MemBuffer, "%-8s %15s  %-48s %-48s %-7s\n", "Address",
		"Size(bytes)", "Label", "FileName", "Line");	    
	if ((*pDBF)(data, MemBuffer, PDBF_HEADING, NULL) == TRUE) {
	    FillBufferWithCharN(MemBuffer, '=', 130); 
	    strcat(MemBuffer, "\n");
	    if ((*pDBF)(data, MemBuffer, PDBF_SEP, NULL) == TRUE) {
		if (pbiHead == NULL) {
		    sprintf(MemBuffer, "\tBlock-list empty\n");  
		    if ((*pDBF)(data, MemBuffer, PDBF_BLKSEMPTY,
				NULL) == TRUE) {
			FillBufferWithCharN(MemBuffer, '=', 130); 
			strcat(MemBuffer, "\n");
			(*pDBF)(data, MemBuffer, PDBF_SEP, NULL);
		    }
		}
		else {
		    pbi        = pbiHead;
		    nDumpBytes = 0; // Start of run.
		    fLast      = DumpItems(data, pbi, pDBF, nRefOnly);
		    if (fLast == TRUE) {
			FillBufferWithCharN(MemBuffer, '=', 130);
			strcat(MemBuffer, "\n");
			if ((*pDBF)(data, MemBuffer, PDBF_SEP, NULL) == TRUE) {
			    sprintf(MemBuffer, "%-8s %15d\n", "Total",
				    nDumpBytes);
			    if ((*pDBF)(data, MemBuffer, PDBF_TOTAL,
					NULL) == TRUE) {
				FillBufferWithCharN(MemBuffer, '=', 130);
				strcat(MemBuffer, "\n\n");
				(*pDBF)(data, MemBuffer, PDBF_SEP, NULL);
			    }
			}
		    }
		}
	    }
	}
    }

    IdleTestBlocks();		// What the heck...		
}
#endif // __DEBUG__

#ifdef __DEBUG__
static int DMMWordNormalizeCaseStrnCmp(char *x, char *y, uint n, uint m)
{
    uint i = 0, nGoUpTo = 0;

    MAssertSzInternal(x != 0,
		      DMM_IE[DMM_IE_NORMALIZE_CASE_STRCMP_FILTER_NULL]);
    MAssertSzInternal(y != 0, DMM_IE[DMM_IE_NORMALIZE_CASE_STRCMP_STR_NULL]);
    MAssertSzInternal(n > 0,
		      DMM_IE[DMM_IE_NORMALIZE_CASE_STRCMP_FILTER_LEN_INVALID]);
    MAssertSzInternal(m > 0,
		      DMM_IE[DMM_IE_NORMALIZE_CASE_STRCMP_STR_LEN_INVALID]);


    nGoUpTo = (n < m) ? n : m;

    for (i = 0; i < nGoUpTo; i++) {
	if ((x[i] != y[i]) &&
	    (x[i] != (y[i] + 'a' - 'A')) &&
	    (y[i] != (x[i] + 'a' - 'A')))
	    break;
	else
	    continue;
    }

    if (i != nGoUpTo) {
	if (x[i] < y[i])
	    return(-1);
	else
	    return(1);
    }
    else {
	return(0);
    }
}
#endif

#ifdef __DEBUG__
// DumpItemsByLabelAndFile(blockinfo *)
// 
// Dump log items in reverse order by label and file.

static flag DumpItemsByLabelAndFile(blockinfo *pbi)
{
    flag fLast         = TRUE;
    flag fShouldDump   = FALSE;
    char *pathLessName = 0;

    MAssertSzInternal(pbi != 0,
		      DMM_IE[DMM_IE_DUMPITEMSBYLABELANDFILE_BLOCKINFO_NULL]);

    if (pbi->pbiNext != NULL) {
	fLast = DumpItemsByLabelAndFile(pbi->pbiNext);
    }

    if (fLast == FALSE) {
	return(fLast);
    }

    pathLessName = PathLessFileName(pbi->szFileName);

    if (szDMMLabelUserData && szDMMFileNameUserData) {
	if ((DMMWordNormalizeCaseStrnCmp(szDMMLabelUserData,
					pbi->szLabel,
					nDMMLabelLenUserData,
					pbi->nLabelBlockSize) == 0) &&
	    (DMMWordNormalizeCaseStrnCmp(szDMMFileNameUserData,
					 pathLessName,
					 nDMMFileNameLenUserData,
					 strlen(pathLessName)) == 0)) {
	    fShouldDump = TRUE;
	}
    }
    else if (szDMMLabelUserData) {
	if (DMMWordNormalizeCaseStrnCmp(szDMMLabelUserData,
					pbi->szLabel,
					nDMMLabelLenUserData,
					pbi->nLabelBlockSize) == 0) {
	    fShouldDump = TRUE;
	}
    }
    else if (szDMMFileNameUserData) {
	if (DMMWordNormalizeCaseStrnCmp(szDMMFileNameUserData,
					pathLessName, nDMMFileNameLenUserData,
					strlen(pathLessName)) == 0) {
	    fShouldDump = TRUE;
	}
    }
    else { // !szDMMLabelUserData && !szDMMFileNameUserData.
	fShouldDump = TRUE;
    }

    if ((nDMMRefOnlyUserData && !pbi->fReferenced) || (fShouldDump == FALSE)) {
	 fLast = TRUE;
    }
    else {
	sprintf(MemBuffer, "%08X %15d  %-48s %-48s (%d)\n",
		(uint)pbi->pb, pbi->size, pbi->szLabel, pathLessName,
		pbi->nLine);

	nDumpBytes += pbi->size;

	memMgrAllocInfo.Address   = (uint) pbi->pb;
	memMgrAllocInfo.SizeBytes = pbi->size;
	memMgrAllocInfo.szLabel   = pbi->szLabel;
	memMgrAllocInfo.szFile    = pathLessName;
	memMgrAllocInfo.Line      = pbi->nLine;

	fLast = (*pDBFDMMUserData)(pvDMMUserData, (c_char *) MemBuffer,
				   PDBF_ITEM, (void *) &memMgrAllocInfo);
    }
    
    return(fLast);
}
#endif // __DEBUG__

#ifdef __DEBUG__
// DumpBlocksLogByLabelAndFile()
//
// Call client-supplied callback function to print out the contents of
// all blocks still allocated. If client returns false at any point,
// stop pumping output.
// 
// If refOnly == TRUE, only print out blocks for which fReferenced == TRUE.

void DumpBlocksLogByLabelAndFile(void *data, PDBF pDBF, c_uint nRefOnly,
				 char *label, char *fileName)
{
    blockinfo *pbi        = 0;
    flag       fLast      = TRUE;
    
    // If client calls this fucnction with a null pointer to dump
    // blocks callback function, we fire an assert. The philosophy
    // is that if client calls us to dump blocks, he *must* have
    // a valid callback function. Note that "data" can be null since,
    // it usually means that either we are being called from a
    // C module, or that we are not being called from a C++ class.
    // Clients will typically static_cast<void *>(this) and send
    // it as data. We just return "data" as the first argument of
    // of the callback function we call.

    MAssertSzClient(pDBF != 0,
		    DMM_CE[DMM_CE_CLIENT_CALLBACK_FUNCTION_NULL]);

    MemBuffer[0] = '\n';
    FillBufferWithCharN(MemBuffer + 1, '=', 130);
    strcat(MemBuffer, "\n");
    if ((*pDBF)(data, MemBuffer, PDBF_SEP, NULL) == TRUE) {
	sprintf(MemBuffer, "%-8s %15s  %-48s %-48s %-7s\n", "Address",
		"Size(bytes)", "Label", "FileName", "Line");	    
	if ((*pDBF)(data, MemBuffer, PDBF_HEADING, NULL) == TRUE) {
	    FillBufferWithCharN(MemBuffer, '=', 130); 
	    strcat(MemBuffer, "\n");
	    if ((*pDBF)(data, MemBuffer, PDBF_SEP, NULL) == TRUE) {
		if (pbiHead == NULL) {
		    sprintf(MemBuffer, "\tBlock-list empty\n");  
		    if ((*pDBF)(data, MemBuffer, PDBF_BLKSEMPTY,
				NULL) == TRUE) {
			FillBufferWithCharN(MemBuffer, '=', 130); 
			strcat(MemBuffer, "\n");
			(*pDBF)(data, MemBuffer, PDBF_SEP, NULL);
		    }
		}
		else {
		    pbi        = pbiHead;
		    nDumpBytes = 0; // Start of run.
		    pvDMMUserData         = data;
		    pDBFDMMUserData       = pDBF;
		    nDMMRefOnlyUserData   = nRefOnly;
		    szDMMLabelUserData    = label;
		    if (label)
			nDMMLabelLenUserData = strlen(label);
		    szDMMFileNameUserData = fileName;
		    if (fileName)
			nDMMFileNameLenUserData = strlen(fileName);
		    fLast      = DumpItemsByLabelAndFile(pbi);
		    if (fLast == TRUE) {
			FillBufferWithCharN(MemBuffer, '=', 130);
			strcat(MemBuffer, "\n");
			if ((*pDBF)(data, MemBuffer, PDBF_SEP, NULL) == TRUE) {
			    sprintf(MemBuffer, "%-8s %15d\n", "Total",
				    nDumpBytes);
			    if ((*pDBF)(data, MemBuffer, PDBF_TOTAL,
					NULL) == TRUE) {
				FillBufferWithCharN(MemBuffer, '=', 130);
				strcat(MemBuffer, "\n\n");
				(*pDBF)(data, MemBuffer, PDBF_SEP, NULL);
			    }
			}
		    }
		}
	    }
	}
    }

    IdleTestBlocks();		// What the heck...		
}
#endif // __DEBUG__

#ifdef __DEBUG__
// TestSentinel()
//
// Ensure that none of the sentinel bytes for this memory block have
// been overwritten.

static void TestSentinel(blockinfo *pbi)
{
    uint  i = 0;
    bool  byteGood = true;
    char _the_buf[512] = { '\0' };
    byte *pb = pbi->pb - nSentinel;
  
    for (i = 0; i < nSentinel; i++) {
	if (pb[i] != bSentinelGarbage) {
	    byteGood = false;
	    memset(_the_buf, '\0', 512);
	    sprintf(_the_buf, DMM_CE[DMM_CE_MEMORY_OVERRUN_PRE],
		    nSentinel - i, (uint)pbi->pb, pbi->size,
		    pbi->szLabel, PathLessFileName(pbi->szFileName),
		    pbi->nLine);
	    MAssertSzClient(byteGood == true, _the_buf);
	}
	if (pb[pbi->size + nSentinel + i] != bSentinelGarbage) {
	    byteGood = false;
	    memset(_the_buf, '\0', 512);
	    sprintf(_the_buf, DMM_CE[DMM_CE_MEMORY_OVERRUN_POST],
		    i + 1, (uint)pbi->pb, pbi->size,
		    pbi->szLabel, PathLessFileName(pbi->szFileName),
		    pbi->nLine);
	    MAssertSzClient(byteGood == true, _the_buf);
	}
    }
}
#endif // __DEBUG__

#ifdef __DEBUG__
// pbiGetBlockInfo(byte *pb)
//
// pbiGetBlockInfo searches the memory log to find the block that pb
// points into and returns a pointer to the corresponding blockinfo
// structure of the memory log. Note: pb *must* point into an
// allocated block or you will get an assertion failure; the function
// either asserts or succeeds --- it never returns an error.
//
// blockinfo *pbi;
// ...
// pbi = pbiGetBlockInfo(pb);
//
// pbi->pb points to the start of pb's block, pbi-size is the size of
// the block that pb points into.
	    
static blockinfo *pbiGetBlockInfo(byte *pb)
{
    blockinfo *pbi = 0;
    
    for (pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext) {
	byte *pbStart = pbi->pb;	// for readability.
 	byte *pbEnd   = pbi->pb + pbi->size - 1;

	if (fPtrGrtrEq(pb, pbStart) && fPtrLessEq(pb, pbEnd))
	    break;
    }
    
    // Couldn't find that pointer? Is it (a) garbage? (b) pointing
    // to a block that was freed? or (c) pointing to a block that
    // moved when it was resized by fResizeMemory?

    // Moved test to caller since we want to be able to call
    // fBaptizeBlock() for stack objects which will basically
    // be a no-op.
    // smohanty: 11/12/1997
    
    //    MAssert(pbi != NULL);
    
    return(pbi);
}
#endif // __DEBUG__

#ifdef __DEBUG__
// fCreateBlockInfo(pbNew, sizeNew)
//
// This function creates a log entry for the memory block defined by
// pbNew:sizeNew. The function returns TRUE if it successfully creates
// the log information; FALSE otherwise.
//
// if (fCreateBlockInfo(pbNew, sizeNew))
//     success --- the memory log has an entry.
// else
//     failure --- no entry, so release pbNew.

static flag fCreateBlockInfo(byte *pbNew, size_t sizeNew)
{
    blockinfo *pbi = 0;
    
    MAssertSzInternal(pbNew != 0, DMM_IE[DMM_IE_CREATEBLOCKINFO_BLOCK_NULL]);
    MAssertSzInternal(sizeNew != 0, DMM_IE[DMM_IE_CREATEBLOCKINFO_SIZE_NULL]);
 
    pbi = (blockinfo *) pvAllocBlock(sizeof(blockinfo));
    
    if (pbi != NULL) {
	// Initialize the block.
	memset(pbi, bNewGarbage, sizeof(blockinfo));
	// Start with pristine state.
	pbi->szLabel     = NULL;
	pbi->szFileName  = NULL;
	// Allocate enough space for block label, and initialize it.
	pbi->szLabel = (char *) pvAllocBlock(EXPECTED_MAX_LABEL_LENGTH + 1);
	if (pbi->szLabel != NULL) { // Success
	    // Initialize the block.
	    memset(pbi->szLabel, bNewGarbage, EXPECTED_MAX_LABEL_LENGTH + 1);
 	    strcpy(pbi->szLabel, NEW_BLOCK_LABEL);
 	    pbi->szLabel[NEW_BLOCK_LABEL_LENGTH] = '\0';
 	    pbi->nLabelBlockSize = EXPECTED_MAX_LABEL_LENGTH + 1;
 	}
 	else {			// Failure
 	    memset(pbi, bFreeGarbage, sizeof(blockinfo));
 	    PrivateFreeBlock(pbi);
 	    pbi = NULL;
 	}
 	// And now for the unnamed file.
 	if (pbi != NULL) {
 	    pbi->szFileName = (char *)
 		pvAllocBlock(EXPECTED_MAX_FILENAME_LENGTH + 1);
 	    if (pbi->szFileName != NULL) { // Success.
		// Initialize the block.
		memset(pbi->szFileName, bNewGarbage,
		       EXPECTED_MAX_FILENAME_LENGTH + 1);
 		strcpy(pbi->szFileName, NEW_BLOCK_FILENAME);
 		pbi->szFileName[NEW_BLOCK_FILENAME_LENGTH] = '\0';
 		pbi->nFileNameBlockSize = EXPECTED_MAX_FILENAME_LENGTH + 1;
 	    }
 	    else {		// Failure. Fake a total memory failure.
 		// The label.
 		memset(pbi->szLabel, bFreeGarbage, pbi->nLabelBlockSize);
 		PrivateFreeBlock(pbi->szLabel);
 		pbi->szLabel = NULL;
 		// The block.
 		memset(pbi, bFreeGarbage, sizeof(blockinfo));
 		PrivateFreeBlock(pbi);
 		pbi = NULL;
 	    }
 	}
 	if (pbi != NULL) {
 	    pbi->pb              = pbNew;
 	    pbi->size            = sizeNew;
 	    pbi->pbiNext         = pbiHead;
 	    pbi->nLine           = 0;
 	    pbi->fReferenced     = TRUE;
	    pbi->pvUserData      = NULL;
 	    pbiHead              = pbi;
 	    IncrementAllocCounter(sizeNew);
 	}
    }

    return((flag)(pbi != NULL));
}
#endif // __DEBUG__

#ifdef __DEBUG__
// FreeBlockInfo(pbToFree)
//
// This functions destroys the log entry for the memory block that
// pbToFree points to. pbToFree *must* poit to the start of an
// allocated block; otherwise, you will get an assertion failure.

static void FreeBlockInfo(byte *pbToFree)
{
    blockinfo *pbi = 0, *pbiPrev = 0;
 
    pbiPrev = NULL;
    for (pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext) {
	// Here is where we test equality between pbToFree and the
	// the start of "some" block.
	if (fPtrEqual(pbi->pb, pbToFree)) {
	    if (pbiPrev == NULL) // first time through for-loop.
		pbiHead = pbi->pbiNext;
 	    else
 		pbiPrev->pbiNext = pbi->pbiNext;
 	    break;
 	}
 	pbiPrev = pbi;
    }

    // If pbi is NULL, then pbToFree is invalid.
    MAssertSzInternal(pbi != 0, DMM_IE[DMM_IE_FREEBLOCKINFO_BLOCKINFO_NULL]);
  
    // Did we overwrite the sentinel bytes?
    TestSentinel(pbi);
  
    DecrementAllocCounter(pbi->size);

     // Destroy the contents of *pbi before freeing them.

     // The label.
    memset(pbi->szLabel, bFreeGarbage, pbi->nLabelBlockSize);
    PrivateFreeBlock(pbi->szLabel);
    pbi->szLabel = NULL;

    // The file.
    memset(pbi->szFileName, bFreeGarbage, pbi->nFileNameBlockSize);
    PrivateFreeBlock(pbi->szFileName);
    pbi->szFileName = NULL;

    // The block.
    // To silence BoundsChecker. I have a handle to pbi->pb. It thinks that I am losing
    // pbi->pb. Coufound the brain-dead tool.
    pbi->pb = NULL;
    memset(pbi, bFreeGarbage, sizeof(blockinfo));
    PrivateFreeBlock(pbi);
    pbi = NULL;
}
#endif // __DEBUG__

#ifdef __DEBUG__
// sizeOfBlock(pb)
//
// sizeOfBlock returns the size of the block that pb points to.  pb
// *must* point to the start of an allocated block; otherwise you will
// get an assertion failure.

static size_t sizeOfBlock(byte *pb)
{
    blockinfo *pbi = 0;
    
    pbi = pbiGetBlockInfo(pb);

    MAssertSzInternal(pbi != 0, DMM_IE[DMM_IE_SIZEOFBLOCK_BLOCKINFO_NULL]);
    MAssertSzInternal(pb == pbi->pb,
		      DMM_IE[DMM_IE_SIZEOFBLOCK_MIDDLE_OF_BLOCK]);

    return(pbi->size);
}
#endif // __DEBUG__
