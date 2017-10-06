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

static state_t lastState = WAIT;

static timer_t ledTimer;

cell_t cells[CELLS_IN_SERIES];

uint8_t gpio_out = 0;

static Uint8 output = START_INDICATOR;



void LED_Timer_Callback(void * timer_addr);

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


	// @todo: Determine if I should keep default project code

    BIOS_start();    /* does not return */
    return(0);
}

//This accomplishes the same thing as BatteryControllerTask - Why does this even exist?
Void UpdateState()
{
    uint16_t * testPtr = NULL;

    SoftwareInit();

    Timer_Setup(&ledTimer, &LED_Timer_Callback);
    Timer_Start(&ledTimer, 1000);
	static cellStatus_t status;

	while(1)
	{
		Timer_Update();
		I2C_Update();
		state_t currentState = GetState();
		uint8_t expanderInputPort0 = I2C_GetPortInput(PORT_0);
		uint8_t expanderInputPort1 = I2C_GetPortInput(PORT_1);
		switch (currentState)
		{
			case WAIT:
			{
				// todo: Remove this test..

				/// Update SPI outputs
				Uint16 i = 0;
				for (i = 0; i < DRV8860_IN_SERIES; i++)
				{
					/// Open all relays
					SPI_PushToQueue(0xFF, RELAYS);
				}
				SPI_SendTx(RELAYS);
				//DRV8860 is the buffer length or something like that
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
			}
			case CHARGE:
			{
				// If switching from other state, update outputs
				if (lastState != CHARGE)
				{
					/// Update I2C Outputs
					I2C_SetPortOutput(PORT_0, START_INDICATOR);
					I2C_SetPortOutput(PORT_1, CHARGE_LED_YELLOW);
					//I2C_SendOutput();

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
			}
			case BALANCE:
			{
				break;
			}
			case CHARGE_BALANCE:
			{
				break;
			}
			case ERROR:
			{
				// Make system safe
				break;
			}
			default:
				// Error
				break;
		}
		lastState = currentState;
	}
}

void LED_Timer_Callback(void * timer_addr)
{
	Timer_Start(&ledTimer, 1000);
	output ^= START_INDICATOR;
	I2C_SetPortOutput(PORT_0, output);
	//I2C_SendOutput();

	gpio_out = I2C_GetPortInput(PORT_0);

}
