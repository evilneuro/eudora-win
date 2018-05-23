// File:     ldapv2.h    
// Module:   LDAP.DLL
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 04/28/1997 Qualcomm Inc.  All Rights Reserved.
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//
// Description:
// Asynchronous interface to an LDAP server running V2 LDAP.  
// When a function returns immediately to the caller with a result code of
// LDAP_PENDING_REQUEST then the caller will receive a callback indicating
// that something has happened on that request.  Next the caller must make
// result parsing function calls in order to extract query results and errors. 
//
#ifndef ILDAPV2_H
#define ILDAPV2_H

#include <ole2.h>
#include <ldapguids.h>
#include <ldaperr.h>



// [1] Interface Types and Constants
//
// Version 2 LDAP interface to an LDAP server
//
// CALLBACK to user: notification that something has happened with the request indicated 
// by msg_id.  Caller should use the "Extract Result and Error Methods" (see below) to 
// determine what has happened
//
typedef (*LDAPV2_notify )( UINT    handle,          // handle to LDAP response data
						   LPVOID  UserData );

// CALLBACK for ILDAPV2:Open and ILDAPV2:Unbind, since there is no handle returned
// for these two operations
//
typedef (*LDAPV2_control)( int     ResultCode,		// see ldaperr.h
                           LPVOID  UserData );

// The LDAP DLL user does not have to remember the type of operation
// a request was to perform since we remember it for him.
#define LDAPV2_RES_UNKNOWN      0x00    //
#define LDAPV2_RES_BIND			0x01    // authentication
#define LDAPV2_RES_SEARCHENTRY	0x02    // one, of possibly many, LDAP database entries has arrived
#define LDAPV2_RES_SEARCHRESULT	0x03    // marks the end of a Search Request
#define LDAPV2_RES_MODIFY		0x04    // changes one or more of an entry's attribute values, can delete values and attributes
#define LDAPV2_RES_ADD			0x05    // add a new entry to the directory
#define LDAPV2_RES_DELETE		0x06    // remove an entry from the directory
#define LDAPV2_RES_MODRDN		0x07    // change the last component of the name of an entry in the directory
#define LDAPV2_RES_ASSERTION	0x08    // LDAP's Compare Request 

// PARSING FLAGS
// -> used to override the default behavior of some of the PDU parsing functions
#define PARSE_FLAGSNONE         0x00    //
#define PARSE_RETURNSIZE        0x01    // just returns size of buffer required to extract complete value (in bytes)


// SEARCH FILTERS
//  -> on general Tree Searches the user gets a lot of control on how to match an entry
enum    MatchType {
        EqualityMatch,                  // uses both <AttribType, value>
        Substrings,                     // uses both <AttribType, value>
        GreaterOrEqual,                 // uses both <AttribType, value>
        LessOrEqual,                    // uses both <AttribType, value>
        Present,                        // uses only <AttribType>, does it exist in an entry ?
        ApproxMatch,                    // uses both <AttribType, value>
};

// An array of Filters can be specified to restrict what is returned on a search operation
// -> NOTE: that not all fields are used for all filter types
typedef struct {
	    // Fields used for all types of filters
        MatchType type;                 // how to perform a test with <AttribType, value>
        char*     AttribType;           // AttributeType string
        LPVOID    value;                // either AttributeValue or Substring Filter LDAPstring
        DWORD     ValueLength;          // byte count in value
                                        // The default value is ANDED for two or more array entries
        BOOL      Ored;                 // only need to be set in first array entry---OR all array entries are ANDED
										//
		// Fields only used if type == Substrings
		// -> type of substring match desired (note: its legal to set one, two, or all three of these)
		BOOL      leading;				// LDAP's "initial"
		BOOL      any;					// LDAP's "any"
		BOOL      trailing;				// LDAP's "final"
} Filter_t; 

              
// ENTRY MODIFICATION
// -> to modify (add) an entry in the DIT we must send the LDAP server a list of attributes with their value(s) to change (add).
//    DON'T change teh ModifyType integer values, since they are used directly in the ASN.1 PDU generated!
enum    ModifyType {
	    Add     = 0,					// add values to the listed attribute, creating the attribute if necessary
		Delete  = 1,					// delete  values listed from the given attribute
		Replace = 2,					// replace existing values of the given attribute with th enew values listed, creating the attribute if necessary
};

typedef struct {
        LPVOID     value_data;			// can be binary or just a string; in either case its NOT zero terminated !
		DWORD      length;				// number of meaningfull bytes in the value buffer
} Value_t;

typedef struct {
	    ModifyType operation;			// In AddEntry this values is assumed to be Add
	    char*      type_name;			// type name of attribute
		Value_t*   values;				// an array of values for the attribute
		UINT       value_count;			// number of Value_t structures pointed to be values
} Attribute_t;




