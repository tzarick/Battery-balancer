/*
 * Timer.c
 *
 *  Created on: Feb 20, 2016
 *      Author: Sean Harrington
 */

#include "Timer.h"

//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

/// The maximum amount of timers that will be updated. This can be increased
/// with the penalty of more memory and CPU consumption.
#define MAX_TIMERS			(150)

//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

timer_t * mValidTimers[MAX_TIMERS] = {0};

//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

/// The amount of timers that have been setup
static UInt16 mTimersSetup;

/// Whether the module is initialized
static Bool mInitialized = FALSE;

/// The current time in ticks
static Int32 mInternalTimerTicks;

/// Speed of the system clock in MHz
#define SYS_CLK_MHZ	60

/// Timer period in microseconds
#define TIMER_PERIOD	1000


//__interrupt void  TINT0_ISR(void);

//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------

void Timer_Init(void)
{
	mTimersSetup = 0;
	mInternalTimerTicks = 0;

	//PieVectTable.TINT0 = &TINT0_ISR;

	InitCpuTimers();
	ConfigCpuTimer(&CpuTimer0, SYS_CLK_MHZ, TIMER_PERIOD);

	mInitialized = TRUE;
	return mInitialized;
}

void Timer_Update(void)
{
	Int32 currentTime = mInternalTimerTicks;
	Int16 i;
	for (i = 0; i < mTimersSetup; i++)
	{
		if ((currentTime >= mValidTimers[i]->_endTime) && (mValidTimers[i]->_timerActive == TRUE))
		{
			/// Flip elapsed and active flag
			mValidTimers[i]->_timeElapsed = TRUE;
			mValidTimers[i]->_timerActive = FALSE;

			/// Call the timer's callback function
			if (mValidTimers[i]->callback != NULL)
			{
				mValidTimers[i]->callback((void *)&(mValidTimers[i]));
			}
		}
	}
}

error_t Timer_Setup(timer_t * timer, timerCallback * cb)
{
	error_t retVal = errNone;
	timer->_startTime = 0;
	timer->_endTime = 0;
	timer->_timeElapsed = FALSE;
	timer->_timerActive = FALSE;
	timer->callback = cb;

	if (mTimersSetup < MAX_TIMERS)
	{
		mValidTimers[mTimersSetup] = timer;
		mTimersSetup++;
	}
	else
	{
		// todo: FIX ME
		retVal = 0;
	}
	return retVal;
}

void Timer_Start(timer_t * timer, Int32 runtime)
{
	timer->_startTime = mInternalTimerTicks;
	timer->_endTime = mInternalTimerTicks + runtime;
	timer->_timeElapsed = FALSE;
	timer->_timerActive = TRUE;
}

void Timer_Stop(timer_t * timer)
{
	timer->_timerActive = FALSE;
}

Bool Timer_HasElapsed(timer_t * timer)
{
	return timer->_timeElapsed;
}

Bool Timer_IsActive(timer_t * timer)
{
	return timer->_timerActive;
}

void Timer_ISR(void)
{
	mInternalTimerTicks++;
}
