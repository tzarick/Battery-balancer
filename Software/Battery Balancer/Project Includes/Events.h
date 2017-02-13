/*
 * Events.h
 *
 *  Created on: Jan 26, 2016
 *      Author: Sean Harrington
 */

#ifndef PROJECT_INCLUDES_EVENTS_H_
#define PROJECT_INCLUDES_EVENTS_H_

#define WAIT_EVENT				(0x1)
#define CHARGE_EVENT 			(0x2)
#define BALANCE_EVENT 			(0x4)
#define CHARGE_BALANCE_EVENT 	(0x8)
#define ERROR_EVENT				(0x10)
#define ALL_STATE_EVENTS		(0x1F)

#define I2C_SEND_DONE_EVENT		(Event_Id_05)
#define I2C_SEND_EVENT			(Event_Id_06)
#define I2C_COMPLETE_EVENT		(Event_Id_07)
#define I2C_ERROR_EVENT			(Event_Id_08)
#define I2C_READ_EVENT			(Event_Id_09)
#define I2C_NEW_DATA_EVENT		(Event_Id_10)

#define SPI_TX_READY_EVENT		(Event_Id_11)
#define SPI_DONE_EVENT			(Event_Id_12)

#endif /* PROJECT_INCLUDES_EVENTS_H_ */
