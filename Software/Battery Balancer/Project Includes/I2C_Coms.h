/*
 * I2C_Coms.h
 *
 *  Created on: Jan 23, 2016
 *      Author: Sean Harrington
 */

// @todo: File description

#ifndef INCLUDE_I2C_COMS_H_
#define INCLUDE_I2C_COMS_H_


//----------------------------------------------------------------------------------
// Common Includes

#include "Common_Includes.h"

//----------------------------------------------------------------------------------
// Public variables

/// Available states that the I2C module can be in.
typedef enum {
	I2C_NOT_IN_PROGRESS = 0,
	I2C_SENDING_READ = 1,
	I2C_DATA_READY = 3,
	I2C_SENDING_WRITE = 4,
	I2C_TXN_ERROR = 5
} i2c_states;

/// Available ports on the TCA9555 to read from and write to.
typedef enum {
	PORT_0,
	PORT_1
} tca9555_ports;

//----------------------------------------------------------------------------------
// Public functions

///	Sets up the I2C module to communicate with the TCA9555 GPIO
/// expander IC at 400 kHz. It also configures the inputs and outputs on the TCA9555
void I2C_Init();

/// Gets the modules current state.
///
///	@pre		I2C module is first initialized.
///	@returns	Current state as one values in #i2c_states type.
i2c_states I2C_GetState();

/// Gets the current input states on the TCA9555 IC.
///
///	@pre		I2C module is first initialized.
///	@pre		@pm{port} must be a valid port
///	@param		port -- The TCA9555 port to read.
///	@returns	Uint8 value describing port states with a bitfield. (This value can
///				be anded with a enum value in expander_input_sector_X type where X
///				is the port.
uint8_t I2C_GetPortInput(tca9555_ports port);

///	Sets the desired outputs states on the TCA9555 IC.
///
///	@pre		I2C module is first initialized.
///	@pre		@pm{port} must be a valid port
///	@param		port -- The TCA9555 port to set outputs on
///	@param		output -- The desired output as a bitfield. (This value can be set
///				by using expander_output_sector_X type values where X is the port
///				number to update.
void I2C_SetPortOutput(tca9555_ports port, uint8_t output);

///	Sends set port outputs to the TCA9555 IC.
///
///	@pre		I2C module is first initialized.
///	@pre		I2C state is not currently transaction (Use I2C_GetState)
void I2C_SendOutput(void);

//----------------------------------------------------------------------------------

#endif /* INCLUDE_I2C_COMS_H_ */
