
/*
 *  scallback.c - contains callback functions for memory allocation/deallocation.
 * 
 *  Functions:
 *    CD_malloc() - allocates memory and returns pointer to allocated buffer.
 *    CD_free() - frees memory buffer
 *    CD_realloc() - reallocates memory and returns pointer to new buffer.
 *    CD_memset() - call to the function memset()
 *    CD_memcpy() - call to the function memcpy()
 *    CD_memcmp() - call to the function memcmp()
 *    CD_time() - returns proper time with epoch difference.
 *
 */
#include "stdafx.h"

#include <stddef.h>		/* Contains definition of size_t */
#include <stdlib.h>
#include <string.h>



#include <time.h>

#include "QCSSL.h"

/*
 * Simple callbacks
 *
 * These callbacks just serve as glue between the OS APIs
 * and SSL Plus.
 *
 * Your programs can probably use these callbacks unmodified.
 */

/* The size_t declarations for the parameters have been changed back
 * to uint32_t for the win16 port.
 */

void* CD_malloc(uint32_t size)
{	return malloc((size_t)(size == 0 ? 1 : size));
}

void CD_free(void *block)
{	free(block);
}

void* CD_realloc(void *block, uint32_t newSize)
{	
	void * ptr;
	if (block == 0)
		ptr = CD_malloc(newSize);
	else
		ptr = realloc(block, (size_t)(newSize == 0 ? 1 : newSize));
	return ptr;
}

void CD_memset(void *block, int value, uint32_t length)
{	memset(block, value, (size_t)length);
}

/* Even though it's called memcpy, it needs to handle overlapping regions */
void CD_memcpy(void *dest, const void *src, uint32_t length)
{	memmove(dest, src, (size_t)length);
}

int CD_memcmp(const void *a, const void *b, uint32_t length)
{	return memcmp(a, b, (size_t)length);
}


#define EPOCH_DIFFERENCE	0L
uint32_t
CD_time(void)
{	
    return time(0) + EPOCH_DIFFERENCE;
}
