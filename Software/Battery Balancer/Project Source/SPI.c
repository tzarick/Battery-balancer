/*
 * SPI.c
 *
 *  Created on: Feb 6, 2016
 *      Author: Sean Harrington
 */

#include "SPI.h"

//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

#define SPI_CS_LOW		GpioDataRegs.GPACLEAR.bit.GPIO15 = 1
#define SPI_CS_HIGH		GpioDataRegs.GPASET.bit.GPIO15 = 1

//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

#define LCD_MAX_CHARS_PER_LINE		26
#define DRV8860_IN_SERIES			17

static Uint8 mReceiveBuffer[DRV8860_IN_SERIES];

// todo: Determine maximum byte array to talk to LCD
static Uint8 mTransmitBuffer[LCD_MAX_CHARS_PER_LINE];

//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------

void SPI_Init(void)
{
	/// todo: Setup GPIO registers here
	EALLOW;

	// SPI Chip Select line
	GpioDataRegs.GPADAT.bit.GPIO15 = 1;			//ensure CS is high
	GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;         // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;          // output
	GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 0;        //Synch to SYSCLKOUT only
	GpioCtrlRegs.GPAPUD.bit.GPIO15 = 1;          //disable pull up
	GpioDataRegs.GPADAT.bit.GPIO15 = 1;			//ensure CS is high


	GpioCtrlRegs.GPAPUD.bit.GPIO24 = 0;     // Enable pull-up on GPIO24 (SPISIMOB)
	GpioCtrlRegs.GPAPUD.bit.GPIO13 = 0;     // Enable pull-up on GPIO13 (SPISOMIB)
	GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0;     // Enable pull-up on GPIO14 (SPICLKB)

	GpioCtrlRegs.GPAQSEL2.bit.GPIO24 = 3;   // Asynch input GPIO24 (SPISIMOB)
	GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = 3;   // Asynch input GPIO13 (SPISOMIB)
	GpioCtrlRegs.GPAQSEL1.bit.GPIO14 = 3;   // Asynch input GPIO14 (SPICLKB)

	GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 3;    // Configure GPIO24 as SPISIMOB
	GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 3;    // Configure GPIO13 as SPISOMIB
	GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 3;    // Configure GPIO14 as SPICLKB

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

	SpibRegs.SPICCR.bit.SPISWRESET = 1;
}
