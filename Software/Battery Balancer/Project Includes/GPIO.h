/*
 * GPIO.h
 *
 *  Created on: Jan 23, 2016
 *      Author: Sean Harrington
 */

#ifndef PROJECT_INCLUDES_GPIO_H_
#define PROJECT_INCLUDES_GPIO_H_

//---------------------------------------------------------------------------------
// Common Includes

#include "Common_Includes.h"


//------------------------------------------------
// Public variables

#define BULK_CHARGE_POWER_RELAY_ON 	(GpioDataRegs.GPASET.bit.GPIO5)
#define BULK_CHARGE_POWER_RELAY_OFF	(GpioDataRegs.GPACLEAR.bit.GPIO5)
#define BULK_CHARGE_ENABLE 			(GpioDataRegs.GPASET.bit.GPIO11)
#define BULK_CHARGE_DISABLE			(GpioDataRegs.GPACLEAR.bit.GPIO11)
#define BALANCE_RELAY_ENABLE		(GpioDataRegs.GPASET.bit.GPIO9)
#define BLANACE_RELAY_DISABLE		(GpioDataRegs.GPACLEAR.bit.GPIO9)

// GPIO Expander Port 0 output pins (bitfield)
typedef enum {
	ERROR_INDICATOR = 1,
	STOP_INDICATOR 	= 4,
	START_INDICATOR = 64
} expander_output_sector_1;

// GPIO Expander Port 1 output pins (bitfield)
typedef enum {
	CHARGE_LED_GREEN 	= 1,
	CHARGE_LED_YELLOW 	= 2,
	BALANCE_LED_GREEN 	= 4,
	BALANCE_LED_YELLOW	= 8,
	ALARM 				= 64
} expander_output_sector_2;

// GPIO Expander Port 1 input pins (bitfield)
typedef enum {
	ERROR_BUTTON 				= 2,
	STOP_BUTTON 				= 8,
	SWITCH_CHARGE_AND_BALANCE 	= 16,
	SWITCH_CHARGE				= 32,
	START_BUTTON				= 128
} expander_input_sector_1;

// GPIO Expander Port 2 input pins (bitfield)
typedef enum {
	LCD_UP_BUTTON	= 16,
	LCD_DOWN_BUTTON	= 32,
} expander_input_sector_2;

Void Gpio_Init();

//------------------------------------------------

#endif /* PROJECT_INCLUDES_GPIO_H_ */
