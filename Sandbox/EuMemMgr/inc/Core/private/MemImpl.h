///////////////////////////////////////////////////////////////////////////////
// MemImpl.h
// 
// This file is to be included only in MemImpl.cpp, the implementation
// of the Debug Memory Manager.
//
// Created:  05/12/97 smohanty
// Modified: 01/16/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_IMPL_H_
#define __MEM_IMPL_H_

// This file is included in
//
//     MemImpl.cpp
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_IMPL_CPP__)
#if defined(WIN32)
#error This file is to be included only in \
MemImpl.cpp
#else
#error This file is to be included only in
#error
#error     MemImpl.cpp
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#if defined(WIN32)
#include <string.h>
#ifdef __DEBUG__
#include <crtdbg.h>
#endif // __DEBUG__
#endif

#if defined(__unix__)
#include <iostream.h>
#include <string.h>
#endif // __unix__

#include "MemInternalIface.h"
#include "MemAssertUtilsInternalIface.h"

// Reserve memory 128K.
#define N_RESERVE_MEMORY 131072

#if !defined(TRUE) && !defined(FALSE)
static c_uint
TRUE  = 1;
static c_uint
FALSE = 0;
#endif

#ifdef __DEBUG__

static c_char *const
NEW_BLOCK_LABEL               = "ANONYMOUS";
static c_char *const
NEW_BLOCK_FILENAME            = "XXXXXXXX.XXX";
static c_uint
NEW_BLOCK_LABEL_LENGTH        = 9;  // Length of "NEWBORN".
static c_uint
NEW_BLOCK_FILENAME_LENGTH     = 12; // Length of "XXXXXXXX.XXX".
static c_uint
EXPECTED_MAX_LABEL_LENGTH     = 32; 
static c_uint
EXPECTED_MAX_FILENAME_LENGTH  = 32;

MemMgrAllocInfo memMgrAllocInfo;

typedef struct BLOCKINFO {
    struct   BLOCKINFO *pbiNext;
    byte               *pb;	            // Start of block.
    size_t              size;	            // Length of block.
    flag                fReferenced;        // Ever referenced?
    char               *szLabel;	    // Memory label.
    uint                nLabelBlockSize;    // Length of label.
    char               *szFileName;         // File location of allocation.
    uint                nFileNameBlockSize; // Length of file name.
    uint                nLine;	            // Line Location of allocation.
    void               *pvUserData;
} blockinfo;		   	            // Naming: blockinfo *pbi.

// Keep track of allocated bytes.
static void
IncrementAllocCounter(size_t sizeBytes);

static void
DecrementAllocCounter(size_t sizeBytes);

// Basic log information.
static flag
fCreateBlockInfo(byte *pbNew, size_t sizeNew);

static void
FreeBlockInfo(byte *pbToFree);

static blockinfo *
pbiGetBlockInfo(byte *pb);
//static void       UpdateBlockInfo(byte *pbOld, byte *pbNew, size_t size);

static size_t
sizeOfBlock(byte *pb);

static void
TestSentinel(blockinfo *pbi);

// The functions in this file must compare arbitrary pointers, an
// operation that the ANSI standard does not guarantee to be portable.
//
// The macros below isolate the pointer comparisons needed in this
// file. The implementation assumes "flat" pointers, for which
// straightforward comparisons will always work. The definitions below
// will not work for some of the common 80x86 models.

#define fPtrLess(pLeft, pRight)   (((uint) pLeft)  < ((uint) pRight))
#define fPtrGrtr(pLeft, pRight)   (((uint) pLeft)  > ((uint) pRight))
#define fPtrEqual(pLeft, pRight)  (((uint) pLeft) == ((uint) pRight))
#define fPtrLessEq(pLeft, pRight) (((uint) pLeft) <= ((uint) pRight))
#define fPtrGrtrEq(pLeft, pRight) (((uint) pLeft) >= ((uint) pRight))

static c_byte
bNewGarbage                = 0xCD;

static c_byte
bFreeGarbage               = 0xDD;

static c_byte
bSentinelGarbage           = 0xFD;

#if defined(__unix__)
static c_uint
nSentinel                  = 8;
#elif defined(WIN32)
static c_uint
nSentinel                  = 4;
#else
static c_uint
nSentinel                  = 4;
#endif
#endif // __DEBUG__

static flag
fHeapInsolvent = 0;	// Heap insolvent.

#ifdef __DEBUG__
static uint
nBytesAllocated            = 0;

static uint
nMaxBytesAllocated         = 0;

static flag
fTestOnAllocOrFree         = 0;

static char
MemBuffer[1024]            = { '\0' };

static int
nDumpBytes                 = 0;

// New
static void *
pvDMMUserData              = 0;

static PDBF
pDBFDMMUserData            = 0;

static uint
nDMMRefOnlyUserData        = 0;

static char *
szDMMLabelUserData         = 0;

static uint
nDMMLabelLenUserData       = 0;

static char *
szDMMFileNameUserData      = 0;

static uint
nDMMFileNameLenUserData    = 0;

// Added 01/14/98
c_char * const
_strAcquiredReserveMem =
"\tAcquired %d bytes Reserve Memory.\n";

c_char * const
_strFreedReserveMem =
"\tFreed %d bytes Reserve Memory\n";

c_char * const
_strDetectMemLeak =
"\tDetected Memory Leak\n\t\t%d bytes.\n\tDumping log.\n";

