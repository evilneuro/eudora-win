// WIN32 compatibility routines.


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.


#include <ctype.h>
#include <stdio.h>
#include <assert.h>
// #include "netstream.h"

#include "exports.h"
#include "ImapDefs.h"
#include "mm.h"
#include "osdep.h"
#include <time.h>

#include "DebugNewHelpers.h"


extern const char *days[];	/* day name strings */
extern const char *months[];	/* month name strings */

// Disable "assignment within conditional expression" warning:
#pragma warning( disable : 4706 )

static void do_date (char *date,char *prefix,char *fmt,int suffix, time_t tTime);



// ============================ CRString class ==================/
CRString::CRString(UINT StringID)
{
	// Set this so the resource gets loaded from the DLL's resource.
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
	LoadString(StringID);
}



// ================ MEMORY allocation routines ===================//


/* Get a block of free storage
 * Accepts: size of desired block
 * Returns: free storage block
 */

void *fs_get (size_t size)
{
	void *block = NULL;
	size_t len	= size ? size : (size_t) 1;

	block = calloc (1, len);

	if (!block)
		fatal ("Out of free storage");

	return (block);
}


/* Resize a block of free storage
 * Accepts: ** pointer to current block
 *	    new size
 */

void fs_resize (void **block,size_t size)
{
  if (!(*block = realloc (*block,size ? size : (size_t) 1)))
    fatal ("Can't resize free storage");
}


/* Return a block of free storage
 * Accepts: ** pointer to free storage block
 */

void fs_give (void **block)
{
	if ( block && *block )
	{
		free (*block);
		*block = NIL;
	}

}


// ======== fatal ==================//

/* Report a fatal error
 * Accepts: string to output
 */

void fatal (char *string)
{
  mm_log (string, IMAPERROR);		/* pass up the string */

  abort ();			/* die horribly */
}



// ===================== nl_nt ===============================//

/* Copy string with CRLF newlines
 * Accepts: destination string
 *	    pointer to size of destination string buffer
 *	    source string
 *	    length of source string
 * Returns: length of copied string
 */

unsigned long strcrlfcpy (char **dst,unsigned long *dstl,char *src,
			  unsigned long srcl)
{
				/* flush destination buffer if too small */
  if (*dst && (srcl > *dstl)) fs_give ((void **) dst);
  if (!*dst) {			/* make a new buffer if needed */
    *dst = (char *) fs_get ((size_t) (*dstl = srcl) + 1);
    if (dstl) *dstl = srcl;	/* return new buffer length to main program */
  }
				/* copy strings */
  if (srcl) memcpy (*dst,src,(size_t) srcl);
  *(*dst + srcl) = '\0';	/* tie off destination */
  return srcl;			/* return length */
}


/* Length of string after strcrlfcpy applied
 * Accepts: source string
 * Returns: length of string
 */

unsigned long strcrlflen (STRING *s)
{
  return SIZE (s);		/* no-brainer on DOS! */
}


//====================== env_nt =================================//



/* Write current time
 * Accepts: destination string
 *	    optional format of day-of-week prefix
 *	    format of date and time
 *	    flag whether to append symbolic timezone
 */

// Modification by JOK - 4/13/98 - can now pass in a time_t.
// 

static void do_date (char *date,char *prefix,char *fmt,int suffix, time_t tTime)
{
	time_t tn;

	if (tTime)
		tn = tTime;
	else
		tn = time (0);

	struct tm *t = gmtime (&tn);

	int zone = t->tm_hour * 60 + t->tm_min;
	int julian = t->tm_yday;

	t = localtime (&tn);		/* get local time now */

				/* minus UTC minutes since midnight */
	zone = t->tm_hour * 60 + t->tm_min - zone;
	/* julian can be one of:
	*  36x  local time is December 31, UTC is January 1, offset -24 hours
	*    1  local time is 1 day ahead of UTC, offset +24 hours
	*    0  local time is same day as UTC, no offset
	*   -1  local time is 1 day behind UTC, offset -24 hours
	* -36x  local time is January 1, UTC is December 31, offset +24 hours
	*/
	if (julian = t->tm_yday -julian)
    zone += ((julian < 0) == (abs (julian) == 1)) ? -24*60 : 24*60;

	if (prefix)
	{			/* want day of week? */
		sprintf (date,prefix,days[t->tm_wday]);
		date += strlen (date);	/* make next sprintf append */
	}
				/* output the date */
	sprintf (date,fmt,t->tm_mday,months[t->tm_mon],t->tm_year+1900,
	   t->tm_hour,t->tm_min,t->tm_sec,zone/60,abs (zone) % 60);

	if (suffix)
	{			/* append timezone suffix if desired */
		char *tz;
		tzset ();			/* get timezone from TZ environment stuff */
		tz = tzname[daylight ? (((struct tm *) t)->tm_isdst > 0) : 0];
		if (tz && tz[0]) sprintf (date + strlen (date)," (%s)",tz);
	}
}




/* Write current time in RFC 822 format
 * Accepts: destination string
 */

void rfc822_date (char *date)
{
	do_date (date,"%s, ","%d %s %d %02d:%02d:%02d %+03d%02d",T, 0);
}


/* Write current time in internal format
 * Accepts: destination string
 */

void internal_date (char *date)
{
	do_date (date,NIL,"%02d-%s-%d %02d:%02d:%02d %+03d%02d",NIL, 0);
}


void append_date (char *date, time_t tTime)
{
	do_date (date,NIL,"%02d-%s-%d %02d:%02d:%02d %+03d%02d",NIL, tTime);
}



#endif // IMAP4

