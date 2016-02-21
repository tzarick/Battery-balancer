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


//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

static Bool mInitialized = FALSE;

static Int32 mInternalTimerTicks;

// Speed of the system clock in MHz
#define SYS_CLK_MHZ	60

// Timer period in microseconds
#define TIMER_PERIOD	1000

//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------

void Timer_Init(void)
{
	mInternalTimerTicks = 0;
	InitCpuTimers();
	ConfigCpuTimer(&CpuTimer0, SYS_CLK_MHZ, TIMER_PERIOD);
	mInitialized = TRUE;
	return mInitialized;
}

void Timer_Update(void)
{

}

void Timer_Setup(timer_t * timer, Int32 runtime, timerCallback * cb)
{
	timer->_startTime = mInternalTimerTicks;
	timer->_endTime = mInternalTimerTicks + runTime;
	timer->_timeElapsed = FALSE;
	timer->_timerActive = TRUE;
	timer->callback = cb;
}

void Timer_ISR(void)
{
	mInternalTimerTicks++;
}

