#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reltoabs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


#define False	0
#define True	1

typedef struct
{
	char	scheme[ 32 ];
	BOOL	isSite;
	char	site[ 128 ];
	char	path[ 256 ];
	BOOL	isQuery;
	char	query[ 255 ];
	BOOL	isFragment;
	char	fragment[ 255 ];
} DeepURL, *DeepURLPtr;

static LPSTR DuhString(LPSTR url, DeepURLPtr duh);
static LPSTR URLCombinePaths(LPSTR into,LPSTR base,LPSTR rel);
static BOOL TrailingDotDot(LPSTR string);
static LPSTR RemLastComponent(LPSTR path);
static BYTE URLToken(LPSTR url,LPSTR token,LPSTR *spot);
static void DeepURLParse(LPSTR url, DeepURLPtr duh);
static LPSTR PToken(LPSTR string, LPSTR token, LPSTR *spotP, LPSTR delims);
static void RemoveChar( char it, LPSTR string );


/************************************************************************
 * URLCombine - combine a base and a relative url
 ************************************************************************/
LPSTR URLCombine( LPSTR result, LPSTR base, LPSTR rel)
{
	DeepURLPtr baseURL = new DeepURL;
	DeepURLPtr relURL = new DeepURL;
	char localResult[ 256 ];

	// initialize the DeepURL objects
	memset( baseURL, 0, sizeof( DeepURL ) );
	memset( relURL, 0, sizeof( DeepURL ) );
	
	// move the relative url in; it may or may not stay
	strcpy( localResult, rel );
	
	if (baseURL && relURL)
	{
		DeepURLParse(rel,relURL);
		// if the relative URL has a scheme, we're done already
		if (! relURL->scheme[0] )
		{
			DeepURLParse(base,baseURL);
			// if the base URL doesn't have a scheme, we're done
			if ( baseURL->scheme[0] )
			{
				// sigh.  Here we go.
				
				// first, we copy the scheme, since to get here the base must
				// have a scheme and the relative must not
				strcpy( relURL->scheme, baseURL->scheme );
				
				// if the site is defined, we're pretty much done
				if (!relURL->isSite)
				{
					// nope.  inherit the site 
					// KCM - yes '='
					if (relURL->isSite = baseURL->isSite)
						strcpy( relURL->site, baseURL->site );
					
					// Combine the paths
					URLCombinePaths( relURL->path, baseURL->path, relURL->path );
				}
				DuhString(localResult,relURL);
			}
		}
	}
	
	delete baseURL;
	delete relURL;

	return(strcpy(result,localResult));
}

/************************************************************************
 * DuhString - convert a deep url handle to a string
 ************************************************************************/
static LPSTR DuhString(LPSTR url, DeepURLPtr duh)
{
	*url = 0;
	
	// All duh's have schemes
	strcat(url,duh->scheme);
	strcat(url,":");
	
	// May or may not have a site.  If we do, add // and site
	if ( duh->isSite )
	{
		strcat(url,"/");
		strcat(url,"/");
		strcat(url,duh->site);
	}
	
	// May or may not have a path.  (Weird, but...)
	// if we do, concatenate.  Will have leading slash already
	if (duh->path)
	{
		strcat(url,duh->path);
	}
	
	// May or may not have a query.  If we do, add ? and query
	if (duh->isQuery)
	{
		strcat(url,"?");
		strcat(url,duh->query);
	}
	
	// May or may not have a fragment id.  If we do, add # and fragment
	if (duh->isFragment)
	{
		strcat(url,"#");
		strcat(url,duh->fragment);
	}
	
	return(url);
}

/************************************************************************
 * URLCombinePaths - weld two paths into one
 ************************************************************************/
