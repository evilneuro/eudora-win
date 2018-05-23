// depend.h - compatibility routines for NT.


// Utility class:
// Data Types

class CRString : public CString
{
public:
	CRString(UINT StringID);
};



// ============== fs_nt ================/
void *fs_get (size_t size);
void fs_resize (void **block,size_t size);
void fs_give (void **block);

// =========== ftl_nt ==================/
void fatal (char *string);

//============ nl_nt ==================/
unsigned long strcrlfcpy (char **dst,unsigned long *dstl,char *src,
			  unsigned long srcl);
unsigned long strcrlflen (STRING *s);

// =========== env_nt ====================/
static void do_date (char *date,char *prefix,char *fmt,int suffix);
void rfc822_date (char *date);
void internal_date (char *date);
void append_date (char *date, time_t tTime);


