// ObjectSpec.cpp -- HTML "OBJECT" Specification

// them
#include "stdafx.h"
#include "assert.h"

// us
#include "pghtmdef.h"
#include "objectspec.h"


#include "DebugNewHelpers.h"


///////////////////////////////////////////////////////////////////////////////
// local intelligence

char* dupe_string( char* str );


///////////////////////////////////////////////////////////////////////////////
// HtmlObjectParam

HtmlObjectParam::HtmlObjectParam()
{
	// default construction: everything blank
	memset( this, '\0', sizeof(HtmlObjectSpec) );
}

HtmlObjectParam::~HtmlObjectParam()
{
	delete [] name;
	delete [] value;
	delete [] valuetype;
}

void HtmlObjectParam::SetAttribute( int nCode, char* value )
{
}


///////////////////////////////////////////////////////////////////////////////
// life, death, and taxes

HtmlObjectSpec::HtmlObjectSpec()
{
	// default construction: everything blank
	memset( this, '\0', sizeof(HtmlObjectSpec) );
}

HtmlObjectSpec::~HtmlObjectSpec()
{
	delete [] id;
	delete [] title;

	delete [] classid;
	delete codebase;
	delete data;
	delete archive;
	delete standby;
//	delete usemap;   // unused: client-side image maps
	delete name;

	if ( params ) {
		// TODO: smoke the linked parameter list
	}
}


///////////////////////////////////////////////////////////////////////////////
// public api

// SetAttribute: decodes from Paige definitions [pghtmdef.h] and initializes
// the appropriate field.
void HtmlObjectSpec::SetAttribute( int nCode, char* value )
{
	switch ( nCode )
	{
	case object_id_attribute:
		id = dupe_string( value );
		break;

	case object_title_attribute:
		title = dupe_string( value );
		break;

	case object_classid_attribute:
	{
		char* tmp;
		char uuid[64] = "";

		// check for "clsid:" prefix
		if ( strnicmp( value, "clsid:", 6 ) == 0 )
			tmp = &value[6];
		else
			tmp = value;

		// by convention, clsid strings are in braces, but folks often forget
		// to put them in their html.
		*uuid = '{';

		if ( *tmp == '{' )
			tmp++;

		strcat( uuid, tmp );

		if ( uuid[strlen(uuid) - 1] != '}' )
			strcat( uuid, "}" );

		classid = dupe_string( uuid );
	}
		break;

	case image_width_attribute:
		width = atoi( value );
		break;

	case image_height_attribute:
		height = atoi( value );
		break;

	default:
		assert(0);   // arrgh!
	}
}


///////////////////////////////////////////////////////////////////////////////
// help me

// dupe_string: "new" version of strdup
char* dupe_string( char* str )
{
	char* ps = DEBUG_NEW_NOTHROW char[strlen( str ) + 1];

	if ( ps )
		strcpy( ps, str );

	return ps;
}
