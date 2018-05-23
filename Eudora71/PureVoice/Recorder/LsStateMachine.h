/*////////////////////////////////////////////////////////////////////////////

NAME:
	LsStateMachine - 

FILE:		LsStateMachine.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/??/96   lss     Initial
01/08/97   lss	   Track the last 3 states instead of 2
  
/*////////////////////////////////////////////////////////////////////////////

#ifndef _LSSTATEMACHINE_H_
#define _LSSTATEMACHINE_H_

#define BIND_EVENT_STATE_ACTION(Event, State, Action, Doc)					\
					Event.Init( State );									\
					State.Init( Event, Action);								\
					Action.Init( State, Doc )									

class LsStateEvent;
class LsStateAction;

class LsStateMachine
{
public:
//	LIFECYCLE
	LsStateMachine();
	void Init( LsStateEvent& Event, LsStateAction& Action );
	~LsStateMachine();
	// Default ctor, Copy ctor or assignment operator not allowed

//	OPERATIONS
	BOOL Update();

//	ACCESS
	LsStateType LastState( int last = 0 );
	LsStateType State( LsStateType state = INQUIRY_STATE );
	LsStateType NextState( LsStateType nextState = INQUIRY_STATE );
	void Param( void* param, BOOL bDelParam = FALSE );
	void* Param();

//  ENUMS
	enum {
		INQUIRY_STATE,

		NO_CHANGE_STATE,
		CHANGE_STATE,

		NONE,
		INITIAL,
		END,
		IDLE,
		PLAY,
		RECORD,
		PAUSE,
		REPOSITION,
		OPENFILE,
		ATTACH,
		ENCODE,
		RECORDENCODE,
		NEWRECORD,
		SAVEAS,

		RETURN_TO_LAST_STATE
	};

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
	BOOL Initial();
	BOOL End();
	BOOL Idle();
	BOOL Play();
	BOOL Record();
	BOOL Pause();
	BOOL Reposition();
	BOOL OpenFile();
	BOOL Attach();
	BOOL Encode();
	BOOL RecordEncode();
	BOOL NewRecord();
	BOOL SaveAs();

	void SaveState();
	void RestoreState();

	BOOL LsStateMachine::On_Action(
		LsStateType	state,
		BOOL		(LsStateAction::*action)(),
		LsStateType	fail = RETURN_TO_LAST_STATE );

	BOOL LsStateMachine::On_Action_Post(
		BOOL (LsStateAction::*post_action)(), 
		BOOL goodPrim );

	BOOL LsStateMachine::On_Action_Next(
		LsStateType	state, 
		BOOL		(LsStateAction::*action)(), 
		LsStateType	next,
		LsStateType	fail = RETURN_TO_LAST_STATE );

	BOOL LsStateMachine::On_Action_Return(
		LsStateType	state, 
		BOOL		(LsStateAction::*action)(),
		LsStateType	last_state,
		LsStateType	fail = RETURN_TO_LAST_STATE );

	BOOL Common_On_Action(
		LsStateType	state, 
		BOOL		(LsStateAction::*action)(), 
		LsStateType	next,
		LsStateType	fail );

//	ATTRIBUTES
	#define	STATE_STORAGE_LENGTH 3
	struct StateStorage 
	 {
		LsStateType NextState;
		LsStateType CurrentState;
		LsStateType LastState[STATE_STORAGE_LENGTH];
	
		StateStorage() : NextState(NONE), CurrentState(NONE)
		 {
			for( int i = 0; i < STATE_STORAGE_LENGTH; ++i )
				LastState[i] = NONE;			
		 }

		StateStorage& operator=( const StateStorage& rhs )
		 {
			if (this == &rhs) return *this;
			NextState = rhs.NextState;
			CurrentState = rhs.CurrentState;
			for( int i = 0; i < STATE_STORAGE_LENGTH; ++i )
				LastState[i] = rhs.LastState[i];

			return *this;
		 }

		int Length() { return STATE_STORAGE_LENGTH; }
	 };

	StateStorage	m_State;
	StateStorage	m_StateMem;		// used by SaveState and RestoreState

	BOOL			mb_RtnFrmLastFailedState;
	BOOL			mb_ReturnState;
/*
	LsStateType		me_NextState;
	LsStateType		me_CurrentState;
	LsStateType		me_LastState;
	LsStateType		me_LastState1;	// last last state
*/	
	void*			mp_param;

	LsStateEvent*	mp_Event;
	LsStateAction*	mp_Action;

// Do not allow copy ctor or assignment
	LsStateMachine( const LsStateMachine& );
	LsStateMachine& operator=( const LsStateMachine& );
};

#endif