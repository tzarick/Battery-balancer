/*
 * CellStatus.c
 *
 *  Created on: Feb 6, 2016
 *      Author: Sean Harrington
 */

#include "CellStatus.h"

//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------

// todo: THIS MAKES VERY LITTLE SENSE. FIX
extern cell_voltage Cell_Voltages[CELLS_IN_SERIES];

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

// todo: Move cellStatus init here, clean up global array?

cellStatus_t CellStatus_WorstCellStatus(void)
{
	cellStatus_t result = CELL_OK;
	Uint8 i;
	for (i = 0; i < CELLS_IN_SERIES; i++)
	{
		if (Cell_Voltages[i] >= MAX_CELL_CRITICAL_VOLTAGE)
		{
			/// Immediately return to handle situation ASAP
			return CELL_CRITICAL;
		}
		if (Cell_Voltages[i] >= MAX_CELL_VOLTAGE)
		{
			result = CELL_MAX_VOLT;
		}
	}
	return result;
}
