/*
 * State.c
 *
 *  Created on: Jan 24, 2016
 *      Author: Sean Harrington
 */


//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------

#include "State.h"
#include "Events.h"

//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

typedef struct
{
	state_t balancer_state;
	Bool initialized;
} balancer_state_t;

static balancer_state_t system_state;

//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------

Void InitializeState()
{
	system_state.balancer_state = WAIT;
	system_state.initialized = TRUE;
}

state_t GetState()
{
	// Check if initialized. Throw error if not.
	return system_state.balancer_state;
}

Bool SetState(state_t nextState)
{
	// Semaphore for safety?
	// Check for value of nextState to see if within bounds?
	system_state.balancer_state = nextState;
	return TRUE;
}


//---------------------------------------------------------------------
// SYS/BIOS Functions (USER SHOULD NOT CALL)
//---------------------------------------------------------------------


//todo: Update RTOS to include this task somehow
Void StateChangeTask()
{
	UInt events;
	while (TRUE)
	{
		events = Event_pend(StateChangeEvent, Event_Id_NONE, ALL_STATE_EVENTS, BIOS_WAIT_FOREVER);

		// Determine event posted
		if (events & ERROR_EVENT)
		{
			SetState(ERROR);
			// Go into error state
		}
		else if (events & WAIT_EVENT)
		{
			SetState(WAIT);
		}
		else if (events & CHARGE_EVENT)
		{
			// Go into charge state
			SetState(CHARGE);
		}
		else if (events & BALANCE_EVENT)
		{
			SetState(BALANCE);
		}
		else if (events & CHARGE_BALANCE_EVENT)
		{
			SetState(CHARGE_BALANCE);
		}
		else
		{
			// What event just fired? Error
		}
	}
}

