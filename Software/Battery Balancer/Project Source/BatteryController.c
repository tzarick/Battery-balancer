/*
 * BatteryController.c
 *
 *  Created on: Mar 4, 2016
 *      Author: Sean Harrington
 */

#include "BatteryController.h"
#include "State.h"
#include "CellStatus.h"
#include "Timer.h"
#include "SPI.h"
#include "GPIO.h"
#include "I2C_Coms.h"
#include "Initialize.h"

//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------

extern cell_t cells[CELLS_IN_SERIES];

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

/// Hysteresis voltage (in millivolts) for bulk charge on/off. No cell
/// can be higher than CHARGE_ON_MAX_VOLT - CHARGE_ON_VOLT_TOL in order
/// for bulk charging to be enabled.
///
/// To avoid oscillation, this parameter should be set greater than
/// approx. (Ibulk) * Ri, where Ibulk is the DC charge current setpoint and
/// Ri is the typical internal resistance of a series cell at high SOC.
#define CHARGE_ON_VOLT_TOL			(50) 	//mV

/// The minimum voltage (in millivolts) that a cell should have in order
/// to be eligible for balancing.
#define BALANCE_MIN_VOLT_THRESH		(3000) 	//mV

///	Voltage difference threshold required to enable balancing for a cell.
/// If a cell is not currently balancing, it must be more than
/// BALANCE_ON_VOLT_TOL volts above the lowest cell in order to begin
/// being balanced.
#define BALANCE_ON_VOLT_TOL			(25) 	//mV

/// Voltage difference threshold required to disable balancing for a cell.
/// If a cell is currently balancing, it must be within
/// BALANCE_OFF_VOLT_TOL volts of the minimum cell in order to stop being
/// balanced.
///
/// If this value is negative, a balancing cell will be balanced until it
/// has a lower voltage (under load) than the minimum non-balancing cell.
#define BALANCE_OFF_VOLT_TOL		(-50)	//mV

/// Time that an individual cell must relax after balancing before it
/// can be balanced again.
#define BALANCE_RELAXATION_TIME		(5000)	// mSeconds

//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

Bool batteryController_NeedsBalanced(cell_t * cell);

void batteryController_TimerCallback(void * timerAddr);

//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------

void BatteryController_Task(void)
{
    SoftwareInit();

	uint16_t i;

	timer_t balanceDoneTimer;
	Timer_Setup(&balanceDoneTimer, NULL);

	// Run this task forever.
	// todo: Update this to variable that disables on error
	while(1)
	{
		I2C_Update();
		state_t state = GetState();		//States 0x00E0 and 0x00FF show up for no apparent reason
										//Valid States: WAIT, CHARGE, CHARGE_BALANCE, BALANCE, ERROR

		if ((state == CHARGE) || (state == CHARGE_BALANCE))
		{
			for (i = 0; i < CELLS_IN_SERIES; i++)
			{
				// If any cell above maximum cell voltage
				if (cells[i].voltage >= MAX_CELL_VOLTAGE)
				{
					// If only charging, go to wait. If charge and balance,
					// go to balance.
					(state == CHARGE) ? SetState(WAIT) : SetState(BALANCE);
				}
				if (cells[i].voltage >= MAX_CELL_CRITICAL_VOLTAGE)
				{
					// Go into error state.
					// todo: Assert error
					SetState(ERROR);
				}
			}
		}
		if ((state == BALANCE) || (state == CHARGE_BALANCE))
		{
			// todo: This probably needs changed. We will leave balancing
			// as soon as no cells are balancing but we should wait for
			// relaxation time before knowing for sure to leave this state
			Bool doneBalancing = TRUE;
			uint8_t cellCluster = 0;
			int sendQueue = 0;

			// cellCluster was an attempt to group SPI transmissions by driver - will probably require a complete redo
			for (i = 1; i <= CELLS_IN_SERIES; i++)
			{
				if (batteryController_NeedsBalanced(&cells[i]))
				{
					cells[i].balance = TRUE;
					doneBalancing = FALSE;
					cellCluster++;
				}
				else
				{
					cells[i].balance = FALSE;
					cellCluster++;
				}
				if (i % 8 == 0 && i != 0) {
					SPI_PushToQueue(cellCluster, RELAYS);
					SPI_SendTx(RELAYS);
					sendQueue++;
					cellCluster = 0;
				} else if (i == CELLS_IN_SERIES) {
					SPI_PushToQueue(cellCluster, RELAYS);
					SPI_SendTx(RELAYS);
					sendQueue++;
					cellCluster = 0;
				}
				if (sendQueue % DRV8860_IN_SERIES == 0 && sendQueue != 0) {
					//SPI_SendTx(RELAYS);
				}
				cellCluster <<= 1;
			}
			if (doneBalancing)
			{
				/// No cells balancing. Start timer that will transition
				/// states if cell doesn't balance in #BALANCE_RELAXATION_TIME
				if (!Timer_IsActive(&balanceDoneTimer))
				{
					Timer_Start(&balanceDoneTimer, BALANCE_RELAXATION_TIME);
				}
			}
			else
			{
				/// Cell still balancing. Stop balancer timeout timer.
				Timer_Stop(&balanceDoneTimer);
			}
		}
		if (state == WAIT)
		{
			//TEMPORARY

			//Process front panel inputs- Find out which port is which
			uint8_t expanderInputPort0 = I2C_GetPortInput(PORT_0);
			uint8_t expanderInputPort1 = I2C_GetPortInput(PORT_1);
		    uint16_t * testPtr = NULL;

			/// Update SPI outputs
			Uint16 i = 0;
			//DRV8860 is the buffer length
			//Send 2 Bytes (1 bit per relay) to each DRV8860s
			for (i = 0; i < DRV8860_IN_SERIES; i++)
			{
				/// Open all relays (1 == Close, 0 == Open)
				SPI_PushToQueue(0xF1, RELAYS);
			}
			SPI_SendTx(RELAYS);



			//SPI_DRV8860_GetFaults(testPtr, 1);

			//Should trigger when START is pressed - Spoiler: It doesn't
			if (expanderInputPort0 & START_BUTTON)
			{
				SetState(WAIT);

				/*if (expanderInputPort0 & SWITCH_CHARGE_AND_BALANCE)
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
				*/
			}
			else if (expanderInputPort0 & STOP_BUTTON) {
				SetState(WAIT);
			}
		}
		else {
			//todo: Handle Error State
		}
	}
}

Bool batteryController_NeedsBalanced(cell_t * cell)
{
	if (Timer_HasElapsed(&cell->relaxationTimer) &&
			(cell->balance == FALSE))
	{
		return ((cell->voltage - CellStatus_MinCellVolt()) >
				BALANCE_OFF_VOLT_TOL);
	}
	else
	{
		return FALSE;
	}
}

void batteryController_TimerCallback(void * timerAddr)
{
	/// Change state to WAIT if done balancing. Set to CHARGE if
	/// state was CHARGE_BALANCE
	(GetState() == BALANCE) ? SetState(WAIT) : SetState(CHARGE);
}
