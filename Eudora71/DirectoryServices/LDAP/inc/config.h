// LDAP Directory Services 
//
// File:     config.h
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 06/10/1997 Qualcomm Inc.  All Rights Reserved.
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
//
//
#ifndef CONFIG_H
#define CONFIG_H

#include "lviewex.h"
#include "log.h"



// [A] Interface Types and Constants
#define UTIL_BUF_SIZE  2000
#define LDAP_SECTION   "LDAP--%ld"


// -> configuration data on defined LDAP servers
typedef struct {
    char* UserSrvName;        // String that the user wants to use to refer
                              // to this server
    char* DomainName;         //
    UINT  Port;               //
    char* SearchBase;         // An LDAPDN that is the base object entry
                              // relative to which the search is performed.
    char* SectionName;        // "LDAP--%ld" is the string used to define
                              // where the attribute MAP is for this server
    char* LogFile;            //
    BOOL  OverWrite;          //
    DWORD LogFlags;           // used to enable/disable certain log statements
    LPSTR szUID;              // unique ID just for this configuration item  
    BOOL  LogonRequired;      // for LDAP servers that require plantext logons
	BOOL  SSLRequired;		  // for LDAP servers that require SSL logons
    char* AccountName;        //
    char* AccountPassword;    //
    DWORD ServerTimeOut;      // {0..300} where 0 means no time limit, and
                              // all other values are time out in seconds
    DWORD ServerHitLimit;     // {0..LDAP maxInt} where 0 means no limit
    char* SearchFilter;       // used to form a term from a word
    char* CNSearchFilter;     // used to form a term from the whole query
    BOOL  ReturnAll;          // results are not filtered
} Server_t;					   


// -> configuration data on attribute names per server   
typedef struct {                                        
    char*     x500Name;       // ASCII LDAP version of X.500 OIDs
    DS_FIELD  InternalName;   // built in names we understand
    char*     UserName;       // user defined an attribute name that we
                              // don't have built in
    DS_TYPE   DataType;       // binary or ASCII string ?
} NameMap_t;                                            

// -> LogFlags Values (these values are used as a BITMASK)
#define  LOG_PROTOCOL  0x01
#define  LOG_CONFIG    0x02


// [B] All the details on how to obtain and maintain LDAP configuration data is
//     hidden in this object.
//
typedef class Config* LPCONFIG;
class Config
{
      public:
      // [A] Class Management Methods
      Config( IDirConfig* ConfigIO, LPSTR szServerName, LPSTR szSectionName );
	  virtual ~Config();
	  virtual void     SetLog( LPLOG LogTo );

	  // [B] Manage the loading and storing of configuration data
	  //  -> mapping tables for X.500 attribute names to user friendly names (e.g., organizationPerson to Name)
	  virtual void     LoadServerList();
      virtual void     LoadServerInfo();
      virtual void     StoreServerList();
	  virtual void     StoreLogInfo();
      virtual void     StoreNetInfo();
      virtual void     StoreOptionInfo();
      virtual void     StoreAttribMap();
      virtual void     DumpNameMap();
      virtual void     DumpServers();

    BOOL GetReturnAll();
    void SetReturnAll( BOOL );

	  // -> access loaded configuration data
      virtual BOOL     MakePropPages( HPROPSHEETPAGE* paHPropPages );		   // Use when object is in the mode of accessing config data of just one server
	  virtual void     GetServerProperties( Server_t** properties );           // <-- don't free returned pointer
	  virtual void     FirstAttribute( NameMap_t** attribute );                // <-- don't free returned pointer
	  virtual void     NextAttribute(  NameMap_t** attribute );                // <-- don't free returned pointer
      virtual BOOL     IsLogEnabled( DWORD LogType );                          // <-- use one of the LogFlags Constants defined above
      virtual BOOL     MapAttribName( LPSTR szName, DS_FIELD* internal_name, DS_TYPE* data_type, LPSTR* szUserDefined );    // <-- don't free returned pointer in szUserDefined
      virtual char*    KnownFields( DS_FIELD KnownField );                     // <-- don't free returned pointer
																			   //
      virtual void     GetFirstServer( Server_t** properties );                // Use when object is in the mode of reading in and listing
      virtual void     GetNextServer(  Server_t** properties );                // all configured server names
      virtual void     ReadPreDefinedConfig();                                 // load in pre-defined configuration data

