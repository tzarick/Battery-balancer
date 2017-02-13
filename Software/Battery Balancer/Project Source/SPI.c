/*
 * SPI.c
 *
 *  Created on: Feb 6, 2016
 *      Author: Sean Harrington
 */

#include "SPI.h"
#include "Error.h"
#include "Events.h"
#include "Timer.h"

//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------

#define DELAY_US(A)  DSP28x_usDelay(((((long double) A * 1000.0L) / (long double)CPU_RATE) - 9.0L) / 5.0L)

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

#ifdef PE_BOARD
#define SPI_CS_LOW		GpioDataRegs.GPACLEAR.bit.GPIO11 = 1
#define SPI_CS_HIGH		GpioDataRegs.GPASET.bit.GPIO11 = 1
#else
#define SPI_CS_LOW		GpioDataRegs.GPACLEAR.bit.GPIO15 = 1
#define SPI_CS_HIGH		GpioDataRegs.GPASET.bit.GPIO15 = 1
#endif

#define NO_DEVICE		0

#define LCD_MAX_CHARS_PER_LINE		26

#define RECEIVE_BUFFER_LENGTH		(DRV8860_IN_SERIES)
#define LCD_TX_BUFFER_LENGTH		(LCD_MAX_CHARS_PER_LINE)
#define RELAY_TX_BUFFER_LENGTH		(DRV8860_IN_SERIES)



//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

static enum
{
	RX_OVERRUN 	= 128,		//< Triggered when RX missed
	SPI_READY	= 64,		//< Triggered when done receiving or sending
	TX_FULL		= 32		//< Triggered when SPITXBUF moves to SPIDAT
} spi_int_bitmask;

static uint8_t mReceiveBuffer[DRV8860_IN_SERIES];

static Uint16 mRxBufferSize;

// todo: Determine maximum byte array to talk to LCD
static uint8_t mLcdTxBuffer[LCD_TX_BUFFER_LENGTH];
static uint8_t mRelayTxBuffer[RELAY_TX_BUFFER_LENGTH];

static Uint16 mLcdTxIndex;
static Uint16 mRelayTxIndex;

static uint8_t mDeviceInUse;

static timer_t mDelayTimer;

/// Current index to send in LCD transmit buffer
static Uint16 mLcdStartIndex = 0;

/// Current index to send in relay transmit buffer
static Uint16 mRelayStartIndex = 0;

//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------

void SPI_Init(void)
{
	mRxBufferSize = 0;
	mLcdTxIndex = 0;
	mRelayTxIndex = 0;
	mDeviceInUse = NO_DEVICE;

	/// todo: Setup GPIO registers here
	EALLOW;

	// SPI Chip Select line
#ifdef PE_BOARD
	GpioDataRegs.GPADAT.bit.GPIO11 = 1;			//ensure CS is high
	GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 0;         // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;          // output
	GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = 0;        //Synch to SYSCLKOUT only
	GpioCtrlRegs.GPAPUD.bit.GPIO11 = 1;          //disable pull up
	GpioDataRegs.GPADAT.bit.GPIO11 = 1;			//ensure CS is high
#else
	GpioDataRegs.GPADAT.bit.GPIO15 = 1;			//ensure CS is high
	GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;         // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;          // output
	GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 0;        //Synch to SYSCLKOUT only
	GpioCtrlRegs.GPAPUD.bit.GPIO15 = 1;          //disable pull up
	GpioDataRegs.GPADAT.bit.GPIO15 = 1;			//ensure CS is high
#endif

	GpioCtrlRegs.GPAPUD.bit.GPIO24 = 0;     // Enable pull-up on GPIO24 (SPISIMOB)
#ifdef PE_BOARD
	GpioCtrlRegs.GPAPUD.bit.GPIO25 = 0;
	GpioCtrlRegs.GPAPUD.bit.GPIO26 = 0;
#else
	GpioCtrlRegs.GPAPUD.bit.GPIO13 = 0;     // Enable pull-up on GPIO13 (SPISOMIB)
	GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0;     // Enable pull-up on GPIO14 (SPICLKB)
#endif

	//GpioCtrlRegs.GPAQSEL2.bit.GPIO24 = 3;   // Asynch input GPIO24 (SPISIMOB)
#ifdef PE_BOARD
	GpioCtrlRegs.GPAQSEL2.bit.GPIO25 = 3;
	//GpioCtrlRegs.GPAQSEL2.bit.GPIO26 = 3;
#else
	GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = 3;   // Asynch input GPIO13 (SPISOMIB)
	GpioCtrlRegs.GPAQSEL1.bit.GPIO14 = 3;   // Asynch input GPIO14 (SPICLKB)
#endif


	GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 3;// Configure GPIO24 as SPISIMOB
#ifdef PE_BOARD
	GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 3;
	GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 3;
#else
	GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 3;    // Configure GPIO13 as SPISOMIB
	GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 3;    // Configure GPIO14 as SPICLKB
#endif

	GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;
#ifdef PE_BOARD
	GpioCtrlRegs.GPADIR.bit.GPIO26 = 1;
#else
	GpioCtrlRegs.GPADIR.bit.GPIO14 = 1;
#endif

	EDIS;

	SpibRegs.SPICCR.bit.SPISWRESET = 0;     // Reset SPI
	SpibRegs.SPICCR.all = 0x0047;           //8-bit no loopback

	//SpibRegs.SPICCR.bit.SPILBK = 1;			// Loopback on temporarily for testing

	/// Master mode, TALK enabled, Interrupt Enable
	SpibRegs.SPICTL.all = 0x0007;

	// Reference: Hardware/Front-Panel/Calculations
	// Requires LSPCLK of SYSCLKOUT/6
	SpibRegs.SPIBRR = 86;

	// SPIFFENA ON
	SpibRegs.SPIFFTX.all = 0xE040;
	SpibRegs.SPIFFRX.all = 0x6061;
    //SpibRegs.SPIPRI.bit.FREE = 1;           // Set so breakpoints don't disturb xmission

	SpibRegs.SPICCR.bit.SPISWRESET = 1;

	Timer_Setup(&mDelayTimer, NULL);
}