static LPSTR URLCombinePaths(LPSTR into,LPSTR base,LPSTR rel)
{
	char	localInto[256];
	char	token[256];
	LPSTR	spot;
		
  if (! strlen(rel))
  {
  	strcpy(localInto,base);						// empty rel
  	RemLastComponent(localInto);
  	if (strlen(localInto) && localInto[strlen(localInto)-1]!='/') 
		strcat(localInto,"/");
  }
  else if (!strlen(base)) strcpy(localInto,rel);		// empty base
  else if (rel[0]=='/') strcpy(localInto,rel);	// absolute rel
  else
  {
  	// Sigh.  Do it the hard way
  	strcpy(localInto,base);
  	RemLastComponent(localInto);	// delete everything after last /
  	
  	//if (*localInto && localInto[*localInto]!='/') PCatC(localInto,'/'); // make sure there is a trailing /
  	
  	if (strlen(rel)>=2 && rel[strlen(rel)-1]=='.' && rel[strlen(rel)-2]=='/') 
		rel[ strlen(rel) - 1 ] = '\0';  // delete dot from trailing /.'s
  	while (strlen(rel)>=3 && rel[strlen(rel)-1]=='/' && rel[strlen(rel)-2]=='.' && rel[strlen(rel)-3]=='/') 
		rel[ strlen(rel) - 2 ] = '\0';  // delete ./ from trailing /./'s
  	
  	for (spot=rel;PToken(rel,token,&spot, "/");)
  	{
  		if (strlen(localInto)>1)	// we have a non-zero component somewhere
  		{
  			if (strlen(token)==1 && token[0]=='.')
  			{
  				;	// ignore current directory specification
				if (localInto[strlen(localInto)- 1]!='/') strcat(localInto,"/");	// but make sure path ends in /
			} 				
 			else if (strlen(localInto)>1 && strlen(token)==2 && token[0]==token[1] && token[1]=='.' && !TrailingDotDot(localInto))
  			{
				if (localInto[strlen(localInto)-1]=='/' && strlen(localInto)>1) 
					localInto[ strlen( localInto ) - 1 ] = '\0';
  				RemLastComponent(localInto);
				if (localInto[strlen(localInto)-1]!='/') strcat(localInto,"/");	// make sure path ends in /
  			}
  			else
  			{
				if (localInto[strlen(localInto)-1]!='/') strcat(localInto,"/");
				strcat(localInto,token);
  			}
  		}
  		else
		{
			if (localInto[strlen(localInto)-1]!='/') strcat(localInto,"/");
				strcat(localInto,token);
		}
  	}
  	if (rel[strlen(rel)-1]=='/' && localInto[strlen(localInto)-1]!='/') strcat(localInto,"/");
  }
  strcpy(into,localInto);
  return(into);
}

/************************************************************************
 * TrailingDotDot - does the string end in a component of ".."?
 ************************************************************************/
static BOOL TrailingDotDot(LPSTR string)
{
	char local[ 256 ];
	
	strcpy(local,string);
	if (local[strlen(local)-1]=='/')
		local[ strlen(local)-1] = '\0';	// trim trailing /
	if (strlen(local)<2) return(False);
	if (local[strlen(local)-1]!='.') return(False);
	if (local[strlen(local)-2]!='.') return(False);
	if (strlen(local)==2 || local[strlen(local)-3]=='/') return(True);
	return(False);
}
	
/************************************************************************
 * RemLastComponent - remove the last component of a url, unless that is
 *  a single slash
 ************************************************************************/
static LPSTR RemLastComponent(LPSTR path)
{
	LPSTR spot;
	
	for (spot=path+strlen(path)-1;spot>=path;spot--)
		if (*spot=='/')
		{
			*spot = '\0';
			break;
		}
	return(path);
}

/************************************************************************
 * DeepURLParse - parse a url into lots of components
 ************************************************************************/
typedef enum
{
	urlsNot,			// we don't know what we have
	urlsScheme,			// just saw a scheme
	urlsColon,			// just saw the colon after the scheme
	urlsFirstSlash,		// just saw our first slash
	urlsSecondSlash,	// just saw a second slash
	urlsSite,			// just saw a site component
	urlsQuestion,		// just saw a question mark
	urlsPoundSign,		// just saw a pound sign
	urlsAllDone
} URLStateEnum;