// [2] Protocol Interface to LDAP Version 2.  
//     One ILDAPV2 object is used per LDAP session.
//
typedef class ILDAPV2* LPILDAPV2;
class ILDAPV2 : public IUnknown 
{
      public:
      // [A] Interface types     
      //  -> caller controls how searches handle alias entries
      //  -> NOTE: don't change these numbers they are as appear in RFC 1777
	  enum    DefAlias {
              NeverDerefAliases   = 0,  // most restricted type of tree search
              DerefInSearching    = 1,  // most restricted tyoe of search to find the main search starting point
			  DerefFindingBaseObj = 2,  // widest possible search to find the main search starting point 
			  DerefAlways         = 3,  // (USE AS DEFAULT) used to get the widest possible search
	  };

	  //  -> caller can set limits on how a search is to be conducted
	  typedef struct {
              UINT      SizeLimit;      // max number of entries to return (0 == no limt)
			  UINT      TimeLimit;      // max time in seconds to spend  (0 == no limit)
			  BOOL      AttrsOnly;      // only return the attribute names and not its values
			  DefAlias  Deref;          //
	  } Limits_t;



	  // [B] Session Management Methods
      //  -> returns Connection wide error notifications lpException routine
      virtual HRESULT __stdcall SetExceptionCallBack
          ( LDAPV2_control lpException,         // in:  stays set until reset
            LPVOID         UserData ) = 0;      // in:

	  //  -> open TCP/IP connection to the "hostname" LDAP server
	  virtual HRESULT __stdcall Connect
		  ( LDAPV2_control lpCompletion,	    // in:  notify caller of Open method result
			LPVOID         UserData  ) = 0;		// in:  callers' context to return in notify call
	
	  //  -> terminate session with LDAP server, don't do Unbind or Abandon, just kill the session hard
	  virtual HRESULT __stdcall KillSession() = 0;
	
	  //  -> authenticate to the LDAP server with in the clear password or as anonymous
	  virtual HRESULT __stdcall BindSimple
		  ( char*          dn,					// in:  Distinguished Name to authenticate as (or NULL for anonymous binds)
		    char*          password,			// in:  in clear text (or NULL for unauthenticated or anonymous binds)
			LDAPV2_notify  lpCompletion,		// in:
			LPVOID         UserData,			// in:
			UINT*          handle    ) = 0;		// out: Handle or Identifier of BindSimple request
	  
	  //  -> authenticate to the LDAP server with Kerberous V4 protecting password
	  virtual HRESULT __stdcall BindKerberosV4
		  ( char*          dn,					// in:  (cannot be NULL)
		    char*          ticket,				// in:  Kerberos ticket (cannot be NULL)
			LDAPV2_notify  lpCompletion,		// in:
			LPVOID         UserData,			// in:
			UINT*          handle    ) = 0;		// out:		
      
	  //  -> close LDAP session and TCP/IP connection
	  virtual HRESULT __stdcall Unbind							
		  ( LDAPV2_control lpCompletion,		// in:
			LPVOID         UserData  ) = 0;	    // in:

	  // -> (synchronous) cancel request identified by "handle"
	  virtual HRESULT __stdcall Abandon
          ( UINT           handle    ) = 0;     // in:



      // [C] Modify Management Methods 
	  //  These methods will likely require authentication that allowed modify access.
	  //
      //  -> only leaf objects (in the DIB) may be deleted with this operation
	  virtual HRESULT __stdcall DeleteEntry
		  ( char*          dn,					// in:
			LDAPV2_notify  lpCompletion,	    // in:
			LPVOID         UserData,			// in:
		    UINT*          handle    ) = 0;		// out:

	  // -> add a new entry in the DIB at the locatoon specified by "dn" with values specified by "Attributes"
	  //    Note: that all components of the dn except for the last RDN component must exist for the add to succeed.
	  virtual HRESULT __stdcall AddEntry
		  ( char*          dn,					// in:
            Attribute_t*   Attributes,			// in:  ptr to an array of Attribute_t entries
            UINT           AttribCount,			// in:  number of array entries in Attributes
			LDAPV2_notify  lpCompletion,		// in:
			LPVOID         UserData,			// in:
		    UINT*          handle    ) = 0;		// out:

	  // -> modify a new entry in the DIB at the locatoon specified by "dn" with values specified by "Attributes"
	  //    Note: that if an attribute will be delete if an attribute is specified without any values with a "Delete" operation, or
	  //    if all of its values are marked for deletion.
	  virtual HRESULT __stdcall ModifyEntry
		  ( char*          dn,					// in:  the server will not perform any alias dereferencing in determining the object to be modified
            Attribute_t*   Attributes,			// in:  ptr to an array of Attribute_t entries
            UINT           AttribCount,			// in:  number of array entries in Attributes
			LDAPV2_notify  lpCompletion,		// in:
			LPVOID         UserData,			// in:
		    UINT*          handle    ) = 0;		// out:

