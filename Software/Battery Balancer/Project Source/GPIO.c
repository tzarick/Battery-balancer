/*
 * GPIO.c
 *
 *  Created on: Jan 23, 2016
 *      Author: Sean Harrington
 */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "GPIO.h"
#include "State.h"
#include "Events.h"

//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------



#define DISABLE_PULLUP	1

//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------

// @todo: Documentation
Void Gpio_Init()
{

	// Setup GPIO Registers
	EALLOW;


	EDIS;

	EALLOW;

	// Hex encoder as 3 position switch (temporary)
	//GPIO A Mux1 register
	GpioCtrlRegs.GPAMUX1.bit.GPIO12 = GPIO_MUX;
	//GPIO A direction register
	GpioCtrlRegs.GPADIR.bit.GPIO12 = GPIO_IN;
	//GPIO A Pull-Up disable register 
	GpioCtrlRegs.GPAPUD.bit.GPIO12 = DISABLE_PULLUP;

	GpioCtrlRegs.GPAMUX1.bit.GPIO13 = GPIO_MUX;
	GpioCtrlRegs.GPADIR.bit.GPIO13 = GPIO_IN;
	GpioCtrlRegs.GPAPUD.bit.GPIO13 = DISABLE_PULLUP;

	GpioCtrlRegs.GPAMUX1.bit.GPIO14 = GPIO_MUX;
	GpioCtrlRegs.GPADIR.bit.GPIO14 = GPIO_IN;
	GpioCtrlRegs.GPAPUD.bit.GPIO14 = DISABLE_PULLUP;

	GpioCtrlRegs.GPAMUX1.bit.GPIO15 = GPIO_MUX;
	GpioCtrlRegs.GPADIR.bit.GPIO15 = GPIO_IN;
	GpioCtrlRegs.GPAPUD.bit.GPIO15 = DISABLE_PULLUP;

	// Toggle XINT1 on GPIO 12
	/*
	GpioIntRegs.GPIOXINT1SEL.all = 12;
	XIntruptRegs.XINT1CR.bit.POLARITY = 1;
	XIntruptRegs.XINT1CR.bit.ENABLE = 1;
	*/
	EDIS;
}

void HWI_Service_TCA9555(void)
{
	Event_post(I2C_Event, I2C_NEW_DATA_EVENT);
}

Void HWI_Switch_Service()
{
	if (GpioDataRegs.GPADAT.bit.GPIO12 == 1)
	{
		/*
		GpioDataRegs.GPASET.bit.GPIO9 = 1;
		GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
		GpioDataRegs.GPBCLEAR.bit.GPIO41 = 1;
		GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;
		*/
		Event_post(StateChangeEvent, WAIT_EVENT);
	}
	if (GpioDataRegs.GPADAT.bit.GPIO12 == 1 &&
			GpioDataRegs.GPADAT.bit.GPIO13 == 1)
	{
		/*
		GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
		GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
		GpioDataRegs.GPBCLEAR.bit.GPIO41 = 1;
		GpioDataRegs.GPASET.bit.GPIO11 = 1;
		*/
		Event_post(StateChangeEvent, CHARGE_EVENT);
	}
	if (GpioDataRegs.GPADAT.bit.GPIO12 == 1 &&
			GpioDataRegs.GPADAT.bit.GPIO14 == 1)
	{
		/*
		GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
		GpioDataRegs.GPBSET.bit.GPIO34 = 1;
		GpioDataRegs.GPBCLEAR.bit.GPIO41 = 1;
		GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;
		*/
		Event_post(StateChangeEvent, BALANCE_EVENT);
	}
	if (GpioDataRegs.GPADAT.bit.GPIO12 == 1 &&
			GpioDataRegs.GPADAT.bit.GPIO15 == 1)
	{
		/*
		GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
		GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
		GpioDataRegs.GPBSET.bit.GPIO41 = 1;
		GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;
		*/
		Event_post(StateChangeEvent, CHARGE_BALANCE_EVENT);
	}
}

