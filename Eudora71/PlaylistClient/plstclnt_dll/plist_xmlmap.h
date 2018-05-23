// Include this file without PLIST_XML_DEC_MAP defined if you just
// need to use the enums

// Include it with PLIST_XML_DEC_MAP defined to actually declare the map

// No chance the table can get out of whack, that's why I'm doing it this way

#ifndef PLIST_XML_DEC_MAP
typedef enum {
#define PLIST_XML_VAL(x)
#else
static ElementMap gXMLTags[] = {
#define PLIST_XML_VAL(x) ,x
#endif
    kXMLclientUpdateResponse	PLIST_XML_VAL("clientUpdateResponse"),
    kXMLplaylist                PLIST_XML_VAL("playlist"),
	kXMLwidth                   PLIST_XML_VAL("width"),
	kXMLheight                  PLIST_XML_VAL("height"),
    kXMLentry                   PLIST_XML_VAL("entry"),
	kXMLID                 		PLIST_XML_VAL("id"),
	kXMLtitle                   PLIST_XML_VAL("title"),
    kXMLsrc                     PLIST_XML_VAL("src"),
	kXMLhref                    PLIST_XML_VAL("url"),
    kXMLshowFor                 PLIST_XML_VAL("showFor"),
	kXMLshowForMax              PLIST_XML_VAL("showForMax"),
	kXMLdayMax                  PLIST_XML_VAL("dayMax"),
	kXMLblackBefore             PLIST_XML_VAL("blackBefore"),
	kXMLblackAfter              PLIST_XML_VAL("blackAfter"),
	kXMLstartDT                 PLIST_XML_VAL("startDT"),
	kXMLendDT                   PLIST_XML_VAL("endDT"),
	kXMLclientUpdate            PLIST_XML_VAL("clientUpdate"),
	kXMLclientInfo            	PLIST_XML_VAL("clientInfo"),
	kXMLuserAgent            	PLIST_XML_VAL("userAgent"),
	kXMLfaceTime            	PLIST_XML_VAL("faceTime"),
	kXMLfaceTimeUsedToday      	PLIST_XML_VAL("faceTimeUsedToday"),
	kXMLpastry			      	PLIST_XML_VAL("pastry"),
	kXMLfaceTimeLeft	      	PLIST_XML_VAL("faceTimeLeft"),
	kXMLplaylistVersion         PLIST_XML_VAL("playListVersion"),
	kXMLisRunout				PLIST_XML_VAL("isRunout"),
	kXMLisSponsor				PLIST_XML_VAL("isSponsor"),
	kXMLisButton				PLIST_XML_VAL("isButton"),
	kXMLdeletedByUser			PLIST_XML_VAL("deleted"),
	kXMLactive					PLIST_XML_VAL("active"),
	kXMLclientMode				PLIST_XML_VAL("clientMode"),
	kXMLprofile				PLIST_XML_VAL("profile"),
	kXMLrerunInterval			PLIST_XML_VAL("rerunInterval"),
	kXMLscreen					PLIST_XML_VAL("screen"),
	kXMLdistributor				PLIST_XML_VAL("distributorID"),
	kXMLlocale                  PLIST_XML_VAL("locale"),

	kXMLfault                   PLIST_XML_VAL("fault"),

#ifndef PLIST_XML_DEC_MAP
    kXMLid_none                			         // always last!
} PlistXMLEnum;
#else
    kXMLid_none,                "biteMe"         // always last!
};
#endif

#undef PLIST_XML_VAL
