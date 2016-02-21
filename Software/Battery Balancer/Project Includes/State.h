/*
 * State.h
 *
 *  Created on: Jan 24, 2016
 *      Author: Sean Harrington
 */

#ifndef PROJECT_INCLUDES_STATE_H_
#define PROJECT_INCLUDES_STATE_H_

//------------------------------------------------
// Common Includes

#include "Common_Includes.h"

//------------------------------------------------
// Public variables

// Bitmask of states the system can be in
typedef enum
{
	WAIT,
	CHARGE,
	BALANCE,
	CHARGE_BALANCE,
	ERROR
} state;

//------------------------------------------------
// Public functions

Void InitializeState();

state GetState();

Bool SetState(state nextState);

//------------------------------------------------


#endif /* PROJECT_INCLUDES_STATE_H_ */
