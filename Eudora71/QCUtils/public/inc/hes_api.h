/*
 *   Redistribution and use for noncommercial purposes in source and binary
 * forms are permitted provided that the above copyright notice and this
 * paragraph are duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such distribution
 * and use acknowledge that the software was developed at Stanford University.
 * 
 *   Copying or redistribution for sale or incorporation in derivative works
 * which are sold or use in providing services for fee are prohibited and in
 * violation of owners copyrights.  Any revenue producing copying or uses
 * are subject to royalty license to be negotiated with the copyright owner.
 *
 *   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.  STANFORD UNIVERSITY
 * DOES NOT WARRANT THAT THE USE OF THIS SOFTWARE WILL NOT INFRINGE ANY
 * COPYRIGHT.
 */

/*
   Hes_api.h
*/

#ifdef __cplusplus
extern "C" {
#endif


/* Error codes. */
#define	HES_ER_UNINIT	-1		/* uninitialized */
#define	HES_ER_OK		0		/* no error */
#define	HES_ER_NOTFOUND	1		/* Hesiod name not found by server */
#define HES_ER_CONFIG	2		/* local problem (no config file?) */
#define HES_ER_NET		3		/* network problem */


/* Declaration of interface routines */

#ifdef MSDOS
#define	HESAPI	PASCAL __export 
#else
#define	HESAPI	CALLBACK
#endif

extern char * FAR HESAPI hes_to_bind(char FAR * HesiodName, char FAR * HesiodNameType);
extern char ** FAR HESAPI hes_resolve(char FAR * HesiodName, char FAR * HesiodNameType);
extern int FAR HESAPI hes_error();

#ifdef __cplusplus
}
#endif  /* __cplusplus */