static void DeepURLParse(LPSTR url, DeepURLPtr duh)
{
	LPSTR	spot, end;
	char	token[ 256 ];
	URLStateEnum state=urlsNot;
	BYTE	c;

	RemoveChar(' ',url );
	RemoveChar('\015',url);
	RemoveChar('\t',url);

	end = url+strlen(url);
	
	for (spot=url;c=URLToken(url,token,&spot);)
	{
		switch (state)
		{
			case urlsNot:
				switch (c)
				{
					case '/': state=urlsFirstSlash; break;
					case '?': state=urlsQuestion; duh->isQuery=True; break;
					case '#': state=urlsPoundSign; duh->isFragment=True; break;
					default:
						if ( ('a'<=c && c<='z' || 'A'<=c && c<='Z' ||						// legal scheme chars
					 			'0'<=c && c<='9' || c=='+' || c=='-' || c=='.')		// legal scheme chars
					 		&& spot<end && *spot==':')	// followed by colon
						{
							// we have a scheme
							spot++;	// skip colon
							strcpy(duh->scheme,token);
							state = urlsScheme;
						}
						else
						{
							strcpy(duh->path,token);
							state = urlsSite;
						}
						break;
				}
				break;
			
			case urlsScheme:
				switch(c)
				{
					case '/': state=urlsFirstSlash; break;
					case '?': state=urlsQuestion; duh->isQuery=True; break;
					case '#': state=urlsPoundSign; duh->isFragment=True; break;
					default:
						// must be a path
						strcpy(duh->path,token);
						state = urlsSite;
						break;
				}
				break;
			
			case urlsFirstSlash:
				if (c=='/')
				{
					state = urlsSecondSlash;
					duh->isSite = True;
				}
				else
				{
					strcat(duh->path,"/");
					switch(c)
					{
						case '?': state=urlsQuestion; duh->isQuery=True; break;
						case '#': state=urlsPoundSign; duh->isFragment=True; break;
						default:
							state = urlsSite;
							strcat(duh->path,token);
							break;
					}
				}
				break;
			
			case urlsSecondSlash:
				state = urlsSite;
				switch(c)
				{
					case '/': strcpy(duh->path,token); break;	// empty site
					case '?': state=urlsQuestion; duh->isQuery=True; break;
					case '#': state=urlsPoundSign; duh->isFragment=True; break;
					default: strcpy(duh->site,token); break;
				}
				break;
			
			case urlsSite:
				switch(c)
				{
					case '?': state=urlsQuestion; duh->isQuery=True; break;
					case '#': state=urlsPoundSign; duh->isFragment=True; break;
					default: strcat(duh->path,token); break;
				}
				break;				
			
			case urlsQuestion:
				if (c=='#') {state=urlsPoundSign; duh->isFragment=True;}
				else strcat(duh->query,token);
				break;
			
			case urlsPoundSign:
				strcat(duh->fragment,token);
				break;
			
			default:
//				ASSERT(0);	// we don't belong here!
				break;
		}
	}	
}

/************************************************************************
 * URLToken - tokenize a url
 ************************************************************************/
static BYTE URLToken(LPSTR url,LPSTR token,LPSTR *spot)
{
	LPSTR end = url+strlen(url);
	BYTE c;
	char temp[2] = {0,0};
	
	token[0] = token[1] = 0;
	
	if (*spot<end)
	{
		// pop off first char
		c = **spot;
		++*spot;
		temp[0] = c;
		strcat(token,temp);
		
		if (c=='/' || c=='?' || c==':' || c=='#')
			; // we're done
		else
			for (c=**spot;!(c=='/' || c=='?' || c==':' || c=='#') && *spot<end;c=*++*spot)
			{
				temp[0] = c;
				strcat(token,temp);
			}
	}
	
	return(token[0]);
}


/************************************************************************
 * PToken - grab a token out of a string
 *  Returns pointer to token argument
 *  Saves state in spotP
 ************************************************************************/
static LPSTR PToken(LPSTR string, LPSTR token, LPSTR *spotP, LPSTR delims)
{
	LPSTR	spot;
	LPSTR	end = string + strlen(string) + 1;
	LPSTR	tSpot = token;

	*token = 0;
	if (*spotP>=end) return(NULL);
	for (spot = *spotP; spot<end; spot++)
	{
		if (!strchr(delims,*spot)) *tSpot++ = *spot;
		else break;
	}
	*spotP = spot+1;
	*tSpot = '\0';
	return(token);
}

