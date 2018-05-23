// PgStuffBucket.h
//
// Ah, the ol' "stuff bucket"! This is a place to put things that you want
// keep with each Paige instance. For instance, messages need to keep a
// ptr to the QCMessage object that is unique to each Paige instance assoc-
// iated with a particular message. This is how we glue our C++ implimentation
// to Paige's "C" implimentation.

// BOG: boy, I got a problem with how this "stuff" is turning out, but I don't
// really know what it is; the whole type/subtype thing, I don't know. I think
// that it is time to turn this into a real class, make everything private, and
// add some accessor methods; it's the only way we can be sure that
// type/subtype are always set. This is just turning into more than I had
// originally intended.

#ifndef _PG_STUFF_BUCKET_H_
#define _PG_STUFF_BUCKET_H_

#include "MimeStorage.h"     // for QCMessage


typedef struct _PgStuffBucket
{
    // how this paige instance is being used.
    enum qc_doc_type {
	kUndefinedType = 0,	// a paige object, floating in space
	kDocument,		// paige document ("New Document")
	kMessage,		// mail message (comp, read, preview)
	kStationary,	// stationary editor document
	kAd				// ad window
    };

    // a further refinement of this instance's use
    enum qc_doc_subtype {
	kUndefinedSubtype = 0,	// don't know what this really means
	kCompMessage,		// paige in a composition window
	kReadMessage,		// paige used for reading messages
	kPreviewMessage		// paige in a preview pane
    };

    qc_doc_type kind;
    qc_doc_subtype subkind;
    QCMessage* pMessage;     // if kind == kMessage
    CWnd* pWndOwner;         // allways set
    long hlinkEvent;         // hyperlink id to be processed (PgEmeddedImage.h)
    bool hlinkProcessing;    // protects non-reentrant hlink processing
    bool bOleExport;         // true during copy/paste & drag/drop
    COleDataSource* pDS;     // provided when bOleExport is true
	long cUniqueID;          // counter for generating unique identifiers

    // construction/destruction
    _PgStuffBucket();
    ~_PgStuffBucket(){};

	// unique id generator
	long UniqueID();
	void UniqueIDString( char* pOutBuf );

} PgStuffBucket;

#endif     // _PG_STUFF_BUCKET_H_