error_t SPI_PushToQueue(uint8_t output, spi_device_t device)
{

	if ((mLcdTxIndex >= LCD_TX_BUFFER_LENGTH) || (mRelayTxIndex >= RELAY_TX_BUFFER_LENGTH))
	{
		return errSpiTxBufError;
	}

	switch (device)
	{
		case LCD:
		{
			mLcdTxBuffer[mLcdTxIndex] = output;
			mLcdTxIndex++;
			break;
		}
		case RELAYS:
		{
			mRelayTxBuffer[mRelayTxIndex] = output;
			mRelayTxIndex++;
			break;
		}
	}

	return errNone;
}

error_t SPI_PopFromQueue(uint8_t * queue_item)
{
	// ASSERT queue_item is valid memory!

	error_t retVal = errNone;

	if (mRxBufferSize >= DRV8860_IN_SERIES)
	{
		retVal = errSpiRxBufError;
	}
	else if (mRxBufferSize == 0)
	{
		retVal = errSpiRxEmptyError;
	}

	// todo: Change me! This implements a stack.
	*queue_item = mReceiveBuffer[mRxBufferSize--];
	return retVal;
}

error_t SPI_SendTx(spi_device_t device)
{

	error_t retVal = errNone;

	// Check if device already sending. If so, return error.
	/*
	if (mDeviceInUse != NO_DEVICE)
	{
		return errSpiDeviceSwitchError;
	}
	*/

	switch (device)
	{
		case LCD:
		{
			// If queue end index is 0, queue is empty so abort.
			if (mLcdTxIndex == 0)
			{
				retVal = errSpiTxEmptyError;
				return retVal;
			}
			SPI_CS_LOW;
			SpibRegs.SPITXBUF = ((Uint16)(mLcdTxBuffer[mLcdStartIndex])) << 8;
			mLcdStartIndex ++;
			mDeviceInUse = LCD;
			// Change mode to LCD mode.

			if (mLcdStartIndex >= mLcdTxIndex)
			{
				// Buffer empty. Reset indexes
				mLcdStartIndex = 0;
				mLcdTxIndex = 0;
				mDeviceInUse = NO_DEVICE;
			}
			break;
		}
		case RELAYS:
		{
			// If queue end index is 0, queue is empty so abort.
			if (mRelayTxIndex == 0)
			{
				retVal = errSpiTxEmptyError;
				return retVal;
			}
			SPI_CS_LOW;
			SpibRegs.SPITXBUF = ((Uint16)(mRelayTxBuffer[mRelayStartIndex])) << 8;
			mRelayStartIndex ++;
			mDeviceInUse = RELAYS;

			break;
		}
	}
	return retVal;
}

void SPI_DRV8860_GetFaults(uint16_t * faultArray, uint16_t arrayLength)
{
	Uint16 i = 0;
	EALLOW;

	GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0;    // Configure GPIO14 as GPIO

	//GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;

	GpioDataRegs.GPACLEAR.bit.GPIO14 = 1;
	//GpioDataRegs.GPACLEAR.bit.GPIO26 = 1;

	Timer_Start(&mDelayTimer, 2);
	while(mDelayTimer._timeElapsed != TRUE)
	{
		Timer_Update();
	}


	/// Inform the DRV8860 to read fault registers
	SPI_CS_LOW;

	Timer_Start(&mDelayTimer, 2);
	while(mDelayTimer._timeElapsed != TRUE)
	{
		Timer_Update();
	}

	SPI_CS_HIGH;

	Timer_Start(&mDelayTimer, 2);
	while(mDelayTimer._timeElapsed != TRUE)
	{
		Timer_Update();
	}

	GpioDataRegs.GPASET.bit.GPIO14 = 1;

	GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 3;    // Configure GPIO14 as SPICLKB

	//GpioDataRegs.GPASET.bit.GPIO26 = 1;

	for (i = 0; i < arrayLength; i++)
	{
		SpibRegs.SPITXBUF = 0;
	}

	//GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 3;
	EDIS;

}

void SPI_HandleInterrupt(void)
{
	uint16_t interruptSource = SpibRegs.SPIFFRX.bit.RXFFINT | SpibRegs.SPIFFRX.bit.RXFFOVF;

	mReceiveBuffer[mRxBufferSize] = SpibRegs.SPIRXBUF;
	SpibRegs.SPIFFRX.bit.RXFFINTCLR = 1;
	mRxBufferSize ++;

	if (mRelayStartIndex >= mRelayTxIndex)
	{
		SPI_CS_HIGH;
		mRelayStartIndex = 0;
		mRelayTxIndex = 0;
		mDeviceInUse = NO_DEVICE;
	}
	else
	{
		Event_post(SPI_Event, SPI_TX_READY_EVENT);
	}
}

void SPI_HandleEvent()
{
	UInt events;
	while (TRUE)
	{
		events = Event_pend(SPI_Event, Event_Id_NONE, (SPI_TX_READY_EVENT | SPI_DONE_EVENT), BIOS_WAIT_FOREVER);
		switch (events)
		{
			case SPI_TX_READY_EVENT:
			{
				SPI_SendTx(mDeviceInUse);
				break;
			}
			case SPI_DONE_EVENT:
			{
				break;
			}
		}
	}
}
