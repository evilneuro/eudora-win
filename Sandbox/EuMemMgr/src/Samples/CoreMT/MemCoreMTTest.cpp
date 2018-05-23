#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#ifdef sun
#include <thread.h>
#endif
#include "MemIfaceMT.h"
#include "DMMDumpHandlerIface.h"

#ifdef __DEBUG__
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef __DEBUG__
extern "C"
flag
DumpBlocksLogCallback(void *pv, c_char * const buf, pDBF_OutType, void *)
{
    DMMDumpHandler *pBO = static_cast<DMMDumpHandler *>(pv);
    pBO->ProcessOutput(buf);
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
    if (fNewBlockMT((void **)&p, (*pTheArgument) * 2)) {
	char buf[28];
	sprintf(buf, "Thread %d", *pTheArgument);
	fBaptizeBlockMT(p, buf);
	sprintf(p, "Thread %d", *pTheArgument);
	if ((*pTheArgument % 2) != 0) {
	    FreeBlockMT((void *) p);
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
    DMMDumpHandler _blockOutput;
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
	::DumpBlocksLogByLabelAndFileMT((void *) &_blockOutput,
					DumpBlocksLogCallback, 0, 0, 0);
#endif // __DEBUG__
	sleep(duration);
	counter += duration;
    }
    
    ::ShutdownDebugMemoryManagerMT();

    return(0);
}

	
