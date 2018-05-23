// plist_parser.cpp
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

#include "stdafx.h"

#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

#include "plist_utils.h"
#include "plist_parser.h"
#include "plist_debug.h"
#include "plist_cinfo.h"

#include "DebugNewHelpers.h"

//	Disable "assignment within conditional expression" warning
#pragma warning(disable : 4706)

////////////////////////////////////////////////////////////////////////////////


static ElementMap g_plistElements[] = {
    clientUpdateResponse,   "clientUpdateResponse",
    playlist,               "playlist",
    playlistID,             "playlistID",
	width,                  "width",
	height,                 "height",
    entry,                  "entry",
	entryID,                "adID",          // TODO: this should be "entryID"
	title,                  "title",
    src,                    "src",
	href,                   "url",           // this should be "href"
    showFor,                "showFor",
	showForMax,             "showForMax",
	dayMax,                 "dayMax",
	blackBefore,            "blackBefore",
	blackAfter,             "blackAfter",
	startDT,                "startDT",
	endDT,                  "endDT",
	pastry,              	"pastry",
	reqInterval,            "reqInterval",
	historyLength,          "histLength",
	scheduleAlgorithm,      "scheduleAlgorithm",
	faceTimeQuota,      	"faceTimeQuota",
	flush,                  "flush",
	fault,                  "fault",
	reset,                  "reset",
	rerunInterval,          "rerunInterval",
	profile,                "profile",
	clientMode,             "clientMode",
	nag,                    "nag",

    id_none,                "biteMe"         // always last!
};


///////////////////////////////////////////////////////////////////////////////
// Public Iface

// ImportPlaylist -- takes a ptr to a PlaylistHeader, and fills it out. right
// now, we're using "nextNode" to link the list of entries---kind of a hack.
// "CommitPlaylist" consumes the list.

int PlaylistParser::ImportPlaylist( char* xml, PlistCInfoPtr pcip )
{
    int ret = -1;
	int len = strlen( xml );

	OutputDebugStrings("imp->",xml);

    m_pPlaylistHeader = NULL;
    m_pEntries = NULL;

    int prsRet = Parse( xml, len, true );
    
    // Update the clientinfo
    // TODO: what about partial clientinfos?
    if (m_pciValid) PlistCInfoSet(pcip,m_pcip,m_pciValid);

    if ( !prsRet && !m_error ) {
    	if ( m_pPlaylistHeader ) {
        	set_nextnode( m_pPlaylistHeader, m_pEntries );
        	ret = CommitPlaylist( &m_pPlaylistHeader );
        }
        else
        	ret = -2;
    }

// TODO: handle case where commit fails. is it really a good idea for
// "CommitPlaylist" to consume the list? Hmmm. now that i think about
// it, commit will only fail on i/o, so we'll have to just flag the
// playlist as still needing to be retrieved and bail.

    m_pPlaylistHeader = NULL;
    m_pEntries = NULL;
    return ret;
}

// same as above, but can be called repeatedly for incremental import. on "done",
// the return value is a db handle---otherwise it's an err code, with zero
// meaning "no error".
//
// TODO: this is an ugly hack---need to come up with something better. right now,
// it can not be called for multiple imports with the same instance.

int PlaylistParser::ImportPlaylist( char* xml, unsigned len, bool done )
{
    int ret = -1;
    int prsRet = Parse( xml, len, done );

	OutputDebugStrings("imp->",xml);

	if ( done ) {
		if ( !prsRet && !m_error && m_pPlaylistHeader ) {
			set_nextnode( m_pPlaylistHeader, m_pEntries );
			ret = CommitPlaylist( &m_pPlaylistHeader );
		}
	}

    return (done ? ret : (prsRet ? -1 : 0));
}


///////////////////////////////////////////////////////////////////////////////
// Impl

bool PlaylistParser::initElementMap( ElementMap** map )
{
    *map = g_plistElements;
    return true;
}


