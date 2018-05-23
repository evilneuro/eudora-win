/*////////////////////////////////////////////////////////////////////////////

NAME:
	LsStateMachine - 

FILE:		LsStateMachine.CPP
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.
	Must call Init() before any use.

DEPENDENCIES:
	Coupled with LsStateEvent and LsStateAction

NOTES:
	Need to have a small event queue so only one event is addressed
	at a time

FILES:


REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/??/96   lss     -Initial
01/08/97   lss	   -Added another path from Pause to RecordEncode with event
				    of REALTIME and stop in Pause state
01/17/97   lss     -Changed path from Idle to Play in OpenFile
03/25/97   lss     -Added SAVEAS state for saving to a file
  
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#include "LsStateEvent.h"
#include "LsStateAction.h"

// LOCAL INCLUDES
#include "LsStateMachine.h"

// LOCAL DEFINES
#ifdef _DEBUG
char *STATE_NAME[] = {
		"INQUIRY_STATE",

		"NO_CHANGE_STATE",
		"CHANGE_STATE",

		"NONE",
		"INITIAL",
		"END",
		"IDLE",
		"PLAY",
		"RECORD",
		"PAUSE",
		"REPOSITION",
		"OPENFILE",
		"ATTACH",
		"ENCODE",
		"RECORDENCODE",
		"NEWRECORD",
		"SAVEAS"
};
#endif

#define BEGIN_ON_EVENT_NEXT			if (0) {}
#define END_ON_EVENT_NEXT			
//#define END_ON_EVENT_NEXT													\
//					else													\
//					 {														\
//						ret = FALSE;										\
//					 }

#define ON_EVENT_NEXT( event, next_state )									\
					else if (event) NextState( next_state )


#define BEGIN_ON_STATE_NEXT			switch ( LastState() ) {
#define END_ON_STATE_NEXT			}
#define ON_STATE_NEXT( last_state, next_state )								\
					case (last_state) : NextState( next_state );			\
										mb_ReturnState = TRUE; break

#ifdef _DEBUG
#define ON_ACTION( state, action )											\
					if ((state) == NextState())								\
					 {														\
						LsStateType ls = LastState();						\
						State( NextState() );								\
						TRACE("Entering State: %s\n", STATE_NAME[state]);	\
						action;												\
						NextState( NONE );									\
						mp_Action->UpdateView();							\
						TRACE("Exiting State: %s\n", STATE_NAME[state]);	\
						mp_Event->EventAck();								\
					 }

#define ON_ACTION_POST( post_action )										\
					if ( (NONE != NextState()) && ret )						\
					 {														\
						TRACE("Entering Post Action for State: %s\n",		\
								STATE_NAME[State()]);						\
						post_action;										\
					 }

#define ON_ACTION_FAIL( state, action, fail )								\
					if ((state) == NextState())								\
					 {														\
						SaveState();										\
						State( NextState() );								\
				TRACE("Entering State: %s\n", STATE_NAME[state]);			\
						if (action)											\
						 {													\
							NextState( NONE );								\
							mp_Action->UpdateView();						\
						 }													\
						else												\
						 {													\
							RestoreState();									\
							NextState( fail );								\
							ret = FALSE;									\
						 }													\
				TRACE("Exiting State: %s (%d)\n", STATE_NAME[state], ret);	\
						mp_Event->EventAck();								\
					 }


#define ON_ACTION_NEXT( state, action, next_state )							\
					if ((state) == NextState())								\
					 {														\
						SaveState();										\
						State( NextState() );								\
						TRACE("Entering State: %s\n", STATE_NAME[state]);	\
						if (action)											\
						 {													\
							NextState( next_state );						\
							mp_Action->UpdateView();						\
						 }													\
						else												\
						 {													\
							RestoreState();									\
							NextState( IDLE );								\
							ret = FALSE;									\
						 }													\
				TRACE("Exiting State: %s (%d)\n", STATE_NAME[state], ret);	\
						mp_Event->EventAck();								\
					 }

#define ON_ACTION_RETURN( state, action, last_state )						\
					if ( ((state) == NextState()) &&						\
						 ((last_state) == State()) )						\
					 {														\
						SaveState();										\
						State( NextState() );								\
						TRACE("Entering State: %s\n", STATE_NAME[state]);	\
						if (action)											\
						 {													\
							NextState( last_state );						\
							mp_Action->UpdateView();						\
						 }													\
						else												\
						 {													\
							RestoreState();									\
							NextState( IDLE );								\
							ret = FALSE;									\
						 }													\
				TRACE("Exiting State: %s (%d)\n", STATE_NAME[state], ret);	\
						mp_Event->EventAck();								\
					 }
#else
#define ON_ACTION( state, action )											\
					if ((state) == NextState())								\
					 {														\
						LsStateType ls = LastState();						\
						State( NextState() );								\
						action;												\
						NextState( NONE );									\
						mp_Action->UpdateView();							\
						mp_Event->EventAck();								\
					 }

#define ON_ACTION_POST( post_action )										\
					if ( (NONE != NextState()) && ret )						\
					 {														\
						post_action;										\
					 }

#define ON_ACTION_FAIL( state, action, fail )								\
					if ((state) == NextState())								\
					 {														\
						SaveState();										\
						State( NextState() );								\
						if (action)											\
						 {													\
							NextState( NONE );								\
							mp_Action->UpdateView();						\
						 }													\
						else												\
						 {													\
							RestoreState();									\
							NextState( fail );								\
							ret = FALSE;									\
						 }													\
						mp_Event->EventAck();								\
					 }


#define ON_ACTION_NEXT( state, action, next_state )							\
					if ((state) == NextState())								\
					 {														\
						SaveState();										\
						State( NextState() );								\
						if (action)											\
						 {													\
							NextState( next_state );						\
							mp_Action->UpdateView();						\
						 }													\
						else												\
						 {													\
							RestoreState();									\
							NextState( IDLE );								\
							ret = FALSE;									\
						 }													\
						mp_Event->EventAck();								\
					 }

#define ON_ACTION_RETURN( state, action, last_state )						\
					if ( ((state) == NextState()) &&						\
						 ((last_state) == State()) )						\
					 {														\
						SaveState();										\
						State( NextState() );								\
						if (action)											\
						 {													\
							NextState( last_state );						\
							mp_Action->UpdateView();						\
						 }													\
						else												\
						 {													\
							RestoreState();									\
							NextState( IDLE );								\
							ret = FALSE;									\
						 }													\
						mp_Event->EventAck();								\
					 }
#endif

#define Event()		mp_Event->Event()

/*
#define ON_ACTION_FAIL( state, action, fail )								\
					if ((state) == NextState())								\
					 {														\
						LsStateType ls = LastState();						\
						State( NextState() );								\
				TRACE("Entering State: %s\n", STATE_NAME[state]);			\
						if (action)											\
						 {													\
							NextState( NONE );								\
							mp_Action->UpdateView();						\
						 }													\
						else												\
						 {													\
							NextState( fail );								\
							me_LastState = ls;								\
							me_CurrentState = LastState();					\
							ret = FALSE;									\
						 }													\
				TRACE("Exiting State: %s (%d)\n", STATE_NAME[state], ret);	\
						mp_Event->EventAck();								\
					 }
*/

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

