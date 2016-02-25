/*
 * Error.h
 *
 *  Created on: Jan 26, 2016
 *      Author: Sean Harrington
 */

#ifndef PROJECT_INCLUDES_ERROR_H_
#define PROJECT_INCLUDES_ERROR_H_

//---------------------------------------------------------------------------------
// Common Includes

#include "Common_Includes.h"


//---------------------------------------------------------------------------------
// Public variables

typedef enum {
	errNone,

	// I2C Related errors
	errI2cNotInitialized,
	errI2cSendOutputFail,
	errI2cReadInputFail,

	// CAN Errors
	errCanBusError,
	errCanRtrError,
	errCanTimeoutError,		//< CAN message not received fast enough

	// SPI Errors
	errSpiTxBufError,			//< Spi TX buffer is full
	errSpiRxBufError,			//< Spi RX buffer is full
	errSpiTxEmptyError,			//< Spi TX buffer is empty
	errSpiRxEmptyError,			//< Spi RX buffer is empty
	errSpiDeviceSwitchError,	//< Attempted to start a transaction while another
								// 	device was already transacting

	// System Fatal Errors
	errStackOverflow
} error_t;

/// Calls Error_HandleError if condition is not satisfied and never returns
#define ASSERT(condition, error)	if(!(condition)) 				\
									{								\
										Error_HandleError(error); 	\
									}								\

//---------------------------------------------------------------------------------

void Error_HandleError(error_t error);

#endif /* PROJECT_INCLUDES_ERROR_H_ */
