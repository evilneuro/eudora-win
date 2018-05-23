#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#ifdef sun
#include <thread.h>
#endif

#include "NewIfaceMT.h"
#include "DMMDumpHandlerIface.h"

#ifdef __DEBUG__
#define DEBUG_NEW new(THIS_FILE, __LINE__)
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifdef __DEBUG__
extern "C"
flag
DumpBlocksLogCallback(void *pv, c_char * const buf, pDBF_OutType pDBF_OT,
		      void *pBlock)
{
    DMMDumpHandler *dmmDH = static_cast<DMMDumpHandler *>(pv);
    dmmDH->ProcessOutput(buf);

    // Then print the first few bytes if this is a leak block...
    if (pDBF_OT == PDBF_ITEM) {
	char _the_buf[33] = { '\0' };
	int   i = 0;
	for (i = 0; i < 3; i++) {
	    _the_buf[i] = '>';
	}

	MAssertSzInternalMT(pBlock != NULL, "How did this Happen?");
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

	dmmDH->ProcessOutput(_the_buf);
	dmmDH->ProcessOutput("\n");
    }

    return(TRUE);
}
#endif // __DEBUG__

extern "C"
void *
the_one_and_only_thread_routine(void *arg)
{
    int *pTheArgument = static_cast<int *>(arg);

    char *p;
    sleep(*pTheArgument);
    p = new char[(*pTheArgument) * 2];
    if (p != NULL) {
	char buf[28] = { '\0' };
	sprintf(buf, "Thread %d", *pTheArgument);
	fBaptizeBlockMT(p, buf);
	if ((*pTheArgument % 2) != 0) {
	    delete [] p;
	}
    }
	
    return(arg);
}

int
main(int, char **)
{

#ifdef sun
    thr_setconcurrency(2);
#endif

#ifdef __DEBUG__
    DMMDumpHandler dmmDH;
#endif // __DEBUG__

    ::InitializeDebugMemoryManagerMT();

    int            counter = 0;
    int            duration = 20;
    int            status;
    pthread_t      threadArray[15];
    int            threadArgArray[15] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
                                 
       
    pthread_attr_t thread_attr;

    // Create fifteen threads.
    status = pthread_attr_init(&thread_attr);
    assert(status == 0);

    status = pthread_attr_setdetachstate(&thread_attr,
					 PTHREAD_CREATE_DETACHED);
    assert(status == 0);

    for (int i = 0; i < 15; i++) {
	status = pthread_create(&threadArray[i], &thread_attr,
				the_one_and_only_thread_routine,
				static_cast<void *>(&threadArgArray[i]));
	assert(status == 0);
    }

    while (counter < 60) {

// Dump 1 --- all blocks.
#ifdef __DEBUG__
	printf("\n\nDump %d\n\n", (counter / duration) + 1);
	::DumpBlocksLogByLabelAndFileMT((void *) &dmmDH,
					DumpBlocksLogCallback, 0, 0, 0);
#endif // __DEBUG__

	sleep(duration);
	counter += duration;
    }

    char *_p = new char[10];
    delete _p;

    char *_g = new char [12];
    strcpy(_g, "Praveen Yaramada");

    delete [] _g;

    sleep(200);

    ::ShutdownDebugMemoryManagerMT();

    return(0);
}

	
