/*
 * Battery_Status.h
 *
 *  Created on: Jan 28, 2016
 *      Author: Sean Harrington
 */

#ifndef PROJECT_INCLUDES_CELLSTATUS_H_
#define PROJECT_INCLUDES_CELLSTATUS_H_


//---------------------------------------------------------------------------------
// Common Includes

#include "Common_Includes.h"

#define CELLS_IN_SERIES		134

// todo: Update me. These are based on Samsung cells
#define MAX_CELL_VOLTAGE			4100	//mV
#define MAX_CELL_CRITICAL_VOLTAGE	4180	//mV

//---------------------------------------------------------------------------------
// Public variables

typedef Int16 cell_voltage;

typedef enum {
	CELL_OK,
	CELL_MAX_VOLT,
	CELL_CRITICAL
} cellStatus_t;

extern cell_voltage Cell_Voltages[CELLS_IN_SERIES];


//---------------------------------------------------------------------------------

cellStatus_t CellStatus_WorstCellStatus(void);

#endif /* PROJECT_INCLUDES_CELLSTATUS_H_ */