static void RemoveChar( char it, LPSTR string )
{
	LPSTR p = string;

	while ( *p )
	{
		if ( *p == it )
			strcpy( p, p+1 );
			// repeat check on new *p
		else
			p++;	// on to the next char
	}
}

#ifdef KCM_TEST
main()
{
	char test[ 256 ];

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g:h"),"g:h"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g:h","g:h",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g"),"http://a/b/c/g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g","http://a/b/c/g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","./g"),"http://a/b/c/g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","./g","http://a/b/c/g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g/"),"http://a/b/c/g/"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g/","http://a/b/c/g/",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","/g"),"http://a/g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","/g","http://a/g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","//g"),"http://g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","//g","http://g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","?y"),"http://a/b/c/?y"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","?y","http://a/b/c/?y",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g?y"),"http://a/b/c/g?y"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g?y","http://a/b/c/g?y",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","#s"),"#s"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","#s","#s",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g#s"),"http://a/b/c/g#s"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g#s","http://a/b/c/g#s",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g?y#s"),"http://a/b/c/g?y#s"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g?y#s","http://a/b/c/g?y#s",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q",";x"),"http://a/b/c/;x"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q",";x","http://a/b/c/;x",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g;x"),"http://a/b/c/g;x"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g;x","http://a/b/c/g;x",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g;x?y#s"),"http://a/b/c/g;x?y#s"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g;x?y#s","http://a/b/c/g;x?y#s",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","."),"http://a/b/c/"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q",".","http://a/b/c/",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","./"),"http://a/b/c/"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","./","http://a/b/c/",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q",".."),"http://a/b/"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","..","http://a/b/",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","../"),"http://a/b/"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","../","http://a/b/",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","../g"),"http://a/b/g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","../g","http://a/b/g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","../.."),"http://a/"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","../..","http://a/",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","../../"),"http://a/"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","../../","http://a/",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","../../g"),"http://a/g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","../../g","http://a/g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","../../../g"),"http://a/../g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","../../../g","http://a/../g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","../../../../g"),"http://a/../../g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","../../../../g","http://a/../../g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","/./g"),"http://a/./g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","/./g","http://a/./g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","/../g"),"http://a/../g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","/../g","http://a/../g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g."),"http://a/b/c/g."))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g.","http://a/b/c/g.",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q",".g"),"http://a/b/c/.g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q",".g","http://a/b/c/.g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g.."),"http://a/b/c/g.."))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g..","http://a/b/c/g..",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","..g"),"http://a/b/c/..g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","..g","http://a/b/c/..g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","./../g"),"http://a/b/g"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","./../g","http://a/b/g",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","./g/."),"http://a/b/c/g/"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","./g/.","http://a/b/c/g/",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g/./h"),"http://a/b/c/g/h"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g/./h","http://a/b/c/g/h",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g/../h"),"http://a/b/c/h"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g/../h","http://a/b/c/h",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g;x=1/./y"),"http://a/b/c/g;x=1/y"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g;x=1/./y","http://a/b/c/g;x=1/y",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g;x=1/../y"),"http://a/b/c/y"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g;x=1/../y","http://a/b/c/y",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g?y/./x"),"http://a/b/c/g?y/x"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g?y/./x","http://a/b/c/g?y/x",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g?y/../x"),"http://a/b/c/x"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g?y/../x","http://a/b/c/x",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g#s/./x"),"http://a/b/c/g#s/./x"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g#s/./x","http://a/b/c/g#s/./x",test);

		if (strcmp(URLCombine(test,"http://a/b/c/d;p?q","g#s/../x"),"http://a/b/c/g#s/../x"))
			printf("%s + %s = %s | %s\n","http://a/b/c/d;p?q","g#s/../x","http://a/b/c/g#s/../x",test);

	return 0;
}
#endif //KCM_TEST

