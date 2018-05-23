// ObjectSpec.h -- HTML "OBJECT" Specification
//
// this definition is essentially a one to one mapping of the associated
// attributes of the "OBJECT" tag as of HTML 4.01.
//    --> http://www.w3.org/TR/html401/struct/objects.html
//
// although not an attribute of the OBJECT tag, also inluded here are any
// parameters that may be contained within the tag. parameters are specified
// with the "PARAM" tag, and are optional.

#if !defined( _OBJECTSPEC_H_ )
#define _OBJECTSPEC_H_

struct HtmlObjectParam
{
	char* name;
	char* value;
	char* valuetype;

	HtmlObjectParam* next;

	// frankenstein, et kevorkian
	HtmlObjectParam();
	~HtmlObjectParam();

	// operations
	void SetAttribute( int nCode, char* value );
};


// not really sure what to do about "coreattr" stuff at this point. i'm gonna
// put "id" and "title" here for now. although i think this should be done with
// derivation, i currently don't have a plan.

struct HtmlObjectSpec
{
	// 4.x coreattrs (id, title, etc.)
	char* id;
	char* title;

	// OBJECT attributes
	bool declare;         // declaration only; instantiate later
	char* classid;        // usually MS guid, but can be uri to script, etc.
	char* codebase;       // base uri for classid, data, archive
	char* data;           // reference to object's data
	int type;             // content type for data
	int codetype;         // content type for code
	char* archive;        // space-separated list of uri; preload code, data, etc.
	char* standby;        // message to show while loading
	int height;           // override height
	int width;            // override width
//	char* usemap;         // client-side image map [unused]
	char* name;           // submit as part of form
	int tabindex;         // tab-order position

	HtmlObjectParam* params;   // initialization parameters for object

	// life, death, and taxes
	HtmlObjectSpec();
	~HtmlObjectSpec();

	// operations
	void SetAttribute( int nCode, char* value );
};

#endif   // _OBJECTSPEC_H_
