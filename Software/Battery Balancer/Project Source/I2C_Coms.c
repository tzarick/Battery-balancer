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
#include "Error.h"

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

#define WRITE_BUFFER_LEN		2

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

typedef enum {
	portOutput0,
	portOutput1,
	portInput0,
	portInput1,
	TOTAL_PORTS
} gpio_ports;

uint8_t localGpioStates[TOTAL_PORTS];

uint8_t tca9555GpioStates[TOTAL_PORTS];

//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

static gpio_ports mPortBeingRead;

static gpio_ports mPortBeingWritten;

static i2c_states mCurrentState;

static i2c_states mLastState;

static uint8_t mLastDataReceived;

static Bool mInitialized = FALSE;

static uint16_t mWriteIndex = 0;

static uint8_t mWriteBuffer[WRITE_BUFFER_LEN] = {0};

static Bool mNewInputs = FALSE;

static Bool mNewOutputs = FALSE;

//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

static void I2C_ConfigureTCA9555(void);

static void I2C_WriteRegister(uint8_t address, uint8_t data);

//static void I2C_ReadRegister(uint8_t address);


//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------

Void I2C_Init()
{
	mCurrentState = I2C_NOT_IN_PROGRESS;
	mLastDataReceived = 0;

	// todo: Decide if you actually want this in GPIO.c
	localGpioStates[portOutput0] = STOP_INDICATOR;
	localGpioStates[portOutput1] = CHARGE_LED_GREEN + CHARGE_LED_YELLOW +
			BALANCE_LED_GREEN + BALANCE_LED_YELLOW;

	EALLOW;

	// Enable I2C-A on GPIO32 - GPIO33


	GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;   // Enable pullup on GPIO32
	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 2;  // GPIO32 = SDAA
	GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3; // Asynch input
	GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;   // Enable pullup on GPIO33
	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 2;  // GPIO33 = SCLA
	GpioCtrlRegs.GPAQSEL2.bit.GPIO29 = 3; // Asynch input

	GpioCtrlRegs.GPAMUX2.bit.GPIO26 = GPIO_MUX;
	GpioCtrlRegs.GPADIR.bit.GPIO26 = GPIO_IN;
	//GpioCtrlRegs.GPAPUD.bit.GPIO26 = GPIO_PULLUP;

	GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 26;		// XINT1 is GPIO26
	XIntruptRegs.XINT1CR.bit.POLARITY = 0;		// falling edge
	XIntruptRegs.XINT1CR.bit.ENABLE = 1;		// enable XINT1

	// todo Confirm. This address means all pins are tied low.

	//---------------------------------------------------------
	// Clock setup 400 KHz
	//	(1/400 KHz) = (I2CPSC + 1)[(5 + CLKL) + (5 + (CLKH)]
	//					------------------------------------
	//						SYSCLKOUT Freq (60,000,000)
	//---------------------------------------------------------
	//I2C prescalar register
	I2caRegs.I2CPSC.all = 4;
	//I2C clock low-time divide register
	I2caRegs.I2CCLKL = 5;
	//I2C clock high-time divide reigster
	I2caRegs.I2CCLKH = 5;

	//I2caRegs.I2CSAR = 0x47;
	//I2C slave address reigster
	I2caRegs.I2CSAR = SLAVE_ADDRESS;
	//I2caRegs.I2COAR	= 0x01;

	// Configure interrupt setup
	I2caRegs.I2CIER.all = 0x3F;
	// Enable I2C module
	I2caRegs.I2CMDR.all = 0x20;
	// Check FIFO configuration
	//I2caRegs.I2CFFTX.all = 0x6020;

	EDIS;

	mPortBeingRead = portOutput0;
	mPortBeingWritten = portOutput0;
	I2C_ConfigureTCA9555();

	// Post complete event now that module is initialized.
	// Outside module just needs to post I2C_SEND_EVENT now.
	Event_post(I2C_Event, I2C_COMPLETE_EVENT);

	mInitialized = TRUE;
	return;
}