	  // -> change the last component of the name of an entry in the directory
	  virtual HRESULT __stdcall ModifyRDN
		  ( char*          dn,					// in:  
		    char*          NewRdn,				// in: change the last component of the dn to this value
			BOOL		   DeleteOldRdn,		// in: old RDN attribute values retained in entry or deleted from entry ?
			LDAPV2_notify  lpCompletion,		// in:
			LPVOID         UserData,			// in:
		    UINT*          handle    ) = 0;		// out:



	  // [D] Search Management Methods
	  //  -> return all matches by searching all entries in the subtree rooted at "dn"
	  virtual HRESULT __stdcall TreeSearch
		  ( char*          dn,					// in:
		    Limits_t*      options,				// in:  settings to restrict/control the search procedure on the LDAP server
            Filter_t*      Filter,				// in:  ptr to an array of Filter_t entries
            UINT           FilterCount,			// in:  number of array entries in Filter
			LDAPV2_notify  lpCompletion,		// in:	
			LPVOID         UserData,			// in:	
			UINT*          handle    ) = 0;		// out: Handle or Identifier of TreeSearch request
	  
	  //  -> return the contents of the entry pointed to by "dn"
	  virtual HRESULT __stdcall ReadEntry
		  ( char*          dn,					// in:
		    Limits_t*      options,				// in:
			LDAPV2_notify  lpCompletion,		// in:
			LPVOID         UserData,			// in:
			UINT*          handle    ) = 0;		// out:
	  
	  //  -> return all entries that are children of "dn"
	  virtual HRESULT __stdcall ListChildren
		  ( char*          dn,					// in:
		    Limits_t*      options,				// in:
			LDAPV2_notify  lpCompletion,		// in:
			LPVOID         UserData,			// in:
			UINT*          handle    ) = 0;		// out:



	  // [E] Extract Result and Error Methods
	  // All extraction methods are synchronous, and MUST be called in PROPER order
	  // Example of parsing out Search entry results:
	  // GetEntryName
	  // while( S_OK == EnumAttributes()) 
	  // {
      //     -> attribute type available
	  //     more = FALSE;
	  //     while( S_OK == EnumAttribValue(..., buffer, ... &more)) 
	  //     {
	  //         -> attribute value larger than buffer used in EnumAttribValue ?
	  //         for( ;more; ) ContinueValueRead(..., buffer, ... &more);
	  //     }
	  // }
	  // FreeResult or FreeData
      //
      // Example of parsing out result of all other requests:
      // GetResultCode
      // FreeResult
      
      // Return the success or error information stored in the returned entry
      // Caller MUST not use this function on LDAPV2_RES_SEARCHENTRY responses.
      // Return Values:
      // E_FAIL                -- no request with matching handle or
      //                          serious parsing failure, cannot decode PDU
      // E_PENDING             -- response has not yet been received
      // TYPE_E_BUFFERTOOSMALL -- szMatchedDn or szServerNotice string too big to be copied into parameter
      //                       -- length parameter contains size of buffer required
      // S_OK                  -- dn parameter has real value
      //
      virtual HRESULT __stdcall GetResultCode
		  ( UINT           handle,			      // in:
            int*           ResultCode,		      // out: see ldaperr.h
  		    UINT*          OpCode,			      // out: (e.g. LDAPV2_RES_BIND)
			char*          szMatchedDn,	          // out: on ResultCode LDAP error: substring of DN that was found or NULL
			DWORD*         DnLength,              // in:  indicates size of szMatchedDn buffer; out: indicates size of string copied into szMatchedDn
			char*          szServerNotice,        // out: on ResultCode LDAP error: diagnostic message from server to user or NULL
			DWORD*         NoticeLength ) = 0;    // in:  indicates size of szServerNotice buffer; out: indicates size of string copied into szServerNotice


      // Indicates that the caller is done "looking at" the current entry, all allocated
      // resources are released
      // Return Values:
      // E_FAIL                -- no request with matching handle 
      // S_OK                  -- All allocated resources successfully deleted
      //
      virtual HRESULT __stdcall FreeResult
		  ( UINT           handle    ) = 0;     // in:

      // Indicates that the caller is done "looking at" the current PDU, all resources
      // allocated to holding received data are released.
      // Return Values:
      // E_FAIL                -- no request with matching handle 
      // S_OK                  -- All allocated resources successfully deleted
      //
      virtual HRESULT __stdcall FreeData
		  ( UINT           handle    ) = 0;     // in:

