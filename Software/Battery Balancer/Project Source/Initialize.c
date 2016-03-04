/*
 * Initialize.c
 *
 *  Created on: Jan 23, 2016
 *      Author: Sean Harrington
 */




//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------

#include <CellStatus.h>
#include "Initialize.h"
#include "GPIO.h"
#include "CAN.h"
#include "I2C_Coms.h"
#include "Timer.h"
#include "SPI.h"
#include "State.h"

//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------
/*
extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadEnd;
extern Uint16 RamfuncsRunStart;
extern Uint16 RamfuncsLoadSize;
*/

cell_voltage Cell_Voltages[CELLS_IN_SERIES];

extern cell_t cells[CELLS_IN_SERIES];

extern struct CPUTIMER_VARS CpuTimer0;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

// Value used to set GPxDIR register as GPIO output
#define GPIO_OUT 	1

// Value used to set GPxDIR register as GPIO input
#define GPIO_IN		0

// Value used to set GPxMUXy. Using this value with set the pin as a GPIO
#define GPIO_MUX	0


//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Private (Internal) function definitions
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------
void HardwareInit()
{
	/*InitSysCtrl();
	memcpy(&RamfuncsRunStart,&RamfuncsLoadStart,(unsigned long)&RamfuncsLoadSize);
	InitFlash();
	*/


	Gpio_Init();
	// @todo: I2C setup

	// @todo: CAN setup
	CAN_Init();


}

Void SoftwareInit()
{
	InitializeState();

	Timer_Init();

	I2C_Init();
	I2C_SetPortOutput(PORT_0, START_INDICATOR);
	I2C_SendOutput();
	SPI_Init();

	/// Initialize the cells to a known state
	Uint16 i;
	for (i = 0; i < CELLS_IN_SERIES; i++)
	{
		CellStatus_InitCell(&cells[i]);
	}
}
