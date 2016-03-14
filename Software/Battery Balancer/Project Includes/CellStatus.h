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
#include "Timer.h"
#include "Error.h"

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

typedef struct
{
	cell_voltage voltage;		///< Cell voltage in mV
	cellStatus_t status;		///< The voltage status
	Bool balance;				///< Whether the balance relay is active for cell
	timer_t relaxationTimer;	///< Cell relaxation time after relay opened
} cell_t;

//---------------------------------------------------------------------------------

error_t CellStatus_InitCell(cell_t * cell);

///
cellStatus_t CellStatus_WorstCellStatus(cell_t * firstCell, Uint16 cellAmount);

cell_voltage CellStatus_MinCellVolt(void);

#endif /* PROJECT_INCLUDES_CELLSTATUS_H_ */