int PlaylistParser::startElement( int id, const char* /*name*/, const char** atts )
{
	switch ( id ) {
		case playlist:
		{
			PlaylistHeader* pH = DEBUG_NEW_NOTHROW PlaylistHeader;

			if ( pH ) {
				memset( pH, '\0', sizeof(PlaylistHeader) );
				set_nodetype( pH, kPlaylistHeader );
				set_nodesize( pH, sizeof(PlaylistHeader) );
				m_pPlaylistHeader = pH;
			}
		}
			break;
		case entry:
		{
			Node* pE = m_pEntries;

			if ( pE ) {
				for ( ; pE->nextNode != 0; pE = nextnode(pE) )
					;

				m_pCurEntry = DEBUG_NEW_NOTHROW Entry;
				set_nextnode( pE, m_pCurEntry );
			}
			else {
				m_pCurEntry = m_pEntries = DEBUG_NEW_NOTHROW Entry;
			}

			if ( m_pCurEntry ) {
				memset( m_pCurEntry, '\0', sizeof(Entry) );
				set_nodetype( m_pCurEntry, kEntry );
				set_nodesize( m_pCurEntry, sizeof(Entry) );

				// read attributes
				for (const char **a=atts;*a;a++)
				{
					if (!strcmp(*a,"isRunout"))
						m_pCurEntry->type = kRunout;
					else if (!strcmp(*a,"isButton"))
						m_pCurEntry->type = kButton;
					else if (!strcmp(*a,"isSponsor"))
						m_pCurEntry->type = kSponsor;
					if (!*++a) break;	// skip value for now
				}

				// we keep an entries width and height, but this is currently
				// specified on a per-playlist basis.
				if (m_pcip)
				{
					m_pCurEntry->width = m_pcip->adWindowWidth;
					m_pCurEntry->height = m_pcip->adWindowHeight;
				}
			}
		}
			break;
		case pastry:
		{
			const char **a;

			if (m_pcip) for (a=atts;*a;a++)
			{
				strcat(m_pcip->pastry,*a);
				strcat(m_pcip->pastry,"=\"");
				a++;
				if (*a) strcat(m_pcip->pastry,*a);
				strcat(m_pcip->pastry,"\" ");
			}
			m_pciValid |= PLCIpastryValid;
		}
			break;
		case reset:
		case flush:
		{
			if ( m_pcip ) {
				int theCmd;

				if ( id == flush )
					theCmd = kCmdFlush;
				else
					theCmd = kCmdReset;

				PlistCmd* pcmd = DEBUG_NEW_NOTHROW PlistCmd( theCmd, atts );

				if ( pcmd ) {
					if ( m_pcip->commands ) {
						PlistCmd* ptrv = m_pcip->commands;
						for ( ; ptrv->next; ptrv = ptrv->next )
							;

						ptrv->next = pcmd;
					}
					else
						m_pcip->commands = pcmd;

					m_pciValid |= PLCIcommandsValid;
				}
			}
		}
			break;
		case fault:
		{
			m_error = -1;
		}
			break;
		case src:
		{
			const char** a;

			for ( a = atts; *a; a++ )
				if ( stricmp( "checksum", *a ) == 0 )
					strcpy( m_pCurEntry->checksum, *(++a) );
		}
			break;
		case nag:
		{
			if ( m_pcip ) {
				const char** a;

				for ( a = atts; *a; a++ ) {
					if ( stricmp( "level", *a ) == 0 )
						if ( m_pcip->nagInfo = DEBUG_NEW_NOTHROW int[2] )
							m_pcip->nagInfo[0] = atoi( *(++a) );
				}
			}
		}
			break;
	}

    return 0;
}

