/*
 * FrontPanel.c
 *
 *  Created on: Mar 4, 2016
 *      Author: Sean Harrington
 */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Private (Internal) functions
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------


void FrontPanel_Task(void)
{
	while (1)
	{
		state_t currentState = GetState();
		uint8_t expanderInputPort0 = I2C_GetPortInput(PORT_0);
		uint8_t expanderInputPort1 = I2C_GetPortInput(PORT_1);
		switch (currentState)
		{
			case WAIT:
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
}
