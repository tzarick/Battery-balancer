/*
 * SPI.h
 *
 *  Created on: Feb 6, 2016
 *      Author: Sean Harrington
 */

#ifndef PROJECT_INCLUDES_SPI_H_
#define PROJECT_INCLUDES_SPI_H_

//---------------------------------------------------------------------------------
// Common Includes

#include "Common_Includes.h"
#include "Error.h"


//---------------------------------------------------------------------------------
// Public variables

// Devices connected to the SPI module
typedef enum
{
	LCD = 1,
	RELAYS = 2
} spi_device_t;

//---------------------------------------------------------------------------------

/// The SPI module is
void SPI_Init(void);

// todo: Document
error_t SPI_PushToQueue(UInt8 output, spi_device_t device);

// todo: Document
error_t SPI_PopFromQueue(Uint8 * queue_item);

// todo: Document
error_t SPI_SendTx(spi_device_t device);

#endif /* PROJECT_INCLUDES_SPI_H_ */