LsStateMachine::LsStateMachine() :
	mp_Event(0), mp_Action(0), mp_param(0), 
	mb_RtnFrmLastFailedState( FALSE ), mb_ReturnState( FALSE )
{
	NextState( INITIAL );
}

void LsStateMachine::Init( LsStateEvent& Event, LsStateAction& Action )
{
	mp_Event = &Event;
	mp_Action = &Action;
}

LsStateMachine::~LsStateMachine()
{
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL LsStateMachine::Update()
{
	BOOL	real_ret, ret, no_more = FALSE;
	int		state = State();

	// see if user wants to quit the application
	if ( Event() == LsStateEvent::END )
	 {
		NextState( END );
		return End();
	 }

	while(1)
	 {
		switch( state )
		 {
			case INITIAL:
				ret = Initial();
			 break;
			case END:
				ret = End();
			 break;
			case IDLE:
				ret = Idle();
			 break;
			case PLAY:
				ret = Play();
			 break;
			case PAUSE:
				ret = Pause();
			 break;
			case REPOSITION:
				ret = Reposition();
			 break;
			case RECORD:
				ret = Record();
			 break;
			case OPENFILE:
				ret = OpenFile();
			 break;
			case ENCODE:
				ret = Encode();
			 break;
			case RECORDENCODE:
				ret = RecordEncode();
			 break;
			case ATTACH:
				ret = Attach();
			 break;
			case NEWRECORD:
				ret = NewRecord();
			 break;
			case SAVEAS:
				ret = SaveAs();
			 break;
			case NONE:
				TRACE("NONE state!!!\n");
			 break;
			default:
				TRACE("Hey, illegal state!!!\n");
			 return FALSE;
		 }
		state = NextState();

		// this if-else is used to return the status of
		// state that automatically returns to previous state
		if ( mb_ReturnState )
		 {
			real_ret = ret;
			mb_ReturnState = FALSE;
			no_more = TRUE;
		 }
		else if ( !no_more )
			real_ret = ret;

		if ( state == NONE )
			break;
	 } 

	return real_ret;
}

//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

LsStateType LsStateMachine::LastState( int last /* = 0 */ )
{
/*
	if ( last == 0 )
		return me_LastState;
	return me_LastState1;
*/
	if ( last < 0 )
		last = 0;
	else if ( last > (m_State.Length() - 1) )
		last = m_State.Length() - 1;

	return m_State.LastState[last];	
}

LsStateType LsStateMachine::State( LsStateType state )
{
/*
	if ( state == INQUIRY_STATE )
		return me_CurrentState;

	me_LastState1 = me_LastState;
	me_LastState = me_CurrentState;

	return me_CurrentState = state;
*/
	if ( state == INQUIRY_STATE )
		return m_State.CurrentState;

	for( int i = m_State.Length() - 1; i > 0 ; --i )
		m_State.LastState[i] = m_State.LastState[i-1];

	m_State.LastState[0] = m_State.CurrentState;

	return m_State.CurrentState = state;
}

LsStateType LsStateMachine::NextState( LsStateType nextState )
{
/*
	if ( nextState == INQUIRY_STATE )
		return me_NextState;

	return me_NextState = nextState;
*/
	if ( nextState == INQUIRY_STATE )
		return m_State.NextState;

	return m_State.NextState = nextState;
}

void LsStateMachine::Param( void* param, BOOL bDelParam )
{
	mp_param = param;
}

void* LsStateMachine::Param()
{
	return mp_param;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void LsStateMachine::SaveState()
{
	m_StateMem = m_State;
}

void LsStateMachine::RestoreState()
{
	m_State = m_StateMem;
}

BOOL LsStateMachine::Initial()
{
	BOOL ret = TRUE;

//	ON_ACTION( INITIAL, mp_Action->OnInitial() );
	ret = On_Action( INITIAL, &LsStateAction::OnInitial );

	BEGIN_ON_EVENT_NEXT
		ON_EVENT_NEXT( Event() == LsStateEvent::RECORD, RECORD );
		ON_EVENT_NEXT( Event() == LsStateEvent::OPENFILE, OPENFILE );
	END_ON_EVENT_NEXT

//	ON_ACTION_POST( mp_Action->PostOnInitial() );
	On_Action_Post( &LsStateAction::PostOnInitial, ret );

	return ret;
}

BOOL LsStateMachine::End()
{
	BOOL ret = TRUE;

	ret = On_Action( END, &LsStateAction::OnIdle, NONE );

	// We end up here by user clicking on Attach
	if ( (LastState(1) == IDLE) && (LastState() == ATTACH) )
	 {
		TRACE("Quit from attach\n");
		AfxGetMainWnd()->PostMessage( WM_CLOSE );
		return TRUE;
	 }

	BEGIN_ON_EVENT_NEXT
		// this is the case when user quits without attaching
		// but now wants to attach
		ON_EVENT_NEXT( Event() == LsStateEvent::ATTACH, ATTACH );
		// this is the case when user changes his/her mind
		// and cancels the quit operation
		ON_EVENT_NEXT( Event() == LsStateEvent::STOP, IDLE );
	END_ON_EVENT_NEXT

	return ret;
}

BOOL LsStateMachine::Idle()
{
	BOOL ret = TRUE;

//	ON_ACTION( IDLE, mp_Action->OnIdle() );
	ret = On_Action( IDLE, &LsStateAction::OnIdle, NONE );

	BEGIN_ON_EVENT_NEXT
		ON_EVENT_NEXT( Event() == LsStateEvent::PLAY, PLAY );
		ON_EVENT_NEXT( Event() == LsStateEvent::RECORD, RECORD );
		ON_EVENT_NEXT( Event() == LsStateEvent::OPENFILE, OPENFILE );
		ON_EVENT_NEXT( Event() == LsStateEvent::SAVEAS, SAVEAS );
		ON_EVENT_NEXT( Event() == LsStateEvent::ATTACH, ATTACH );
		ON_EVENT_NEXT( Event() == LsStateEvent::NEW, NEWRECORD );
		ON_EVENT_NEXT( (Event() == LsStateEvent::ENCODE) &&
				(mp_Action->IsValidToConvert()), ENCODE );
//		ON_EVENT_NEXT( (Event() == LsStateEvent::DECODE) &&
//				(mp_Action->IsValidToDecode()), DECODE );
		ON_EVENT_NEXT( Event() == LsStateEvent::REPOSITION, 
															REPOSITION );
	END_ON_EVENT_NEXT

	return ret;
}

BOOL LsStateMachine::Play()
{
	BOOL ret = TRUE;

//	ON_ACTION_FAIL( PLAY, mp_Action->OnPlay(), IDLE );
	ret = On_Action( PLAY, &LsStateAction::OnPlay, IDLE );

	BEGIN_ON_EVENT_NEXT
		ON_EVENT_NEXT( Event() == LsStateEvent::STOP, IDLE );
//		ON_EVENT_NEXT( Event() == LsStateEvent::GAIN, GAIN );
		ON_EVENT_NEXT( Event() == LsStateEvent::PAUSE, PAUSE );
		ON_EVENT_NEXT( Event() == LsStateEvent::REPOSITION, 
															REPOSITION );
	END_ON_EVENT_NEXT

	return ret;
}

BOOL LsStateMachine::Pause()
{
	BOOL ret = TRUE;

	// This whole if-else is for checking if we came from
	// Ghost State (see REPOSITION state). This implementation
	// is a kludge. Will fix this later.
	if ( (NextState() == PAUSE) && (State() == PAUSE) )
	 {
		NextState( NONE );
	 }
	else
	 {
//		ON_ACTION_FAIL( PAUSE, mp_Action->OnPause(), IDLE );
		ret = On_Action( PAUSE, &LsStateAction::OnPause );
	 }

	BEGIN_ON_EVENT_NEXT
		ON_EVENT_NEXT( (Event() == LsStateEvent::STOP) &&
					   (mp_Action->RecordMode() == LsStateAction::REALTIME)
					   && (LastState() == RECORD),
					   RECORDENCODE );
		ON_EVENT_NEXT( Event() == LsStateEvent::STOP, IDLE );
		ON_EVENT_NEXT( ((Event() == LsStateEvent::PLAY) ||
						(Event() == LsStateEvent::PAUSE)) &&
						 (LastState() == PLAY), PLAY );
		ON_EVENT_NEXT( (Event() == LsStateEvent::REPOSITION) &&
					   (LastState() != RECORD), REPOSITION );
		ON_EVENT_NEXT( (Event() == LsStateEvent::RECORD) ||
						((Event() == LsStateEvent::PAUSE) &&
						 LastState() == RECORD), RECORD );
	END_ON_EVENT_NEXT

	return ret;
}

BOOL LsStateMachine::Record()
{
	BOOL ret = TRUE;
#ifndef LS_PLAYER_ONLY
//	ON_ACTION_FAIL( RECORD, mp_Action->OnRecord(), IDLE );
	ret = On_Action( RECORD, &LsStateAction::OnRecord );
/*
	// This will kick back to INITIAL state if record
	// failed and the last state was from INITIAL
	// Klugde for now. Will code "return to last state if failed" mechanism
	if ( !ret && (State() == INITIAL) )
	 {
		NextState( INITIAL );
		return FALSE;
	 }
*/
	// WARNING: Ordering is important! Note last two ON_EVENT_NEXT
	BEGIN_ON_EVENT_NEXT
		ON_EVENT_NEXT( Event() == LsStateEvent::PAUSE, PAUSE );
		ON_EVENT_NEXT( (Event() == LsStateEvent::STOP) &&
					   (mp_Action->RecordMode() == LsStateAction::REALTIME),
					    RECORDENCODE );
		ON_EVENT_NEXT( Event() == LsStateEvent::STOP, IDLE );
	END_ON_EVENT_NEXT
#endif
	return ret;
}

BOOL LsStateMachine::Encode()
{
	BOOL ret = TRUE;
#ifndef LS_PLAYER_ONLY
//	ON_ACTION_FAIL( ENCODE, mp_Action->OnEncode(), IDLE );
	ret = On_Action( ENCODE, &LsStateAction::OnEncode );

	BEGIN_ON_EVENT_NEXT
		ON_EVENT_NEXT( Event() == LsStateEvent::STOP, IDLE );
	END_ON_EVENT_NEXT
#endif
	return ret;
}

BOOL LsStateMachine::RecordEncode()
{
	BOOL ret = TRUE;
#ifndef LS_PLAYER_ONLY
//	ON_ACTION_FAIL( RECORDENCODE, mp_Action->OnRecordEncode(), IDLE );
	ret = On_Action( RECORDENCODE, &LsStateAction::OnRecordEncode );

	BEGIN_ON_EVENT_NEXT
		ON_EVENT_NEXT( Event() == LsStateEvent::STOP, IDLE );
	END_ON_EVENT_NEXT
#endif
	return ret;
}

BOOL LsStateMachine::Reposition()
{
	BOOL ret = TRUE;
	LsStateType next_state = NONE;

	// This becomes a ghost state if we came from PAUSE state.
	// Ghost state means that as far as the record
	// of state transitions is concerned, it never happened.
	// This is achieved by saving the state record then
	// restoring it afterwards.
	// This particular implementation is a kludge. Will
	// fix this later.
	if ( State() == PAUSE )
	 {
		next_state = PAUSE;
		SaveState();
	 }

//	ON_ACTION( REPOSITION, mp_Action->OnReposition() );
	ret = On_Action( REPOSITION, &LsStateAction::OnReposition );

	BEGIN_ON_STATE_NEXT
		ON_STATE_NEXT( IDLE, IDLE );
		ON_STATE_NEXT( PLAY, PLAY );
		ON_STATE_NEXT( PAUSE, PAUSE );
	END_ON_STATE_NEXT

	if ( next_state != NONE )
	 {
		RestoreState();
		NextState( next_state );
	 }

	return ret;
}

BOOL LsStateMachine::OpenFile()
{
	BOOL ret = TRUE;

//	ON_ACTION( OPENFILE, mp_Action->OnOpenFile() );
	ret = On_Action( OPENFILE, &LsStateAction::OnOpenFile );

	// if failed, just return 
	if ( !ret ) return ret;

	BEGIN_ON_STATE_NEXT
		ON_STATE_NEXT( IDLE, PLAY );
		ON_STATE_NEXT( INITIAL, PLAY );
	END_ON_STATE_NEXT

	return ret;
}

BOOL LsStateMachine::SaveAs()
{
	BOOL ret = TRUE;

	ret = On_Action( SAVEAS, &LsStateAction::OnSaveAs );

	BEGIN_ON_STATE_NEXT
		ON_STATE_NEXT( IDLE, IDLE );
	END_ON_STATE_NEXT

	return ret;
}

BOOL LsStateMachine::Attach()
{
	BOOL ret = TRUE;
#ifndef LS_PLAYER_ONLY
//	ON_ACTION( ATTACH, mp_Action->OnAttach() );
	ret = On_Action( ATTACH, &LsStateAction::OnAttach );

	BEGIN_ON_STATE_NEXT
//		ON_STATE_NEXT( IDLE, IDLE );
		ON_STATE_NEXT( IDLE, END );
		ON_STATE_NEXT( END, END );
	END_ON_STATE_NEXT
#endif
	return ret;
}

BOOL LsStateMachine::NewRecord()
{
	BOOL ret = TRUE;
#ifndef LS_PLAYER_ONLY
	//ON_ACTION( NEWRECORD, mp_Action->OnNewRecord() );
	ret = On_Action( NEWRECORD, &LsStateAction::OnNewRecord );

	BEGIN_ON_STATE_NEXT
		ON_STATE_NEXT( IDLE, INITIAL );
	END_ON_STATE_NEXT
#endif
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline BOOL LsStateMachine::On_Action(
	LsStateType	state,
	BOOL		(LsStateAction::*action)(),
	LsStateType	fail	// = RETURN_TO_LAST_STATE
)
{
	return Common_On_Action( state, action, NONE, fail );
}

inline BOOL LsStateMachine::On_Action_Post(
	BOOL (LsStateAction::*post_action)(),
	BOOL goodPrim )
{
	if ( (NONE != NextState()) && goodPrim )
	 {
#ifdef _DEBUG
	TRACE("Post Action for State: %s\n", STATE_NAME[State()]);
#endif
		return ((mp_Action->*post_action)());
	 }
	return TRUE;
}
/*
BOOL LsStateMachine::On_Action_Ghost(
	LsStateType	state, 
	BOOL		(LsStateAction::*action)(), 
	LsStateType	last_state,
	LsStateType	fail	// = RETURN_TO_LAST_STATE
)
{
	// This particular path of state is a ghost state.
	// Ghost state means that as far as the record
	// of state transitions is concerned, it never happened.
	// This is achieved by saving the state record then
	// restoring it afterwards.

	LsStateType next_state = State();
	SaveState();

	ret = Common_On_Action( state, action, next, fail );
	mb_UpdateView = FALSE;

	RestoreState();
	NextState( next_state );

	return ret;
}
*/
inline BOOL LsStateMachine::On_Action_Next(
	LsStateType	state, 
	BOOL		(LsStateAction::*action)(), 
	LsStateType	next,
	LsStateType	fail	// = RETURN_TO_LAST_STATE
)
{
	return Common_On_Action( state, action, next, fail );
}

inline BOOL LsStateMachine::On_Action_Return(
	LsStateType	state, 
	BOOL		(LsStateAction::*action)(),
	LsStateType	last_state,
	LsStateType	fail	// = RETURN_TO_LAST_STATE
)
{
	if ( last_state == State() )
		return Common_On_Action( state, action, last_state, fail );

	return FALSE;
}

BOOL LsStateMachine::Common_On_Action(
	LsStateType	state, 
	BOOL		(LsStateAction::*action)(), 
	LsStateType	next,
	LsStateType	fail
)
{
	BOOL ret = TRUE;

	if ( state == NextState() )
	 {
		if ( mb_RtnFrmLastFailedState )
		 {
			mb_RtnFrmLastFailedState = FALSE;
//	 		if ( mb_UpdateView )
//			 {
				mp_Action->UpdateView();
//				mb_UpdateView = TRUE;
//			 }
			NextState( NONE );
			return TRUE;
		 }
		SaveState();
		State( NextState() );
#ifdef _DEBUG
	TRACE("Entering State: %s\n", STATE_NAME[state]);
#endif
		if ( (mp_Action->*action)() || (fail == NONE) )
		 {
			NextState( next );
			mp_Action->UpdateView();
		 }
		else
		 {
			RestoreState();
			if ( fail == RETURN_TO_LAST_STATE )
			 {
				NextState( State() );
				mb_RtnFrmLastFailedState = TRUE;
				mb_ReturnState = TRUE;
			 }
			else
				NextState( fail );
			
			ret = FALSE;
		 }
#ifdef _DEBUG
	TRACE("Exiting State: %s (%d)\n", STATE_NAME[state], ret);
#endif
		mp_Event->EventAck();
	 }
	else
		mp_Event->EventAck( FALSE );

	return ret;
}
