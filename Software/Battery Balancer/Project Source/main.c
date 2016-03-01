/*
 *  ======== main.c ========
 */

//----------------------------------------
// BIOS header files
//----------------------------------------

#include "Common_Includes.h"

//-----------------------------------------
// ControlSuite Header Files
//-----------------------------------------

#include "Initialize.h"
#include "State.h"
#include "I2C_Coms.h"
#include "GPIO.h"
#include "CellStatus.h"
#include "SPI.h"

// @todo: Document events

static state lastState = WAIT;

cell_t cells[CELLS_IN_SERIES];


/*
 *  ======== taskFxn ========
 */
Void taskFxn(UArg a0, UArg a1)
{
    System_printf("enter taskFxn()\n");

    Task_sleep(10);

    System_printf("exit taskFxn()\n");

    System_flush(); /* force SysMin output to console */
}

/*
 *  ======== main ========
 */
Int main()
{ 
	// Init PLL, watchdog, periph clocks - see F2806x_SysCtrl.c file
	// Clock frequency set to 90 MHz - see F2806x_Examples.h
	InitSysCtrl();

	// Copy InitFlash fxn to RAM and run it - sets flash wait states for 90MHz
	//memcpy(&RamfuncsRunStart,&RamfuncsLoadStart,(unsigned long)&RamfuncsLoadSize);
	//InitFlash();

	HardwareInit();
	SoftwareInit();
	// @todo: Determine if I should keep default project code
    Task_Handle task;
    Error_Block eb;

    System_printf("enter main()\n");

    Error_init(&eb);
    task = Task_create(taskFxn, NULL, &eb);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }



    /*
    Uint16 i;
    for (i = 0; i < DRV8860_IN_SERIES; i++)
    {
        SPI_PushToQueue((i+10), RELAYS);
    }
    SPI_SendTx(RELAYS);
	*/

    BIOS_start();    /* does not return */
    return(0);
}

Void UpdateState()
{
    uint16_t * testPtr = NULL;


	static cellStatus_t status;

	while(1)
	{
		state currentState = GetState();
		uint8_t expanderInputPort0 = I2C_GetPortInput(PORT_0);
		uint8_t expanderInputPort1 = I2C_GetPortInput(PORT_1);
		switch (currentState)
		{
			case WAIT:
				// todo: Remove this test..

				SPI_DRV8860_GetFaults(testPtr, 1);

				if (expanderInputPort0 & START_BUTTON)
				{
					if (expanderInputPort0 & SWITCH_CHARGE_AND_BALANCE)
					{
						SetState(CHARGE_BALANCE);
					}
					else if (expanderInputPort0 & SWITCH_CHARGE)
					{
						// Set next state to CHARGE
						SetState(CHARGE);
					}
					else
					{
						// Balance only mode
						SetState(BALANCE);
					}
				}
				break;
			case CHARGE:

				// If switching from other state, update outputs
				if (lastState != CHARGE)
				{
					/// Update I2C Outputs
					I2C_SetPortOutput(PORT_0, START_INDICATOR);
					I2C_SetPortOutput(PORT_1, CHARGE_LED_YELLOW);
					I2C_SendOutput();

					/// Update SPI outputs
					Uint16 i;
					for (i = 0; i < DRV8860_IN_SERIES; i++)
					{
						/// Open all relays
						SPI_PushToQueue(0, RELAYS);
					}
					SPI_SendTx(RELAYS);

					/// Update screen
					/// todo
				}

				//status = CellStatus_WorstCellStatus(&cells[0], CELLS_IN_SERIES);

				// If cell is in critical state, go into error state
				if (status == CELL_CRITICAL)
				{
					SetState(ERROR);
					break;
				}
				else if (status == CELL_MAX_VOLT)
				{
					// Update LCD
					SetState(WAIT);
				}

				break;
			case BALANCE:
				break;
			case CHARGE_BALANCE:
				break;
			case ERROR:
				// Make system safe

				break;
			default:
				// Error
				break;
		}
		lastState = currentState;
	}
}
