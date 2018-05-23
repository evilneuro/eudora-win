/*////////////////////////////////////////////////////////////////////////////

NAME:
	LsStateEvent - 

FILE:		LsStateEvent.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.
	Must call Init() before any use.

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

#ifndef _LSSTATEEVENT_H_
#define _LSSTATEEVENT_H_

class LsStateMachine;

class LsStateEvent
{
public:
//	LIFECYCLE
	LsStateEvent();
	void Init( LsStateMachine& State );
	~LsStateEvent();
	// Copy ctor or assignment operator not allowed

//	OPERATIONS
	BOOL Event( int event, void* param = 0, BOOL bDelParam = FALSE );
	void EventAck( BOOL bAck = TRUE );

//	ACCESS
	int LastEvent();
	int Event();
			
	enum {
		NONE,

		INITIAL,
		PLAY,
		RECORD,
		STOP,
		PAUSE,
		REPOSITION,
		OPENFILE,
		ATTACH,
		ENCODE,
		PLAYEND,
		NEW,
		SAVEAS,
		END
	};

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS

//	ATTRIBUTES
	LsStateMachine*	mp_State;
	int				me_Event;
	int				me_LastEvent;

// Do not allow copy ctor or assignment
	LsStateEvent( const LsStateEvent& );
	LsStateEvent& operator=( const LsStateEvent& );

};

#endif
