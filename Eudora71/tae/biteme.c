#include "TAE.h"

#include <stdio.h>

#include <string.h>

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#ifndef _timeb
#define _timeb timeb
#define _ftime ftime
#endif

#include "TAEDictionary.h"

// should be in TAEDictionary.h:

#define kClusterObject				0x0000
#define kCategoryObject				0x0001
#define kCollectionObject			0x0002
#define kEndOfObjects				0x0003

#define kNumberEdgesMask			0x7FFF
#define kEndMatchToRead				0x8000

#define TAE_MAXSHORT				32767

#define TAE_ToLower(c)				(((c >= 'A') && (c <= 'Z')) ? (c + 32) : c)

#define SOFARSIZE 1023
#define DEBUG 0

char sofar_buf[SOFARSIZE + 1];

void TAExplode(struct TAEDictionary *ptaedict, unsigned int iPos, 
	       char* sofar, unsigned int sofarindex,
	       char* hint, int hintsize, int* printcount)
{
  int cur;

  if (!ptaedict) {
    return;
  }

  if (DEBUG) {
    fprintf(stderr,"> iPos=%d index=%d '%s'\n",iPos,sofarindex,sofar);
  }

  if (iPos <= ptaedict->iDataLen) {
    short nEndMatchCollectionID = 0;
    short nTranslatedID = 0;
    short nTotalEdges = 0;

    //	Read in the number of edges
    nTotalEdges = TAEStream_ReadShortAt(&(ptaedict->taestream), &iPos);
    if (DEBUG) {
      fprintf(stderr,"> nTotalEdges=%d\n",nTotalEdges);
    }

    //	Read in the end match collection ID if it exists
    if ((nTotalEdges & kEndMatchToRead) != 0) {
      nEndMatchCollectionID = TAEStream_ReadShortAt(&(ptaedict->taestream), &iPos);
      nTranslatedID = (short)(nEndMatchCollectionID + 1);
      if (DEBUG) {
	fprintf(stderr,"> nTranslatedID=%d\n",nTranslatedID);
      }
    }

    //	Discard the end match exists bit from the number of edges
    nTotalEdges &= kNumberEdgesMask;

    if ((nTotalEdges < 0) || (nTotalEdges > (unsigned int) TAE_MAXSHORT)) {
      //	Illegal number of edges, the file is corrupted
      //	Note that we aren't actually limited to less than 32,767 edges,
      //	this is just a sanity check that can be adjusted.
      fprintf(stderr, "\nIllegal number of edges: %s the file is corrupted\n", nTotalEdges);
      return;
    } else if (nTotalEdges > 0) {
      //	Read in the edges
      char cEdgeCharacter = '\0';
      int last = nTotalEdges-1;
      int origPos = iPos;

      if ((hintsize <= 0) || (hint == NULL)) {
	if (last > 0) {
	  cur = rand() % last;
	} else {
	  cur = 0;
	}
      } else {
	cur = 0;
      }

      for (; cur <= last; cur++) {
	iPos = origPos + cur*(2+sizeof(int));
	cEdgeCharacter = TAEStream_ReadCharAt(&(ptaedict->taestream), &iPos);	

	if (DEBUG) {
	  fprintf(stderr,"> Edgechar #%d: %c\n",cur,cEdgeCharacter);
	}
	if ((hintsize <= 0) || (hint == NULL) || (*hint == cEdgeCharacter)) {
	  sofar[sofarindex++] = cEdgeCharacter;
	  sofar[sofarindex] = '\0';

	  if (nTranslatedID != 0) {
	    if (*printcount > 0) {
	      (*printcount)--;
	      fprintf(stdout,"%s\n", sofar);
	    }
	    break;
	  } else {
	    iPos = TAEStream_ReadIntAt(&(ptaedict->taestream), &iPos) + ptaedict->lDataStart;
	    if (--hintsize > 0) {
	      TAExplode(ptaedict, iPos, sofar, sofarindex,++hint,hintsize,printcount);
	    } else {
	      TAExplode(ptaedict, iPos, sofar, sofarindex,NULL,0,printcount);
	    }
	    sofar[--sofarindex] = '\0';
	  }
	} else {

	}
      }
    }
  } else {
    fprintf(stderr,"<Read past end of stream iPos=%d?\n",iPos);
  }
  if (DEBUG) {
    fprintf(stderr,"<\n");
  }
}

main(int argc, char **argv)
{
	struct TAEDictState		taeds;

	// Diagnostics: Variables for timing
	struct _timeb	 tstruct;
	long			 lBeginTime = 0;
	long			 lEndTime = 0;
	int              printcount = 1;

	// Diagnostics: Get start time
	_ftime( &tstruct );
	lBeginTime = tstruct.millitm;

	srand(lBeginTime);

	if (TAEInitDictionary(&taeds, "dict.dat")) {
	  // the dictionary is now in taeds.pvdict
	  int i;

	  if (DEBUG) {
	    fprintf(stderr,"Loaded dictionary dict.dat\n");
	  }


	  for (i = 0; i < SOFARSIZE; i++) {
	    sofar_buf[i] = '\0';
	  }

	  TAExplode(taeds.pvdict,((struct TAEDictionary *)(taeds.pvdict))->iDictionaryRootPos,sofar_buf,0,"you ",4,&printcount);
	  TAECloseDictionary(&taeds);
	} else {
	  fprintf(stderr,"ERROR: failed to open dictionary dict.dat\n");
	}
	return 0;
}
