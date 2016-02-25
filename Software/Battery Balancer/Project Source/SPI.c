/*
 * SPI.c
 *
 *  Created on: Feb 6, 2016
 *      Author: Sean Harrington
 */

#include "SPI.h"
#include "Error.h"

//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

#define SPI_CS_LOW		GpioDataRegs.GPACLEAR.bit.GPIO15 = 1
#define SPI_CS_HIGH		GpioDataRegs.GPASET.bit.GPIO15 = 1

#define NO_DEVICE		0

#define LCD_MAX_CHARS_PER_LINE		26
#define DRV8860_IN_SERIES			17

#define RECEIVE_BUFFER_LENGTH		(DRV8860_IN_SERIES)
#define LCD_TX_BUFFER_LENGTH		(LCD_MAX_CHARS_PER_LINE)
#define RELAY_TX_BUFFER_LENGTH		(DRV8860_IN_SERIES)

#define PE		1

//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

static enum
{
	RX_OVERRUN 	= 128,		//< Triggered when RX missed
	SPI_READY	= 64,		//< Triggered when done receiving or sending
	TX_FULL		= 32		//< Triggered when SPITXBUF moves to SPIDAT
} spi_int_bitmask;

static Uint8 mReceiveBuffer[DRV8860_IN_SERIES];

static Uint16 mRxBufferSize;

// todo: Determine maximum byte array to talk to LCD
static Uint8 mLcdTxBuffer[LCD_TX_BUFFER_LENGTH];
static Uint8 mRelayTxBuffer[RELAY_TX_BUFFER_LENGTH];

static Uint16 mLcdTxIndex;
static Uint16 mRelayTxIndex;

static Uint8 mDeviceInUse;

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
#ifdef PE


#endif

	GpioDataRegs.GPADAT.bit.GPIO15 = 1;			//ensure CS is high
	GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;         // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;          // output
	GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 0;        //Synch to SYSCLKOUT only
	GpioCtrlRegs.GPAPUD.bit.GPIO15 = 1;          //disable pull up
	GpioDataRegs.GPADAT.bit.GPIO15 = 1;			//ensure CS is high


	GpioCtrlRegs.GPAPUD.bit.GPIO24 = 0;     // Enable pull-up on GPIO24 (SPISIMOB)
#ifdef PE
	GpioCtrlRegs.GPAPUD.bit.GPIO25 = 0;
	GpioCtrlRegs.GPAPUD.bit.GPIO26 = 0;
#else
	GpioCtrlRegs.GPAPUD.bit.GPIO13 = 0;     // Enable pull-up on GPIO13 (SPISOMIB)
	GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0;     // Enable pull-up on GPIO14 (SPICLKB)
#endif

	GpioCtrlRegs.GPAQSEL2.bit.GPIO24 = 3;   // Asynch input GPIO24 (SPISIMOB)
#ifdef PE
	GpioCtrlRegs.GPAQSEL2.bit.GPIO25 = 3;
	GpioCtrlRegs.GPAQSEL2.bit.GPIO26 = 3;
#else
	GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = 3;   // Asynch input GPIO13 (SPISOMIB)
	GpioCtrlRegs.GPAQSEL1.bit.GPIO14 = 3;   // Asynch input GPIO14 (SPICLKB)
#endif


	GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 3;// Configure GPIO24 as SPISIMOB
#ifdef PE
	GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 3;
	GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 3;
#else
	GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 3;    // Configure GPIO13 as SPISOMIB
	GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 3;    // Configure GPIO14 as SPICLKB
#endif
	EDIS;

	SpibRegs.SPICCR.bit.SPISWRESET = 0;     // Reset SPI
	SpibRegs.SPICCR.all = 0x0047;           //8-bit no loopback

	/// Master mode, TALK enabled, Interrupt Enable
	SpibRegs.SPICTL.all = 0x0007;

	// Reference: Hardware/Front-Panel/Calculations
	// Requires LSPCLK of SYSCLKOUT/6
	SpibRegs.SPIBRR = 86;

	// SPIFFENA OFF
	SpibRegs.SPIFFTX.all = 0xA040;
	SpibRegs.SPIFFRX.all = 0x8000;
    SpibRegs.SPIPRI.bit.FREE = 1;           // Set so breakpoints don't disturb xmission

	SpibRegs.SPICCR.bit.SPISWRESET = 1;
}

error_t SPI_PushToQueue(UInt8 output, spi_device_t device)
{

	if ((mLcdTxIndex >= LCD_TX_BUFFER_LENGTH) || (mRelayTxIndex >= RELAY_TX_BUFFER_LENGTH))
	{
		return errSpiTxBufError;
	}

	switch (device)
	{
		case LCD:
		{
			mLcdTxBuffer[mLcdTxIndex++] = output;
			break;
		}
		case RELAYS:
		{
			mRelayTxBuffer[mRelayTxIndex++] = output;
			break;
		}
	}

	return errNone;
}

error_t SPI_PopFromQueue(Uint8 * queue_item)
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
}

error_t SPI_SendTx(spi_device_t device)
{
	static Uint16 lcdStartIndex = 0;
	static Uint16 relayStartIndex = 0;

	// Check if device already sending. If so, return error.
	if (mDeviceInUse != NO_DEVICE)
	{
		return errSpiDeviceSwitchError;
	}

	switch (device)
	{
		case LCD:
		{
			// If queue end index is 0, queue is empty so abort.
			if (mLcdTxIndex == 0)
			{
				return errSpiTxEmptyError;
			}

			SpibRegs.SPITXBUF = ((Uint16)(mLcdTxBuffer[lcdStartIndex++])) << 8;
			mDeviceInUse = LCD;
			// Change mode to LCD mode.

			if (lcdStartIndex >= mLcdTxIndex)
			{
				// Buffer empty. Reset indexes
				lcdStartIndex = 0;
				mLcdTxIndex = 0;
			}
			break;
		}
		case RELAYS:
		{
			// If queue end index is 0, queue is empty so abort.
			if (mRelayTxIndex == 0)
			{
				return errSpiTxEmptyError;
			}

			SpibRegs.SPITXBUF = ((Uint16)(mRelayTxBuffer[relayStartIndex++])) << 8;
			mDeviceInUse = RELAYS;

			if (relayStartIndex >= mLcdTxIndex)
			{
				relayStartIndex = 0;
				mLcdTxIndex = 0;
			}
			break;
		}
	}
}

void SPI_HandleInterrupt(void)
{
	switch (SpibRegs.SPISTS.all)
	{
		case RX_OVERRUN:
		{
			break;
		}
		case SPI_READY:
		{

			break;
		}
		case TX_FULL:
		{
			break;
		}
	}
}
