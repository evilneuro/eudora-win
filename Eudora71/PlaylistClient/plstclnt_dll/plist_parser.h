// plist_parser.h

#ifndef _PLIST_PARSER_H_
#define _PLIST_PARSER_H_

#include "xml_parser_base.h"
#include "plist_stg.h"
#include "plist_cinfo.h"
#include "plist_cmd.h"

///////////////////////////////////////////////////////////////////////////////
// The ol' ElementMap

enum element_ids {
    clientUpdateResponse,
    playlist,
    playlistID,
	width,
	height,
    entry,
	entryID,
	title,
    src,
	href,
    showFor,
	showForMax,
	dayMax,
	blackBefore,
	blackAfter,
	startDT,
	endDT,
	pastry,
	reqInterval,
	historyLength,
	scheduleAlgorithm,
	faceTimeQuota,
	flush,
	reset,
	fault,
	rerunInterval,
	profile,
	clientMode,
	nag
};


///////////////////////////////////////////////////////////////////////////////
// PlaylistParser -- turns xml plists into binary plists

class PlaylistParser : public XmlParser
{
	int m_error;
	char m_accum[1025];

    PlaylistHeader*   m_pPlaylistHeader;
    Entry*            m_pEntries;
    Entry*            m_pCurEntry;
	PlistCmd*         m_pCommands;

public:
    PlaylistParser::PlaylistParser(){
    	m_pEntries = m_pCurEntry = NULL;
		m_pPlaylistHeader = 0;
		m_pCommands = 0;
		*m_accum = 0; m_error = 0;
		m_pcip = new PlistCInfo;
		if (m_pcip) memset(m_pcip, '\0', sizeof(PlistCInfo));
		m_pciValid=0;
	}
    ~PlaylistParser(){delete m_pcip;};

    int ImportPlaylist( char* xml, PlistCInfoPtr pcip );
    int ImportPlaylist( char* xml, unsigned len, bool done );

	PlistCInfoPtr m_pcip;
	int m_pciValid;

private:
    bool initElementMap( ElementMap** map );
    int startElement( int id, const char* name, const char** atts );
    int endElement( int id, const char* name );
    void handleData( int id, const char* data, int len );
};

#endif    // _PLIST_PARSER_H_