      // Extract the "dn" associated with the "current" returned entry
      // Return Values:
      // E_FAIL                -- no request with matching handle or
      //                          serious parsing failure, cannot decode PDU
      // E_PENDING             -- response has not yet been received
      // TYPE_E_BUFFERTOOSMALL -- Attribute string too big to be copied into AttribType parameter
      //                       -- length parameter contains size of buffer required
      // TYPE_E_WRONGTYPEKIND  -- caller should call GetResultCode not this function
      // S_OK                  -- dn parameter has real value
      //
	  virtual HRESULT __stdcall GetEntryName
		  ( UINT           handle,				// in:
		    char*          dn,  				// out:
            DWORD*         length    ) = 0;     // in:    indicates size of dn buffer; out: indicates size of string copied into dn

      // Enumerate all attributes in the "current" returned entry
      // function return value indicates when hit end of list (can then restart)        
      // Return Values:
      // E_FAIL                -- no request with matching handle or
      //                          serious parsing failure, cannot decode PDU
      // E_PENDING             -- response has not yet been received
      // TYPE_E_BUFFERTOOSMALL -- Attribute string too big to be copied into AttribType parameter
      //                       -- length parameter contains total size of buffer required
      // S_OK                  -- AttribType has real value
      // S_FALSE with          -- no more attributes in PDU, 
      //   AttribType[0] == 0  -- call again will reset and start at beginnin
      //   && length == 0
      //
	  virtual HRESULT __stdcall EnumAttributes
		  ( UINT           handle,				// in:
		    UINT           flags,				// in:    currently unused (assumes PARSING_RETURNSIZE)
			char*          AttribType,			// out:
			DWORD*         length    ) = 0;		// in:    indicates size of AttributeType buffer; out: indicates size of string copied into AttributeType 

      // Enumerate all values of the "current" attribute
      // function return value indicates when hit end of list (can then restart)
      // Note that if the "value" input buffer is too small to hold the entire size
      // of the next value, then the "more" paramter is set to TRUE,  The caller
      // should invoke the ContinueValueRead member until no more value data is left.
      // Return Values:
      // E_FAIL                -- no request with matching handle or
      //                          serious parsing failure, cannot decode PDU
      // E_PENDING             -- response has not yet been received
      // TYPE_E_BUFFERTOOSMALL -- ONLY RETURNED if flags parameter is set to PARSE_RETURNSIZE
	  //					   -- on input:  ignores the value passed in the length parameter
      //                       -- on output: length parameter contains total size of buffer required
      // S_OK                  -- the "value" bufer has been filled
      // S_FALSE with          -- no more values for the current attribute
      //   length == 0         -- call again will reset and start at begining
      //
	  virtual HRESULT __stdcall EnumAttribValues
		  ( UINT           handle,				// in:
            UINT		   flags,				// in:    see PARSING FLAGS above
		    LPVOID         value,				// out:   can be binary or characters
			DWORD*         length,   			// in:    indicates size of value buffer; out: indicates size of string copied into value
            BOOL*          more      ) = 0;     // out:   if TRUE then user should call ContinueValueRead member      

      // For large data values, or at least larger than the caller's buffer, this function
      // can be called multiple times to extract out a chunk of an attribute's value.  The
      // first invocation of this function can occur only after a call to the EnumAttribValues
      // member.
      // Return Values:
      // E_FAIL                -- no request with matching handle
      // E_PENDING             -- response has not yet been received
      // S_OK                  -- value buffer has been filled
      // S_FALSE with          -- no more data for the current value
      //   length == 0         
      //
      virtual HRESULT __stdcall ContinueValueRead
          ( UINT           handle,				// in:
		    LPVOID         value,				// out:   can be binary or characters
			DWORD*         length,				// in:    indicates size of value buffer; out: indicates size of data copied into value 
	        BOOL*          more      ) = 0;     // out:   if TRUE then user should call ContinueValueRead member      



	  // [G] Misc Methods
      // Allows the caller to compare an assertion with an entry in the LDAP database.
      // Return Values:
      // E_FAIL                -- no request with matching handle
	  // E_OUTOFMEMORY         -- request failed; ran out of malloced memory
      // S_OK                  -- value buffer has been filled
      //
      virtual HRESULT __stdcall TestAssertion
          ( char*          dn,					// in: the entry to be tested
            char*          AttribType,			// in: Does this attribute
            LPVOID         AttribValue,			// in:    have this value in the LDAP database ?
            DWORD          ValueLength,			// in: byte count in value
		    LDAPV2_notify  lpCompletion,		// in:
		    LPVOID         UserData,			// in:
		    UINT*          handle    ) = 0;		// out:



      // [F] Extended Class Management Methods
	  virtual BOOL Initialize( LPSTR szHost, int Port, char* TraceFileName, BOOL OverWrite = FALSE ) = 0;    
      virtual void SetLog( LPVOID LogTo ) = 0;										 
};


#endif

