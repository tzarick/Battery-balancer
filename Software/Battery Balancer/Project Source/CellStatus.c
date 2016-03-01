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

error_t CellStatus_InitCell(cell_t * cell)
{
	cell->balance = FALSE;
	error_t retVal = Timer_Setup(&cell->relaxationTimer, NULL);
	cell->status = CELL_OK;
	cell->voltage = -1;
	return retVal;
}

cellStatus_t CellStatus_WorstCellStatus(cell_t * firstCell, Uint16 cellAmount)
{
	cellStatus_t result = CELL_OK;
	uint8_t i;

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
