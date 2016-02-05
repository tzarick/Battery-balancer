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
#define I2C_SEND_PORT1_EVENT	(Event_Id_05)
#define I2C_SEND_EVENT			(Event_Id_06)
#define I2C_COMPLETE_EVENT		(Event_Id_07)

#endif /* PROJECT_INCLUDES_EVENTS_H_ */