      // -> modify configuration data
      virtual BOOL     MakeNewServer( char* szBuffer );
      virtual void     ChangePort( UINT Port );
      virtual void     ChangeDomainName(  char* szDomain );
      virtual void     ChangeUserSrvName( char* szUserSrvName );
      virtual void     ChangeLogInfo( char* LogPath, BOOL OverWrite, DWORD flags );
      virtual BOOL     ChangeAttribMap( HWND hwndList );
      virtual void     ChangeOptions( char* SearchBase, const char*,
				      const char*, DWORD ServerTimeOut,
				      DWORD ServerHitLimit,
				      BOOL HitLimit_Valid );

      virtual void     ChangeLogon( BOOL LogonEnabled, char* szAccount, char* szPassword, BOOL SSLEnabled );
      virtual BOOL     DeleteAllConfig();

      // ->
      CListViewEx*     m_pListViewEx;


	  protected:
	  // [C] Utility and parsing methods
      virtual BOOL     NewServerList( LPSTR szServerName, LPSTR szSectionName, LPSTR szSectionUID );
      virtual void     FreeAttributeMap();
      virtual void     DeleteStoredMap( LPSTR szSection );
      virtual void     FreeServerList();
      virtual BOOL     CopyConfigStr( LPSTR* pString, LPSTR szKey, LPSTR szdefault );
      virtual int      TokenEnd();
      virtual DS_FIELD IntoInternalName();
	  virtual char*    IntoAttribString( DS_FIELD ConfiguredName );
      virtual DS_TYPE  IntoInternalTypeName();
      virtual char*    IntoTypeString( DS_TYPE ConfiguredType );

      // -> load in pre-defined configuration data
      virtual BOOL     StorePreDefinedConfig( char szFileName[MAX_PATH] );
      virtual void     StoreAttribTable( char* szFileName, char* szReadSection, char* szWriteSection );
      virtual void     StoreFixedConfig( char* szFileName, char* szReadSection, char* szWriteSection );


      // [D] Internal Data & Type Members
	  LPLOG       m_LogObj;                  // For debug and in the field monitoring
      IDirConfig* m_ConfigIO;                // For read/write of configuration data (don't delete, being shared)
											 //
	  NameMap_t*  m_AttribMap;               // Convert X.500 attribute names to something the user will understand
	  DWORD       m_MapSize;				 //
	  DWORD       m_EnumAttrib;              //
											 //
	  // -> list of configured servers       //
	  Server_t*   m_ServerList;				 // Array of pointers to character strings holding server domain names
	  DWORD       m_ServerSize;              // Number of entries in the Server list
      DWORD       m_NextServer;              // Used to enumerate the list of configured servers
								             //
	  // -> dirty bits telling us what needs to be written out
	  BOOL        m_MapModified;             // indicates whether we need to save the current table to persistant storage
	  BOOL        m_NetModified;             // indicates whether we need to save the current table to persistant storage
      BOOL        m_OptionsModified;         // Have the Search Options been changed ?
      BOOL        m_LogModified;             // Have the Log    Options been changed ?
										     //
	  // -> used to read/write configuration info
	  char        m_UtilBuf[UTIL_BUF_SIZE];  // Utility buffer to read into/write out of, build strings etc.
	  DWORD       m_TotalBytes;				 // Number of meaningfull bytes in m_UtilBuf
	  DWORD       m_CurrentByte;			 // Index into m_UtilBuf indicating next byte to be parsed 
};


#endif











