#include <stdio.h>
#include <string.h>
#define __MEM_CORE_TEST_C__
#include "MemInternalIface.h"

#ifdef __DEBUG__
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef __DEBUG__
// forward declaration.
flag
DumpBlocksLogCallback(void *pv, c_char * const buf, pDBF_OutType pDBF,
		      void *p);
#endif // __DEBUG__

#ifdef __DEBUG__
#if defined(WIN32)
flag
DumpBlocksLogCallback(void *, c_char * const buf, pDBF_OutType, void *)
#elif defined(__unix__)
flag
DumpBlocksLogCallback(void *pv, c_char * const buf, pDBF_OutType pDBF_OT,
		      void *pBlock)
#else
#error
#error If highest warning levels are used (which we use), unused parameter
#error names are an error under Win32 while unspecified parameter names
#error are an error under UNIX. How does your compiler behave?
#error
#endif
{
#if defined(WIN32)
    OutputDebugString(buf);
#elif defined(__unix__)
    printf("%s", buf);
#else
#error Error: Developer, please provide implementation of dumping
#error Error: memory blocks for your platform ...
#endif // WIN32
    return(TRUE);
}
#endif // __DEBUG__

#if defined(WIN32)
int
main(int, char **)
#elif defined(__unix__)
int
main(int argc, char **argv)
#else
#error
#error If highest warning levels are used (which we use), unused parameter
#error names are an error under Win32 while unspecified parameter names
#error are an error under UNIX. How does your compiler behave?
#error
#endif
{
    char *p = 0;
    InitializeDebugMemoryManager();

    if (fNewBlock((void **)&p, 256)) {
	fBaptizeBlock(p, "");
	strcpy(p, "The quick brown fox jumped over the lazy dog.");
    }

    if (p) {
	printf("%s\n", p);
    }

    if (fNewBlock((void **)&p, 1024)) {
	fBaptizeBlock(p, "Char-Memory2");
	strcpy(p, "String2");
    }

    if (p) {
	printf("%s\n", p);
    }

    if (fNewBlock((void **)&p, 12)) {
	fBaptizeBlock(p, "Char-Memory3");
	strcpy(p, "StringLingLingLing");
    }

    if (p) {
	printf("%s\n", p);
	FreeBlock(p);
    }

// Dump 1 --- all blocks.
#ifdef __DEBUG__
    printf("\n\nDump 1\n\n");
    DumpBlocksLogByLabelAndFile(NULL, DumpBlocksLogCallback,
				0, 0, 0);
#endif // __DEBUG__

// Dump 2 --- by file: you can specify the first few characters of the
//                     the file, and in any case. For example, the
//                     following dumps will report the same blocks.
#ifdef __DEBUG__
    printf("\n\nDump 2\n\n");
    DumpBlocksLogByLabelAndFile(NULL, DumpBlocksLogCallback,
				0, 0, "MemCoreTest.cpp");
    DumpBlocksLogByLabelAndFile(NULL, DumpBlocksLogCallback,
				0, 0, "memcoretest.cpp");
    DumpBlocksLogByLabelAndFile(NULL, DumpBlocksLogCallback,
				0, 0, "memcor");
#endif // __DEBUG__

// Dump 3 --- by label: you can specify the first few characters of the
//                      the label, and in any case too.
#ifdef __DEBUG__
    printf("\n\nDump 3\n\n");
    DumpBlocksLogByLabelAndFile(NULL, DumpBlocksLogCallback,
				0, "Char-Memory1", 0);
    DumpBlocksLogByLabelAndFile(NULL, DumpBlocksLogCallback,
				0, "Char-Memory2", 0);
#endif // __DEBUG__

// Dump 4 --- by label and file : blocks of a certain label *and* file.
#ifdef __DEBUG__
    printf("\n\nDump 4\n\n");
    DumpBlocksLogByLabelAndFile(NULL, DumpBlocksLogCallback,
				0, "Char-Memory1", "mEmCoReTeSt.CpP");
#endif // __DEBUG__

    ShutdownDebugMemoryManager();

    return(0);
}

	
