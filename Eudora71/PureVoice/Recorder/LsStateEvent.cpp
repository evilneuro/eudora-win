/*////////////////////////////////////////////////////////////////////////////

NAME:
	LsStateEvent - 

FILE:		LsStateEvent.CPP
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.

DEPENDENCIES:

FILES:


REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#include "LsStateMachine.h"

// LOCAL INCLUDES
#include "LsStateEvent.h"

// LOCAL DEFINES
#ifdef _DEBUG
char *EVENT_NAME[] = {
		"NONE",
		"INITIAL",
		"PLAY",
		"RECORD",
		"STOP",
		"PAUSE",
		"REPOSITION",
		"OPENFILE",
		"ATTACH",
		"ENCODE",
		"PLAYEND",
		"NEW",
		"SAVEAS",
		"END"};
#endif

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

LsStateEvent::LsStateEvent() :
	me_LastEvent(0), me_Event(0)
{
}

LsStateEvent::~LsStateEvent()
{
}

/*inline*/ void LsStateEvent::Init( LsStateMachine& State )
{
	mp_State = &State;
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL LsStateEvent::Event( int event,
	void* param,						// = 0
	BOOL bDelParam )					// = FALSE
{
#ifdef _DEBUG
	TRACE("Event %s(%d) occured.\n", EVENT_NAME[event], event );
#endif
	me_Event = event;
	mp_State->Param( param, bDelParam );
	return mp_State->Update();
}

void LsStateEvent::EventAck( BOOL bAck )
{
#ifdef _DEBUG
	TRACE("Event %s(%d) acknowledged (%d)\n", EVENT_NAME[me_Event], me_Event, bAck );
#endif
	if ( bAck )
	{
		me_LastEvent = me_Event;
		me_Event = NONE;
	}
}
//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

/*inline*/ int LsStateEvent::LastEvent()
{
	return me_LastEvent;
}

/*inline*/ int LsStateEvent::Event()
{
	return me_Event; 
}
