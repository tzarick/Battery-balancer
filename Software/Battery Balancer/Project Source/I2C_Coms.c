/*
 * I2C_Coms.c
 *
 *  Created on: Jan 23, 2016
 *      Author: Sean Harrington
 */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------

#include "I2C_Coms.h"
#include "Events.h"
#include "GPIO.h"
#include "State.h"

//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

// IC2 Address of TCA9555 defined by hardware switches
#define SLAVE_ADDRESS			0x20

// TCA9555 Register Addresses
#define INPUT_PORT_0			0x0
#define INPUT_PORT_1			0x1
#define OUTPUT_PORT_0			0x2
#define OUTPUT_PORT_1			0x3
#define POLARITY_INV_PORT_0		0x4
#define POLARITY_INV_PORT_1		0x5
#define CONFIG_PORT_0			0x6
#define CONFIG_PORT_1			0x7

/// Macros to toggle outputs on the TCA9555 Expander On/Off
// Active High outputs
#define ERROR_INDICATOR_ON 		(mPortOutput0 |= ERROR_INDICATOR)
#define ERROR_INDICATOR_OFF 	(mPortOutput0 &= !ERROR_INDICATOR)
#define STOP_INDICATOR_ON 		(mPortOutput0 |= STOP_INDICATOR)
#define STOP_INDICATOR_OFF 		(mPortOutput0 &= !STOP_INDICATOR)
#define START_INDICATOR_ON 		(mPortOutput0 |= START_INDICATOR)
#define START_INDICATOR_OFF		(mPortOutput0 &= !START_INDICATOR)
#define ALARM_ON				(mPortOutput1 |= ALARM)
#define	ALARM_OFF				(mPortOutput1 &= !ALARM)
// Active Low Outputs
#define CHARGE_LED_GREEN_ON 	(mPortOutput1 &= !CHARGE_LED_GREEN)
#define CHARGE_LED_GREEN_OFF 	(mPortOutput1 |= CHARGE_LED_GREEN)
#define CHARGE_LED_YELLOW_ON 	(mPortOutput1 &= !CHARGE_LED_YELLOW)
#define CHARGE_LED_YELLOW_OFF 	(mPortOutput1 |= CHARGE_LED_YELLOW)
#define BALANCE_LED_GREEN_ON 	(mPortOutput1 &= !BALANCE_LED_GREEN)
#define BALANCE_LED_GREEN_OFF 	(mPortOutput1 |= BALANCE_LED_GREEN)
#define BALANCE_LED_YELLOW_ON 	(mPortOutput1 &= !BALANCE_LED_YELLOW)
#define BALANCE_LED_YELLOW_OFF 	(mPortOutput1 |= BALANCE_LED_YELLOW)

#define TCA9555_INT_LOW 		(0)

// Possible I2C interrupt causes
typedef enum {
	NONE 			= 0,
	ARB_LOST 		= 1,
	NOACK 			= 2,
	REGS_READY 		= 3,
	RX_DATA_READY 	= 4,
	TX_DATA_READY 	= 5,
	STOP 			= 6,
	ADDR_AS_SLAVE 	= 7
} i2c_intcodes;

//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

static Uint8 mCurrentState;

static Uint8 mLastDataReceived;

static Uint8 mPortOutput0;
static Uint8 mPortOutput1;

static Uint8 mPortInput0;
static Uint8 mPortInput1;

typedef enum {
	I2C_NOT_IN_PROGRESS = 0,
	I2C_SENDING_READ = 1,
	I2C_DATA_READY = 3,
	I2C_SENDING_WRITE = 4,
	I2C_TXN_ERROR = 5
} i2c_states;

//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

static void I2C_ConfigureTCA9555(void);

static void I2C_WriteRegister(Uint8 address, Uint8 data);

static void I2C_ReadRegister(Uint8 address);


//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------