void I2C_Update(void)
{
	switch (mCurrentState)
	{
		case I2C_NOT_IN_PROGRESS:
		{
			// If TCA9555 has triggered interrupt, there's new input data to read
			if (mNewInputs)
			{
				mLastState = mCurrentState;
				mCurrentState = I2C_SENDING_READ;
				mPortBeingRead = portInput0;

				/// Attempt to read the register
				I2C_ReadRegister(INPUT_PORT_0);
			}

			// If internal GPIO outputs don't match those currently set on the TCA9555,
			// update them on the TCA9555.
			else if (localGpioStates[portOutput0] != tca9555GpioStates[portOutput0])
			{
				mLastState = mCurrentState;
				mCurrentState = I2C_SENDING_WRITE;
				mPortBeingWritten = portOutput0;
				I2C_WriteRegister(OUTPUT_PORT_0, localGpioStates[portOutput0]);
			}
			else if (localGpioStates[portOutput1] != tca9555GpioStates[portOutput1])
			{
				mLastState = mCurrentState;
				mCurrentState = I2C_SENDING_WRITE;
				mPortBeingWritten = portOutput1;
				I2C_WriteRegister(OUTPUT_PORT_1, localGpioStates[portOutput1]);
			}
			break;
		}
		case I2C_SENDING_READ:
		{
			break;
		}
		case I2C_SENDING_WRITE:
		{
			break;
		}
		case I2C_DATA_READY:
		{
			// todo: Oh come on... this is shit.
			if (TCA9555_INT_LOW == GpioDataRegs.GPADAT.bit.GPIO26)
			{
				mLastState = mCurrentState;
				mCurrentState = I2C_SENDING_READ;
				if (mPortBeingRead == portInput0)
				{
					mPortBeingRead = portInput1;
					I2C_ReadRegister(INPUT_PORT_1);
				}
				else
				{
					mPortBeingRead = portInput0;
					I2C_ReadRegister(INPUT_PORT_0);
				}
			}
			else
			{
				mNewInputs = FALSE;
				mLastState = mCurrentState;
				mCurrentState = I2C_NOT_IN_PROGRESS;
			}
			break;
		}
		case I2C_SEND_DONE:
		{
			// Check to make sure write sent correctly by reading
			mLastState = mCurrentState;
			mCurrentState = I2C_SENDING_READ;
			if (mPortBeingWritten == portOutput0)
			{
				mPortBeingRead = portOutput0;
				I2C_ReadRegister(OUTPUT_PORT_0);
			}
			else
			{
				mPortBeingRead = portOutput1;
				I2C_ReadRegister(OUTPUT_PORT_1);
			}
			break;
		}
	}

}

i2c_states I2C_GetState()
{
	ASSERT(mInitialized, errI2cNotInitialized);
	return mCurrentState;
}

uint8_t I2C_GetPortInput(tca9555_ports port)
{
	ASSERT(mInitialized, errI2cNotInitialized);

	if (port == PORT_0)
	{
		return tca9555GpioStates[portInput0];
	}
	else if(port == PORT_1)
	{
		return tca9555GpioStates[portInput1];
	}
	else
	{
		SetState(ERROR);
		return 0;
	}
}

void I2C_SetPortOutput(tca9555_ports port, uint8_t data)
{
	ASSERT(mInitialized, errI2cNotInitialized);
	if (port == PORT_0)
	{
		localGpioStates[portOutput0] = data;
	}
	else if (port == PORT_1)
	{
		localGpioStates[portOutput1]= data;
	}
	else
	{
		SetState(ERROR);
	}
}

static void I2C_ConfigureTCA9555(void)
{
	//uint8_t inputs = 	ERROR_BUTTON | STOP_BUTTON | SWITCH_CHARGE_AND_BALANCE |
	//				SWITCH_CHARGE | START_BUTTON;
	uint8_t inputs = START_BUTTON;
	uint8_t events;

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
	localGpioStates[portInput0] = mLastDataReceived;
	tca9555GpioStates[portInput0] = mLastDataReceived;

	//inputs = LCD_UP_BUTTON | LCD_DOWN_BUTTON;
	inputs = 0;
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
	localGpioStates[portInput1] = mLastDataReceived;
	tca9555GpioStates[portInput1] = mLastDataReceived;
}

