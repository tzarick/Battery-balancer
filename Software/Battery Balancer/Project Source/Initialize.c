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

void Cell_Voltages_Init(void);

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
	I2C_Init();
	// @todo: CAN setup
	CAN_Init();
}

Void SoftwareInit()
{
	Cell_Voltages_Init();
	Timer_Init();
}


void Cell_Voltages_Init(void)
{
	int i;
	for (i = 0; i < CELLS_IN_SERIES; i++)
	{
		Cell_Voltages[i] = -1;	// Set to negative value until BIM updates
	}
}
