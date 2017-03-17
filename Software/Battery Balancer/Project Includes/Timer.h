/*
 * Timer.h
 *
 *  Created on: Feb 20, 2016
 *      Author: Sean Harrington
 */

#ifndef PROJECT_INCLUDES_TIMER_H_
#define PROJECT_INCLUDES_TIMER_H_

//---------------------------------------------------------------------------------
// Common Includes

#include "Common_Includes.h"
#include "Error.h"

//---------------------------------------------------------------------------------
// Public variables

typedef void timerCallback(void * timerAddr);

/// Timer struct that can be used to check if a certain amount of time has elapsed.
typedef struct
{
	Int32 _startTime;
	Int32 _endTime;
	Bool _timerActive;
	Bool _timeElapsed;
	timerCallback * callback;
} timer_t;



//---------------------------------------------------------------------------------

/// Initializes the Timer module.
void Timer_Init(void);

/// Checks the current time against all active timers
void Timer_Update(void);

/// Sets up a new timer that can keep track of the time it's been active.
///
///	pre:	Module has been initialized.
///	param:	timer (timer_t *) - The timer that should be setup
/// param:	runtime (Int32) - The time in milliseconds that the timer will count
///			down from. Once the time reaches 0, it's #timeElapsed flag will be set.
///	param:	cb (timerCallback *) - The function that should be called when the timer
///			reaches a value of 0. This function can be NULL if no callback is
///			desired.
error_t Timer_Setup(timer_t * timer, timerCallback * cb);

void Timer_Start(timer_t * timer, Int32 runtime);

void Timer_Stop(timer_t * timer);

Bool Timer_HasElapsed(timer_t * timer);

Bool Timer_IsActive(timer_t * timer);


#endif /* PROJECT_INCLUDES_TIMER_H_ */
