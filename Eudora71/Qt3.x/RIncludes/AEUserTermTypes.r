/*
 	File:		AEUserTermTypes.r
 
 	Contains:	AppleEvents AEUT resource format Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1991-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __AEUSERTERMTYPES_R__
#define __AEUSERTERMTYPES_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

#define kAEUserTerminology 				'aeut'				/*   0x61657574   */
#define kAETerminologyExtension 		'aete'				/*   0x61657465   */
#define kAEScriptingSizeResource 		'scsz'				/*   0x7363737a   */


#include "AEObjects.r"

//	"reserved" needs to be defined to be false for resources
//   but undef'ed for type definitions.  We preserve its state
//   and undef it here.

#undef reserved

/*-------------------aeut ¥ Apple Event User Terminology--------------------------------*/
/*
	aeut_RezTemplateVersion:
		0 - original 							<-- default
		1 - wide version
*/
#ifndef aeut_RezTemplateVersion
	#define aeut_RezTemplateVersion 0
#endif


type 'aeut' {
		hex byte;											/* major version in BCD		*/
		hex byte;											/* minor version in BCD		*/
		integer		Language, english = 0, japanese = 11;	/* language code			*/
		integer		Script, roman = 0;						/* script code				*/
		integer = $$Countof(Suites);
		array Suites {
				pstring;									/* suite name				*/
				pstring;									/* suite description		*/
				align word;									/* alignment				*/
				literal longint;							/* suite ID					*/
				integer;									/* suite level				*/
				integer;									/* suite version			*/
				integer = $$Countof(Events);
				array Events {
					pstring;								/* event name				*/
					pstring;								/* event description		*/
					align word;								/* alignment				*/
					literal longint;						/* event class				*/
					literal longint;						/* event ID					*/
					literal longint		noReply = 'null';	/* reply type				*/
					pstring;								/* reply description		*/
					align word;								/* alignment				*/
#if aeut_RezTemplateVersion == 1
					wide array [1] {
#endif

						boolean	replyRequired,					/* if the reply is  		*/
								replyOptional;					/*   required   			*/
						boolean singleItem,						/* if the reply must be a   */
								listOfItems;					/*   list					*/										
						boolean notEnumerated,					/* if the type is			*/
								enumerated;						/*	 enumerated				*/
						boolean	notTightBindingFunction,		/* if the message has tight	*/
								tightBindingFunction;			/*   binding precedence		*/
						boolean	reserved;						/* these 13 bits are 		*/
						boolean	reserved;						/*   reserved; set them		*/
						boolean	reserved;						/*   to "reserved" 			*/
						boolean	reserved;
					/* the following bits are reserved for localization */
						boolean	reserved;
						boolean	reserved;
						boolean	reserved;
						boolean	reserved;
						boolean	verbEvent,						/* for Japanese; nonVerb    */
								nonVerbEvent;					/*  is used as an expr v.s. */
																/*  v.s. verb is a command  */
							/* the following 3 bits are considered as a part of look up key */
						boolean	reserved;	
						boolean	reserved;
						boolean	reserved;
#if aeut_RezTemplateVersion == 1
					};
#endif
					literal longint		noDirectParam = 'null',
										noParams ='null';	/* direct param type		*/
					pstring;								/* direct param description	*/
					align word;								/* alignment				*/
#if aeut_RezTemplateVersion == 1
					wide array [1] {
#endif

						boolean	directParamRequired,			/* if the direct param 		*/
								directParamOptional;			/*   is required      		*/
						boolean singleItem,						/* if the param must be a   */
								listOfItems;					/*   list					*/										
						boolean notEnumerated,					/* if the type is			*/
								enumerated;						/*	 enumerated				*/
						boolean	doesntChangeState,				/* if the event changes     */
								changesState;					/*   server's state			*/
						boolean	reserved;						/* these 12 bits are 		*/
						boolean	reserved;						/*   reserved; set them		*/
						boolean	reserved;						/*   to "reserved"			*/
						boolean	reserved;
						boolean	reserved;
						boolean	reserved;
						boolean	reserved;
						boolean	reserved;
						boolean	reserved;
						boolean	reserved;
						boolean	reserved;
						boolean	reserved;
#if aeut_RezTemplateVersion == 1
					};
#endif
					integer = $$Countof(OtherParams);
					array OtherParams {
						pstring;							/* parameter name			*/
						align word;							/* alignment				*/
						literal longint;					/* parameter keyword		*/
						literal longint;					/* parameter type			*/
						pstring;							/* parameter description	*/
						align word;							/* alignment				*/
#if aeut_RezTemplateVersion == 1
						wide array [1] {
#endif

							boolean	required,
									optional;					/* if param is optional		*/
							boolean singleItem,					/* if the param must be a   */
									listOfItems;				/*   list					*/										
							boolean notEnumerated,				/* if the type is			*/
									enumerated;					/*	 enumerated				*/
							boolean reserved;
							boolean	reserved;					/* these bits are 			*/
							boolean	reserved;					/*   reserved; set them		*/
							boolean	reserved;					/*   to "reserved" 			*/
							boolean	reserved;
						/* the following bits are reserved for localization */
							boolean	reserved;
							boolean	reserved;
							boolean	reserved;
							boolean	reserved;
							boolean	prepositionParam,			/* for Japanese; labeled 	*/
									labeledParam;				/*  param name comes before */
																/*  the param value		    */
								/* the following 3 bits are considered as a part of look up key */
							boolean	notFeminine,				/* feminine					*/
									feminine;
							boolean	notMasculine,				/* masculine				*/	
									masculine;					
							boolean	singular,
									plural;						/* plural					*/
#if aeut_RezTemplateVersion == 1
						};
#endif
					};
				};
				integer = $$Countof(Classes);
				array Classes {
					pstring;								/* class name				*/
					align word;								/* alignment				*/
					literal longint;						/* class ID					*/
					pstring;								/* class description		*/
					align word;								/* alignment				*/
					integer = $$Countof(Properties);
					array Properties {
						pstring;							/* property name			*/
						align word;							/* alignment				*/
						literal longint;					/* property ID				*/
						literal longint;					/* property class			*/
						pstring;							/* property description		*/
						align word;							/* alignment				*/
#if aeut_RezTemplateVersion == 1
						wide array [1] {
#endif

							boolean	reserved;					/* reserved					*/
							boolean singleItem,					/* if the property must be  */
									listOfItems;				/*   a list					*/										
							boolean notEnumerated,				/* if the type is			*/
									enumerated;					/*	 enumerated				*/
							boolean	readOnly,					/* can only read it			*/
									readWrite;					/* can read or write it		*/
							boolean	reserved;					/* these 12 bits are 		*/
							boolean	reserved;					/*   reserved; set them		*/
							boolean	reserved;					/*   to "reserved" 			*/
							boolean	reserved;
					/* the following bits are reserved for localization */
							boolean	reserved;
							boolean	reserved;
							boolean	reserved;
							boolean	reserved;
							boolean	noApostrophe,	/* This bit is special to the French dialect */
									apostrophe;		/* It indicates that the name begins */
													/* with a vowel */
								/* the following 3 bits are considered as a part of look up key */
											/* what if both feminine and masculine? */
								/* the following 3 bits are considered as a part of look up key */
							boolean	notFeminine,				/* feminine					*/
									feminine;
							boolean	notMasculine,				/* masculine				*/	
									masculine;					
							boolean	singular,
									plural;						/* plural					*/
#if aeut_RezTemplateVersion == 1
						};
#endif

					};
					integer = $$Countof(Elements);
					array Elements {
						literal longint;					/* element class			*/
						integer = $$Countof(KeyForms);
						array KeyForms {					/* list of key forms		*/
							literal longint;				/* key form ID				*/
						};
					};
				};
				integer = $$Countof(ComparisonOps);
				array ComparisonOps {
					pstring;								/* comparison operator name	*/
					align word;								/* alignment				*/
					literal longint;						/* comparison operator ID	*/
					pstring;								/* comparison comment		*/
					align word;								/* alignment				*/
				};
				integer = $$Countof(Enumerations);
				array Enumerations {						/* list of Enumerations		*/
					literal longint;						/* Enumeration ID			*/
					integer = $$Countof(Enumerators);
					array Enumerators {						/* list of Enumerators		*/
						pstring;							/* Enumerator name			*/
						align word;							/* alignment				*/
						literal longint;					/* Enumerator ID			*/
						pstring;							/* Enumerator comment		*/
						align word;							/* alignment				*/
					};
				};
			};
		};


/* Description of the Apple Event Terminology Extension resource		*/
/* This resource is provided by your application						*/

type 'aete' as 'aeut';

type 'scsz' {
	boolean				dontReadExtensionTerms,	readExtensionTerms, dontLaunchToGetTerminology = 0, launchToGetTerminology = 1;
	boolean				findAppBySignature,		dontFindAppBySignature;
	boolean				dontAlwaysSendSubject,	alwaysSendSubject;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	boolean				reserved;
	/* Memory sizes are in bytes. Zero means use default. */
	unsigned longint	minStackSize;
	unsigned longint	preferredStackSize;
	unsigned longint	maxStackSize;
	unsigned longint	minHeapSize;
	unsigned longint	preferredHeapSize;
	unsigned longint	maxHeapSize;	
};

#define reserved false


#endif /* __AEUSERTERMTYPES_R__ */