Void I2C_Init()
{
	mCurrentState = I2C_NOT_IN_PROGRESS;
	mLastDataReceived = 0;

	// todo: Decide if you actually want this in GPIO.c
	mPortOutput0 = STOP_INDICATOR;
	mPortOutput1 = CHARGE_LED_GREEN + CHARGE_LED_YELLOW +
			BALANCE_LED_GREEN + BALANCE_LED_YELLOW;

	EALLOW;

	// Enable I2C-A on GPIO32 - GPIO33
	GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;   // Enable pullup on GPIO32
	GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;  // GPIO32 = SDAA
	GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3; // Asynch input
	GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;   // Enable pullup on GPIO33
	GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3; // Asynch input
	GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;  // GPIO33 = SCLA

	// Setup GP0 as interrupt source XINT0
	GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;		// use GPIO17 as GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO26 = 0;		// GPIO17 is input
	GpioCtrlRegs.GPAPUD.bit.GPIO26 = 0;		//Enable pullup


	GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 26;		// XINT1 is GPIO17
	XIntruptRegs.XINT1CR.bit.POLARITY = 0;		// falling edge
	XIntruptRegs.XINT1CR.bit.ENABLE = 1;		// enable XINT1

	// todo Confirm. This address means all pins are tied low.

	//---------------------------------------------------------
	// Clock setup 400 KHz
	//	(1/400 KHz) = (I2CPSC + 1)[(5 + CLKL) + (5 + (CLKH)]
	//					------------------------------------
	//						SYSCLKOUT Freq (60,000,000)
	//---------------------------------------------------------
	I2caRegs.I2CPSC.all = 4;
	I2caRegs.I2CCLKL = 5;
	I2caRegs.I2CCLKH = 5;

	// Configure interrupt setup
	I2caRegs.I2CIER.all = 0x1F;
	// Enable I2C module
	I2caRegs.I2CMDR.all = 0x20;
	// Check FIFO configuration
	I2caRegs.I2CFFTX.all = 0x6000;

	EDIS;

	I2C_ConfigureTCA9555();

	// Post complete event now that module is initialized.
	// Outside module just needs to post I2C_SEND_EVENT now.
	Event_post(I2C_Event, I2C_COMPLETE_EVENT);

	return;
}

Uint8 I2C_GetPortInput(tca9555_ports port)
{
	if (port == PORT_0)
	{
		return mPortInput0;
	}
	else if(port == PORT_1)
	{
		return mPortInput1;
	}
	else
	{
		SetState(ERROR);
		return 0;
	}
}

void I2C_SetPortOutput(tca9555_ports port, Uint8 data)
{
	if (port == PORT_0)
	{
		mPortOutput0 = data;
	}
	else if (port == PORT_1)
	{
		mPortOutput1 = data;
	}
	else
	{
		SetState(ERROR);
	}
}

static void I2C_ConfigureTCA9555(void)
{
	Uint8 inputs = 	ERROR_BUTTON | STOP_BUTTON | SWITCH_CHARGE_AND_BALANCE |
					SWITCH_CHARGE | START_BUTTON;
	Uint8 events;

	// Set inputs and outputs for Port 0 on the TCA9555
	do
	{
		I2C_WriteRegister(CONFIG_PORT_0, inputs);
		// Use event here to pend until last transaction done
		events = Event_pend(I2C_Event, Event_Id_NONE,
				 	 	   (I2C_SEND_DONE_EVENT + I2C_ERROR_EVENT),
						   BIOS_WAIT_FOREVER);
		if (events & I2C_SEND_DONE_EVENT) {
			I2C_ReadRegister(CONFIG_PORT_0);
			Event_pend(I2C_Event, Event_Id_NONE,(I2C_READ_EVENT + I2C_ERROR_EVENT),
					   BIOS_WAIT_FOREVER);
		}
	} while ((mLastDataReceived != inputs) || (mCurrentState == I2C_TXN_ERROR));

	// Read what the inputs are on the newly configuration Port 0
	do
	{
		/// todo: Maybe read twice and compare if we really want to make sure noise
		/// doesn't affect system state negatively on startup.
		I2C_ReadRegister(INPUT_PORT_0);
		events = Event_pend(I2C_Event, Event_Id_NONE,
				 	 	   (I2C_READ_EVENT + I2C_ERROR_EVENT),
						   BIOS_WAIT_FOREVER);
	} while (mCurrentState == I2C_TXN_ERROR);

	// Set internally the input states of TCA9555 on Port 0
	mPortInput0 = mLastDataReceived;

	inputs = LCD_UP_BUTTON | LCD_DOWN_BUTTON;

	// Set inputs and outputs for Port 1 on the TCA9555
	do
	{
		I2C_WriteRegister(CONFIG_PORT_1, inputs);
		// Use event here to pend until last transaction done
		events = Event_pend(I2C_Event, Event_Id_NONE,
						   (I2C_SEND_DONE_EVENT + I2C_ERROR_EVENT),
						   BIOS_WAIT_FOREVER);
		if (events & I2C_SEND_DONE_EVENT) {
			I2C_ReadRegister(CONFIG_PORT_1);
			Event_pend(I2C_Event, Event_Id_NONE,(I2C_READ_EVENT + I2C_ERROR_EVENT),
					   BIOS_WAIT_FOREVER);
		}
	} while ((mLastDataReceived != inputs) || (mCurrentState == I2C_TXN_ERROR));


	// Read what the inputs are on the newly configuration Port 1
	do
	{
		/// todo: Maybe read twice and compare if we really want to make sure noise
		/// doesn't affect system state negatively on startup.
		I2C_ReadRegister(INPUT_PORT_1);
		events = Event_pend(I2C_Event, Event_Id_NONE,
				 	 	   (I2C_READ_EVENT + I2C_ERROR_EVENT),
						   BIOS_WAIT_FOREVER);
	} while (mCurrentState == I2C_TXN_ERROR);

	// Set internally the input states of TCA9555 on Port 1
	mPortInput1 = mLastDataReceived;
}