void I2C_ReadRegister(uint8_t address)
{
	mCurrentState = I2C_SENDING_READ;
	/// todo: Determine if needed after moving to event based I2C
	if (I2caRegs.I2CMDR.bit.STP || I2caRegs.I2CSTR.bit.BB)
	{
		// Message still in flight
		return;
	}
	//while ( !(I2caRegs.I2CSTR.all & I2caRegs.I2CSTR.bit.ARDY));
	//I2C data count register
	I2caRegs.I2CCNT = 1;
	//I2C data transmit regsiter
	I2caRegs.I2CDXR = address;

	//I2C status register
	I2caRegs.I2CSTR.bit.RRDY = 1;
	// Send with bits: Start, Mst, Trx, IRS
	I2caRegs.I2CMDR.all = 0x2620;
}

static void I2C_WriteRegister(uint8_t address, uint8_t data)
{
	
	mCurrentState = I2C_SENDING_WRITE;
	//I2C data count register
	I2caRegs.I2CCNT = 2;
	//I2caRegs.I2CDXR = SLAVE_ADDRESS_WRITE;
	mWriteIndex = 0;

	mWriteBuffer[0] = address;
	mWriteBuffer[1] = data;
	//I2C data transmit register
	I2caRegs.I2CDXR = mWriteBuffer[mWriteIndex];
	mWriteIndex++;
	//I2caRegs.I2CDXR = data;

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
			//mCurrentState = I2C_TXN_ERROR;
			Event_post(I2C_Event, I2C_ERROR_EVENT);
			return;
		}
		else if (interruptSource == TX_DATA_READY)
		{

			if (mCurrentState & I2C_SENDING_WRITE)
			{
				if (mWriteIndex < WRITE_BUFFER_LEN)
				{
					//I2C data transmit register
					I2caRegs.I2CDXR = mWriteBuffer[mWriteIndex];
					mWriteIndex++;
				}
			}
			else if (mCurrentState & I2C_SENDING_READ)
			{
				while (I2caRegs.I2CSTR.bit.ARDY != 1);
				//I2C data count register
				I2caRegs.I2CCNT = 1;
				//I2C mode register
				I2caRegs.I2CMDR.all = 0x2420;
			}
			I2caRegs.I2CSTR.bit.XRDY = 1;
		}
		else if (interruptSource == RX_DATA_READY)
		{
			tca9555GpioStates[mPortBeingRead] = I2caRegs.I2CDRR;
			mLastDataReceived = tca9555GpioStates[mPortBeingRead];
			mLastState = mCurrentState;
			mCurrentState = I2C_DATA_READY;
			I2caRegs.I2CSTR.bit.RRDY = 1;
			Event_post(I2C_Event, I2C_READ_EVENT);
		}
		else if (interruptSource == STOP)
		{
			if (mWriteIndex == WRITE_BUFFER_LEN)
			{
				mLastState = mCurrentState;
				mCurrentState = I2C_SEND_DONE;
				Event_post(I2C_Event, I2C_SEND_DONE_EVENT);
			}
			//I2caRegs.I2CSTR.bit.SCD = 1;
		}
		else if (interruptSource == REGS_READY)
		{
			I2caRegs.I2CSTR.bit.ARDY = 1;
			if (mCurrentState & I2C_SENDING_READ)
			{

				//I2caRegs.I2CDXR = SLAVE_ADDRESS;
				// Master Receiver, STOP, START, NACK
				I2caRegs.I2CCNT = 1;
				I2caRegs.I2CSAR = SLAVE_ADDRESS;
				I2caRegs.I2CMDR.all = 0xAC20;
			}
		}

	} while (interruptSource != NONE);
}

void I2C_TCA9555Interrupt(void)
{
	mNewInputs = TRUE;
}