int PlaylistParser::endElement( int id, const char* name )
{
    const char* tagName = name;

    switch ( id ) {
    case clientUpdateResponse:
		tagName = "clientUpdateResponse";
		break;
    case playlist:
		tagName = "playlist";
		break;
    case playlistID:
		tagName = "playlistID";
        strcpy( m_pPlaylistHeader->playlistID, m_accum );
		break;
	case width:
		if (m_pcip)
		{
			m_pcip->adWindowWidth = atoi(m_accum);
			m_pciValid |= PLCIadWindowWidthValid;
		}
		break;
	case height:
		tagName = "height";
		if (m_pcip)
		{
			m_pcip->adWindowHeight = atoi(m_accum);
			m_pciValid |= PLCIadWindowHeightValid;
		}
		break;
    case entry:
		tagName = "entry";
		break;
	case entryID:
		tagName = "entryID";
        strcpy( m_pCurEntry->entryID, m_accum );
		break;
	case title:
		tagName = "title";
        strcpy( m_pCurEntry->title, m_accum );
		break;
    case src:
		tagName = "src";
		UnescapeUrl( m_accum );
        strcpy( m_pCurEntry->src, m_accum );
		break;
	case href:
		tagName = "href";
        strcat( m_pCurEntry->href, m_accum );
		break;
    case showFor:
		tagName = "showFor";
        m_pCurEntry->showFor = atoi( m_accum );
#ifdef FAST_AD_SCHEDULING
		m_pCurEntry->showFor /= 20;
#endif
		break;
	case showForMax:
		tagName = "showForMax";
        m_pCurEntry->showForMax = atoi( m_accum );
#ifdef FAST_AD_SCHEDULING
		m_pCurEntry->showForMax /= 20;
#endif
		break;
	case dayMax:
		tagName = "dayMax";
        m_pCurEntry->dayMax = atoi( m_accum );
		break;
	case blackBefore:
		tagName = "blackBefore";
        m_pCurEntry->blackBefore = atoi( m_accum );
		break;
	case blackAfter:
		tagName = "blackAfter";
        m_pCurEntry->blackAfter = atoi( m_accum );
		break;
	case startDT:
		tagName = "startDT";
		strcpy( m_pCurEntry->startDT, m_accum );
		break;
	case endDT:
		tagName = "endDT";
		strcpy( m_pCurEntry->endDT, m_accum );
		break;
	case reqInterval:
		if (m_pcip)
		{
			m_pcip->reqInterval = 3600*atoi(m_accum);
			m_pciValid |= PLCIreqIntervalValid;
#ifdef FAST_AD_SCHEDULING
			m_pcip->reqInterval /= 24*60/10;	// Pretend a day is ten minutes long
#endif
		}
		break;
	case historyLength:
		if (m_pcip)
		{
			m_pcip->historyLength = 24*60*D_SIXTY*atoi(m_accum);
			m_pciValid |= PLCIhistoryLengthValid;
		}
		break;
	case rerunInterval:
		if (m_pcip)
		{
			m_pcip->rerunInterval = 24*60*D_SIXTY*atoi(m_accum);
			m_pciValid |= PLCIrerunIntervalValid;
		}
		break;
	case scheduleAlgorithm:
		if (m_pcip)
		{
			if (strcmp(m_accum,"random"))
			{
				m_pcip->scheduleType = PlistCInfoSchedRandom;
				m_pciValid |= PLCIscheduleTypeValid;
			}
			else if (strcmp(m_accum,"linear"))
			{
				m_pcip->scheduleType = PlistCInfoSchedLinear;
				m_pciValid |= PLCIscheduleTypeValid;
			}
			// ignore others
		}
		break;
	case faceTimeQuota:
		if (m_pcip)
		{
			m_pcip->faceTimeQuota = 60*atoi(m_accum);
			m_pciValid |= PLCIfaceTimeQuotaValid;
#ifdef FAST_AD_SCHEDULING
			m_pcip->faceTimeQuota = 3*24;	// one 3-second showings of 24 ads
#endif
		}
		break;
	case profile:
		if ( m_pcip ) {
			m_pcip->profile = DupeString( m_accum );
			m_pciValid |= PLCIprofileValid;
		}
		break;
	case clientMode:
		// since this field is also set by the client, we steal the uppermost bit
		// for a dirty flag. in "process_plist_commands," which occurs immediately
		// following an update, before MGR_Idle returns, we flip the bit back
		// and send PLIST_SET_CLIENTMODE. [barf]
		if ( m_pcip ) {
			m_pcip->clientMode = atoi(m_accum);
			m_pcip->clientMode |= 0x80000000;
			m_pciValid |= PLCIclientModeValid;
		}
		break;

	case nag:
		if ( m_pcip && m_pcip->nagInfo ) {
			m_pcip->nagInfo[1] = (int) DupeString( m_accum );
			m_pciValid |= PLCInagInfoValid;
		}
		break;

    default:
        // not that this can't happen, but we should
        // make note of it!
//        assert( 0 );
        tagName = "Unknown Tag";
    };

	if ( *m_accum ) {
		printf( "%s=%s\n", tagName, m_accum );
		*m_accum = 0;
	}

    return 0;
}

void PlaylistParser::handleData( int /*id*/, const char* data, int len )
{
	int curLen = strlen( m_accum );
	strncat( m_accum, data, len );
	m_accum[len + curLen] = 0;
}

