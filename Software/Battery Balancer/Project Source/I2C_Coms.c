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

// Possible I2C interrupt causes
typedef enum {
	NONE = 0,
	ARB_LOST = 1,
	NOACK = 2,
	REGS_READY = 3,
	RX_DATA_READY = 4,
	TX_DATA_READY = 5,
	STOP = 6,
	ADDR_AS_SLAVE = 7
} i2c_intcodes;

// GPIO Expander Port 0 pins (bitfield)
typedef enum {
	ERROR_INDICATOR = 1,
	STOP_INDICATOR = 4,
	START_INDICATOR = 64
} expander_sector_1;

// GPIO Expander Port 1 pins (bitfield)
typedef enum {
	CHARGE_LED_GREEN = 1,
	CHARGE_LED_YELLOW = 2,
	BALANCE_LED_GREEN = 4,
	BALANCE_LED_YELLOW = 8,
	ALARM = 64
} expander_sector_2;


//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

static Bool mGpioExpanderIntVal;
static Uint8 mCurrentState;
static Uint8 mCurrentRegisterAddr;

static Uint8 mPortOutput0;
static Uint8 mPortOutput1;

typedef enum {
	I2C_NOT_IN_PROGRESS = 0,
	I2C_SENDING_READ = 1,
	I2C_DATA_READY = 3,
	I2C_SENDING_WRITE = 4,
} i2c_states;

//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

static void I2C_ReadRegister(Uint8 address);

void I2C_ExpanderInterrupt(void);

//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------

Void I2C_Init()
{
	mGpioExpanderIntVal = TRUE;		// Active Low
	mCurrentState = I2C_NOT_IN_PROGRESS;
	mCurrentRegisterAddr = INPUT_PORT_0;

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

	/*
	GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 26;		// XINT1 is GPIO17
	XIntruptRegs.XINT1CR.bit.POLARITY = 0;		// falling edge
	XIntruptRegs.XINT1CR.bit.ENABLE = 1;		// enable XINT1
	*/
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

	// Post complete event now that module is initialized.
	// Outside module just needs to post I2C_SEND_EVENT now.
	Event_post(I2C_SendEvent, I2C_COMPLETE_EVENT);

	return;
}

// todo: Set up HWI for this in cfg
void I2C_ExpanderInterrupt(void)
{
	mCurrentRegisterAddr = INPUT_PORT_0;
	mGpioExpanderIntVal = FALSE;

	I2C_ReadRegister(INPUT_PORT_0);
}

Bool I2C_SendOutputTask()
{
	while(TRUE)
	{
		Event_pend(I2C_SendEvent, I2C_SEND_EVENT + I2C_COMPLETE_EVENT, Event_Id_NONE, BIOS_WAIT_FOREVER);

		mCurrentState = I2C_SENDING_WRITE;
		// Check port validity.
		// todo: Determine if needed after moving to event based I2C.
		if (I2caRegs.I2CMDR.bit.STP || I2caRegs.I2CSTR.bit.BB)
		{
			// Message still in flight
			// todo: No longer works with event based handling here.
			return FALSE;
		}

		// Send requested GPIO output for port 0 until slave confirms output is
		// as requested.
		do
		{
			I2C_WriteRegister(OUTPUT_PORT_0, mPortOutput0);
			// Use event here to pend until last transaction done
			Event_pend(I2C_SendEvent, Event_Id_NONE, I2C_SEND_PORT1_EVENT, BIOS_WAIT_FOREVER);

			// todo: Read address just assigned to make sure it matches up. If not
			// repeat send.
			I2C_ReadRegister(OUTPUT_PORT_0);

		} while (I2caRegs.I2CDRR != mPortOutput0);

		// Send requested GPIO output for port 1 until slave confirms output is
		// as requested.
		do
		{
			I2C_WriteRegister(OUTPUT_PORT_1, mPortOutput1);

			// Wait until TX buffer empty again (transaction complete).
			Event_pend(I2C_SendEvent, Event_Id_NONE, I2C_SEND_PORT1_EVENT, BIOS_WAIT_FOREVER);

			I2C_ReadRegister(OUTPUT_PORT_1);
		} while (I2caRegs.I2CDRR != mPortOutput1);

		mCurrentState = I2C_NOT_IN_PROGRESS;
		Event_post(I2C_SendEvent, I2C_COMPLETE_EVENT);
	}
}

static void I2C_ReadRegister(Uint8 address)
{
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
			mCurrentState = I2C_NOT_IN_PROGRESS;
			return;
		}
		else if ((mCurrentState & I2C_SENDING_WRITE) &&
				(interruptSource == TX_DATA_READY))
		{
			// This may not work if the post saves even if pend isn't waiting on
			// it. Example: Sends 1 byte, sends post, I2C_SendOutput continues,
			// this interrupt triggers again and posts again. As soon as
			// I2C_SendOutput is called again, it may immediately skip the post
			Event_post(I2C_SendEvent, I2C_SEND_PORT1_EVENT);
		}
		// todo: Add check as well for what caused the interrupt
		else if ((mCurrentState & I2C_SENDING_READ) &&
				(interruptSource == REGS_READY))
		{
			I2caRegs.I2CCNT = 1;
			//I2caRegs.I2CDXR = SLAVE_ADDRESS;

			// Master Receiver, STOP, START, NACK
			I2caRegs.I2CMDR.all = 0xAC20;
		}
		else if ((mCurrentState & I2C_DATA_READY) &&
				(interruptSource == RX_DATA_READY))
		{
			I2caRegs.I2CDRR;

			if (mGpioExpanderIntVal = GpioDataRegs.GPADAT.bit.GPIO26)
			{
				// Another read is required for the other register
				// todo: Figure out if to call in this HWI or call outside
				I2C_ReadRegister(INPUT_PORT_1);
			}
			mCurrentState = I2C_NOT_IN_PROGRESS;
		}
	} while (interruptSource != NONE);
}