// DMM internal error.
enum {
DMM_IE_PBLOCK_NULL = 0,
DMM_IE_REQUESTING_ZERO_BYTES = 1,
DMM_IE_DEALLOCATING_NULL = 2, 
DMM_IE_FILENAME_TO_STRIP_PATH_FROM_NULL = 3,
DMM_IE_DUMPITEMS_BLOCKINFO_NULL = 4, 
DMM_IE_NORMALIZE_CASE_STRCMP_FILTER_NULL = 5, 
DMM_IE_NORMALIZE_CASE_STRCMP_STR_NULL = 6,
DMM_IE_NORMALIZE_CASE_STRCMP_FILTER_LEN_INVALID = 7,
DMM_IE_NORMALIZE_CASE_STRCMP_STR_LEN_INVALID = 8,
DMM_IE_DUMPITEMSBYLABELANDFILE_BLOCKINFO_NULL = 9,
DMM_IE_CREATEBLOCKINFO_BLOCK_NULL = 10,
DMM_IE_CREATEBLOCKINFO_SIZE_NULL = 11,
DMM_IE_FREEBLOCKINFO_BLOCKINFO_NULL = 12,
DMM_IE_SIZEOFBLOCK_BLOCKINFO_NULL = 13,
DMM_IE_SIZEOFBLOCK_MIDDLE_OF_BLOCK = 14
};

c_char * const DMM_IE[] = {
    "pBlock is equal to NULL for PDBF_ITEM.",
    "Requesting for zero bytes of memory.",
    "Deallocating a NULL address.",
    "Filename to strip path from is NULL.",
    "Dumpitems blockinfo is equal NULL.",
    "Normalized-case word compare filter is NULL.",
    "Normalized-case word compare string is NULL.",
    "Normalized-case word compare filter length should be > 0.",
    "Normalized-case word compare string length should be > 0.",
    "DumpItemsByLabelAndFile blockinfo is NULL.",
    "fCreateBlockInfo block is NULL.",
    "fCreateBlockInfo size is NULL.",
    "FreeBlockInfo blockinfo NULL.",
    "sizeOfBlock blockinfo NULL.",
    "sizeOfBlock middle of block."
};

// DMM client error.
enum {
DMM_CE_INIT_TWICE = 0,
DMM_CE_REQUESTING_ZERO_BYTES = 1,
DMM_CE_DEALLOCATING_NULL = 2,
DMM_CE_POINTER_ADDRESS_NULL = 3,
DMM_CE_DEALLOCATING_INVALID_BLOCK = 4,
DMM_CE_SAVING_DATA_ON_NULL_BLOCK = 5,
DMM_CE_SAVING_DATA_ON_INVALID_BLOCK = 6,
DMM_CE_RETRIEVING_DATA_ON_NULL_BLOCK = 7,
DMM_CE_RETRIEVING_DATA_ON_INVALID_BLOCK = 8,
DMM_CE_RETRIEVING_LINENUMBER_ON_NULL_BLOCK = 9,
DMM_CE_RETRIEVING_LINENUMBER_ON_INVALID_BLOCK = 10,
DMM_CE_RETRIEVING_FILENAME_ON_NULL_BLOCK = 11,
DMM_CE_RETRIEVING_FILENAME_ON_INVALID_BLOCK = 12,
DMM_CE_BAPTIZING_NULL_BLOCK = 13,
DMM_CE_BAPTISMAL_LABEL_NULL = 14,
DMM_CE_BAPTISMAL_LABEL_LENGTH_INVALID = 15,
DMM_CE_BAPTIZING_MIDDLE_OF_BLOCK = 16,
DMM_CE_NOTING_ORIGIN_OF_NULL_BLOCK = 17,
DMM_CE_ORIGIN_NULL = 18,
DMM_CE_ORIGIN_LENGTH_NULL = 19,
DMM_CE_NOTING_ORIGIN_OF_MIDDLE_OF_BLOCK = 20,
DMM_CE_CLIENT_CALLBACK_FUNCTION_NULL = 21,
DMM_CE_MEMORY_OVERRUN_PRE = 22,
DMM_CE_MEMORY_OVERRUN_POST = 23
};

c_char * const DMM_CE[] = {
    "Initializing DebugMemoryManager twice?",
    "Requesting for zero bytes of memory.",
    "Deallocating a NULL address.",
    "Address of pointer to receive memory is NULL.",
    "Deallocating invalid block.",
    "Saving user data on NULL block.",
    "Saving user data on invalid block.",
    "Retrieving user data on NULL block.",
    "Retrieving user data on invalid block.",
    "Retrieving line number information on NULL block.",
    "Retrieving line number information on invalid block.",
    "Retrieving file name information on NULL block.",
    "Retrieving file name information on invalid block.",
    "Baptizing null block.",
    "Baptismal label is null.",
    "Baptismal label length should be greater than zero.",
    "Baptizing middle of block.",
    "Noting origin of null block.",
    "Origin is null.",
    "Origin length should be greater than zero.",
    "Noting origin of middle of block.",
    "Client supplied callback function is NULL.",
    "Memory overrun at byte %d immediately before block\n\taddress=%08X\n\tsize=%d\n\tlabel=%s\n\tfilename=%s\n\tline=%d",
    "Memory overrun at byte %d immediately after block\n\taddress=%08X\n\tsize=%d\n\tlabel=%s\n\tfilename=%s\n\tline=%d"
};

#endif // __DEBUG__
#endif // __MEM_IMPL_H_
