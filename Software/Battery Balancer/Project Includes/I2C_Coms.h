/*
 * I2C_Coms.h
 *
 *  Created on: Jan 23, 2016
 *      Author: Sean Harrington
 */

// @todo: File description

#ifndef INCLUDE_I2C_COMS_H_
#define INCLUDE_I2C_COMS_H_


//------------------------------------------------
// Common Includes

#include "Common_Includes.h"

//------------------------------------------------
// Public variables

typedef enum {
	PORT_0,
	PORT_1
} tca9555_ports;

//------------------------------------------------
// Public functions
Void I2C_Init();

Uint8 I2C_GetPortInput(tca9555_ports port);

void I2C_SetPortOutput(tca9555_ports, Uint8 output);

//------------------------------------------------

#endif /* INCLUDE_I2C_COMS_H_ */