void I2C_UpdateInputTask()
{
	Uint8 event;
	while(TRUE)
	{
		/// Wait for interrupt from TCA9555 (see HWI_Service_TCA9555)
		Event_pend(I2C_Event, Event_Id_NONE, I2C_NEW_DATA_EVENT, BIOS_WAIT_FOREVER);

		mCurrentState = I2C_SENDING_READ;

		/// Attempt to read the register
		do
		{
			I2C_ReadRegister(INPUT_PORT_0);

			/// Wait until event says data is received.
			event = Event_pend(I2C_Event, Event_Id_NONE,
					  (I2C_READ_EVENT + I2C_ERROR_EVENT), BIOS_WAIT_FOREVER);
			if (event & I2C_READ_EVENT)
			{
				mPortInput0 = mLastDataReceived;
			}
		} while (mCurrentState == I2C_TXN_ERROR);

		/// Check if the interrupt pin has gone back high
		/// todo: Fix mGpioExpanderIntVal
		if (TCA9555_INT_LOW == GpioDataRegs.GPADAT.bit.GPIO26)
		{
			// Another read is required for the other register
			// todo: Figure out if to call in this HWI or call outside
			do {
				I2C_ReadRegister(INPUT_PORT_1);
				event = Event_pend(I2C_Event, Event_Id_NONE,
						  (I2C_READ_EVENT + I2C_ERROR_EVENT), BIOS_WAIT_FOREVER);
				if (event & I2C_READ_EVENT)
				{
					mPortInput1 = mLastDataReceived;
				}
			}
			while (mCurrentState == I2C_TXN_ERROR);
		}
		mCurrentState = I2C_NOT_IN_PROGRESS;
	}
}

Bool I2C_SendOutputTask()
{
	Uint8 events;
	while(TRUE)
	{
		Event_pend(I2C_Event, I2C_SEND_EVENT + I2C_COMPLETE_EVENT, Event_Id_NONE,
				   BIOS_WAIT_FOREVER);

		// Send requested GPIO output for port 0 until slave confirms output is
		do
		{
			I2C_WriteRegister(OUTPUT_PORT_0, mPortOutput0);
			// Use event here to pend until last transaction done
			events = Event_pend(I2C_Event, Event_Id_NONE,
					 	 	   (I2C_SEND_DONE_EVENT + I2C_ERROR_EVENT),
							   BIOS_WAIT_FOREVER);

			// If first send successful, proceed with read.
			if (events & I2C_SEND_DONE_EVENT) {
				I2C_ReadRegister(OUTPUT_PORT_0);
				Event_pend(I2C_Event, Event_Id_NONE,
						  (I2C_READ_EVENT + I2C_ERROR_EVENT), BIOS_WAIT_FOREVER);
			}
			// Send failed. While loop will catch and try again
			else if (events & I2C_ERROR_EVENT) {
				// Nothing to do?
			}
			// todo: Read address just assigned to make sure it matches up. If not
			// repeat send.
		} while ((mLastDataReceived != mPortOutput0) || (mCurrentState == I2C_TXN_ERROR));

		// Send requested GPIO output for port 1 until slave confirms output is
		// as requested.
		do
		{
			I2C_WriteRegister(OUTPUT_PORT_1, mPortOutput1);

			// Wait until TX buffer empty again (transaction complete).
			events = Event_pend(I2C_Event, Event_Id_NONE,
					 	 	   (I2C_SEND_DONE_EVENT + I2C_ERROR_EVENT),
							   BIOS_WAIT_FOREVER);

			// If first send successful, proceed with read.
			if (events & I2C_SEND_DONE_EVENT) {
				I2C_ReadRegister(OUTPUT_PORT_1);
				Event_pend(I2C_Event, Event_Id_NONE,
						  (I2C_READ_EVENT+I2C_ERROR_EVENT), BIOS_WAIT_FOREVER);
			}
			// Send failed. While loop will catch and try again
			else if (events & I2C_ERROR_EVENT) {
				// Nothing to do?
			}
		} while ((mLastDataReceived != mPortOutput1) || (mCurrentState == I2C_TXN_ERROR));

		mCurrentState = I2C_NOT_IN_PROGRESS;
		Event_post(I2C_Event, I2C_COMPLETE_EVENT);
	}
}

static void I2C_ReadRegister(Uint8 address)
{
	mCurrentState = I2C_SENDING_READ;
	/// todo: Determine if needed after moving to event based I2C
	if (I2caRegs.I2CMDR.bit.STP || I2caRegs.I2CSTR.bit.BB)
	{
		// Message still in flight
		return;
	}

	I2caRegs.I2CCNT = 1;
	I2caRegs.I2CDXR = address;

	// Send with bits: Start, Mst, Trx, IRS
	I2caRegs.I2CEMDR.all = 0x2620;
}

static void I2C_WriteRegister(Uint8 address, Uint8 data)
{
	mCurrentState = I2C_SENDING_WRITE;
	I2caRegs.I2CCNT = 2;
	//I2caRegs.I2CDXR = SLAVE_ADDRESS_WRITE;
	I2caRegs.I2CDXR = address;
	I2caRegs.I2CDXR = data;

	// Master transmitter, START, STOP
	I2caRegs.I2CMDR.all = 0x2E20;

}

void I2C_Interrupt(void)
{
	Uint16 interruptSource;
	do
	{
		// Each read of INTCODE clears the flag the caused the interrupt
		// and loads in the next lower priority interrupt code if pending
		interruptSource = I2caRegs.I2CISRC.bit.INTCODE;

		if ((interruptSource == ARB_LOST) || (interruptSource == NOACK))
		{
			// todo: Find out if something needs to be reset
			mCurrentState = I2C_TXN_ERROR;
			Event_post(I2C_Event, I2C_ERROR_EVENT);
			return;
		}
		else if (interruptSource == TX_DATA_READY)
		{
			if (mCurrentState & I2C_SENDING_WRITE)
			{
				Event_post(I2C_Event, I2C_SEND_DONE_EVENT);
			}
			else if (mCurrentState & I2C_SENDING_READ)
			{
				I2caRegs.I2CCNT = 1;
				//I2caRegs.I2CDXR = SLAVE_ADDRESS;
				// Master Receiver, STOP, START, NACK
				I2caRegs.I2CMDR.all = 0xAC20;
			}
		}
		else if ((mCurrentState & I2C_DATA_READY) &&
				(interruptSource == RX_DATA_READY))
		{
			mLastDataReceived = I2caRegs.I2CDRR;
			Event_post(I2C_Event, I2C_READ_EVENT);
		}
	} while (interruptSource != NONE);
}


